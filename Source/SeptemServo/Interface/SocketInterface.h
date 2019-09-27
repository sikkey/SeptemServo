// Copyright (c) 2013-2019 7Mersenne All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Networking.h"

/**
 * 
 */
class SEPTEMSERVO_API ISocketInterface
{
public:
	virtual FSocket* GetSocket() = 0;
};
