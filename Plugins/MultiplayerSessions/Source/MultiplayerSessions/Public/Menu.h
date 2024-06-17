// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumberOfPublicConnections = 4, FString TypeOfMatch = "FreeForAll", FString LobbyPath = "/Game/ThirdPerson/Maps/Lobby");
	
protected:
	//初始化方法
	virtual bool Initialize() override;
	//UI从关卡内移除时执行的方法
	virtual void NativeDestruct() override;

	//
	//接收MultiplayerSessionsSubsystem的创建会话委托，创建其回调方法，方法参数需和委托参数一致
	//
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResult, bool bWasSuccessful);//来自MultiplayerSessionsSubsystem查询会话完成委托绑定函数
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);//来自MultiplayerSessionsSubsystem加入会话完成委托绑定函数
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);//来自MultiplayerSessionsSubsystem开始会话完成委托绑定函数
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);//来自MultiplayerSessionsSubsystem销毁会话完成委托绑定函数
	
private:
	UPROPERTY(meta=(BindWidget))
	UButton* HostButton;//绑定UMG里的按钮,绑定的按钮名必须和变量名一致，否则会崩溃
	UPROPERTY(meta=(BindWidget))
	UButton* JoinButton;

	UFUNCTION()
	void HostButtonClicked();//host按钮点击方法
	UFUNCTION()
	void JoinButtonClicked();//Join按钮点击方法

	//关闭菜单方法
	UFUNCTION()
	void MenuTearDown();
	
	//自己创建的子系统类，方便从该类中调用子系统方法
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;
	
	//创建会话：连接数
	int32 NumPublicConnections = 4;
	//创建会话：会话类型
	FString TypeMatch = "FreeForAll";
	//大厅路径，蓝图调用添加
	FString PathToLobby;
};
