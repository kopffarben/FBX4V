
#include "Stdafx.h"
#include "NodeMeshSplit.h"

using namespace VVVV::Nodes::FBX;

void FBXMeshNode::Evaluate(int SpreadMax)
{
	if (!this->FScene->IsConnected) return;
	if (this->FScene->SliceCount == 0) return;
	if (this->FScene[0] == nullptr) return;

	String^ scenename = this->FScene[0]->GUID;
	bool scenechanged = !this->PrevSceneGUID->Equals(scenename);

	if (scenechanged || FEval[0]) this->ReadyToWork = true;
	this->PrevSceneGUID = scenename;

	if (this->FScene[0]->FBXThreadContext->Working) return;

	if(this->ReadyToWork)
	{
		this->ReadyToWork = false;
		this->VertsEvalFinished = false;

		if(VerticesThread != nullptr)
		{
			delete VerticesThread;
			delete ShapesThread;
			delete SkinningThread;
		}
		if(MaterialNodeOffsets != nullptr)
			MaterialNodeOffsets->Clear();

		this->FEvaled[0] = false;

		this->FScene[0]->FBXThreadContext->Action = gcnew Action(this, &FBXMeshNode::FBXEvaluateVertices);
		this->FScene[0]->FBXThreadContext->Set = true;
	}
	if(this->VertsEvalFinished)
	{
		VertsEvalFinished = false;
		this->FEvaled[0] = true;

		//Fill Output streams
		if (FVertices[0] == nullptr) FVertices[0] = gcnew MemoryStream();
		FVertices[0]->Position = 0;
		FVertices[0]->SetLength((long)VerticesThread->Length);
		FVertices[0]->Position = 0;
		VerticesThread->Position = 0;
		VerticesThread->CopyTo(FVertices[0]);
		VerticesThread->Position = 0;

		if (FShapes[0] == nullptr) FShapes[0] = gcnew MemoryStream();
		FShapes[0]->Position = 0;
		FShapes[0]->SetLength((long)ShapesThread->Length);
		FShapes[0]->Position = 0;
		ShapesThread->Position = 0;
		ShapesThread->CopyTo(FShapes[0]);
		ShapesThread->Position = 0;

		if (FSkinning[0] == nullptr) FSkinning[0] = gcnew MemoryStream();
		FSkinning[0]->Position = 0;
		FSkinning[0]->SetLength((long)SkinningThread->Length);
		FSkinning[0]->Position = 0;
		SkinningThread->Position = 0;
		SkinningThread->CopyTo(FSkinning[0]);
		SkinningThread->Position = 0;
	}
}

