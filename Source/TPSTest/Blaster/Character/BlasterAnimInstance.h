// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TPSTest/Blaster/BlasterTypes/TruningInPlasce.h"
#include "TPSTest/Blaster/Weapon/Weapon.h"
#include "BlasterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class TPSTEST_API UBlasterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:

	//定义动画实例的基础属性
	UPROPERTY(BlueprintReadOnly, Category=INVALID_NAME_CHARACTERS, meta=(AllowPrivateAccess = "true"))//AllowPrivateAccess:允许蓝图访问私有变量
	class ABlasterCharacter* BlasterCharacter;
	UPROPERTY(BlueprintReadOnly, Category=INVALID_NAME_CHARACTERS, meta=(AllowPrivateAccess = "true"))//AllowPrivateAccess:允许蓝图访问私有变量
	float Speed;
	UPROPERTY(BlueprintReadOnly, Category=INVALID_NAME_CHARACTERS, meta=(AllowPrivateAccess = "true"))
	bool bIsInAir;
	UPROPERTY(BlueprintReadOnly, Category=INVALID_NAME_CHARACTERS, meta=(AllowPrivateAccess = "true"))
	bool bIsAccelerating;//是否加速中
	UPROPERTY(BlueprintReadOnly, Category=INVALID_NAME_CHARACTERS, meta=(AllowPrivateAccess = "true"))
	bool bWeaponEquipped;//是否装备武器
	UPROPERTY(BlueprintReadOnly, Category=INVALID_NAME_CHARACTERS, meta=(AllowPrivateAccess = "true"))
	bool bIsCrouched;//是否蹲下
	UPROPERTY(BlueprintReadOnly, Category=INVALID_NAME_CHARACTERS, meta=(AllowPrivateAccess = "true"))
	bool bAiming;//是否瞄准
	UPROPERTY(BlueprintReadOnly, Category=INVALID_NAME_CHARACTERS, meta=(AllowPrivateAccess = "true"))
	float YawOffset;//平行偏移值
	UPROPERTY(BlueprintReadOnly, Category=INVALID_NAME_CHARACTERS, meta=(AllowPrivateAccess = "true"))
	float Lean;//倾斜值
	UPROPERTY(BlueprintReadOnly, Category=INVALID_NAME_CHARACTERS, meta=(AllowPrivateAccess = "true"))
	float AO_Yaw;//瞄准偏移的Yaw值
	UPROPERTY(BlueprintReadOnly, Category=INVALID_NAME_CHARACTERS, meta=(AllowPrivateAccess = "true"))
	float AO_Pitch;//瞄准偏移的Pitch值
	
	FRotator CharacterRotationLastFrame;//上一帧的旋转值
	FRotator CharacterRotation;//当前旋转值
	FRotator DeltaRotation;//平滑过度旋转值

	//fabrik IK
	UPROPERTY(BlueprintReadOnly, Category=INVALID_NAME_CHARACTERS, meta=(AllowPrivateAccess = "true"))
	FTransform LeftHandTransform;
	AWeapon* EquippedWeapon;//通过character的getter方法获取

	UPROPERTY(BlueprintReadOnly, Category=INVALID_NAME_CHARACTERS, meta=(AllowPrivateAccess = "true"))
	ETurningInPlace TurningInPlace;//通过Character的getter方法获取
}; 
