#include "stdafx.h"

#include "Engine.h"

#include "ColorShader.h"
#include "Model.h"


Engine::Engine()
{
	m_graphics = 0;
	m_camera = 0;
	m_model = 0;
	m_colorShader = 0;
	m_light = 0;
}


Engine::Engine(const Engine& other)
{
}


Engine::~Engine()
{
}


bool Engine::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	
	if (!InitializeCamera())
	{
		return false;
	}

	if (!InitializeGraphics(screenWidth, screenHeight, hwnd))
	{
		return false;
	}

	if (!InitializeModels(hwnd))
	{
		return false;
	}

	m_light = new Light();
	m_light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_light->SetDirection(-1.0f, -1.0f, -1.0f);

	if (!InitializeShaders(hwnd))
	{
		return false;
	}

	return true;
}


void Engine::Shutdown()
{
	if (m_colorShader)
	{
		m_colorShader->Shutdown();
		delete m_colorShader;
		m_colorShader = 0;
	}

	// Release the model object.
	if (m_model)
	{
		m_model->Shutdown();
		delete m_model;
		m_model = 0;
	}

	// Release the camera object.
	if (m_camera)
	{
		delete m_camera;
		m_camera = 0;
	}

	if (m_graphics)
	{
		m_graphics->Shutdown();
		delete m_graphics;
		m_graphics = 0;
	}
	return;
}

bool Engine::InitializeCamera()
{

	// Create the camera object.
	m_camera = new Camera;
	if (!m_camera)
	{
		return false;
	}

	// Set the initial position of the camera.
	m_camera->SetPosition(1.0f, 1.0f, -10.0f);
	m_camera->SetRotation(10.0f, 0.0f, 0.0f);
}

bool Engine::InitializeModels(HWND hwnd)
{
	m_model = new ModelClass();
	bool result = m_model->Initialize(m_graphics->GetDevice(), "../Plough-Snail/Assets/Models/Hexagon.txt", 0);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
		return false;
	}

	if (!m_model)
	{
		return false;
	}
}

bool Engine::InitializeGraphics(int screenWidth, int screenHeight, HWND hwnd)
{
	// Create the Direct3D object.
	m_graphics = new Graphics;
	if (!m_graphics)
	{
		return false;
	}

	// Initialize the Direct3D object.
	bool result = m_graphics->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}
}

bool Engine::InitializeShaders(HWND hwnd)
{
	m_colorShader = new ColorShader;
	if (!m_colorShader)
	{
		return false;
	}

	bool result = m_colorShader->Initialize(m_graphics->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the color shader object.", L"Error", MB_OK);
		return false;
	}
}

bool Engine::Frame()
{
	bool result;
	

	// Update the rotation variable each frame.
	static float rotation = 0.0f;
	rotation += (float)D3DX_PI * 0.01f;
	if (rotation > 360.0f)
	{
		rotation -= 360.0f;
	}

	// Render the graphics scene.
	result = Render(rotation);
	if (!result)
	{
		return false;
	}

	return true;
}


bool Engine::Render(float rotation)
{

	D3DXMATRIX viewMatrix, projectionMatrix, worldMatrix;
	bool result;


	// Clear the buffers to begin the scene.
	m_graphics->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_camera->Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_camera->GetViewMatrix(viewMatrix);
	m_graphics->GetWorldMatrix(worldMatrix);
	m_graphics->GetProjectionMatrix(projectionMatrix);


	//rotate
	D3DXMatrixRotationY(&worldMatrix, rotation);
	//D3DXMatrixRotationX(&worldMatrix, rotation);

	// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
	m_model->Render(m_graphics->GetDeviceContext());



	// Render the model using the color shader.
	result = m_colorShader->Render(m_graphics->GetDeviceContext(), m_model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_light->GetDiffuseColor(), m_light->GetDirection());
	if (!result)
	{
		return false;
	}

	// Present the rendered scene to the screen.
	m_graphics->EndScene();

	return true;
}
