cmake_minimum_required(VERSION 3.0)
project(blog)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(pico/cmake/utils.cmake)
include(cmake/FindMySQL.cmake)

include_directories(.)
include_directories(./src/)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_FLAGS
    "$ENV{CXXFLAGS} -rdynamic -O2 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations"
)
set(CMAKE_C_FLAGS
    "$ENV{CXXFLAGS} -rdynamic -O2 -fPIC -ggdb -std=c11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations"
)

option(TESTS_BUILD "enable tests/ build" OFF)



include_directories("${PROJECT_BINARY_DIR}")


find_package(MySQL REQUIRED)
if(MYSQL_FOUND)
  include_directories(${MYSQL_INCLUDE_DIR})
  link_directories(${MYSQL_LIBRARY_DIR})
endif()

find_package(yaml-cpp REQUIRED)
include_directories(${YAML_INCLUDE_DIR})
link_directories(${YAML_LIBRARY_DIR})

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
include_directories(${JSONCPP_INCLUDE_DIR})


set(servlets
    src/servlets/article_servlet.cc
    src/servlets/category_servlet.cc
    src/servlets/comment_servlet.cc
    src/servlets/login_servlet.cc
    src/servlets/message_servlet.cc
    src/servlets/project_servlet.cc
    src/servlets/root_servlet.cc
    src/servlets/tag_servlet.cc
)

set(filters
    src/filters/params_filter.cc
    src/filters/session_filter.cc
)

set(LIB_SRC

    src/oss/oss_client.cc

    src/tables.hpp

    src/util.cc
    ${servlets}
    ${filters}
    )

add_library(blog SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(blog)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

include_directories("${PROJECT_SOURCE_DIR}/pico")
add_subdirectory(pico)
set(EXTRA_LIBS ${EXTRA_LIBS} pico blog)

set(LIBS 
    ${LIBS}
    pico
    blog
    ${JSONCPP_LIBRARIES}
    ${YAML_CPP_LIBRARIES}
    ${MYSQL_LIBRARIES}
    pthread
    mbedcrypto
    mbedtls
    )

if(TESTS_BUILD)

add_executable(test_submodule tests/test_submodule.cc)
add_dependencies(test_submodule ${EXTRA_LIBS})
force_redefine_file_macro_for_sources(test_submodule)
target_link_libraries(test_submodule ${LIBS})

add_executable(test_oss tests/test_oss.cc)
add_dependencies(test_oss ${EXTRA_LIBS})
force_redefine_file_macro_for_sources(test_oss)
target_link_libraries(test_oss ${LIBS})

add_executable(test_hmac_sha1 tests/test_hmac_sha1.cc)
add_dependencies(test_hmac_sha1 ${EXTRA_LIBS})
force_redefine_file_macro_for_sources(test_hmac_sha1)
target_link_libraries(test_hmac_sha1 ${LIBS})

add_executable(test_server tests/test_server.cc)
add_dependencies(test_server ${EXTRA_LIBS})
force_redefine_file_macro_for_sources(test_server)
target_link_libraries(test_server ${LIBS})

add_executable(test_app tests/test_app.cc)
add_dependencies(test_app ${EXTRA_LIBS})
force_redefine_file_macro_for_sources(test_app)
target_link_libraries(test_app ${LIBS})

add_executable(test_log tests/test_log.cc)
add_dependencies(test_log ${EXTRA_LIBS})
force_redefine_file_macro_for_sources(test_log)
target_link_libraries(test_log ${LIBS})

add_executable(test_mapper tests/test_mapper.cc)
add_dependencies(test_mapper ${EXTRA_LIBS})
force_redefine_file_macro_for_sources(test_mapper)
target_link_libraries(test_mapper ${LIBS})

endif()

add_executable(app pico/pico/main.cc)
add_dependencies(app ${EXTRA_LIBS})
force_redefine_file_macro_for_sources(app)
target_link_libraries(app ${LIBS})