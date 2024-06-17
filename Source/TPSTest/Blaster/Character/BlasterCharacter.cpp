// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "TPSTest/Blaster/BlasterComponents/CombatComponent.h"
#include "TPSTest/Blaster/Weapon/Weapon.h"

// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	//绑定相机臂，设置相机臂参数
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 500.f;
	CameraBoom->bUsePawnControlRotation = true;

	//绑定相机,将相机绑定到相机臂
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	//设置镜头不跟随控制器旋转
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	//添加UMG组件
	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	//添加战斗组件
	Combat = CreateDefaultSubobject<UCombatComponent>("CombatComponent");
	Combat->SetIsReplicated(true);//设置组件可复制

	//启用MovementComponent的可蹲伏功能
    ACharacter::GetMovementComponent()->NavAgentProps.bCanCrouch = true;
	//设置胶囊忽略相机
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	//设置mesh忽略相机
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	//设置网络更新频率
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

//获取需要复制的属性
void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, BaseAimYaw);
}

//当类和组件注册完成时就会调用该函数，用于组件初始化，如果玩家有调用该组件，就绑定战斗组件的友元类
void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents(); 
	//判断战斗组件是否存在
	if (Combat)
	{
		//设置Combat组件的友元类
		Combat->Character = this;
	}
}

//播放开火蒙太奇
void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	//判断是否有添加战斗组件，是否有装备上武器
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	//获取动画实例，如果动画实例还有开火蒙太奇存在，播放开火蒙太奇
	if (AnimInstance && FireWeaponMontage)
	{
		//播放开火蒙太奇
		AnimInstance->Montage_Play(FireWeaponMontage);
		//获取当前是瞄准状态还是非瞄准状态。瞄准状态：播放瞄准蒙太奇；非瞄准状态：播放非瞄准蒙太奇
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
		
	}

}

// Called when the game starts or when spawned
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay(); 

	//添加输入绑定映射上下文步骤：
	//1.获取PlayerController
	//2.从playerController中获取增强输入子系统
	//3.通过增强输入子系统添加映射上下文
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		//AddMappingContext参数：上下文，优先级
		Subsystem->AddMappingContext(MappingContext, 0);
	}
	
}

