// Fill out your copyright notice in the Description page of Project Settings.


#include "Casing.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

// Sets default values
ACasing::ACasing()
{
	PrimaryActorTick.bCanEverTick = true;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);
	CasingMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	CasingMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	CasingMesh->SetSimulatePhysics(true);//启用物理
	CasingMesh->SetEnableGravity(true);//启用重力
	CasingMesh->SetNotifyRigidBodyCollision(true);
	ShowEjectionImpulse = 10.f;
	
}

void ACasing::BeginPlay()
{
	Super::BeginPlay();
	//添加子弹壳冲击力，是其能够弹出武器
	CasingMesh->AddImpulse(GetActorForwardVector() * ShowEjectionImpulse);
	CasingMesh->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);
}

void ACasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (ShellSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
	}
	Destroy();
}

void ACasing::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

