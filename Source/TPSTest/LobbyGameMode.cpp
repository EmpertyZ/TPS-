// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	//获取GameState
	if (GameState)
	{
		//通过GameState获取玩家数组，进而获取玩家数量
		int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
		//打印玩家数量
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				1,
				60.f,
				FColor::Green,
				FString::Printf(TEXT("当前玩家数量为：%d"), NumberOfPlayers)
			);
		}

		//获取加入的玩家名称
		APlayerState* PlayerState = NewPlayer->GetPlayerState<APlayerState>();
		if (PlayerState)
		{
			
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					45.f,
					FColor::Purple,
					FString::Printf(TEXT("进入玩家名字为：%s"), *PlayerState->GetPlayerName())
				);
			}
		}
	}
	
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
	//打印玩家数量
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			1,
			60.f,
			FColor::Green,
			FString::Printf(TEXT("当前玩家数量为：%d"), NumberOfPlayers-1)
		);
	}

	//获取加入的玩家名称
	APlayerState* PlayerState = Exiting->GetPlayerState<APlayerState>();
	if (PlayerState)
	{
			
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				45.f,
				FColor::Purple,
				FString::Printf(TEXT("退出玩家名字为：%s"), *PlayerState->GetPlayerName())
			);
		}
	}
	
}
