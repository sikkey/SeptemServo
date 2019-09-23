// Copyright (c) 2013-2019 7Mersenne All Rights Reserved.


#include "ClientThread.h"

FClientThread::FClientThread()
{
	ClientSocket = nullptr;
	ClientState = 0;
}

FClientThread::~FClientThread()
{
}

bool FClientThread::Init()
{
	return true;
}

uint32 FClientThread::Run()
{
	return 0U;
}

void FClientThread::Stop()
{
}

void FClientThread::Exit()
{
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

		if (!bConnect)
		{
			UE_LOG(LogTemp, Display, TEXT("FClientThread: connect to server failed! \n"));
			ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ClientSocket);
			ClientSocket = nullptr;
		}
		else {
			ClientState = 1;
		}
	}
}

void FClientThread::Disconnect()
{
	ReleaseSocket();
}

void FClientThread::ReleaseSocket()
{
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
			m_SendBuffer.SetNumZeroed(InBufferLength);
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
