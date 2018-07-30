
#include "Stdafx.h"
#include "NodeManager.h"

using namespace VVVV::Nodes::FBX;

void FBXFileNode::Evaluate(int SpreadMax)
{
	this->FBusy[0] = this->FBXMainThreadContext->Working;
	if (this->FReload[0])
	{
		bool valid = this->FFBXFile->SliceCount != 0;
		if (valid) valid = System::IO::File::Exists(FFBXFile[0]);
		if (valid)
		{
			if (this->FBXMainThreadContext->Working) return;
			this->Nodepath;
			this->PluginHost->GetNodePath(false, this->Nodepath);
			this->SceneW = gcnew FBXSceneWrapper();
			this->SceneW->FBXThreadContext = this->FBXMainThreadContext;
			this->SceneW->Logger = this->FLogger;
			this->SceneW->FbxFileName = Path::GetFileName(this->FFBXFile[0]);
			this->FBXMainThreadContext->Action = gcnew Action(this, &FBXFileNode::LoadFBX);
			this->FBXMainThreadContext->Set = true;
		}
	}
	if(FImporting[0])
	{
		bool importresult = !this->FBXMainThreadContext->Working;
		if(importresult)
		{
			this->FSCene[0] = SceneW;
			this->FImporting[0] = false;
		}
	}
	if(this->SceneW != nullptr)
	{
		FFBXMessage[0] = this->SceneW->LastMessage;
		FFBXError[0] = this->SceneW->LastError;
		FFBXSuccess[0] = this->SceneW->LastSuccess;
	}
}
void FBXFileNode::CreateNodes(FbxNode* pfbxnode, VVVV::FBX::FBXJoint^ pjoint)
{
	for (int i = 0; i<pfbxnode->GetChildCount(); i++)
	{
		this->JointID++;

		FbxNode* cfbxnode = pfbxnode->GetChild(i);
		String^ cname = pjoint->Name + "/" + gcnew String(cfbxnode->GetName());
		if(this->SceneW->SequentialNodePaths == nullptr) this->SceneW->SequentialNodePaths = gcnew List<String^>();
		this->SceneW->SequentialNodePaths->Add(cname);

		FBXJoint^ cjoint = gcnew FBXJoint(this->JointID, cname);
		cjoint->AttachedNode = cfbxnode;
		pjoint->AddChild(cjoint);

		cjoint->BaseTransform = FBXUtils::AsVVVVMatrix(cfbxnode->EvaluateLocalTransform());

		this->CreateNodes(cfbxnode, cjoint);
	}
}
void FBXFileNode::BakeAnims()
{
	int asc = this->Scene->GetSrcObjectCount(FbxCriteria::ObjectType(FbxAnimStack::ClassId));

	double bakeprog = 0;

	for(int i=0; i<asc; i++)
	{
		FbxAnimStack* curranims = FbxCast<FbxAnimStack>(this->Scene->GetSrcObject(FbxCriteria::ObjectType(FbxAnimStack::ClassId), i));
		this->Scene->SetCurrentAnimationStack(curranims);
		String^ asfilename = FBXUtils::EscapeFileName(gcnew String(curranims->GetName())) + ".fbba";
		MemoryStream^ stream = gcnew MemoryStream();
		FileStream^ filestream = gcnew FileStream(Path::Combine(this->SceneW->FbmFolder, asfilename), FileMode::Create);

		double dur = curranims->LocalStop.Get().GetSecondDouble();
		int nodecount = this->FSkeleton->JointTable->Count;
		int sampcount = Math::Ceiling(dur * this->FBakeFPS[0]) + 1;
		int sampsize = FBX4V_FBBA_SAMPSIZE;
		int channelsize = sampcount * sampsize;
		stream->SetLength((long)(channelsize * nodecount));
		filestream->SetLength((long)(channelsize * nodecount));

		stream->Write(BitConverter::GetBytes(sampsize), 0, 4);
		stream->Write(BitConverter::GetBytes(sampcount), 0, 4);
		stream->Write(BitConverter::GetBytes(this->FBakeFPS[0]), 0, 4);

		int currnode = 0;
		for each (IJoint^ j in this->FSkeleton->JointTable->Values)
		{
			FBXJoint^ fbxj = dynamic_cast<FBXJoint^>(j);
			if (fbxj != nullptr)
			{
				double currprog = 0;
				stream->Position = fbxj->Id * channelsize + FBX4V_FBBA_METASIZE;
				this->SceneW->LastMessage = "Baking " + asfilename + ": " + fbxj->Name;
				for (int k = 0; k<sampcount; k++)
				{
					FbxTime time = FBXUtils::SecondsToTime(currprog);
					Matrix4x4 tr = FBXUtils::AsVVVVMatrix(fbxj->AttachedNode->EvaluateGlobalTransform(time));

					for(int mi = 0; mi<tr.Values->Length; mi++)
					{
						stream->Write(BitConverter::GetBytes((float)(tr.Values[mi])), 0, 4);
					}

					currprog += 1 / (double)(this->FBakeFPS[0]);
				}
			}
			currnode++;
		}
		stream->Position = 0;
		filestream->Position = 0;
		stream->CopyTo(filestream);
		this->SceneW->LastSuccess = "Done baking " + asfilename;
	}
}
void FBXFileNode::FBXMainThread()
{
	ThreadContext^ context = this->FBXMainThreadContext;
	if (context == nullptr)
	{
		// should never happen!
		throw gcnew Exception("Context was not specified for main FBX thread");
	}
	//context->Timer->Start();
	while(true)
	{
		//if (context->Timer->Elapsed.TotalMilliseconds < 5.0)
			//continue;
		//context->Timer->Restart();
		if(context->Quit)
		{
			return;
		}
		if(context->Forms->Count > 0)
		{
			Application::DoEvents();
		}
		/*for each (Form^ form in context->Forms->Values)
		{
			form->Update();
		}*/
		//if(!context->Set) Thread::Sleep(5);
		if(context->Set)
		{
			context->Set = false;
			context->Working = true;
			try
			{
				if (context->Action != nullptr) context->Action->Invoke();
			}
			catch (const std::exception e)
			{
				this->SceneW->LastError = "std::exception:\n" + gcnew String(e.what());
			}
			catch (Exception^ e)
			{
				this->SceneW->LastError = "Managed Exception:\n" + e->Message + "\n" + e->StackTrace;
			}
			context->Working = false;
			context->CallerWaitHandle->Set();
		}
	}
}

