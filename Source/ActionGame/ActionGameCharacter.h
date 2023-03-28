// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "InputActionValue.h"
#include "Abilities/GameplayAbility.h"
#include "ActionGameTypes.h"
#include "ActionGameCharacter.generated.h"

class UAG_AbilitySystemComponentBase;
class UAG_AttributeSetBase;

class UGameplayEffect;
class UGameplayAbility;

class UAG_MotionWarpingComponent;
class UAG_CharacterMovementComponent;
class UInventoryComponent;

UCLASS(config=Game)
class AActionGameCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

public:
	AActionGameCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;

	bool ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> Effect, FGameplayEffectContextHandle InEffectContext);

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PawnClientRestart() override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Input)
	float TurnRate;

protected:
	void GiveAbilities();
	void ApplyStartupEffects();

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	UPROPERTY(EditDefaultsOnly)
	UAG_AbilitySystemComponentBase* AbilitySystemComponent;

	UPROPERTY(Transient)
	UAG_AttributeSetBase* AttributeSet;
			
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MotionWarp")
	UAG_MotionWarpingComponent* AGMotionWarpingComponent;

	UAG_CharacterMovementComponent* AGCharacterMovementComponent;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE FCharacterData GetCharacterData() { return CharacterData; }

	FORCEINLINE UAG_MotionWarpingComponent* GetAGMotionWarpingComponent() { return AGMotionWarpingComponent; }

	UFUNCTION(BlueprintCallable)
	void SetCharacterData(const FCharacterData& InCharacterData);

	FORCEINLINE class UFootstepsComponent* GetFootstepsComponent() const { return FootstepsComponent; }

	void OnMaxMovementSpeedChanged(const FOnAttributeChangeData& Data);

	FORCEINLINE UInventoryComponent* GetInventoryComponent() { return InventoryComponent; }

protected:
	UPROPERTY(ReplicatedUsing = OnRep_CharacterData)
	FCharacterData CharacterData;

	UFUNCTION()
	void OnRep_CharacterData();

	virtual void InitFromCharacterData(const FCharacterData& InCharacterData, bool bFromReplication = false);

	UPROPERTY(EditDefaultsOnly)
	class UCharacterDataAsset* CharacterDataAsset;

	UPROPERTY(BlueprintReadOnly)
	UFootstepsComponent* FootstepsComponent;

	// Enhanced Input

	UPROPERTY(EditDefaultsOnly)
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly)
	class UInputAction* MoveForwardInputAction;

	UPROPERTY(EditDefaultsOnly)
	UInputAction* MoveSideInputAction;

	UPROPERTY(EditDefaultsOnly)
	UInputAction* TurnInputAction;

	UPROPERTY(EditDefaultsOnly)
	UInputAction* LookUpInputAction;

	UPROPERTY(EditDefaultsOnly)
	UInputAction* JumpInputAction;

	UPROPERTY(EditDefaultsOnly)
	UInputAction* CrouchInputAction;

	UPROPERTY(EditDefaultsOnly)
	UInputAction* SprintInputAction;

	UPROPERTY(EditDefaultsOnly)
	UInputAction* DropItemInputAction;

	UPROPERTY(EditDefaultsOnly)
	UInputAction* EquipNextInputAction;

	UPROPERTY(EditDefaultsOnly)
	UInputAction* UnequipInputAction;

	void OnMoveForwardAction(const FInputActionValue& Value);
	void OnMoveSideAction(const FInputActionValue& Value);
	void OnTurnAction(const FInputActionValue& Value);
	void OnLookUpAction(const FInputActionValue& Value);
	void OnJumpActionStarted(const FInputActionValue& Value);
	void OnJumpActionStopped(const FInputActionValue& Value);
	void OnCrouchActionStarted(const FInputActionValue& Value);
	void OnCrouchActionStopped(const FInputActionValue& Value);
	void OnSprintActionStarted(const FInputActionValue& Value);
	void OnSprintActionStopped(const FInputActionValue& Value);
	void OnDropItemTriggered(const FInputActionValue& Value);
	void OnEquipNextTriggered(const FInputActionValue& Value);
	void OnUnequipTriggered(const FInputActionValue& Value);

	// Gameplay Events
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag JumpEventTag;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer InAirTags;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer CrouchTags;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer SprintTags;

	// Gameplay Effects
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> CrouchStateEffect;

	FDelegateHandle MaxMovementSpeedChangedDelegateHandle;

	// Inventory

	UPROPERTY(EditAnywhere, Replicated)
	UInventoryComponent* InventoryComponent = nullptr;

};

