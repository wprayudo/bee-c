#define BEE_URI "localhost:3301"
#define BUFFER_SIZE   256*1024

#define log_error(fmt, ...)  fprintf(stderr, "%s:%d E>" fmt, __func__, __LINE__, ##__VA_ARGS__)

#include <stddef.h>
#include <sys/types.h>
#include <stdio.h>
#include <inttypes.h>

#include <bee/bee.h>
#include <beer/beer_net.h>
#include <beer/beer_opt.h>
#include <msgpuck/msgpuck.h>

int
beer_request_set_sspace(struct beer_request *req, const char *space,
		       uint32_t slen)
{
	if (!req->stream || !space) return -1;
	int32_t sno = beer_get_spaceno(req->stream, space, slen);
	if (sno == -1) return -1;
	return beer_request_set_space(req, sno);
}

int
beer_request_set_sspacez(struct beer_request *req, const char *space)
{
	if (!req->stream || !space) return -1;
	return beer_request_set_sspace(req, space, strlen(space));
}


int
beer_request_set_sindex(struct beer_request *req, const char *index,
		       uint32_t ilen)
{
	if (!req->stream || !index || !req->space_id) return -1;
	int32_t ino = beer_get_indexno(req->stream, req->space_id, index, ilen);
	if (ino == -1) return -1;
	return beer_request_set_index(req, ino);
}

int
beer_request_set_sindexz(struct beer_request *req, const char *index)
{
	if (!req->stream || !index) return -1;
	return beer_request_set_sindex(req, index, strlen(index));
}


int profile_space_no;

struct msg_profile {
	int id;
	char *user;
	int like;
	char *msg;
};

struct beer_stream *bee_connection(const char *uri) {
	struct beer_stream *connection = beer_net(NULL);
	if (beer_set(connection, BEER_OPT_URI, uri) == -1)
		goto rollback;
	beer_set(connection, BEER_OPT_SEND_BUF, BUFFER_SIZE);
	beer_set(connection, BEER_OPT_RECV_BUF, BUFFER_SIZE);
	if (beer_connect(connection) == -1) {
		log_error("Failed to connect (%s)", beer_strerror(connection));
		goto rollback;
	}
	return connection;
rollback:
	beer_stream_free(connection);
	return NULL;
}

void msg_profile_free(struct msg_profile *msg) {
	if (msg) {
		free(msg->msg);
		free(msg->user);
		free(msg);
	}
}

int msg_profile_decode(struct beer_reply *rpl, struct msg_profile **rv) {
	uint32_t str_len = 0;
	if (rpl->code != 0) {
		log_error("Query error %d: %.*s", (int )BEER_REPLY_ERR(rpl),
			  (int )(rpl->error_end - rpl->error), rpl->error);
		return -1;
	}
	if (mp_typeof(*rpl->data) != MP_ARRAY) {
		log_error("Bad reply format");
		return -1;
	} else if (mp_decode_array(&rpl->data) == 0) {
		return 0;
	} else if (mp_decode_array(&rpl->data) > 0) {
		log_error("Bad reply format");
		return -1;
	}
	struct msg_profile *profile = malloc(sizeof(struct msg_profile));
	if (profile == NULL) {
		log_error("OOM");
		return -1;
	}
	const char *data = rpl->data;
	if (mp_typeof(*data) != MP_ARRAY) {
		log_error("Bad reply format");
		goto rollback;
	}
	uint32_t len = mp_decode_array(&data);
	if (len < 2) {
		log_error("Bad reply format");
		goto rollback;
	}
	if (mp_typeof(*data) != MP_UINT) {
		log_error("Bad reply format");
		goto rollback;
	}
	profile->id = mp_decode_uint(&data);
	if (mp_typeof(*data) != MP_STR) {
		log_error("Bad reply format");
		goto rollback;
	}
	profile->user = (char *)mp_decode_str(&data, &str_len);
	profile->user = strndup(profile->user, str_len);
	if (profile->user == NULL) {
		log_error("OOM");
		goto rollback;
	}
	if (mp_typeof(*data) != MP_UINT) {
		log_error("Bad reply format");
		goto rollback;
	}
	profile->like = mp_decode_uint(&data);
	if (mp_typeof(*data) != MP_STR) {
		log_error("Bad reply format");
		goto rollback;
	}
	profile->msg = (char *)mp_decode_str(&data, &str_len);
	profile->msg = strndup(profile->msg, str_len);
	if (profile->msg == NULL) {
		log_error("OOM");
		goto rollback;
	}
	len -= 4;
	while (len > 0) mp_next(&data);
	rpl->data = data;
	*rv = profile;
	return 0;
rollback:
	msg_profile_free(profile);
	*rv = NULL;
	return -1;
}

