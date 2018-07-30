
#include "Stdafx.h"
#include "FBXTreeWindows.h"

using namespace VVVV::FBX;

// Scene Explorer
void FBXSceneExplorerWindow::InitializeComponent()
{
	TreeNode^ scenenode = gcnew TreeNode("Scene");
	TreeNode^ objectnode = gcnew TreeNode("Objects");
	this->Text = "FBX Scene Explorer";
	this->SceneTreeNode = scenenode;
	this->ObjectTreeNode = objectnode;
	this->FBXNodeTree = gcnew TreeView();
	this->FBXNodeTree->DrawMode = TreeViewDrawMode::OwnerDrawText;
	this->FBXNodeTree->AfterExpand += gcnew System::Windows::Forms::TreeViewEventHandler(this, &FBXSceneExplorerWindow::OnAfterExpand);
	this->FBXNodeTree->DrawNode += gcnew System::Windows::Forms::DrawTreeNodeEventHandler(this, &FBXSceneExplorerWindow::OnDrawTreeNode);
	this->SuspendLayout();
	//
	// FBXNodeTree
	//
	this->FBXNodeTree->BackColor = System::Drawing::Color::FromArgb(255, 64, 64, 64);
	this->FBXNodeTree->BorderStyle = System::Windows::Forms::BorderStyle::None;
	this->FBXNodeTree->Dock = System::Windows::Forms::DockStyle::Fill;
	this->FBXNodeTree->ForeColor = System::Drawing::Color::White;
	this->FBXNodeTree->LineColor = System::Drawing::Color::White;
	this->FBXNodeTree->Location = System::Drawing::Point(0, 0);
	this->FBXNodeTree->Name = "FBXNodeTree";
	scenenode->Name = "Scene";
	scenenode->Text = "Scene";
	objectnode->Name = "Objects";
	objectnode->Text = "Objects";
	this->FBXNodeTree->Nodes->Add(scenenode);
	this->FBXNodeTree->Nodes->Add(objectnode);

	this->BackColor = System::Drawing::Color::FromArgb(255, 64, 64, 64);
	this->ForeColor = System::Drawing::Color::White;
	this->Controls->Add(this->FBXNodeTree);
	this->Name = "FBX SceneExplorer";
	this->ResumeLayout(false);
}

