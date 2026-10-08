#include "S1CharacterSelectWidget.h"

#include "S1GameInstance.h"
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
	constexpr int32 CHARACTER_LIST_PANEL = 0;
	constexpr int32 CHARACTER_CREATE_PANEL = 1;
}

TSharedRef<SWidget> US1CharacterSelectWidget::RebuildWidget()
{
	TSharedRef<SVerticalBox> ListPanel =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
		[
			SAssignNew(CharacterListBox, SVerticalBox)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
		[
			SAssignNew(SelectedCharacterText, STextBlock)
			.Text(FText::FromString(TEXT("선택된 캐릭터가 없습니다.")))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
		[
			SAssignNew(PlayButton, SButton)
			.HAlign(HAlign_Center)
			.OnClicked(FOnClicked::CreateUObject(
				this, &US1CharacterSelectWidget::HandlePlayClicked))
			[SNew(STextBlock).Text(FText::FromString(TEXT("플레이")))]
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SAssignNew(OpenCreateButton, SButton)
			.HAlign(HAlign_Center)
			.OnClicked(FOnClicked::CreateUObject(
				this, &US1CharacterSelectWidget::HandleOpenCreateClicked))
			[SNew(STextBlock).Text(FText::FromString(TEXT("새 캐릭터 만들기")))]
		];

	TSharedRef<SVerticalBox> CreatePanel =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
		[
			SAssignNew(CharacterNameInput, SEditableTextBox)
			.HintText(FText::FromString(TEXT("캐릭터 이름 (2~12자)")))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
		[
			SAssignNew(SelectedTypeText, STextBlock)
			.Text(FText::FromString(TEXT("직업: 기사")))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(2.f)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.OnClicked(FOnClicked::CreateUObject(
					this, &US1CharacterSelectWidget::HandleSelectPlayerType, 1))
				[SNew(STextBlock).Text(FText::FromString(TEXT("기사")))]
			]
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(2.f)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.OnClicked(FOnClicked::CreateUObject(
					this, &US1CharacterSelectWidget::HandleSelectPlayerType, 2))
				[SNew(STextBlock).Text(FText::FromString(TEXT("마법사")))]
			]
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(2.f)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.OnClicked(FOnClicked::CreateUObject(
					this, &US1CharacterSelectWidget::HandleSelectPlayerType, 3))
				[SNew(STextBlock).Text(FText::FromString(TEXT("궁수")))]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
		[
			SAssignNew(CreateButton, SButton)
			.HAlign(HAlign_Center)
			.OnClicked(FOnClicked::CreateUObject(
				this, &US1CharacterSelectWidget::HandleCreateClicked))
			[SNew(STextBlock).Text(FText::FromString(TEXT("캐릭터 생성")))]
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SAssignNew(BackButton, SButton)
			.HAlign(HAlign_Center)
			.OnClicked(FOnClicked::CreateUObject(
				this, &US1CharacterSelectWidget::HandleBackClicked))
			[SNew(STextBlock).Text(FText::FromString(TEXT("목록으로")))]
		];

	return SNew(SBorder)
		.Padding(32.f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.BorderBackgroundColor(FLinearColor(0.025f, 0.035f, 0.055f, 0.96f))
		[
			SNew(SBox).WidthOverride(520.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				.Padding(0.f, 0.f, 0.f, 20.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("CHARACTER SELECT")))
					.Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 28))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 14.f)
				[
					SAssignNew(StatusText, STextBlock)
					.AutoWrapText(true)
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SAssignNew(PanelSwitcher, SWidgetSwitcher)
					.WidgetIndex(CHARACTER_LIST_PANEL)
					+ SWidgetSwitcher::Slot()[ListPanel]
					+ SWidgetSwitcher::Slot()[CreatePanel]
				]
			]
		];
}

void US1CharacterSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (US1GameInstance* GameInstance = GetGameInstance<US1GameInstance>())
	{
		GameInstance->OnCharacterCreated.AddUniqueDynamic(
			this, &US1CharacterSelectWidget::HandleCharacterCreated);
		GameInstance->OnEnterGameCompleted.AddUniqueDynamic(
			this, &US1CharacterSelectWidget::HandleEnterGameCompleted);
	}
	RefreshCharacterList();
}

