#
#Copyright(C) thecodeway.com
#

include_directories(
    ../DXUT/Core
    ../DXUT/Optional
	../Effects11/inc
	../libsph
)

add_definitions(-DUSE_DIRECT3D11_2)

add_executable(sph WIN32
    sph_main.cpp
)

target_link_libraries(sph
	libsph
	DXUT_Core
	DXUT_Optional
	d3dcompiler.lib
	usp10.lib
	dxguid.lib
	winmm.lib
	comctl32.lib
)
