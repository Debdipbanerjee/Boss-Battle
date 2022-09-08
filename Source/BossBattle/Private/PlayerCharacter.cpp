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

	EnemyDetectionCollider->OnComponentBeginOverlap.
	AddDynamic(this, &APlayerCharacter::OnEnemyDetectionBeginOverlap);
	EnemyDetectionCollider->OnComponentEndOverlap.
	AddDynamic(this, &APlayerCharacter::OnEnemyDetectionEndOverlap);
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
}

void APlayerCharacter::LookUpAtRate(float Rate)
{
}

void APlayerCharacter::MoveForward(float Value)
{
}

void APlayerCharacter::MoveRight(float Value)
{
}

void APlayerCharacter::CycleTarget(bool Clockwise)
{
}

void APlayerCharacter::CycleTargetClockwise()
{
}

void APlayerCharacter::CycleTargetCounterClockwise()
{
}

void APlayerCharacter::LookAtSmooth()
{
}

float APlayerCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	return 0.0f;
}

void APlayerCharacter::OnEnemyDetectionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void APlayerCharacter::OnEnemyDetectionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void APlayerCharacter::Attack()
{
}

void APlayerCharacter::EndAttack()
{
}

void APlayerCharacter::Roll()
{
}

void APlayerCharacter::StartRoll()
{
}

void APlayerCharacter::EndRoll()
{
}

void APlayerCharacter::RollRotateSmooth()
{
}

void APlayerCharacter::FocusTarget()
{
}

void APlayerCharacter::ToggleCombatMode()
{
}

void APlayerCharacter::SetInCombat(bool InCombat)
{
}

