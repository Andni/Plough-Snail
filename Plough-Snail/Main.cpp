#include "stdafx.h"

#include "SystemClass.h"

HINSTANCE g_hInstance = 0;

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	SystemClass* pEveMap;
	bool result;


	// Create the pEveMap object.
	pEveMap = new SystemClass(hInstance, nCmdShow);
	if (!pEveMap)
	{
		return 0;
	}		

	// Initialize and run the pEveMap object.
	result = pEveMap->Initialize();
	if (result)
	{
		pEveMap->Run();
	}

	// Shutdown and release the pEveMap object.
	pEveMap->Shutdown();
	
	delete pEveMap;
	pEveMap = 0;

	return 0;
}