/*

Application Core

Highest level in the application structure: hosts both the main loop and UI events and input handlers, 
as well as variables needed throughout the program.

I designed it as a singleton both to experiment with them and to avoid making it a global in main.cpp.

*/

#pragma once

#include <windows.h>
#include <vector>
#include "Sprite.h"
#include "GameData.h"
#include "Timer.h"
#include "NetDefs.h"

class AppCore
{
public:
	static AppCore& GetInstance(void);
	void MainLoop(void);
	void InitResources(HINSTANCE InstanceHandle, HWND WindowHandle, HWND ConnectWindowHandle);
	void FreeResources(void);
	void KeyDown(UINT Key);
	void KeyUp(UINT Key);
	void StartHosting(void);
	void ConnectToRemote(sockaddr_in InputAddr);
	void NetInput(void);

	enum eGameStates {IDLE, HOSTING, SERVERPROBING, CONNECTED};

	HWND GetMainWindowHandle(void) const;
	HWND GetConnectWindowHandle(void) const;

protected:
	AppCore(void);
	~AppCore(void);
	AppCore(AppCore const&);

	void HostingLoop(void);
	void ProbingLoop(void);
	void ConnectedLoop(void);
	void DisplayFrame(void);
	void CreateStatUpdatePack(void);
	void ReadStatUpdatePack(void);

	static AppCore* m_pClassPtr;
	HINSTANCE m_hinst;
	HWND m_hmwnd, m_hcwnd;
	RECT m_ClientRec;
	HDC m_hFrontBuffer, m_hBackBuffer, m_hTempBuffer;
	HBITMAP m_hFrontBufferBitmap, m_hBackBufferBitmap;

	SOCKET m_hSocket;
	int m_iLastSockError;
	sockaddr_in m_LocalAddr,m_ServerAddr;
	float m_fServerLatency, m_fLastRecvPacketSimTime;
	std::vector<ClientData> m_ClientDataVec;
	char m_sNetBuffer[1024];
	UINT m_uiPacketLength;

	eGameStates m_eGameState;
	bool m_bKeys[256];
	std::vector<GameEntity> m_EntVec;

	Timer m_SimTimer, m_NetTimer, m_AliveTimer;
	float m_fTimeElapsedSinceStart, m_fLastFrameTime;
	Sprite *PlayerSpr,*AISpr,*BgSpr;

	//Simulation constants
	const float m_cMaxSpeed;
};
