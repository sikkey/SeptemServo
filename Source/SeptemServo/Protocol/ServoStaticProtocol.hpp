// Copyright (c) 2013-2019 7Mersenne All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ServoProtocol.h"
#include "NetBufferWrapper.hpp"

/**
*	Template Servo Net Packet
*	Head + TSNetBufferWrapper<T> + Foot
*/
template<typename T>
struct SEPTEMSERVO_API TSNetPacket
{
	FSNetBufferHead Head;
	TSNetBufferWrapper<T> Body;
	FSNetBufferFoot Foot;

	// session id
	int32 sid;
	bool bFastIntegrity;

	bool IsValid();

	// check data integrity with fastcode
	static bool FastIntegrity(uint8* DataPtr, int32 DataLength, uint8 fastcode)
	{
		uint8 xor = 0;
		for (int32 i = 0; i < DataLength; ++i)
		{
			xor ^= DataPtr[i];
		}
		return xor == fastcode;
	}

	bool CheckIntegrity();

	TSNetPacket()
		:sid(0)
		, bFastIntegrity(false)
	{}

	/*
	* param BytesRead return the count of bytes been read. if BytesRead > BufferSize means failed
	*/
	TSNetPacket(uint8* Data, int32 BufferSize, int32& BytesRead, int32 InSyncword = DEFAULT_SYNCWORD_INT32)
	{
		Head.syncword = InSyncword;
		// 1. find syncword for head
		int32 index = BufferBufferSyncword(Data, BufferSize, Head.syncword);
		uint8 fastcode = 0;

		if (-1 == index)
		{
			// failed to find syncword in the whole buffer
			BytesRead = BufferSize+1;
			return;
		}

		// 2. read head
		if (!Head.MemRead(Data + index, BufferSize - index))
		{
			// failed to read from the rest buffer
			BytesRead = BufferSize+1;
			return;
		}

		index += FSNetBufferHead::MemSize();
		fastcode ^= Head.XOR();

		if (0 == Head.uid)
		{
			sid = Head.SessionID();
		}

		// 3. check and read body

		if (0 != Head.uid)
		{
			// it is not a heart beat packet
			//if (!Body.MemRead(Data + index, BufferSize - index, Head.size))
			if(!Body.Deserialize(Data + index, BufferSize - index))
			{
				// failed to read from the rest buffer
				BytesRead = BufferSize + 1;
				return;
			}
			index += Body.MemSize();
			fastcode ^= Body.XOR();
		}

		// 4. read foot
		if (!Foot.MemRead(Data + index, BufferSize - index))
		{
			// failed to read from the rest buffer
			BytesRead = BufferSize + 1;
			return;
		}

		index += FSNetBufferFoot::MemSize();
		fastcode ^= Foot.XOR();

		BytesRead = index;
		bFastIntegrity = 0 == fastcode;

		sid = Head.SessionID();

		return;
	}

	uint64 GetTimestamp();

	// static class template not need heartbeat
	//static TSNetPacket* CreateHeartbeat(int32 InSyncword = DEFAULT_SYNCWORD_INT32);
	void ReUse(uint8* Data, int32 BufferSize, int32& BytesRead, int32 InSyncword = DEFAULT_SYNCWORD_INT32);
	void WriteToArray(TArray<uint8>& InBufferArr);
	void OnDealloc();
	void OnAlloc();
	bool operator < (TSNetPacket<T> && Other);
};

/**
 * T::		the class  handle in TSNetPacket<T>
 * SingletonPtrMode::	the thread-safe mode of pSingleton
 */
template<typename T, ESPMode SingletonPtrMode = ESPMode::Fast, SPPMode PoolMode = SPPMode::Fast>
class SEPTEMSERVO_API TServoProtocol
{
protected:
	static TServoProtocol<T, SingletonPtrMode, PoolMode> * pSingleton;
	static FCriticalSection mCriticalSection;

	int32 Syncword;
	// force to push/pop TSharedPtr
	TNetPacketPool<TSNetPacket<T> , ESPMode::ThreadSafe>* PacketPool;
	int32 PacketPoolCount;
	Septem::TSharedRecyclePool<TSNetPacket<T> , ESPMode::ThreadSafe> RecyclePool;
private:
	TServoProtocol()
		:Syncword(DEFAULT_SYNCWORD_INT32)
		, PacketPoolCount(0)
		, RecyclePool(MAX_NETPACKET_IN_POOL)
	{
		check(pSingleton == nullptr && "Protocol singleton can't create 2 object!");
		pSingleton = this;
		
		switch (PoolMode)
		{
		case SPPMode::Stack :
			PacketPool = new TNetPacketStack<FSNetPacket, ESPMode::ThreadSafe>();
			break;
		case SPPMode::Heap :
			PacketPool = new TNetPacketHeap<FSNetPacket, ESPMode::ThreadSafe>();
			break;
		default:
			PacketPool = new TNetPacketQueue<FSNetPacket, ESPMode::ThreadSafe>();
			break;
		}
	}

public:
	virtual ~TServoProtocol()
	{
		pSingleton = nullptr;
		delete PacketPool;
	}

	// thread safe; singleton will init when first call get()
	static TServoProtocol<T, SingletonPtrMode, PoolMode>* Get();
	// thread safe; singleton will init when first call getRef()
	static TServoProtocol<T, SingletonPtrMode, PoolMode>& GetRef();
};

