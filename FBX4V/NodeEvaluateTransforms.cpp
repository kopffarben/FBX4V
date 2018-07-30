
#include "Stdafx.h"
#include "NodeEvaluateTransforms.h"

using namespace VVVV::Nodes::FBX;

FBXEvaluateTransformsNode::FBXEvaluateTransformsNode(IPluginHost^ host)
{
	this->PrevSceneGUID = "";

	host->GetNodePath(false, Nodepath);
	this->AnimStackEnumName = "FBX4V.AnimStack." + this->Nodepath;
	EnumManager::UpdateEnum(this->AnimStackEnumName, "none", gcnew array<String^> {"none"});
	host->CreateEnumInput("AnimStack", TSliceMode::Dynamic, TPinVisibility::True, this->FAnimStackIn);
	this->FAnimStackIn->SetSubType(this->AnimStackEnumName);

	BakedAnimStack = gcnew Spread<MemoryStream^>();
	SampSize = gcnew Spread<int>();
	SampCount = gcnew Spread<int>();
	SampRate = gcnew Spread<int>();
	JointID = gcnew Spread<int>();
}

void FBXEvaluateTransformsNode::Evaluate(int SpreadMax)
{
	SprMax = Math::Max(FAnimStackIn->SliceCount, FTimeIn->SliceCount);
	if (!this->FScene->IsConnected)
	{
		FSkeletonOutput->SliceCount = 0;
		return;
	}
	if (this->FScene->SliceCount == 0)
	{
		FSkeletonOutput->SliceCount = 0;
		return;
	}
	if (this->FScene[0] == nullptr)
	{
		FSkeletonOutput->SliceCount = 0;
		return;
	}
	FSkeletonOutput->SliceCount = SprMax;
	FAbsTime->SliceCount = SprMax;
	BakedAnimStack->SliceCount = SprMax;
	SampSize->SliceCount = SprMax;
	SampCount->SliceCount = SprMax;
	SampRate->SliceCount = SprMax;
	JointID->SliceCount = SprMax;

	String^ scenename = this->FScene[0]->GUID;

	if (this->PrevSceneGUID->Equals(scenename))
	{
		if (FAnimStackIn->PinIsChanged || FTimeIn->Stream->IsChanged)
		{
			for(int i=0; i<SprMax; i++)
			{
				EvaluateBones(i);
			}
			this->FSkeletonOutput->Stream->IsChanged = true;
		}
		else
		{
			this->FSkeletonOutput->Stream->IsChanged = false;
		}
	}
	else
	{
		this->FJointList->SliceCount = FScene[0]->NodeHierarchy->JointTable->Count;
		this->FJointIDs->SliceCount = FScene[0]->NodeHierarchy->JointTable->Count;
		this->FJointPIDs->SliceCount = FScene[0]->NodeHierarchy->JointTable->Count;
		int jj = 0;
		for each (IJoint^ j in FScene[0]->NodeHierarchy->JointTable->Values)
		{
			FBXJoint^ fbxj = dynamic_cast<FBXJoint^>(j);
			if (fbxj != nullptr)
			{
				this->FJointList[jj] = fbxj->Name;
				this->FJointIDs[jj] = fbxj->Id;
				FBXJoint^ pjoint = dynamic_cast<FBXJoint^>(fbxj->Parent);
				if (pjoint != nullptr)
				{
					this->FJointPIDs[jj] = pjoint->Id;
				}
				else this->FJointPIDs[jj] = -1;
			}
			jj++;
		}
		EvaluateEnums();
		for(int i=0; i<SprMax; i++)
		{
			FSkeletonOutput[i] = safe_cast<Skeleton^>(FScene[0]->NodeHierarchy->DeepCopy());
			EvaluateBones(i);
		}
		this->FSkeletonOutput->Stream->IsChanged = true;
	}
	this->PrevSceneGUID = scenename;
	PrevSprMax = SprMax;
}

