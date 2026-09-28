// Fill out your copyright notice in the Description page of Project Settings.


#include "TriggerComponent.h"


UTriggerComponent::UTriggerComponent() 
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTriggerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (moverActor != nullptr)
	{
		Mover = moverActor->FindComponentByClass<UMover>();
		if (Mover != nullptr) {
			if (isPressurePlate) {
				OnComponentBeginOverlap.AddDynamic(this, &UTriggerComponent::OnOverlapBegin);
				OnComponentEndOverlap.AddDynamic(this, &UTriggerComponent::OnOverlapEnd);
			}
		}
	}
	

}

void UTriggerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UTriggerComponent::Trigger(bool shouldMoveC)
{
	Mover->shouldMove = shouldMoveC;
}


void UTriggerComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	activatorCount++;
	moverMovementTrigger(true, OtherActor);
}

void UTriggerComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	moverMovementTrigger(false, OtherActor);
	activatorCount--;
}

void UTriggerComponent::moverMovementTrigger(bool shouldMoveB, AActor* overlappingActor) {
	if (Mover && overlappingActor && overlappingActor->ActorHasTag("PressurePlateActivator")  && activatorCount != 0)
	{
		Trigger(shouldMoveB);
	}
}
