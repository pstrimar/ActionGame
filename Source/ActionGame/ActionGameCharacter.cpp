// Copyright Epic Games, Inc. All Rights Reserved.

#include "ActionGameCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/AttributeSets/AG_AttributeSetBase.h"
#include "DataAssets/CharacterDataAsset.h"
#include "AbilitySystem/Components/AG_AbilitySystemComponentBase.h"

#include "Net/UnrealNetwork.h"

#include "ActorComponents/AG_CharacterMovementComponent.h"
#include "ActorComponents/AG_MotionWarpingComponent.h"
#include "ActorComponents/FootstepsComponent.h"
#include "ActorComponents/InventoryComponent.h"
#include "AbilitySystemLog.h"


//////////////////////////////////////////////////////////////////////////
// AActionGameCharacter

AActionGameCharacter::AActionGameCharacter(const FObjectInitializer& ObjectInitializer) : 
	Super(ObjectInitializer.SetDefaultSubobjectClass<UAG_CharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// set our turn rate for input
	TurnRate = 50.f;

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	AGCharacterMovementComponent = Cast<UAG_CharacterMovementComponent>(GetCharacterMovement());

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	// Ability System

	AbilitySystemComponent = CreateDefaultSubobject<UAG_AbilitySystemComponentBase>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxMovementSpeedAttribute()).AddUObject(this, &AActionGameCharacter::OnMaxMovementSpeedChanged);

	AttributeSet = CreateDefaultSubobject<UAG_AttributeSetBase>(TEXT("AttributeSet"));

	FootstepsComponent = CreateDefaultSubobject<UFootstepsComponent>(TEXT("FootstepsComponent"));

	AGMotionWarpingComponent = CreateDefaultSubobject<UAG_MotionWarpingComponent>(TEXT("MotionWarpingComponent"));
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	InventoryComponent->SetIsReplicated(true);
}

void AActionGameCharacter::PostLoad()
{
	Super::PostLoad();

	if ((CharacterDataAsset))
	{
		SetCharacterData(CharacterDataAsset->CharacterData);
	}
}

void AActionGameCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AActionGameCharacter::OnMaxMovementSpeedChanged(const FOnAttributeChangeData& Data)
{
	GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
}

UAbilitySystemComponent* AActionGameCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AActionGameCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AActionGameCharacter, CharacterData);
	DOREPLIFETIME(AActionGameCharacter, InventoryComponent);
}

void AActionGameCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AActionGameCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveActiveEffectsWithTags(InAirTags);
	}
}

void AActionGameCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	if (!CrouchStateEffect.Get()) return;

	if (AbilitySystemComponent)
	{
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();

		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(CrouchStateEffect, 1, EffectContext);
		if (SpecHandle.IsValid())
		{
			FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			if (!ActiveGEHandle.WasSuccessfullyApplied())
			{
				ABILITY_LOG(Log, TEXT("Ability %s failed to apply crouch effect %s"), *GetName(), *GetNameSafe(CrouchStateEffect));
			}
		}
	}
}

void AActionGameCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if (AbilitySystemComponent && CrouchStateEffect.Get())
	{
		AbilitySystemComponent->RemoveActiveGameplayEffectBySourceEffect(CrouchStateEffect, AbilitySystemComponent);
	}

	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AActionGameCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) 
	{
		if (MoveForwardInputAction)
		{
			EnhancedInputComponent->BindAction(MoveForwardInputAction, ETriggerEvent::Triggered, this, &AActionGameCharacter::OnMoveForwardAction);
		}

		if (MoveSideInputAction)
		{
			EnhancedInputComponent->BindAction(MoveSideInputAction, ETriggerEvent::Triggered, this, &AActionGameCharacter::OnMoveSideAction);
		}

		if (JumpInputAction)
		{
			EnhancedInputComponent->BindAction(JumpInputAction, ETriggerEvent::Started, this, &AActionGameCharacter::OnJumpActionStarted);
			EnhancedInputComponent->BindAction(JumpInputAction, ETriggerEvent::Completed, this, &AActionGameCharacter::OnJumpActionStopped);
		}

		if (TurnInputAction)
		{
			EnhancedInputComponent->BindAction(TurnInputAction, ETriggerEvent::Triggered, this, &AActionGameCharacter::OnTurnAction);
		}

		if (LookUpInputAction)
		{
			EnhancedInputComponent->BindAction(LookUpInputAction, ETriggerEvent::Triggered, this, &AActionGameCharacter::OnLookUpAction);
		}

		if (CrouchInputAction)
		{
			EnhancedInputComponent->BindAction(CrouchInputAction, ETriggerEvent::Started, this, &AActionGameCharacter::OnCrouchActionStarted);
			EnhancedInputComponent->BindAction(CrouchInputAction, ETriggerEvent::Completed, this, &AActionGameCharacter::OnCrouchActionStopped);
		}

		if (SprintInputAction)
		{
			EnhancedInputComponent->BindAction(SprintInputAction, ETriggerEvent::Started, this, &AActionGameCharacter::OnSprintActionStarted);
			EnhancedInputComponent->BindAction(SprintInputAction, ETriggerEvent::Completed, this, &AActionGameCharacter::OnSprintActionStopped);
		}

		if (EquipNextInputAction)
		{
			EnhancedInputComponent->BindAction(EquipNextInputAction, ETriggerEvent::Triggered, this, &AActionGameCharacter::OnEquipNextTriggered);
		}

		if (DropItemInputAction)
		{
			EnhancedInputComponent->BindAction(DropItemInputAction, ETriggerEvent::Triggered, this, &AActionGameCharacter::OnDropItemTriggered);
		}

		if (UnequipInputAction)
		{
			EnhancedInputComponent->BindAction(UnequipInputAction, ETriggerEvent::Triggered, this, &AActionGameCharacter::OnUnequipTriggered);
		}

		if (AttackInputAction)
		{
			EnhancedInputComponent->BindAction(AttackInputAction, ETriggerEvent::Started, this, &AActionGameCharacter::OnAttackActionStarted);
			EnhancedInputComponent->BindAction(AttackInputAction, ETriggerEvent::Completed, this, &AActionGameCharacter::OnAttackActionStopped);
		}

		if (AimInputAction)
		{
			EnhancedInputComponent->BindAction(AimInputAction, ETriggerEvent::Started, this, &AActionGameCharacter::OnAimActionStarted);
			EnhancedInputComponent->BindAction(AimInputAction, ETriggerEvent::Completed, this, &AActionGameCharacter::OnAimActionStopped);
		}
	}
}

void AActionGameCharacter::OnMoveForwardAction(const FInputActionValue& Value)
{
	const float Magnitude = Value.GetMagnitude();

	if (Controller != nullptr && Magnitude != 0.f)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// add movement 
		AddMovementInput(ForwardDirection, Magnitude);
	}
}

void AActionGameCharacter::OnMoveSideAction(const FInputActionValue& Value)
{
	const float Magnitude = Value.GetMagnitude();

	if (Controller != nullptr && Magnitude != 0.f)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(RightDirection, Magnitude);
	}
}

void AActionGameCharacter::OnTurnAction(const FInputActionValue& Value)
{
	if (Controller != nullptr)
	{
		// add yaw input to controller
		AddControllerYawInput(Value.GetMagnitude() * TurnRate * GetWorld()->GetDeltaSeconds());
	}
}

void AActionGameCharacter::OnLookUpAction(const FInputActionValue& Value)
{
	if (Controller != nullptr)
	{
		// add pitch input to controller
		AddControllerPitchInput(Value.GetMagnitude() * TurnRate * GetWorld()->GetDeltaSeconds());
	}
}

void AActionGameCharacter::OnJumpActionStarted(const FInputActionValue& Value)
{
	AGCharacterMovementComponent->TryTraversal(AbilitySystemComponent);
}

