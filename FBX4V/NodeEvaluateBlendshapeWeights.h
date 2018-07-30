#pragma once

#include "Stdafx.h"
#include "WrapperClasses.h"
#include "FBXUtils.h"

using namespace System::ComponentModel::Composition;
using namespace System::Runtime::InteropServices;
using namespace System::Threading;
using namespace System::Threading::Tasks;
using namespace System::Collections::Generic;
using namespace VVVV::PluginInterfaces::V1;
using namespace VVVV::PluginInterfaces::V2;
using namespace VVVV::FBX;

namespace VVVV {
	namespace Nodes {
		namespace FBX {

			[PluginInfo(
				Name = "EvaluateBlendshapeWeights",
				Category = "FBX",
				Author = "microdee",
				Tags = "import, scene, asset")]
			public ref class FBXEvaluateBlendshapeWeightsNode : IPluginEvaluate, IDisposable, IPartImportsSatisfiedNotification
			{
			public:

				[ImportingConstructor()]
				FBXEvaluateBlendshapeWeightsNode(IPluginHost^ host)
				{
					host->GetNodePath(false, Nodepath);
					this->AnimStackEnumName = "FBX4V.AnimStack." + this->Nodepath;
					EnumManager::UpdateEnum(this->AnimStackEnumName, "none", gcnew array<String^> {"none"});
					host->CreateEnumInput("AnimStack", TSliceMode::Single, TPinVisibility::True, this->FAnimStackIn);
					this->FAnimStackIn->SetSubType(this->AnimStackEnumName);
				}
				~FBXEvaluateBlendshapeWeightsNode()
				{

				}
				virtual void OnImportsSatisfied()
				{
					this->PrevSceneGUID = "";
				}

				[Input("Scene")]
				Pin<FBXSceneWrapper^>^ FScene;
				[Input("Node Path")]
				Pin<String^>^ FNodePath;
				[Input("Recursive", DefaultBoolean = true)]
				Pin<bool>^ FRecursive;

				IEnumIn^ FAnimStackIn;

				[Input("Relative")]
				ISpread<bool>^ FRelative;
				[Input("Time")]
				Pin<double>^ FTime;

				[Output("Mesh Path")]
				ISpread<String^>^ FMeshPath;
				[Output("Weights")]
				ISpread<float>^ FWeights;
				[Output("Weights Bin Size")]
				ISpread<int>^ FWeightsBin;

				virtual void Evaluate(int SpreadMax);
			private:

				String^ PrevSceneGUID;
				array<FbxMesh*>^ Meshes;
				array<FbxShape*>^ Shapes;
				array<FbxAnimCurve*>^ AnimCurves;
				array<String^>^ AnimStackNames;
				int TotalShapeCount;
				String^ Nodepath;
				String^ AnimStackEnumName;
				FbxAnimStack* CurrAnimStack;
				int SelectedAnimStack;

				void FBXEvaluate();
				void FBXDiscoverMeshes();
				void RecursiveList(FbxNode* parent);
			};
		}
	}
}