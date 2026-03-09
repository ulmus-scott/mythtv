# SPDX-License-Identifier: GPL-2.0-or-later


#
# Maybe build libmythdvdnav.
#
function(find_or_build_mythdvdnav)
  if(LIBS_USE_INSTALLED)
    pkg_check_modules(MythDvdNav "mythdvdnav" IMPORTED_TARGET)
    if (TARGET PkgConfig::MythDvdNav)
      set(ProjectDepends
          ${ProjectDepends} PkgConfig::MythDvdNav
          PARENT_SCOPE)
      return()
    endif()
  endif()

  if(ENABLE_STRICT_BUILD_ORDER)
    get_property(
      after_libs
      TARGET embedded_libs
      PROPERTY MANUALLY_ADDED_DEPENDENCIES)
  endif()

  message(STATUS "Will build libmythdvdnav (embedded)")
  if(LIBS_INSTALL_MYTHDVDNAV)
    set(CMDLINE_ARGS_MYTHDVDNAV ${CMDLINE_ARGS_LIBS})
  else()
    set(CMDLINE_ARGS_MYTHDVDNAV ${CMDLINE_ARGS})
  endif()
  ExternalProject_Add(
    mythdvdnav
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mythtv/external/libmythdvdnav
    DOWNLOAD_COMMAND
      ${CMAKE_COMMAND} -E echo "Using libmythdvdnav in <SOURCE_DIR>"
    CMAKE_ARGS --no-warn-unused-cli ${CMDLINE_ARGS_MYTHDVDNAV} ${PLATFORM_ARGS}
    CMAKE_CACHE_ARGS
      -DCMAKE_FIND_ROOT_PATH:STRING=${CMAKE_FIND_ROOT_PATH}
      -DCMAKE_JOB_POOL_COMPILE:STRING=compile
      -DCMAKE_JOB_POOL_LINK:STRING=link
      -DCMAKE_JOB_POOLS:STRING=${CMAKE_JOB_POOLS}
      -DPKG_CONFIG_LIBDIR:PATH=${PKG_CONFIG_LIBDIR}
      -DPKG_CONFIG_PATH:PATH=${PKG_CONFIG_PATH}
    BUILD_ALWAYS ${LIBS_ALWAYS_REBUILD}
    USES_TERMINAL_BUILD TRUE
    DEPENDS external_libs ${after_libs})

  add_dependencies(embedded_libs mythdvdnav)
  if(LIBS_INSTALL_MYTHDVDNAV)
    set_target_properties(embedded_libs PROPERTIES REQUIRES_RW TRUE)
  endif()
endfunction()

find_or_build_mythdvdnav()
