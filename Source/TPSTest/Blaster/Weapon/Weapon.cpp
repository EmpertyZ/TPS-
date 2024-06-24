// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "TPSTest/Blaster/Character/BlasterCharacter.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	//设置该Actor启用网络同步
	bReplicates = true;

	//初始化绑定组件
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	//设置武器碰撞，在服务器才控制他的碰撞，默认设无
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//绑定胶囊网格体组件,设置胶囊体碰撞，在服务器才控制器碰撞，默认为无
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//绑定拾取UMG
	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
	
	//设置武器碰撞,判断当前是否是服务器上，如果是就启用碰撞
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		//绑定重叠函数委托
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
		//绑定结束重叠函数委托
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::EndSphereOverlap);
	}
	
}

//胶囊重叠函数
void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		//调用Character的SetOverlappingWeapon,设置武器
		BlasterCharacter->SetOverlappingWeapon(this);
	}
	
}

void AWeapon::EndSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		//调用Character的SetOverlappingWeapon,设置null指针
		BlasterCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::ShowPickupWidget(bool bWasShowPickup)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bWasShowPickup);
	}
}

//设置武器状态
void AWeapon::SetWeaponState(EWeaponState NewWeaponState)
{
	WeaponState = NewWeaponState;
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		//执行到这里的时候已经装备了武器，需要把重叠UI显示给关掉
		ShowPickupWidget(false);
		//通过获取的武器调用其碰撞胶囊
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	}

}

//开火方法
void AWeapon::Fire(const FVector& HitTarget)
{
	//武器播放开火动画
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	//获取武器生成子弹socket
	const USkeletalMeshSocket* AmmoEjectSocket = GetWeaponMesh()->GetSocketByName(FName("AmmoEject"));
	//生成子弹壳
	if (AmmoEjectSocket)
	{
		//通过socket获取变换
		FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(GetWeaponMesh());
		if (CasingClass)
		{
			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<ACasing>(CasingClass, SocketTransform.GetLocation(), SocketTransform.GetRotation().Rotator());
			}
		}
	}
}

//绑定武器状态复制的方法,当武器状态更新时调用该方法
void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped://当武器为被装备的状态时，隐藏显示UI，设置武器胶囊碰撞为无碰撞
		ShowPickupWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		ShowPickupWidget(true);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

//需要复制的属性方法
void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//添加需要被复制的属性
	DOREPLIFETIME(AWeapon, WeaponState);
}

