// Copyright (c) 2013-2019 7Mersenne All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/Private/HAL/PThreadRunnableThread.h"
#include "Networking.h"
#include "Interface/SocketInterface.h"

#ifndef __TELNET_HEART_BEAT_DECLARE__
#define __TELNET_HEART_BEAT_DECLARE__
// telnet NOP 0xFF 0xF1
const unsigned char __TELNET_HEART_BEAT__[2] = { 255, 241 };
#endif // !__TELNET_HEART_BEAT_DECLARE__

#ifndef __SOCKET_BUFF_LENTH__
#define __SOCKET_BUFF_LENTH__ 1024
#endif // !__SOCKET_BUFF_LENTH__

/**
 * Socket Sender
 * the socket ptr is handled by RecvThread
 * SenderThread just use to async call for socket.send
 */
class SEPTEMSERVO_API FClientRecvThread : public FRunnable
{
public:
	FClientRecvThread();
	FClientRecvThread(ISocketInterface* InSocketInterface, FIPv4Endpoint& InServerEndPoint);
	~FClientRecvThread();

	// Begin FRunnable interface.
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;
	// End FRunnable interface
	
protected:
	//=========== Socket Begin  ==============//
	// weak handle value
	ISocketInterface* SocketInterface;
	TArray<uint8> ReceivedData;
	//=========== Socket End  ==============//

	//=========== Server Settings Begin  ==============//
	FIPv4Endpoint ServerEndPoint;

	int32 Syncword;
	//=========== Server Settings End	 ==============//

	//---------------------------------------------
	// thread control
	//---------------------------------------------

	/** If true, the thread should exit. */
	TAtomic<bool> TimeToDie;
	FThreadSafeCounter LifecycleStep;
};
