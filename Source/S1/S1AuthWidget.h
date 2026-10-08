#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "S1AuthWidget.generated.h"

class SButton;
class SEditableTextBox;
class STextBlock;
class SWidgetSwitcher;

UCLASS()
class S1_API US1AuthWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	FReply HandleLoginClicked();
	FReply HandleOpenSignUpClicked();
	FReply HandleCreateAccountClicked();
	FReply HandleBackToLoginClicked();

	UFUNCTION()
	void HandleLoginCompleted(bool bSuccess, const FString& Message);

	UFUNCTION()
	void HandleSignUpCompleted(
		bool bSuccess,
		int32 ResultCode,
		const FString& Message);

	bool ValidateAccountName(
		const FString& AccountName,
		FString& ErrorMessage) const;
	bool ValidatePassword(
		const FString& Password,
		FString& ErrorMessage) const;
	void SetRequestInFlight(bool bInFlight);
	void SetStatus(const FString& Message, const FLinearColor& Color);
	FString GetSignUpErrorMessage(
		int32 ResultCode,
		const FString& FallbackMessage) const;

private:
	TSharedPtr<SWidgetSwitcher> PanelSwitcher;
	TSharedPtr<STextBlock> StatusText;
	TSharedPtr<SEditableTextBox> LoginAccountInput;
	TSharedPtr<SEditableTextBox> LoginPasswordInput;
	TSharedPtr<SButton> LoginButton;
	TSharedPtr<SButton> OpenSignUpButton;
	TSharedPtr<SEditableTextBox> SignUpAccountInput;
	TSharedPtr<SEditableTextBox> SignUpPasswordInput;
	TSharedPtr<SEditableTextBox> ConfirmPasswordInput;
	TSharedPtr<SButton> CreateAccountButton;
	TSharedPtr<SButton> BackToLoginButton;
	bool bRequestInFlight = false;
};