void FBXSceneExplorerWindow::AddNodeToTree(FbxNode* fbxnode, TreeNode^ treenode)
{
	FbxScene* scene = fbxnode->GetScene();
	TreeNode^ currnode = treenode->Nodes->Add("Node: " + gcnew String(fbxnode->GetName()) + "   ");
	for (int i = 0; i<fbxnode->GetNodeAttributeCount(); i++)
	{
		FbxNodeAttribute* nodeattr = fbxnode->GetNodeAttributeByIndex(i);

		String^ attrtype = VVVV::FBX::ManagedEnums::AttributeType()[nodeattr->GetAttributeType()];
		String^ trimmedtype = attrtype->TrimStart('e');
		TreeNode^ attrnode = currnode->Nodes->Add(trimmedtype + ": " + gcnew String(nodeattr->GetName()) + "   ");

		if (nodeattr->GetClassId().Is(FbxMesh::ClassId))
		{
			FbxMesh* mesh = (FbxMesh*)nodeattr;
			attrnode->Nodes->Add("Vertex Count: " + mesh->GetControlPointsCount().ToString() + "   ");

			if (mesh->GetElementNormalCount() > 0)       attrnode->Nodes->Add("Element: Normal (" + mesh->GetElementNormalCount().ToString() + ")   ");
			if (mesh->GetElementUVCount() > 0)           attrnode->Nodes->Add("Element: UV (" + mesh->GetElementUVCount().ToString() + ")   ");
			if (mesh->GetElementTangentCount() > 0)      attrnode->Nodes->Add("Element: Tangent (" + mesh->GetElementTangentCount().ToString() + ")   ");
			if (mesh->GetElementBinormalCount() > 0)     attrnode->Nodes->Add("Element: Binormal (" + mesh->GetElementBinormalCount().ToString() + ")   ");
			if (mesh->GetElementMaterialCount() > 0)     attrnode->Nodes->Add("Element: Material (" + mesh->GetElementMaterialCount().ToString() + ")   ");
			if (mesh->GetElementPolygonGroupCount() > 0) attrnode->Nodes->Add("Element: Polygon Group (" + mesh->GetElementPolygonGroupCount().ToString() + ")   ");
			if (mesh->GetElementVertexColorCount() > 0)  attrnode->Nodes->Add("Element: Color (" + mesh->GetElementVertexColorCount().ToString() + ")   ");
			if (mesh->GetElementUserDataCount() > 0)     attrnode->Nodes->Add("Element: User Data (" + mesh->GetElementUserDataCount().ToString() + ")   ");

			for (int j = 0; j<mesh->GetDeformerCount(); j++)
			{
				FbxDeformer* deformer = mesh->GetDeformer(j);
				String^ deftype = gcnew String(VVVV::FBX::ManagedEnums::DeformerType()[deformer->GetDeformerType()]);
				deftype = deftype->TrimStart('e');
				TreeNode^ defnode = attrnode->Nodes->Add("Deformer: " + deftype + " (" + gcnew String(deformer->GetName()) + ")   ");
				if(deformer->Is<FbxBlendShape>())
				{
					FbxBlendShape* defblendshape = (FbxBlendShape*)deformer;
					for(int bc = 0; bc<defblendshape->GetBlendShapeChannelCount(); bc++)
					{
						FbxBlendShapeChannel* channel = defblendshape->GetBlendShapeChannel(bc);
						TreeNode^ channelnode = defnode->Nodes->Add("BlendShapeChannel: " + gcnew String(channel->GetName()) + "   ");
						for(int ts = 0; ts<channel->GetTargetShapeCount(); ts++)
						{
							FbxShape* shape = channel->GetTargetShape(ts);
							channelnode->Nodes->Add("Shape: " + gcnew String(shape->GetName()) + "   ");
						}
					}
				}
			}
			/*
			for (int bsd = 0; bsd < scene->GetSrcObjectCount(FbxCriteria::ObjectType(FbxBlendShape::ClassId)); bsd++)
			{
				FbxBlendShape* auxdefblendshape = (FbxBlendShape*)scene->GetSrcObject(FbxCriteria::ObjectType(FbxBlendShape::ClassId), bsd);
				String^ deftype = gcnew String(VVVV::FBX::ManagedEnums::DeformerType()[auxdefblendshape->GetDeformerType()]);
				deftype = deftype->TrimStart('e');
				TreeNode^ defnode = attrnode->Nodes->Add("Deformer: " + deftype + " (" + gcnew String(auxdefblendshape->GetName()) + ")   ");
				for (int bc = 0; bc<auxdefblendshape->GetBlendShapeChannelCount(); bc++)
				{
					FbxBlendShapeChannel* channel = auxdefblendshape->GetBlendShapeChannel(bc);
					TreeNode^ channelnode = defnode->Nodes->Add("BlendShapeChannel: " + gcnew String(channel->GetName()) + "   ");
					for (int ts = 0; ts<channel->GetTargetShapeCount(); ts++)
					{
						FbxShape* shape = channel->GetTargetShape(ts);
						channelnode->Nodes->Add("Shape: " + gcnew String(shape->GetName()) + "   ");
					}
				}
			}
			*/
		}
		else if (nodeattr->GetClassId().Is(FbxSkeleton::ClassId))
		{
			FbxSkeleton* skeleton = (FbxSkeleton*)nodeattr;

			String^ skttype = VVVV::FBX::ManagedEnums::SkeletonType()[skeleton->GetSkeletonType()];
			attrnode->Nodes->Add("Type: " + skttype + "   ");
		}
		for (int j = 0; j<fbxnode->GetMaterialCount(); j++)
		{
			FbxSurfaceMaterial* nodemat = fbxnode->GetMaterial(j);
			TreeNode^ mattreenode = currnode->Nodes->Add("Material: " + gcnew String(nodemat->GetName()) + "   ");
			mattreenode->Nodes->Add("Shading Model: " + gcnew String(nodemat->ShadingModel.Get()) + "   ");
			for (int k = 0; k<FbxLayerElement::sTypeTextureCount; k++)
			{
				this->AddPropertyToTree(nodemat->FindProperty(FbxLayerElement::sTextureChannelNames[k]), mattreenode);
			}

			if (nodemat->GetSrcPropertyCount() > 0)
				this->AddPropertyToTree(nodemat->RootProperty, mattreenode);
		}

		if (nodeattr->GetSrcPropertyCount() > 0)
			this->AddPropertyToTree(nodeattr->RootProperty, attrnode);
	}
	if (fbxnode->GetSrcPropertyCount() > 0)
		this->AddPropertyToTree(fbxnode->RootProperty, currnode);
	for (int i = 0; i<fbxnode->GetChildCount(); i++)
	{
		this->AddNodeToTree(fbxnode->GetChild(i), currnode);
	}
}
void FBXSceneExplorerWindow::AddPropertyToTree(FbxProperty currentprop, TreeNode^ treenode)
{
	String^ cproptype = VVVV::FBX::ManagedEnums::FBXType()[currentprop.GetPropertyDataType().GetType()];
	cproptype = cproptype->Substring(4);

	TreeNode^ proptreenode = treenode->Nodes->Add(
		"Property " + cproptype + ": " +
		gcnew String(currentprop.GetNameAsCStr()) + "   ");

	for (int i = 0; i<currentprop.GetSrcObjectCount(FbxCriteria::ObjectType(FbxFileTexture::ClassId)); i++)
	{
		FbxFileTexture* texture = (FbxFileTexture*)currentprop.GetSrcObject(FbxCriteria::ObjectType(FbxFileTexture::ClassId), i);
		proptreenode->Nodes->Add("Texture: " + gcnew String(texture->GetFileName()) + "   ");
	}
	for (int i = 0; i<currentprop.GetSrcPropertyCount(); i++)
	{
		this->AddPropertyToTree(currentprop.GetSrcProperty(i), proptreenode);
	}
}
void FBXSceneExplorerWindow::AddRawPropertyToTree(FbxProperty currentprop, TreeNode^ treenode)
{
	String^ cproptype = VVVV::FBX::ManagedEnums::FBXType()[currentprop.GetPropertyDataType().GetType()];
	cproptype = cproptype->Substring(4);

	TreeNode^ proptreenode = treenode->Nodes->Add(
		"Property " + cproptype + ": " +
		gcnew String(currentprop.GetName()) + "   ");

	for (int i = 0; i<currentprop.GetSrcObjectCount(); i++)
	{
		AddObjectToTree(currentprop.GetSrcObject(i), proptreenode);
	}
	for (int i = 0; i<currentprop.GetSrcPropertyCount(); i++)
	{
		this->AddRawPropertyToTree(currentprop.GetSrcProperty(i), proptreenode);
	}
}
void FBXSceneExplorerWindow::AddObjectToTree(FbxObject* fbxobj, TreeNode^ treenode)
{
	TreeNode^ currnode = treenode->Nodes->Add(gcnew String(fbxobj->GetTypeName()) + ": " + gcnew String(fbxobj->GetName()) + "   ");
	for (int i = 0; i<fbxobj->GetSrcObjectCount(); i++)
	{
		this->AddObjectToTree(fbxobj->GetSrcObject(i), currnode);
	}
	for (int i = 0; i<fbxobj->GetSrcPropertyCount(); i++)
	{
		this->AddRawPropertyToTree(fbxobj->GetSrcProperty(i), currnode);
	}
}

