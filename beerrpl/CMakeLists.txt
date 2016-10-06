#============================================================================#
# build flags
#============================================================================#

# default flags
if (${CMAKE_BUILD_TYPE} STREQUAL "None")
    set (beerrpl_cflags "-std=gnu99")
else()
    set (beerrpl_cflags "-std=gnu99 -Wall -Wextra")
    set (beerrpl_cflags "${beerrpl_cflags} -Wno-sign-compare -Wno-strict-aliasing")
endif()

# Only add -Werror if it's a debug build, done by developers.
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set (beerrpl_cflags "${beerrpl_cflags} -Werror")
endif()

#============================================================================#
# Build beer rpl project
#============================================================================#

#
# source files
#

set (beerrpl_sources beer_log.c beer_dir.c beer_xlog.c beer_snapshot.c beer_rpl.c
     ${CMAKE_SOURCE_DIR}/third_party/crc32.c)

#----------------------------------------------------------------------------#
# Builds
#----------------------------------------------------------------------------#

# Here we manage to build static/dynamic libraries ourselves,
# do not use the top level settings.
string(REPLACE "-static-libgcc" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
string(REPLACE "-static" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

if (CMAKE_COMPILER_IS_GNUCC AND NOT CMAKE_COMPILER_IS_CLANG)
    set (beer_cflags "${beer_cflags} -static-libgcc")
endif()

#
# Static library
#

project(beerrpl)
add_library(beerrpl STATIC ${beerrpl_sources})
set_target_properties(beerrpl PROPERTIES COMPILE_FLAGS "${beerrpl_cflags}")
set_target_properties(beerrpl PROPERTIES VERSION ${LIBBEER_VERSION} SOVERSION ${LIBBEER_SOVERSION})
set_target_properties(beerrpl PROPERTIES OUTPUT_NAME "beerpl")

#
# Shared library
#

project(beerrpl_shared)
add_library(beerrpl_shared SHARED ${beerrpl_sources})
target_link_libraries(beerrpl_shared beer_shared beernet_shared)
set_target_properties(beerrpl_shared PROPERTIES OUTPUT_NAME beerrpl)
set_target_properties(beerrpl_shared PROPERTIES COMPILE_FLAGS "${beerrpl_cflags}")
set_target_properties(beerrpl_shared PROPERTIES VERSION ${LIBBEER_VERSION} SOVERSION ${LIBBEER_SOVERSION})
set_target_properties(beerrpl_shared PROPERTIES OUTPUT_NAME "beerpl")

#----------------------------------------------------------------------------#
# Install
#----------------------------------------------------------------------------#

install (TARGETS beerrpl ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install (TARGETS beerrpl_shared LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
