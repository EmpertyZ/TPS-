// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"

#include "Projectile.h"
#include "Engine/SkeletalMeshSocket.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	if (!HasAuthority()) return;
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	//生成子弹
	//获取武器socket，判断其是否存在
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket)
	{
		//生成子弹的变换
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		//通过获取的子弹变换，计算子弹的路径：子弹路径=击中位置-枪口位置
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		//通过获取的路径来获取子弹的旋转
		FRotator TargetRotation = ToTarget.Rotation();
		//武器socket存在的话，就生成子弹, 生成子弹前先判断子弹是否存在
		if (ProjectileClass && InstigatorPawn)
		{
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.Owner = GetOwner();//设置生成类的所有者
			SpawnParameters.Instigator = InstigatorPawn;//设置造成伤害的所有者
			//生成子弹
			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParameters);
			}
		}
	}
}
