// Copyright (c) 2013-2019 7Mersenne All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/Private/HAL/PThreadRunnableThread.h"
#include "Networking.h"
#include "Interface/SocketInterface.h"
#include "ClientRecvThread.h"

#ifndef __TELNET_HEART_BEAT_DECLARE__
#define __TELNET_HEART_BEAT_DECLARE__
// telnet NOP 0xFF 0xF1
const unsigned char __TELNET_HEART_BEAT__[2] = { 255, 241 };
#endif // !__TELNET_HEART_BEAT_DECLARE__

#ifndef __SOCKET_BUFF_LENTH__
#define __SOCKET_BUFF_LENTH__ 1024
#endif // !__SOCKET_BUFF_LENTH__

/**
 * Client thread
 * Handle socket*
 *
 */
class SEPTEMSERVO_API FClientThread : public FRunnable, public ISocketInterface
{
public:
	FClientThread();
	~FClientThread();

	// Begin FRunnable interface.
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;
	// End FRunnable interface

	//=========== Client Socket Begin  ==============//
	void ConnectToServer();
	void Disconnect();
	void ReleaseSocket();
	void SendByte(uint8 InByte);
	void SendNop();
	void SendBuffer(uint8* InBuffer, int32 InBufferLength, bool bCopy = false);

	/*
	void SendByteUDP(uint8 InByte);
	void SendNopUDP();
	*/

	virtual FSocket* GetSocket() override;
	//=========== Client Socket End  ==============//
protected:
	//=========== Server Settings Begin  ==============//
	FIPv4Endpoint ServerEndPoint;
	//=========== Server Settings End	 ==============//

	//=========== Client Begin  ==============//
	FSocket* ClientSocket;
	/**
	* 0: disconnect
	* 1: connect
	*/
	int32 ClientState = 0;

	TArray<uint8> m_SendBuffer;
	FClientRecvThread* ClientRecvThread;
	//=========== Client End  ==============//

	//---------------------------------------------
	// thread control
	//---------------------------------------------

	/** If true, the thread should exit. */
	TAtomic<bool> TimeToDie;
	FThreadSafeCounter LifecycleStep;

	// Client thread
	FRunnableThread* Thread;

public:
	static FClientThread* Create(FIPv4Endpoint InServerEndPoint);

	/**
	* Tells the thread to exit. If the caller needs to know when the thread
	* has exited, it should use the bShouldWait value and tell it how long
	* to wait before deciding that it is deadlocked and needs to be destroyed.
	* NOTE: having a thread forcibly destroyed can cause leaks in TLS, etc.
	*
	* @return True if the thread exited graceful, false otherwise
	*/
	bool KillThread();// use KillThread instead of thread->kill
};

