// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKart.h"

#include "Engine/World.h"

#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameStateBase.h"
#include "GoKartMovementComponent.h"
#include "GoKartMovementReplicator.h"


// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Replicates this pawn from the server state
	bReplicates = true;
	SetReplicateMovement(false);

	Collider = CreateDefaultSubobject<UBoxComponent>(TEXT("Root"));
	RootComponent = Collider;

	// Car mesh
	MeshOffsetRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MeshOffsetRoot"));
	MeshOffsetRoot->SetupAttachment(RootComponent);
	BaseMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> CarMesh(TEXT("/Game/Vehicle/Sedan/Sedan_SkelMesh.Sedan_SkelMesh"));
	BaseMesh->SetSkeletalMesh(CarMesh.Object);
	BaseMesh->SetupAttachment(MeshOffsetRoot);

	MovementComponent = CreateDefaultSubobject<UGoKartMovementComponent>("Movement Component");
	MovementComponent->SetIsReplicated(false);
	MovementReplicator = CreateDefaultSubobject<UGoKartMovementReplicator>("Replicator Component");
	MovementReplicator->SetIsReplicated(true);

	// Create a spring arm component
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->TargetOffset = FVector(0.f, 0.f, 200.f);
	SpringArm->SetRelativeRotation(FRotator(-15.f, 0.f, 0.f));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 500.0f;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 7.f;
	//SpringArm->bInheritPitch = false;
	//SpringArm->bInheritRoll = false;

	// Create camera component 
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	Camera->FieldOfView = 90.f;

	if (HasAuthority())
	{
		NetUpdateFrequency = 1;
	}
	
}

void AGoKart::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &AGoKart::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGoKart::Steer);
	PlayerInputComponent->BindAxis("LookUp");
	PlayerInputComponent->BindAxis("LookRight");
}

void AGoKart::MoveForward(float Value) 
{
	MovementComponent->SetThrottle(Value);
}

void AGoKart::Steer(float Value) 
{
	MovementComponent->SetSteering(Value);
}