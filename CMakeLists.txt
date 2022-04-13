cmake_minimum_required(VERSION 3.0)
project(blog)

include(pico/cmake/utils.cmake)

include_directories(.)
include_directories(./src/)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_FLAGS
    "$ENV{CXXFLAGS} -rdynamic -O2 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations"
)
set(CMAKE_C_FLAGS
    "$ENV{CXXFLAGS} -rdynamic -O2 -fPIC -ggdb -std=c11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations"
)

option(TESTS_BUILD "enable tests/ build" ON)

link_directories(/usr/lib/mysql)

include_directories("${PROJECT_BINARY_DIR}")

include_directories(/usr/include/mysql)



set(
    LIB_SRC
    src/ConnectionPool/CommonConnectionPool.cpp
    src/ConnectionPool/Connection.cpp
    src/ConnectionPool/ResultSet.cpp
    src/oss/oss_client.cc
    src/util.cc
    )

add_library(blog SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(blog)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

include_directories("${PROJECT_SOURCE_DIR}/pico")
add_subdirectory(pico)
set(EXTRA_LIBS ${EXTRA_LIBS} pico blog)

set(LIBS ${LIBS} pico blog mysqlclient mbedtls mbedcrypto)

if(TESTS_BUILD)

add_executable(test_submodule tests/test_submodule.cc)
add_dependencies(test_submodule ${EXTRA_LIBS})
force_redefine_file_macro_for_sources(test_submodule)
target_link_libraries(test_submodule ${LIBS})

add_executable(test_cp tests/test_cp.cc)
add_dependencies(test_cp ${EXTRA_LIBS})
force_redefine_file_macro_for_sources(test_cp)
target_link_libraries(test_cp ${LIBS})

add_executable(test_oss tests/test_oss.cc)
add_dependencies(test_oss ${EXTRA_LIBS})
force_redefine_file_macro_for_sources(test_oss)
target_link_libraries(test_oss ${LIBS})

add_executable(test_hmac_sha1 tests/test_hmac_sha1.cc)
add_dependencies(test_hmac_sha1 ${EXTRA_LIBS})
force_redefine_file_macro_for_sources(test_hmac_sha1)
target_link_libraries(test_hmac_sha1 ${LIBS})

endif()