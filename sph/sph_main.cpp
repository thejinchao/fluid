#include "stdafx.h"
#include "resource.h"

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
ID3DXFont*                      g_pFont = NULL;         // Font for drawing text
ID3DXSprite*                    g_pTextSprite = NULL;   // Sprite for batching draw text calls
CModelViewerCamera              g_Camera;               // A model viewing camera
bool                            g_bShowHelp = true;     // If true, it renders the UI control text
CDXUTDialogResourceManager      g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg                 g_SettingsDlg;          // Device settings dialog
CDXUTDialog                     g_HUD;                  // dialog for standard controls

ID3DXMesh*						g_pWallBox=0;
ID3DXEffect*					g_pEffect=0;				//!< DX Shader
IDirect3DVertexBuffer9*			m_pointsVertexBuffer;		//!< Vertex buffer for sph points

SPH::System*					g_pSPHSystem=0;

D3DMATERIAL9					g_materialPoint;

//--------------------------------------------------------------------------------------
// Vertex struct
//--------------------------------------------------------------------------------------
struct PointVertex
{
	enum { VERTEX_FVF = D3DFVF_XYZ|D3DFVF_DIFFUSE};
	float x, y, z;
	DWORD color;
};

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           3
#define IDC_CHANGEDEVICE        4
#define IDC_RESET		        5

#define PARTICLE_COUNTS			4096*2

//--------------------------------------------------------------------------------------
void resetSPHSystem(void)
{
	SPH::float_3 wall_min = {-25, 00, -25};
	SPH::float_3 wall_max = {25, 50, 25};
	SPH::float_3 fluid_min = {-15, 5, -15};
	SPH::float_3 fluid_max = {15, 35, 15};
	SPH::float_3 gravity = {0.0, -9.8f, 0};
	g_pSPHSystem->init(PARTICLE_COUNTS, &wall_min, &wall_max, &fluid_min, &fluid_max, &gravity);
}

//--------------------------------------------------------------------------------------
HRESULT CALLBACK onCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D9CreateDevice( pd3dDevice ) );
    V_RETURN( g_SettingsDlg.OnD3D9CreateDevice( pd3dDevice ) );
    // Initialize the font
    V_RETURN( D3DXCreateFont( pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
                              OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                              L"Arial", &g_pFont ) );

	// Setup the camera's view parameters
    g_Camera.SetViewQuat( D3DXQUATERNION( -0.275f, 0.3f, 0.0f, 0.7f ) );

	//create effect
	LPD3DXBUFFER errorBuf;
	V_RETURN(D3DXCreateEffectFromResourceA(pd3dDevice, GetModuleHandle(0), MAKEINTRESOURCEA(IDR_FX_SPH), 0, 0, D3DXFX_NOT_CLONEABLE|D3DXSHADER_DEBUG, NULL, &g_pEffect, &errorBuf));

	resetSPHSystem();

	return S_OK;
}

//--------------------------------------------------------------------------------------
HRESULT CALLBACK onResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* userContext)
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D9ResetDevice() );
    V_RETURN( g_SettingsDlg.OnD3D9ResetDevice() );

    if( g_pFont )
        V_RETURN( g_pFont->OnResetDevice() );

    // Create a sprite to help batch calls when drawing many lines of text
    V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pTextSprite ) );

    // Setup the camera's projection parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams( D3DX_PI / 4, fAspectRatio, 0.1f, 1000.0f );
    g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );

    D3DXVECTOR3 vecEye( 50.0f, 50.0f, 50.0f );
    D3DXVECTOR3 vecAt (0.0f, 0.0f, 0.0f );
    g_Camera.SetViewParams( &vecEye, &vecAt );

    g_HUD.SetLocation( pBackBufferSurfaceDesc->Width - 170, 0 );
    g_HUD.SetSize( 170, 170 );

	V_RETURN(::D3DXCreateSphere(pd3dDevice, 0.5, 8, 8, &g_pWallBox, 0));

	//Reset effect
	if(g_pEffect!=0)
	{
		V_RETURN(g_pEffect->OnResetDevice());
	}

	//reset canvas vertex buf
	if(m_pointsVertexBuffer==0)
	{
		V_RETURN(pd3dDevice->CreateVertexBuffer(sizeof(PointVertex)*PARTICLE_COUNTS, 0, PointVertex::VERTEX_FVF, D3DPOOL_DEFAULT, &m_pointsVertexBuffer, NULL));
	}

	return S_OK;
}

