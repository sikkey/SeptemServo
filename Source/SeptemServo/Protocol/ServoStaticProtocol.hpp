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
 * PtrMode::	the thread-safe mode of TSharedPtr
 * PoolMode:: the mdoe of pools
 */
template<typename T, ESPMode PtrMode = ESPMode::ThreadSafe, SPPMode PoolMode = SPPMode::Fast>
class SEPTEMSERVO_API TServoProtocol
{
protected:
	static TServoProtocol<T, PtrMode, PoolMode> * pSingleton;
	static FCriticalSection mCriticalSection;

	int32 Syncword;
	// force to push/pop TSharedPtr
	TNetPacketPool<TSNetPacket<T> , PtrMode>* PacketPool;
	int32 PacketPoolCount;
	Septem::TSharedRecyclePool<TSNetPacket<T> , PtrMode> RecyclePool;
private:
	TServoProtocol()
		:Syncword(DEFAULT_SYNCWORD_INT32)
		, PacketPool(nullptr)
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
	static TServoProtocol<T, PtrMode, PoolMode>* Get();
	// thread safe; singleton will init when first call getRef()
	static TServoProtocol<T, PtrMode, PoolMode>& GetRef();

	// danger call, but fast
	static TServoProtocol<T, PtrMode, PoolMode>* Singleton();
	// danger call, but fast
	static TServoProtocol<T, PtrMode, PoolMode>& SingletonRef();

	// push recv packet into packet pool
	bool Push(const TSharedPtr<TSNetPacket<T>, PtrMode>& InNetPacket);
	// pop from packet pool
	bool Pop(TSharedPtr<TSNetPacket<T>, PtrMode>& OutNetPacket);
	int32 PacketPoolNum();

	//=========================================
	//		Net Packet Pool Memory Management
	//=========================================
	static int32 RecyclePoolMaxnum;

	// please call ReUse or set value manulity after recycle alloc
	TSharedPtr< TSharedPtr<TSNetPacket<T>, PtrMode> AllocNetPacket();
	// recycle dealloc
	void DeallockNetPacket(const TSharedPtr < TSNetPacket<T>& InSharedPtr, bool bForceRecycle = false);
	int32 RecyclePoolNum();
};

template<typename T, ESPMode PtrMode, SPPMode PoolMode>
TServoProtocol<T, PtrMode, PoolMode>* TServoProtocol<T, PtrMode, PoolMode>::pSingleton = nullptr;

template<typename T, ESPMode PtrMode, SPPMode PoolMode>
FCriticalSection TServoProtocol<T, PtrMode, PoolMode>::mCriticalSection;

template<typename T, ESPMode PtrMode, SPPMode PoolMode>
int32 TServoProtocol<T, PtrMode, PoolMode>::RecyclePoolMaxnum = 0;

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

template<typename T, ESPMode PtrMode, SPPMode PoolMode>
inline TServoProtocol<T, PtrMode, PoolMode>* TServoProtocol<T, PtrMode, PoolMode>::Get()
{
	if (nullptr == pSingleton) {
		FScopeLock lockSingleton(&mCriticalSection);
		pSingleton = new TServoProtocol<T, PtrMode, PoolMode>();
	}

	return pSingleton;
}

template<typename T, ESPMode PtrMode, SPPMode PoolMode>
inline TServoProtocol<T, PtrMode, PoolMode>& TServoProtocol<T, PtrMode, PoolMode>::GetRef()
{
	if (nullptr == pSingleton) {
		FScopeLock lockSingleton(&mCriticalSection);
		pSingleton = new TServoProtocol<T, PtrMode, PoolMode>();
	}

	return *pSingleton;
}

template<typename T, ESPMode PtrMode, SPPMode PoolMode>
inline TServoProtocol<T, PtrMode, PoolMode>* TServoProtocol<T, PtrMode, PoolMode>::Singleton()
{
	return pSingleton;
}

template<typename T, ESPMode PtrMode, SPPMode PoolMode>
inline TServoProtocol<T, PtrMode, PoolMode>& TServoProtocol<T, PtrMode, PoolMode>::SingletonRef()
{
	check(pSingleton);
	return *pSingleton;
}

template<typename T, ESPMode PtrMode, SPPMode PoolMode>
inline bool TServoProtocol<T, PtrMode, PoolMode>::Push(const TSharedPtr<TSNetPacket<T>, PtrMode>& InNetPacket)
{
	if (PacketPool->Push(InNetPacket))
	{
		++PacketPoolCount;
		return true;
	}

	return false;
}

template<typename T, ESPMode PtrMode, SPPMode PoolMode>
inline bool TServoProtocol<T, PtrMode, PoolMode>::Pop(TSharedPtr<TSNetPacket<T>, PtrMode>& OutNetPacket)
{
	if (PacketPool->Pop(OutNetPacket))
	{
		--PacketPoolCount;
		return true;
	}

	return false;
}

template<typename T, ESPMode PtrMode, SPPMode PoolMode>
inline int32 TServoProtocol<T, PtrMode, PoolMode>::PacketPoolNum()
{
	return PacketPoolCount;
}

template<typename T, ESPMode PtrMode, SPPMode PoolMode>
inline TSharedPtr < TSharedPtr<TSNetPacket<T>, PtrMode> TServoProtocol<T, PtrMode, PoolMode>::AllocNetPacket()
{
	return RecyclePool.Alloc();
}

template<typename T, ESPMode PtrMode, SPPMode PoolMode>
inline void TServoProtocol<T, PtrMode, PoolMode>::DeallockNetPacket(const TSharedPtr < TSNetPacket<T>& InSharedPtr, bool bForceRecycle = false)
{
	if (!InSharedPtr.IsValid())
		return;

	InSharedPtr->OnDealloc();
	if (bForceRecycle)
	{
		RecyclePool.DeallocForceRecycle(InSharedPtr);
	}
	else
	{
		RecyclePool.Dealloc(InSharedPtr);
	}
}

template<typename T, ESPMode PtrMode, SPPMode PoolMode>
inline int32 TServoProtocol<T, PtrMode, PoolMode>::RecyclePoolNum()
{
	return RecyclePool.Num();
}
