#ifndef _SYSTEMCLASS_H_
#define _SYSTEMCLASS_H_

#pragma once

#include "stdafx.h"
#include "Engine.h"

#define MAX_LOADSTRING 100

LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
extern HINSTANCE g_hInstance;

class SystemClass
{
public:

	// Forward declarations of functions included in this code module:
	ATOM MyRegisterClass(HINSTANCE hInstance);
	BOOL InitInstance();


	// Global Variables:
	SystemClass* ApplicationHandle = 0;
	HWND hWnd;
	TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
	TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name


	SystemClass(const SystemClass&);
	SystemClass::SystemClass(
		HINSTANCE hInstance,
		int       nCmdShow);
	~SystemClass();

	bool Initialize();
	void Shutdown();
	void Run();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	SystemClass();
	bool Frame();
	void InitializeWindows(int& screenWidth, int& screenHeight);
	void ShutdownWindows();

private:
	WNDCLASSEX wcex;
	LPCWSTR m_applicationName;
	HINSTANCE m_hinstance;
	HWND m_hwnd;
	int m_nCmdShow;

	//InputClass* m_Input;
	Engine* m_Engine;
};

#endif
