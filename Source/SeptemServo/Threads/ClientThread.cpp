// Copyright (c) 2013-2019 7Mersenne All Rights Reserved.


#include "ClientThread.h"

FClientThread::FClientThread()
{
	ClientSocket = nullptr;
	ClientState = 0;
	ClientRecvThread = nullptr;
	TimeToDie = false;
	LifecycleStep.Set(0);
	bRepeatConnect = true;
}

FClientThread::~FClientThread()
{
	if (LifecycleStep.GetValue() == 2)
	{
		UE_LOG(LogTemp, Display, TEXT("FClientThread destruct: cannot exit safe"));
	}

	if (nullptr != ClientRecvThread)
	{
		delete ClientRecvThread;
		ClientRecvThread = nullptr;
	}

	// cleanup thread
	if (nullptr != Thread)
	{
		delete Thread;
		Thread = nullptr;
	}

	if (nullptr != ClientSocket)
	{
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ClientSocket);
		ClientSocket = nullptr;
	}
		
}

bool FClientThread::Init()
{
	LifecycleStep.Set(1);

	ConnectToServer();

	return true;
}

uint32 FClientThread::Run()
{
	LifecycleStep.Set(2);
	UE_LOG(LogTemp, Display, TEXT("FClientThread: running begin \n"));
	while (!TimeToDie)
	{
		if (ClientRecvThread)
		{
			//UE_LOG(LogTemp, Display, TEXT("FClientThread: running gogogoogo \n"));
		}
		else {
			if (bRepeatConnect)
			{
				ConnectToServer();
			}
		}
	}
	return 0U;
}

void FClientThread::Stop()
{
	TimeToDie = true;
}

void FClientThread::Exit()
{
	LifecycleStep.Set(3);
	ReleaseSocket();
	LifecycleStep.Set(4);
}

void FClientThread::ConnectToServer()
{
	if (nullptr == ClientSocket)
	{
		// 1. create socket
		ClientSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("ClientThread"), false);

		// 2. connect socket
		TSharedRef<FInternetAddr> ServerAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr(ServerEndPoint.Address.Value, ServerEndPoint.Port);
		bool bConnect = ClientSocket->Connect(*ServerAddr);
		UE_LOG(LogTemp, Display, TEXT("FClientThread: connect to %s \n"), *ServerAddr->ToString(true));
		if (!bConnect)
		{
			UE_LOG(LogTemp, Display, TEXT("FClientThread: connect to server failed! \n"));
			ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ClientSocket);
			ClientSocket = nullptr;
		}
		else {
			ClientState = 1;

			// create recv thread
			ClientRecvThread = FClientRecvThread::Create(this, ServerEndPoint);
		}
	}
}

void FClientThread::Disconnect()
{
	ReleaseSocket();
}

void FClientThread::ReleaseSocket()
{
	if (nullptr != ClientRecvThread)
	{
		// block call to close recv thread
		ClientRecvThread->KillThread();
		delete ClientRecvThread;
		ClientRecvThread = nullptr;
	}

	if (nullptr != ClientSocket)
	{
		ClientSocket->Shutdown(ESocketShutdownMode::ReadWrite);
		ClientSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ClientSocket);
		ClientSocket = nullptr;
		ClientState = 0;
		UE_LOG(LogTemp, Display, TEXT("FClientThread: client socket close \n"));
	}
}

void FClientThread::SendByte(uint8 InByte)
{
	if (ClientSocket)
	{
		int32 bytesSend = 0;

		static uint8 buffer[2] = { 0 };
		buffer[0] = InByte;

		if (ClientSocket->Send(buffer, 1, bytesSend))
		{
			UE_LOG(LogTemp, Display, TEXT("FSocketSenderThread: socket send  %d bytes done. buffer num = %d \n"), bytesSend, 1);
		}
	}
}



void FClientThread::SendNop()
{
	if (ClientSocket)
	{
		int32 bytesSend = 0;

		if (ClientSocket->Send(__TELNET_HEART_BEAT__, 2, bytesSend))
		{
			UE_LOG(LogTemp, Display, TEXT("FSocketSenderThread: socket send  %d bytes done. buffer num = %d \n"), bytesSend, 2);
		}
	}
}

