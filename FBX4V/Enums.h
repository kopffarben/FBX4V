#pragma once


using namespace System;
using namespace System::Collections::Generic;

namespace VVVV {
	namespace FBX {
		public enum class DestinationCoordSys
		{
			MayaZUp,
			MayaYUp,
			Max,
			MotionBuilder,
			OpenGL,
			DirectX
		};
		public ref class ManagedEnums
		{
		public:
			static array<String^>^ AttributeType()
			{
				return gcnew array<String^>
				{
					"eUnknown",
					"eNull",
					"eMarker",
					"eSkeleton",
					"eMesh",
					"eNurbs",
					"ePatch",
					"eCamera",
					"eCameraStereo",
					"eCameraSwitcher",
					"eLight",
					"eOpticalReference",
					"eOpticalMarker",
					"eNurbsCurve",
					"eTrimNurbsSurface",
					"eBoundary",
					"eNurbsSurface",
					"eShape",
					"eLODGroup",
					"eSubDiv",
					"eCachedEffect",
					"eLine"
				};
			}
			static array<String^>^ DeformerType()
			{
				return gcnew array<String^>{
					"eUnknown",		//!< Unknown deformer type
					"eSkin",		//!< Type FbxSkin
					"eBlendShape",	//!< Type FbxBlendShape
					"eVertexCache"	//!< Type FbxVertexCacheDeformer
				};
			}
			static array<String^>^ ClusterLinkMode()
			{
				return gcnew array<String^>{
					"eNormalize",	//!< Unknown deformer type
					"eAddative",	//!< Type FbxSkin
					"eTotalOne"		//!< Type FbxBlendShape
				};
			}
			static Dictionary<int, String^>^ FBXType()
			{
				Dictionary<int, String^>^ dir = gcnew Dictionary<int, String^>();
				dir->Add(0, "eFbxUndefined");			//!< Unidentified.
				dir->Add(1, "eFbxChar");				//!< 8 bit signed integer.
				dir->Add(2, "eFbxUChar");				//!< 8 bit unsigned integer.
				dir->Add(3, "eFbxShort");				//!< 16 bit signed integer.
				dir->Add(4, "eFbxUShort");				//!< 16 bit unsigned integer.
				dir->Add(5, "eFbxUInt");				//!< 32 bit unsigned integer.
				dir->Add(6, "eFbxLongLong");			//!< 64 bit signed integer.
				dir->Add(7, "eFbxULongLong");			//!< 64 bit unsigned integer.
				dir->Add(8, "eFbxHalfFloat");			//!< 16 bit floating point.
				dir->Add(9, "eFbxBool");				//!< Boolean.
				dir->Add(10, "eFbxInt");				//!< 32 bit signed integer.
				dir->Add(11, "eFbxFloat");				//!< Floating point value.
				dir->Add(12, "eFbxDouble");				//!< Double width floating point value.
				dir->Add(13, "eFbxDouble2");			//!< Vector of two double values.
				dir->Add(14, "eFbxDouble3");			//!< Vector of three double values.
				dir->Add(15, "eFbxDouble4");			//!< Vector of four double values.
				dir->Add(16, "eFbxDouble4x4");			//!< Four vectors of four double values.
				dir->Add(17, "eFbxEnum");	            //!< Enumeration.
				dir->Add(-17, "eFbxEnumM");	            //!< Enumeration allowing duplicated items.
				dir->Add(18, "eFbxString");         	//!< String.
				dir->Add(19, "eFbxTime");				//!< Time value.
				dir->Add(20, "eFbxReference");			//!< Reference to object or property.
				dir->Add(21, "eFbxBlob");				//!< Binary data block type.
				dir->Add(22, "eFbxDistance");			//!< Distance.
				dir->Add(23, "eFbxDateTime");			//!< Date and time.
				dir->Add(24, "eFbxTypeCount");      	//!< Indicates the number of type identifiers constants.
				return dir;
			}
			static array<String^>^ SkeletonType()
			{
				return gcnew array<String^>
				{
					"eRoot",
					"eLimb",
					"eLimbNode",
					"eEffector"
				};
			}
		};
	}
}