#pragma once

#include "Stdafx.h"
#include "Defines.h"

using namespace System::IO;
using namespace System::Windows::Forms;
using namespace System::ComponentModel::Composition;
using namespace System::Runtime::InteropServices;
using namespace VVVV::PluginInterfaces::V2;
using namespace VVVV::Nodes::PDDN;
using namespace VVVV::FBX;

namespace VVVV {
	namespace Nodes {
		namespace FBX {

#ifdef BETA
			[StartableAttribute(Name = "FBX4V Beta Notification", Lazy = false)]
			public ref class StartableBetaNotification : IStartable
			{
			public:
				virtual void Start();

				virtual void Shutdown()
				{

				}
			};
#endif
#ifdef NONCOMMERCIAL
			[StartableAttribute(Name = "FBX4V Non-Commercial Notification", Lazy = false)]
			public ref class StartableNonCommercialNotification : IStartable
			{
			public:
				virtual void Start();

				virtual void Shutdown()
				{

				}
			};
#endif
		}
	}
}