void FBXMeshNode::FBXEvaluateVertices()
{
	String^ msgtext = "Start processing meshes";
	FScene[0]->LastMessage = msgtext;
	this->FMeshPath->SliceCount = 0;
	for (int i = 0; i<this->FNodePath->SliceCount; i++)
	{
		FbxNode* srcnode = FBXUtils::GetNodeFromPath(FScene[0]->Object, FNodePath[i]);
		RecursiveList(srcnode);
	}

	this->FBBMax->SliceCount = this->FMeshPath->SliceCount;
	this->FBBMin->SliceCount = this->FMeshPath->SliceCount;
	this->FWeightCount->SliceCount = this->FMeshPath->SliceCount;
	this->FShapeMeta->SliceCount = this->FMeshPath->SliceCount;
	this->FBoneBindings->SliceCount = this->FMeshPath->SliceCount;
	this->FHasUV->SliceCount = this->FMeshPath->SliceCount;
	this->FHasBlendShapes->SliceCount = this->FMeshPath->SliceCount;
	this->FHasSkinning->SliceCount = this->FMeshPath->SliceCount;

	this->FUVLayers->SliceCount = this->FMeshPath->SliceCount;
	this->FVertCount->SliceCount = this->FMeshPath->SliceCount;
	this->FVertFlags->SliceCount = this->FMeshPath->SliceCount;
	this->FMeshOffs->SliceCount = this->FMeshPath->SliceCount;

	if (VerticesThread != nullptr) delete VerticesThread;
	if (ShapesThread != nullptr) delete ShapesThread;
	if (SkinningThread != nullptr) delete SkinningThread;
	VerticesThread = gcnew MemoryStream();
	VerticesThread->Position = 0;
	ShapesThread = gcnew MemoryStream();
	ShapesThread->Position = 0;
	SkinningThread = gcnew MemoryStream();
	SkinningThread->Position = 0;

	// Prepare Materials
	msgtext = "Caching materials";
	FScene[0]->LastMessage = msgtext;

	this->MaterialNodeOffsets->Clear();
	this->VertexLayout->Clear();
	int offset = 0;
	for each (String^ np in this->FScene[0]->SequentialNodePaths)
	{
		FbxNode* node = FBXUtils::GetNodeFromPath(this->FScene[0]->Object, np);
		this->MaterialNodeOffsets->Add(np, offset);
		offset += node->GetMaterialCount();
	}

	for (int i = 0; i < this->FMeshPath->SliceCount; i++)
	{
		msgtext = "Initializing";
		FScene[0]->LastMessage = FMeshPath[i] + ": " + msgtext;
		FbxMesh* mesh = (FbxMesh*)FBXUtils::GetAttributeFromPath(FScene[0]->Object, FMeshPath[i]);

		//bounding box
		mesh->ComputeBBox();
		FbxDouble3 bbmin = mesh->BBoxMin.Get();
		FbxDouble3 bbmax = mesh->BBoxMax.Get();
		FBBMin[i] = Vector3D::Vector3D(bbmin[0], bbmin[1], bbmin[2]);
		FBBMax[i] = Vector3D::Vector3D(bbmax[0], bbmax[1], bbmax[2]);


		FUVLayers[i]->SliceCount = mesh->GetUVLayerCount();

		//texcoords
		FbxStringList uvlayers;
		mesh->GetUVSetNames(uvlayers);
		ISpread<String^>^ uvlsprd = FUVLayers[i];
		FHasUV[i] = false;
		for (int j = 0; j<mesh->GetUVLayerCount(); j++)
		{
			FHasUV[i] = true;
			uvlsprd[j] = gcnew String(uvlayers[j]);
		}

		//Skinning
		FBoneBindings[i]->SliceCount = 0;
		ISpread<SkinningBoneBinding^>^ bonesprd = FBoneBindings[i];
		FHasSkinning[i] = false;
		for (int j = 0; j<mesh->GetDeformerCount(FbxDeformer::EDeformerType::eSkin); j++)
		{
			FHasSkinning[i] = true;
			FbxSkin* skin = (FbxSkin*)mesh->GetDeformer(j, FbxDeformer::EDeformerType::eSkin);
			for (int k = 0; k<skin->GetClusterCount(); k++)
			{
				SkinningBoneBinding^ binding = gcnew SkinningBoneBinding();
				FbxCluster* cluster = skin->GetCluster(k);
				FbxNode* bonenode = cluster->GetLink();
				String^ bonename = FBXUtils::GetNodePath(bonenode);
				int boneid = FScene[0]->NodeHierarchy->JointTable[bonename]->Id;
				binding->NodePath = bonename;
				binding->BoneId = boneid;
				PoseList ContainingPoses;
				FbxArray<int> IdInPoses;

				FbxAMatrix tempmat;

				cluster->GetTransformMatrix(tempmat);
				binding->BoneTransform = FBXUtils::AsVVVVMatrix(tempmat);

				cluster->GetTransformLinkMatrix(tempmat);
				binding->BoneTransformLink = FBXUtils::AsVVVVMatrix(tempmat);

				cluster->GetTransformAssociateModelMatrix(tempmat);
				binding->BoneAssociateModel = FBXUtils::AsVVVVMatrix(tempmat);

				cluster->GetTransformParentMatrix(tempmat);
				binding->Parent = FBXUtils::AsVVVVMatrix(tempmat);

				binding->LinkMode = ManagedEnums::ClusterLinkMode()[cluster->GetLinkMode()];

				FbxVector4 lT = bonenode->GetGeometricTranslation(FbxNode::eSourcePivot);
				FbxVector4 lR = bonenode->GetGeometricRotation(FbxNode::eSourcePivot);
				FbxVector4 lS = bonenode->GetGeometricScaling(FbxNode::eSourcePivot);
				binding->GeometricTransform = FBXUtils::AsVVVVMatrix(FbxAMatrix(lT, lR, lS));

				bool ibmsuccess = FbxPose::GetBindPoseContaining(FScene[0]->Object, bonenode, ContainingPoses, IdInPoses);
				if (ibmsuccess)
				{
					FbxPose* assignedpose = ContainingPoses[0];
					int idinpose = assignedpose->Find(bonenode);
					FbxMatrix ibm = assignedpose->GetMatrix(idinpose).Inverse();
					binding->InversePose = FBXUtils::AsVVVVMatrix(ibm);
				}
				else
				{
					binding->InversePose = Matrix4x4::Matrix4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
				}
				FBX4V_VVVV_SPREADADD(bonesprd, binding);
			}
		}

		//Blendshapes
		FHasBlendShapes[i] = mesh->GetDeformerCount(FbxDeformer::EDeformerType::eBlendShape) > 0;

		//Vertex Layout
		FVertFlags[i] = 1;
		InsertVertexLayout("POSITION", "R32G32B32_Float", 12, 0);
		InsertVertexLayout("NORMAL", "R32G32B32_Float", 12, 0);
		FVertFlags[i] = FVertFlags[i] | 0x2;
		if(FAllocPrevPos[0])
		{
			InsertVertexLayout("PREVPOS", "R32G32B32_Float", 12, 0);
			FVertFlags[i] = FVertFlags[i] | (0x1 << 2);
		}
		if (FHasUV[i])
		{
			for (int j = 0; j<FUVLayers[i]->SliceCount; j++)
			{
				InsertVertexLayout("TEXCOORD", "R32G32_Float", 8, j);
				FVertFlags[i] = FVertFlags[i] | (0x1 << (3+j));
			}

			InsertVertexLayout("TANGENT", "R32G32B32A32_Float", 16, 0);
			FVertFlags[i] = FVertFlags[i] | (0x1 << 13);
			InsertVertexLayout("BINORMAL", "R32G32B32A32_Float", 16, 0);
			FVertFlags[i] = FVertFlags[i] | (0x1 << 14);
		}
		InsertVertexLayout("SUBSETID", "R32_UInt", 4, 0);
		FVertFlags[i] = FVertFlags[i] | (0x1 << 15);
		InsertVertexLayout("MATERIALID", "R32_UInt", 4, 0);
		FVertFlags[i] = FVertFlags[i] | (0x1 << 16);
		if (FAllocIID[0])
		{
			InsertVertexLayout("INSTANCEID", "R32_UInt", 4, 0);
			FVertFlags[i] = FVertFlags[i] | (0x1 << 17);
		}
		InsertVertexLayout("SUBSETVERTEXID", "R32_UInt", 4, 0);
		FVertFlags[i] = FVertFlags[i] | (0x1 << 18);
		InsertVertexLayout("FEATUREFLAGS", "R32_UInt", 4, 0);
		FVertFlags[i] = FVertFlags[i] | (0x1 << 19);

		int polycount = mesh->GetPolygonCount();
		FVertCount[i] = polycount * 3; // assuming triangulation went as expected
	}
	FVertexLayout->SliceCount = 0;
	FVertSize[0] = 0;
	for each (VertexComponent^ vc in VertexLayout->Values)
	{
		FBX4V_VVVV_SPREADADD(FVertexLayout, vc);
		FVertSize[0] += vc->Size;
	}

	VerticesThread->SetLength(0);
	VerticesThread->Position = 0;
	ShapesThread->SetLength(0);
	ShapesThread->Position = 0;
	SkinningThread->SetLength(0);
	SkinningThread->Position = 0;
	BoneBindingIdOffs = 0;

	for (int i = 0; i < this->FMeshPath->SliceCount; i++)
	{
		FbxMesh* mesh = (FbxMesh*)FBXUtils::GetAttributeFromPath(FScene[0]->Object, FMeshPath[i]);

		if (!mesh->IsTriangleMesh())
		{
			msgtext = "Mesh is not triangulated (somehow, despite all the efforts made at import)";
			FError[i] = msgtext;
			FScene[0]->LastError = FMeshPath[i] + ": " + msgtext;
		}

		if(FCalcNorm[0])
		{
			msgtext = "Generating Normals";
			FScene[0]->LastMessage = FMeshPath[i] + ": " + msgtext;
			if (!mesh->GenerateNormals(true, true, FGenNormCW[0]))
			{
				msgtext = "Failed generating normals";
				FError[i] = msgtext;
				FScene[0]->LastError = FMeshPath[i] + ": " + msgtext;
			}
		}

		int realuvcount = mesh->GetUVLayerCount();
		if(FHasUV[i])
		{
			msgtext = "Generating Tangents";
			FScene[0]->LastMessage = FMeshPath[i] + ": " + msgtext;
			int selecteduvlayer = FTanLayer[0] % FUVLayers[i]->SliceCount;
			if (!mesh->GenerateTangentsData(selecteduvlayer))
			{
				msgtext = "Failed generating tangents";
				FError[i] = msgtext;
				FScene[0]->LastError = FMeshPath[i] + ": " + msgtext;
			}
		}

		int indexcount = mesh->GetPolygonVertexCount();
		int* indicesfbx = mesh->GetPolygonVertices();

		msgtext = "Filling vertices";
		FScene[0]->LastMessage = FMeshPath[i] + ": " + msgtext;

		FbxGeometryElementTangent* tanelement;
		FbxGeometryElementBinormal* binormelement;
		bool hasmaterial = mesh->GetElementMaterialCount() > 0;
		FbxGeometryElementMaterial* matelement = mesh->GetElementMaterial(0);
		if (matelement == nullptr) hasmaterial = false;
		if (FHasUV[i])
		{
			tanelement = mesh->GetElementTangent(0);
			binormelement = mesh->GetElementBinormal(0);
		}
		int polycount = mesh->GetPolygonCount();
		int vc = 0;
		int bonecompstart = 0;
		int meshstartpos = (i==0) ? 0 : FMeshOffs[i];
		if (i != FMeshPath->SliceCount - 1) FMeshOffs[i + 1] = FMeshOffs[i] + polycount * 3 * FVertSize[0];
		VerticesThread->SetLength(VerticesThread->Length + polycount * 3 * FVertSize[0]);

		for(int p=0; p<polycount; p++)
		{
			for(int v=0; v<3; v++)
			{
				int vcp = 0;
				// vertex init
				int vertoffs = meshstartpos + vc * FVertSize[i];
				this->VerticesThread->Position = vertoffs;
				int ctrlp = mesh->GetPolygonVertex(p, v);

				// position
				FbxVector4 pos = mesh->GetControlPointAt(ctrlp);
				for (int ii = 0; ii<3; ii++)
					this->VerticesThread->Write(BitConverter::GetBytes((float)pos[ii]), 0, 4);

				// normal
				FbxVector4 norm;
				bool normres = mesh->GetPolygonVertexNormal(p, v, norm);
				if(normres)
				{
					for (int ii = 0; ii<3; ii++)
						this->VerticesThread->Write(BitConverter::GetBytes((float)norm[ii]), 0, 4);
				}
				else
				{
					for (int ii = 0; ii<3; ii++)
						this->VerticesThread->Write(BitConverter::GetBytes((float)0), 0, 4);
				}

				if(FHasUV[i])
				{
					// Texcoords
					ISpread<String^>^ uvlspr = FUVLayers[i];
					for(int uvl=0; uvl < uvlspr->SliceCount; uvl++)
					{
						FbxVector2 uv; bool uvunmapped;
						bool uvres = mesh->GetPolygonVertexUV(p, v, FBX4V_CHARPOINTER(uvlspr[uvl]), uv, uvunmapped);
						int uvoffs = VertexLayout["TEXCOORD" + uvl.ToString()]->Offset;
						this->VerticesThread->Position = vertoffs + uvoffs;
						if(uvres || !uvunmapped)
						{
							this->VerticesThread->Write(BitConverter::GetBytes((float)uv[0]), 0, 4);
							this->VerticesThread->Write(BitConverter::GetBytes((float)uv[1]), 0, 4);
						}
						else
						{
							this->VerticesThread->Write(BitConverter::GetBytes((float)0), 0, 4);
							this->VerticesThread->Write(BitConverter::GetBytes((float)0), 0, 4);
						}
					}

					// Tangents
					int tanoffs = VertexLayout["TANGENT0"]->Offset;
					this->VerticesThread->Position = vertoffs + tanoffs;
					FbxVector4 tangent = GetVertexDataFromElement(tanelement, ctrlp, vc);
					for (int ii = 0; ii<4; ii++)
						this->VerticesThread->Write(BitConverter::GetBytes((float)tangent[ii]), 0, 4);

					FbxVector4 binorm = GetVertexDataFromElement(binormelement, ctrlp, vc);
					for (int ii = 0; ii<4; ii++)
						this->VerticesThread->Write(BitConverter::GetBytes((float)binorm[ii]), 0, 4);
				}
				// end if HasUV

				// Material
				int ssidoffs = VertexLayout["SUBSETID0"]->Offset;
				this->VerticesThread->Position = vertoffs + ssidoffs;
				this->VerticesThread->Write(BitConverter::GetBytes((UInt32)i), 0, 4);

				int matidoffs = VertexLayout["MATERIALID0"]->Offset;
				this->VerticesThread->Position = vertoffs + matidoffs;
				int vmat = 0;
				if(hasmaterial) vmat = GetVertexDataFromElement(matelement, ctrlp, vc, mesh->GetNode());
				this->VerticesThread->Write(BitConverter::GetBytes((UInt32)vmat), 0, 4);

				int ssvidoffs = VertexLayout["SUBSETVERTEXID0"]->Offset;
				this->VerticesThread->Position = vertoffs + ssvidoffs;
				this->VerticesThread->Write(BitConverter::GetBytes((UInt32)vc), 0, 4);

				int flagsoffs = VertexLayout["FEATUREFLAGS0"]->Offset;
				this->VerticesThread->Position = vertoffs + flagsoffs;
				this->VerticesThread->Write(BitConverter::GetBytes(FVertFlags[i]), 0, 4);
				vc++;
			} // end for each vertex
		} // end for each polygon

		array<List<int>^>^ verticesperctrlp;
		int ctrlpcount = mesh->GetControlPointsCount();
		int vertindexcount = mesh->GetPolygonVertexCount();
		int* vertindices = mesh->GetPolygonVertices();
		if(FHasSkinning[i] || FHasBlendShapes[i])
		{
			// vertices mapped to their control point
			verticesperctrlp = gcnew array<List<int>^>(ctrlpcount);
			// ugly
			for (int ii = 0; ii<ctrlpcount; ii++)
			{
				verticesperctrlp[ii] = gcnew List<int>();
				for (int v = 0; v<vertindexcount; v++)
				{
					if (vertindices[v] == ii) verticesperctrlp[ii]->Add(v);
				}
			}
		}

		FShapeMeta[i] = gcnew ShapeData();
		if(FHasBlendShapes[i])
		{
			msgtext = "Filling BlendShapes";
			FScene[0]->LastMessage = FMeshPath[i] + ": " + msgtext;
			int currshape = 0;
			FShapeMeta[i]->ShapeSize = FVertCount[i] * 24;
			int shapestartpos = (i == 0) ? 0 : FShapeMeta[i]->FirstShapeOffset;
			for (int d = 0; d<mesh->GetDeformerCount(FbxDeformer::EDeformerType::eBlendShape); d++)
			{
				FHasBlendShapes[i] = true;
				FbxBlendShape* bshape = (FbxBlendShape*)mesh->GetDeformer(d, FbxDeformer::EDeformerType::eBlendShape);
				for (int sc = 0; sc<bshape->GetBlendShapeChannelCount(); sc++)
				{
					FbxBlendShapeChannel* channel = bshape->GetBlendShapeChannel(sc);
					for (int ts = 0; ts<channel->GetTargetShapeCount(); ts++)
					{
						FbxShape* shape = channel->GetTargetShape(ts);
						int cpc = shape->GetControlPointsCount();

						FbxGeometryElementNormal* snormelem = shape->GetElementNormal();
						ShapesThread->SetLength(ShapesThread->Length + FShapeMeta[i]->ShapeSize);

						for(int cp=0; cp<cpc; cp++)
						{
							FbxVector4 pos = shape->GetControlPointAt(cp);
							for(int v=0; v<verticesperctrlp[cp]->Count; v++)
							{
								int vid = verticesperctrlp[cp][v];
								int addr = shapestartpos + FShapeMeta[i]->ShapeSize * currshape + vid * 24;
								ShapesThread->Position = addr;

								for (int ii = 0; ii<3; ii++)
									this->ShapesThread->Write(BitConverter::GetBytes((float)pos[ii]), 0, 4);

								FbxVector4 snormal = GetVertexDataFromElement(snormelem, cp, vid);
								for (int ii = 0; ii<3; ii++)
									this->ShapesThread->Write(BitConverter::GetBytes((float)snormal[ii]), 0, 4);
							} // end for each verts in cp
						} // end for each cp
						currshape++;
					} // end for each target shape
				} // end for each channel
			} // end for each Shape Deformer
			FShapeMeta[i]->ShapeCount = currshape;
			if (i != FMeshPath->SliceCount - 1)
			{
				FShapeMeta[i + 1] = gcnew ShapeData();
				FShapeMeta[i + 1]->FirstShapeOffset = ShapesThread->Length;
			}
		} // end if FHasBlendShapes

		if (FHasSkinning[i])
		{
			msgtext = "Filling bones";
			FScene[0]->LastMessage = FMeshPath[i] + ": " + msgtext;
			int vertexweightsize = 8 + FMaxWpV[0] * 8;
			int skinningstartpos = (meshstartpos / FVertSize[0]) * vertexweightsize;
			SkinningThread->SetLength(skinningstartpos + polycount * 3 * vertexweightsize);
			Dictionary<int, List<Tuple<int, float>^>^>^ vertexweights = gcnew Dictionary<int, List<Tuple<int, float>^>^>();

			ISpread<SkinningBoneBinding^>^ bonenodesprd = FBoneBindings[i];

			for (int d = 0; d < mesh->GetDeformerCount(FbxDeformer::EDeformerType::eSkin); d++)
			{
				FbxSkin* skin = (FbxSkin*)mesh->GetDeformer(d, FbxDeformer::EDeformerType::eSkin);
				for (int c = 0; c < skin->GetClusterCount(); c++) // badum-tss
				{
					FbxCluster* cluster = skin->GetCluster(c);
					FbxCluster::ELinkMode linkmode = cluster->GetLinkMode();

					int indexcount = cluster->GetControlPointIndicesCount();
					int* indices = cluster->GetControlPointIndices();
					double* weights = cluster->GetControlPointWeights();
					String^ linkpath = FBXUtils::GetNodePath(cluster->GetLink());

					int boneid = 0;
					while(boneid < bonenodesprd->SliceCount)
					{
						if (bonenodesprd[boneid]->NodePath == linkpath) break;
						boneid++;
					}

					for (int cpi = 0; cpi < indexcount; cpi++)
					{
						int ctrlp = indices[cpi];
						float weight = (float)weights[cpi];
						for(int v=0; v<verticesperctrlp[ctrlp]->Count; v++)
						{
							int vid = verticesperctrlp[ctrlp][v];

							if(!vertexweights->ContainsKey(vid))
								vertexweights->Add(vid, gcnew List<Tuple<int, float>^>());

							vertexweights[vid]->Add(gcnew Tuple<int, float>(boneid + BoneBindingIdOffs, weight));
							int gvid = vid + meshstartpos / FVertSize[0];

							SkinningThread->Position = gvid * vertexweightsize + 4;
							SkinningThread->Write(BitConverter::GetBytes((UInt32)linkmode), 0, 4);
						}
					}
				} // end for each cluster
			} // end for each deformer
			for each (int v in vertexweights->Keys)
			{
				int vv = v + meshstartpos / FVertSize[0];
				List<Tuple<int, float>^>^ wlist = vertexweights[v];
				wlist->Sort(gcnew WeightComparer());
				wlist->Reverse();
				SkinningThread->Position = vv * vertexweightsize;
				SkinningThread->Write(BitConverter::GetBytes((UInt32)Math::Min(wlist->Count, FMaxWpV[0])), 0, 4);
				for(int vw = 0; vw < Math::Min(wlist->Count, FMaxWpV[0]); vw++)
				{
					SkinningThread->Position = vv * vertexweightsize + 8 + vw * 8;
					SkinningThread->Write(BitConverter::GetBytes((UInt32)wlist[vw]->Item1), 0, 4);
					SkinningThread->Write(BitConverter::GetBytes(wlist[vw]->Item2), 0, 4);
				}
			}
		} // end if HasSkinning
		BoneBindingIdOffs += FBoneBindings[i]->SliceCount;
	} // end for each mesh
	VertsEvalFinished = true;
	FScene[0]->LastSuccess = "Finished Meshes";
}
FbxVector4 FBXMeshNode::GetVertexDataFromElement(FbxLayerElementTemplate<FbxVector4>* element, int ctrlp, int vc)
{
	// Holy cow the amount of shit you have to go through for getting actual vertex data.
	// How on earth this format got this widespread???

	FbxVector4 result;
	switch (element->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (element->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			result = element->GetDirectArray().GetAt(ctrlp);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = element->GetIndexArray().GetAt(ctrlp);
			result = element->GetDirectArray().GetAt(index);
		}
		break;

		default:
			result = FbxVector4::FbxVector4(0, 0, 0, 1);
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (element->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			result = element->GetDirectArray().GetAt(vc);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = element->GetIndexArray().GetAt(vc);
			result = element->GetDirectArray().GetAt(index);
		}
		break;

		default:
			result = FbxVector4::FbxVector4(0, 0, 0, 1);
		}
		break;
	}
	return result;
}
int FBXMeshNode::GetVertexDataFromElement(FbxGeometryElementMaterial* element, int ctrlp, int vc, FbxNode* parentnode)
{
	int result;
	try
	{
		switch (element->GetMappingMode())
		{
		case FbxGeometryElement::eByControlPoint:
			switch (element->GetReferenceMode())
			{

			case FbxGeometryElement::eIndexToDirect:
			{
				result = element->GetIndexArray().GetAt(ctrlp);
			}
			break;

			default:
				result = 0;
			}
			break;

		case FbxGeometryElement::eByPolygonVertex:
			switch (element->GetReferenceMode())
			{
			case FbxGeometryElement::eIndexToDirect:
			{
				result = element->GetIndexArray().GetAt(vc);
			}
			break;

			default:
				result = 0;
			}
			break;
		}
		FbxSurfaceMaterial* mat = parentnode->GetMaterial(result);
		FbxScene* scene = parentnode->GetScene();
		for (int i = 0; i<scene->GetSrcObjectCount<FbxSurfaceMaterial>(); i++)
		{

			if (mat->GetUniqueID() == scene->GetSrcObject<FbxSurfaceMaterial>(i)->GetUniqueID())
			{
				result = i;
				break;
			}
		}
	}
	catch (const std::exception&)
	{
		result = 0;
	}
	return result;
}
void FBXMeshNode::RecursiveList(FbxNode* parent)
{
	for(int i=0; i<parent->GetNodeAttributeCount(); i++)
	{
		FbxNodeAttribute* nodeattr = parent->GetNodeAttributeByIndex(i);
		if(nodeattr->GetClassId().Is(FbxMesh::ClassId))
		{
			FbxMesh* mesh = (FbxMesh*)nodeattr;

			FBX4V_VVVV_SPREADADD(FMeshPath, FBXUtils::GetNodePath(parent) + "\\" + i);
		}
	}
	if(this->FRecursive[0])
	{
		for (int i = 0; i<parent->GetChildCount(); i++)
		{
			this->RecursiveList(parent->GetChild(i));
		}
	}
}
void FBXMeshNode::InsertVertexLayout(String^ name, String^ format, int size, int semid)
{
	if (this->VertexLayout == nullptr)
		this->VertexLayout = gcnew Dictionary<String^, VertexComponent^>();
	String^ uname = name + semid.ToString();
	if (VertexLayout->ContainsKey(uname)) return;

	int offs = 0;
	for each (VertexComponent^ el in this->VertexLayout->Values)
	{
		offs += el->Size;
	}
	this->VertexLayout->Add(uname, gcnew VertexComponent(name, format, size, offs, semid));
}