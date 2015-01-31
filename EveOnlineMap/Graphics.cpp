#include "stdafx.h"

#include <map>
#include <vector>

#include "Vertex.h"
#include "Graphics.h"

IDXGISwapChain *swapchain = NULL;             // the pointer to the swap chain interface
ID3D11Device *dev = NULL;                     // the pointer to our Direct3D device interface
ID3D11DeviceContext *devcon = NULL;           // the pointer to our Direct3D device context
ID3D11RenderTargetView *backbuffer = NULL;

ID3D11VertexShader *pVertexShader = NULL;
ID3D11PixelShader *pPixelShader = NULL;

ID3D10Texture2D *pBackBuffer = NULL;
ID3D11Buffer *pVertexBuffer = NULL;
ID3D11InputLayout *pInputLayout = NULL;

ID3D11Debug *pDebug = NULL;


Graphics::Graphics()
{
	m_swapChain = 0;
	m_device = 0;
	m_deviceContext = 0;
	m_renderTargetView = 0;
	m_depthStencilBuffer = 0;
	m_depthStencilState = 0;
	m_depthStencilView = 0;
	m_rasterState = 0;

	m_screenHeight = 100;
	m_screenWidth = 100;
}

Graphics::Graphics(const Graphics& other)
{}



bool Graphics::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen,
	float screenDepth, float screenNear)
{

	m_screenHeight = screenHeight;
	m_screenWidth = screenWidth;

	// Store the vsync setting.
	m_vsync_enabled = vsync;

	VideoCardInfo vcInfo;
	if (!GetVideoCardInfo(vcInfo))
	{
		return false;
	}


	if (!InitializeSwapChain(fullscreen, hwnd, vcInfo.refreshNumerator, vcInfo.refreshDenominator))
	{
		return false;
	}

	if (!InitializeDepthBuffer())
	{
		return false;
	}


	if (!SetupRasteraizerState())
	{
		return false;
	}

	SetupViewPort();
	SetupMatrices(screenDepth, screenNear);

	return true;
}

void Graphics::Shutdown()
{
	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	if (m_swapChain)
	{
		m_swapChain->SetFullscreenState(false, NULL);
	}

	if (m_rasterState)
	{
		m_rasterState->Release();
		m_rasterState = 0;
	}

	if (m_depthStencilView)
	{
		m_depthStencilView->Release();
		m_depthStencilView = 0;
	}

	if (m_depthStencilState)
	{
		m_depthStencilState->Release();
		m_depthStencilState = 0;
	}

	if (m_depthStencilBuffer)
	{
		m_depthStencilBuffer->Release();
		m_depthStencilBuffer = 0;
	}

	if (m_renderTargetView)
	{
		m_renderTargetView->Release();
		m_renderTargetView = 0;
	}

	if (m_deviceContext)
	{
		m_deviceContext->Release();
		m_deviceContext = 0;
	}

	if (m_device)
	{
		m_device->Release();
		m_device = 0;
	}

	if (m_swapChain)
	{
		m_swapChain->Release();
		m_swapChain = 0;
	}

	return;
}

//helpers

static bool operator <(const DXGI_ADAPTER_DESC & a, const DXGI_ADAPTER_DESC & b)
{
	return a.DeviceId < b.DeviceId;
}

