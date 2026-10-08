#pragma once

#include "CoreMinimal.h"

// Client mirror of GameServer/GameServer/DuskDefinition.h. The external IOCP
// server remains authoritative; these values only configure its presentation.
namespace S1DuskDefinition
{
	inline constexpr uint32 MonsterDefinitionId = 1;
	inline constexpr float CapsuleRadiusCm = 28.f;
	inline constexpr float CapsuleHalfHeightCm = 62.f;

	static_assert(CapsuleRadiusCm == 28.f);
	static_assert(CapsuleHalfHeightCm == 62.f);
}