int msg_profile_get(struct beer_stream *beer, int id, struct msg_profile **rv) {
	static struct beer_stream *obj = NULL;
	static struct beer_reply  *rpl = NULL;
	if (!obj) {
		obj = beer_object(NULL);
		if (!obj) return -1;
	}
	if (!rpl) {
		rpl = beer_reply_init(NULL);
		if (!rpl) return -1;
	}
	beer_object_reset(obj);
	beer_object_add_array(obj, 1);
	beer_object_add_int(obj, id);
	if (beer_select(beer, profile_space_no, 0, UINT32_MAX, 0, 0, obj) == -1) {
		log_error("Failed to append request");
		return -1;
	}
	if (beer_flush(beer) == -1) {
		log_error("Failed to send request (%s)", beer_strerror(beer));
		return -1;
	}
	if (beer->read_reply(beer, rpl) == -1) {
		log_error("Failed to recv/parse result");
		if (beer_error(beer)) log_error("%s", beer_strerror(beer));
		return -1;
	}
	return msg_profile_decode(rpl, rv);
}

int msg_profile_get_multi(struct beer_stream *beer, int *id, size_t cnt,
			  struct msg_profile ***rv) {
	static struct beer_request *req = NULL;
	if (!req) {
		req = beer_request_select(NULL, beer);
		beer_request_set_sspacez(req, "messages");
		beer_request_set_sindexz(req, "primary");
		if (!req) return -1;
	}
	for (int i = 0; i < cnt; ++i) {
		beer_request_set_key_format(req, "[%d]", id[i]);
		if (beer_request_encode(req) == -1) {
			log_error("OOM");
			return -1;
		}
	}
	if (beer_flush(beer) == -1) {
		log_error("Failed to send result (%s)", beer_strerror(beer));
		return -1;
	}
	struct msg_profile **retval = calloc(cnt + 1, sizeof(struct msg_profile *));
	size_t rcvd = 0; retval[cnt] = NULL;
	struct beer_iter it; beer_iter_reply(NULL, beer);
	while (beer_next(&it)) {
		int rv = msg_profile_decode(BEER_IREPLY_PTR(&it), &retval[rcvd]);
		if (retval[rcvd] != NULL) ++rcvd;
		if (rv == -1) {
			for (int i = 0; i < rcvd; i++)
				msg_profile_free(i);
			free(retval);
			while (beer_next(&it));
			return -1;
		}
	}
	*rv = retval;
	return rcvd;
}

