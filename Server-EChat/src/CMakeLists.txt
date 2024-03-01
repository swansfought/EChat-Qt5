
aux_source_directory(. DIR_SUB_SRCS)

add_library(echatlib STATIC ${DIR_SUB_SRCS}) # STATIC SHARED

# 使用pthread多线程
find_package(Threads REQUIRED)
target_link_libraries (echatlib ${CMAKE_THREAD_LIBS_INIT}) 

# 使用MySQL数据库
set(mysql_lib mysqlclient)
target_link_libraries(echatlib ${mysql_lib}) 

# 使用jsoncpp
set(jsoncpp_lib ${PROJECT_SOURCE_DIR}/lib/libjsoncpp.a)
target_link_libraries(echatlib ${jsoncpp_lib}) 

