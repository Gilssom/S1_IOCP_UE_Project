#include "S1AuthWidget.h"

#include "S1GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr int32 LOGIN_PANEL_INDEX = 0;
	constexpr int32 SIGN_UP_PANEL_INDEX = 1;
}

TSharedPtr<SWidget> US1AuthWidget::GetInitialFocusTarget() const
{
	if (!PanelSwitcher.IsValid()) return nullptr;
	return PanelSwitcher->GetActiveWidgetIndex() == SIGN_UP_PANEL_INDEX
		? StaticCastSharedPtr<SWidget>(SignUpAccountInput)
		: StaticCastSharedPtr<SWidget>(LoginAccountInput);
}

void US1AuthWidget::RestoreInputFocus()
{
	// Async replies must not focus a hidden/removed screen or another PIE user.
	if (!IsInViewport() || !FSlateApplication::IsInitialized()) return;
	if (auto* Player = GetOwningLocalPlayer())
		if (auto User = Player->GetSlateUser())
			FSlateApplication::Get().SetUserFocus(User->GetUserIndex(), GetInitialFocusTarget());
}

TSharedRef<SWidget> US1AuthWidget::RebuildWidget()
{
	TSharedRef<SVerticalBox> LoginPanel =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
		[
			SAssignNew(LoginAccountInput, SEditableTextBox)
			.HintText(FText::FromString(TEXT("아이디")))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 16.f)
		[
			SAssignNew(LoginPasswordInput, SEditableTextBox)
			.HintText(FText::FromString(TEXT("비밀번호")))
			.IsPassword(true)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
		[
			SAssignNew(LoginButton, SButton)
			.HAlign(HAlign_Center)
			.OnClicked(FOnClicked::CreateUObject(this, &US1AuthWidget::HandleLoginClicked))
			[SNew(STextBlock).Text(FText::FromString(TEXT("로그인")))]
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SAssignNew(OpenSignUpButton, SButton)
			.HAlign(HAlign_Center)
			.OnClicked(FOnClicked::CreateUObject(this, &US1AuthWidget::HandleOpenSignUpClicked))
			[SNew(STextBlock).Text(FText::FromString(TEXT("회원가입")))]
		];

	TSharedRef<SVerticalBox> SignUpPanel =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
		[
			SAssignNew(SignUpAccountInput, SEditableTextBox)
			.HintText(FText::FromString(TEXT("아이디 (영문, 숫자, 밑줄 3~20자)")))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
		[
			SAssignNew(SignUpPasswordInput, SEditableTextBox)
			.HintText(FText::FromString(TEXT("비밀번호 (8~64자)")))
			.IsPassword(true)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 16.f)
		[
			SAssignNew(ConfirmPasswordInput, SEditableTextBox)
			.HintText(FText::FromString(TEXT("비밀번호 확인")))
			.IsPassword(true)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
		[
			SAssignNew(CreateAccountButton, SButton)
			.HAlign(HAlign_Center)
			.OnClicked(FOnClicked::CreateUObject(this, &US1AuthWidget::HandleCreateAccountClicked))
			[SNew(STextBlock).Text(FText::FromString(TEXT("계정 생성")))]
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SAssignNew(BackToLoginButton, SButton)
			.HAlign(HAlign_Center)
			.OnClicked(FOnClicked::CreateUObject(this, &US1AuthWidget::HandleBackToLoginClicked))
			[SNew(STextBlock).Text(FText::FromString(TEXT("로그인으로 돌아가기")))]
		];

	return SNew(SBorder)
		.Padding(32.f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.BorderBackgroundColor(FLinearColor(0.025f, 0.035f, 0.055f, 0.96f))
		[
			SNew(SBox).WidthOverride(420.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 22.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("S1 ACCOUNT")))
					.Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 28))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 14.f)
				[
					SAssignNew(StatusText, STextBlock)
					.Text(FText::GetEmpty())
					.AutoWrapText(true)
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SAssignNew(PanelSwitcher, SWidgetSwitcher)
					.WidgetIndex(LOGIN_PANEL_INDEX)
					+ SWidgetSwitcher::Slot()[LoginPanel]
					+ SWidgetSwitcher::Slot()[SignUpPanel]
				]
			]
		];
}

void US1AuthWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (US1GameInstance* GameInstance = GetGameInstance<US1GameInstance>())
	{
		GameInstance->OnLoginCompleted.AddUniqueDynamic(this, &US1AuthWidget::HandleLoginCompleted);
		GameInstance->OnSignUpCompleted.AddUniqueDynamic(this, &US1AuthWidget::HandleSignUpCompleted);
	}
}

