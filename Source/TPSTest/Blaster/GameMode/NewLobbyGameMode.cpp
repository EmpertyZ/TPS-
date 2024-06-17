// Fill out your copyright notice in the Description page of Project Settings.


#include "NewLobbyGameMode.h"

#include "GameFramework/GameStateBase.h"

void ANewLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	//通过GameState来获取玩家数组，从而获取玩家数量
	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
	//通过玩家数量来判断是否要进入游戏
	if (NumberOfPlayers == 2)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			bUseSeamlessTravel = true;
			//开启服务器旅行
			World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));
		}
	}
}
 