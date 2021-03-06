#ifndef BEER_DIR_H_INCLUDED
#define BEER_DIR_H_INCLUDED

/*
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the
 *    following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * <COPYRIGHT HOLDER> OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

enum beer_dir_type {
	BEER_DIR_XLOG,
	BEER_DIR_SNAPSHOT
};

struct beer_dir_file {
	uint64_t lsn;
	char *name;
};

struct beer_dir {
	enum beer_dir_type type;
	char *path;
	struct beer_dir_file *files;
	int count;
};

void beer_dir_init(struct beer_dir *d, enum beer_dir_type type);
void beer_dir_free(struct beer_dir *d);

int beer_dir_scan(struct beer_dir *d, char *path);

int beer_dir_match_gt(struct beer_dir *d, uint64_t *out);
int beer_dir_match_inc(struct beer_dir *d, uint64_t lsn, uint64_t *out);

#endif