void US1AuthWidget::NativeDestruct()
{
	if (US1GameInstance* GameInstance = GetGameInstance<US1GameInstance>())
	{
		GameInstance->OnLoginCompleted.RemoveDynamic(this, &US1AuthWidget::HandleLoginCompleted);
		GameInstance->OnSignUpCompleted.RemoveDynamic(this, &US1AuthWidget::HandleSignUpCompleted);
	}
	Super::NativeDestruct();
}

FReply US1AuthWidget::HandleLoginClicked()
{
	if (bRequestInFlight)
		return FReply::Handled();

	const FString AccountName = LoginAccountInput->GetText().ToString();
	const FString Password = LoginPasswordInput->GetText().ToString();
	FString ErrorMessage;
	if (!ValidateAccountName(AccountName, ErrorMessage) || !ValidatePassword(Password, ErrorMessage))
	{
		SetStatus(ErrorMessage, FLinearColor(1.f, 0.25f, 0.25f));
		return FReply::Handled();
	}

	US1GameInstance* GameInstance = GetGameInstance<US1GameInstance>();
	if (GameInstance == nullptr)
		return FReply::Handled();
	if (!GameInstance->IsConnectedToGameServer())
		GameInstance->ConnectToGameServer();
	if (!GameInstance->IsConnectedToGameServer())
	{
		SetStatus(TEXT("게임 서버에 연결할 수 없습니다."), FLinearColor(1.f, 0.25f, 0.25f));
		return FReply::Handled();
	}

	SetRequestInFlight(true);
	SetStatus(TEXT("로그인 중..."), FLinearColor(0.65f, 0.8f, 1.f));
	GameInstance->LoginFromAuthUI(AccountName, Password);
	return FReply::Handled();
}

FReply US1AuthWidget::HandleOpenSignUpClicked()
{
	if (!bRequestInFlight && PanelSwitcher.IsValid())
	{
		PanelSwitcher->SetActiveWidgetIndex(SIGN_UP_PANEL_INDEX);
		SetStatus(TEXT("새 계정을 생성합니다."), FLinearColor::White);
		return FReply::Handled().SetUserFocus(SignUpAccountInput.ToSharedRef(), EFocusCause::SetDirectly);
	}
	return FReply::Handled();
}

FReply US1AuthWidget::HandleCreateAccountClicked()
{
	if (bRequestInFlight)
		return FReply::Handled();

	const FString AccountName = SignUpAccountInput->GetText().ToString();
	const FString Password = SignUpPasswordInput->GetText().ToString();
	const FString ConfirmPassword = ConfirmPasswordInput->GetText().ToString();
	FString ErrorMessage;
	if (!ValidateAccountName(AccountName, ErrorMessage) || !ValidatePassword(Password, ErrorMessage))
	{
		SetStatus(ErrorMessage, FLinearColor(1.f, 0.25f, 0.25f));
		return FReply::Handled();
	}
	if (Password != ConfirmPassword)
	{
		SetStatus(TEXT("비밀번호 확인이 일치하지 않습니다."), FLinearColor(1.f, 0.25f, 0.25f));
		return FReply::Handled();
	}

	US1GameInstance* GameInstance = GetGameInstance<US1GameInstance>();
	if (GameInstance == nullptr)
		return FReply::Handled();
	if (!GameInstance->IsConnectedToGameServer())
		GameInstance->ConnectToGameServer();
	if (!GameInstance->IsConnectedToGameServer())
	{
		SetStatus(TEXT("게임 서버에 연결할 수 없습니다."), FLinearColor(1.f, 0.25f, 0.25f));
		return FReply::Handled();
	}

	SetRequestInFlight(true);
	SetStatus(TEXT("계정 생성 중..."), FLinearColor(0.65f, 0.8f, 1.f));
	GameInstance->SignUp(AccountName, Password);
	return FReply::Handled();
}

FReply US1AuthWidget::HandleBackToLoginClicked()
{
	if (!bRequestInFlight && PanelSwitcher.IsValid())
	{
		PanelSwitcher->SetActiveWidgetIndex(LOGIN_PANEL_INDEX);
		SetStatus(FString(), FLinearColor::White);
		return FReply::Handled().SetUserFocus(LoginAccountInput.ToSharedRef(), EFocusCause::SetDirectly);
	}
	return FReply::Handled();
}

