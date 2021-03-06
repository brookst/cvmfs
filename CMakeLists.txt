#
# CMake build script to configure and build CernVM-FS and all it's
# external dependencies, if they are statically linked into the binaries
#
# See externals/CMake-Register_External_Lib.txt for details on external inclusion
#

cmake_minimum_required (VERSION 2.6.2)
set (PROJECT_NAME "CernVM-FS")
project (${PROJECT_NAME})

#
# The version numbers
#
# DON'T DELETE
## CVMFS_VERSION 2.1.13
#---------------------
set (CernVM-FS_VERSION_MAJOR 2)
set (CernVM-FS_VERSION_MINOR 1)
set (CernVM-FS_VERSION_PATCH 13)
set (CernVM-FS_VERSION_STRING "${CernVM-FS_VERSION_MAJOR}.${CernVM-FS_VERSION_MINOR}.${CernVM-FS_VERSION_PATCH}")

#
# set the path where cmake looks for additional modules
#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

#
# Set install prefix to /usr.  Cvmfs is not relocatable.
#
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX
    "/usr" CACHE PATH "/usr install prefix" FORCE
  )
else (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  if (NOT ${CMAKE_INSTALL_PREFIX} STREQUAL "/usr")
    Message("Warning: CernVM-FS is not relotable and expects to be installed under /usr")
  endif ()
endif (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)


#
# check if we are on Mac OS X
#
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set (MACOSX TRUE)
else ()
	set (MACOSX FALSE)
endif ()

#
# check if we use Clang
#
if (CMAKE_CXX_COMPILER MATCHES ".*clang")
	set(USING_CLANG 1)
endif (CMAKE_CXX_COMPILER MATCHES ".*clang")

#
# provide some options to the user
#
option (BUILD_CVMFS "Build the CernVM-FS FUSE module" ON)
if (MACOSX)
	option (BUILD_LIBCVMFS "Build the CernVM-FS client library" OFF)
else ()
	option (BUILD_LIBCVMFS "Build the CernVM-FS client library" ON)
endif()
option (BUILD_SERVER "Build writer's end programs" ON)
option (BUILD_SERVER_DEBUG "Build writer's end programs with debug symbols and debug outputs" OFF)
option (BUILD_UNITTESTS "Build the CernVM-FS unit test set" OFF)
option (SQLITE3_BUILTIN "Don't use system SQLite3" ON)
option (LIBCURL_BUILTIN "Don't use system libcurl" ON)
option (ZLIB_BUILTIN "Don't use system zlib" ON)
option (SPARSEHASH_BUILTIN "Don't use system installation of google sparse hash" ON)
option (LEVELDB_BUILTIN "Don't use system leveldb" ON)
option (INSTALL_MOUNT_SCRIPTS "Install CernVM-FS mount tools in /etc and /sbin" ON)

#
# define where to find the external dependencies
#
# for the external libraries there should be a CVMFS-CMakeLists.txt file provided
# in the given directory, in order to configure and compile them properly
#
set (EXTERNALS_LIB_LOCATION       "${CMAKE_SOURCE_DIR}/externals")
set (EXTERNALS_BUILD_LOCATION     "${CMAKE_BINARY_DIR}/externals")
set (SQLITE3_BUILTIN_LOCATION     "${EXTERNALS_BUILD_LOCATION}/build_sqlite3")
set (CARES_BUILTIN_LOCATION       "${EXTERNALS_BUILD_LOCATION}/build_c-ares")
set (LIBCURL_BUILTIN_LOCATION     "${EXTERNALS_BUILD_LOCATION}/build_libcurl")
set (ZLIB_BUILTIN_LOCATION        "${EXTERNALS_BUILD_LOCATION}/build_zlib")
set (SPARSEHASH_BUILTIN_LOCATION  "${EXTERNALS_BUILD_LOCATION}/build_sparsehash")
set (LEVELDB_BUILTIN_LOCATION     "${EXTERNALS_BUILD_LOCATION}/build_leveldb")
set (VJSON_BUILTIN_LOCATION       "${EXTERNALS_BUILD_LOCATION}/build_vjson")
set (GOOGLETEST_BUILTIN_LOCATION  "${EXTERNALS_BUILD_LOCATION}/build_googletest")

# create the directory for controlled out of source building of external stuff
file (MAKE_DIRECTORY ${EXTERNALS_BUILD_LOCATION})

#
# run the bootstrap shellscript (not needed in the distributed version of the source)
#

if (EXISTS "${CMAKE_SOURCE_DIR}/bootstrap.sh")
	message (STATUS "running bootstrap.sh ...")
	execute_process (
    COMMAND           sh ${CMAKE_SOURCE_DIR}/bootstrap.sh ${EXTERNALS_BUILD_LOCATION}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE   BOOTSTRAPPING_RESULT
	)
  if (BOOTSTRAPPING_RESULT GREATER 0)
    message (FATAL_ERROR "bootstrapping failed")
  endif (BOOTSTRAPPING_RESULT GREATER 0)
endif (EXISTS "${CMAKE_SOURCE_DIR}/bootstrap.sh")

#
# set some default flags
#
# flags in CMAKE_C**_FLAGS are always passed to the compiler
#
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fno-exceptions -fno-strict-aliasing -fasynchronous-unwind-tables -fno-omit-frame-pointer -fvisibility=hidden -Wall -D_REENTRANT -D__EXTENSIONS__ -D_LARGEFILE64_SOURCE -D__LARGE64_FILES")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fno-strict-aliasing -fasynchronous-unwind-tables -fno-omit-frame-pointer -fvisibility=hidden -Wall -D_REENTRANT -D__EXTENSIONS__ -D_LARGEFILE64_SOURCE -D__LARGE64_FILES")

