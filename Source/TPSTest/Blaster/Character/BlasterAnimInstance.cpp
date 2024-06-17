// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	//获取蓝图拥有者
	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	//判断是否有获取到蓝图拥有者
	if (BlasterCharacter == nullptr)
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}
	if (BlasterCharacter == nullptr) return;
	//获取角色速度
	FVector Velocity = BlasterCharacter->GetVelocity();
	//将速度转换为Float的速度
	Velocity.Z = 0.f;
	Speed = Velocity.Size();
	//获取角色的移动组件，通过组件获取角色是否在空中
	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();
	//获取角色的移动组件，通过组件获取角色是否在加速 
	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	//获取角色装备武器状态
	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();
	//获取角色当前状态是否蹲下
	bIsCrouched = BlasterCharacter->bIsCrouched;
	//获取角色当前状态是否在瞄准
	bAiming = BlasterCharacter->IsAiming();
	//获取角色所装备的武器
	EquippedWeapon = BlasterCharacter->GetEquippedWeapon();
	//从角色获取角色转身状态
	TurningInPlace = BlasterCharacter->GetETurningInPlace();

	//获取动画偏移转向值
	FRotator AimRotation = BlasterCharacter->GetBaseAimRotation();
	//获取移动的偏移转向值
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());
	//求出平地X轴偏移值
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	//计算平滑过度后的旋转值
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaSeconds, 5.f);
	//设置平滑过度后的偏移量
	YawOffset = DeltaRotation.Yaw;

	//lean值，倾斜度计算
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = BlasterCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaSeconds;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	//通过角色获取AO_Yaw
	AO_Yaw = BlasterCharacter->GetAO_Yaw();
	AO_Pitch = BlasterCharacter->GetAO_Pitch();

	//判断角色是否有装备武器,看是否能获取到武器的mesh,是否能获取到角色的mesh
	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh())
	{
		//获取武器Mesh上的插槽的相对变换,设置相对场景/世界变换
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		//获取武器mesh到角色武器socket的相对变换, TransformToBoneSpace( FName BoneName, FVector InPosition, FRotator InRotation, FVector& OutPosition, FRotator& OutRotation )
		BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);//参数中带&的为输出值
		//设置所求出来的相对位置和旋转
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));
	}
}

