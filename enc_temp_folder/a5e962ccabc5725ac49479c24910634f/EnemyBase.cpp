// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBase.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"


AEnemyBase::AEnemyBase()
{
	Weapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Weapon"));
	Weapon->SetupAttachment(GetMesh(), "RightHandItem");
	Weapon->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

	PrimaryActorTick.bCanEverTick = true;
	MovingBackwards = false;
	MovingForward = false;
	Interruptable = true;
	LastStumbleIndex = 0;

}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	ActiveState = State::IDLE;

	//getting the player
	Target = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TickStateMachine();
}

void AEnemyBase::TickStateMachine()
{
	switch (ActiveState)
	{
	case State::IDLE:
		StateIdle();
		break;
	case State::CHASE_CLOSE:
		StateChaseClose();
		break;
	case State::CHASE_FAR:
		StateChaseFar();
		break;
	case State::ATTACK:
		StateAttack();
		break;
	case State::STUMBLE:
		StateStumble();
		break;
	case State::TAUNT:
		StateTaunt();
		break;
	case State::DEAD:
		StateDead();
		break;
	}
}

void AEnemyBase::SetState(State NewState)
{
	if (ActiveState != State::DEAD)
	{
		ActiveState = NewState;
	}
}

void AEnemyBase::StateIdle()
{
	if (Target && FVector::Distance(Target->GetActorLocation(), GetActorLocation()) <= 1200.0f)
	{
		TargetLocked = true;

		SetState(State::CHASE_CLOSE);
	}
}

void AEnemyBase::StateChaseClose()
{
	float Distance = FVector::Distance(GetActorLocation(), Target->GetActorLocation());

	if (Distance <= 300.0f)
	{
		FVector TargetDirection = Target->GetActorLocation() - GetActorLocation();

		float DotProduct = FVector::DotProduct(GetActorForwardVector(), TargetDirection.GetSafeNormal());

		if (DotProduct > 0.95f && !Attacking && !Stumbling)
		{
			Attack(false);
		}
	}
	else
	{
		AAIController* AIController = Cast<AAIController>(Controller);

		if (!AIController->IsFollowingAPath())
		{
			AIController->MoveToActor(Target);
		}
	}
}


void AEnemyBase::MoveForward()
{
	FVector NewLocation = GetActorLocation() + (GetActorForwardVector() * 500.0f * GetWorld()->GetDeltaSeconds());

	SetActorLocation(NewLocation, true);
}

void AEnemyBase::Attack(bool Rotate)
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

	int RandomIndex = FMath::RandRange(0, AttackAnimations.Num() - 1);
	PlayAnimMontage(AttackAnimations[RandomIndex]);
}

void AEnemyBase::AttackNextReady()
{
	Super::AttackNextReady();
}

void AEnemyBase::EndAttack()
{
	Super::EndAttack();
	SetState(State::CHASE_CLOSE);
}

void AEnemyBase::AttackLunge()
{
	Super::AttackLunge();
}

void AEnemyBase::StateChaseFar()
{
	if (FVector::Distance(GetActorLocation(), Target->GetActorLocation()) <= 850.0f)
	{
		SetState(State::CHASE_CLOSE);
	}
}

void AEnemyBase::StateAttack()
{
	if (AttackDamaging)
	{
		TSet<AActor*> OverlappingActors;
		Weapon->GetOverlappingActors(OverlappingActors);

		for (AActor* OtherActor : OverlappingActors)
		{
			if (OtherActor == this)
			{
				continue;
			}

			if (!AttackHitActors.Contains(OtherActor))
			{
				float AppliedDamage = UGameplayStatics::ApplyDamage(OtherActor, 1.0f, GetController(), this, UDamageType::StaticClass());

				if (AppliedDamage > 0.0f)
				{
					AttackHitActors.Add(OtherActor);
				}
			}
		}
	}

	if (MovingForward)
	{
		MoveForward();
	}
}

void AEnemyBase::StateStumble()
{
	if (Stumbling)
	{
		if (MovingBackwards)
		{
			AddMovementInput(-GetActorForwardVector(), 40.0f * GetWorld()->GetDeltaSeconds());
		}
		else
		{
			SetState(State::CHASE_CLOSE);
		}
	}
}

void AEnemyBase::StateTaunt()
{
}

void AEnemyBase::StateDead()
{
}

void AEnemyBase::FocusTarget()
{
}

float AEnemyBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	return 0.0f;
}

void AEnemyBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