void FClientThread::SendBuffer(uint8 * InBuffer, int32 InBufferLength, bool bCopy)
{
	if (ClientSocket)
	{
		int32 bytesSend = 0;

		if (bCopy)
		{
			m_SendBuffer.SetNumUninitialized(InBufferLength);
			FMemory::Memcpy(m_SendBuffer.GetData(), InBuffer, InBufferLength);
			if (ClientSocket->Send(m_SendBuffer.GetData(), InBufferLength, bytesSend))
			{
				UE_LOG(LogTemp, Display, TEXT("FSocketSenderThread: socket send  %d bytes done. buffer num = %d \n"), bytesSend, 2);
			}
		}
		else {
			if (ClientSocket->Send(InBuffer, InBufferLength, bytesSend))
			{
				UE_LOG(LogTemp, Display, TEXT("FSocketSenderThread: socket send  %d bytes done. buffer num = %d \n"), bytesSend, 2);
			}
		}
	}
}

/*
void FClientThread::SendByteUDP(uint8 InByte)
{
	if (ClientSocket)
	{
		int32 bytesSend = 0;

		static uint8 buffer[2] = { 0 };
		buffer[0] = InByte;

		TSharedRef<FInternetAddr> ServerAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr(ServerEndPoint.Address.Value, ServerEndPoint.Port);

		if (ClientSocket->SendTo(buffer, 1, bytesSend, *ServerAddr))
		{
			UE_LOG(LogTemp, Display, TEXT("ATestClientNoThreadActor: socket udp send done. \n"));
		}
	}
}

void FClientThread::SendNopUDP()
{
	if (ClientSocket)
	{
		int32 bytesSend = 0;

		if (ClientSocket->Send(__TELNET_HEART_BEAT__, 2, bytesSend))
		{
			UE_LOG(LogTemp, Display, TEXT("FSocketSenderThread: socket send  %d bytes done. buffer num = %d \n"), bytesSend, 2);
		}
	}
}
*/

FSocket * FClientThread::GetSocket()
{
	return ClientSocket;
}

FClientThread * FClientThread::Create(FIPv4Endpoint InServerEndPoint)
{
	// create runnable
	FClientThread* runnable = new FClientThread();
	runnable->ServerEndPoint = InServerEndPoint;

	// create thread with runnable
	FRunnableThread* thread = FRunnableThread::Create(runnable, TEXT("FClientThread"), 0, TPri_BelowNormal); //windows default = 8mb for thread, could specify 

	if (nullptr == thread)
	{
		// create failed
		delete runnable;
		return nullptr;
	}

	// setting thread
	runnable->Thread = thread;
	return runnable;
}

/**
 * Tells the thread to exit. If the caller needs to know when the thread
 * has exited, it should use the bShouldWait value and tell it how long
 * to wait before deciding that it is deadlocked and needs to be destroyed.
 * NOTE: having a thread forcibly destroyed can cause leaks in TLS, etc.
 *
 * @return True if the thread exited graceful, false otherwise
 */
bool FClientThread::KillThread()
{
	bool bDidExit = true;

	TimeToDie = true;

	if (nullptr != Thread)
	{
		if (ClientRecvThread)
		{
			ClientRecvThread->KillThread();
		}

		// Trigger the thread so that it will come out of the wait state if
		// it isn't actively doing work
		//if(event) event->Trigger();

		Stop();

		// If waiting was specified, wait the amount of time. If that fails,
		// brute force kill that thread. Very bad as that might leak.
		Thread->WaitForCompletion();	//block call

		// Clean up the event
		// if(event) FPlatformProcess::ReturnSynchEventToPool(event);
		// event = nullptr;

		// here will call Stop()
		delete Thread;
		Thread = nullptr;

		// socket had been safe release in exit();
		if (nullptr != ClientSocket)
		{
			ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ClientSocket);
			ClientSocket = nullptr;
		}
	}

	return bDidExit;
}
