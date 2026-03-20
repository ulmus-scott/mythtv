#
# Copyright (C) 2026 David Hampton
#
# See the file LICENSE_FSF for licensing information.
#

add_executable(test_huffman test_huffman.cpp test_huffman.h)

target_include_directories(test_huffman PRIVATE . ../..)

target_link_libraries(test_huffman PUBLIC mythtv Qt${QT_VERSION_MAJOR}::Test)

add_test(NAME Huffman COMMAND test_huffman)
