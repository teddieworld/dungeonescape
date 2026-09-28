// Copyright Epic Games, Inc. All Rights Reserved.

#include "DungeonEscapeCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DungeonEscape.h"
#include "HoldGargoyle.h"
#include "CollectableItem.h"
#include "Lock.h"

ADungeonEscapeCharacter::ADungeonEscapeCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;

	USceneComponent* capsuleRoot = GetRootComponent();


	Gargoyle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Gargoyle Statue"));
	Gargoyle->SetupAttachment(capsuleRoot);
}

void ADungeonEscapeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ADungeonEscapeCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ADungeonEscapeCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADungeonEscapeCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADungeonEscapeCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ADungeonEscapeCharacter::LookInput);

		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ADungeonEscapeCharacter::Interact);
	}
	else
	{
		UE_LOG(LogDungeonEscape, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ADungeonEscapeCharacter::Interact()
{
	FVector start = FirstPersonCameraComponent->GetComponentLocation();
	FVector end = start + (FirstPersonCameraComponent->GetForwardVector() * maxInteractionDistance);
	DrawDebugLine(GetWorld(), start, end, FColor::Red, false, 5.0f);

	FCollisionShape InteractionSphere = FCollisionShape::MakeSphere(interactionSphereRadius);
	DrawDebugSphere(GetWorld(), end, interactionSphereRadius, 20, FColor::Red, false, 5.0f);

	FHitResult HitResult;
	
	bool hasHit = GetWorld()->SweepSingleByChannel(HitResult, start, end, FQuat::Identity, ECC_GameTraceChannel2, InteractionSphere);

	if (hasHit) {
		AActor* HitActor = HitResult.GetActor();
		if (HitActor->ActorHasTag("CollectableItem")) {
			ACollectableItem* CollectableItem = Cast<ACollectableItem>(HitActor);
			
			if (CollectableItem) {
				inventory.Add(CollectableItem->itemName);
				if (Gargoyle) {
					Gargoyle->SetVisibility(true);
				}
				CollectableItem->Destroy();
			}
		}

		else if (HitActor->ActorHasTag("Lock")) {
			ALock* Lock = Cast<ALock>(HitActor);
			
			if (Lock){
				if (!Lock->getKeyPlaced()) {
					int32 itemsRemoved = inventory.RemoveSingle(Lock->KeyItemName);
					if (itemsRemoved == 1) {
						Lock->setKeyPlaced(true);
					}
					if (Gargoyle) {
						Gargoyle->SetVisibility(false);
					}
				}
				else {
					inventory.Add(Lock->KeyItemName);
					Lock->setKeyPlaced(false);
				}
			}
		}
	}
}


void ADungeonEscapeCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}

void ADungeonEscapeCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void ADungeonEscapeCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ADungeonEscapeCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void ADungeonEscapeCharacter::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void ADungeonEscapeCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}
