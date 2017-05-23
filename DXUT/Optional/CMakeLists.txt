#
#Copyright(C) thecodeway.com
#

include_directories(
	../Core
)

add_library(DXUT_Optional
    DXUTcamera.cpp     
    DXUTcamera.h       
    DXUTgui.cpp        
    DXUTgui.h          
    DXUTguiIME.cpp     
    DXUTguiIME.h       
    DXUTLockFreePipe.h 
    DXUTres.cpp        
    DXUTres.h          
    DXUTsettingsdlg.cpp
    DXUTsettingsdlg.h  
    ImeUi.cpp          
    ImeUi.h            
    SDKmesh.cpp        
    SDKmesh.h          
    SDKmisc.cpp        
    SDKmisc.h          
)
