// Copyright (c) 2013-2019 7Mersenne All Rights Reserved.


#include "ProtocolFactory.h"
#include "Core/Public/Misc/ScopeLock.h"

FProtocolFactory::FProtocolFactory()
{
	check(pSingleton == nullptr && "singleton can't create 2 object!");
	pSingleton = this;
}

FProtocolFactory::~FProtocolFactory()
{
	pSingleton = nullptr;
}

FProtocolFactory * FProtocolFactory::Get()
{
	if (nullptr == pSingleton) {
		FScopeLock lockSingleton(&mCriticalSection);
		pSingleton = new FProtocolFactory();
	}

	return pSingleton;
}

FProtocolFactory & FProtocolFactory::GetRef()
{
	if (nullptr == pSingleton) {
		FScopeLock lockSingleton(&mCriticalSection);
		pSingleton = new FProtocolFactory();
	}

	return *pSingleton;
}

FProtocolFactory * FProtocolFactory::Singleton()
{
	check(pSingleton && "singleton doesn't exist!");
	return pSingleton;
}

FProtocolFactory & FProtocolFactory::SingletonRef()
{
	check(pSingleton && "singleton doesn't exist!");
	return *pSingleton;
}

bool FProtocolFactory::RegisterProtocolDeserialize(int32 InUid, std::function<void (FSNetBufferHead&, uint8*, int32, int32&)> && InLambda)
{
	auto itr = ProtocolDeserializeDelegates.insert(std::pair<int32, std::function<void (FSNetBufferHead&, uint8*, int32, int32&)> >(InUid, InLambda));
	return itr.second;
}

void FProtocolFactory::CallProtocolDeserialize(FSNetBufferHead & InHead, uint8 * Buffer, int32 BufferSize, int32 & RecivedBytesRead)
{
	auto itr = ProtocolDeserializeDelegates.find(InHead.uid);
	
	if (itr == ProtocolDeserializeDelegates.end())
	{
		// not find uid class
		UE_LOG(LogTemp, Display, TEXT("FProtocolFactory: cannot find packet class with uid = %d, please register it in FProtocolFactory first. \n"), InHead.uid);
	}
	else {
		itr->second(InHead, Buffer, BufferSize, RecivedBytesRead);
	}
}

void FProtocolFactory::CallProtocolDeserializeWithoutCheck(FSNetBufferHead & InHead, uint8 * Buffer, int32 BufferSize, int32 & RecivedBytesRead)
{
	ProtocolDeserializeDelegates[InHead.uid](InHead, Buffer, BufferSize, RecivedBytesRead);
}

bool FProtocolFactory::IsProtocolRegister(int32 InUid)
{
	auto itr = ProtocolDeserializeDelegates.find(InUid);
	return itr != ProtocolDeserializeDelegates.end();
}

FProtocolFactory* FProtocolFactory::pSingleton = nullptr;
FCriticalSection FProtocolFactory::mCriticalSection;
