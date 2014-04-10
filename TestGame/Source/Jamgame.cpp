#include "Jamgame.h"

#include <Windowsx.h>
#include <Jamgine/Include/DirectX/DirectXShared.h>

namespace
{
	Jamgame* g_jamgame = nullptr;
}


LRESULT CALLBACK MainWndProc(HWND p_hwnd, UINT p_msg, WPARAM p_wParam, LPARAM p_lParam)
{
	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before m_hMainWnd is valid.
	return g_jamgame->MsgProc(p_hwnd, p_msg, p_wParam, p_lParam);
}


Jamgame::Jamgame()
:	m_gameTimer(nullptr), m_gamePaused(false), m_mousePositionX(0), m_mousePositionY(0), m_lMouseClicked(false),
	m_jamgine(nullptr)
{
	g_jamgame = this;
}

Jamgame::~Jamgame()
{
	// Free everything here
}

void Jamgame::Initialize(HINSTANCE p_hInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	m_hInstance = p_hInstance;
	
	ErrorMessage hr;	
	// Jamengine
	hr = Jamgine::JamgineEngine::CreateEngine(&m_jamgine, Jamgine::GraphicalSystem::DIRECTX);
	if (hr == J_FAIL)
		return;

	Jamgine::Data_Send l_data;
	l_data.hInstance	= p_hInstance;
	l_data.messageProc	= &MainWndProc;
	l_data.clientWidth	= 800;
	l_data.clientHeight = 800;

	// Init engine
	hr = m_jamgine->Initialize(l_data);

	// Gametimer
	m_gameTimer = new GameTimer();
	m_gameTimer->Reset();	

	m_jamgine->LoadTexture(&a, "Box1.dds");
	m_jamgine->LoadTexture(&b, "Box2.dds");
	m_jamgine->LoadTexture(&c, "Box3.dds");
	m_jamgine->LoadTexture(&d, "Box4.dds");
}

int Jamgame::Run()
{
	MSG l_msg = { 0 };
	
	while (l_msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.
		if (PeekMessage(&l_msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&l_msg);
			DispatchMessage(&l_msg);
		}
		else
		{
			m_gameTimer->Tick();
			if (!m_gamePaused)
			{
				Update();
				Render();
			}
			else
			{
				Sleep(100);
			}
		}
	}
	return 1;
}

void Jamgame::Update()
{

}

void Jamgame::Render()
{
	m_jamgine->Render(Jamgine::Position(0, 0), a, Jamgine::SpriteEffect::NONE);
	m_jamgine->Render(Jamgine::Position(0, 0), c, Jamgine::SpriteEffect::NONE);
	m_jamgine->Render(Jamgine::Position(0, 0), a, Jamgine::SpriteEffect::NONE);
	m_jamgine->Render(Jamgine::Position(0, 0), b, Jamgine::SpriteEffect::NONE);
	m_jamgine->Render(Jamgine::Position(0, 0), b, Jamgine::SpriteEffect::NONE);
	m_jamgine->Render(Jamgine::Position(0, 0), a, Jamgine::SpriteEffect::NONE);
	m_jamgine->Render(Jamgine::Position(0, 0), c, Jamgine::SpriteEffect::NONE);
	m_jamgine->Render(Jamgine::Position(0, 0), d, Jamgine::SpriteEffect::NONE);

	m_jamgine->PostRender();
}

LRESULT CALLBACK Jamgame::MsgProc(HWND p_hwnd, UINT p_msg, WPARAM p_wParam, LPARAM p_lParam)
{
	switch (p_msg)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.  
		// We pause the game when the window is deactivated and unpause it 
		// when it becomes active.  
	case WM_ACTIVATE:
		if (LOWORD(p_wParam) == WA_INACTIVE)
		{
			//m_appPaused = true;
			//m_gameTimer->Stop();
		}
		else
		{
			//m_appPaused = false;
			//m_gameTimer->Start();
		}
		return 0;

		// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// The WM_MENUCHAR message is sent when a menu is active and the user presses 
		// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return MAKELRESULT(0, MNC_CLOSE);
	case WM_LBUTTONDOWN:
//		g_jamgame->m_lMouseClicked = true;
		return 0;
	case WM_LBUTTONUP:
//		m_lMouseClicked = false;
		return 0;

	case WM_MOUSEMOVE:
//		m_mousePositionX = GET_X_LPARAM(p_lParam);
//		m_mousePositionY = GET_Y_LPARAM(p_lParam);
		return 0;
	}
	return DefWindowProc(p_hwnd, p_msg, p_wParam, p_lParam);
}