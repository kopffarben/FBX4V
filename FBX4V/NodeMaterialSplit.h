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
			public ref class MaterialProperty
			{
			public:
				MaterialProperty()
				{
					Values = gcnew List<double>();
					TexturePaths = gcnew List<String^>();
				}
				~MaterialProperty()
				{
					
				}

				String^ Name;
				List<double>^ Values;
				List<String^>^ TexturePaths;
			private:

			};

			[PluginInfo(
				Name = "Materials",
				Category = "FBX",
				Author = "microdee",
				Tags = "import, scene, asset")]
			public ref class FBXMaterialSplitNode : IPluginEvaluate, IDisposable, IPartImportsSatisfiedNotification
			{
			public:

				[ImportingConstructor()]
				FBXMaterialSplitNode(IPluginHost^ host)
				{

				}
				~FBXMaterialSplitNode()
				{

				}
				virtual void OnImportsSatisfied()
				{
					this->PrevSceneGUID = "";
				}

				[Input("Scene")]
				Pin<FBXSceneWrapper^>^ FScene;

				[Output("Name")]
				ISpread<String^>^ FName;
				[Output("Shading Model")]
				ISpread<String^>^ FShadModel;
				[Output("Multilayer")]
				ISpread<bool>^ FMultilayer;

				[Output("Properties")]
				ISpread<ISpread<MaterialProperty^>^>^ FProp;

				/*
				[Output("Emissive")]
				ISpread<String^>^ FEmissive;
				[Output("Emissive Factor")]
				ISpread<String^>^ FEmissiveFact;
				[Output("Ambient")]
				ISpread<String^>^ FAmbient;
				[Output("Ambient Factor")]
				ISpread<String^>^ FAmbientFact;
				[Output("Diffuse")]
				ISpread<String^>^ FDiff;
				[Output("Diffuse Factor")]
				ISpread<String^>^ FDiffFact;
				[Output("Specular")]
				ISpread<String^>^ FSpec;
				[Output("Specular Factor")]
				ISpread<String^>^ FSpecFact;
				[Output("Shininess")]
				ISpread<String^>^ FShiny;
				[Output("Bump")]
				ISpread<String^>^ FBump;
				[Output("Normal Map")]
				ISpread<String^>^ FNormalMap;
				[Output("Bump Factor")]
				ISpread<String^>^ FBumpFact;
				[Output("Transparent Color")]
				ISpread<String^>^ FTransCol;
				[Output("Transparency Factor")]
				ISpread<String^>^ FTransFact;
				[Output("Reflection")]
				ISpread<String^>^ FRefl;
				[Output("Reflection Factor")]
				ISpread<String^>^ FReflFact;
				[Output("Displacement Color")]
				ISpread<String^>^ FDispCol;
				[Output("Displacement Factor")]
				ISpread<String^>^ FDispFact;
				[Output("Vector Displacement Color")]
				ISpread<String^>^ FVecDispCol;
				[Output("Vector Displacement Factor")]
				ISpread<String^>^ FVecDispFact;
				*/

				virtual void Evaluate(int SpreadMax);
			private:

				String^ PrevSceneGUID;
				FbxAnimStack* CurrAnimStack;
				int SelectedAnimStack;

				void FBXEvaluate();
			};
		}
	}
}