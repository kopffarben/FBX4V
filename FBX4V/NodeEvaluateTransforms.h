#pragma once

#include "Stdafx.h"
#include <limits>
#include "WrapperClasses.h"
#include "FBXJoint.h"
#include "FBXUtils.h"

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
				Name = "EvaluateTransforms",
				Category = "FBX",
				Author = "microdee",
				Tags = "import, scene, asset, treeview"
			)]
			public ref class FBXEvaluateTransformsNode : IPluginEvaluate
			{
			public:
				[Import()]
				IPluginHost2^ PluginHost;

				[Input("Scene")]
				Pin<FBXSceneWrapper^>^ FScene;

				[Input("Time")]
				Pin<double>^ FTimeIn;

				IEnumIn^ FAnimStackIn;

				[Input("Relative")]
				ISpread<bool>^ FRelative;


				[Output("Node Transform Hierarchy")]
				ISpread<Skeleton^>^ FSkeletonOutput;
				[Output("Joint Names", Visibility = PinVisibility::OnlyInspector)]
				ISpread<String^>^ FJointList;
				[Output("Joint ID's", Visibility = PinVisibility::OnlyInspector)]
				ISpread<int>^ FJointIDs;
				[Output("Joint Parent ID's", Visibility = PinVisibility::OnlyInspector)]
				ISpread<int>^ FJointPIDs;
				[Output("Absolute Time")]
				ISpread<double>^ FAbsTime;

				[ImportingConstructor()]
				FBXEvaluateTransformsNode(IPluginHost^ host);

				virtual void Evaluate(int SpreadMax);

			private:
				void EvaluateEnums();
				void EvaluateBones(int ii);

				FbxScene* Scene;
				String^ PrevSceneGUID;
				int SprMax;
				int PrevSprMax;
				String^ Nodepath;
				String^ AnimStackEnumName;
				Spread<MemoryStream^>^ BakedAnimStack;
				Spread<int>^ SampSize;
				Spread<int>^ SampCount;
				Spread<int>^ SampRate;
				Spread<int>^ JointID;
			};
		}
	}
}