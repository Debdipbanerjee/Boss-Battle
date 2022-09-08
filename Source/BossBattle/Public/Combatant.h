// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Combatant.generated.h"

UCLASS()
class BOSSBATTLE_API ACombatant : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACombatant();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	AActor* Target;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool TargetLocked;

	bool Attacking;
	bool AttackDamaging;
	bool MovingForward;
	bool MovingBackwards;
	bool NextAttackReady;
	bool Stumbling;

	bool RotateTowardsTarget;

	UPROPERTY(EditAnywhere, Category = "Animation")
	float RotationSmoothing;

	UPROPERTY(EditAnywhere, Category = "Animations")
	TArray<UAnimMontage*> AttackAnimations;

	UPROPERTY(EditAnywhere, Category = "Animations")
	TArray<UAnimMontage*> TakeHit_StumbleBackwards;

	// Actors hit with the last attack - Used to stop duplicate hits
	TArray<AActor*> AttackHitActors;

	virtual void Attack();

	// anim called: rotate and jump towards target
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void AttackLunge();

	// anim called: rotate and jump towards target
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void EndAttack();

	// set if weapon applies damage
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void SetAttackDamaging(bool Damaging);

	// anim called: set if moving forward
	UFUNCTION(BlueprintCallable, Category = "Animation")
	virtual void SetMovingForward(bool IsMovingForward);

	// anim called: set if moving backwards
	UFUNCTION(BlueprintCallable, Category = "Animation")
	virtual void SetMovingBackwards(bool IsMovingBackwards);

	// anim called: set if moving backwards
	UFUNCTION(BlueprintCallable, Category = "Animation")
	virtual void EndStumble();

	// called by anim to singlat that the next attack is potentially allowed
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void AttackNextReady();

	virtual void LookAtSmooth();

	// anim called: get rate of actors look rotation
	UFUNCTION(BlueprintCallable, Category = "Animation")
	float GetCurrentRotationSpeed();

	float LastRotationSpeed;
};
