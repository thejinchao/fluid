#
#Copyright(C) thecodeway.com
#

cmake_minimum_required (VERSION 2.8)
project(fluid)

add_definitions(-DUNICODE -D_UNICODE)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_subdirectory(libsph)
add_subdirectory(DXUT)
add_subdirectory(sph)
