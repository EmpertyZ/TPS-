// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "TPSTest/Blaster/BlasterTypes/TruningInPlasce.h"
#include "GameFramework/Character.h"
#include "TPSTest/Blaster/BlasterComponents/CombatComponent.h"
#include "BlasterCharacter.generated.h"

UCLASS()
class TPSTEST_API ABlasterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//需要复制的属性方法
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//当类和组件注册完成时就会调用该函数，用于组件初始化
	virtual void PostInitializeComponents() override;

	//播放开火蒙太奇
	void PlayFireMontage(bool bAiming);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	//移动方法
	void Move(const FInputActionValue& Value);
	//左右看方法
	void Look(const FInputActionValue& Value);
	//装备武器方法
	void EquipButtonPressed();
	//蹲伏方法
	void CrouchButtonPressed();
	//开始瞄准方法
	void AimButtonPressed();
	//结束瞄准方法
	void AimButtonReleased();
	//开枪方法
	void FireButtonPressed();
	//停火方法
	void FireButtonReleased();
	void UpdateBaseAimYawForHasAuthority();
	//计算瞄准偏移方法
	void AimOffset(float DeltaTime);


	
private:
	UPROPERTY(VisibleAnywhere, Category=Camera)
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, Category=Camera)
	class UCameraComponent* FollowCamera;
	//添加UMG组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	//输入映射相关属性
	UPROPERTY(EditAnywhere, Category=Input)
	class UInputMappingContext* MappingContext;
	UPROPERTY(EditAnywhere, Category=input)
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, Category=input)
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere, Category=input)
	UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, Category=input)
	UInputAction* EquipAction;
	UPROPERTY(EditAnywhere, Category=input)
	UInputAction* CrouchAction;
	UPROPERTY(EditAnywhere, Category=input)
	UInputAction* AimAction;
	UPROPERTY(EditAnywhere, Category=input)
	UInputAction* FireAction;
	
	//战斗组件
	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* Combat;

	//重叠的武器,添加网络同步,需要被复制
	//UPROPERTY(Replicated)//添加网络同步,需要被复制
	UPROPERTY(ReplicatedUsing=OnRep_OverlappingWeapon)//属性更新时会调用绑定的函数
	class AWeapon* OverlappingWeapon;

	//通知绑定的函数
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	//服务端RPC调用装备武器方法
	UFUNCTION(Server, Reliable)//启用RPC，Reliable可靠的，可靠的消息发送失败时，客户端会再次发送
	void ServerEquipButtonPressed();
	
	float AO_Yaw;//瞄准偏移的Yaw值
	float InterpAO_Yaw;
	UPROPERTY(replicated)
	float BaseAimYaw = 0.f;//用于同步客户端的Yaw值
	float AO_Pitch;//瞄准偏移的Pitch值
	FRotator StartingAimRotation;//移动时的偏移Yaw值
	
	ETurningInPlace TurningInPlace;//获取旋转状态
	void TurnInPlace(float DeltaTime);//计算并设置旋转状态方法

	UPROPERTY(EditAnywhere, Category=combat)
	class UAnimMontage* FireWeaponMontage;//开火蒙太奇
	
public:	
	void SetOverlappingWeapon(AWeapon* Weapon);
	//获取是否装备武器，返回值bool
	bool IsWeaponEquipped();

	//判断是否瞄准方法
	bool IsAiming();

	//获取AO_Yaw的getter方法
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	//AO_Pitch的getter
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	
	//EquippedWeapon的getter
	FORCEINLINE AWeapon* GetEquippedWeapon() const { return Combat ? Combat->EquippedWeapon : nullptr; }
	//获取旋转状态的getter
	FORCEINLINE ETurningInPlace GetETurningInPlace() const { return TurningInPlace; }
	
};
