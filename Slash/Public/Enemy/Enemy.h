// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "Characters/CharacterTypes.h"
#include "Enemy.generated.h"


class UWidgetComponent;
class UHealthBarComponent;
class UPawnSensingComponent;

UCLASS()
class SLASH_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	virtual void Destroyed() override;

	void GetHit_Implementation(const FVector& ImpactPoint) override;

protected:
	virtual void BeginPlay() override;
	bool InTargetRange(AActor* Target, double Radius);
	void MoveToTarget(AActor* Target);
	virtual void DoAttack() override;
	void Attack(const FInputActionValue& Value);
	virtual int32 PlayDeathMontage() override;
	virtual void AttackEnd() override;
	virtual bool CanAttack() override;
	virtual void HandleDamage(float DamageAmount) override;
	virtual void Die() override;

	UFUNCTION()
		void PawnSeen(APawn* SeenPawn); //Callback for delegate onpawnseen

	UPROPERTY(EditAnywhere, Category = "Combat")
		float DeathLifeSpan = 8.f;

	UPROPERTY(BlueprintReadOnly)
		TEnumAsByte<EDeathPose> DeathPose;

	AActor* ChoosePatrolTarget();

	UPROPERTY(BlueprintReadOnly)
		EEnemyState EnemyState = EEnemyState::EES_Patrolling;

private:
	void CheckPatrolTarget();
	void CheckCombatTarget();

	FTimerHandle PatrolTimer;
	void PatrolTimerFinished();

	void HideHealthBar();
	void ShowHealthBar();
	void LoseInterest();
	void StartPatrolling();
	void ChaseTarget();
	bool IsOutsideCombatRadius();
	bool IsOutsideAttackRadius();
	bool IsInsideAttackRadius();
	bool IsChasing();
	bool IsAttacking();
	void ClearPatrolTimer();
	bool IsDead();
	bool IsEngaged();
	void StartAttackTimer();
	void ClearAttackTimer();
	void SpawnDefaultWeapon();
	void InitializeEnemy();


	UPROPERTY(VisibleAnywhere)
		UPawnSensingComponent* PawnSensing;

	UPROPERTY(VisibleAnywhere)
		UHealthBarComponent* HealthBarWidget; //was previously UWidgetComponent

	UPROPERTY(EditAnywhere)
		TSubclassOf<class AWeapon> WeaponClass;

	// Current patrol target
	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
		AActor* PatrolTarget;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
		TArray<AActor*> PatrolTargets;

	UPROPERTY(EditAnywhere)
		double PatrolRadius = 200.f;

	UPROPERTY()
		class AAIController* EnemyController;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
		float PatrolWaitMin = 5.f;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
		float PatrolWaitMax = 10.f;

	FTimerHandle AttackTimer;


	UPROPERTY()
		AActor* CombatTarget;

	UPROPERTY(EditAnywhere)
		double CombatRadius = 1500.f;

	UPROPERTY(EditAnywhere)
		double AttackRadius = 150.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
		float AttackMin = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Combat")
		float AttackMax = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Combat")
		float PatrollingSpeed = 125.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
		float ChasingSpeed = 300.f;
};
