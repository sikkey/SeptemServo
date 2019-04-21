// Copyright (c) 2013-2019 7Mersenne All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StaticNetBodyBase.h"

/**
* Servo Net packet Buffer Wrapper
* Wrapper a obj , which means a serialized class object packet body
 * Ensure T is inherit from FStaticNetBodyBase
 */
#pragma pack(push, 1)
template<typename T>
struct SEPTEMSERVO_API TSNetBufferWrapper
{
public:
	T obj;

	TSNetBufferWrapper()
		:obj()
	{
	}

	TSNetBufferWrapper(T&& InObj)
		:obj(InObj)
	{
	}

	/**
	 * Reset the values
	 */
	void Reset()
	{
		FStaticNetBodyBase* base = &T;
		base->Reset();
	}

	/**
	 * Serializes the Servo Net packet Body Base value into the buffer *Data .
	 * this:: The value to serialize.
	 *
	 * @param Data the ptr of the buffer
	 * @param BufferSize the max size of the buffer cache
	 * @param OutSize return the size write into the buffer
	 * @return true If serialize succeed, or false means data buffer write overflow
	 */
	bool Serialize(uint8* Data, int32 BufferSize, int32& OutSize)
	{
		FStaticNetBodyBase* base = &T;
		return base->Serialize(Data, BufferSize, int32& OutSize);
	}

	/**
	 * Deserializes the Servo Net packet Body Base value from the buffer *Data .
	 * this:: The value to deserialize.
	 *
	 * @param Data the ptr of the buffer
	 * @param BufferSize the max size of the buffer cache
	 * @return true If serialize succeed, or false means data buffer read overflow
	 */
	bool Deserialize(uint8* Data, int32 BufferSize)
	{
		FStaticNetBodyBase* base = &T;
		return base->Deserialize(Data, BufferSize);
	}

	/**
	 * Get the memory size of the class inherit from Servo Net packet Body Base
	 * use sizeof by default
	 *
	 * @return the size
	 */
	virtual int32 MemSize()
	{
		FStaticNetBodyBase* base = &T;
		return base->MemSize();
	}

	/**
	 * wapper T.XOR()
	 *
	 * @return xor
	 */
	uint8 XOR()
	{
		FStaticNetBodyBase* base = &T;
		return base->XOR();
	}
};
#pragma pack(pop)
