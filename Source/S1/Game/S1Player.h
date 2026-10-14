// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Protocol.pb.h"
#include "S1Player.generated.h"

class UAbilitySystemComponent;
class US1KnightCombatComponent;
class UTextRenderComponent;
class US1EntityHealthComponent;

UCLASS()
class S1_API AS1Player : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AS1Player(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual ~AS1Player();

protected:
	virtual void BeginPlay() override;
	virtual void NotifyControllerChanged() override;
	virtual void Tick(float DeltaTime) override;

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	US1KnightCombatComponent* GetKnightCombatComponent() const { return KnightCombatComponent; }

	virtual FVector GetVelocity() const override;
	bool IsMyPlayer() const;

	Protocol::MoveState GetMoveState() { return PlayerInfo->state(); }
	void SetMoveState(Protocol::MoveState State);

public:
	void SetPlayerInfo(const Protocol::PosInfo& Info);
	void SetDestInfo(const Protocol::PosInfo& Info);
	void ApplyServerHealth(uint32 CurrentHp, uint32 MaxHp, bool bDead);
	Protocol::PosInfo* GetPlayerInfo() { return PlayerInfo; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<US1KnightCombatComponent> KnightCombatComponent;

	UPROPERTY(VisibleAnywhere, Category = "Combat|Presentation")
	TObjectPtr<UTextRenderComponent> HealthLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|HP")
	TObjectPtr<US1EntityHealthComponent> EntityHealth;
	UPROPERTY(EditDefaultsOnly, Category="Combat|HP")
	bool bShowHealthDebugText = false;

	void ApplyRemoteAnimationMoveState(Protocol::MoveState State);

	class Protocol::PosInfo* PlayerInfo; // 현재 위치
	class Protocol::PosInfo* DestInfo;	// 목적지
	FVector RemoteVisualVelocity = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, Category = "Network|Movement")
	float RemoteLocationInterpSpeed = 12.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Network|Movement")
	float RemoteRotationInterpSpeed = 15.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Network|Movement")
	float RemoteSnapDistance = 300.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Network|Movement")
	float RemoteArrivalTolerance = 2.0f;
};