bool Graphics::GetVideoCardInfo(VideoCardInfo& vcInfo)
{
	HRESULT result;

	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput;
	unsigned int numModes, i, numerator, denominator, stringLength;
	DXGI_MODE_DESC* displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	int error;

	// Create a DirectX graphics interface factory.
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(result))
	{
		return false;
	}


	//list adapters
	UINT j = 0;
	IDXGIAdapter * pAdapter;
	std::map <DXGI_ADAPTER_DESC, IDXGIAdapter*> vAdapters;
	while (factory->EnumAdapters(j, &pAdapter) != DXGI_ERROR_NOT_FOUND)
	{

		result = pAdapter->GetDesc(&adapterDesc);
		++j;
		if (FAILED(result))
		{
			continue;
		}

		vAdapters.insert(std::pair<DXGI_ADAPTER_DESC, IDXGIAdapter*>(adapterDesc, pAdapter));
	}

	
	//select the best adapter
	/*adapter = 0;
	UINT adpt = 0;
	SIZE_T bestMemory = 0;
	for (std::map<DXGI_ADAPTER_DESC, IDXGIAdapter*>::iterator it = vAdapters.begin(); it != vAdapters.end(); ++it)
	{
		if (it->first.DedicatedVideoMemory > bestMemory)
		{
			adapter = it->second;
			bestMemory = it->first.DedicatedVideoMemory;
			adpt = it->first.DeviceId;
		}
	}*/
	


	// Use the factory to create an adapter for the primary graphics interface (video card).
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result))
	{
		return false;
	}

	UINT k = 0; 
	IDXGIOutput * pOutput;
	std::vector<IDXGIOutput*> vOutputs;
	while (adapter->EnumOutputs(k, &pOutput) != DXGI_ERROR_NOT_FOUND)
	{
		vOutputs.push_back(pOutput);
		++k;
	}


	// Enumerate the primary adapter output (monitor).
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))
	{
		return false;
	}

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// Create a list to hold all the possible display modes for this monitor/video card combination.
	displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
	{
		return false;
	}

	// Now fill the display mode list structures.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result))
	{
		return false;
	}

	// Now go through all the display modes and find the one that matches the screen width and height.
	// When a match is found store the numerator and denominator of the refresh rate for that monitor.
	for (i = 0; i < numModes; i++)
	{
		if (displayModeList[i].Width == (unsigned int)m_screenWidth)
		{
			if (displayModeList[i].Height == (unsigned int)m_screenHeight)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	// Get the adapter (video card) description.
	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		return false;
	}

	// Store the dedicated video card memory in megabytes.
	m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// Convert the name of the video card to a character array and store it.
	error = wcstombs_s(&stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128);
	if (error != 0)
	{
		return false;
	}

	// Release the display mode list.
	delete[] displayModeList;
	displayModeList = 0;

	// Release the adapter output.
	adapterOutput->Release();
	adapterOutput = 0;

	// Release the adapter.
	adapter->Release();
	adapter = 0;

	// Release the factory.
	factory->Release();
	factory = 0;

	return true;
}

bool Graphics::InitializeSwapChain(
	bool fullscreen,
	HWND hwnd,
	UINT numerator,
	UINT denominator)
{

	DXGI_SWAP_CHAIN_DESC swapChainDesc;

	// Initialize the swap chain description.
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// Set to a single back buffer.
	swapChainDesc.BufferCount = 1;

	// Set the width and height of the back buffer.
	swapChainDesc.BufferDesc.Width = m_screenWidth;
	swapChainDesc.BufferDesc.Height = m_screenHeight;

	// Set regular 32-bit surface for the back buffer.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Set the refresh rate of the back buffer.
	if (m_vsync_enabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	// Set the usage of the back buffer.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// Set the handle for the window to render to.
	swapChainDesc.OutputWindow = hwnd;

	// Turn multisampling off.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// Set to full screen or windowed mode.
	if (fullscreen)
	{
		swapChainDesc.Windowed = false;
	}
	else
	{
		swapChainDesc.Windowed = true;
	}

	// Set the scan line ordering and scaling to unspecified.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Discard the back buffer contents after presenting.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// Don't set the advanced flags.
	swapChainDesc.Flags = 0;

	// Set the feature level to DirectX 11.
	D3D_FEATURE_LEVEL featureLevel;
	featureLevel = D3D_FEATURE_LEVEL_11_0;

	HRESULT result;

	// Create the swap chain, Direct3D device, and Direct3D device context.
	result = D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		&featureLevel,
		1,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&m_swapChain,
		&m_device,
		NULL,
		&m_deviceContext);

	if (FAILED(result))
	{
		return false;
	}

	ID3D11Texture2D* backBufferPtr;
	// Get the pointer to the back buffer.
	result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(result))
	{
		return false;
	}

	// Create the render target view with the back buffer pointer.
	result = m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView);
	if (FAILED(result))
	{
		return false;
	}

	// Release pointer to the back buffer as we no longer need it.
	backBufferPtr->Release();
	backBufferPtr = 0;

	return true;
}

