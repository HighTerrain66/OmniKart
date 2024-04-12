// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"



UCLASS()
class OMNIKART_API AGoKart : public APawn
{
	GENERATED_BODY()

public:

	UPROPERTY(VisibleDefaultsOnly)
	class UGoKartMovementComponent* MovementComponent;

	UPROPERTY(VisibleDefaultsOnly)
	class UGoKartMovementReplicator* MovementReplicator;

	UPROPERTY(VisibleDefaultsOnly)
	class USceneComponent* MeshOffsetRoot;

	/** Spring arm that will offset the camera */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArm;

	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;

	

	


	// Sets default values for this pawn's properties
	AGoKart();


	USceneComponent* GetMeshOffsetRoot() { return MeshOffsetRoot;};

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	float Speed = 0.f;
	UPROPERTY(EditAnywhere)
	class USkeletalMeshComponent* BaseMesh;

	UPROPERTY(EditDefaultsOnly)
	class UBoxComponent* Collider;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:

	UFUNCTION()
	void MoveForward(float Value);

	UFUNCTION()
	void Steer(float Value);

	
};
