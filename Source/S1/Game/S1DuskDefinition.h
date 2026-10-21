#pragma once

#include "CoreMinimal.h"

// Native/editor preview defaults before a definition row is applied. Runtime
// capsule values use the same authored table exported to the authoritative server.
namespace S1DuskDefinition
{
	inline constexpr uint32 MonsterDefinitionId = 1;
	inline constexpr float CapsuleRadiusCm = 28.f;
	inline constexpr float CapsuleHalfHeightCm = 62.f;

	static_assert(CapsuleRadiusCm == 28.f);
	static_assert(CapsuleHalfHeightCm == 62.f);
}