void AActionGameCharacter::OnJumpActionStopped(const FInputActionValue& Value)
{
	//StopJumping();
}

void AActionGameCharacter::OnCrouchActionStarted(const FInputActionValue& Value)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->TryActivateAbilitiesByTag(CrouchTags, true);
	}
}

void AActionGameCharacter::OnCrouchActionStopped(const FInputActionValue& Value)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->CancelAbilities(&CrouchTags);
	}
}

void AActionGameCharacter::OnSprintActionStarted(const FInputActionValue& Value)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->TryActivateAbilitiesByTag(SprintTags, true);
	}
}

void AActionGameCharacter::OnSprintActionStopped(const FInputActionValue& Value)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->CancelAbilities(&SprintTags);
	}
}

void AActionGameCharacter::OnDropItemTriggered(const FInputActionValue& Value)
{
	FGameplayEventData EventPayload;
	EventPayload.EventTag = UInventoryComponent::DropItemTag;
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, UInventoryComponent::DropItemTag, EventPayload);
}

void AActionGameCharacter::OnEquipNextTriggered(const FInputActionValue& Value)
{
	FGameplayEventData EventPayload;
	EventPayload.EventTag = UInventoryComponent::EquipNextTag;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, UInventoryComponent::EquipNextTag, EventPayload);
}

void AActionGameCharacter::OnUnequipTriggered(const FInputActionValue& Value)
{
	FGameplayEventData EventPayload;
	EventPayload.EventTag = UInventoryComponent::UnequipTag;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, UInventoryComponent::UnequipTag, EventPayload);
}

void AActionGameCharacter::OnAttackActionStarted(const FInputActionValue& Value)
{
	FGameplayEventData EventPayload;
	EventPayload.EventTag = AttackStartedEventTag;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, AttackStartedEventTag, EventPayload);
}

void AActionGameCharacter::OnAttackActionStopped(const FInputActionValue& Value)
{
	FGameplayEventData EventPayload;
	EventPayload.EventTag = AttackStoppedEventTag;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, AttackStoppedEventTag, EventPayload);
}

void AActionGameCharacter::OnAimActionStarted(const FInputActionValue& Value)
{
	FGameplayEventData EventPayload;
	EventPayload.EventTag = AimStartedEventTag;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, AimStartedEventTag, EventPayload);
}

void AActionGameCharacter::OnAimActionStopped(const FInputActionValue& Value)
{
	FGameplayEventData EventPayload;
	EventPayload.EventTag = AimStoppedEventTag;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, AimStoppedEventTag, EventPayload);
}

//////////////////////////////////////////////////////////////////////////
// Abilities

void AActionGameCharacter::OnRep_CharacterData()
{
	InitFromCharacterData(CharacterData, true);
}

void AActionGameCharacter::InitFromCharacterData(const FCharacterData& InCharacterData, bool bFromReplication)
{

}

void AActionGameCharacter::SetCharacterData(const FCharacterData& InCharacterData)
{
	CharacterData = InCharacterData;

	InitFromCharacterData(CharacterData);
}

bool AActionGameCharacter::ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> Effect, FGameplayEffectContextHandle InEffectContext)
{
	if (!Effect.Get()) return false;

	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(Effect, 1, InEffectContext);
	if (SpecHandle.IsValid())
	{
		FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

		return ActiveGEHandle.WasSuccessfullyApplied();
	}
	return false;
}

void AActionGameCharacter::GiveAbilities()
{
	if (HasAuthority() && AbilitySystemComponent)
	{
		for (auto DefaultAbility : CharacterData.Abilities)
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(DefaultAbility));
		}
	}
}

void AActionGameCharacter::ApplyStartupEffects()
{
	if (HasAuthority())
	{
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		for (auto CharacterEffect : CharacterData.Effects)
		{
			ApplyGameplayEffectToSelf(CharacterEffect, EffectContext);
		}
	}
}

// Server
void AActionGameCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	GiveAbilities();
	ApplyStartupEffects();
}

// Client 
void AActionGameCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}




