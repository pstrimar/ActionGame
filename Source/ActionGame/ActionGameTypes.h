// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionGameTypes.generated.h"

USTRUCT(BlueprintType)
struct FCharacterData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS")
	TArray<TSubclassOf<class UGameplayEffect>> Effects;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS")
	TArray<TSubclassOf<class UGameplayAbility>> Abilities;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
	class UCharacterAnimDataAsset* CharacterAnimDataAsset;
};

USTRUCT(BlueprintType)
struct FCharacterAnimationData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly)
	class UBlendSpace* MovementBlendSpace = nullptr;

	UPROPERTY(EditDefaultsOnly)
	class UAnimSequenceBase* IdleAnimationAsset = nullptr;

	UPROPERTY(EditDefaultsOnly)
	class UBlendSpace* CrouchMovementBlendSpace = nullptr;

	UPROPERTY(EditDefaultsOnly)
	class UAnimSequenceBase* CrouchIdleAnimationAsset = nullptr;
};

UENUM(BlueprintType)
enum class EFoot : uint8
{
	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right")
};

UCLASS(BlueprintType, Blueprintable)
class UItemStaticData : public UObject 
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName Name;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class AItemActor> ItemActorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName AttachmentSocket = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bCanBeEquipped = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FCharacterAnimationData CharacterAnimationData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSubclassOf<class UGameplayAbility>> GrantedAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSubclassOf<UGameplayEffect>> OngoingEffects;
};

UENUM(BlueprintType)
enum class EItemState : uint8
{
	None UMETA(DisplayName = "None"),
	Equipped UMETA(DisplayName = "Equipped"),
	Dropped UMETA(DisplayName = "Dropped")
};

UENUM(BlueprintType)
enum class EMovementDirectionType : uint8
{
	None UMETA(DisplayName = "None"),
	OrientToMovement UMETA(DisplayName = "OrientToMovement"),
	Strafe UMETA(DisplayName = "Strafe"),
};


UCLASS(BlueprintType, Blueprintable)
class UWeaponStaticData : public UItemStaticData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class UGameplayEffect> DamageEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USkeletalMesh* SkeletalMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UStaticMesh* StaticMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float FireRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USoundBase* AttackSound;
};

UCLASS(BlueprintType, Blueprintable)
class UProjectileStaticData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DamageRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float GravityMultiplyer = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float InitialSpeed = 3000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxSpeed = 3000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UStaticMesh* StaticMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSubclassOf<UGameplayEffect>> Effects;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TEnumAsByte<EObjectTypeQuery>> RadialDamageQueryTypes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TEnumAsByte<ETraceTypeQuery> RadialDamageTraceType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UNiagaraSystem* OnStopVFX =  nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USoundBase* OnStopSFX = nullptr;
};