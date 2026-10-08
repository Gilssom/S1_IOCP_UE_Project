#if WITH_DEV_AUTOMATION_TESTS
#include "Game/S1MyPlayer.h"
#include "Combat/S1KnightCombatComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "World/S1WorldEntitySubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "Protocol.pb.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1KnightQApprovalTest,
    "S1.Combat.KnightQApproval", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FS1KnightQApprovalTest::RunTest(const FString& Parameters)
{
    Protocol::S_DAMAGE_RESULT Hit;
    Hit.set_action_instance_id(123);
    Hit.set_hit_index(1);
    Hit.set_damage(38);
    std::string Wire;
    TestTrue(TEXT("Damage packet serializes"), Hit.SerializeToString(&Wire));
    Protocol::S_DAMAGE_RESULT Parsed;
    TestTrue(TEXT("Damage packet parses"), Parsed.ParseFromString(Wire));
    TestEqual(TEXT("Second hit preserved on wire"), Parsed.hit_index(), 1u);
    TestEqual(TEXT("Server result is not locally recalculated"), Parsed.damage(), 38u);

    UClass* DefaultsClass = LoadClass<AS1MyPlayer>(nullptr, TEXT("/Game/Blueprints/BP_MyPlayer.BP_MyPlayer_C"));
    UAnimMontage* Montage = LoadObject<UAnimMontage>(nullptr,
        TEXT("/Game/Blueprints/Characters/AM_Terra_Q_AttackMontage.AM_Terra_Q_AttackMontage"));
    FObjectPropertyBase* Charge = FindFProperty<FObjectPropertyBase>(US1KnightCombatComponent::StaticClass(), TEXT("ChargeAnimation"));
    if (!TestNotNull(TEXT("Player defaults"), DefaultsClass) || !TestNotNull(TEXT("Q montage"), Montage) ||
        !TestNotNull(TEXT("Q assignment property"), Charge)) return false;
    TestTrue(TEXT("Server 3.1s duration matches authored Q"), FMath::IsNearlyEqual(Montage->GetPlayLength(), 3.1f, 0.001f));
    TestEqual(TEXT("Q montage playback scale"), Montage->RateScale, 1.f);
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    if (!TestNotNull(TEXT("Test world"), World)) return false;
    GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
    FActorSpawnParameters Spawn;
    Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AS1MyPlayer* Player = World->SpawnActor<AS1MyPlayer>(AS1MyPlayer::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Spawn);
    if (TestNotNull(TEXT("Native test player"), Player))
    {
        const AS1MyPlayer* Defaults = CastChecked<AS1MyPlayer>(DefaultsClass->GetDefaultObject());
        Player->GetMesh()->SetAnimInstanceClass(UAnimInstance::StaticClass());
        Player->GetMesh()->SetSkeletalMeshAsset(Defaults->GetMesh()->GetSkeletalMeshAsset());
        Player->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        US1KnightCombatComponent* Combat = Player->GetKnightCombatComponent();
        Charge->SetObjectPropertyValue_InContainer(Combat, Montage);
        Combat->OnActionStarted.AddLambda([Player](ES1KnightCombatAction Action, int32 Step, float Yaw)
        {
            Player->HandleKnightActionStarted(Action, Step, Yaw);
        });
        const float Speed = Player->GetCharacterMovement()->MaxWalkSpeed;
        TestTrue(TEXT("Q predicts immediately"), Combat->RequestChargePresentation(45.f));
        const uint32 First = Combat->GetCurrentPredictionSequence();
        TestEqual(TEXT("Prediction registered"), First, 1u);
        TestFalse(TEXT("Jump prohibited while Q plays"), Player->CanJump());
        TestEqual(TEXT("Q does not reduce movement speed"), Player->GetCharacterMovement()->MaxWalkSpeed, Speed);
        Player->AddMovementInput(FVector::ForwardVector, 1.f, true);
        TestFalse(TEXT("Movement input is retained"), Player->GetPendingMovementInputVector().IsNearlyZero());
        TestFalse(TEXT("Repeated Q cannot restart"), Combat->RequestChargePresentation(90.f));
        TestEqual(TEXT("Repeated Q does not turn actor"), Player->GetActorRotation().Yaw, 45.0);
        Protocol::S_COMBAT_ACTION_RESULT Result;
        Result.set_action_type(Protocol::COMBAT_ACTION_KNIGHT_Q);
        Result.set_sequence(First);
        Result.set_combo_step(0);
        Result.set_decision(Protocol::COMBAT_DECISION_REJECTED);
        Result.set_reason(Protocol::COMBAT_REJECT_ACTION_BLOCKED);
        // Expected diagnostic: rejection is deliberately exercised by this test.
        AddExpectedError(TEXT("Q rejected."), EAutomationExpectedErrorFlags::Contains, 2);
        Player->ApplyCombatActionResult(Result);
        TestEqual(TEXT("Rejection releases action"), Combat->GetActionState(), ES1KnightActionState::Ready);
        TestFalse(TEXT("Rejected montage stops"), Player->GetMesh()->GetAnimInstance()->Montage_IsActive(Montage));
        TestTrue(TEXT("Next input is immediately usable"), Combat->RequestChargePresentation(90.f));
        const uint32 OlderPending = Combat->GetCurrentPredictionSequence();
        Combat->InterruptByServer();
        TestTrue(TEXT("New independent Q starts"), Combat->RequestChargePresentation(180.f));
        const uint32 Current = Combat->GetCurrentPredictionSequence();
        Result.set_sequence(OlderPending);
        Player->ApplyCombatActionResult(Result);
        TestEqual(TEXT("Late old rejection preserves current Q"), Combat->GetCurrentPredictionSequence(), Current);
        TestEqual(TEXT("Current Q stays active"), Combat->GetActionState(), ES1KnightActionState::ChargePresentation);
        Result.set_sequence(Current);
        Result.set_decision(Protocol::COMBAT_DECISION_ACCEPTED);
        Result.set_action_instance_id(456);
        Player->ApplyCombatActionResult(Result);
        TestEqual(TEXT("Approval does not restart or replace prediction"), Combat->GetCurrentPredictionSequence(), Current);
        Player->ResetPredictedCombat(true);
        TestEqual(TEXT("Session reset clears Q predictions"), Player->PredictedQActions.Num(), 0);
        TestEqual(TEXT("Session reset clears action"), Combat->GetActionState(), ES1KnightActionState::Ready);

        // Exercise the real HP consumer with two hits of one action, not just protobuf fields.
        US1WorldEntitySubsystem* Entities = NewObject<US1WorldEntitySubsystem>(World);
        Entities->Players.Add(400, Player);
        Protocol::S_DAMAGE_RESULT Damage;
        Damage.set_action_instance_id(567);
        Damage.set_source_object_id(501);
        Damage.set_target_object_id(400);
        Damage.set_hit_index(0);
        Damage.set_room_event_sequence(10);
        Damage.set_damage(25);
        Damage.set_current_hp(175);
        Damage.set_max_hp(200);
        TestTrue(TEXT("First hit updates HP"), Entities->ApplyDamageResult(Damage));
        Damage.set_room_event_sequence(11);
        TestFalse(TEXT("Same hit with another event number still deduplicates"), Entities->ApplyDamageResult(Damage));
        Damage.set_hit_index(1);
        Damage.set_room_event_sequence(12);
        Damage.set_damage(38);
        Damage.set_current_hp(137);
        TestTrue(TEXT("Second hit of same action is not lost"), Entities->ApplyDamageResult(Damage));
        UTextRenderComponent* Label = Player->FindComponentByClass<UTextRenderComponent>();
        if (TestNotNull(TEXT("HP label"), Label))
            TestEqual(TEXT("Authoritative HP after both hits"), Label->Text.ToString(), FString(TEXT("Knight 137/200")));
        Damage.set_hit_index(0);
        Damage.set_room_event_sequence(10);
        Damage.set_current_hp(175);
        TestFalse(TEXT("Old first hit cannot restore old HP"), Entities->ApplyDamageResult(Damage));
        Entities->ResetCombatImpactState();
        TestEqual(TEXT("Session clears damage dedup state"), Entities->RecentDamageKeys.Num(), 0);
        Entities->Players.Reset();
        Combat->OnActionStarted.Clear();
        Player->Destroy();
    }
    World->DestroyWorld(false);
    GEngine->DestroyWorldContext(World);
    return !HasAnyErrors();
}
#endif
