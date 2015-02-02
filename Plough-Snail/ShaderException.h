#ifndef _SHADEREXCEPTION_H_
#define _SHADEREXCEPTION_H_

#include <d3d11.h>
#include <fstream>

using namespace std;


class ShaderException : public exception
{
protected:
	string m_message;
	ID3D10Blob* m_errorMessage;
	HWND m_hwnd;
	WCHAR* m_filename;

public:
	explicit ShaderException(const string& message);
	explicit ShaderException(const char* message);
	explicit ShaderException(ID3D10Blob*, WCHAR*);

	const char* what();
	ID3D10Blob* GetErrorMessage();
	WCHAR* GetFilename();
};

#endif