// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKartMovementComponent.h"

#include "GoKart.h"
#include "GameFramework/GameStateBase.h"

UGoKartMovementComponent::UGoKartMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	Velocity = FVector::ZeroVector;

	Mass = 2000.f;
	MaxDrivingForce = 30000;
	MinTurningRadius = 15;
	DragCoefficient = 15;
	RollingResistanceCoefficient = 0.015f;

	Throttle = 0.f;
	SteeringThrow = 0.f;
}


void UGoKartMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UGoKartMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetOwnerRole() == ROLE_AutonomousProxy || GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
	{
		LastMove = CreateMove(DeltaTime);
		SimulateMove(LastMove);
	}
}

void UGoKartMovementComponent::SetThrottle(float Value) 
{
	Throttle = Value;
}

float UGoKartMovementComponent::GetThrottle() const
{
	return Throttle;
}

void UGoKartMovementComponent::SetSteering(float Value) 
{
	SteeringThrow = Value;
}

float UGoKartMovementComponent::GetSteering() const
{
	return SteeringThrow;
}

void UGoKartMovementComponent::UpdateTransform(float DeltaTime)
{
	FVector Torque = GetOwner()->GetActorForwardVector() * MaxDrivingForce * Throttle;
	FVector Force = Torque + CalculateAirResistance() + CalculateRollingResistance();
	FVector Acceleration = Force / Mass;

	float AccelerationDueToGravity = GetWorld()->GetGravityZ();
	Velocity = Velocity + Acceleration * DeltaTime;

	ApplyRotation(DeltaTime);

	UpdateLocationFromVelocity(DeltaTime);
}

void UGoKartMovementComponent::SimulateMove(const FGoKartMove& Move)
{
	Throttle = Move.Throttle;
	SteeringThrow = Move.SteeringThrow;
	UpdateTransform(Move.DeltaTime);
}

FGoKartMove UGoKartMovementComponent::CreateMove(float DeltaTime) 
{
	FGoKartMove Move;
	Move.DeltaTime = DeltaTime;
	Move.SteeringThrow = SteeringThrow;
	Move.Throttle = Throttle;
	Move.Time = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	return Move;
}

void UGoKartMovementComponent::UpdateLocationFromVelocity(float DeltaTime)
{
	FVector Translation = Velocity * DeltaTime * 100;

	FHitResult HitResult;
	GetOwner()->AddActorWorldOffset(Translation, true, &HitResult);

	if (HitResult.IsValidBlockingHit())
	{
		Velocity *= 0.6f;
	}
}

void UGoKartMovementComponent::ApplyRotation(float DeltaTime) 
{
	float DeltaLocation = FVector::DotProduct(GetOwner()->GetActorForwardVector(), Velocity) * DeltaTime;
	float RotationAngle = FMath::RadiansToDegrees(DeltaLocation / MinTurningRadius)  * SteeringThrow;
	FQuat RotationDelta(GetOwner()->GetActorUpVector(), FMath::DegreesToRadians(RotationAngle));
	GetOwner()->AddActorWorldRotation(RotationDelta);
	Velocity = RotationDelta.RotateVector(Velocity);
}

FVector UGoKartMovementComponent::CalculateAirResistance() 
{
	return -Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
}

FVector UGoKartMovementComponent::CalculateRollingResistance() 
{
	return -Velocity.GetSafeNormal() * RollingResistanceCoefficient * -GetWorld()->GetGravityZ();
}