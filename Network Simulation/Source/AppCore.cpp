#include "AppCore.h"

AppCore* AppCore::m_pClassPtr = NULL;

AppCore::AppCore(void):
m_cMaxSpeed(0.4f),
m_hSocket(INVALID_SOCKET)
{
	UINT i;
	m_eGameState = IDLE;
	for(i=0;i<256;i++)
	{
		m_bKeys[i] = false;
	}
}

AppCore::~AppCore(void)
{
}

AppCore& AppCore::GetInstance(void)
{
	if(m_pClassPtr == NULL)
	{
		m_pClassPtr = new AppCore();
	}	
		
	return *m_pClassPtr;
}

void AppCore::MainLoop(void)
{
	switch(m_eGameState)
	{
	case IDLE:
		break;
	case HOSTING:
		HostingLoop();
		break;
	case SERVERPROBING:
		ProbingLoop();
		break;
	case CONNECTED:
		ConnectedLoop();
		break;
	}
	DisplayFrame();
}

void AppCore::InitResources(HINSTANCE InstanceHandle, HWND WindowHandle, HWND ConnectWindowHandle)
{
	//record handles to program instance and program main window (used for almost everything)
	m_hinst = InstanceHandle;
	m_hmwnd = WindowHandle;
	m_hcwnd = ConnectWindowHandle;
	//Get main window client area description
	GetClientRect(m_hmwnd,&m_ClientRec);
	//Acquire DC to main window client area (front buffer)
	m_hFrontBuffer = GetDC(m_hmwnd);
	//Create Back buffer to be compatible with front buffer
	m_hBackBuffer = CreateCompatibleDC(m_hFrontBuffer);
	//Create a temporary device contest to be used as a source for copying the various bitmaps to the back buffer
	m_hTempBuffer = CreateCompatibleDC(m_hBackBuffer);
	
	//Create bitmaps for Device Contexts
	m_hFrontBufferBitmap = CreateCompatibleBitmap(m_hFrontBuffer, m_ClientRec.right, m_ClientRec.bottom);
	m_hBackBufferBitmap = (HBITMAP)SelectObject(m_hBackBuffer, m_hFrontBufferBitmap);
	FillRect(m_hBackBuffer, &m_ClientRec, (HBRUSH)GetStockObject(0));

	PlayerSpr = new Sprite(SPR1,SPR1M,m_hBackBuffer, m_hTempBuffer);
	AISpr = new Sprite(SPR2, SPR2M, m_hBackBuffer, m_hTempBuffer);
	BgSpr = new Sprite(LVBG, BGMASK, m_hBackBuffer, m_hTempBuffer);
}

void AppCore::FreeResources(void)
{
	//No idea why
	SelectObject(m_hBackBuffer,m_hBackBufferBitmap);
	//Release DCs
	DeleteDC(m_hBackBuffer);
	DeleteDC(m_hTempBuffer);
	ReleaseDC(m_hmwnd,m_hFrontBuffer);

	delete BgSpr;
	delete AISpr;
	delete PlayerSpr;

	if(m_hSocket != INVALID_SOCKET)
	{
		closesocket(m_hSocket);
	}
}

void AppCore::KeyDown(UINT Key)
{
	if(Key < 256)
	{
		m_bKeys[Key] = true;
	}
}

void AppCore::KeyUp(UINT Key)
{
	if(Key < 256)
	{
		m_bKeys[Key] = false;
	}
}

