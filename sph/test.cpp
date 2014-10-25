#include "stdafx.h"
#include <d3d9.h>
#pragma comment(lib,"d3d9.lib")
//Macro definition
#define SafeRelease(pObject) if(pObject != NULL){pObject->Release();pObject = NULL;}
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE)
//Custom date struct definition
struct CUSTOMVERTEX
{
	FLOAT x,y,z,rhw;
	DWORD color;
};
//Global Variable definition
LPDIRECT3D9 g_pd3d = NULL;
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL;
LPCTSTR g_lpClassName = L"DirectX";
//
//Initialize the D3D object and D3D Device object
//
HRESULT InitD3D(HWND hWnd)
{
	if(NULL == (g_pd3d = Direct3DCreate9(D3D_SDK_VERSION)))
	{
		return E_FAIL;
	}
	//Inquire Current Display Mode Information
	D3DDISPLAYMODE d3ddm;
	if(FAILED(g_pd3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,&d3ddm)))
	{
		return E_FAIL;
	}
	//Set Parameters will be sent to LPDIRECT3D9::CreateDevice
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp,sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = d3ddm.Format;
	//Create Device
	if(FAILED(g_pd3d->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,hWnd,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&d3dpp,&g_pd3dDevice)))
	{
		return E_FAIL;
	}
	return S_OK;
}
//
//Initialize the Vertices Buffer
//
HRESULT InitVB()
{
	//The data of vertices
	CUSTOMVERTEX vertices[] =
	{
		{150.0f,350.0f,0.5f,1.0f,D3DCOLOR_XRGB(255,0,0)}, //point 1
		{250.0f,150.0f,0.5f,1.0f,D3DCOLOR_XRGB(0,255,0)}, //point 2
		{350.0f,350.0f,0.5f,1.0f,D3DCOLOR_XRGB(0,0,255)}, //point 3
		{450.0f,150.0f,0.5f,1.0f,D3DCOLOR_XRGB(255,0,0)}, //point 4
		{550.0f,350.0f,0.5f,1.0f,D3DCOLOR_XRGB(0,255,0)}, //point 5
		{650.0f,150.0f,0.5f,1.0f,D3DCOLOR_XRGB(0,0,255)}, //point 6
	};
	//Create the Vertex Buffer in memory
	if(FAILED(g_pd3dDevice->CreateVertexBuffer(sizeof(vertices),0,D3DFVF_CUSTOMVERTEX,D3DPOOL_DEFAULT,&g_pVB,NULL)))
	{
		return E_FAIL;
	}
	//Copy the data of vertices to our Vertices Buffer
	VOID* pVertices;
	if(FAILED(g_pVB->Lock(0,sizeof(vertices),(void**)&pVertices,0)))
	{
		return E_FAIL;
	}
	memcpy(pVertices,vertices,sizeof(vertices));
	g_pVB->Unlock();
	return S_OK;
}
//
//Release the Resource
//
VOID Cleanup()
{
	SafeRelease(g_pVB);
	SafeRelease(g_pd3d);
	SafeRelease(g_pd3dDevice);
}
//
//Render Scene,Main Drawing Function
//
VOID Render()
{
	if(NULL == g_pd3dDevice)
	{
		return;
	}
	//Set Back Buffer
	g_pd3dDevice->Clear(0,NULL,D3DCLEAR_TARGET,D3DCOLOR_XRGB(0,0,0),1.0f,0);
	//Begin Drawing the Scene
	g_pd3dDevice->BeginScene();
	g_pd3dDevice->SetStreamSource(0,g_pVB,0,sizeof(CUSTOMVERTEX));
	g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
	//Comment three lines of the following four lines,and compile again,it will draw different graphics
	g_pd3dDevice->DrawPrimitive(D3DPT_POINTLIST,0,6); //point list
	//g_pd3dDevice->DrawPrimitive(D3DPT_LINELIST,0,3); //line list
	//g_pd3dDevice->DrawPrimitive(D3DPT_LINESTRIP,0,5); //line strip
	//g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,0,4); //triangle strip
	g_pd3dDevice->EndScene();
	//Present the Drawing Code
	g_pd3dDevice->Present(NULL,NULL,NULL,NULL);
}
//
//Main Window Function
//
LRESULT CALLBACK MainWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_PAINT:
		Render();
		ValidateRect(hWnd,NULL);
		return 0;
	case WM_KEYUP:
		switch(wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hWnd);
			break;
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}
//
//Application Entry Point
//
INT WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
	MSG msg;
	WNDCLASSEX wcex;
	//Register a Window ClassEx
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0u;
	wcex.lpfnWndProc = MainWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL,IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL,IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = g_lpClassName;
	wcex.hIconSm = LoadIcon(NULL,IDI_APPLICATION);
	RegisterClassEx(&wcex);
	//Create a Window
	HWND hWnd = CreateWindow(
		wcex.lpszClassName, //class name
		L"DirectX Window", //window name
		WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX, //style
		112, //x
		86, //y
		800, //width
		600, //height
		NULL, //parent
		NULL, //menu
		wcex.hInstance, //current hInstance
		NULL //param
		);
	//Initialize Direct3D
	if(SUCCEEDED(InitD3D(hWnd)))
	{
		if(SUCCEEDED(InitVB()))
		{
			//Show the Window
			ShowWindow(hWnd,nCmdShow);
			UpdateWindow(hWnd);
			//Go to Message Loop
			while (GetMessage(&msg,NULL,0,0)) 
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	//Release the Resource
	Cleanup();
	UnregisterClass(wcex.lpszClassName,wcex.hInstance);
	return 0;
}
