// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementComponent.generated.h"

USTRUCT()
struct FGoKartMove
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	float Throttle = 0.f;
	UPROPERTY()
	float SteeringThrow = 0.f;
	UPROPERTY()
	float DeltaTime = 0.f;
	UPROPERTY()
	float Time = 0.f;

	bool IsValid() const
	{
		return FMath::Abs(Throttle) <= 1 && FMath::Abs(SteeringThrow) <= 1;
	};
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class OMNIKART_API UGoKartMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UGoKartMovementComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void UpdateTransform(float DeltaTime);
	void SimulateMove(const FGoKartMove& Move);
	FGoKartMove CreateMove(float DeltaTime);

	void SetThrottle(float Value);
	float GetThrottle() const;

	void SetSteering(float Value);
	float GetSteering() const;

	FVector Velocity;
	void SetVelocity(FVector Value) {Velocity = Value;};
	FVector GetVelocity() { return Velocity; };
	
	FGoKartMove GetLastMove() {return LastMove;};
	
private:

	FVector CalculateAirResistance();
	FVector CalculateRollingResistance();
	
	void UpdateLocationFromVelocity(float DeltaTime);
	void ApplyRotation(float DeltaTime);
	// CAR STATS

	//The mass of the car in kg
	UPROPERTY(EditAnywhere)
	float Mass;

	UPROPERTY(EditAnywhere)
	float MaxDrivingForce;
	UPROPERTY(EditAnywhere)
	float MinTurningRadius;
	UPROPERTY(EditAnywhere)
	float DragCoefficient;

	UPROPERTY(EditAnywhere)
	float RollingResistanceCoefficient;

	
	float Throttle;
	float SteeringThrow;

	FGoKartMove LastMove;
	
};
