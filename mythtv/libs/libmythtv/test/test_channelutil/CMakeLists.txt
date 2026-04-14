#
# Copyright (C) 2026 David Hampton
#
# See the file LICENSE_FSF for licensing information.
#

add_executable(test_channelutil test_channelutil.cpp test_channelutil.h)

target_include_directories(test_channelutil PRIVATE . ../..)

target_link_libraries(test_channelutil PUBLIC mythtv Qt${QT_VERSION_MAJOR}::Test)

add_test(NAME ChannelUtil COMMAND test_channelutil)