bool Graphics::InitializeDepthBuffer()
{

	HRESULT result;
	D3D11_TEXTURE2D_DESC depthBufferDesc;

	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = m_screenWidth;
	depthBufferDesc.Height = m_screenHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	result = m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
	if (FAILED(result))
	{
		return false;
	}


	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	// Initialize the description of the stencil state.
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the depth stencil state.
	result = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
	if (FAILED(result))
	{
		return false;
	}

	// Set the depth stencil state.
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	// Initailze the depth stencil view.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	result = m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
	if (FAILED(result))
	{
		return false;
	}

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	return true;
}

bool Graphics::SetupRasteraizerState()
{
	D3D11_RASTERIZER_DESC rasterDesc;

	// Setup the raster description which will determine how and what polygons will be drawn.
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	HRESULT result = m_device->CreateRasterizerState(&rasterDesc, &m_rasterState);
	if (FAILED(result))
	{
		return false;
	}

	// Now set the rasterizer state.
	m_deviceContext->RSSetState(m_rasterState);

	return true;
}

void Graphics::SetupViewPort()
{

	D3D11_VIEWPORT viewport;

	// Setup the viewport for rendering.
	viewport.Width = (float)m_screenWidth;
	viewport.Height = (float)m_screenHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	// Create the viewport.
	m_deviceContext->RSSetViewports(1, &viewport);
}

void Graphics::SetupMatrices(float screenDepth, float screenNear)
{
	float fieldOfView, screenAspect;

	// Setup the projection matrix.
	fieldOfView = (float)D3DX_PI / 4.0f;
	screenAspect = (float)m_screenWidth / (float)m_screenHeight;

	// Create the projection matrix for 3D rendering.
	D3DXMatrixPerspectiveFovLH(&m_rojectionMatrix, fieldOfView, screenAspect, screenNear, screenDepth);


	// Initialize the world matrix to the identity matrix.
	D3DXMatrixIdentity(&m_worldMatrix);

	// Create an orthographic projection matrix for 2D rendering.
	D3DXMatrixOrthoLH(&m_orthoMatrix, (float)m_screenWidth, (float)m_screenHeight, screenNear, screenDepth);
}


void Graphics::BeginScene(float red, float green, float blue, float alpha)
{
	float color[4];


	// Setup the color to clear the buffer to.
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	// Clear the back buffer.
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, color);

	// Clear the depth buffer.
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	return;
}


void Graphics::EndScene()
{
	// Present the back buffer to the screen since rendering is complete.
	if (m_vsync_enabled)
	{
		// Lock to screen refresh rate.
		m_swapChain->Present(1, 0);
	}
	else
	{
		// Present as fast as possible.
		m_swapChain->Present(0, 0);
	}

	return;
}

ID3D11Device* Graphics::GetDevice()
{
	return m_device;
}


ID3D11DeviceContext* Graphics::GetDeviceContext()
{
	return m_deviceContext;
}

void Graphics::GetProjectionMatrix(D3DXMATRIX& projectionMatrix)
{
	projectionMatrix = m_rojectionMatrix;
	return;
}


void Graphics::GetWorldMatrix(D3DXMATRIX& worldMatrix)
{
	worldMatrix = m_worldMatrix;
	return;
}


void Graphics::GetOrthoMatrix(D3DXMATRIX& orthoMatrix)
{
	orthoMatrix = m_orthoMatrix;
	return;
}

void Graphics::GetVideoCardInfo(char* cardName, int& memory)
{
	strcpy_s(cardName, 128, m_videoCardDescription);
	memory = m_videoCardMemory;
	return;
}

