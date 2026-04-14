#
# Copyright (C) 2026 David Hampton
#
# See the file LICENSE_FSF for licensing information.
#

add_executable(test_livetvchain test_livetvchain.cpp test_livetvchain.h)

target_include_directories(test_livetvchain PRIVATE . ../..)

target_link_libraries(test_livetvchain PUBLIC mythtv Qt${QT_VERSION_MAJOR}::Test)

add_test(NAME LiveTvChain COMMAND test_livetvchain)
