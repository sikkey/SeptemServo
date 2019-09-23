// Copyright (c) 2013-2019 7Mersenne All Rights Reserved.

#include "ClientRecvThread.h"

FClientRecvThread::FClientRecvThread()
{
	SocketInterface = 0;

	ReceivedData.Reset(__SOCKET_BUFF_LENTH__);

	LifecycleStep.Set(0);
	TimeToDie = false;
}

FClientRecvThread::FClientRecvThread(ISocketInterface * InSocketInterface, FIPv4Endpoint & InServerEndPoint)
{
	SocketInterface = InSocketInterface;
	ServerEndPoint = InServerEndPoint;

	ReceivedData.Reset(__SOCKET_BUFF_LENTH__);

	LifecycleStep.Set(0);
	TimeToDie = false;
}

FClientRecvThread::~FClientRecvThread()
{
}

bool FClientRecvThread::Init()
{
	LifecycleStep.Set(1);
	return true;
}

uint32 FClientRecvThread::Run()
{
	LifecycleStep.Set(2);
	uint32 pendingDataSize = 0;
	int32 BytesRead = 0;
	bool bRcev = false;

	while (!TimeToDie)
	{
		FPlatformProcess::Sleep(0.01f);
		FSocket* socket = SocketInterface->GetSocket();
		if (socket && socket->HasPendingData(pendingDataSize) && pendingDataSize > 0)
		{
			BytesRead = 0;
			bRcev = false;

			// read all buffer
			ReceivedData.SetNumUninitialized(pendingDataSize);
			// Attention: ReceivedData.Num must >= pendingDataSize
			bRcev = socket->Recv(ReceivedData.GetData(), ReceivedData.Num(), BytesRead);

			if (bRcev && BytesRead > 0)
			{
				if (BytesRead > ReceivedData.Num())
				{
					//stack overflow
					UE_LOG(LogTemp, Display, TEXT("[Warnning]FConnectThread: receive stack overflow!\n"));
					continue;
				}

				// TODO: recv data for while every syncword
				UE_LOG(LogTemp, Display, TEXT("FConnectThread: receive byte = %d length = %d\n"), ReceivedData.GetData()[0], ReceivedData.Num());
			}
		}
	}

	// ExitCode:0 means no error
	return 0;
}

void FClientRecvThread::Stop()
{
}

void FClientRecvThread::Exit()
{
	LifecycleStep.Set(3);
}