void US1AuthWidget::HandleLoginCompleted(bool bSuccess, const FString& Message)
{
	SetRequestInFlight(false);
	SetStatus(
		bSuccess ? TEXT("로그인에 성공했습니다.") : Message,
		bSuccess ? FLinearColor(0.25f, 1.f, 0.45f) : FLinearColor(1.f, 0.25f, 0.25f));
	if (bSuccess && LoginPasswordInput.IsValid())
		LoginPasswordInput->SetText(FText::GetEmpty());
	if (!bSuccess) RestoreInputFocus();
}

void US1AuthWidget::HandleSignUpCompleted(bool bSuccess, int32 ResultCode, const FString& Message)
{
	SetRequestInFlight(false);
	if (!bSuccess)
	{
		SetStatus(GetSignUpErrorMessage(ResultCode, Message), FLinearColor(1.f, 0.25f, 0.25f));
		RestoreInputFocus();
		return;
	}

	LoginAccountInput->SetText(SignUpAccountInput->GetText());
	LoginPasswordInput->SetText(FText::GetEmpty());
	SignUpPasswordInput->SetText(FText::GetEmpty());
	ConfirmPasswordInput->SetText(FText::GetEmpty());
	PanelSwitcher->SetActiveWidgetIndex(LOGIN_PANEL_INDEX);
	SetStatus(TEXT("계정이 생성되었습니다. 로그인해 주세요."), FLinearColor(0.25f, 1.f, 0.45f));
	RestoreInputFocus();
}

bool US1AuthWidget::ValidateAccountName(const FString& AccountName, FString& ErrorMessage) const
{
	if (AccountName.Len() < 3 || AccountName.Len() > 20)
	{
		ErrorMessage = TEXT("아이디는 3~20자로 입력하세요.");
		return false;
	}
	for (TCHAR Character : AccountName)
	{
		const bool bValid =
			(Character >= TEXT('A') && Character <= TEXT('Z')) ||
			(Character >= TEXT('a') && Character <= TEXT('z')) ||
			(Character >= TEXT('0') && Character <= TEXT('9')) ||
			Character == TEXT('_');
		if (!bValid)
		{
			ErrorMessage = TEXT("아이디에는 영문, 숫자, 밑줄만 사용할 수 있습니다.");
			return false;
		}
	}
	if (AccountName.Equals(TEXT("admin"), ESearchCase::IgnoreCase))
	{
		ErrorMessage = TEXT("사용할 수 없는 아이디입니다.");
		return false;
	}
	return true;
}

bool US1AuthWidget::ValidatePassword(const FString& Password, FString& ErrorMessage) const
{
	if (Password.Len() < 8 || Password.Len() > 64)
	{
		ErrorMessage = TEXT("비밀번호는 8~64자로 입력하세요.");
		return false;
	}
	return true;
}

void US1AuthWidget::SetRequestInFlight(bool bInFlight)
{
	bRequestInFlight = bInFlight;
	if (LoginButton.IsValid()) LoginButton->SetEnabled(!bInFlight);
	if (OpenSignUpButton.IsValid()) OpenSignUpButton->SetEnabled(!bInFlight);
	if (CreateAccountButton.IsValid()) CreateAccountButton->SetEnabled(!bInFlight);
	if (BackToLoginButton.IsValid()) BackToLoginButton->SetEnabled(!bInFlight);
}

void US1AuthWidget::SetStatus(const FString& Message, const FLinearColor& Color)
{
	if (!StatusText.IsValid())
		return;
	StatusText->SetText(FText::FromString(Message));
	StatusText->SetColorAndOpacity(Color);
}

FString US1AuthWidget::GetSignUpErrorMessage(int32 ResultCode, const FString& FallbackMessage) const
{
	switch (static_cast<Protocol::SignUpResult>(ResultCode))
	{
	case Protocol::SIGN_UP_RESULT_INVALID_ACCOUNT_NAME:
		return TEXT("아이디 형식이 올바르지 않습니다.");
	case Protocol::SIGN_UP_RESULT_INVALID_PASSWORD:
		return TEXT("비밀번호 형식이 올바르지 않습니다.");
	case Protocol::SIGN_UP_RESULT_ACCOUNT_ALREADY_EXISTS:
		return TEXT("이미 사용 중인 아이디입니다.");
	case Protocol::SIGN_UP_RESULT_RESERVED_ACCOUNT_NAME:
		return TEXT("사용할 수 없는 아이디입니다.");
	case Protocol::SIGN_UP_RESULT_DATABASE_UNAVAILABLE:
		return TEXT("현재 계정을 생성할 수 없습니다. 잠시 후 다시 시도하세요.");
	default:
		return FallbackMessage.IsEmpty() ? TEXT("계정 생성에 실패했습니다.") : FallbackMessage;
	}
}