void FBXSceneExplorerWindow::OnDrawTreeNode(System::Object ^sender, System::Windows::Forms::DrawTreeNodeEventArgs ^e)
{
	System::Drawing::Font^ font = gcnew System::Drawing::Font(this->Font, System::Drawing::FontStyle::Regular);
	System::Drawing::Brush^ brush;

	if (e->Node->Text->Contains(":"))
	{
		int splitat = e->Node->Text->IndexOf(':');
		String^ darktext = e->Node->Text->Remove(splitat + 1);
		String^ whitetext = e->Node->Text->Substring(splitat + 1);

		brush = gcnew System::Drawing::SolidBrush(System::Drawing::Color::FromArgb(255, 160, 160, 160));
		e->Graphics->DrawString(darktext, font, brush, e->Bounds.Left, e->Bounds.Top);

		brush = gcnew System::Drawing::SolidBrush(System::Drawing::Color::White);
		System::Drawing::SizeF s = e->Graphics->MeasureString(darktext, font);
		e->Graphics->DrawString(whitetext, font, brush, e->Bounds.Left + (int)s.Width, e->Bounds.Top);
	}
	else
	{
		brush = gcnew System::Drawing::SolidBrush(System::Drawing::Color::White);
		e->Graphics->DrawString(e->Node->Text, font, brush, e->Bounds.Left, e->Bounds.Top);
	}
}
