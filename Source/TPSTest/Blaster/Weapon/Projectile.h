// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

//子弹类
UCLASS()
class TPSTEST_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	

	AProjectile();
	virtual void Tick(float DeltaTime) override;
	
protected:
	virtual void BeginPlay() override;

	//当actor被销毁时调用
	virtual void Destroyed() override;
	
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
private:
	UPROPERTY(EditAnywhere)
	UBoxComponent* CollisionBox;
	
	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* Tracer;//弹道效果
	class UParticleSystemComponent* TracerComponent;//弹道效果组件

	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;//子弹击中特效
	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;//子弹击中音效

};
