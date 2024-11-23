# CMakeLists.txt for ParseLibrary.

set(PARSE_LIBRARY_SOURCES
    Source/Common.h
    Source/FormatString.cpp
    Source/FormatString.h
    Source/GeneralParseAlgorithm.cpp
    Source/GeneralParseAlgorithm.h
    Source/Grammar.cpp
    Source/Grammar.h
    Source/JsonValue.cpp
    Source/JsonValue.h
    Source/Lexer.cpp
    Source/Lexer.h
    Source/LookAheadParseAlgorithm.cpp
    Source/LookAheadParseAlgorithm.h
    Source/Parser.cpp
    Source/Parser.h
    Source/QuickParseAlgorithm.cpp
    Source/QuickParseAlgorithm.h
    Source/SlowParseAlgorithm.cpp
    Source/SlowParseAlgorithm.h
    Source/StringTransformer.cpp
    Source/StringTransformer.h
)

source_group("Sources" TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PARSE_LIBRARY_SOURCES})

add_library(ParseParty STATIC ${PARSE_LIBRARY_SOURCES})