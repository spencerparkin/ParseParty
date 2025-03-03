# CMakeLists.txt for ParseTool.

set(PARSE_TOOL_SOURCES
    Source/Application.cpp
    Source/Application.h
    Source/Frame.cpp
    Source/Frame.h
)

source_group("Sources" TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PARSE_TOOL_SOURCES})

add_executable(ParseTool WIN32 ${PARSE_TOOL_SOURCES})
target_compile_definitions(ParseTool PUBLIC __WXMSW__ _UNICODE)

target_link_libraries(ParseTool PRIVATE
    ParseParty
)

target_compile_definitions(ParseTool PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _USE_MATH_DEFINES
)

# Change this to wherever you have wxWidgets installed and built.
set(WX_WIDGETS_ROOT "C:/wxWidgets")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_link_directories(ParseTool PRIVATE
        "${WX_WIDGETS_ROOT}/lib/vc_x64_lib"
    )
else()
    target_link_directories(ParseTool PRIVATE
        "${WX_WIDGETS_ROOT}/lib/vc_lib"
    )
endif()

target_include_directories(ParseTool PRIVATE
    "${WX_WIDGETS_ROOT}/include"
    "${WX_WIDGETS_ROOT}/include/msvc"
)

target_include_directories(ParseTool PUBLIC
    "${PROJECT_SOURCE_DIR}/ParseLibrary/Source"
)