//--------------------------------------------------------------------------------------
void CALLBACK onFrameMove(double fTime, float fElapsedTime, void* pUserContext)
{
	g_pSPHSystem->tick();

	PointVertex* pVertices;
	m_pointsVertexBuffer->Lock(0, 0, (void**)&pVertices, 0);

	const SPH::float_3* p = g_pSPHSystem->getPointBuf();
	unsigned int stride = g_pSPHSystem->getPointStride();

	for(unsigned int n = 0; n<g_pSPHSystem->getPointCounts(); n++)
	{
		pVertices->x = p->x;
		pVertices->y = p->y;
		pVertices->z = p->z;

		pVertices->color = D3DCOLOR_XRGB(255, 255, 255);

		p = (const SPH::float_3*)(((const char*)p) + stride);
		pVertices++;
	}
	m_pointsVertexBuffer->Unlock();

	// Update the camera's position based on user input 
	g_Camera.FrameMove( fElapsedTime );
}

//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText()
{
    // The helper object simply helps keep track of text position, and color
    // and then it calls pFont->DrawText( g_pSprite, strMsg, -1, &rc, DT_NOCLIP, g_clr );
    // If NULL is passed in as the sprite object, then it will work however the 
    // pFont->DrawText() will not be batched together.  Batching calls will improves performance.
    CDXUTTextHelper txtHelper( g_pFont, g_pTextSprite, 15 );

    // Output statistics
    txtHelper.Begin();
    txtHelper.SetInsertionPos( 5, 5 );
    txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
    txtHelper.DrawTextLine( DXUTGetFrameStats( true/*DXUTIsVsyncEnabled()*/ ) );
    txtHelper.DrawTextLine( DXUTGetDeviceStats() );

    txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );

    // Draw help
    if( g_bShowHelp )
    {
        const D3DSURFACE_DESC* pd3dsdBackBuffer = DXUTGetD3D9BackBufferSurfaceDesc();
        txtHelper.SetInsertionPos( 10, pd3dsdBackBuffer->Height - 15 * 6 );
        txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 0.75f, 0.0f, 1.0f ) );
        txtHelper.DrawTextLine( L"Controls (F1 to hide):" );

        txtHelper.SetInsertionPos( 40, pd3dsdBackBuffer->Height - 15 * 5 );
        txtHelper.DrawTextLine( L"Rotate camera: Right mouse button\n"
                                L"Zoom camera: Mouse wheel scroll\n"
                                L"Hide help: F1" );
    }
    else
    {
        txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
        txtHelper.DrawTextLine( L"Press F1 for help" );
    }

    txtHelper.End();
}

//--------------------------------------------------------------------------------------
void CALLBACK onFrameRender(IDirect3DDevice9* pd3dDevice, double time, float fElapsedTime, void* pUserContext)
{
    D3DXMATRIXA16 mProj;
    D3DXMATRIXA16 mView;
    D3DXMATRIXA16 mViewProjection;

    // If the settings dialog is being shown, then
    // render it instead of rendering the app's scene
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.OnRender( fElapsedTime );
        return;
    }

	HRESULT hr;
    // Clear the render target and the zbuffer 
    V( pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DXCOLOR(0.1f, 0.3f, 0.6f, 0.0f), 1.0f, 0 ) );

	float pointSize = 2.0f;
	pd3dDevice->SetRenderState(D3DRS_POINTSIZE, *((DWORD*)&pointSize));

	if (SUCCEEDED(pd3dDevice->BeginScene())) 
	{
		g_pEffect->SetTechnique( "Maintech" );

		//set matrix
		D3DXMATRIX g_matTrans;
		D3DXMatrixMultiply(&g_matTrans, g_Camera.GetViewMatrix(), g_Camera.GetProjMatrix());
        g_pEffect->SetMatrix("g_mWorldViewProjection", &g_matTrans );

	    UINT iPass, cPasses;
        g_pEffect->Begin( &cPasses, 0 );

        for( iPass = 0; iPass < cPasses; iPass++ )
        {
            g_pEffect->BeginPass( iPass );

			pd3dDevice->SetStreamSource(0, m_pointsVertexBuffer, 0, sizeof(PointVertex));
			pd3dDevice->SetFVF(PointVertex::VERTEX_FVF);
			pd3dDevice->DrawPrimitive(D3DPT_POINTLIST, 0, g_pSPHSystem->getPointCounts());
            g_pEffect->EndPass();
        }
        g_pEffect->End();

        RenderText();

        V( g_HUD.OnRender( fElapsedTime ) );
		V(pd3dDevice->EndScene());
	}

}

