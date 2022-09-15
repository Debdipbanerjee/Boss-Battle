// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBoss.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"


AEnemyBoss::AEnemyBoss()
{
	LongAttack_Cooldown = 5.0f;
	LongAttack_Timestamp = -LongAttack_Cooldown;
}

void AEnemyBoss::StateChaseClose()
{
	float Distance = FVector::Distance(GetActorLocation(), Target->GetActorLocation());
	AAIController* AIController = Cast<AAIController>(Controller);

	FVector TargetDirection = Target->GetActorLocation() - GetActorLocation();

	float DotProduct = FVector::DotProduct(GetActorForwardVector(), TargetDirection.GetSafeNormal());

	if (Distance <= 900.0f && DotProduct >= 0.95f)
	{
		if (Distance <= 300.0f)
		{
			Attack(false);
			return;
		}
		else if (UGameplayStatics::GetTimeSeconds(GetWorld()) >= LongAttack_Timestamp + LongAttack_Cooldown && AIController->LineOfSightTo(Target))
		{
			LongAttack_Timestamp = UGameplayStatics::GetTimeSeconds(GetWorld());
			LongAttack(true);
			return;
		}
	}

	if (AIController->IsFollowingAPath())
	{
		AIController->MoveToActor(Target);
	}
}

void AEnemyBoss::LongAttack(bool Rotate)
{
	Super::Attack();

	SetMovingBackwards(false);
	SetMovingForward(false);
	SetState(State::ATTACK);
	Cast<AAIController>(Controller)->StopMovement();

	if (Rotate)
	{
		FVector Direction = Target->GetActorLocation() - GetActorLocation();
		Direction = FVector(Direction.X, Direction.Y, 0);

		FRotator Rotation = FRotationMatrix::MakeFromX(Direction).Rotator();
		SetActorRotation(Rotation);
	}

	float Distance = FVector::Distance(GetActorLocation(), Target->GetActorLocation());
	LongAttack_ForwardSpeed = Distance + 600.0f;

	int RandomIndex = FMath::RandRange(0, LongAttackAnimations.Num() - 1);
	PlayAnimMontage(LongAttackAnimations[RandomIndex]);
}

void AEnemyBoss::MoveForward()
{
	FVector NewLocation = GetActorLocation() + (GetActorForwardVector() * LongAttack_ForwardSpeed * GetWorld()->GetDeltaSeconds());
	SetActorLocation(NewLocation);
}

float AEnemyBoss::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (DamageCauser == this)
	{
		return 0.0f;
	}

	if (QuickHitsTaken == 0 || GetWorld()->GetTimeSeconds() - QuickHitsTimestamp <= 1.0f)
	{
		QuickHitsTaken++;
		QuickHitsTimestamp = GetWorld()->GetTimeSeconds();

		if (QuickHitsTaken >= 4)
		{
			Interruptable = false;
		}
		else
		{
			QuickHitsTaken = 0;
			Interruptable = true;
		}
	}

	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}
