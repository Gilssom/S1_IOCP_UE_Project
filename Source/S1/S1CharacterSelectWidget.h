#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Input/Reply.h"
#include "S1CharacterSelectWidget.generated.h"

class SButton;
class SEditableTextBox;
class STextBlock;
class SVerticalBox;
class SWidgetSwitcher;

UCLASS()
class S1_API US1CharacterSelectWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void RefreshCharacterList();
	TSharedPtr<SWidget> GetInitialFocusTarget() const;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	friend class FS1AuthFocusTest;
	void RestoreInputFocus();
	FReply HandlePlayClicked();
	FReply HandleOpenCreateClicked();
	FReply HandleCreateClicked();
	FReply HandleBackClicked();
	FReply HandleSelectCharacter(int64 CharacterId);
	FReply HandleSelectPlayerType(int32 PlayerType);

	UFUNCTION()
	void HandleCharacterCreated(bool bSuccess, const FString& Message);

	UFUNCTION()
	void HandleEnterGameCompleted(bool bSuccess, const FString& Message);

	bool ValidateCharacterName(
		const FString& CharacterName,
		FString& ErrorMessage) const;
	void SetRequestInFlight(bool bInFlight);
	void SetStatus(const FString& Message, const FLinearColor& Color);
	FString GetPlayerTypeName(int32 PlayerType) const;

private:
	TSharedPtr<SWidgetSwitcher> PanelSwitcher;
	TSharedPtr<SVerticalBox> CharacterListBox;
	TSharedPtr<STextBlock> StatusText;
	TSharedPtr<STextBlock> SelectedCharacterText;
	TSharedPtr<STextBlock> SelectedTypeText;
	TSharedPtr<SEditableTextBox> CharacterNameInput;
	TSharedPtr<SButton> PlayButton;
	TSharedPtr<SButton> OpenCreateButton;
	TSharedPtr<SButton> CreateButton;
	TSharedPtr<SButton> BackButton;
	int64 SelectedCharacterId = 0;
	int32 SelectedPlayerType = 1;
	bool bRequestInFlight = false;
};
