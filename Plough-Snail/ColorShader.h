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

#include "ShaderException.h"

using namespace std;


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

	struct ShaderInitInfo
	{

	};

	struct ShaderCompileInfo
	{

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

	void CompileVertexShader(WCHAR*, ID3D10Blob**, ID3D10Blob**, HWND);
	void CompilePixelShader(WCHAR*, ID3D10Blob**, ID3D10Blob**, HWND);
	
	void CreateVertexShader(
		__in  const void *pShaderBytecode,
		__in  SIZE_T BytecodeLength,
		__in_opt  ID3D11ClassLinkage *pClassLinkage,
		__out_opt  ID3D11VertexShader **ppVertexShader,
		ID3D11Device*,
		HWND);
	void CreatePixelShader(
		__in  const void *pShaderBytecode,
		__in  SIZE_T BytecodeLength,
		__in_opt  ID3D11ClassLinkage *pClassLinkage,
		__out_opt  ID3D11PixelShader **ppPixelShader,
		ID3D11Device*,
		HWND);


	HRESULT CreateInputLayout(ID3D11Device*, const void *, int);
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);
	void ShutdownShader();
	

	bool SetShaderParameters(ID3D11DeviceContext*, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, D3DXVECTOR4, D3DXVECTOR3);
	void RenderShader(ID3D11DeviceContext*, int);

	void HandleShaderException(ShaderException e, HWND);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_ixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_lightBuffer;
};

#endif
