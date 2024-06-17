// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

//构造函数
UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem()://绑定会委托回调
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
	StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete))

{
	//获取在线子系统
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		//通过在线子系统获取SessionInterface
		SessionInterface = Subsystem->GetSessionInterface();
	}

}

void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{

	//判断获取到的会话接口是否为空，为空则没有从子系统中获取到会话
	if (!SessionInterface.IsValid())
	{
		return;
	}
	//创建会话步骤：
	//1.判断当前是否有已存在的会话，有的话则销毁该会话
	//2.创建会话句柄，用于后续清除句柄,以CreateSession为例CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
	//3.创建会话设置，设置会话设置
	//4.获取玩家UID
	//5.创建会话
	//6.判断会话是否创建成功，如果创建失败清理创建会话句柄
	auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
		//进入该方法时说明会话已经存在了一个，需要销毁当前会话，同时保存当前会话参数设置,以便退出重新创房
		bCreateSessionOnDestroy = true;
		LastNumPublicConnections = NumPublicConnections;
		LastMatchType = MatchType;
		//调用销毁会话方法
		DestroySession();
	}
	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
	LastSessionSettings = MakeShareable(new FOnlineSessionSettings);
	//会话设置，是否开启前往大厅功能，一定要开启不然无法跳转到房主等待房间
	LastSessionSettings->bUseLobbiesIfAvailable = true;
	//会话设置：判断是否开启局域网连接，如果获取到的当前子系统为Steam或者其他平台则不开启，没有获取到子系统则开启
	LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	//会话设置：公开连接数
	LastSessionSettings->NumPublicConnections = NumPublicConnections;
	//会话设置：允许中途加入
	LastSessionSettings->bAllowJoinInProgress = true;
	//会话设置：允许在线加入
	LastSessionSettings->bAllowJoinViaPresence = true;
	//会话设置：允许公开
	LastSessionSettings->bShouldAdvertise = true;
	//会话设置：允许被好友看到当前状态
	LastSessionSettings->bUsesPresence = true;
	//会话设置：设置当前游戏类型
	LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	//会话设置，设置会话绑定唯一，可以让其他玩家搜索房间时看到彼此是谁
	LastSessionSettings->BuildUniqueId = 1;
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	//创建会话会返回一个bool值，通过返回的bool值来判断是否创建会话完成，如果不完成则清除创建会话句柄
	if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings))
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		//创建会话失败，广播失败消息到Menu类
		MultiplayerOnCreateSessionComplete.Broadcast(false);
	}


	
} 

void UMultiplayerSessionsSubsystem::FindSession(int32 MaxSearchResults)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}
	//搜索会话步骤：
	//1.添加搜索会话句柄
	//2.设置搜索会话设置
	//3.获取玩家uid
	//4.搜寻会话,参数（*玩家UID,搜寻设置.ToSharedRef()）
	FindSessionCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	//判断是否有搜索到会话
	if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))
	{
		//未搜索到会话则清除委托句柄
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionCompleteDelegateHandle);
		//未搜索到会话，广播失败消息到Menu类,参数（空数组，bool）
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
	
	
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SearchResult)
{
	if (!SessionInterface)
	{
		//会话接口不存在，直接广播失败消息到Menu,中止执行
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::Type::UnknownError);
		return;
	}
	//加入绘画步骤：
	//1.添加加入会话句柄
	//2.获取uid
	//3.加入会话，参数（*玩家uid, SessionName, SearchResult）
	SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	//加入会话会有返回值如果返回失败，直接清除句柄广播失败消息
	if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SearchResult))
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::Type::UnknownError);
	}
	
}

void UMultiplayerSessionsSubsystem::StartSession()
{
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
	if (!SessionInterface)
	{
		//会话接口不存在，直接广播失败消息到Menu
		MultiplayerOnDestroySessionComplete.Broadcast(false);
		return;
	}
	//销毁会话步骤： 
	//1.添加销毁句柄
	//2.执行销毁会话方法
	//3.判断销毁会话是否完成，未完成直接广播失败，清理句柄
	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);
	if (SessionInterface->DestroySession(NAME_GameSession))
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		MultiplayerOnDestroySessionComplete.Broadcast(false);
	}
	
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface)
	{
		//能到这里说明创建会话完成，创建会话完成则通过创建会话委托句柄清除掉创建会话委托
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		
	}

	//广播创建会话成功消息到Menu类
	MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnFindSessionComplete(bool bWasSuccessful)
{
	if (SessionInterface)
	{
		//到这里说明搜索会话完成，搜索会话完成后清除搜索会话完成句柄
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionCompleteDelegateHandle);
	}
	//先判断搜索结果，如果搜索结果为0，直接广播空值，不为0广播正常参数
	if (LastSessionSearch->SearchResults.Num() <= 0)
	{
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Blue,
				FString(TEXT("未搜索到会话"))
			);	
		}
		return;
	}
	MultiplayerOnFindSessionsComplete.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface)
	{
		//到这里说明加入会话完成，加入会话完成后清除该会话句柄
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}
	//加入会话成功，广播消息到Menu
	MultiplayerOnJoinSessionComplete.Broadcast(Result);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface)
	{
		//进入到这里说明销毁会话完成，清理销毁会话委托句柄
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}
	//判断是否销毁成功，且是否有执行创建会话方法，二者皆有的话就执行创建会话方法
	if (bWasSuccessful && bCreateSessionOnDestroy)
	{
		bCreateSessionOnDestroy = false;
		CreateSession(LastNumPublicConnections, LastMatchType);
	}
	//广播销毁会话完成情况
	MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}