void US1CharacterSelectWidget::NativeDestruct()
{
	if (US1GameInstance* GameInstance = GetGameInstance<US1GameInstance>())
	{
		GameInstance->OnCharacterCreated.RemoveDynamic(
			this, &US1CharacterSelectWidget::HandleCharacterCreated);
		GameInstance->OnEnterGameCompleted.RemoveDynamic(
			this, &US1CharacterSelectWidget::HandleEnterGameCompleted);
	}
	Super::NativeDestruct();
}

void US1CharacterSelectWidget::RefreshCharacterList()
{
	if (!CharacterListBox.IsValid())
		return;

	CharacterListBox->ClearChildren();
	SelectedCharacterId = 0;

	US1GameInstance* GameInstance = GetGameInstance<US1GameInstance>();
	if (GameInstance == nullptr)
		return;

	const TArray<FS1CharacterSummary>& Characters =
		GameInstance->GetCharacters();

	if (Characters.IsEmpty())
	{
		CharacterListBox->AddSlot().AutoHeight().Padding(0.f, 4.f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("캐릭터가 없습니다. 새 캐릭터를 만들어 주세요.")))
		];
	}

	for (const FS1CharacterSummary& Character : Characters)
	{
		const int64 CharacterId = Character.CharacterId;
		const FString Label = FString::Printf(
			TEXT("%s  |  Lv.%d  |  %s"),
			*Character.CharacterName,
			Character.Level,
			*GetPlayerTypeName(Character.PlayerType));

		CharacterListBox->AddSlot().AutoHeight().Padding(0.f, 4.f)
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
			.OnClicked(FOnClicked::CreateUObject(
				this,
				&US1CharacterSelectWidget::HandleSelectCharacter,
				CharacterId))
			[SNew(STextBlock).Text(FText::FromString(Label))]
		];
	}

	if (!Characters.IsEmpty())
		HandleSelectCharacter(Characters[0].CharacterId);

	if (OpenCreateButton.IsValid())
		OpenCreateButton->SetEnabled(Characters.Num() < 3);
	if (PlayButton.IsValid())
		PlayButton->SetEnabled(SelectedCharacterId > 0);
}

FReply US1CharacterSelectWidget::HandleSelectCharacter(int64 CharacterId)
{
	SelectedCharacterId = CharacterId;
	if (const US1GameInstance* GameInstance = GetGameInstance<US1GameInstance>())
	{
		for (const FS1CharacterSummary& Character : GameInstance->GetCharacters())
		{
			if (Character.CharacterId == CharacterId && SelectedCharacterText.IsValid())
			{
				SelectedCharacterText->SetText(FText::FromString(
					FString::Printf(TEXT("선택: %s"), *Character.CharacterName)));
				break;
			}
		}
	}
	if (PlayButton.IsValid())
		PlayButton->SetEnabled(!bRequestInFlight && CharacterId > 0);
	return FReply::Handled();
}

FReply US1CharacterSelectWidget::HandlePlayClicked()
{
	if (bRequestInFlight || SelectedCharacterId <= 0)
		return FReply::Handled();
	if (US1GameInstance* GameInstance = GetGameInstance<US1GameInstance>())
	{
		SetRequestInFlight(true);
		SetStatus(TEXT("게임에 접속하는 중..."), FLinearColor(0.65f, 0.8f, 1.f));
		GameInstance->EnterCharacter(SelectedCharacterId);
	}
	return FReply::Handled();
}

FReply US1CharacterSelectWidget::HandleOpenCreateClicked()
{
	if (!bRequestInFlight && PanelSwitcher.IsValid())
	{
		PanelSwitcher->SetActiveWidgetIndex(CHARACTER_CREATE_PANEL);
		SetStatus(TEXT("이름과 직업을 선택하세요."), FLinearColor::White);
	}
	return FReply::Handled();
}

FReply US1CharacterSelectWidget::HandleCreateClicked()
{
	if (bRequestInFlight || !CharacterNameInput.IsValid())
		return FReply::Handled();

	const FString CharacterName = CharacterNameInput->GetText().ToString();
	FString ErrorMessage;
	if (!ValidateCharacterName(CharacterName, ErrorMessage))
	{
		SetStatus(ErrorMessage, FLinearColor(1.f, 0.25f, 0.25f));
		return FReply::Handled();
	}

	if (US1GameInstance* GameInstance = GetGameInstance<US1GameInstance>())
	{
		SetRequestInFlight(true);
		SetStatus(TEXT("캐릭터 생성 중..."), FLinearColor(0.65f, 0.8f, 1.f));
		GameInstance->CreateCharacter(CharacterName, SelectedPlayerType);
	}
	return FReply::Handled();
}