if (CMAKE_SIZEOF_VOID_P EQUAL 4)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=i686")
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=i686")
endif (CMAKE_SIZEOF_VOID_P EQUAL 4)

if (NOT USING_CLANG)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-optimize-sibling-calls")
	set (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fno-optimize-sibling-calls")
endif (NOT USING_CLANG)

set (INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES})

#
# check existence of include files
#
include (CheckIncludeFile)
check_include_file (sys/xattr.h HAVE_XATTR_H)
check_include_file (zlib.h HAVE_ZLIB_H)
check_include_file (netinet/in.h HAVE_NETINET_IN_H)
check_include_file (arpa/inet.h HAVE_ARPA_INET_H)
check_include_file (sys/socket.h HAVE_SYS_SOCKET_H)
check_include_file (sys/un.h HAVE_SYS_UN_H)
check_include_file (sys/time.h HAVE_SYS_TIME_H)
check_include_file (sys/uio.h HAVE_SYS_UIO_H)
check_include_file (sys/stat.h HAVE_SYS_STAT_H)
check_include_file (sys/types.h HAVE_SYS_TYPES_H)
check_include_file (sys/wait.h HAVE_SYS_WAIT_H)
check_include_file (sys/select.h HAVE_SYS_SELECT_H)
check_include_file (pthread.h HAVE_PTHREAD_H)
check_include_file (termios.h HAVE_TERMIOS_H)
check_include_file (utime.h HAVE_UTIME_H)
check_include_file (signal.h HAVE_SIGNAL_H)
check_include_file (errno.h HAVE_ERRNO_H)
check_include_file (dirent.h HAVE_DIRENT_H)
check_include_file (unistd.h HAVE_UNISTD_H)
check_include_file (fcntl.h HAVE_FCNTL_H)
check_include_file (netdb.h HAVE_NETDB_H)
check_include_file (syslog.h HAVE_SYSLOG_H)
check_include_file (sys/resource.h HAVE_SYS_RESOURCE_H)
check_include_file (execinfo.h HAVE_EXECINFO_H)
check_include_file (poll.h HAVE_POLL_H)
if (NOT MACOSX)
	check_include_file (sys/statfs.h HAVE_SYS_STATFS_H)
  check_include_file (attr/xattr.h HAVE_ATTR_XATTR_H)
endif ()

#
# configure the config.h.in file
#
configure_file (
	"${CMAKE_SOURCE_DIR}/config_cmake.h.in"
	"${CMAKE_BINARY_DIR}/cvmfs_config.h"
)
set (INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES} ${CMAKE_BINARY_DIR})

include_directories (${INCLUDE_DIRECTORIES})



