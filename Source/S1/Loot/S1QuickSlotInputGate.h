#pragma once
#include "Widgets/SWidget.h"

/** Compare the owning viewport instance, not its Slate type (PIE uses a derived viewport). */
struct FS1QuickSlotInputGate
{
 static bool AllowsFocus(const TSharedPtr<SWidget>& Focus,const TSharedPtr<SWidget>& OwningViewport,bool bModal)
 {return !bModal&&Focus.IsValid()&&OwningViewport.IsValid()&&Focus.Get()==OwningViewport.Get();}
};
