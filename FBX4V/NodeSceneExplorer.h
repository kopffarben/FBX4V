#pragma once

#include "Stdafx.h"
#include "WrapperClasses.h"
#include "FBXTreeWindows.h"

using namespace System::IO;
using namespace System::Windows::Forms;
using namespace System::ComponentModel::Composition;
using namespace System::Runtime::InteropServices;
using namespace VVVV::PluginInterfaces::V1;
using namespace VVVV::PluginInterfaces::V2;
using namespace VVVV::Nodes::PDDN;
using namespace VVVV::FBX;

namespace VVVV {
	namespace Nodes {
		namespace FBX {

			[PluginInfo(
				Name = "SceneExplorer",
				Category = "FBX",
				Author = "microdee",
				Tags = "import, scene, asset, treeview",
				AutoEvaluate = true
			)]
			public ref class FBXSceneExplorerNode : IPluginEvaluate, IPartImportsSatisfiedNotification
			{
			public:
				[Import()]
				IPluginHost2^ PluginHost;

				[Input("Scene")]
				Pin<FBXSceneWrapper^>^ FScene;
				[Input("Show Window")]
				Pin<bool>^ FShow;

				virtual void Evaluate(int SpreadMax);
				virtual void OnImportsSatisfied();

			private:

				void FBXEvaluate();
				void SetWindowVisibility()
				{
					if(this->FShow[0]) this->Window->Show();
					else this->Window->Hide();
				}

				FBXSceneExplorerWindow^ Window;
				FbxScene* Scene;
				String^ PrevSceneGUID;
				bool prevshow;
			};
		}
	}
}