void AppCore::StartHosting(void)
{
	m_ClientDataVec.clear();
	m_EntVec.clear();
	GameEntity TempEntity;

	//Start simulation timer and initialize time tracking variables
	m_SimTimer.SetTimeBase();
	m_NetTimer.SetTimeBase();
	m_fLastFrameTime = 0.0f;

	//Initialize communication socket
	if(m_hSocket != INVALID_SOCKET)
	{
		closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;
	}

	m_hSocket = socket (AF_INET,SOCK_DGRAM,0);
	if(m_hSocket == INVALID_SOCKET)
	{
		MessageBox(NULL, TEXT("Unable to initialize socket"), NULL, MB_ICONERROR | MB_OK);
	}

	m_LocalAddr.sin_family = AF_INET;
	m_LocalAddr.sin_port = htons(NetDefs::cServerPort);
	m_LocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(&(m_LocalAddr.sin_zero), 0x0, 8);

	if(bind(m_hSocket,(sockaddr*)&m_LocalAddr,sizeof(m_LocalAddr)) != 0)
	{
		MessageBox(NULL, TEXT("Unable to bind socket"), NULL, MB_ICONERROR | MB_OK);
	}

	if(WSAAsyncSelect(m_hSocket,m_hmwnd,WM_SOCKET,FD_READ) != 0)
	{
		MessageBox(NULL, TEXT("Unable to set socket to asyncronous mode"), NULL, MB_ICONERROR | MB_OK);
	}

	//Player starting condition
	TempEntity.Type = GameEntity::PLAYER;
	TempEntity.ID = 0;
	TempEntity.Pos.x = 50.0f;
	TempEntity.Pos.y = 40.0f;
	m_EntVec.push_back(TempEntity);

	//AI entities
	TempEntity.Type = GameEntity::AI;
	TempEntity.ID = 1;
	TempEntity.Pos.x = 120.0f;
	TempEntity.Pos.y = 80.0f;
	TempEntity.Vel.x = float(rand());
	TempEntity.Vel.y = float(rand());
	TempEntity.Vel.NormaliseSelf();
	TempEntity.Vel = TempEntity.Vel * m_cMaxSpeed;
	m_EntVec.push_back(TempEntity);

	TempEntity.Type = GameEntity::AI;
	TempEntity.ID = 2;
	TempEntity.Pos.x = 600.0f;
	TempEntity.Pos.y = 500.0f;
	TempEntity.Vel.x = float(rand());
	TempEntity.Vel.y = float(rand());
	TempEntity.Vel.NormaliseSelf();
	TempEntity.Vel = TempEntity.Vel * m_cMaxSpeed;
	m_EntVec.push_back(TempEntity);

	TempEntity.Type = GameEntity::AI;
	TempEntity.ID = 3;
	TempEntity.Pos.x = 80.0f;
	TempEntity.Pos.y = 220.0f;
	TempEntity.Vel.x = float(rand());
	TempEntity.Vel.y = float(rand());
	TempEntity.Vel.NormaliseSelf();
	TempEntity.Vel = TempEntity.Vel * m_cMaxSpeed;
	m_EntVec.push_back(TempEntity);

	m_eGameState = HOSTING;
}

void AppCore::ConnectToRemote(sockaddr_in InputAddr)
{
	m_ServerAddr = InputAddr;

	m_EntVec.clear();

	//Socket initialization
	if(m_hSocket != INVALID_SOCKET)
	{
		closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;
	}

	m_hSocket = socket (AF_INET,SOCK_DGRAM,0);
	if(m_hSocket == INVALID_SOCKET)
	{
		MessageBox(NULL, TEXT("Unable to initialize socket"), NULL, MB_ICONERROR | MB_OK);
	}

	m_LocalAddr.sin_family = AF_INET;
	m_LocalAddr.sin_port = htons(0);
	m_LocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(m_hSocket,(sockaddr*)&m_LocalAddr,sizeof(m_LocalAddr)) != 0)
	{
		MessageBox(NULL, TEXT("Unable to bind socket"), NULL, MB_ICONERROR | MB_OK);
	}

	if(WSAAsyncSelect(m_hSocket,m_hmwnd,WM_SOCKET,FD_READ) != 0)
	{
		MessageBox(NULL, TEXT("Unable to set socket to asyncronous mode"), NULL, MB_ICONERROR | MB_OK);
	}

	//Send Connect message to server
	NetDefs::MsgConnectStr Packet;
	m_uiPacketLength = sizeof(Packet);
	CopyMemory(&m_sNetBuffer[0],&Packet,m_uiPacketLength);
	sendto(m_hSocket,m_sNetBuffer,m_uiPacketLength,NULL,(sockaddr*)&m_ServerAddr,sizeof(m_ServerAddr));
	//Start Timer to evaluate network latency
	m_NetTimer.SetTimeBase();
	m_eGameState = SERVERPROBING;
}

