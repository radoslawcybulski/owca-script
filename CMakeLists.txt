cmake_minimum_required(VERSION 3.10)

if(POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW) 
endif()

project(owca-script)

include(CTest)
enable_testing()

if(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHs")
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fsanitize=address")
	add_definitions(-D_WIN32_WINNT=0x0601)
	add_definitions(-D_DISABLE_VECTOR_ANNOTATION=1)
endif()

add_subdirectory(googletest)

set(BUILD_SHARED_LIBS OFF)
set(CMAKE_CXX_STANDARD 23)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)


file(GLOB_RECURSE src owca-script/*.cpp)
file(GLOB_RECURSE hdr owca-script/*.h)

#add_library(owca-script SHARED ${src} ${hdr})
#target_include_directories(owca_shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(owca-script-static STATIC ${src} ${hdr})
target_include_directories(owca-script-static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE src tests/*.cpp)
file(GLOB_RECURSE hdr tests/*.h)

add_executable(owca-script-test ${src} ${hdr})
target_link_libraries(owca-script-test PUBLIC owca-script-static gtest gtest_main)

add_test(NAME owca-script-test COMMAND owca-script-test)
