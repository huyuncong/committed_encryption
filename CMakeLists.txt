cmake_minimum_required (VERSION 2.8.11)
project (committed-encryption)
set(NAME "committed-encryption")

if(POLICY CMP0042)
  cmake_policy(SET CMP0042 NEW) # use rpath on macOS
endif()

if(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

include_directories(${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} cmake/)
set(CMAKE_C_FLAGS "-pthread -Wall -Wno-unused-result -march=native -O3 -maes -mrdseed -mavx2")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++0x")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
OPTION(USE_RANDOM_DEVICE "Option description" OFF)
IF(USE_RANDOM_DEVICE)
     ADD_DEFINITIONS(-DEMP_USE_RANDOM_DEVICE)
ENDIF(USE_RANDOM_DEVICE)

find_package(gcrypt REQUIRED)
find_package(emp-tool REQUIRED)
include_directories(${EMP-TOOL_INCLUDE_DIRS})

add_executable(keygen "keygen.cpp" ${sources})
target_link_libraries(keygen ${EMP-TOOL_LIBRARIES}) 

add_executable(encrypt "encrypt.cpp" ${sources})
target_link_libraries(encrypt ${EMP-TOOL_LIBRARIES} ${GCRYPT_LIBRARIES})

add_executable(encrypt_malicious "encrypt_malicious.cpp" ${sources})
target_link_libraries(encrypt_malicious ${EMP-TOOL_LIBRARIES} ${GCRYPT_LIBRARIES})

add_executable(decrypt "decrypt.cpp" ${sources})
target_link_libraries(decrypt ${EMP-TOOL_LIBRARIES} ${GCRYPT_LIBRARIES})

add_executable(check "check.cpp" ${sources})
target_link_libraries(check ${EMP-TOOL_LIBRARIES} ${GCRYPT_LIBRARIES})
