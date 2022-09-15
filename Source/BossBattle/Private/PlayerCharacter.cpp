// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/CameraShake.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnemyBase.h"


// Sets default values
APlayerCharacter::APlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TargetLockDistance = 1500.0f;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	BaseTurnRate = 45.0f;
	BaseLookUpRate = 45.0f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.0f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Boom"));
	CameraBoom->SetupAttachment(RootComponent);

	// The camera follows at this distance behind the character
	CameraBoom->TargetArmLength = 500.0f;

	// Rotate the arm based on the controller
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));

	// Attach the camera to the end of the boom and let the boom adjust 
	// to match the controller orientation
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	// Camera does not rotate relative to arm
	FollowCamera->bUsePawnControlRotation = false;

	Weapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Weapon"));
	Weapon->SetupAttachment(GetMesh(), "RightHandItem");
	Weapon->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

	EnemyDetectionCollider = CreateDefaultSubobject<USphereComponent>(TEXT("Enemy Detection Collider"));
	EnemyDetectionCollider->SetupAttachment(RootComponent);
	EnemyDetectionCollider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	EnemyDetectionCollider->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn,
		ECollisionResponse::ECR_Overlap);
	EnemyDetectionCollider->SetSphereRadius(TargetLockDistance);

	Attacking = false;
	Rolling = false;
	TargetLocked = false;
	NextAttackReady = false;
	AttackDamaging = false;
	AttackIndex = 0;

	PassiveMovementSpeed = 450.0f;
	CombatMovementSpeed = 250.0f;
	GetCharacterMovement()->MaxWalkSpeed = PassiveMovementSpeed;
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	EnemyDetectionCollider->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OnEnemyDetectionBeginOverlap);
	EnemyDetectionCollider->OnComponentEndOverlap.AddDynamic(this, &APlayerCharacter::OnEnemyDetectionEndOverlap);

	TSet<AActor*> NearActors;
	EnemyDetectionCollider->GetOverlappingActors(NearActors);

	for (auto& EnemyActor : NearActors)
	{
		if (Cast<AEnemyBase>(EnemyActor))
		{
			NearbyEnemies.Add(EnemyActor);
		}
	}
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FocusTarget();

	if (Rolling)
	{
		AddMovementInput(GetActorForwardVector(), 600 * GetWorld()->GetDeltaSeconds());
	}
	else if (Stumbling && MovingBackwards)
	{
		AddMovementInput(-GetActorForwardVector(), 40.0f * GetWorld()->GetDeltaSeconds());
	}
	else if (Attacking && AttackDamaging)
	{
		TSet<AActor*>WeaponOverlappingActors;
		Weapon->GetOverlappingActors(WeaponOverlappingActors);

		for (AActor* HitActor : WeaponOverlappingActors)
		{
			// making sure character doesn't hitr himself
			if (HitActor == this)
			{
				continue;
			}

			if (!AttackHitActors.Contains(HitActor))
			{
				float AppliedDamage = UGameplayStatics::ApplyDamage(HitActor, 1.0f, GetController(), this, UDamageType::StaticClass());

				if (AppliedDamage > 0.0f)
				{
					AttackHitActors.Add(HitActor);

					GetWorld()->GetFirstPlayerController()->PlayerCameraManager->StartCameraShake(CameraShakeMinor);
				}
			}
		}
	}

	if (Target != NULL && TargetLocked)
	{
		FVector TargetDirection = Target->GetActorLocation() - GetActorLocation();

		if (TargetDirection.Size2D() > 400)
		{
			FRotator Difference = UKismetMathLibrary::NormalizedDeltaRotator
			(Controller->GetControlRotation(), TargetDirection.ToOrientationRotator());

			if (FMath::Abs(Difference.Yaw) > 30.0f)
			{
				AddControllerYawInput(DeltaTime * -Difference.Yaw * 0.5f);
			}
		}
	}
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	//Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("CombatModeToggle", IE_Pressed, this,
		&APlayerCharacter::ToggleCombatMode);
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this,
		&APlayerCharacter::Attack);
	PlayerInputComponent->BindAction("Roll", IE_Pressed, this,
		&APlayerCharacter::Roll);
	PlayerInputComponent->BindAction("CycleTarget+", IE_Pressed, this,
		&APlayerCharacter::CycleTargetClockwise);
	PlayerInputComponent->BindAction("CycleTarget-", IE_Pressed, this,
		&APlayerCharacter::CycleTargetCounterClockwise);
}

void APlayerCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void APlayerCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void APlayerCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f) && !Attacking && !Rolling && !Stumbling)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		AddMovementInput(Direction, Value);
	}

	InputDirection.X = Value;
}

void APlayerCharacter::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f) && !Attacking && !Rolling && !Stumbling)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(Direction, Value);
	}

	InputDirection.Y = Value;
}

