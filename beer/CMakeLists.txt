#============================================================================#
# build flags
#============================================================================#

set(LIBBEER_NAME "beer")
set(LIBBEER_VERSIONMAJOR "2.0")
set(LIBBEER_VERSIONMINOR "0")
set(LIBBEER_VERSION "${LIBBEER_VERSIONMAJOR}.${LIBBEER_VERSIONMINOR}")
set(LIBBEER_SOVERSION "${LIBBEER_VERSIONMAJOR}")

#============================================================================#
# Build beer project
#============================================================================#

if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
    set(CMAKE_INSTALL_LIBDIR lib)
endif(NOT DEFINED CMAKE_INSTALL_LIBDIR)

## source files
set (BEER_SOURCES
     ${CMAKE_CURRENT_SOURCE_DIR}/beer_mem.c
     ${CMAKE_CURRENT_SOURCE_DIR}/beer_reply.c
     ${CMAKE_CURRENT_SOURCE_DIR}/beer_stream.c
     ${CMAKE_CURRENT_SOURCE_DIR}/beer_buf.c
     ${CMAKE_CURRENT_SOURCE_DIR}/beer_object.c
     ${CMAKE_CURRENT_SOURCE_DIR}/beer_ping.c
     ${CMAKE_CURRENT_SOURCE_DIR}/beer_auth.c
     ${CMAKE_CURRENT_SOURCE_DIR}/beer_select.c
     ${CMAKE_CURRENT_SOURCE_DIR}/beer_insert.c
     ${CMAKE_CURRENT_SOURCE_DIR}/beer_call.c
     ${CMAKE_CURRENT_SOURCE_DIR}/beer_delete.c
     ${CMAKE_CURRENT_SOURCE_DIR}/beer_update.c
     ${CMAKE_CURRENT_SOURCE_DIR}/beer_assoc.c
     ${CMAKE_CURRENT_SOURCE_DIR}/beer_schema.c
     ${CMAKE_CURRENT_SOURCE_DIR}/beer_iter.c
     ${CMAKE_CURRENT_SOURCE_DIR}/beer_request.c
     ${CMAKE_CURRENT_SOURCE_DIR}/beer_iob.c
     ${CMAKE_CURRENT_SOURCE_DIR}/beer_io.c
     ${CMAKE_CURRENT_SOURCE_DIR}/beer_opt.c
     ${CMAKE_CURRENT_SOURCE_DIR}/beer_net.c
     ${PROJECT_SOURCE_DIR}/third_party/uri.c
     ${PROJECT_SOURCE_DIR}/third_party/sha1.c
     ${PROJECT_SOURCE_DIR}/third_party/base64.c
     ${PROJECT_SOURCE_DIR}/third_party/PMurHash.c
     ${PROJECT_SOURCE_DIR}/third_party/msgpuck.c
)

#----------------------------------------------------------------------------#
# Builds
#----------------------------------------------------------------------------#

## Static library
project(beer)
add_library(${PROJECT_NAME} STATIC ${BEER_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION   ${LIBBEER_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${LIBBEER_SOVERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "bee")
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS -fPIC)

install (TARGETS ${PROJECT_NAME}
         ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
         LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
         COMPONENT library)

## Shared library
project(beer_shared)
add_library(${PROJECT_NAME} SHARED ${BEER_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION   ${LIBBEER_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${LIBBEER_SOVERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "bee")

install (TARGETS ${PROJECT_NAME}
         ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
         LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
         COMPONENT library)

#----------------------------------------------------------------------------#
# Install
#----------------------------------------------------------------------------#

message(STATUS "  * lib${LIBBEER_NAME}.so.${LIBBEER_VERSION}      ")
