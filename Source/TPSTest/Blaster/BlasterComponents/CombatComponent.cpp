// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "TPSTest/Blaster/Character/BlasterCharacter.h"
#include "TPSTest/Blaster/Weapon/Weapon.h"

#define TRACE_LENGTH 80000.f;

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 450.f;
}


// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}
	
	
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	//提前设置瞄准状态，以防网络差的时候，没同步到服务端的瞄准，导致延迟不流畅问题
	bAiming = bIsAiming;
	//向服务器同步瞄准状态
	ServerSetAiming(bIsAiming);
	if (Character)
	{
		//判断当前是否为瞄准状态，如果是就使用瞄准移动速度，不是就使用基础移动速度
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

//设置RPC同步方法:设置bAiming
void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character)
	{
		//判断当前是否为瞄准状态，如果是就使用瞄准移动速度，不是就使用基础移动速度
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

//开火方法
void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed)
	{
		//开火获取当前集中结果
		FHitResult TraceHitResult;
		TraceUnderCrosshairs(TraceHitResult);
		//调用服务端开火方法，将开火同步到服务端
		ServerFire(TraceHitResult.ImpactPoint);
	}
}
//服务端开火方法，开火需要多端同步，先把开火同步到服务端，然后在通过服务端循行netMulticast同步到各个客户端
void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	//调用服务端开火后，在从服务端同步到其他客户端
	MulticastFire(TraceHitTarget);
}
//服务端开火方法，开火需要多端同步，先把开火同步到服务端，然后在通过服务端循行netMulticast同步到各个客户端
void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;
	if (Character)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

//准心射击检测
void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	//获取屏幕尺寸
	FVector2d ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	//通过屏幕尺寸获取准心位置
	FVector2d CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	//将2d位置转换为3d的position和direction
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);
	//判断是否获取到转换后的3d位置及方向
	if (bScreenToWorld)
	{
		//获取转换后的3d位置和方向后，发射射线检测
		FVector Start = CrosshairWorldPosition;
		FVector End = CrosshairWorldPosition + CrosshairWorldDirection * TRACE_LENGTH;
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECC_Visibility
		);


	}
}

//装备武器方法
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	//先检查是否又玩家且是否有武器传入
	if (Character == nullptr || WeaponToEquip == nullptr) return;
	EquippedWeapon = WeaponToEquip;
	//设置武器为，装备武器状态
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	//绑定武器到Character骨骼上
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
	//绑定武器后，设置武器所有者,SetOwner已经拥有网络同步了
	EquippedWeapon->SetOwner(Character);
	//装备武器后设置角色移动组件:禁止跟随旋转运动(角色移动时不会跟随方向键转向，始终面向前方)
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	//装备武器后设置角色朝向始终跟随相机旋转
	Character->bUseControllerRotationYaw = true;
}

//武器被装备或被卸下时(装备动作在服务器执行)更新相应动作
void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		//装备武器后设置角色移动组件:禁止跟随旋转运动(角色移动时不会跟随方向键转向，始终面向前方)
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		//装备武器后设置角色朝向始终跟随相机旋转
		Character->bUseControllerRotationYaw = true;
	}
}


// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	//获取准心检测到的目标
	FHitResult TraceHitResult;
	TraceUnderCrosshairs(TraceHitResult);
}

//添加需要复制的属性:武器
void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
}

