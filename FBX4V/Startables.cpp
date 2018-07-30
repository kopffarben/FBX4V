
#include "Stdafx.h"
#include "Startables.h"

using namespace VVVV::Nodes::FBX;

#ifdef BETA
void StartableBetaNotification::Start()
{
	System::Windows::Forms::MessageBox::Show("VVVV loaded a Beta version of FBX4V\nBeta version can be used only for testing purposes. Please do not use it in any project be it commercial or not.\nThanks for your understanding!", "FBX4V Beta Notice");
}
#endif
#ifdef NONCOMMERCIAL
void StartableNonCommercialNotification::Start()
{
	System::Windows::Forms::MessageBox::Show("VVVV loaded a Non-Commercial version of FBX4V\nNon-Commercial version can be used only for evaluation, education or research. Please do not use it in any commercial project.\nThanks for your understanding!", "FBX4V Non-Commercial Notice");
}
#endif