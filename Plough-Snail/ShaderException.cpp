#include "stdafx.h"
#include "ShaderException.h"


ShaderException::ShaderException(const string& message) : m_message(message) {}

ShaderException::ShaderException(const char* message) : m_message(message) {}

ShaderException::ShaderException(ID3D10Blob* errorMessage, WCHAR* filename)
{
	m_errorMessage = errorMessage;
	m_filename = filename;
}

ID3D10Blob* ShaderException::GetErrorMessage()
{
	return m_errorMessage;
}

WCHAR* ShaderException::GetFilename()
{
	return m_filename;
}