//移动方法
void ABlasterCharacter::Move(const FInputActionValue& Value)
{
	//获取输入值得2d向量
	FVector2D MovementVector = Value.Get<FVector2D>();
	//声明旋转值和参考轴
	//旋转值
	const FRotator Rotation = Controller->GetControlRotation();
	//参考轴
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	//通过计算参考轴与XY轴中的X来求出向前向量
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	//通过计算参考轴与XY轴中的Y来求出向右向量
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	//调用移动组件的添加输入
	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

//左右看方法
void ABlasterCharacter::Look(const FInputActionValue& Value)
{
	//获取输入的2D向量
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		//判断是控制是否存在，存在就执行设置控制器旋转
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

//按下装备武器按钮，装备武器方法
void ABlasterCharacter::EquipButtonPressed()
{

	/**
	 *装备武器
	 *  判断是否拥有战斗组件且是否为服务器，为服务器的话则执行装备武器
	 *  不是服务器，则执行RPC调用
	 **/
	if (Combat)
	{
		if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
		}
	}
}

//蹲伏方法
void ABlasterCharacter::CrouchButtonPressed()
{
	//判断当前状态是否为蹲伏，如果蹲伏按蹲伏键的时候就为不蹲伏
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

//开始瞄准方法
void ABlasterCharacter::AimButtonPressed()
{
	//检查是否有启用战斗组件，如果有则设置瞄准状态
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}
//结束瞄准
void ABlasterCharacter::AimButtonReleased()
{
	//检查是否有启用战斗组件，如果有则设置瞄准状态
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}
//开枪方法
void ABlasterCharacter::FireButtonPressed()
{
	//检查是否启用战斗组件，如果有则设置开火
	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}
//停火方法
void ABlasterCharacter::FireButtonReleased()
{
	//检查是否启用战斗组件，如果有则设置停火
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

//更新服务端的动画yaw，让其能够同步其他客户端
void ABlasterCharacter::UpdateBaseAimYawForHasAuthority()
{
	if (HasAuthority())
	{
		BaseAimYaw = GetBaseAimRotation().Yaw;
	}
}

//计算瞄准偏移方法
void ABlasterCharacter::AimOffset(float DeltaTime)
{
	//判断角色是否有装备武器
	if (Combat && Combat->EquippedWeapon == nullptr) return;
	//获取角色移动速度
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	//获取角色是否在空中
	bool bIsInAir = GetCharacterMovement()->IsFalling();
	//判断角色是否有移动或者在空中，如果不在这进行计算瞄准偏移
	if (Speed == 0.f && !bIsInAir)
	{
		UpdateBaseAimYawForHasAuthority();
		//获取停止移动时的旋转偏移Yaw值
		FRotator CurrentAimRotation = FRotator(0.f, BaseAimYaw, 0.f);
		//获取移动时的偏移Yaw值和当前位置的偏移Yaw值的差值
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		//获取所计算的差值中的Yaw即为动画蓝图所用的瞄准偏移Yaw值
		AO_Yaw = DeltaAimRotation.Yaw;

		//不处于转身状态
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;//不执行转身，不开启执行旋转根骨骼
			
			
		}
		//处于站着瞄准状态，禁止鼠标旋转控制角色转身
		bUseControllerRotationYaw = false;
		TurnInPlace(DeltaTime);
		
	}
	
	//角色移动或这跳跃时
	if (Speed > 0.f || bIsInAir)
	{
		UpdateBaseAimYawForHasAuthority();
		//移动时把记录移动的偏移Yaw值
		StartingAimRotation = FRotator(0.f, BaseAimYaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;//设置为无转身状态
	}

	AO_Pitch = GetBaseAimRotation().Pitch;
	//因为网络传输中会把负数转化为其他的角度，将 < 0度 的旋转值转换为[270, 360]
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		//判断pitch是否大于90，且不为本地运行，却道是在非本地客户端情况下转换数值
		FVector2d InRange(270.f, 360.f);
		FVector2d OutRange(-90.f, 0.f);
		//对于包含[Input:Range]的给定值，返回包含[Output:Range]的相应百分比。
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}

}



//服务端RPC调用装备武器方法
void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

//通知绑定的函数
void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	//函数最开始的AWeapon是从nullptr开始的，当属性变化时会从nullptr转变为武器，再改改变时则是从武器转变为nullptr
	//则可以推断出，只要LastWeapon存在时，玩家就是脱离胶囊，当LastWeapon为空时，则玩家重叠胶囊
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

//计算并设置旋转状态方法
void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	
	//转身边界值超过+-90后，设置转身状态
	if (AO_Yaw > 50.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -50.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	//处于转身状态
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		//设置旋转根骨骼
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 8.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 3.f)
		{
			//转至指定角度，设置转身状态
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			UpdateBaseAimYawForHasAuthority();
			//更新转身后的Yaw值
			StartingAimRotation = FRotator(0.f, BaseAimYaw, 0.f);
		}
	}
	
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	//因为服务器端的显示UMG是服务端本地调用，OverlappingWeapon最初肯定是从nullptr开始，只有碰到胶囊,并且执行下一句赋值代码才会有值
	//如果未执行赋值就有值说明已经结束重叠了
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	//判断是否是本地控制器还是服务器模拟的，如果是本地控制的则显示拾取组件
	 if (IsLocallyControlled())
	 {
		 if (OverlappingWeapon)
		 {
		 	OverlappingWeapon->ShowPickupWidget(true);
		 }
	 }
}

//获取是否装备武器，返回值bool
bool ABlasterCharacter::IsWeaponEquipped()
{
	//判断是否有装备武器和战斗组件，如果有则说明武器在身上
	return (Combat && Combat->EquippedWeapon);
}

//判断是否在瞄准状态的方法
bool ABlasterCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//调用计算偏移方法，实时更新偏移值
	AimOffset(DeltaTime);
}

// Called to bind functionality to input
void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//绑定增强输入相关方法
	//将默认输入组件转换为增强输入组件
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//bindAction，参数（InputAction，触发方式，目标对象，绑定的方法名）
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Started, this, &ThisClass::EquipButtonPressed);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ThisClass::CrouchButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &ThisClass::AimButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ThisClass::AimButtonReleased);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &ThisClass::FireButtonPressed);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ThisClass::FireButtonReleased);
		
	}
	
}


