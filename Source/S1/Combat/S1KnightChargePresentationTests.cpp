#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/S1KnightCombatComponent.h"
#include "Game/S1Player.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequenceBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1KnightChargePresentationTest,
	"S1.Combat.KnightChargePresentation", EAutomationTestFlags::EditorContext |
	EAutomationTestFlags::EngineFilter)

bool FS1KnightChargePresentationTest::RunTest(const FString& Parameters)
{
	UAnimMontage* Montage = LoadObject<UAnimMontage>(nullptr,
		TEXT("/Game/Blueprints/Characters/AM_Terra_Q_AttackMontage.AM_Terra_Q_AttackMontage"));
	FObjectPropertyBase* ChargeProperty = FindFProperty<FObjectPropertyBase>(
		US1KnightCombatComponent::StaticClass(), TEXT("ChargeAnimation"));
	if (!TestNotNull(TEXT("Q montage loads"), Montage) ||
		!TestNotNull(TEXT("Existing ChargeAnimation property"), ChargeProperty))
	{
		return false;
	}
	if (!TestEqual(TEXT("Q has one slot track"), Montage->SlotAnimTracks.Num(), 1))
	{
		return false;
	}
	TestEqual(TEXT("Q uses the Terra AnimBP slot"),
		Montage->SlotAnimTracks[0].SlotName, FName(TEXT("UpperBody")));
	TestTrue(TEXT("Q automatically blends out"), Montage->bEnableAutoBlendOut);

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("Transient test world"), World))
	{
		return false;
	}
	GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);

	const TCHAR* PlayerClasses[] = {
		TEXT("/Game/Blueprints/BP_MyPlayer.BP_MyPlayer_C"),
		TEXT("/Game/Blueprints/BP_Player.BP_Player_C")
	};
	for (const TCHAR* ClassPath : PlayerClasses)
	{
		UClass* PlayerClass = LoadClass<AS1Player>(nullptr, ClassPath);
		if (!TestNotNull(ClassPath, PlayerClass))
		{
			continue;
		}
		const AS1Player* PlayerDefaults = CastChecked<AS1Player>(PlayerClass->GetDefaultObject());
		UObject* ConfiguredCharge = ChargeProperty->GetObjectPropertyValue_InContainer(
			PlayerDefaults->GetKnightCombatComponent());
		TestEqual(TEXT("Local and remote blueprint Q assignment"),
			ConfiguredCharge, static_cast<UObject*>(Montage));
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AS1Player* Player = World->SpawnActor<AS1Player>(
			AS1Player::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
		if (!TestNotNull(TEXT("Test player spawns"), Player))
		{
			continue;
		}
		US1KnightCombatComponent* Combat = Player->GetKnightCombatComponent();
		ChargeProperty->SetObjectPropertyValue_InContainer(Combat, ConfiguredCharge);

		// Copy the saved Blueprint settings into a native test actor. Set the
		// test animation class before the mesh to avoid initializing the production
		// AnimBP in a synthetic world with no controller, map, or server connection.
		Player->GetMesh()->SetAnimInstanceClass(UAnimInstance::StaticClass());
		Player->GetMesh()->SetSkeletalMeshAsset(PlayerDefaults->GetMesh()->GetSkeletalMeshAsset());
		UAnimInstance* Anim = Player->GetMesh()->GetAnimInstance();
		if (!TestNotNull(TEXT("Test animation instance"), Anim))
		{
			Player->Destroy();
			continue;
		}
		Player->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		const bool bOriginalOrient = Player->GetCharacterMovement()->bOrientRotationToMovement;
		int32 SentActions = 0;
		Combat->OnActionStarted.AddLambda(
			[&SentActions](ES1KnightCombatAction Action, int32 Step, float Yaw)
			{
				++SentActions;
			});
		TestTrue(TEXT("Authored Q starts"), Combat->RequestChargePresentation(45.0f));
		TestTrue(TEXT("Authored montage is actually playing"), Anim->Montage_IsPlaying(Montage));
		TestFalse(TEXT("Second Q does not restart the action"), Combat->RequestChargePresentation(90.0f));
		TestEqual(TEXT("One Q action request"), SentActions, 1);
		Combat->InterruptByServer();
		TestEqual(TEXT("Interrupt releases action"), Combat->GetActionState(), ES1KnightActionState::Ready);
		TestFalse(TEXT("Interrupt stops authored Q"), Anim->Montage_IsActive(Montage));
		TestEqual(TEXT("Interrupt restores movement rotation"),
			Player->GetCharacterMovement()->bOrientRotationToMovement, bOriginalOrient);

		TestTrue(TEXT("Accepted remote Q starts"),
			Combat->PlayReplicatedAction(ES1KnightCombatAction::Charge, 0, 1, 90.0f));
		TestTrue(TEXT("Remote Q plays the same montage"), Anim->Montage_IsPlaying(Montage));
		TestEqual(TEXT("Remote playback sends no action request"), SentActions, 1);
		TestFalse(TEXT("Duplicate remote Q does not replay"),
			Combat->PlayReplicatedAction(ES1KnightCombatAction::Charge, 0, 1, 90.0f));
		Combat->InterruptByServer();

		UAnimSequenceBase* Sequence = LoadObject<UAnimSequenceBase>(nullptr,
			TEXT("/Game/ParagonTerra/Characters/Heroes/Terra/Animations/Q_swing_B.Q_swing_B"));
		if (TestNotNull(TEXT("Legacy Q sequence loads"), Sequence))
		{
			ChargeProperty->SetObjectPropertyValue_InContainer(Combat, Sequence);
			TestTrue(TEXT("Legacy sequence remains playable"), Combat->RequestChargePresentation(0.0f));
			TestTrue(TEXT("Legacy Q uses a dynamic montage"), Anim->DynamicMontage_IsPlayingFrom(Sequence));
			Combat->InterruptByServer();
			TestFalse(TEXT("Interrupt stops legacy Q"), Anim->DynamicMontage_IsPlayingFrom(Sequence));
		}
		Combat->OnActionStarted.Clear();
		Player->Destroy();
	}
	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return !HasAnyErrors();
}

#endif