void AppCore::NetInput(void)
{
	//Get packet and record sender information
	sockaddr_in SenderAddr;
	int SenderAddrLen = sizeof(SenderAddr);
	int RecvBytes = recvfrom(m_hSocket,m_sNetBuffer,1024,NULL,(sockaddr*)&SenderAddr,&SenderAddrLen);

	if(RecvBytes == SOCKET_ERROR) //If there's an error
	{
		//Get the last error
		m_iLastSockError = WSAGetLastError();
		//If it isn't just a notification that the function would be blocking, pop up a message box
		if(m_iLastSockError != WSAEWOULDBLOCK)
		{
			//MessageBox(NULL, TEXT("Error while reading from socket"), NULL, MB_ICONERROR | MB_OK);
		}
	}
	else //If reading from the socket doesn't result in an error, process data received
	{
		switch(m_eGameState)
		{
		case HOSTING: //Behavior when hosting
			if(m_sNetBuffer[0] == NetDefs::cMsgConnect) //Connection request received
			{
				//Validate connection request (make sure an instance of this program is trying to connect)
				NetDefs::MsgConnectStr* ptr = (NetDefs::MsgConnectStr*)m_sNetBuffer;
				if(ptr->MagicNo == NetDefs::cMagicNo)
				{
					//A valid connection request has been received. Add the originating address to the client list
					ClientData TempCD;
					TempCD.Address = SenderAddr;
					TempCD.ActivityTimer.SetTimeBase();
					m_ClientDataVec.push_back(TempCD);
					//And send simulation status back
					CreateStatUpdatePack();
					sendto(m_hSocket,m_sNetBuffer,m_uiPacketLength,NULL,(sockaddr*)&SenderAddr,SenderAddrLen);
				}
			}
			else if(m_sNetBuffer[0] == NetDefs::cMsgStillAlive) //Still alive notification received
			{
				//Validate
				NetDefs::MsgStillAliveStr* ptr = (NetDefs::MsgStillAliveStr*)m_sNetBuffer;
				if(ptr->MagicNo == NetDefs::cMagicNo)
				{
					UINT i;
					//If sender is in connected clients list
					for(i=0;i<m_ClientDataVec.size();i++)
					{
						if(SenderAddr.sin_addr.s_addr == m_ClientDataVec[i].Address.sin_addr.s_addr && SenderAddr.sin_port == m_ClientDataVec[i].Address.sin_port)
						{
							//Reset timeout counter for client disconnection due to inactivity
							m_ClientDataVec[i].ActivityTimer.SetTimeBase();
						}
					}
				}
			}
			break;
		case SERVERPROBING: //Behavior when handshaking with server
			//Check to ensure received packet is coming from server, ignore otherwise
			if(SenderAddr.sin_addr.s_addr == m_ServerAddr.sin_addr.s_addr && SenderAddr.sin_port == m_ServerAddr.sin_port)
			{
				if(m_sNetBuffer[0] == NetDefs::cMsgStatCom) //A simulation state notification has been received: the server has accepted the connection request
				{
					m_fServerLatency = m_NetTimer.GetElapsedMsecs()/2.0f;
					m_fLastRecvPacketSimTime = 0.0f;
					m_SimTimer.SetTimeBase();
					m_fLastFrameTime = 0.0f;
					ReadStatUpdatePack();
					m_NetTimer.SetTimeBase();
					m_AliveTimer.SetTimeBase();
					m_eGameState = CONNECTED;
				}
			}
			break;
		case CONNECTED: //Behavior when connected to a server
			//Check to ensure received packet is coming from server, ignore otherwise
			if(SenderAddr.sin_addr.s_addr == m_ServerAddr.sin_addr.s_addr && SenderAddr.sin_port == m_ServerAddr.sin_port)
			{
				if(m_sNetBuffer[0] == NetDefs::cMsgStatCom) //New simulation state notification received
				{
					ReadStatUpdatePack();
					m_NetTimer.SetTimeBase();
				}
			}
			break;
		}
	}
}

HWND AppCore::GetMainWindowHandle(void) const
{
	return m_hmwnd;
}

HWND AppCore::GetConnectWindowHandle(void) const
{
	return m_hcwnd;
}