//deprecated
//
//void Graphics::InitD3D(HWND hWnd)
//{
//	DXGI_SWAP_CHAIN_DESC swapChainDescription;
//	ZeroMemory(&swapChainDescription, sizeof(DXGI_SWAP_CHAIN_DESC));
//
//	swapChainDescription.BufferCount = 1;
//	swapChainDescription.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
//	swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//	swapChainDescription.OutputWindow = hWnd;
//	swapChainDescription.SampleDesc.Count = 1;
//	swapChainDescription.Windowed = true;
//
//	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
//
//	HRESULT res = D3D11CreateDeviceAndSwapChain(
//		NULL,
//		D3D_DRIVER_TYPE_HARDWARE,
//		NULL,
//		D3D11_CREATE_DEVICE_DEBUG,
//		featureLevels,
//		1,
//		D3D11_SDK_VERSION,
//		&swapChainDescription,
//		&swapchain,
//		&dev,
//		NULL,
//		&devcon);
//
//
//	HRESULT hr = dev->QueryInterface(IID_PPV_ARGS(&pDebug));
//
//	// get the address of the back buffer
//	ID3D11Texture2D *pBackBuffer;
//	swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
//
//	// use the back buffer address to create the render target
//	dev->CreateRenderTargetView(pBackBuffer, NULL, &backbuffer);
//	pBackBuffer->Release();
//
//	// set the render target as the back buffer
//	devcon->OMSetRenderTargets(1, &backbuffer, NULL);
//
//	// Set the viewport
//	D3D11_VIEWPORT viewport;
//	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
//
//	viewport.TopLeftX = 0;
//	viewport.TopLeftY = 0;
//	viewport.Width = 200;
//	viewport.Height = 100;
//
//	devcon->RSSetViewports(1, &viewport);
//
//	InitGraphics();
//	InitPipeline();
//
//}
//
//void Graphics::RenderFrame(void)
//{
//	// clear the back buffer to a deep blue
//	devcon->ClearRenderTargetView(backbuffer, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));
//
//	// do 3D rendering on the back buffer here
//	UINT stride = sizeof(Vertex);
//	UINT offset = 0;
//	devcon->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
//	devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
//	//devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//	devcon->Draw(3, 0);
//
//	// switch the back buffer and the front buffer
//	swapchain->Present(0, 0);
//}
//
//void Graphics::InitGraphics()
//{
//	D3D11_BUFFER_DESC bd;
//	ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
//
//	bd.Usage = D3D11_USAGE_DYNAMIC;
//	bd.ByteWidth = sizeof(Vertex) * 3;
//	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//
//	dev->CreateBuffer(&bd, NULL, &pVertexBuffer);
//
//
//	Vertex OurVertices[] =
//	{
//		{ 0.0f, 0.5f, 0.0f, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f) },
//		{ 0.45f, -0.5, 0.0f, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f) },
//		{ -0.45f, -0.5f, 0.0f, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f) }
//	};
//
//	D3D11_MAPPED_SUBRESOURCE ms;
//	devcon->Map(pVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
//	memcpy(ms.pData, OurVertices, sizeof(OurVertices));
//	devcon->Unmap(pVertexBuffer, NULL);
//
//}
//
//void Graphics::InitPipeline()
//{
//
//	ID3DBlob *vsBlob, *psBlob;
//	D3DX11CompileFromFile(L"Shaders.hlsl", 0, 0, "VShader", "vs_4_0", 0, 0, 0, &vsBlob, 0, 0);
//	D3DX11CompileFromFile(L"Shaders.hlsl", 0, 0, "PShader", "ps_4_0", 0, 0, 0, &psBlob, 0, 0);
//
//	dev->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, &pVertexShader);
//	dev->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), NULL, &pPixelShader);
//
//	devcon->VSSetShader(pVertexShader, 0, 0);
//	devcon->PSSetShader(pPixelShader, 0, 0);
//
//	D3D11_INPUT_ELEMENT_DESC ied[] =
//	{
//		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//	};
//
//	dev->CreateInputLayout(ied, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &pInputLayout);
//	devcon->IASetInputLayout(pInputLayout);
//}

void Graphics::CleanD3D()
{
	// close and release all existing COM objects


	pVertexShader->Release();
	pPixelShader->Release();
	pVertexBuffer->Release();
	swapchain->Release();
	backbuffer->Release();
	//if(pBackBuffer != NULL) pBackBuffer->Release();
	dev->Release();
	devcon->Release();
	pInputLayout->Release();
	swapchain->Release();

	if (pDebug != nullptr)
	{
		pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
		pDebug = nullptr;
	}
}