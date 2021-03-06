# - Set Autotools variables
#
# Sets various needed Autotools variables until LibGCal moves to the CMake
# buildsystem exclusively.
#
# Copyright (c) 2009 Mike Arthur <mike@mikearthur.co.uk>

set(prefix "${CMAKE_INSTALL_PREFIX}")
set(exec_prefix "${prefix}")
set(libdir "${exec_prefix}/${GCAL_LIBRARY_INSTALL_DIR}")
set(includedir "${prefix}/${GCAL_INCLUDE_INSTALL_DIR}")
set(VERSION "${GCAL_VERSION}")
set(PACKAGE "${CMAKE_PROJECT_NAME}")
set(srcdir "${GCAL_SOURCE_DIR}")
set(csourcedir "${GCAL_SOURCE_DIR}")
set(headerdir "${GCAL_HEADER_DIR}")
