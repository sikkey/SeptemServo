// Copyright (c) 2013-2019 7Mersenne All Rights Reserved.

#include "ClientRecvThread.h"
#include "../Protocol/ServoProtocol.h"
#include "../Protocol/ServoStaticProtocol.hpp"
#include "../Protocol/ProtocolFactory.h"

FClientRecvThread::FClientRecvThread()
{
	SocketInterface = 0;

	ReceivedData.Reset(__SOCKET_BUFF_LENTH__);

	LifecycleStep.Set(0);
	TimeToDie = false;
	Syncword = DEFAULT_SYNCWORD_INT32;
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
	FServoProtocol* ServoProtocol = FServoProtocol::Get();
	FProtocolFactory* ProtocolFactory = FProtocolFactory::Get();
	FSNetBufferHead PacketHead;
	PacketHead.syncword = Syncword;

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

				int32 TotalBytesRead = 0;
				int32 RecivedBytesRead = 0;
				int32 HeadIndex = 0;
				while (TotalBytesRead < BytesRead)
				{

					//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
					// New version of protocol, use buffer body and template class body
					//------------------------------------------------------------------------------------------------------------------------------------------------------------------------

					// 1. use syncword to find the index of PacketHead
					HeadIndex = Septem::BufferBufferSyncword(ReceivedData.GetData() + TotalBytesRead, BytesRead - TotalBytesRead, Syncword);

					if (-1 == HeadIndex)
					{
						// failed to find syncword in the whole buffer
						TotalBytesRead = BytesRead;
						break;
					}

					// 2. discard buffer before syncword
					TotalBytesRead += HeadIndex;

					// 3. read head @ ReceivedData.GetData() + TotalBytesRead
					if (!PacketHead.MemRead(ReceivedData.GetData() + TotalBytesRead, BytesRead - TotalBytesRead))
					{
						// failed to find syncword in the whole buffer
						TotalBytesRead += FSNetBufferHead::MemSize();
						break;
					}

					TotalBytesRead += FSNetBufferHead::MemSize();

					// 4. select PacketHead.version for deserialize packet body
					if (PacketHead.IsSerializedPacket() && ProtocolFactory->IsProtocolRegister(PacketHead.uid))
					{
						// serialiezed packet
						ProtocolFactory->CallProtocolDeserializeWithoutCheck(PacketHead, ReceivedData.GetData() + TotalBytesRead, ReceivedData.Num() - TotalBytesRead, RecivedBytesRead);
						TotalBytesRead += RecivedBytesRead;
					}
					else {
						// buffer packet
						TSharedPtr<FSNetPacket, ESPMode::ThreadSafe> pPacket(ServoProtocol->AllocNetPacket());
						pPacket->ReUse(PacketHead, ReceivedData.GetData() + TotalBytesRead, ReceivedData.Num() - TotalBytesRead, RecivedBytesRead);
						TotalBytesRead += RecivedBytesRead;
						FPlatformMisc::MemoryBarrier();
						UE_LOG(LogTemp, Display, TEXT("FConnectThread: write bytes %d, total write bytes %d \n"), RecivedBytesRead, TotalBytesRead);

						if (pPacket->IsValid())
						{
							ServoProtocol->Push(pPacket);
						}
						else {
							// packet is illegal, dealloc shared pointer
							ServoProtocol->DeallockNetPacket(pPacket);
						}
					}
				}
			}
			else
			{
				//Error: pendingDataSize>0 Rcev failed
				UE_LOG(LogTemp, Display, TEXT("FConnectThread: pending data size > 0, rcev failed. Check the length of ReceivedData.Num()"));
			}
		}
	}

	// ExitCode:0 means no error
	return 0;
}

void FClientRecvThread::Stop()
{
	LifecycleStep.Set(0);
}

void FClientRecvThread::Exit()
{
	LifecycleStep.Set(3);
}


