# File       CMakeLists.txt
# Author     David Hellmans (david.hellmanns (at) rwth-aachen.de)
# Brief      CMake file of FHEBLOOM approach.
# 
# Copyright  BLOOM: Bloom filter based outsourced oblivious matchings
#            Copyright (C) 2017 Communication and Distributed Systems (COMSYS), RWTH Aachen
#            
#            This program is free software: you can redistribute it and/or modify
#            it under the terms of the GNU Affero General Public License as published
#            by the Free Software Foundation, either version 3 of the License, or
#            (at your option) any later version.
#            
#            This program is distributed in the hope that it will be useful,
#            but WITHOUT ANY WARRANTY; without even the implied warranty of
#            MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#            GNU Affero General Public License for more details.
#            You should have received a copy of the GNU Affero General Public License
#            along with this program. If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 2.8)
project(FHEBLOOM)
include(ExternalProject)
ExternalProject_Add(
        NTL

        PREFIX ${CMAKE_BINARY_DIR}/libs

        URL http://www.shoup.net/ntl/ntl-9.10.0.tar.gz
        SOURCE_DIR libs/NTL
        DOWNLOAD_DIR libs/tmp
        CONFIGURE_COMMAND cd ${CMAKE_BINARY_DIR}/libs/NTL/src/ && ./configure NTL_THREADS=on NTL_THREAD_BOOST=on NTL_GF2X_LIB=on NTL_GMP_LIP=on
        BUILD_COMMAND cd ${CMAKE_BINARY_DIR}/libs/NTL/src/ && make all -j4
        INSTALL_COMMAND cd ${CMAKE_BINARY_DIR}/libs/NTL/src/ && sudo make install
)

ExternalProject_Add(
        HElib
        DEPENDS NTL

        PREFIX ${CMAKE_BINARY_DIR}/libs
        GIT_REPOSITORY "https://github.com/shaih/HElib.git"
        GIT_TAG master

        SOURCE_DIR libs/HElib/
        CONFIGURE_COMMAND cp ${CMAKE_SOURCE_DIR}/libs_conf/HElib/Makefile ${CMAKE_BINARY_DIR}/libs/HElib/src
        BUILD_COMMAND cd ${CMAKE_BINARY_DIR}/libs/HElib/src && make all -j4
        INSTALL_COMMAND cd ${CMAKE_BINARY_DIR}/libs/HElib/src
        TEST_COMMAND cd ${CMAKE_BINARY_DIR}/libs/HElib/src && make test
)

include_directories("${CMAKE_BINARY_DIR}/libs/HElib/src")
link_directories()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -Wshadow -Wpedantic -pthread -fopenmp")

set(SOURCE_GENERAL src/fhebloom_config.cpp src/fhebloom_config.h src/commandline.h src/commandline.cpp)
set(SOURCE_CLIENT src/fhebloom_client.cpp)
set(SOURCE_SERVER src/fhebloom_server.cpp)

add_executable(fhebloom_client ${SOURCE_CLIENT} ${SOURCE_GENERAL})
add_dependencies(fhebloom_client HElib)
add_executable(fhebloom_server ${SOURCE_SERVER} ${SOURCE_GENERAL})
add_dependencies(fhebloom_server HElib)

target_link_libraries(fhebloom_client ${CMAKE_BINARY_DIR}/libs/HElib/src/fhe.a)
target_link_libraries(fhebloom_client boost_program_options)
target_link_libraries(fhebloom_client boost_system)
target_link_libraries(fhebloom_client boost_filesystem)
target_link_libraries(fhebloom_client boost_regex)
target_link_libraries(fhebloom_client ${CMAKE_BINARY_DIR}/libs/NTL/src/ntl.a)
target_link_libraries(fhebloom_client gmp)
target_link_libraries(fhebloom_client gf2x)

target_link_libraries(fhebloom_server ${CMAKE_BINARY_DIR}/libs/HElib/src/fhe.a)
target_link_libraries(fhebloom_server boost_program_options)
target_link_libraries(fhebloom_server boost_system)
target_link_libraries(fhebloom_server boost_filesystem)
target_link_libraries(fhebloom_server boost_regex)
target_link_libraries(fhebloom_server ${CMAKE_BINARY_DIR}/libs/NTL/src/ntl.a)
target_link_libraries(fhebloom_server gmp)
target_link_libraries(fhebloom_server gf2x)
