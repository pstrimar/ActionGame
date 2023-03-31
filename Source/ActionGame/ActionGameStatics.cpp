// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionGameStatics.h"

#include "Kismet/KismetSystemLibrary.h"
#include "AbilitySystemComponent.h"
#include "Actors/Projectile.h"
#include "AbilitySystemBlueprintLibrary.h"

static TAutoConsoleVariable<int32> CVarShowRadialDamage(
	TEXT("ShowDebugRadialDamage"),
	0,
	TEXT("Draws debug info about radial damage")
	TEXT(" 0: 0ff/n")
	TEXT(" 1: On/n"),
	ECVF_Cheat
);

const UItemStaticData* UActionGameStatics::GetItemStaticData(TSubclassOf<UItemStaticData> ItemDataClass)
{
	if (IsValid(ItemDataClass))
	{
		return GetDefault<UItemStaticData>(ItemDataClass);
	}

	return nullptr;
}

void UActionGameStatics::ApplyRadialDamage(UObject* WorldContextObject, AActor* DamageCauser, FVector Location, float Radius, float DamageAmount, TArray<TSubclassOf<UGameplayEffect>> DamageEffects, const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes, ETraceTypeQuery TraceType)
{
	TArray<AActor*> OutActors;
	TArray<AActor*> ActorsToIgnore = { DamageCauser };

	UKismetSystemLibrary::SphereOverlapActors(WorldContextObject, Location, Radius, ObjectTypes, nullptr, ActorsToIgnore, OutActors);

	const bool bDebug = static_cast<bool>(CVarShowRadialDamage.GetValueOnAnyThread());

	for (AActor* Actor : OutActors)
	{
		FHitResult HitResult;
		
		if (UKismetSystemLibrary::LineTraceSingle(WorldContextObject, Location, Actor->GetActorLocation(), TraceType, true, ActorsToIgnore, EDrawDebugTrace::None, HitResult, true))
		{
			AActor* Target = HitResult.GetActor();

			if (Target == Actor)
			{
				bool bWasApplied = false;

				if (UAbilitySystemComponent* AbilityComponenet = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target))
				{
					FGameplayEffectContextHandle EffectContext = AbilityComponenet->MakeEffectContext();
					EffectContext.AddInstigator(DamageCauser, DamageCauser);

					for (auto Effect : DamageEffects)
					{
						FGameplayEffectSpecHandle SpecHandle = AbilityComponenet->MakeOutgoingSpec(Effect, 1, EffectContext);
						if (SpecHandle.IsValid())
						{
							UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, FGameplayTag::RequestGameplayTag(TEXT("Attribute.Health")), -DamageAmount);

							FActiveGameplayEffectHandle ActiveGEHandle = AbilityComponenet->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

							if (ActiveGEHandle.WasSuccessfullyApplied())
							{
								bWasApplied = true;
							}
						}
					}
				}

				if (bDebug)
				{
					DrawDebugLine(WorldContextObject->GetWorld(), Location, Actor->GetActorLocation(), bWasApplied ? FColor::Green : FColor::Red, false, 4.f, 0, 1);
					DrawDebugSphere(WorldContextObject->GetWorld(), HitResult.Location, 16, 16, bWasApplied ? FColor::Green : FColor::Red, false, 4.f, 0, 1);
					DrawDebugString(WorldContextObject->GetWorld(), HitResult.Location, *GetNameSafe(Target), nullptr, FColor::White, 0, false, 1.f);
				}
			}
			else
			{
				if (bDebug)
				{
					DrawDebugLine(WorldContextObject->GetWorld(), Location, Actor->GetActorLocation(), FColor::Red, false, 4.f, 0, 1);
					DrawDebugSphere(WorldContextObject->GetWorld(), HitResult.Location, 16, 16, FColor::Red, false, 4.f, 0, 1);
					DrawDebugString(WorldContextObject->GetWorld(), HitResult.Location, *GetNameSafe(Target), nullptr, FColor::Red, 0, false, 1.f);
				}
			}
		}
		else
		{
			if (bDebug)
			{
				DrawDebugLine(WorldContextObject->GetWorld(), Location, Actor->GetActorLocation(), FColor::Red, false, 4.f, 0, 1);
				DrawDebugSphere(WorldContextObject->GetWorld(), HitResult.Location, 16, 16, FColor::Red, false, 4.f, 0, 1);
				DrawDebugString(WorldContextObject->GetWorld(), HitResult.Location, *GetNameSafe(HitResult.GetActor()), nullptr, FColor::Red, 0, false, 1.f);
			}
		}
	}

	if (bDebug)
	{
		DrawDebugSphere(WorldContextObject->GetWorld(), Location, Radius, 16, FColor::Red, false, 4.f, 0, 1);
	}
}

AProjectile* UActionGameStatics::LaunchProjectile(UObject* WorldContextObject, TSubclassOf<UProjectileStaticData> ProjectileDataClass, FTransform Transform, AActor* Owner, APawn* Instigator)
{
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;

	if (World && World->IsServer())
	{
		if (AProjectile* Projectile = World->SpawnActorDeferred<AProjectile>(AProjectile::StaticClass(), Transform, Owner, Instigator, ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
		{
			Projectile->ProjectileDataClass = ProjectileDataClass;
			Projectile->FinishSpawning(Transform);

			return Projectile;
		}
	}

	return nullptr;
}
