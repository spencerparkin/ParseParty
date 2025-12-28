# CMakeLists.txt for ParseParty project.

cmake_minimum_required(VERSION 3.27)

project(ParseParty)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(ParseLibrary)

if(ENABLE_PARSE_TOOL)
	add_subdirectory(ParseTool)
endif()