void AppCore::HostingLoop(void)
{
	UINT i,j,k;

	//Evaluate time elapsed between current and last frame
	m_fTimeElapsedSinceStart = m_SimTimer.GetElapsedMsecs();
	float TimeDelta = m_fTimeElapsedSinceStart - m_fLastFrameTime;
	m_fLastFrameTime = m_fTimeElapsedSinceStart;

	//Collision with boundaries handling
	for(i=0;i<m_EntVec.size();i++)
	{
		if(m_EntVec[i].Pos.x < (m_ClientRec.left + 20))
		{
			if(m_EntVec[i].Vel.x < 0.0f)
			{
				m_EntVec[i].Vel.x = 0.0f - m_EntVec[i].Vel.x;
			}
		}
		if(m_EntVec[i].Pos.x > (m_ClientRec.right - 20))
		{
			if(m_EntVec[i].Vel.x > 0.0f)
			{
				m_EntVec[i].Vel.x = 0.0f - m_EntVec[i].Vel.x;
			}
		}
		if(m_EntVec[i].Pos.y < (m_ClientRec.top + 20))
		{
			if(m_EntVec[i].Vel.y < 0.0f)
			{
				m_EntVec[i].Vel.y = 0.0f - m_EntVec[i].Vel.y;
			}
		}
		if(m_EntVec[i].Pos.y > (m_ClientRec.bottom - 20))
		{
			if(m_EntVec[i].Vel.y > 0.0f)
			{
				m_EntVec[i].Vel.y = 0.0f - m_EntVec[i].Vel.y;
			}
		}
	}

	//Collision between entities handling
	Vector2 Distance;
	Vector2 V1Along, V1Perp, V2Along, V2Perp;
	bool CollisionAlreadyHandled;
	//Loop around
	for(i=0;i<m_EntVec.size();i++)
	{
		for(j=0;j<m_EntVec.size();j++)
		{
			if(i != j) //don't consider the case of an entity colliding with itself
			{
				Distance = m_EntVec[j].Pos - m_EntVec[i].Pos;
				//If the distance between centers is less than the ray
				if(Distance.Length() < 40)
				{
					//Normalize distance vector to only get direction
					Distance.NormaliseSelf();
					//Check if collision has already been handled
					CollisionAlreadyHandled = false;
					for(k=0;k<m_EntVec[i].Colliders.size();k++)
					{
						if(m_EntVec[i].Colliders[k] == m_EntVec[j].ID)
						{
							CollisionAlreadyHandled = true;
						}
					}
					//If not, handle it
					if(CollisionAlreadyHandled == false)
					{
						//Calculate velocity components for entities along and parallel to line going through centers
						V1Along = Distance*m_EntVec[i].Vel.Dot(Distance);
						V1Perp = m_EntVec[i].Vel - V1Along;
						V2Along = Distance*m_EntVec[j].Vel.Dot(Distance);
						V2Perp = m_EntVec[j].Vel - V2Along;
						//There is a collision. As mass is equal and no energy is lost, solve by swapping velocities along line of collision
						Vector2 Temp = V1Along;
						V1Along = V2Along;
						V2Along = Temp;
						//Recalculate actual velocities for entities
						m_EntVec[i].Vel = V1Along + V1Perp;
						m_EntVec[j].Vel = V2Along + V2Perp;
						//Mark collision as handled
						m_EntVec[i].Colliders.push_back(m_EntVec[j].ID);
						m_EntVec[j].Colliders.push_back(m_EntVec[i].ID);
					}
				}
				//Else check and eventually unmark this collision as handled
				else
				{
					for(k=0;k<m_EntVec[i].Colliders.size();k++)
					{
						if(m_EntVec[i].Colliders[k] == m_EntVec[j].ID)
						{
							m_EntVec[i].Colliders.erase(m_EntVec[i].Colliders.begin()+k);
						}
					}
				}

			}
		}
	}

	//Calculate new positions for objects
	for(i=0;i<m_EntVec.size();i++)
	{
		m_EntVec[i].Pos += (m_EntVec[i].Vel * TimeDelta) + (0.5 * m_EntVec[i].Acc * TimeDelta * TimeDelta);
	}

	//Calculate new velocities for objects
	for(i=0;i<m_EntVec.size();i++)
	{
		m_EntVec[i].Vel += (m_EntVec[i].Acc * TimeDelta);

		//And cap them to a reasonable maximum
		if(m_EntVec[i].Vel.Length() > m_cMaxSpeed)
		{
			m_EntVec[i].Vel.NormaliseSelf();
			m_EntVec[i].Vel = m_EntVec[i].Vel * m_cMaxSpeed;
		}
	}

	//Change acceleration according to user input
	m_EntVec[0].Acc.x = 0.0f;
	m_EntVec[0].Acc.y = 0.0f;
	if(m_bKeys['W']) {m_EntVec[0].Acc.y = -0.0004f;}
	if(m_bKeys['S']) {m_EntVec[0].Acc.y = 0.0004f;}
	if(m_bKeys['A']) {m_EntVec[0].Acc.x = -0.0004f;}
	if(m_bKeys['D']) {m_EntVec[0].Acc.x = 0.0004f;}

	//Send status updates to clients: if more than 100ms have passed since last update
	if(m_NetTimer.HavePassed(100.0f) == true)
	{
		CreateStatUpdatePack();
		for(i=0;i<m_ClientDataVec.size();i++) //for all clients connected
		{
			if(m_ClientDataVec[i].ActivityTimer.HavePassed(2000.0f) == true) //If last still alive message has been received more than 2 secs before
			{
				//Remove client from list
				m_ClientDataVec.erase(m_ClientDataVec.begin()+i);
			}
		}
		for(i=0;i<m_ClientDataVec.size();i++) //for all remaining clients connected
		{
			//Send the new state
			sendto(m_hSocket,m_sNetBuffer,m_uiPacketLength,NULL,(sockaddr*)&m_ClientDataVec[i].Address,sizeof(sockaddr));
		}
	}
}

