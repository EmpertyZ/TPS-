// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MultiplayerSessionsSubsystem.generated.h"

//
//完成会话方法后执行的委托，以创建会话方法为例，完成创建会话后，调用该委托，执行该委托时，Menu类绑定的回调将会被执行
//
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& SessionResult, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool, bWasSuccessful);


/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//构造函数
	UMultiplayerSessionsSubsystem();

	//创建会话完成后委托，被其他类调用的委托需要是public属性
	FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete;
	//查询会话完成后委托，被其他类调用的
	FMultiplayerOnFindSessionsComplete MultiplayerOnFindSessionsComplete;
	//加入会话完成后委托
	FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionComplete;
	//开始会话完成后委托
	FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;
	//销毁会话完成委托
	FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;
	
	//
	//会话接口实现方法，用于菜单类调用
	//
	void CreateSession(int32 NumPublicConnections, FString MatchType); //参数：最大支持多少玩家游玩，房间类型
	void FindSession(int32 MaxSearchResults); //参数：最大搜索结果
	void JoinSession(const FOnlineSessionSearchResult& SearchResult); //参数：搜索到的会话
	void StartSession();
	void DestroySession();

protected:
	//
	//在线会话委托绑定的回调方法，这些方法不需要在别的类调用，所以放在protected里面
	//
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
private:
	//用于存储会话接口
	IOnlineSessionPtr SessionInterface;
	//创建会话设置
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	//
	//添加在线会话接口委托，用于绑定调用委托方法
	//
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionCompleteDelegateHandle;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteDelegateHandle;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;

	//销毁会话判断及销毁前的创建会话参数
	bool bCreateSessionOnDestroy{ false };
	int32 LastNumPublicConnections;
	FString LastMatchType;
}

;
