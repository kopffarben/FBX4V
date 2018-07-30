#pragma once

#include "Stdafx.h"

using namespace System::IO;
using namespace System::Windows::Forms;
using namespace System::Runtime::InteropServices;

namespace VVVV {
	namespace FBX {
		public ref class FBXSceneExplorerWindow : Form
		{
		public:
			void InitializeComponent();

			TreeView^ FBXNodeTree;
			TreeNode^ SceneTreeNode;
			TreeNode^ ObjectTreeNode;

			void AddNodeToTree(FbxNode* fbxnode, TreeNode^ treenode);
			void AddObjectToTree(FbxObject* fbxobj, TreeNode^ treenode);
			void AddPropertyToTree(FbxProperty currentprop, TreeNode^ treenode);
			void AddRawPropertyToTree(FbxProperty currentprop, TreeNode^ treenode);

		private:
			void OnAfterExpand(System::Object ^sender, System::Windows::Forms::TreeViewEventArgs ^e)
			{
				this->FBXNodeTree->Invalidate();
			}
			void OnDrawTreeNode(System::Object ^sender, System::Windows::Forms::DrawTreeNodeEventArgs ^e);
		};
	}
}