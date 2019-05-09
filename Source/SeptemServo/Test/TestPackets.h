// Copyright (c) 2013-2019 7Mersenne All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "../Protocol/StaticNetBodyBase.h"

// TODO: implement Test packets
#pragma pack(push, 1)
struct SEPTEMSERVO_API FExoRobo16f
{
	FVector hipLocation;
	FVector hipDirection;
	float angle[10];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SEPTEMSERVO_API FNetBody1exo : public FStaticNetBodyBase
{
	// uid : 10002
	//==============
	// int32 roboData.size
	// int32 roboData.frameId
	// int32 roboData.length
	// FVector hipLocation
	// FVector hipDirection
	// 10 angle
	int32 size;
	int32 frameId;
	int32 length;
	FVector hipLocation;
	FVector hipDirection;
	float angle[10];

	virtual int32 MemSize() override;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SEPTEMSERVO_API FNetBody2exo : public FStaticNetBodyBase
{
	// uid : 10003
	//==============
	// int32 roboData.size
	// int32 roboData.frameId
	// int32 roboData.length
	// FVector hipLocation
	// FVector hipDirection
	// float [10] angle
	// FVector hipLocation
	// FVector hipDirection
	// float [10] angle
	int32 size;
	int32 frameId;
	int32 length;
	FExoRobo16f robo[2];

	virtual int32 MemSize() override;
};
#pragma pack(pop)

/**
 * 
 */
class SEPTEMSERVO_API FTestPackets
{
public:
	FTestPackets();
	~FTestPackets();

	void OnRegistPackets();
};
