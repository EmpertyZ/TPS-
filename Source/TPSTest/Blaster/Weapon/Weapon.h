// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Casing.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMax"),
};


UCLASS()
class TPSTEST_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();
	virtual void Tick(float DeltaTime) override;

	//需要复制的属性方法
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	//胶囊开始重叠函数
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
	//胶囊结束重叠函数
	UFUNCTION()
	virtual void EndSphereOverlap(
			UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex
	);


public:	
	void ShowPickupWidget(bool bWasShowPickup);

	//设置武器状态
	void SetWeaponState(EWeaponState NewWeaponState);
	//AreaSphere的getter方法
	FORCEINLINE USphereComponent* GetAreaSphere() const {return AreaSphere;}
	//武器mesh的getter
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	virtual void Fire(const FVector& HitTarget);

		
	//武器准心,资产
	UPROPERTY(EditAnywhere, Category="Weapon Crosshair")
	UTexture2D* CrosshairCenter;//准心中间
	UPROPERTY(EditAnywhere, Category="Weapon Crosshair")
	UTexture2D* CrosshairTop;//准心顶部
	UPROPERTY(EditAnywhere, Category="Weapon Crosshair")
	UTexture2D* CrosshairLeft;//准心左侧
	UPROPERTY(EditAnywhere, Category="Weapon Crosshair")
	UTexture2D* CrosshairRight;//准心右侧
	UPROPERTY(EditAnywhere, Category="Weapon Crosshair")
	UTexture2D* CrosshairBottom;//准心底部
private:
	//创建武器mesh
	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;
	//碰撞胶囊体
	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	USphereComponent* AreaSphere;
	//添加网络同步，需要被复制，武器状态
	UPROPERTY(ReplicatedUsing=OnRep_WeaponState, VisibleAnywhere, Category="Weapon Properties")
	EWeaponState WeaponState;
	UPROPERTY(VisibleAnywhere, Category="Weapon Properties")
	UWidgetComponent* PickupWidget;
	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	TSubclassOf<ACasing> CasingClass;
	
	UFUNCTION()
	void OnRep_WeaponState();//绑定武器状态复制的方法,当武器状态更新时调用该方法

};
