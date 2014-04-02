#pragma once

#include <Jamgine\Include\Jamgine.h>
#include <TestGame\Include\GameTimer.h>
#include <Windows.h>

class Jamgame
{
public:
	Jamgame();
	~Jamgame();
	void Initialize();
	int Run();
private:
	HINSTANCE	m_hInstance;
	HWND		m_handle;
	GameTimer*	m_gameTimer;
	bool		m_gamePaused;
	int			m_mousePositionX;
	int			m_mousePositionY;
	bool		m_lMouseClicked;

	Jamgine::JamgineEngine*	m_jamgine;

	void Update();
	void Render();

	LRESULT MsgProc(HWND p_hwnd, UINT p_msg, WPARAM p_wParam, LPARAM p_lParam);
};

