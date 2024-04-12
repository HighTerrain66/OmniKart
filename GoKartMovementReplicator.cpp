// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKartMovementReplicator.h"

#include "GoKart.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Public/DrawDebugHelpers.h"

UGoKartMovementReplicator::UGoKartMovementReplicator()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UGoKartMovementReplicator::BeginPlay()
{
	Super::BeginPlay();

	MovementComponent = GetOwner()->FindComponentByClass<UGoKartMovementComponent>();
	MeshOffsetRoot = Cast<AGoKart>(GetOwner())->GetMeshOffsetRoot();
	// ...
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		FGoKartMove Move;
		Move.Throttle = 0.f;
		Move.SteeringThrow = 0.f;
		Move.DeltaTime = 0.f;
		Move.Time = 0.f;
		Server_SendMove(Move);
	}
}

void UGoKartMovementReplicator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGoKartMovementReplicator, ServerState);
	//DOREPLIFETIME(AGoKart, Throttle);
	//DOREPLIFETIME(AGoKart, SteeringThrow);
}

FString GetEnumText(ENetRole Role)
{
	switch (Role)
	{
	case ROLE_None:
		return "None";
	case ROLE_SimulatedProxy:
		return "SimulatedProxy";
	case ROLE_AutonomousProxy:
		return "AutonomousProxy";
	case ROLE_Authority:
		return "Authority";
	default:
		return "ERROR";
	}
}

// Called every frame
void UGoKartMovementReplicator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (MovementComponent == nullptr) return;

	FGoKartMove LastMove = MovementComponent->GetLastMove();

	// Actor is controlled by a client
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		//The transform is updated in the movementcomponent for locally controlled pawns
		//MovementComponent->UpdateTransform(DeltaTime);

		//FGoKartMove Move = MovementComponent->CreateMove(DeltaTime);
		UnacknowledgedMoves.Add(LastMove);
		Server_SendMove(LastMove);
	}

	// Actor is directly controlled by the server
	if (GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
	{
		//FGoKartMove Move = MovementComponent->CreateMove(DeltaTime);
		//Server_SendMove(Move);
		UpdateServerState(LastMove);
	}

	// Actor is being simulated 
	if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		//MovementComponent->SimulateMove(ServerState.LastMove);
		ClientTick(DeltaTime);
	}
	DrawDebugString(GetWorld(), FVector(0, 0, 100), GetEnumText(GetOwnerRole()), GetOwner(), FColor::Red, DeltaTime);
	// ...
}

void UGoKartMovementReplicator::ClientTick(float DeltaTime) 
{
	ClientTimeSinceUpdate += DeltaTime;

	if (ClientTimeBetweenLastUpdates < KINDA_SMALL_NUMBER) return;
	if (MovementComponent == nullptr) return;

	float Alpha = ClientTimeSinceUpdate / ClientTimeBetweenLastUpdates;

	FHermiteCubicSpline Spline = CreateSpline();

	MeshOffsetRoot->SetWorldLocation(Spline.InterpolateLocation(Alpha));

	FVector NewDerivative = Spline.InterpolateDerivative(Alpha);
	FVector NewVelocity = NewDerivative / VelocityToDerivative();
	MovementComponent->SetVelocity(NewVelocity);

	MeshOffsetRoot->SetWorldRotation(FQuat::Slerp(ClientTransformAtLastUpdate.GetRotation(), ServerTransformAtLastUpdate.GetRotation(), Alpha));
}

FHermiteCubicSpline UGoKartMovementReplicator::CreateSpline() 
{
	FHermiteCubicSpline Spline;
	Spline.StartLocation = ClientTransformAtLastUpdate.GetLocation();
	Spline.TargetLocation = ServerState.Transform.GetLocation();
	Spline.StartDerivative = ClientStartVelocity * VelocityToDerivative();
	Spline.TargetDerivative = ServerState.Velocity * VelocityToDerivative();
	return Spline;
}

void UGoKartMovementReplicator::OnRep_ServerState() 
{
	switch (GetOwnerRole())
	{
	case ROLE_AutonomousProxy:
		AutonomousProxy_OnRep_ServerState();
		break;
	case ROLE_SimulatedProxy:
		SimulatedProxy_OnRep_ServerState();
		break;
	default:
		break;
	
	}
}

void UGoKartMovementReplicator::SimulatedProxy_OnRep_ServerState() 
{
	GetOwner()->SetActorTransform(ServerState.Transform);

	ClientTimeBetweenLastUpdates = ClientTimeSinceUpdate;
	ClientTimeSinceUpdate = 0;
	if (MeshOffsetRoot != nullptr)
	{
		ClientTransformAtLastUpdate = MeshOffsetRoot->GetComponentTransform();
		if (MovementComponent != nullptr)
		{
			ClientStartVelocity = MovementComponent->GetVelocity();
		}
	}
	ServerTransformAtLastUpdate = ServerState.Transform;
}

void UGoKartMovementReplicator::AutonomousProxy_OnRep_ServerState() 
{
	if (MovementComponent == nullptr) {return;}

	GetOwner()->SetActorTransform(ServerState.Transform);
	MovementComponent->SetVelocity(ServerState.Velocity);
	ClearAcknowledgedMoves(ServerState.LastMove);

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		MovementComponent->SimulateMove(Move);
	}
}

void UGoKartMovementReplicator::Server_SendMove_Implementation(FGoKartMove Move)
{
	if (MovementComponent == nullptr) return;
	MovementComponent->SimulateMove(Move);
	UpdateServerState(Move);
	
	//ReplicatedThrottle = Throttle;
	//TODO: Update last move
}

bool UGoKartMovementReplicator::Server_SendMove_Validate(FGoKartMove Move)
{
	if (!Move.IsValid()) return false;
	float ProposedSimulatedTime = ClientSimulatedTime + Move.DeltaTime;
	if (ProposedSimulatedTime > GetWorld()->GetTimeSeconds()) return false;
	ClientSimulatedTime = ProposedSimulatedTime;
	return true;
	
}

void UGoKartMovementReplicator::ClearAcknowledgedMoves(FGoKartMove LastMove) 
{
	TArray<FGoKartMove> NewMoves;

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		if (Move.Time > LastMove.Time)
		{
			NewMoves.Add(Move);
		}
	}

	UnacknowledgedMoves = NewMoves;
}

void UGoKartMovementReplicator::UpdateServerState(const FGoKartMove& Move) 
{
	ServerState.LastMove = Move;
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = MovementComponent->GetVelocity();
}


