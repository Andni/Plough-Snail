////////////////////////////////////////////////////////////////////////////////
// Filename: colorshaderclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "colorshader.h"

ColorShader::ColorShader()
{
	m_vertexShader = 0;
	m_ixelShader = 0;
	m_layout = 0;
	m_matrixBuffer = 0;
	m_lightBuffer = 0;
}


ColorShader::ColorShader(const ColorShader& other)
{
}


ColorShader::~ColorShader()
{
}

bool ColorShader::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result;


	// Initialize the vertex and pixel shaders.
	result = InitializeShader(device, hwnd, L"../EveOnlineMap/ColorVS.hlsl", L"../EveOnlineMap/ColorPS.hlsl");
	if (!result)
	{
		return false;
	}

	return true;
}

void ColorShader::Shutdown()
{
	// Shutdown the vertex and pixel shaders as well as the related objects.
	ShutdownShader();

	return;
}

bool ColorShader::Render(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix,
	D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, D3DXVECTOR4 diffuseColor, D3DXVECTOR3 lightDirection)
{
	bool result;


	// Set the shader parameters that it will use for rendering.
	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, diffuseColor, lightDirection);
	if (!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, indexCount);

	return true;
}

bool ColorShader::TryInitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	try
	{
		InitializeShader(device, hwnd, vsFilename, psFilename);
	}
	catch (exception e)
	{
		return false;
	}
	return true;
}

bool ColorShader::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT result;
	D3D11_BUFFER_DESC matrixBufferDesc;


	// Initialize the pointers this function will use to null.
	ID3D10Blob* errorMessage = 0;
	ID3D10Blob* vertexShaderBuffer = 0;
	ID3D10Blob* pixelShaderBuffer = 0;
	
	if (!CompileVertexShader(vsFilename, &vertexShaderBuffer, &errorMessage, hwnd))
	{
		return false;
	}

	if (!CompilePixelShader(psFilename, &pixelShaderBuffer, &errorMessage, hwnd))
	{
		return false;
	}

	// Create the vertex shader from the buffer.
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result))
	{
		return false;
	}

	// Create the pixel shader from the buffer.
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_ixelShader);
	if (FAILED(result))
	{
		return false;
	}

	// Now setup the layout of the data that goes into the shader.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
	result = CreateInputLayout(device, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize());
	if (FAILED(result))
	{
		return false;
	}

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Setup the description of the light dynamic constant buffer that is in the pixel shader.
	// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	D3D11_BUFFER_DESC lightBufferDesc;
	ZeroMemory(&lightBufferDesc, sizeof(lightBufferDesc));
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	
	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&lightBufferDesc, NULL, &m_lightBuffer);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

void ColorShader::ShutdownShader()
{
	// Release the matrix constant buffer.
	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}

	// Release the layout.
	if (m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	// Release the pixel shader.
	if (m_ixelShader)
	{
		m_ixelShader->Release();
		m_ixelShader = 0;
	}

	// Release the vertex shader.
	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}

	return;
}

void ColorShader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long bufferSize, i;
	ofstream fout;


	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	// Write out the error message.
	for (i = 0; i<bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();

	// Release the error message.
	errorMessage->Release();
	errorMessage = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}

bool ColorShader::SetShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix,
	D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, D3DXVECTOR4 diffuseColor, D3DXVECTOR3 lightDirection)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	unsigned int bufferNumber;

	// Transpose the matrices to prepare them for the shader.
	D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	// Unlock the constant buffer.
	deviceContext->Unmap(m_matrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Finanly set the constant buffer in the vertex shader with the updated values.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	return true;
}


void ColorShader::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_layout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_ixelShader, NULL, 0);

	// Render the triangle.
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}

bool ColorShader::CompileVertexShader(WCHAR* vsFilename, ID3D10Blob** vertexShaderBuffer, ID3D10Blob** errorMessage, HWND hwnd)
{
	// Compile the vertex shader code.
	HRESULT result = D3DX11CompileFromFile(vsFilename, NULL, NULL, "ColorVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
		vertexShaderBuffer, errorMessage, NULL);
	if (FAILED(result))
	{
		throw ShaderException(*errorMessage, hwnd, vsFilename);
		return false;;
	}
	return true;
}


bool ColorShader::CompilePixelShader(WCHAR* psFilename, ID3D10Blob** pixelShaderBuffer, ID3D10Blob** errorMessage, HWND hwnd)
{

	// Compile the pixel shader code.
	HRESULT result = D3DX11CompileFromFile(psFilename, NULL, NULL, "ColorPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
		pixelShaderBuffer, errorMessage, NULL);
	if (FAILED(result))
	{
		throw ShaderException(*errorMessage, hwnd, psFilename);
		return false;
	}
	return true;
}

HRESULT ColorShader::CreateInputLayout(ID3D11Device* device, const void * vertexShaderBuffer, int vertexShaderBufferSize)
{
	D3D11_INPUT_ELEMENT_DESC polygonLayout[4];

	ZeroMemory(&polygonLayout[0], sizeof(D3D11_INPUT_ELEMENT_DESC));
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	ZeroMemory(&polygonLayout[1], sizeof(D3D11_INPUT_ELEMENT_DESC));
	polygonLayout[1].SemanticName = "COLOR";
	polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	ZeroMemory(&polygonLayout[2], sizeof(D3D11_INPUT_ELEMENT_DESC));
	polygonLayout[2].SemanticName = "TEXTURE";
	polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	ZeroMemory(&polygonLayout[3], sizeof(D3D11_INPUT_ELEMENT_DESC));
	polygonLayout[3].SemanticName = "NORMAL";
	polygonLayout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;


	// Get a count of the elements in the layout.
	unsigned int numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	return device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer,
		vertexShaderBufferSize, &m_layout);
}

void ColorShader::HandleShaderException(ShaderException e)
{
	// If the shader failed to compile it should have writen something to the error message.
	ID3D10Blob* errorMessage = e.GetErrorMessage();
	if (errorMessage)
	{
		OutputShaderErrorMessage(errorMessage, e.GetHWND(), e.GetFilename());
	}
	// If there was  nothing in the error message then it simply could not find the file itself.
	else
	{
		MessageBox(e.GetHWND(), e.GetFilename(), L"Missing Shader File", MB_OK);
	}
}

ShaderException::ShaderException(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* filename)
{
	m_errorMessage = errorMessage;
	m_hwnd = hwnd;
	m_filename = filename;
}

HWND ShaderException::GetHWND()
{
	return m_hwnd;
}

ID3D10Blob* ShaderException::GetErrorMessage()
{
	return m_errorMessage;
}

WCHAR* ShaderException::GetFilename()
{
	return m_filename;
}