cmake_minimum_required(VERSION 3.0)
project(popen-experiments VERSION 0.1.0)

set (ROOT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/")
include_directories(
  "${CMAKE_CURRENT_SOURCE_DIR}/include/"
)
#Includes
file(GLOB INCLUDE_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/include/octopipes/*.h"
)

#Library
file(GLOB POPEN_SRC
  "${ROOT_SOURCE_DIR}/*.c"
)

set(CXX_FLAGS "-g -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -pthread")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -pthread")
set(CMAKE_C_STANDARD 11)
set(CMAKE_BUILD_TYPE RELEASE)

#Build clients
add_executable(popen-exp ${POPEN_SRC})
target_link_libraries(popen-exp -lpthread)

#Install rules
install(TARGETS popen-exp CONFIGURATIONS Release RUNTIME DESTINATION bin)
