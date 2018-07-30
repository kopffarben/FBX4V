#pragma once

#include "Stdafx.h"
#include "WrapperClasses.h"
#include "FBXJoint.h"
#include "FBXUtils.h"

using namespace System::ComponentModel::Composition;
using namespace System::Runtime::InteropServices;
using namespace System::Threading;
using namespace System::Threading::Tasks;
using namespace VVVV::PluginInterfaces::V1;
using namespace VVVV::PluginInterfaces::V2;
using namespace VVVV::FBX;

namespace VVVV {
	namespace Nodes {
		namespace FBX {
			static ref class StaticProps
			{
			public:
				static bool MessageShown = false;
			};
			[PluginInfo(
				Name = "FBXFile",
				Category = "FBX",
				Author = "microdee",
				Tags = "import, scene, asset")]
			public ref class FBXFileNode : IPluginEvaluate, IDisposable
			{
			public:

				[Import()]
				IPluginHost2^ PluginHost;

				[Import()]
				ILogger^ FLogger;

				[Input("Filename", StringType = VVVV::PluginInterfaces::V2::StringType::Filename, FileMask = "FBX (*.fbx)|*.fbx")]
				Pin<String^>^ FFBXFile;

				[Input("Import Settings", DefaultString = "IOSRoot")]
				Pin<String^>^ FIOSettings;
				[Input("Destination CoordSys", DefaultEnumEntry = "MayaYUp")]
				Pin<DestinationCoordSys>^ FDstCoordSys;
				[Input("New Triangulation Method")]
				Pin<bool>^ FNewTriang;
				[Input("Remove Bad Polygons")]
				Pin<bool>^ FRemoveBadPolys;
				[Input("Bake AnimStacks")]
				Pin<bool>^ FBakeAnims;
				[Input("Baking Sampling FPS", DefaultValue = 30)]
				Pin<int>^ FBakeFPS;
				[Input("Reload", IsBang = true)]
				Pin<bool>^ FReload;

				[Output("Scene")]
				ISpread<FBXSceneWrapper^>^ FSCene;

				[Output("Version")]
				ISpread<String^>^ FVersion;
				[Output("Scene Name")]
				ISpread<String^>^ FName;
				[Output("Importing")]
				ISpread<bool>^ FImporting;
				[Output("Baking AnimStacks")]
				ISpread<bool>^ FBakingAnims;
				[Output("FBX Thread is Busy")]
				ISpread<bool>^ FBusy;
				[Output("FBX Message")]
				ISpread<String^>^ FFBXMessage;
				[Output("FBX Error")]
				ISpread<String^>^ FFBXError;
				[Output("FBX Success")]
				ISpread<String^>^ FFBXSuccess;

				virtual void Evaluate(int SpreadMax);

				[ImportingConstructor()]
				FBXFileNode(IPluginHost^ host)
				{
					this->FSkeleton = gcnew Skeleton();
					this->FBXMainThreadContext = gcnew ThreadContext();
					this->FBXMainThreadContext->Thread = Task::Factory->StartNew(gcnew Action(this, &FBXFileNode::FBXMainThread));
#ifdef BETA
					if(!StaticProps::MessageShown)
					{
						System::Windows::Forms::MessageBox::Show("VVVV loaded a Beta version of FBX4V.\nBeta version can be used only for testing purposes. Please do not use it in any project be it commercial or not.\nThanks for your understanding!", "FBX4V Beta Notice");
						StaticProps::MessageShown = true;
					}
#endif
#ifdef NONCOMMERCIAL
					if (!StaticProps::MessageShown)
					{
						System::Windows::Forms::MessageBox::Show("VVVV loaded a Non-Commercial version of FBX4V.\nNon-Commercial version can be used only for evaluation, education or research. Please do not use it in any commercial project.\nThanks for your understanding!", "FBX4V Non-Commercial Notice");
						StaticProps::MessageShown = true;
					}
#endif
#ifdef DEBUGVERSION
					if (!StaticProps::MessageShown)
					{
						System::Windows::Forms::MessageBox::Show("VVVV loaded a Debug version of FBX4V.\nThis version is significantly slower and it should be only used when debugging with Visual Studio", "FBX4V Debug Notice");
						StaticProps::MessageShown = true;
					}
#endif
				}

				~FBXFileNode()
				{
					FBXMainThreadContext->Quit = true;
				}
			private:
				FbxManager* Manager;
				FbxImporter* Importer;
				FbxScene* Scene;
				FBXSceneWrapper^ SceneW;
				Skeleton^ FSkeleton;
				ThreadContext^ FBXMainThreadContext;
				String^ Nodepath;
				int JointID;
				bool BakeIt;

				void FBXMainThread();
				void LoadFBX();
				void BakeAnims();
				void CreateNodes(FbxNode* pfbxnode, VVVV::FBX::FBXJoint^ pjoint);
				void DestroyFBX()
				{
					if (this->Scene != NULL) this->Scene->Destroy();
					if (this->Manager != NULL) this->Manager->Destroy();
				}
			};
		}
	}
}
