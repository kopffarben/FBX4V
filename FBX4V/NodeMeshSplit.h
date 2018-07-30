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

			public ref class WeightComparer : Comparer<Tuple<int, float>^>
			{
			public:
				virtual int Compare(Tuple<int, float>^ x, Tuple<int, float>^ y) override
				{
					if (x->Item2 < y->Item2) return -1;
					else if (x->Item2 == y->Item2) return 0;
					else return 1;
				}
			};
			public ref class VertexComponent
			{
			public:
				VertexComponent() { }
				VertexComponent(String^ sem, String^ form, int s, int o, int sid)
				{
					Semantic = sem;
					Format = form;
					Size = s;
					Offset = o;
					SemanticIndex = sid;
				}
				~VertexComponent() { }
				String^ Semantic;
				String^ Format;
				int Size;
				int Offset;
				int SemanticIndex;
			};
			public ref class SkinningBoneBinding
			{
			public:
				SkinningBoneBinding() { }
				~SkinningBoneBinding() { }
				String^ NodePath;
				String^ LinkMode;
				int BoneId;
				Matrix4x4 InversePose;
				Matrix4x4 BoneTransform;
				Matrix4x4 BoneTransformLink;
				Matrix4x4 BoneAssociateModel;
				Matrix4x4 GeometricTransform;
				Matrix4x4 Parent;
			};

			public ref class ShapeData
			{
			public:
				ShapeData() { }
				ShapeData(int firstshapeoffset, int shapesize, int shapecount)
				{
					FirstShapeOffset = firstshapeoffset;
					ShapeSize = shapesize;
					ShapeCount = shapecount;
				}
				~ShapeData() { }
				int FirstShapeOffset;
				int ShapeSize;
				int ShapeCount;
			};

			[PluginInfo(
				Name = "Mesh",
				Category = "FBX",
				Author = "microdee",
				Tags = "import, scene, asset")]
			public ref class FBXMeshNode : IPluginEvaluate, IDisposable, IPartImportsSatisfiedNotification
			{
			public:

				[ImportingConstructor()]
				FBXMeshNode(IPluginHost^ host)
				{
					this->MaterialNodeOffsets = gcnew Dictionary<String^, int>();
					this->VertexLayout = gcnew Dictionary<String^, VertexComponent^>();
					this->PrevSceneGUID = "";
				}
				~FBXMeshNode()
				{
					try
					{
						delete VerticesThread;
						delete ShapesThread;
					}
					catch (const std::exception&)
					{

					}
				}
				virtual void OnImportsSatisfied()
				{
					FVertexLayout->SliceCount = 6;
					FVertexLayout[0] = gcnew VertexComponent("POSITION", "R32G32B32_Float", 12, 0, 0);
					FVertexLayout[1] = gcnew VertexComponent("NORMAL", "R32G32B32_Float", 12, 12, 0);
					FVertexLayout[2] = gcnew VertexComponent("SUBSETID", "R32_UInt", 4, 24, 0);
					FVertexLayout[3] = gcnew VertexComponent("MATERIALID", "R32_UInt", 4, 28, 0);
					FVertexLayout[4] = gcnew VertexComponent("SUBSETVERTEXID", "R32_UInt", 4, 32, 0);
					FVertexLayout[5] = gcnew VertexComponent("FEATUREFLAGS", "R32_UInt", 4, 36, 0);
					FVertSize[0] = 36;
				}

				[Input("Scene")]
				Pin<FBXSceneWrapper^>^ FScene;
				[Input("Node Path")]
				Pin<String^>^ FNodePath;
				[Input("Recursive", DefaultBoolean = true)]
				Pin<bool>^ FRecursive;
				[Input("Source UV Layer for Tangents")]
				Pin<int>^ FTanLayer;
				[Input("Overwrite Existing Tangents")]
				Pin<bool>^ FTanOverWrite;
				[Input("Generate Normals")]
				Pin<bool>^ FCalcNorm;
				[Input("CW Winding For Generated Normals")]
				Pin<bool>^ FGenNormCW;
				[Input("Inverse Normals")]
				Pin<bool>^ FInvNorm;
				[Input("Allocate PREVPOS Component")]
				Pin<bool>^ FAllocPrevPos;
				[Input("Allocate INSTANCEID Component")]
				Pin<bool>^ FAllocIID;
				[Input("Max Skinning Weights per Vertex", DefaultValue = 7)]
				Pin<int>^ FMaxWpV;
				[Input("Re-evaluate", IsBang = true)]
				Pin<bool>^ FEval;

				[Output("Mesh Path", Order = 0)]
				ISpread<String^>^ FMeshPath;
				[Output("Vertices", Order = 1)]
				ISpread<Stream^>^ FVertices;
				[Output("Vertex Count", Order = 2)]
				ISpread<int>^ FVertCount;
				[Output("Mesh Offset", Order = 3)]
				ISpread<int>^ FMeshOffs;
				[Output("Vertex Flags", Order = 4)]
				ISpread<UInt32>^ FVertFlags;

				[Output("Vertex Layout", Order = 5)]
				ISpread<VertexComponent^>^ FVertexLayout;
				[Output("Vertex Size", Order = 8)]
				ISpread<int>^ FVertSize;

				[Output("Has Texture Coordinates", Order = 9)]
				ISpread<bool>^ FHasUV;
				[Output("UV Layers", Order = 10, BinOrder = 11)]
				ISpread<ISpread<String^>^>^ FUVLayers;

				[Output("Bounding Box Min", Order = 12)]
				ISpread<Vector3D>^ FBBMin;
				[Output("Bounding Box Max", Order = 13)]
				ISpread<Vector3D>^ FBBMax;

				[Output("Has Skinning", Order = 14)]
				ISpread<bool>^ FHasSkinning;
				[Output("Bone Bindings", Order = 15, BinOrder = 16)]
				ISpread<ISpread<SkinningBoneBinding^>^>^ FBoneBindings;
				[Output("Skinning Data", Order = 23)]
				ISpread<Stream^>^ FSkinning;
				/*  skin:
				 *	vertexweightgroup[vertexcount]: (8 + mwc*8)
				 *		uint WeightCount
				 *		uint Mode (0: Normalize, 1: Additive, 2: TotalOne)
				 *		Weights[WeightCount]: (8)
				 *			uint BoneBindingInfoID
				 *			float Weight
				 */
				[Output("Weight Count", Order = 24)]
				ISpread<int>^ FWeightCount;

				[Output("Has BlendShapes", Order = 25)]
				ISpread<bool>^ FHasBlendShapes;
				[Output("Shapes", Order = 26)]
				ISpread<Stream^>^ FShapes;
				/*  shape:
				 *  vertex[vertcount]: (24)
				 *		float3 position
				 *		float3 normal
				 */
				[Output("Shape Metadata", Order = 27)]
				ISpread<ShapeData^>^ FShapeMeta;

				[Output("Evaluated", Order = 29)]
				ISpread<bool>^ FEvaled;
				[Output("Error", IsBang = true, Order = 30)]
				ISpread<String^>^ FError;

				virtual void Evaluate(int SpreadMax);
			private:
				MemoryStream^ VerticesThread;
				MemoryStream^ ShapesThread;
				MemoryStream^ SkinningThread;
				Dictionary<String^, int>^ MaterialNodeOffsets;
				Dictionary<String^, VertexComponent^>^ VertexLayout;

				String^ PrevSceneGUID;
				bool VertsEvalFinished;
				bool ReadyToWork;
				int BoneBindingIdOffs;

				void FBXEvaluateVertices();
				void RecursiveList(FbxNode* parent);
				void InsertVertexLayout(String^ name, String^ format, int size, int semid);
				FbxVector4 GetVertexDataFromElement(FbxLayerElementTemplate<FbxVector4>* element, int ctrlp, int vc);
				int GetVertexDataFromElement(FbxGeometryElementMaterial* element, int ctrlp, int vc, FbxNode* parentnode);
			};
		}
	}
}