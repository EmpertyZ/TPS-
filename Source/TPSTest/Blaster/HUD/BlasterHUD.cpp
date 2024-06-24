// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();
	FVector2d ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2d ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		//计算准心延伸值
		float SpreadScaled = CrosshairSpreadMax * HudPackage.CrosshairSpread;
		
		//绘制准心的上下左右材质
		if (HudPackage.CrosshairTop)
		{
			//向上延伸值
			const FVector2d Spread(0.f, -SpreadScaled);
			DrawCrosshair(HudPackage.CrosshairTop, ViewportCenter, Spread);
		}
		if (HudPackage.CrosshairCenter)
		{
			const FVector2d Spread(0.f, 0.f);//准心中间的点不需要移动
			DrawCrosshair(HudPackage.CrosshairCenter, ViewportCenter, Spread);
		}
		if (HudPackage.CrosshairLeft)
		{
			//向左延伸值
			const FVector2d Spread(-SpreadScaled, 0.f);
			DrawCrosshair(HudPackage.CrosshairLeft, ViewportCenter, Spread);
		}
		if (HudPackage.CrosshairRight)
		{
			//向右延伸值
			const FVector2d Spread(SpreadScaled, 0.f);
			DrawCrosshair(HudPackage.CrosshairRight, ViewportCenter, Spread);
		}
		if (HudPackage.CrosshairBottom)
		{
			//向下延伸值
			const FVector2d Spread(0.f, SpreadScaled);
			DrawCrosshair(HudPackage.CrosshairBottom, ViewportCenter, Spread);
		}
		
	}

}

void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2d ViewportCenter, FVector2d Spread)
{
	//获取传入材质的xy大小
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	//创建绘制的点
	const FVector2d TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y
	);
	//绘制材质
	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		FLinearColor::White
	);
}


