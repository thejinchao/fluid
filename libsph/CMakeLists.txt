#
#Copyright(C) thecodeway.com
#

set(LIBSPH_SOURCE_CPP
    sph_fluid_system.cpp  
    sph_grid_container.cpp
    sph_neighbor_table.cpp
    sph_point_buffer.cpp  
)

set(LIBSPH_SOURCE_H
    sph_fluid_system.h    
    sph_grid_container.h  
    sph_interface.h       
    sph_math.h            
    sph_neighbor_table.h  
    sph_point_buffer.h    
    sph_stdafx.h          
)

set(LIBSPH_SOURCE_CPP_PRECOMPILED 
	sph_stdafx.cpp
)

if(MSVC)
	foreach(src_file ${LIBSPH_SOURCE_CPP})
		set_source_files_properties(
			${src_file}
			PROPERTIES
			COMPILE_FLAGS "/Yusph_stdafx.h"
		)
	endforeach()
	
	set_source_files_properties(${LIBSPH_SOURCE_CPP_PRECOMPILED}
		PROPERTIES
		COMPILE_FLAGS "/Ycsph_stdafx.h"
	)
endif()

add_library(libsph 
	${LIBSPH_SOURCE_CPP_PRECOMPILED}
	${LIBSPH_SOURCE_CPP}
	${LIBSPH_SOURCE_H}         
)

