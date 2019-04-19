// Copyright (c) 2013-2019 7Mersenne All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/*
* Servo Static Net packet Body Base struct
*/
#pragma pack(push, 1)
struct SEPTEMSERVO_API FStaticNetBodyBase
{
	FStaticNetBodyBase() {}

	FStaticNetBodyBase(FStaticNetBodyBase&& InBase)
	{}

	virtual ~FStaticNetBodyBase() {};

	/**
	 * Reset the values
	 */
	virtual void Reset();

	/**
	 * Serializes the Servo Net packet Body Base value into the buffer *Data .
	 * this:: The value to serialize.
	 *
	 * @param Data the ptr of the buffer
	 * @param BufferSize the max size of the buffer cache
	 * @param OutSize return the size write into the buffer
	 * @return true If serialize succeed, or false means data buffer write overflow
	 */
	virtual bool Serialize(uint8* Data, int32 BufferSize, int32& OutSize);

	/**
	 * Deserializes the Servo Net packet Body Base value from the buffer *Data .
	 * this:: The value to deserialize.
	 *
	 * @param Data the ptr of the buffer
	 * @param BufferSize the max size of the buffer cache
	 * @return true If serialize succeed, or false means data buffer read overflow
	 */
	virtual bool Deserialize(uint8* Data, int32 BufferSize);

	/**
	 * Get the memory size of the class inherit from Servo Net packet Body Base
	 * use sizeof by default
	 *
	 * @return the size
	 */
	virtual int32 MemSize();

	/**
	 * sigma xor {every byte}
	 *
	 * @return xor
	 */
	virtual uint8 XOR();
};
#pragma pack(pop)