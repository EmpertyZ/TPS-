// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"

#include "Components/TextBlock.h"

void UOverheadWidget::SetDisPlayText(FString TextToDisplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	//获取当前的NetRole
	ENetRole LocalRole = InPawn->GetRemoteRole();
	FString Role;
	switch (LocalRole)
	{
	case ENetRole::ROLE_Authority:
		Role = FString("Authority");
		break;
	case ENetRole::ROLE_SimulatedProxy:
		Role = FString("SimulatedProxy");
		break;
	case ENetRole::ROLE_AutonomousProxy:
		Role = FString("AutonomousProxy");
		break;
	case ENetRole::ROLE_None:
		Role = FString("None");
		break;
	}
	Role = FString::Printf(TEXT("自身网络规则为：%s"), *Role);
	SetDisPlayText(Role);
}

void UOverheadWidget::NativeDestruct()
{
	//从父项移除
	RemoveFromParent();
	Super::NativeDestruct();
}
