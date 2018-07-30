
#include "Stdafx.h"
#include "NodeSceneExplorer.h"

using namespace VVVV::Nodes::FBX;

void FBXSceneExplorerNode::Evaluate(int SpreadMax)
{
	if (!this->FScene->IsConnected) return;
	if (this->FScene->SliceCount == 0) return;
	if (this->FScene[0] == nullptr) return;

	String^ scenename = this->FScene[0]->GUID;

	if (this->FScene[0]->FBXThreadContext->Working) return;

	if ((this->prevshow != this->FShow[0]) && this->PrevSceneGUID->Equals(scenename))
	{
		this->FScene[0]->FBXThreadContext->Action = gcnew Action(this, &FBXSceneExplorerNode::FBXEvaluate);
		this->FScene[0]->FBXThreadContext->Set = true;
	}
	this->prevshow = this->FShow[0];

	if (this->PrevSceneGUID->Equals(scenename)) return;

	this->PrevSceneGUID = scenename;

	this->FScene[0]->FBXThreadContext->Action = gcnew Action(this, &FBXSceneExplorerNode::FBXEvaluate);
	this->FScene[0]->FBXThreadContext->Set = true;

}
void FBXSceneExplorerNode::FBXEvaluate()
{
	if (this->FShow[0])
	{
		this->Window = gcnew FBXSceneExplorerWindow();
		if (this->FScene[0]->FBXThreadContext->Forms->ContainsKey("SceneExplorer"))
		{
			this->FScene[0]->FBXThreadContext->Forms["SceneExplorer"]->Close();
			this->FScene[0]->FBXThreadContext->Forms["SceneExplorer"] = this->Window;
		}
		else
		{
			this->FScene[0]->FBXThreadContext->Forms->Add("SceneExplorer", this->Window);
		}
		this->Window->Show();
		this->Window->InitializeComponent();
		this->Window->Activate();
		if (this->Window->SceneTreeNode->FirstNode != nullptr) this->Window->SceneTreeNode->FirstNode->Remove();
		this->Window->AddNodeToTree(this->FScene[0]->Object->GetRootNode(), this->Window->SceneTreeNode);
		this->Window->AddObjectToTree(this->FScene[0]->Object, this->Window->ObjectTreeNode);
		this->Window->Update();
	}
	else
	{
		if(this->Window != nullptr) this->Window->Hide();
	}
}
void FBXSceneExplorerNode::OnImportsSatisfied()
{
	this->PrevSceneGUID = "";
}



