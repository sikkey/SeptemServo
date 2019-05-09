// Copyright (c) 2013-2019 7Mersenne All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
//#include "EngineUtils.h"
#include "Networking.h"
#include "../Protocol/ServoProtocol.h"
#include "../Protocol/ServoStaticProtocol.hpp"
#include "Modules/ModuleManager.h"

class FSeptemServoModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/**
	 * Get the singleton of Protocol
	 * Defend create 2 singleton from different .so/.dll modules
	 * @return the singleton of FServoProtocol  
	 */
	static FServoProtocol* ProtocolSingleton();

	/**
	 * Get the singleton of TProtocol<Packet_Class>
	 * Defend create 2 singleton from different .so/.dll modules
	 * @return the singleton of TProtocol<Packet_Class>
	 */
	template<typename T, ESPMode PtrMode = ESPMode::ThreadSafe, SPPMode PoolMode = SPPMode::Fast>
	static TServoProtocol<T, PtrMode, PoolMode> TProtocolSingleton();
};

template<typename T, ESPMode PtrMode, SPPMode PoolMode>
inline TServoProtocol<T, PtrMode, PoolMode> FSeptemServoModule::TProtocolSingleton()
{
	return TServoProtocol<T, PtrMode, PoolMode>::Get();
}
