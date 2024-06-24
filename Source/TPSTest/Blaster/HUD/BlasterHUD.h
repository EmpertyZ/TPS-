// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

//创建保存准心图标的结构体
USTRUCT(Blueprintable)
struct FHUDPackage
{
	GENERATED_BODY()

public:
	UTexture2D* CrosshairCenter;
	UTexture2D* CrosshairTop;
	UTexture2D* CrosshairLeft;
	UTexture2D* CrosshairRight;
	UTexture2D* CrosshairBottom;
	float CrosshairSpread;
};

/**
 * 
 */
UCLASS()
class TPSTEST_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	FHUDPackage HudPackage;

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;//准心最大延伸值
	
	void DrawCrosshair(UTexture2D* Texture, FVector2d ViewportCenter, FVector2d Spread);
public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HudPackage = Package; }
};
