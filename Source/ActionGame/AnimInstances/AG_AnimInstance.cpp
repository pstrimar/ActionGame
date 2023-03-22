// Fill out your copyright notice in the Description page of Project Settings.


#include "AG_AnimInstance.h"
#include "ActionGameCharacter.h"
#include "ActionGameTypes.h"
#include "Animation/AnimSequenceBase.h"
#include "Animation/BlendSpace.h"
#include "DataAssets/CharacterDataAsset.h"
#include "DataAssets//CharacterAnimDataAsset.h"

UBlendSpace* UAG_AnimInstance::GetLocomotionBlendspace() const
{
	if (AActionGameCharacter* ActionGameCharacter = Cast<AActionGameCharacter>(GetOwningActor()))
	{
		FCharacterData Data = ActionGameCharacter->GetCharacterData();

		if (Data.CharacterAnimDataAsset)
		{
			return Data.CharacterAnimDataAsset->CharacterAnimationData.MovementBlendSpace;
		}
	}
	return DefaultCharacterAnimDataAsset ? DefaultCharacterAnimDataAsset->CharacterAnimationData.MovementBlendSpace : nullptr;
}

UAnimSequenceBase* UAG_AnimInstance::GetIdleAnimation() const
{
	if (AActionGameCharacter* ActionGameCharacter = Cast<AActionGameCharacter>(GetOwningActor()))
	{
		FCharacterData Data = ActionGameCharacter->GetCharacterData();

		if (Data.CharacterAnimDataAsset)
		{
			return Data.CharacterAnimDataAsset->CharacterAnimationData.IdleAnimationAsset;
		}
	}
	return DefaultCharacterAnimDataAsset ? DefaultCharacterAnimDataAsset->CharacterAnimationData.IdleAnimationAsset : nullptr;
}

class UBlendSpace* UAG_AnimInstance::GetCrouchLocomotionBlendspace() const
{
	if (AActionGameCharacter* ActionGameCharacter = Cast<AActionGameCharacter>(GetOwningActor()))
	{
		FCharacterData Data = ActionGameCharacter->GetCharacterData();

		if (Data.CharacterAnimDataAsset)
		{
			return Data.CharacterAnimDataAsset->CharacterAnimationData.CrouchMovementBlendSpace;
		}
	}
	return DefaultCharacterAnimDataAsset ? DefaultCharacterAnimDataAsset->CharacterAnimationData.CrouchMovementBlendSpace : nullptr;
}

class UAnimSequenceBase* UAG_AnimInstance::GetCrouchIdleAnimation() const
{
	if (AActionGameCharacter* ActionGameCharacter = Cast<AActionGameCharacter>(GetOwningActor()))
	{
		FCharacterData Data = ActionGameCharacter->GetCharacterData();

		if (Data.CharacterAnimDataAsset)
		{
			return Data.CharacterAnimDataAsset->CharacterAnimationData.CrouchIdleAnimationAsset;
		}
	}
	return DefaultCharacterAnimDataAsset ? DefaultCharacterAnimDataAsset->CharacterAnimationData.CrouchIdleAnimationAsset : nullptr;
}
