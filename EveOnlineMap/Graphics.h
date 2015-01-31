#pragma once

#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_



#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>

// include the Direct3D Library file
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dx11.lib")
#pragma comment (lib, "d3dx10.lib")
#pragma comment (lib, "DXGI.lib")


struct VideoCardInfo;

class Graphics
{


public:
	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();

	void GetProjectionMatrix(D3DXMATRIX&);
	void GetWorldMatrix(D3DXMATRIX&);
	void GetOrthoMatrix(D3DXMATRIX&);

	void GetVideoCardInfo(char*, int&);

	Graphics();
	Graphics(const Graphics&);

	bool Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen,
		float screenDepth, float screenNear);
	void Shutdown();

	void BeginScene(float red, float green, float blue, float alpha);
	void EndScene();


	//deprecated
	void InitD3D(HWND);
	void RenderFrame();
	void CleanD3D();


private:
	bool m_vsync_enabled;
	int m_videoCardMemory;
	char m_videoCardDescription[128];
	IDXGISwapChain* m_swapChain;
	ID3D11Device* m_device;
	ID3D11DeviceContext* m_deviceContext;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11DepthStencilState* m_depthStencilState;
	ID3D11DepthStencilView* m_depthStencilView;
	ID3D11RasterizerState* m_rasterState;
	D3DXMATRIX m_rojectionMatrix;
	D3DXMATRIX m_worldMatrix;
	D3DXMATRIX m_orthoMatrix;

	int m_screenWidth, m_screenHeight;

	void InitPipeline();
	void InitGraphics();


	bool GetVideoCardInfo(VideoCardInfo& vcInfo);
	bool InitializeSwapChain(
		bool fullscreen,
		HWND hwnd,
		UINT numerator,
		UINT denominator);
	bool InitializeDepthBuffer();
	bool SetupRasteraizerState();
	void SetupViewPort();
	void SetupMatrices(float screenDepth, float screenNear);

};

struct VideoCardInfo
{
	int videoMemory;
	int refreshNumerator;
	int refreshDenominator;
	char cardname[];
};

#endif