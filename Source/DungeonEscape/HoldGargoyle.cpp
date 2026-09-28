// Fill out your copyright notice in the Description page of Project Settings.


#include "HoldGargoyle.h"

// Sets default values for this component's properties
UHoldGargoyle::UHoldGargoyle()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHoldGargoyle::BeginPlay()
{
	Super::BeginPlay();

	gargoyle = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
	
}


// Called every frame
void UHoldGargoyle::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	visibleGargoyle();
	// ...
}

void UHoldGargoyle::visibleGargoyle()
{
	if (gargoyle) {
		gargoyle->SetVisibility(true);
	}
}

