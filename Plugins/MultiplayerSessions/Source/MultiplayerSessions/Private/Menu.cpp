// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"



void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
	NumPublicConnections = NumberOfPublicConnections;
	TypeMatch = TypeOfMatch;
	
	//启动时显示到屏幕上
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);//表示菜单可接受焦点
	
	UWorld* World = GetWorld();
	if (World)
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())//获取当前的玩家控制器
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());//焦点小部件为当前菜单
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);//设置鼠标不被游戏窗口限制
			PlayerController->SetInputMode(InputModeData);//设置玩家控制的输入模式
			PlayerController->SetShowMouseCursor(true);//设置鼠标显示在屏幕上
		}
	}

	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}
	
	//判断是否获取到MultiplayerSession子系统，获取到的话绑定该子系统的委托
	if (MultiplayerSessionsSubsystem)
	{
		//绑定广播创建完成会话委托回调
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		//绑定广播查询完成会话委托回调
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		//绑定广播加入完成会话委托回调
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		//绑定广播开始完成会话委托回调
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
		//绑定广播销毁完成会话委托回调
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
	}
}

//初始化方法
bool UMenu::Initialize()
{
	//不存在初始化方法的话，直接不执行
	if(!Super::Initialize())
	{
		return false;
	}
	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);//AddDynamic，动态委托调用绑定方法，无法绑定静态方法
	}
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}
	return true;
}

//UI从关卡内移除时执行的方法
void UMenu::NativeDestruct()
{
	MenuTearDown();
	Super::NativeDestruct();
}

//
//接收MultiplayerSessionsSubsystem的创建会话委托，创建其回调方法，方法参数需和委托参数一致
//
void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Yellow,
				FString(TEXT("创建会话成功"))
			);	
		}

		//创建会话成功后传送到大厅
		if (UWorld* World = GetWorld())
		{
			World->ServerTravel(FString(PathToLobby));
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Yellow,
				FString(TEXT("创建失败"))
			);	
		}
		HostButton->SetIsEnabled(true);
	}
	
}

//来自MultiplayerSessionsSubsystem查询会话完成委托绑定函数
void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResult, bool bWasSuccessful)
{
	//先判断子系统是否存在，不存在直接不执行后续操作，因为子系统不存在无法调用接下来的加入会话功能
	if (!MultiplayerSessionsSubsystem)
	{
		return;
	}
	for(auto Result : SessionResult)
	{
		FString SettingsValue;//临时变量，用于存储获得的会话类型名称
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);//获取传过来的结果中的会话，从该会话里面获取会话设置，从中获取会话类型并赋值给SettingValue
		if (SettingsValue == TypeMatch)//判断与当前存入的会话类型是否一致
		{
			//执行加入会话
			MultiplayerSessionsSubsystem->JoinSession(Result);
			return;
		}
	}

	if (SessionResult.Num() == 0 && bWasSuccessful)
	{
		//如果搜索不到会话或者搜索失败，重新启用JoinButton
		JoinButton->SetIsEnabled(true);
	}
}

//来自MultiplayerSessionsSubsystem加入会话完成委托绑定函数
void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	//因为切到不同的会话，需要重新获取当前的会话
	if (const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		//通过当前会话获取会话接口
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					15.f,
					FColor::Blue,
					FString(TEXT("加入会话"))
				);	
			}
			//通过会话接口获取地址
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);
			//执行客户端连接到该地址
			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					15.f,
					FColor::Blue,
					FString::Printf(TEXT("进入的地址名：%d"), NAME_GameSession)
				);	
			}
		}
	}

	//如果加入会话返回的结果不是成功，需启用JoinButton
	if (Result == EOnJoinSessionCompleteResult::Type::Success)
	{
		JoinButton->SetIsEnabled(true);
	}
	
}

//来自MultiplayerSessionsSubsystem开始会话完成委托绑定函数
void UMenu::OnStartSession(bool bWasSuccessful)
{
}

//来自MultiplayerSessionsSubsystem销毁会话完成委托绑定函数
void UMenu::OnDestroySession(bool bWasSuccessful)
{
}

//host按钮点击方法
void UMenu::HostButtonClicked()
{
	//点击后设置按钮为不可点击，防止出现连续点击的情况
	HostButton->SetIsEnabled(false);
	//创建会话
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, TypeMatch);
	}
	
	
}
//Join按钮点击方法
void UMenu::JoinButtonClicked()
{
	//点击后设置按钮为不可点击，防止出现连续点击的情况
	JoinButton->SetIsEnabled(true);
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->FindSession(1000);
	}
}

//关闭菜单方法
void UMenu::MenuTearDown()
{
	//移除UI
	RemoveFromParent();
	if (const UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			//将输入模式从UI改为游戏内输入，这样进入游戏后才能控制角色
			const FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}
 