//--------------------------------------------------------------------------------------
void CALLBACK onLostDevice(void* userContext)
{
    g_DialogResourceManager.OnD3D9LostDevice();
    g_SettingsDlg.OnD3D9LostDevice();
    if( g_pFont )
        g_pFont->OnLostDevice();
    SAFE_RELEASE( g_pTextSprite );


	SAFE_RELEASE(g_pWallBox);
	//lost effect
	if(g_pEffect)
	{
        g_pEffect->OnLostDevice();
	}
	SAFE_RELEASE(m_pointsVertexBuffer);
}

//--------------------------------------------------------------------------------------
void CALLBACK onDestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9DestroyDevice();
    g_SettingsDlg.OnD3D9DestroyDevice();
    SAFE_RELEASE( g_pFont );

	SAFE_RELEASE(g_pWallBox);
	SAFE_RELEASE(g_pEffect);
	SAFE_RELEASE(m_pointsVertexBuffer);
}

//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN:
            DXUTToggleFullScreen(); break;
        case IDC_TOGGLEREF:
            DXUTToggleREF(); break;
        case IDC_CHANGEDEVICE:
            g_SettingsDlg.SetActive( !g_SettingsDlg.IsActive() ); break;
			break;
		case IDC_RESET:
			resetSPHSystem();
			break;
    }
}

//--------------------------------------------------------------------------------------
// Before handling window messages, DXUT passes incoming windows 
// messages to the application through this callback function. If the application sets 
// *pbNoFurtherProcessing to TRUE, then DXUT will not process this message.
//--------------------------------------------------------------------------------------
LRESULT CALLBACK msgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext )
{
    // Always allow dialog resource manager calls to handle global messages
    // so GUI state is updated correctly
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass all remaining windows messages to camera so it can respond to user input
    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

    return 0;
}


//--------------------------------------------------------------------------------------
// As a convenience, DXUT inspects the incoming windows messages for
// keystroke messages and decodes the message parameters to pass relevant keyboard
// messages to the application.  The framework does not remove the underlying keystroke 
// messages, which are still passed to the application's MsgProc callback.
//--------------------------------------------------------------------------------------
void CALLBACK keyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
    if( bKeyDown )
    {
        switch( nChar )
        {
            case VK_F1:
                g_bShowHelp = !g_bShowHelp; break;
        }
    }
}


//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
    // Initialize dialogs
    g_SettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );

    g_HUD.SetCallback( OnGUIEvent ); int iY = 10;
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, iY, 125, 22 );
    g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 35, iY += 24, 125, 22 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 35, iY += 24, 125, 22, VK_F2 );
    g_HUD.AddButton( IDC_RESET, L"Reset", 35, iY += 24, 125, 22 );

	//create sph system
	g_pSPHSystem = getSPHSystem();
}

//--------------------------------------------------------------------------------------
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR lpCmdLine, int nCmdShow)
{
	srand(::GetTickCount());

    DXUTSetCallbackD3D9DeviceCreated(onCreateDevice);
	DXUTSetCallbackD3D9DeviceReset(onResetDevice);
	DXUTSetCallbackD3D9DeviceLost(onLostDevice);
	DXUTSetCallbackD3D9FrameRender(onFrameRender);
    DXUTSetCallbackD3D9DeviceDestroyed(onDestroyDevice);
	DXUTSetCallbackFrameMove(onFrameMove);
    DXUTSetCallbackMsgProc(msgProc);
    DXUTSetCallbackKeyboard(keyboardProc);
    DXUTSetCursorSettings(true, true);

    InitApp();

	/* Parse  command line, handle  default hotkeys, and show messages. */
	DXUTInit(false, true, _T("-forcevsync:0")); //turn v-sync off!
    DXUTSetHotkeyHandling( true, true, true );  // handle the defaul hotkeys
	DXUTCreateWindow(L"SPH");
	DXUTCreateDevice(true, 800, 600);
	
	DXUTMainLoop();
	return DXUTGetExitCode();
}