#
# set properties for configurable libraries
#
# some libraries are statically linked and has to be compiled from source
# therefore we have to include the specific CVMFS-CMakeLists.txt from the
# different 3rd-party locations.
# If dynamic linking is desired we have to look for the presence of the libs
# in the system and set the LIBRARY and INCLUDE variables properly
#
# either ***_LIBRARIES or ***_ARCHIVE has to be defined for all externals
# in order to successfully link the targets.
#

if (MACOSX)
  find_package (Fuse4x REQUIRED)
  set (INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES} ${FUSE4X_INCLUDE_DIR})
  set (FUSE_LIBRARIES ${FUSE4X_LIBRARIES}) # just abstract the difference here... they are compatible
else (MACOSX)
  find_package (FUSE REQUIRED)
  set (INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES} ${FUSE_INCLUDE_DIR})
endif (MACOSX)

if (LIBCURL_BUILTIN)
  include (${CARES_BUILTIN_LOCATION}/CVMFS-CMakeLists.txt)
  include (${LIBCURL_BUILTIN_LOCATION}/CVMFS-CMakeLists.txt)
  set (CURL_LIBRARIES "")
else (LIBCURL_BUILTIN)
  find_package (CURL REQUIRED)
  set (INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES} ${CURL_INCLUDE_DIRS})
endif (LIBCURL_BUILTIN)

if (SQLITE3_BUILTIN)
  include (${SQLITE3_BUILTIN_LOCATION}/CVMFS-CMakeLists.txt)
  set (SQLITE3_LIBRARY "")
else (SQLITE3_BUILTIN)
  find_package (SQLite3 REQUIRED)
  set (INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES} ${SQLITE3_INCLUDE_DIR})
endif (SQLITE3_BUILTIN)

if (ZLIB_BUILTIN)
  include (${ZLIB_BUILTIN_LOCATION}/CVMFS-CMakeLists.txt)
  set (ZLIB_LIBRARIES "")
else (ZLIB_BUILTIN)
  find_package (ZLIB REQUIRED)
  set (INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES} ${ZLIB_INCLUDE_DIRS})
endif (ZLIB_BUILTIN)

if (SPARSEHASH_BUILTIN)
  include (${SPARSEHASH_BUILTIN_LOCATION}/CVMFS-CMakeLists.txt)
  set (INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES} ${SPARSEHASH_BUILTIN_LOCATION}/src)
else (SPARSEHASH_BUILTIN)
  find_package(Sparsehash)
  set (INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES} ${SPARSEHASH_INCLUDE_DIR})
endif (SPARSEHASH_BUILTIN)

if (LEVELDB_BUILTIN)
  include (${LEVELDB_BUILTIN_LOCATION}/CVMFS-CMakeLists.txt)
  set (INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES} ${LEVELDB_BUILTIN_LOCATION}/src)
else (LEVELDB_BUILTIN)
  find_package(leveldb)
  set (INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES} ${LEVELDB_INCLUDE_DIR})
endif (LEVELDB_BUILTIN)

if (NOT MACOSX)
  set (HAVE_LIB_RT TRUE)
  set (RT_LIBRARY "rt")
else (NOT MACOSX)
  set (HAVE_LIB_RT FALSE)
  set (RT_LIBRARY "")
endif (NOT MACOSX)

find_package (OpenSSL REQUIRED)
set (INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES} ${OPENSSL_INCLUDE_DIR})

set (INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES} ${GOOGLETEST_BUILTIN_LOCATION}/include)

#
# go for the actual compilation
#
add_subdirectory (cvmfs)

if (INSTALL_MOUNT_SCRIPTS)
	add_subdirectory (mount)
endif (INSTALL_MOUNT_SCRIPTS)

#
# compile the unit tests
#
if (BUILD_UNITTESTS)
	enable_testing ()
	add_subdirectory (test/unittests)
endif (BUILD_UNITTESTS)

#
# Documentation
#
install (
		FILES                   README AUTHORS ChangeLog COPYING
		DESTINATION             share/doc/cvmfs-${CernVM-FS_VERSION_STRING}
		PERMISSIONS             OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
        )

#
# configure the packaging stuff
#
include (CPackLists.txt)
