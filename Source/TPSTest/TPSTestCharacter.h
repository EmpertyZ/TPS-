// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "TPSTestCharacter.generated.h"


UCLASS(config=Game)
class ATPSTestCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

public:
	ATPSTestCharacter();

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// To add mapping context
	virtual void BeginPlay();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

public:
	//声明存储OnlineSessionInterface的共享指针指针
	IOnlineSessionPtr OnlineSessionInterface;
	//typedef TSharedPtr<IOnlineSession, ESPMode::ThreadSafe> IOnlineSessionPtr;

protected:
	//声明创建Session方法，蓝图可调用
	UFUNCTION(BlueprintCallable)
	void CreateGameSession();
	//声明搜寻Session方法，蓝图可调用
	UFUNCTION(BlueprintCallable)
	void JoinGameSession();

	//声明创建完成委托绑定的方法
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	//声明查询Session委托绑定方法
	void OnFindSessionComplete(bool bWasSuccessful);
	//声明加入Session委托绑定方法
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type);

private:
	//声明创建会话完成委托
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;//typedef FOnCreateSessionComplete::FDelegate FOnCreateSessionCompleteDelegate;DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCreateSessionComplete, FName, bool);
	//声明查找会话完成委托
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;//DECLARE_MULTICAST_DELEGATE_OneParam(FOnFindSessionsComplete, bool);
	//声明储存查询到的会话结果
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	//声明加入会话委托
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;//DECLARE_MULTICAST_DELEGATE_TwoParams(FOnJoinSessionComplete, FName, EOnJoinSessionCompleteResult::Type);
}

;