void AppCore::ProbingLoop(void)
{
	//Timeout: no server on selected address available assumed. Return to idle
	if(m_NetTimer.HavePassed(5000.0f))
	{
		closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;
		m_eGameState = IDLE;
		MessageBox(NULL, TEXT("No server found at specified address"), NULL, MB_ICONERROR | MB_OK);
	}
}

void AppCore::ConnectedLoop(void)
{
	UINT i;

	//Evaluate time elapsed between current and last frame
	m_fTimeElapsedSinceStart = m_SimTimer.GetElapsedMsecs();
	float TimeDelta = m_fTimeElapsedSinceStart - m_fLastFrameTime;
	m_fLastFrameTime = m_fTimeElapsedSinceStart;

	//Collision with boundaries handling
	for(i=0;i<m_EntVec.size();i++)
	{
		if(m_EntVec[i].Pos.x < (m_ClientRec.left + 20))
		{
			if(m_EntVec[i].Vel.x < 0.0f)
			{
				m_EntVec[i].Vel.x = 0.0f - m_EntVec[i].Vel.x;
			}
		}
		if(m_EntVec[i].Pos.x > (m_ClientRec.right - 20))
		{
			if(m_EntVec[i].Vel.x > 0.0f)
			{
				m_EntVec[i].Vel.x = 0.0f - m_EntVec[i].Vel.x;
			}
		}
		if(m_EntVec[i].Pos.y < (m_ClientRec.top + 20))
		{
			if(m_EntVec[i].Vel.y < 0.0f)
			{
				m_EntVec[i].Vel.y = 0.0f - m_EntVec[i].Vel.y;
			}
		}
		if(m_EntVec[i].Pos.y > (m_ClientRec.bottom - 20))
		{
			if(m_EntVec[i].Vel.y > 0.0f)
			{
				m_EntVec[i].Vel.y = 0.0f - m_EntVec[i].Vel.y;
			}
		}
	}

	//Calculate new positions for objects
	for(i=0;i<m_EntVec.size();i++)
	{
		m_EntVec[i].Pos += (m_EntVec[i].Vel * TimeDelta) + (0.5 * m_EntVec[i].Acc * TimeDelta * TimeDelta);
	}

	//Calculate new velocities for objects
	for(i=0;i<m_EntVec.size();i++)
	{
		m_EntVec[i].Vel += (m_EntVec[i].Acc * TimeDelta);

		//And cap them to a reasonable maximum
		if(m_EntVec[i].Vel.Length() > m_cMaxSpeed)
		{
			m_EntVec[i].Vel.NormaliseSelf();
			m_EntVec[i].Vel = m_EntVec[i].Vel * m_cMaxSpeed;
		}
	}
	//Send Alive message every 200ms
	if(m_AliveTimer.HavePassed(200.0f) == true)
	{
		m_AliveTimer.SetTimeBase();
		NetDefs::MsgStillAliveStr Pack;
		sendto(m_hSocket,(char*)&Pack, sizeof(Pack), NULL, (sockaddr*)&m_ServerAddr, sizeof(m_ServerAddr));
	}

	//If no new status update has been received in for a second or more
	if(m_NetTimer.HavePassed(1000.0f) == true)
	{
		closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;
		m_EntVec.clear();
		m_eGameState = IDLE;
		MessageBox(NULL, TEXT("Connection to server lost"), NULL, MB_ICONERROR | MB_OK);
	}
}

