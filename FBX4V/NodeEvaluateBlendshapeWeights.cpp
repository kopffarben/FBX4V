
#include "Stdafx.h"
#include "NodeEvaluateBlendshapeWeights.h"

using namespace VVVV::Nodes::FBX;

void FBXEvaluateBlendshapeWeightsNode::Evaluate(int SpreadMax)
{
	if (!this->FScene->IsConnected) return;
	if (this->FScene->SliceCount == 0) return;
	if (this->FScene[0] == nullptr) return;
	if (this->FScene[0]->FBXThreadContext->Working) return;

	String^ scenename = this->FScene[0]->GUID;

	bool init = false;
	if (!this->PrevSceneGUID->Equals(scenename))
	{
		init = true;
		EnumManager::UpdateEnum(this->AnimStackEnumName, FScene[0]->AnimStackNames[0], FScene[0]->AnimStackNames->ToArray());
	}
	FAnimStackIn->GetOrd(0, SelectedAnimStack);
	if(init)
	{
		FAnimStackIn->GetOrd(0, SelectedAnimStack);
		this->FScene[0]->FBXThreadContext->Action = gcnew Action(this, &FBXEvaluateBlendshapeWeightsNode::FBXDiscoverMeshes);
		this->FScene[0]->FBXThreadContext->Set = true;
		this->FScene[0]->FBXThreadContext->CallerWaitHandle->WaitOne();
	}

	this->PrevSceneGUID = scenename;
	if(FTime->Stream->IsChanged || FAnimStackIn->PinIsChanged)
	{
		this->FScene[0]->FBXThreadContext->Action = gcnew Action(this, &FBXEvaluateBlendshapeWeightsNode::FBXEvaluate);
		this->FScene[0]->FBXThreadContext->Set = true;
		this->FScene[0]->FBXThreadContext->CallerWaitHandle->WaitOne();
	}
	init = false;
}
void FBXEvaluateBlendshapeWeightsNode::FBXEvaluate()
{
	FWeights->SliceCount = TotalShapeCount * FTime->SliceCount;
	for(int i=0; i<FTime->SliceCount; i++)
	{
		for(int ts=0; ts<TotalShapeCount; ts++)
		{
			int ii = ts + i * TotalShapeCount;
			FbxAnimCurve* ac = AnimCurves[ts];
			double time = (FRelative[0]) ? FTime[i] * CurrAnimStack->LocalStop.Get().GetSecondDouble() : FTime[i];
			FWeights[ii] = ac->Evaluate(FBXUtils::SecondsToTime(time))/100;
		}
	}
}
void FBXEvaluateBlendshapeWeightsNode::FBXDiscoverMeshes()
{
	this->FMeshPath->SliceCount = 0;
	for (int i = 0; i < this->FNodePath->SliceCount; i++)
	{
		FbxNode* srcnode = FBXUtils::GetNodeFromPath(FScene[0]->Object, FNodePath[i]);
		RecursiveList(srcnode);
	}

	SelectedAnimStack = Math::Min(SelectedAnimStack, FScene[0]->Object->GetSrcObjectCount<FbxAnimStack>() - 1);
	CurrAnimStack = FScene[0]->Object->GetSrcObject<FbxAnimStack>(SelectedAnimStack);

	Meshes = gcnew array<FbxMesh*>(this->FMeshPath->SliceCount);
	FWeightsBin->SliceCount = FMeshPath->SliceCount;
	int totalshapecount = 0;
	for (int i = 0; i < this->FMeshPath->SliceCount; i++)
	{
		int cshapecount = 0;
		Meshes[i] = (FbxMesh*)FBXUtils::GetAttributeFromPath(FScene[0]->Object, FMeshPath[i]);
		FbxMesh* mesh = Meshes[i];
		for (int d = 0; d < mesh->GetDeformerCount(FbxDeformer::EDeformerType::eBlendShape); d++)
		{
			FbxBlendShape* bshape = (FbxBlendShape*)mesh->GetDeformer(d, FbxDeformer::EDeformerType::eBlendShape);
			for (int sc = 0; sc < bshape->GetBlendShapeChannelCount(); sc++)
			{
				FbxBlendShapeChannel* channel = bshape->GetBlendShapeChannel(sc);
				for (int ts = 0; ts < channel->GetTargetShapeCount(); ts++)
				{
					totalshapecount++;
					cshapecount++;
				}
			}
		}
		FWeightsBin[i] = cshapecount;
	}

	Shapes = gcnew array<FbxShape*>(totalshapecount);
	AnimCurves = gcnew array<FbxAnimCurve*>(totalshapecount);
	FWeights->SliceCount = totalshapecount;
	TotalShapeCount = totalshapecount;

	int tsc = 0;
	for (int i = 0; i < this->FMeshPath->SliceCount; i++)
	{
		int cshapecount = 0;
		FbxMesh* mesh = Meshes[i];
		for (int d = 0; d < mesh->GetDeformerCount(FbxDeformer::EDeformerType::eBlendShape); d++)
		{
			FbxBlendShape* bshape = (FbxBlendShape*)mesh->GetDeformer(d, FbxDeformer::EDeformerType::eBlendShape);
			for (int sc = 0; sc < bshape->GetBlendShapeChannelCount(); sc++)
			{
				FbxBlendShapeChannel* channel = bshape->GetBlendShapeChannel(sc);
				for (int ts = 0; ts < channel->GetTargetShapeCount(); ts++)
				{
					FbxShape* shape = channel->GetTargetShape(ts);
					Shapes[tsc] = shape;

					FbxAnimLayer* layer = (FbxAnimLayer*)CurrAnimStack->GetSrcObject(FbxCriteria::ObjectType(FbxAnimLayer::ClassId), 0);
					FbxAnimCurve* curve = shape->GetBaseGeometry()->GetShapeChannel(d, sc, layer);
					AnimCurves[tsc] = curve;
					
					tsc++;
				}
			}
		}
	}
}

void FBXEvaluateBlendshapeWeightsNode::RecursiveList(FbxNode* parent)
{
	for (int i = 0; i<parent->GetNodeAttributeCount(); i++)
	{
		FbxNodeAttribute* nodeattr = parent->GetNodeAttributeByIndex(i);
		if (nodeattr->GetClassId().Is(FbxMesh::ClassId))
		{
			FbxMesh* mesh = (FbxMesh*)nodeattr;

			FBX4V_VVVV_SPREADADD(FMeshPath, FBXUtils::GetNodePath(parent) + "\\" + i);
		}
	}
	if (this->FRecursive[0])
	{
		for (int i = 0; i<parent->GetChildCount(); i++)
		{
			this->RecursiveList(parent->GetChild(i));
		}
	}
}