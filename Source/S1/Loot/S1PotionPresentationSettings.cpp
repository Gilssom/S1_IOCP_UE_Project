#include "Loot/S1PotionPresentationSettings.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "NiagaraSystem.h"
#include "UObject/ConstructorHelpers.h"

US1PotionPresentationSettings::US1PotionPresentationSettings()
{
 WorldMesh = FSoftObjectPath(TEXT("/Game/LootSet/Meshes/Potions/potion_large02.potion_large02"));
 Icon = FSoftObjectPath(TEXT("/Game/_Resources/Rair_Potion.Rair_Potion"));
 DropEffect = FSoftObjectPath(TEXT("/Game/DrapEffet/VFX/NE_drop_effects01.NE_drop_effects01"));
 HealingEffect=FSoftObjectPath(TEXT("/Game/VFX_StatusEffect01/FXS/FXS_healing000.FXS_healing000"));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Game/LootSet/Meshes/Potions/potion_large02.potion_large02"));
 static ConstructorHelpers::FObjectFinder<UTexture2D> IconAsset(TEXT("/Game/_Resources/Rair_Potion.Rair_Potion"));
 static ConstructorHelpers::FObjectFinder<UNiagaraSystem> EffectAsset(TEXT("/Game/DrapEffet/VFX/NE_drop_effects01.NE_drop_effects01"));
 CookDefaultMesh=MeshAsset.Object;
 CookDefaultIcon=IconAsset.Object;
 CookDefaultEffect=EffectAsset.Object;
 static ConstructorHelpers::FObjectFinder<UNiagaraSystem> HealingAsset(TEXT("/Game/VFX_StatusEffect01/FXS/FXS_healing000.FXS_healing000"));
 CookDefaultHealingEffect=HealingAsset.Object;
}