void FBXEvaluateTransformsNode::EvaluateBones(int ii)
{
	bool valid = true;
	if (
		FAnimStackIn->PinIsChanged ||
		!(this->PrevSceneGUID->Equals(this->FScene[0]->GUID)) ||
		PrevSprMax != SprMax
		)
	{
		String^ selas;
		this->FAnimStackIn->GetString(ii, selas);
		String^ fbbapath = Path::Combine(FScene[0]->FbmFolder, selas + ".fbba");

		if(File::Exists(fbbapath))
		{
			FileStream^ fbbafile = gcnew FileStream(fbbapath, FileMode::Open);
			if (this->BakedAnimStack[ii] != nullptr) delete this->BakedAnimStack[ii];
			this->BakedAnimStack[ii] = gcnew MemoryStream();
			this->BakedAnimStack[ii]->SetLength(fbbafile->Length);
			this->BakedAnimStack[ii]->Position = 0;
			fbbafile->Position = 0;
			fbbafile->CopyTo(this->BakedAnimStack[ii]);
			fbbafile->Close();
			this->BakedAnimStack[ii]->Position = 0;
			this->SampSize[ii] = FBXUtils::ReadInt(BakedAnimStack[ii]);
			this->SampCount[ii] = FBXUtils::ReadInt(BakedAnimStack[ii]);
			this->SampRate[ii] = FBXUtils::ReadInt(BakedAnimStack[ii]);
			delete fbbafile;
		}
		else
		{
			valid = false;
		}
	}
	if (!valid) return;
	double timein = FTimeIn[ii];

	double fcurrsamp = timein * SampRate[ii];
	if (FRelative[ii]) fcurrsamp = timein * (SampCount[ii] - 1);
	FAbsTime[ii] = fcurrsamp / SampRate[ii];

	int currsamp = (int)(Math::Floor(fcurrsamp)) % (SampCount[ii] - 1);
	double sampblend = fcurrsamp - Math::Floor(fcurrsamp);

	for each (IJoint^ j in FSkeletonOutput[ii]->JointTable->Values)
	{
		FBXJoint^ fbxj = dynamic_cast<FBXJoint^>(j);
		if (fbxj != nullptr)
		{
			if(fbxj->Id == 0)
			{
				fbxj->BaseTransform = Matrix4x4::Matrix4x4(
					1, 0, 0, 0,
					0, 1, 0, 0,
					0, 0, 1, 0,
					0, 0, 0, 1);
				continue;
			}
			int sampoffs = fbxj->Id * SampCount[ii] * SampSize[ii] + FBX4V_FBBA_METASIZE;
			Matrix4x4 cglobtr = FBXUtils::ReadSampleLinear(BakedAnimStack[ii], currsamp, sampblend, sampoffs, SampSize[ii], (SampCount[ii] - 1));
			FBXJoint^ pjoint = dynamic_cast<FBXJoint^>(fbxj->Parent);
			if(pjoint != nullptr)
			{
				sampoffs = pjoint->Id * SampCount[ii] * SampSize[ii] + FBX4V_FBBA_METASIZE;
				if(pjoint->Id == 0)
				{
					fbxj->BaseTransform = cglobtr;
				}
				else
				{
					Matrix4x4 pglobtr = FBXUtils::ReadSampleLinear(BakedAnimStack[ii], currsamp, sampblend, sampoffs, SampSize[ii], (SampCount[ii] - 1));
					fbxj->BaseTransform = cglobtr * VMath::Inverse(pglobtr);
				}
			}
			else
			{
				fbxj->BaseTransform = cglobtr;
			}
		}
	}
}
void FBXEvaluateTransformsNode::EvaluateEnums()
{
	array<String^>^ fbba = Directory::GetFiles(FScene[0]->FbmFolder, "*.fbba");
	int asc = fbba->Length;
	if (asc > 0)
	{
		array<String^>^ animstacks = gcnew array<String^>(asc);
		for (int i = 0; i<asc; i++)
		{
			animstacks[i] = Path::GetFileNameWithoutExtension(fbba[i]);
		}
		EnumManager::UpdateEnum(this->AnimStackEnumName, animstacks[0], animstacks);
	}
}