FReply US1CharacterSelectWidget::HandleBackClicked()
{
	if (!bRequestInFlight && PanelSwitcher.IsValid())
	{
		PanelSwitcher->SetActiveWidgetIndex(CHARACTER_LIST_PANEL);
		SetStatus(FString(), FLinearColor::White);
	}
	return FReply::Handled();
}

FReply US1CharacterSelectWidget::HandleSelectPlayerType(int32 PlayerType)
{
	SelectedPlayerType = PlayerType;
	if (SelectedTypeText.IsValid())
	{
		SelectedTypeText->SetText(FText::FromString(
			FString::Printf(TEXT("직업: %s"), *GetPlayerTypeName(PlayerType))));
	}
	return FReply::Handled();
}

void US1CharacterSelectWidget::HandleCharacterCreated(
	bool bSuccess,
	const FString& Message)
{
	SetRequestInFlight(false);
	if (!bSuccess)
	{
		SetStatus(
			Message.IsEmpty() ? TEXT("캐릭터 생성에 실패했습니다.") : Message,
			FLinearColor(1.f, 0.25f, 0.25f));
		return;
	}

	if (CharacterNameInput.IsValid())
		CharacterNameInput->SetText(FText::GetEmpty());
	if (PanelSwitcher.IsValid())
		PanelSwitcher->SetActiveWidgetIndex(CHARACTER_LIST_PANEL);
	int64 CreatedCharacterId = 0;
	if (const US1GameInstance* GameInstance = GetGameInstance<US1GameInstance>())
	{
		if (!GameInstance->GetCharacters().IsEmpty())
			CreatedCharacterId = GameInstance->GetCharacters().Last().CharacterId;
	}
	RefreshCharacterList();
	if (CreatedCharacterId > 0)
		HandleSelectCharacter(CreatedCharacterId);
	SetStatus(TEXT("캐릭터가 생성되었습니다. 플레이를 눌러 입장하세요."),
		FLinearColor(0.25f, 1.f, 0.45f));
}

void US1CharacterSelectWidget::HandleEnterGameCompleted(
	bool bSuccess,
	const FString& Message)
{
	SetRequestInFlight(false);
	SetStatus(
		bSuccess ? TEXT("게임 접속 성공") : Message,
		bSuccess ? FLinearColor(0.25f, 1.f, 0.45f)
			: FLinearColor(1.f, 0.25f, 0.25f));
}

bool US1CharacterSelectWidget::ValidateCharacterName(
	const FString& CharacterName,
	FString& ErrorMessage) const
{
	if (CharacterName.Len() < 2 || CharacterName.Len() > 12)
	{
		ErrorMessage = TEXT("캐릭터 이름은 2~12자로 입력하세요.");
		return false;
	}

	for (TCHAR Character : CharacterName)
	{
		const bool bEnglish =
			(Character >= 'A' && Character <= 'Z') ||
			(Character >= 'a' && Character <= 'z');
		const bool bNumber = Character >= '0' && Character <= '9';
		const bool bKorean = Character >= 0xAC00 && Character <= 0xD7A3;
		if (!bEnglish && !bNumber && !bKorean)
		{
			ErrorMessage = TEXT("이름에는 한글, 영문, 숫자만 사용할 수 있습니다.");
			return false;
		}
	}
	return true;
}

void US1CharacterSelectWidget::SetRequestInFlight(bool bInFlight)
{
	bRequestInFlight = bInFlight;
	if (PlayButton.IsValid()) PlayButton->SetEnabled(!bInFlight && SelectedCharacterId > 0);
	if (OpenCreateButton.IsValid()) OpenCreateButton->SetEnabled(!bInFlight);
	if (CreateButton.IsValid()) CreateButton->SetEnabled(!bInFlight);
	if (BackButton.IsValid()) BackButton->SetEnabled(!bInFlight);
}

void US1CharacterSelectWidget::SetStatus(
	const FString& Message,
	const FLinearColor& Color)
{
	if (!StatusText.IsValid())
		return;
	StatusText->SetText(FText::FromString(Message));
	StatusText->SetColorAndOpacity(Color);
}

FString US1CharacterSelectWidget::GetPlayerTypeName(int32 PlayerType) const
{
	switch (PlayerType)
	{
	case 1: return TEXT("기사");
	case 2: return TEXT("마법사");
	case 3: return TEXT("궁수");
	default: return TEXT("알 수 없음");
	}
}