template<typename T, ESPMode SingletonPtrMode, SPPMode PoolMode>
TServoProtocol<T, SingletonPtrMode, PoolMode>* TServoProtocol<T, SingletonPtrMode, PoolMode>::pSingleton = nullptr;

template<typename T, ESPMode SingletonPtrMode, SPPMode PoolMode>
FCriticalSection TServoProtocol<T, SingletonPtrMode, PoolMode>::mCriticalSection;

template<typename T>
inline bool TSNetPacket<T>::IsValid()
{
	return bFastIntegrity && Head.size == Body.MemSize();
}

template<typename T>
inline bool TSNetPacket<T>::CheckIntegrity()
{
	if (0 == Head.uid)
	{
		return bFastIntegrity = (Head.XOR() ^ Foot.XOR()) == Head.fastcode;
	}
	return bFastIntegrity = (Head.XOR() ^ Body.XOR() ^ Foot.XOR()) == Head.fastcode;
}

template<typename T>
inline uint64 TSNetPacket<T>::GetTimestamp()
{
	return Foot.timestamp;
}

template<typename T>
inline void TSNetPacket<T>::ReUse(uint8 * Data, int32 BufferSize, int32 & BytesRead, int32 InSyncword)
{
	sid = 0;
	bFastIntegrity = false;

	Head.syncword = InSyncword;
	// 1. find syncword for head
	int32 index = BufferBufferSyncword(Data, BufferSize, Head.syncword);
	uint8 fastcode = 0;

	if (-1 == index)
	{
		// failed to find syncword in the whole buffer
		BytesRead = BufferSize;
		return;
	}

	// 2. read head
	if (!Head.MemRead(Data + index, BufferSize - index))
	{
		// failed to read from the rest buffer
		BytesRead = BufferSize;
		return;
	}

	index += FSNetBufferHead::MemSize();
	fastcode ^= Head.XOR();

	if (0 == Head.uid)
	{
		sid = Head.SessionID();
	}

	// 3. check and read body
	if (0 != Head.uid)
	{
		//if (!Body.MemRead(Data + index, BufferSize - index, Head.size))
		if (!Body.Deserialize(Data + index, BufferSize - index))
		{
			// failed to read from the rest buffer
			BytesRead = BufferSize;
			return;
		}
		index += Body.MemSize();
		fastcode ^= Body.XOR();
	}

	// 4. read foot
	if (!Foot.MemRead(Data + index, BufferSize - index))
	{
		// failed to read from the rest buffer
		BytesRead = BufferSize;
		return;
	}

	index += FSNetBufferFoot::MemSize();
	fastcode ^= Foot.XOR();

	BytesRead = index;
	bFastIntegrity = fastcode == 0;

	sid = Head.SessionID();

	return;
}

template<typename T>
inline void TSNetPacket<T>::WriteToArray(TArray<uint8>& InBufferArr)
{
	int32 BytesWrite = 0;
	int32 writeSize = sizeof(FSNetBufferHead) + sizeof(FSNetBufferFoot);
	if (Head.uid > 0ui16)
	{
		writeSize += Body.MemSize();
	}

	InBufferArr.SetNumZeroed(writeSize);
	uint8* DataPtr = InBufferArr.GetData();

	//1. write heads
	FMemory::Memcpy(DataPtr, &Head, FSNetBufferHead::MemSize());
	BytesWrite += sizeof(FSNetBufferHead);

	//2. write Body
	if (Head.uid != 0)
	{
		//FMemory::Memcpy(DataPtr + BytesWrite, Body.bufferPtr, Body.length);
		int32 outSize = 0;
		bool bSuccessSerialize = Body.Serialize(DataPtr + BytesWrite, InBufferArr.Num() - BytesWrite, outSize);

		check(bSuccessSerialize);

		BytesWrite += outSize;
	}

	//3. write Foot
	FMemory::Memcpy(DataPtr + BytesWrite, &Foot, FSNetBufferFoot::MemSize());
	BytesWrite += FSNetBufferFoot::MemSize();
}

template<typename T>
inline void TSNetPacket<T>::OnDealloc()
{
	sid = 0;
	bFastIntegrity = false;
	Body.Reset();
	Head.size = 0;
}

template<typename T>
inline void TSNetPacket<T>::OnAlloc()
{
	Head.Reset();
	Foot.Reset();

	// timestamp set now 
	Foot.SetNow();
}

template<typename T>
inline bool TSNetPacket<T>::operator<(TSNetPacket<T>&& Other)
{
	return Foot.timestamp < Other.Foot.timestamp;
}

template<typename T, ESPMode SingletonPtrMode, SPPMode PoolMode>
inline TServoProtocol<T, SingletonPtrMode, PoolMode>* TServoProtocol<T, SingletonPtrMode, PoolMode>::Get()
{
	if (nullptr == pSingleton) {
		FScopeLock lockSingleton(&mCriticalSection);
		pSingleton = new TServoProtocol<T, SingletonPtrMode, PoolMode>();
	}

	return pSingleton;
}

template<typename T, ESPMode SingletonPtrMode, SPPMode PoolMode>
inline TServoProtocol<T, SingletonPtrMode, PoolMode>& TServoProtocol<T, SingletonPtrMode, PoolMode>::GetRef()
{
	if (nullptr == pSingleton) {
		FScopeLock lockSingleton(&mCriticalSection);
		pSingleton = new TServoProtocol<T, SingletonPtrMode, PoolMode>();
	}

	return *pSingleton;
}