void FBXFileNode::LoadFBX()
{
	this->FImporting[0] = true;
	this->SceneW->LastMessage = "init";
	if (this->Scene != nullptr) this->Scene->Destroy();
	if (this->Manager != nullptr) this->Manager->Destroy();
	this->SceneW->LastMessage = "Creating Manager";
	this->Manager = FbxManager::Create();
	this->SceneW->LastSuccess = "Manager Created";

	FbxIOSettings* settings = FbxIOSettings::Create(this->Manager, FBX4V_CHARPOINTER(this->FIOSettings[0]));
	this->Manager->SetIOSettings(settings);

	this->SceneW->LastMessage = "Creating Importer";
	this->Importer = FbxImporter::Create(this->Manager, "MainImporter");
	bool imported = Importer->Initialize(FBX4V_CHARPOINTER(this->FFBXFile[0]), -1, settings);

	if (!imported)
	{
		this->SceneW->LastError = "Importer Error: " + gcnew String(this->Importer->GetStatus().GetErrorString());
		return;
	}
	this->SceneW->LastSuccess = "Importer Created";

	this->SceneW->LastMessage = "Creating and filling Scene";
	this->Scene = FbxScene::Create(this->Manager, "");
	imported = this->Importer->Import(this->Scene, false);
	if (!imported)
	{
		this->SceneW->LastError = "Error during import: " + gcnew String(this->Importer->GetStatus().GetErrorString());
	}
	FbxDocumentInfo* info = Scene->GetDocumentInfo();
	SceneW->DocumentInfo = gcnew FBXDocumentInfo();
	SceneW->DocumentInfo->Author = gcnew String(info->mAuthor.Buffer());
	SceneW->DocumentInfo->Comment = gcnew String(info->mComment.Buffer());
	SceneW->DocumentInfo->EmbeddedUrl = gcnew String(info->EmbeddedUrl.Get().Buffer());
	SceneW->DocumentInfo->Keywords = gcnew String(info->mKeywords.Buffer());
	SceneW->DocumentInfo->LastSavedUrl = gcnew String(info->LastSavedUrl.Get().Buffer());
	SceneW->DocumentInfo->OriginalFileName = gcnew String(info->Original_FileName.Get().Buffer());
	SceneW->DocumentInfo->Revision = gcnew String(info->mRevision.Buffer());
	SceneW->DocumentInfo->Subject = gcnew String(info->mSubject.Buffer());
	SceneW->DocumentInfo->Title = gcnew String(info->mTitle.Buffer());
	SceneW->DocumentInfo->Url = gcnew String(info->Url.Get().Buffer());
	SceneW->DocumentInfo->OriginalInfo = gcnew FBXCoreInfo();
	SceneW->DocumentInfo->OriginalInfo->AppName = gcnew String(info->Original_ApplicationName.Get().Buffer());
	SceneW->DocumentInfo->OriginalInfo->AppVendor = gcnew String(info->Original_ApplicationVendor.Get().Buffer());
	SceneW->DocumentInfo->OriginalInfo->AppVersion = gcnew String(info->Original_ApplicationVersion.Get().Buffer());
	SceneW->DocumentInfo->OriginalInfo->DateTime = gcnew String(info->Original_DateTime_GMT.Get().toString().Buffer());
	SceneW->DocumentInfo->LastSavedInfo = gcnew FBXCoreInfo();
	SceneW->DocumentInfo->LastSavedInfo->AppName = gcnew String(info->LastSaved_ApplicationName.Get().Buffer());
	SceneW->DocumentInfo->LastSavedInfo->AppVendor = gcnew String(info->LastSaved_ApplicationVendor.Get().Buffer());
	SceneW->DocumentInfo->LastSavedInfo->AppVersion = gcnew String(info->LastSaved_ApplicationVersion.Get().Buffer());
	SceneW->DocumentInfo->LastSavedInfo->DateTime = gcnew String(info->LastSaved_DateTime_GMT.Get().toString().Buffer());
	this->SceneW->LastSuccess = "Scene is ready";

	this->SceneW->LastMessage = "Converting coordinate system";
	switch (FDstCoordSys[0])
	{
	case DestinationCoordSys::MayaYUp:
		FbxAxisSystem::MayaYUp.ConvertScene(Scene);
		break;
	case DestinationCoordSys::MayaZUp:
		FbxAxisSystem::MayaZUp.ConvertScene(Scene);
		break;
	case DestinationCoordSys::Max:
		FbxAxisSystem::Max.ConvertScene(Scene);
		break;
	case DestinationCoordSys::MotionBuilder:
		FbxAxisSystem::Motionbuilder.ConvertScene(Scene);
		break;
	case DestinationCoordSys::OpenGL:
		FbxAxisSystem::OpenGL.ConvertScene(Scene);
		break;
	case DestinationCoordSys::DirectX:
		FbxAxisSystem::DirectX.ConvertScene(Scene);
		break;
	default:
		break;
	}
	this->SceneW->LastSuccess = "Converted coordinate system";

	FbxGeometryConverter geomconv(this->Manager);
	if(FRemoveBadPolys[0])
	{
		this->SceneW->LastMessage = "Removing bad polygons";
		FbxArray<FbxNode*>* affectednodes = new FbxArray<FbxNode*>();
		geomconv.RemoveBadPolygonsFromMeshes(this->Scene, affectednodes);
		if (affectednodes->Size() > 0)
		{
			this->SceneW->LastSuccess = "Removed bad polygons at these nodes:";
			for (int i = 0; i < affectednodes->Size(); i++)
			{
				this->SceneW->LastSuccess = gcnew String(affectednodes->GetAt(i)->GetName());
			}
		}
		else
			this->SceneW->LastSuccess = "No polygons were bad";
	}
	/*
	this->SceneW->LastMessage = "Splitting meshes per material";
	if (geomconv.SplitMeshesPerMaterial(this->Scene, true))
		this->SceneW->LastSuccess = "Meshes splitted";
	else
		this->SceneW->LastError = "Splitting meshes are ready but some of them were skipped";
	*/

	this->SceneW->LastMessage = "Triangulating meshes";
	if (geomconv.Triangulate(this->Scene, true, !FNewTriang[0]))
		this->SceneW->LastSuccess = "Triangulated meshes";
	else
		this->SceneW->LastError = "Triangulation is ready but some meshes were skipped";
	geomconv.~FbxGeometryConverter();


	this->SceneW->LastMessage = "Building Skeleton";
	this->FSkeleton->ClearAll();
	this->JointID = 0;

	Matrix4x4 roottr = FBXUtils::AsVVVVMatrix(this->Scene->GetRootNode()->EvaluateGlobalTransform());
	FBXJoint^ rootjoint = gcnew FBXJoint(0, gcnew String(this->Scene->GetRootNode()->GetName()));
	rootjoint->AttachedNode = this->Scene->GetRootNode();
	rootjoint->BaseTransform = roottr;

	this->CreateNodes(this->Scene->GetRootNode(), rootjoint);

	this->FSkeleton->InsertJoint("", rootjoint);
	this->FSkeleton->BuildJointTable();
	this->SceneW->NodeHierarchy = this->FSkeleton;

	this->SceneW->LastSuccess = "Skeleton Built";

	this->SceneW->LastMessage = "Get AnimStacks";
	int asc = this->Scene->GetSrcObjectCount(FbxCriteria::ObjectType(FbxAnimStack::ClassId));
	for (int i = 0; i < asc; i++)
	{
		FbxAnimStack* curranims = FbxCast<FbxAnimStack>(this->Scene->GetSrcObject(FbxCriteria::ObjectType(FbxAnimStack::ClassId), i));
		this->Scene->SetCurrentAnimationStack(curranims);
		SceneW->AnimStackNames->Add(FBXUtils::EscapeFileName(gcnew String(curranims->GetName())));
	}


	if (FBakeAnims[0])
	{
		this->SceneW->BakedAnimations = true;
		this->SceneW->BakedAnimationsAreNew = false;

		this->SceneW->FbxHash = FDstCoordSys[0].ToString() + FBXUtils::GetSHA256FromFile(FFBXFile[0]);
		String^ dirname = Path::GetDirectoryName(this->FFBXFile[0]);
		String^ purefilename = Path::GetFileNameWithoutExtension(this->FFBXFile[0]);
		this->SceneW->FbmFolder = Path::Combine(dirname, purefilename + ".fbm");

		this->BakeIt = !Directory::Exists(this->SceneW->FbmFolder);
		if (BakeIt)
			Directory::CreateDirectory(this->SceneW->FbmFolder);
		String^ hashpath = Path::Combine(this->SceneW->FbmFolder, "hash.sha256");
		if (!this->BakeIt) this->BakeIt = !File::Exists(hashpath);

		if (!this->BakeIt)
		{
			String^ oldhash = File::ReadAllText(hashpath);
			if (!String::Equals(oldhash, this->SceneW->FbxHash)) this->BakeIt = true;
		}

		if (this->BakeIt)
		{
			File::WriteAllText(hashpath, this->SceneW->FbxHash);
			this->FBakingAnims[0] = true;
			this->SceneW->BakedAnimationsAreNew = true;

			this->SceneW->LastMessage = "Baking AnimStacks";
			this->BakeAnims();

			this->SceneW->LastSuccess = "All Done";
			this->FBakingAnims[0] = false;
		}
	}
	else
	{
		this->FBakingAnims[0] = false;
		this->BakeIt = false;
		this->SceneW->BakedAnimations = false;
		this->SceneW->BakedAnimationsAreNew = false;

		this->SceneW->LastSuccess = "All Done";
	}

	System::Guid^ guid = System::Guid::NewGuid();
	this->SceneW->GUID = this->Nodepath + "/Scene/" + guid;
	this->Scene->SetName(FBX4V_CHARPOINTER(this->SceneW->GUID));
	int fmajor = 0, fminor = 0, frev = 0;
	FbxManager::GetFileFormatVersion(fmajor, fminor, frev);

	this->FVersion[0] = fmajor.ToString() + "." + fminor.ToString() + "." + frev.ToString();

	this->SceneW->Object = this->Scene;
	this->SceneW->LastSuccess = "All Done";

	FName[0] = gcnew String(this->Scene->GetName());
	this->Importer->Destroy();
}