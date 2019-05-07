// Copyright (c) 2013-2019 7Mersenne All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ServoStaticProtocol.hpp"

// STL
#include<map>
#include<functional>

/**
 * Protocol Factory
 * feature 1. register Template Static Protocol for TClass packet pools
 * feature 2. Produce packet of Template Static Protocols, set into their pools
 */
class SEPTEMSERVO_API FProtocolFactory
{
public:
	FProtocolFactory();
	virtual ~FProtocolFactory();

	// thread safe; singleton will init when first call get()
	static FProtocolFactory* Get();
	// thread safe; singleton will init when first call getRef()
	static FProtocolFactory& GetRef();

	// danger call, but fast
	static FProtocolFactory* Singleton();
	// danger call, but fast
	static FProtocolFactory& SingletonRef();

protected:
	static FProtocolFactory* pSingleton;
	static FCriticalSection mCriticalSection;

	//------------------------------------------------------------------------------------------------------------
	// lamda deserialize packet
	//------------------------------------------------------------------------------------------------------------
public:
	bool RegisterProtocolDeserialize(int32 InUid, std::function< void(FSNetBufferHead&, uint8*, int32, int32&)>&& InLambda);
	void CallProtocolDeserialize(FSNetBufferHead& InHead, uint8* Buffer, int32 BufferSize, int32&RecivedBytesRead);
	void CallProtocolDeserializeWithoutCheck(FSNetBufferHead& InHead, uint8* Buffer, int32 BufferSize, int32&RecivedBytesRead);
	bool IsProtocolRegister(int32 InUid);
protected:
	std::map < int32, std::function< void(FSNetBufferHead&, uint8*, int32, int32&)> > ProtocolDeserializeDelegates;
};