// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class AWeapon;
class ABlasterCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TPSTEST_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	//添加需要复制的属性:武器
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void SetAiming(bool bIsAiming);
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming); //设置RPC同步方法:设置bAiming

	//开火方法
	void FireButtonPressed(bool bPressed);

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);//服务端开火方法，开火需要多端同步，先把开火同步到服务端，然后在通过服务端循行netMulticast同步到各个客户端

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);//多端开火同步

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);//准心射击检测
public:
	//添加友元类，友元类或友元函数可以调用类的private成员
	friend class ABlasterCharacter;

	//装备武器方法
	void EquipWeapon(AWeapon* WeaponToEquip);

	//武器被装备或被卸下时(装备动作在服务器执行)更新相应动作
	UFUNCTION()
	void OnRep_EquippedWeapon();

private:
	ABlasterCharacter* Character;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon; //因为需要知道当前武器的状态和其他属性，所以需要将其设为网络同步

	UPROPERTY(Replicated)
	bool bAiming; //判断是否处于瞄准状态

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;//基础移动速度
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;//瞄准移动速度

	bool bFireButtonPressed;//判断是否处于开火状态

}
;
