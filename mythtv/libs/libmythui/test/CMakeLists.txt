#
# Copyright (C) 2022-2023 David Hampton
#
# See the file LICENSE_FSF for licensing information.
#

if(CMAKE_CROSSCOMPILING)
  return()
endif()

file(GLOB test_dirs "test_*")
foreach(dir ${test_dirs})
  add_subdirectory(${dir})
endforeach()
