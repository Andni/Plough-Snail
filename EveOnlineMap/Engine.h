#pragma once

#ifndef _ENGINE_H_
#define _ENGINE_H_

#include "stdafx.h"
#include "Graphics.h"
#include "Model.h"
#include "Camera.h"
#include "ColorShader.h"
#include "Light.h"

/////////////
// GLOBALS //
/////////////
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;


class Engine
{
public:
	Engine();
	Engine(const Engine&);
	~Engine();

	bool Initialize(int, int, HWND);
	void Shutdown();
	bool Frame();

private:
	bool Render(float rotation);
	
	bool InitializeCamera();
	bool InitializeModels(HWND hwnd);
	bool InitializeLights();
	bool InitializeShaders(HWND hwnd);
	bool InitializeGraphics(int screenWidth, int screenHeight, HWND hwnd);


private:

	Graphics *m_graphics;
	ModelClass *m_model;
	ColorShader *m_colorShader;
	Camera *m_camera;
	Light *m_light;
};

#endif