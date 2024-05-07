// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/SlashCharacter.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GroomComponent.h"
#include "Components/StaticMeshComponent.h"
#include "../Item.h"
#include "../Weapon.h"

// Sets default values
ASlashCharacter::ASlashCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f,400.f,0.0f);

	GetMesh()->SetCollisionObjectType(ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);


	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 300.f;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(SpringArm);

	Hair = CreateDefaultSubobject<UGroomComponent>(TEXT("Hair"));
	Hair->SetupAttachment(GetMesh());
	Hair->AttachmentName = FString("head");
	
	Eyebrows = CreateDefaultSubobject<UGroomComponent>(TEXT("Eyebrows"));
	Eyebrows->SetupAttachment(GetMesh());
	Eyebrows->AttachmentName = FString("head");
}

void ASlashCharacter::GetHit_Implementation(const FVector& ImpactPoint)
{
	PlayHitSound(ImpactPoint);

	SpawnHitParticles(ImpactPoint);
}

// Called when the game starts or when spawned
void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(SlashContext, 0);
		}
	}

	Tags.Add(FName("EngageableTarget"));
}

void ASlashCharacter::Move(const FInputActionValue& Value)
{

	if (ActionState == EActionState::EAS_Unoccupied)
	{
		const FVector2D MovementVector = Value.Get<FVector2D>();
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		UE_LOG(LogTemp, Warning, TEXT("ForwardDirection(%f,%f,%f)"), ForwardDirection.X, ForwardDirection.Y, ForwardDirection.Z);
		AddMovementInput(ForwardDirection, MovementVector.Y);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		UE_LOG(LogTemp, Warning, TEXT("RightDirection(%f,%f,%f)"), RightDirection.X, RightDirection.Y, RightDirection.Z);
		AddMovementInput(RightDirection, MovementVector.X);

		//old implementation only forward and backwards movement
		/*const FVector Forward = GetActorForwardVector();
		AddMovementInput(Forward, MovementValue.Y);

		const FVector Right = GetActorRightVector();
		AddMovementInput(Right, MovementValue.X);*/
	}
}

void ASlashCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerPitchInput(LookAxisVector.Y);
	AddControllerYawInput(LookAxisVector.X);
}

void ASlashCharacter::Disarm()
{
	PlayEquipMontage(FName("Unequip"));
	CharacterState = ECharacterState::ECS_Unequipped;
	ActionState = EActionState::EAS_EquippingWeapon;
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Unequip!"));
}

void ASlashCharacter::Arm()
{
	PlayEquipMontage(FName("Equip"));
	CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
	ActionState = EActionState::EAS_EquippingWeapon;
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Equip!"));
}

void ASlashCharacter::EkeyPressed(const FInputActionValue& Value)
{
	AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);
	if (OverlappingItem)
	{
		EquipWeapon(OverlappingWeapon);
	}
	else 
	{
		if (CanDisarm())
		{
			Disarm();
		}
		else if (CanArm())
		{
			Arm();
		}
	}
}

void ASlashCharacter::EquipWeapon(AWeapon* Weapon)
{
	if (Weapon->GetWeaponType() == EWeaponType::EWT_OneHanded)
	{
		Weapon->Equip(this->GetMesh(), FName("RightHandSocket"), this, this);
	}
	if (Weapon->GetWeaponType() == EWeaponType::EWT_TwoHanded)
	{
		Weapon->Equip(this->GetMesh(), FName("TwoHandedSocket"), this, this);
	}

	//OverlappingWeapon->SetOwner(this); // same functionality as the line below
	//OverlappingWeapon->SetInstigator(this);

	//CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
	OverlappingItem = nullptr;
	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
		EquippedWeapon = Weapon;
	}
	else
	{
		EquippedWeapon = Weapon;
	}

	switch (EquippedWeapon->GetWeaponType())
	{
	case EWeaponType::EWT_OneHanded:
		CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
		break;
	case EWeaponType::EWT_TwoHanded:
		CharacterState = ECharacterState::ECS_EquippedTwoHandedWeapon;
		break;
	default:
		CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
		break;
	}
}

void ASlashCharacter::Attack(const FInputActionValue& Value)
{

	if (CanAttack())
	{
		PlayAttackMontage();
		ActionState = EActionState::EAS_Attacking;
	}
}

void ASlashCharacter::DoAttack()
{
	Super::DoAttack();
}

//void ASlashCharacter::PlayAttackMontage()
//{
//	Super::PlayAttackMontage();
//	if (AttackMontage)
//	{
//		const int32 Selection = FMath::RandRange(0, 1);
//		FName SectionName = FName();
//		if(CharacterState == ECharacterState::ECS_EquippedOneHandedWeapon)
//		{
//			switch (Selection)
//			{
//			case 0:
//				PlayAnimMontage(AttackMontage, 1.0f, FName("Attack1"));
//				break;
//			case 1:
//				PlayAnimMontage(AttackMontage, 1.0f, FName("Attack2"));
//				break;
//			default:
//				break;
//			}
//		}
//		if (CharacterState == ECharacterState::ECS_EquippedTwoHandedWeapon)
//		{
//			switch (Selection)
//			{
//			case 0:
//				PlayAnimMontage(AttackMontage, 1.0f, FName("TwoHandedAttack1"));
//				break;
//			case 1:
//				PlayAnimMontage(AttackMontage, 1.0f, FName("TwoHandedAttack2"));
//				break;
//			default:
//				break;
//			}
//		}
//		
//	}
//}

void ASlashCharacter::PlayEquipMontage(FName SectionName)
{
	PlayAnimMontage(EquipMontage, 1.0f, SectionName);
}

void ASlashCharacter::AttachWeaponToBack()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
	}
}

void ASlashCharacter::AttachWeaponToHand()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
	}
}

void ASlashCharacter::FinishEquipping()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::CanAttack()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState != ECharacterState::ECS_Unequipped;
}

int32 ASlashCharacter::PlayAttackMontage()
{
	if(CharacterState == ECharacterState::ECS_EquippedTwoHandedWeapon && TwoHandedAttackMontageSections.Num() > 0)
	{
		return PlayRandomMontageSection(AttackMontage, TwoHandedAttackMontageSections);
	}
	else if(CharacterState == ECharacterState::ECS_EquippedOneHandedWeapon && AttackMontageSections.Num() > 0)
	{
		return PlayRandomMontageSection(AttackMontage, AttackMontageSections);
	}
	return -1;
}

bool ASlashCharacter::CanDisarm()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState != ECharacterState::ECS_Unequipped;
}

bool ASlashCharacter::CanArm()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState == ECharacterState::ECS_Unequipped &&
		EquippedWeapon;
}

void ASlashCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Jump);
		EnhancedInputComponent->BindAction(EkeyAction, ETriggerEvent::Started, this, &ASlashCharacter::EkeyPressed);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Attack);
	}

}