void AppCore::DisplayFrame(void)
{
	UINT i;

	BgSpr->Draw(400,300);

	for(i=0;i<m_EntVec.size();i++)
	{
		if(m_EntVec[i].Type == GameEntity::AI)
		{
			AISpr->Draw(UINT(m_EntVec[i].Pos.x),UINT(m_EntVec[i].Pos.y));
		}
	}

	for(i=0;i<m_EntVec.size();i++)
	{
		if(m_EntVec[i].Type == GameEntity::PLAYER)
		{
			PlayerSpr->Draw(UINT(m_EntVec[i].Pos.x),UINT(m_EntVec[i].Pos.y));
		}
	}

	BitBlt(m_hFrontBuffer, m_ClientRec.left, m_ClientRec.top, m_ClientRec.right, m_ClientRec.bottom, m_hBackBuffer, 0, 0, SRCCOPY);
	FillRect(m_hBackBuffer, &m_ClientRec, (HBRUSH)GetStockObject(0));	
}

void AppCore::CreateStatUpdatePack(void)
{
	UINT i;
	m_uiPacketLength = 0;

	//Create header
	NetDefs::MsgStatComHeaderStr Header;
	Header.NumEnts = UCHAR(m_EntVec.size());
	Header.TimeVal = m_fTimeElapsedSinceStart;
	CopyMemory(&m_sNetBuffer[m_uiPacketLength],&Header,sizeof(Header));
	m_uiPacketLength += sizeof(Header);

	//Pack Data
	NetDefs::EntNetDataStr TempEnt;
	for(i=0;i<m_EntVec.size();i++)
	{
		if(m_EntVec[i].Type == GameEntity::PLAYER)
		{
			TempEnt.Type = 0;
		}
		else
		{
			TempEnt.Type = 1;
		}
		TempEnt.ID = m_EntVec[i].ID;
		TempEnt.Pos[0] = m_EntVec[i].Pos.x;
		TempEnt.Pos[1] = m_EntVec[i].Pos.y;
		TempEnt.Vel[0] = m_EntVec[i].Vel.x;
		TempEnt.Vel[1] = m_EntVec[i].Vel.y;
		TempEnt.Acc[0] = m_EntVec[i].Acc.x;
		TempEnt.Acc[1] = m_EntVec[i].Acc.y;

		CopyMemory(&m_sNetBuffer[m_uiPacketLength],&TempEnt,sizeof(TempEnt));
		m_uiPacketLength += sizeof(TempEnt);
	}
}

void AppCore::ReadStatUpdatePack(void)
{
	UINT i;

	//If the packet is more recent than the last received update (to deal with out-of-order reception)
	NetDefs::MsgStatComHeaderStr *Header = (NetDefs::MsgStatComHeaderStr*)&m_sNetBuffer[0];
	if(Header->TimeVal > m_fLastRecvPacketSimTime)
	{
		//Set new comparison time value
		m_fLastRecvPacketSimTime = Header->TimeVal;

		//Clear Previous State
		m_EntVec.clear();
		GameEntity TempEnt;
		NetDefs::EntNetDataStr *TempStr;

		//Read 
		TempStr = (NetDefs::EntNetDataStr*)&m_sNetBuffer[sizeof(NetDefs::MsgStatComHeaderStr)];
		for(i=0;i<Header->NumEnts;i++)
		{
			if(TempStr->Type == 0)
			{
				TempEnt.Type = GameEntity::PLAYER;
			}
			else
			{
				TempEnt.Type = GameEntity::AI;
			}	
			TempEnt.ID = TempStr->ID;
			TempEnt.Pos.x = TempStr->Pos[0];
			TempEnt.Pos.y = TempStr->Pos[1];
			TempEnt.Vel.x = TempStr->Vel[0];
			TempEnt.Vel.y = TempStr->Vel[1];
			TempEnt.Acc.x = TempStr->Acc[0];
			TempEnt.Acc.y = TempStr->Acc[1];
			m_EntVec.push_back(TempEnt);
			TempStr++;
		}

		//Compensate for transmission latency by moving entities forward

		//Calculate new positions for objects
		for(i=0;i<m_EntVec.size();i++)
		{
			m_EntVec[i].Pos += (m_EntVec[i].Vel * m_fServerLatency) + (0.5 * m_EntVec[i].Acc * m_fServerLatency * m_fServerLatency);
		}

		//Calculate new velocities for objects
		for(i=0;i<m_EntVec.size();i++)
		{
			m_EntVec[i].Vel += (m_EntVec[i].Acc * m_fServerLatency);

			//And cap them to a reasonable maximum
			if(m_EntVec[i].Vel.Length() > m_cMaxSpeed)
			{
				m_EntVec[i].Vel.NormaliseSelf();
				m_EntVec[i].Vel = m_EntVec[i].Vel * m_cMaxSpeed;
			}
		}
	}
}