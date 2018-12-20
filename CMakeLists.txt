
# -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
# 
# 		CNU Library Manager
# 
# 	Information Engineering Institute
# 	Class 2 Guanzhen, Deng Zhengwang
# 
# -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(LibMan)

INCLUDE_DIRECTORIES(
include
include/sqlite3-secure
)

AUX_SOURCE_DIRECTORY(source DIR_SRCS)
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/source)
SET(SRCLIST ${DIR_SRCS})
LINK_LIBRARIES(
normaliz
libeay32
ssleay32
ws2_32
winmm
wldap32
libcurl
)

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCLIST} ${PROJECT_SOURCE_DIR}/include/sqlite3-secure/sqlite3secure.c)
