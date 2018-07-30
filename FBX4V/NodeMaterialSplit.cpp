
#include "Stdafx.h"
#include "NodeMaterialSplit.h"

using namespace VVVV::Nodes::FBX;

void FBXMaterialSplitNode::Evaluate(int SpreadMax)
{
	if (!this->FScene->IsConnected) return;
	if (this->FScene->SliceCount == 0) return;
	if (this->FScene[0] == nullptr) return;
	if (this->FScene[0]->FBXThreadContext->Working) return;

	String^ scenename = this->FScene[0]->GUID;

	if (!this->PrevSceneGUID->Equals(scenename))
	{
		this->FScene[0]->FBXThreadContext->Action = gcnew Action(this, &FBXMaterialSplitNode::FBXEvaluate);
		this->FScene[0]->FBXThreadContext->Set = true;
		this->FScene[0]->FBXThreadContext->CallerWaitHandle->WaitOne();
	}

	this->PrevSceneGUID = scenename;
}
void FBXMaterialSplitNode::FBXEvaluate()
{
	int matcount = FScene[0]->Object->GetSrcObjectCount<FbxSurfaceMaterial>();
	FProp->SliceCount = matcount;
	FName->SliceCount = matcount;
	FShadModel->SliceCount = matcount;
	FMultilayer->SliceCount = matcount;
	for (int i = 0; i<matcount; i++)
	{
		ISpread<MaterialProperty^>^ propspread = FProp[i];
		FbxSurfaceMaterial* material = FScene[0]->Object->GetSrcObject<FbxSurfaceMaterial>(i);
		propspread->SliceCount = 0;
		FShadModel[i] = gcnew String(material->ShadingModel.Get());
		FMultilayer[i] = material->MultiLayer.Get();
		FName[i] = gcnew String(material->GetName());
		for(int p=0; p<FbxLayerElement::sTypeTextureCount; p++)
		{
			MaterialProperty^ matprop = gcnew MaterialProperty();
			FbxProperty prop = material->FindProperty(FbxLayerElement::sTextureChannelNames[p]);
			matprop->Name = gcnew String(prop.GetNameAsCStr());
			String^ cproptype = VVVV::FBX::ManagedEnums::FBXType()[prop.GetPropertyDataType().GetType()];
			if(cproptype == "eFbxDouble")
			{
				matprop->Values->Add(prop.Get<FbxDouble>());
			}
			if (cproptype == "eFbxDouble2")
			{
				matprop->Values->Add(prop.Get<FbxDouble2>()[0]);
				matprop->Values->Add(prop.Get<FbxDouble2>()[1]);
			}
			if (cproptype == "eFbxDouble3")
			{
				matprop->Values->Add(prop.Get<FbxDouble3>()[0]);
				matprop->Values->Add(prop.Get<FbxDouble3>()[1]);
				matprop->Values->Add(prop.Get<FbxDouble3>()[2]);
			}
			if (cproptype == "eFbxDouble4")
			{
				matprop->Values->Add(prop.Get<FbxDouble4>()[0]);
				matprop->Values->Add(prop.Get<FbxDouble4>()[1]);
				matprop->Values->Add(prop.Get<FbxDouble4>()[2]);
				matprop->Values->Add(prop.Get<FbxDouble4>()[3]);
			}
			for(int t=0; t<prop.GetSrcObjectCount<FbxFileTexture>(); t++)
			{
				FbxFileTexture* texture = (FbxFileTexture*)prop.GetSrcObject<FbxFileTexture>(t);
				matprop->TexturePaths->Add(gcnew String(texture->GetFileName()));
			}
			FBX4V_VVVV_SPREADADD(propspread, matprop);
		}
	}
}