// Copyright (c) 2013-2019 7Mersenne All Rights Reserved.


#include "TestPackets.h"
#include "../Protocol/ProtocolFactory.h"
#include "../Protocol/ServoStaticProtocol.hpp"

FTestPackets::FTestPackets()
{
	OnRegistPackets();
}

FTestPackets::~FTestPackets()
{
}

void FTestPackets::OnRegistPackets()
{
	FProtocolFactory* ProtocolFactory = FProtocolFactory::Get();
	if (ProtocolFactory)
	{
		//ProtocolFactory->RegisterProtocolDeserialize(1002, &(TServoProtocol < FNetBody1exo, ESPMode::ThreadSafe, SPPMode::Queue>::Get()->OnReceivedPacket));
		//ProtocolFactory->RegisterProtocolDeserialize(1003, &(TServoProtocol < FNetBody2exo, ESPMode::ThreadSafe, SPPMode::Queue>::Get()->OnReceivedPacket));
		
		// TODO: use this line with a macro
		// like : __REG_PROTOCOL_DESERIALIZE(UID, TYPE);

		ProtocolFactory->RegisterProtocolDeserialize(10002
			, std::bind(&TServoProtocol < FNetBody1exo, ESPMode::ThreadSafe, SPPMode::Queue>::OnReceivedPacket
				, TServoProtocol < FNetBody1exo, ESPMode::ThreadSafe, SPPMode::Queue>::Get()
				, std::placeholders::_1
				, std::placeholders::_2
				, std::placeholders::_3
				, std::placeholders::_4
			)
		);
		

		
		ProtocolFactory->RegisterProtocolDeserialize(10003
			, std::bind(&TServoProtocol < FNetBody2exo, ESPMode::ThreadSafe, SPPMode::Queue>::OnReceivedPacket
				, TServoProtocol < FNetBody2exo, ESPMode::ThreadSafe, SPPMode::Queue>::Get()
				, std::placeholders::_1
				, std::placeholders::_2
				, std::placeholders::_3
				, std::placeholders::_4
			)
		);

		/*
		ProtocolFactory->RegisterProtocolDeserialize(10002,
			[](FSNetBufferHead & InHead, uint8 * Buffer, int32 BufferSize, int32 & RecivedBytesRead)
		{
			TServoProtocol < FNetBody1exo, ESPMode::ThreadSafe, SPPMode::Queue>::Get()
				->OnReceivedPacket(InHead, Buffer, BufferSize, RecivedBytesRead);
		}
		);

		ProtocolFactory->RegisterProtocolDeserialize(10003, 
			[](FSNetBufferHead & InHead, uint8 * Buffer, int32 BufferSize, int32 & RecivedBytesRead) 
			{
			TServoProtocol < FNetBody2exo, ESPMode::ThreadSafe, SPPMode::Queue>::Get()
					->OnReceivedPacket(InHead, Buffer, BufferSize, RecivedBytesRead);
			}
		);
		*/
	}
}

int32 FNetBody1exo::MemSize()
{
	return sizeof(FNetBody1exo);
}

int32 FNetBody2exo::MemSize()
{
	return sizeof(FNetBody2exo);
}
