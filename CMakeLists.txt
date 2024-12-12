cmake_minimum_required(VERSION 3.10)

project(lem)

include(CTest)
enable_testing()

set(BUILD_SHARED_LIBS ON)
set(CMAKE_CXX_STANDARD 11)

if(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHs")
	add_definitions(-D_WIN32_WINNT=0x0601)
endif()

file(GLOB_RECURSE src owca/*.cpp)
file(GLOB_RECURSE hdr owca/*.h)
add_library(owca_objects OBJECT ${src} ${hdr})
#target_compile_definitions(owca_objects PUBLIC RCDEBUG_IN_VS=1)
#target_compile_definitions(owca_objects PUBLIC RCDEBUG=1)
#target_compile_definitions(owca_objects PUBLIC RCDEBUG_EXECUTION=1)

add_library(owca_shared SHARED ${src} ${hdr})
target_include_directories(owca_shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(owca_static STATIC ${src} ${hdr})
target_include_directories(owca_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(test1 tests/test1.cpp)
target_link_libraries(test1 PUBLIC owca_static)

add_executable(test2 tests/test2.cpp)
target_link_libraries(test2 PUBLIC owca_static)

add_executable(test_files tests/test_files.cpp)
target_link_libraries(test_files PUBLIC owca_static)

add_test(NAME test_1 COMMAND test1)
add_test(NAME test_2 COMMAND test2)

file(GLOB_RECURSE test_cases tests/cases/*)

foreach(test_case ${test_cases})
	string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" test_case1 "${test_case}")
	string(REPLACE "/" "_" test_case2 "${test_case1}")
	string(REPLACE "\\" "_" test_case3 "${test_case2}")
    add_test(NAME "${test_case3}" COMMAND test_files ${test_case})
    math(EXPR i "${i} + 1")
endforeach()
