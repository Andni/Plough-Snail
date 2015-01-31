////////////////////////////////////////////////////////////////////////////////
// Filename: colorshaderclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _COLORSHADER_H_
#define _COLORSHADER_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>
#include <fstream>
using namespace std;


class ShaderException : public exception
{
private:
	ID3D10Blob* m_errorMessage;
	HWND m_hwnd;
	WCHAR* m_filename;

public:
	explicit ShaderException(ID3D10Blob*, HWND, WCHAR*);
	const char* what();
	ID3D10Blob* GetErrorMessage();
	HWND GetHWND();
	WCHAR* GetFilename();
};


////////////////////////////////////////////////////////////////////////////////
// Class name: ColorShader
////////////////////////////////////////////////////////////////////////////////
class ColorShader
{
private:

	struct MatrixBufferType
	{
		D3DXMATRIX world;
		D3DXMATRIX view;
		D3DXMATRIX projection;
	};

	struct LightBufferType
	{
		D3DXVECTOR4 color;
		D3DXVECTOR3 direction;
		float padding;
	};

public:
	ColorShader();
	ColorShader(const ColorShader&);
	~ColorShader();

		bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, D3DXVECTOR4, D3DXVECTOR3);

private:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	bool TryInitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	bool CompileVertexShader(WCHAR*, ID3D10Blob**, ID3D10Blob**, HWND);
	bool CompilePixelShader(WCHAR*, ID3D10Blob**, ID3D10Blob**, HWND);
	
	HRESULT CreateInputLayout(ID3D11Device*, const void *, int);
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);
	void ShutdownShader();
	

	bool SetShaderParameters(ID3D11DeviceContext*, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, D3DXVECTOR4, D3DXVECTOR3);
	void RenderShader(ID3D11DeviceContext*, int);

	void HandleShaderException(ShaderException e);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_ixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_lightBuffer;
};

#endif
