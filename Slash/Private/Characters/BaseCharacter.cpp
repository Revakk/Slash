#include "Characters/BaseCharacter.h"
#include "Components/BoxComponent.h"
#include "../Weapon.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/AttributeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"

ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));

}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseCharacter::DoAttack()
{
}

bool ABaseCharacter::CanAttack()
{
	return true;
}

bool ABaseCharacter::IsAlive()
{
	return Attributes && Attributes->ISsAlive();
}

void ABaseCharacter::AttackEnd()
{
}

void ABaseCharacter::PlayHitReactMontage(const FName& SectionName)
{
	if (HitReactMontage)
	{
		PlayAnimMontage(HitReactMontage, 1.0f, SectionName);
	}
}

void ABaseCharacter::DisableCapsule()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseCharacter::DirectionalHitReact(const FVector& ImpactPoint)
{
	const FVector ForwardVec = GetActorForwardVector();

	const FVector ImpactLowered(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z);
	const FVector ImpactVec = (ImpactLowered - GetActorLocation()).GetSafeNormal();
	double ImpactAngle = FVector::DotProduct(ForwardVec, ImpactVec);//ImpactVec.Dot(GetActorForwardVector());

	ImpactAngle = FMath::Acos(ImpactAngle);
	double ImpactAngleDeg = FMath::RadiansToDegrees(ImpactAngle);

	const FVector CrossProduct = FVector::CrossProduct(ForwardVec, ImpactVec);
	if (CrossProduct.Z < 0)
	{
		ImpactAngleDeg *= -1.0;
		//UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + CrossProduct * 100.f, 5.f, FColor::Blue, 5.f);
	}

	PlayImpactMontage(ImpactAngleDeg);


	/*UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + ForwardVec * 60.f, 5.f, FColor::Red, 5.f);
	UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + ImpactVec * 60.f, 5.f, FColor::Green, 5.f);*/
}

void ABaseCharacter::PlayImpactMontage(float ImpactAngle)
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::SanitizeFloat(ImpactAngle));
	if (ImpactAngle < 45.f && ImpactAngle > -45.f)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("FromFront!"));
		PlayHitReactMontage(FName("FromFront"));
	}
	else if (ImpactAngle < -45.f && ImpactAngle > -135.f)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("FromLeft!"));
		PlayHitReactMontage(FName("FromLeft"));
	}
	else if (ImpactAngle < 135.f && ImpactAngle > 45.f)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("FromRight!"));
		PlayHitReactMontage(FName("FromRight"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("FromBack!"));
		PlayHitReactMontage(FName("FromBack"));
	}
}

void ABaseCharacter::PlayHitSound(const FVector& ImpactPoint)
{
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			HitSound,
			ImpactPoint
		);
	}
}

void ABaseCharacter::SpawnHitParticles(const FVector& ImpactPoint)
{
	if (HitParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			HitParticle,
			ImpactPoint);
	}
}

void ABaseCharacter::HandleDamage(float DamageAmount)
{
	Attributes->ReceiveDamage(DamageAmount);
}

void ABaseCharacter::PlayMontageSection(UAnimMontage* Montage, const FName& Section)
{
	if(Montage)
	{
		PlayAnimMontage(Montage, 1.0f, Section);
	}
}

int32 ABaseCharacter::PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames)
{
	if (AttackMontageSections.Num() == 0) return -1;
	const int32 MaxSectionIndex = SectionNames.Num() - 1;
	const int32 Selection = FMath::RandRange(0, MaxSectionIndex);
	PlayMontageSection(Montage, SectionNames[Selection]);
	return Selection;
}

int32 ABaseCharacter::PlayAttackMontage()
{
	return PlayRandomMontageSection(AttackMontage, AttackMontageSections);
}

int32 ABaseCharacter::PlayDeathMontage()
{
	return PlayRandomMontageSection(DeathMontage, DeathMontageSections);
}

void ABaseCharacter::Die()
{
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (EquippedWeapon && EquippedWeapon->GetWeaponBox())
	{
		EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		EquippedWeapon->IgnoreActors.Empty();
	}
}