int msg_profile_like_add(struct beer_stream *beer, int id) {
	static struct beer_request *req = NULL;
	static struct beer_stream  *obj = NULL;
	static struct beer_reply   *rpl = NULL;
	size_t str_len = 0;
	if (!obj) {
		obj = beer_update_container(NULL);
		if (obj == NULL) {
			log_error("OOM");
			return -1;
		}
		/* update like counter and prepend 'liked ' to msg */
		if (beer_update_arith_int(obj, 2, '+', 1) == -1 ||
		    beer_update_splice(obj, 3, 0, 0, "liked ", 6) == -1) {
			log_error("OOM");
			return -1;
		}
		beer_update_container_close(obj);
	}
	if (!req) {
		req = beer_request_update(NULL, beer);
		beer_request_set_sspacez (req, "messages");
		beer_request_set_ssindexz(req, "primary");
		beer_request_set_tuple(req, obj);
		if (!req) return -1;
	}
	if (!rpl) {
		rpl = beer_reply_init(NULL);
		if (!rpl) return -1;
	}
	beer_request_set_key_format(req, "[%d]", id);
	if (beer_request_encode(req) == -1) {
		log_error("OOM");
		return -1;
	}
	if (beer_flush(beer) == -1) {
		log_error("Failed to send request (%s)", beer_strerror(beer));
		return -1;
	}
	if (beer->read_reply(beer, rpl) == -1) {
		log_error("Failed to recv/parse result");
		if (beer_error(beer)) log_error("%s", beer_strerror(beer));
		return -1;
	}
	if (rpl->code) {
		log_error("Query error %d: %.*s", (int )BEER_REPLY_ERR(rpl),
			  (int )(rpl->error_end - rpl->error), rpl->error);
		return -1;
	}
	return 0;
}

int msg_profile_put(struct beer_stream *beer, int id, const char *usr,
		    size_t usr_len, int likes, const char *msg, size_t msg_len) {
	static struct beer_request *req = NULL;
	static struct beer_reply   *rpl = NULL;
	if (!req) {
		req = beer_request_update(NULL, beer);
		if (!req) return -1;
		if (beer_request_set_sspacez (req, "messages") == -1) {
			log_error("failed to find space 'messages'");
			return -1;
		}
		if (beer_request_set_ssindexz(req, "primary") == -1) {
			log_error("failed to find index 'primary' in space "
				  "'message'");
			return -1;
		}
	}
	if (msg_profile_create(req, id, usr, usr_len, likes, msg, msg_len) == -1) {
		log_error("OOM");
		return -1;
	}
	if (beer_request_set_tuple_format(req, "[%d%.*s%d%.*s]", id, usr_len,
					 usr, likes, msg_len, msg) == -1) {
		log_error("OOM");
		return -1;
	}
	if (beer_request_encode(req) == -1) {
		log_error("OOM");
		return -1;
	}
	if (beer_flush(beer) == -1) {
		log_error("Failed to send request (%s)", beer_strerror(beer));
		return -1;
	}
	if (beer->read_reply(beer, rpl) == -1) {
		log_error("Failed to recv/parse result");
		if (beer_error(beer)) log_error("%s", beer_strerror(beer));
		return -1;
	}
	if (rpl->code) {
		log_error("Query error %d: %.*s", (int )BEER_REPLY_ERR(rpl),
			  (int )(rpl->error_end - rpl->error), rpl->error);
		return -1;
	}
	return 0;
}

int msg_profile_create(struct beer_request *req, int id, const char *usr,
		       size_t usr_len, int likes, const char *msg, size_t msg_len) {
	if (beer_request_set_tuple_format(req, "[%d%.*s%d%.*s]", id, usr_len,
					 usr, likes, msg_len, msg) == -1) {
		return -1;
	}
	return 0;
}

int msg_profile_create_alt(struct beer_stream *obj, int id, const char *usr,
			   size_t usr_len, int likes, const char *msg,
			   size_t msg_len) {
	if (beer_object_add_array(obj, 4) == -1 ||
	    beer_object_add_int(obj, id) == -1 ||
	    beer_object_add_string(obj, usr, usr_len) == -1 ||
	    beer_object_add_int(obj, likes) == -1 ||
	    beer_object_add_string(obj, msg, msg_len) == -1) {
		log_error("OOM");
		return -1;
	}
	return 0;
}

int main() {
	struct beer_stream *beer = bee_connection(BEE_URI);
	profile_space_no = beer_get_spaceno(beer, "messages", strlen("messages"));
	beer_stream_free(beer);
}