void APlayerCharacter::CycleTarget(bool Clockwise)
{
	AActor* SuitableTarget = NULL;

	if (Target)
	{
		FVector CameraLocation = Cast<APlayerController>(GetController())->PlayerCameraManager->GetCameraLocation();

		FRotator TargetDirection = (Target->GetActorLocation() - CameraLocation).ToOrientationRotator();

		float BestYawDifference = INFINITY;

		for (auto& NearEnemy : NearbyEnemies)
		{
			if (NearEnemy == Target)
			{
				continue;
			}

			FVector NearEnemyDirection = NearEnemy->GetActorLocation() - CameraLocation;
			FRotator Difference = UKismetMathLibrary::NormalizedDeltaRotator(NearEnemyDirection.ToOrientationRotator(), TargetDirection);

			if ((Clockwise && Difference.Yaw <= 0.0f) || (!Clockwise && Difference.Yaw >= 0.0f))
			{
				continue;
			}

			float YawDifference = FMath::Abs(Difference.Yaw);
			if (YawDifference < BestYawDifference)
			{
				BestYawDifference = YawDifference;
				SuitableTarget = NearEnemy;
			}
		}
	}
	else
	{
		float BestDistance = INFINITY;

		for (auto& NearEnemy : NearbyEnemies)
		{
			float Distance = FVector::Dist(GetActorLocation(), NearEnemy->GetActorLocation());
			if (Distance < BestDistance)
			{
				BestDistance = Distance;
				SuitableTarget = NearEnemy;
			}
		}
	}

	if (SuitableTarget != NULL)
	{
		Target = SuitableTarget;

		if (!TargetLocked)
		{
			SetInCombat(true);
		}
	}
}

void APlayerCharacter::CycleTargetClockwise()
{ 
	CycleTarget(true);
}

void APlayerCharacter::CycleTargetCounterClockwise()
{
	CycleTarget(false);
}

void APlayerCharacter::LookAtSmooth()
{
	if (!Rolling)
	{
		Super::LookAtSmooth();
	}
}

float APlayerCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (DamageCauser == this || Rolling)
	{
		return 0.0f;
	}

	EndAttack();
	SetMovingBackwards(false);
	SetMovingForward(false);
	Stumbling = true;

	int AnimationIndex;

	do
	{
		AnimationIndex = FMath::RandRange(0, TakeHit_StumbleBackwards.Num() - 1);
	} while (AnimationIndex == LastStumbleIndex);

	PlayAnimMontage(TakeHit_StumbleBackwards[AnimationIndex]);
	LastStumbleIndex = AnimationIndex;

	FVector Direction = DamageCauser->GetActorLocation() - GetActorLocation();
	Direction - FVector(Direction.X, Direction.Y, 0);

	FRotator Rotation = FRotationMatrix::MakeFromX(Direction).Rotator();

	SetActorRotation(Rotation);

	return DamageAmount;
}

void APlayerCharacter::OnEnemyDetectionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<AEnemyBase>(OtherActor) && !NearbyEnemies.Contains(OtherActor))
	{
		NearbyEnemies.Add(OtherActor);
	}
}

void APlayerCharacter::OnEnemyDetectionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<AEnemyBase>(OtherActor) && NearbyEnemies.Contains(OtherActor))
	{
		NearbyEnemies.Remove(OtherActor);
	}
}

void APlayerCharacter::Attack()
{
	if ((!Attacking || NextAttackReady) && !Rolling && !Stumbling && !GetCharacterMovement()->IsFalling())
	{
		Super::Attack();

		if (AttackIndex >= Attacks.Num())
		{
			AttackIndex = 0;
		}

		PlayAnimMontage(Attacks[AttackIndex++]);
	}
}

void APlayerCharacter::EndAttack()
{
	Super::EndAttack();

	AttackIndex = 0;
}

void APlayerCharacter::Roll()
{
	if (Rolling || Stumbling)
	{
		return;
	}

	EndAttack();

	if (InputDirection != FVector::ZeroVector)
	{
		FRotator PlayerRotZeroPitch = Controller->GetControlRotation();
		PlayerRotZeroPitch.Pitch = 0;

		FVector PlayerRight = FRotationMatrix(PlayerRotZeroPitch).GetUnitAxis(EAxis::Y);
		FVector PlayerForward = FRotationMatrix(PlayerRotZeroPitch).GetUnitAxis(EAxis::X);

		FVector DodgeDir = PlayerForward * InputDirection.X + PlayerRight * InputDirection.Y;

		RollRotation = DodgeDir.ToOrientationRotator();
	}
	else
	{
		RollRotation = GetActorRotation();
	}

	SetActorRotation(RollRotation);

	PlayAnimMontage(CombatRoll);
	Rolling = true;
}

void APlayerCharacter::StartRoll()
{
	Rolling = true;

	GetCharacterMovement()->MaxWalkSpeed = 600.0f;

	EndAttack();
}

void APlayerCharacter::EndRoll()
{
	Rolling = false;

	GetCharacterMovement()->MaxWalkSpeed = TargetLocked ? CombatMovementSpeed : PassiveMovementSpeed;
}

void APlayerCharacter::RollRotateSmooth()
{
	FRotator SmoothedRotation = FMath::Lerp(GetActorRotation(), RollRotation, RotationSmoothing * GetWorld()->DeltaTimeSeconds);

	SetActorRotation(SmoothedRotation);
}

void APlayerCharacter::FocusTarget()
{
	if (Target != NULL)
	{
		if (FVector::Dist(GetActorLocation(), Target->GetActorLocation()) >= TargetLockDistance)
		{
			ToggleCombatMode();
		}
	}
}

void APlayerCharacter::ToggleCombatMode()
{
	if (!TargetLocked)
	{
		CycleTarget();
	}
	else
	{
		SetInCombat(false);
	}
}

void APlayerCharacter::SetInCombat(bool InCombat)
{
	TargetLocked = InCombat;

	GetCharacterMovement()->bOrientRotationToMovement = !TargetLocked;
	GetCharacterMovement()->MaxWalkSpeed = TargetLocked ? CombatMovementSpeed : PassiveMovementSpeed;

	if (!TargetLocked)
	{
		Target = NULL;
	}
}

