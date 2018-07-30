#pragma once

#include "Stdafx.h"
using namespace VVVV::SkeletonInterfaces;
using namespace VVVV::Core::Logging;
using namespace System::Windows::Forms;
using namespace System::Threading;
using namespace System::Threading::Tasks;
using namespace System::Diagnostics;

namespace VVVV {
	namespace FBX {
		public ref class FBXCoreInfo
		{
		public:
			FBXCoreInfo()
			{
				
			}
			~FBXCoreInfo()
			{

			}

			String^ DateTime;
			String^ AppVendor;
			String^ AppName;
			String^ AppVersion;
		};

		public ref class FBXDocumentInfo
		{
		public:
			FBXDocumentInfo()
			{

			}
			~FBXDocumentInfo()
			{

			}

			String^ OriginalFileName;
			FBXCoreInfo^ OriginalInfo;
			String^ Url;
			String^ EmbeddedUrl;
			String^ LastSavedUrl;
			FBXCoreInfo^ LastSavedInfo;
			String^ Title;
			String^ Subject;
			String^ Author;
			String^ Keywords;
			String^ Revision;
			String^ Comment;
		};

		public ref class ThreadContext
		{
		public:
			ThreadContext()
			{
				this->FBXWaitHandle = gcnew EventWaitHandle(false, EventResetMode::AutoReset);
				this->CallerWaitHandle = gcnew EventWaitHandle(false, EventResetMode::AutoReset);
				this->Forms = gcnew Dictionary<String^, Form^>();
				this->Timer = gcnew Stopwatch();
				Timer->Start();
				Quit = false;
			}
			Dictionary<String^, Form^>^ Forms;
			Action^ Action;
			EventWaitHandle^ FBXWaitHandle;
			EventWaitHandle^ CallerWaitHandle;
			System::Object^ ActionInputObject;
			Stopwatch^ Timer;
			bool Set;
			bool Working;
			bool Quit;
			Task^ Thread;
		};

		public ref class FBXSceneWrapper
		{
		public:
			FBXSceneWrapper()
			{
				this->SequentialNodePaths = gcnew List<String^>();
				this->AnimStackNames = gcnew List<String^>();
			}
			FBXSceneWrapper(FbxScene* o)
			{
				this->Object = o;
				this->SequentialNodePaths = gcnew List<String^>();
				this->AnimStackNames = gcnew List<String^>();
			}
			~FBXSceneWrapper() { }
			!FBXSceneWrapper() { }

			ThreadContext^ FBXThreadContext;

			FbxScene* Object;
			Skeleton^ NodeHierarchy;
			String^ GUID;
			String^ FbmFolder;
			String^ FbxFileName;
			String^ FbxHash;
			ILogger^ Logger;
			List<String^>^ SequentialNodePaths;
			bool BakedAnimations;
			bool BakedAnimationsAreNew;
			List<String^>^ AnimStackNames;
			FBXDocumentInfo^ DocumentInfo;

			property String^ LastError
			{
				String^ get()
				{
					return this->_LastError;
				}
				void set(String^ value)
				{
					this->_LastError = value;
					this->Logger->Log(VVVV::Core::Logging::LogType::Error, "FBX E: " + value + " (from " + this->FbxFileName + ")");
				}
			}
			property String^ LastMessage
			{
				String^ get()
				{
					return this->_LastMessage;
				}
				void set(String^ value)
				{
					this->_LastMessage = value;
					this->Logger->Log(VVVV::Core::Logging::LogType::Message, "FBX M: " + value + " (from " + this->FbxFileName + ")");
				}
			}
			property String^ LastSuccess
			{
				String^ get()
				{
					return this->_LastSuccess;
				}
				void set(String^ value)
				{
					this->_LastSuccess = value;
					this->Logger->Log(VVVV::Core::Logging::LogType::Message, "FBX S: " + value + " (from " + this->FbxFileName + ")");
				}
			}
		private:
			String^ _LastError;
			String^ _LastMessage;
			String^ _LastSuccess;
		};

		public ref class FBXNodeWrapper
		{
		public:
			FBXNodeWrapper(FBXSceneWrapper^ scene, FbxNode* node)
			{
				this->_Scene = scene;
				this->_Object = node;
			}
			property FBXSceneWrapper^ Scene
			{
				FBXSceneWrapper^ get()
				{
					return this->_Scene;
				}
			}
			property FbxNode* Object
			{
				FbxNode* get()
				{
					return this->_Object;
				}
			}
		private:
			FBXSceneWrapper^ _Scene;
			FbxNode* _Object;
		};
		public ref class FBXBlendShapeWrapper
		{
		public:
			FBXBlendShapeWrapper(FBXSceneWrapper^ scene, FbxBlendShape* o)
			{
				this->_Scene = scene;
				this->_Object = o;
			}
			property FBXSceneWrapper^ Scene
			{
				FBXSceneWrapper^ get()
				{
					return this->_Scene;
				}
			}
			property FbxBlendShape* Object
			{
				FbxBlendShape* get()
				{
					return this->_Object;
				}
			}
		private:
			FBXSceneWrapper^ _Scene;
			FbxBlendShape* _Object;
		};
	}
}