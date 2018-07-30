#pragma once

#include "Stdafx.h"
using namespace VVVV::SkeletonInterfaces;
using namespace VVVV::Utils::VMath;
using namespace System::Security::Cryptography;
using namespace System::IO;
using namespace System::Runtime::InteropServices;

namespace VVVV {
	namespace FBX {
		public ref class FBXUtils
		{
		public:
			//Files
			static String^ GetSHA256FromFile(String^ filepath)
			{
				array<unsigned char>^ bytes = File::ReadAllBytes(filepath);
				SHA256Managed^ sham = gcnew SHA256Managed();
				array<unsigned char>^ hash = sham->ComputeHash(bytes);
				String^ hashString = System::String::Empty;
				for each(unsigned char x in hash)
				{
					hashString += System::String::Format("{0:x2}", x);
				}
				return hashString;
			}
			static String^ EscapeFileName(String^ filepath)
			{
				array<wchar_t>^ invalids = Path::GetInvalidFileNameChars();
				return String::Join("_", filepath->Split(invalids, StringSplitOptions::RemoveEmptyEntries))->TrimEnd('.');
			}
			static int ReadInt(Stream^ input)
			{
				array<unsigned char>^ tmp = gcnew array<unsigned char>(4);
				int res = input->Read(tmp, 0, 4);
				//if (res < 4) throw gcnew Exception("Not enough bytes to interpret");
				return BitConverter::ToInt32(tmp, 0);
			}
			static float ReadFloat(Stream^ input)
			{
				array<unsigned char>^ tmp = gcnew array<unsigned char>(4);
				int res = input->Read(tmp, 0, 4);
				//if (res < 4) throw gcnew Exception("Not enough bytes to interpret");
				return BitConverter::ToSingle(tmp, 0);
			}

			//Matrices
			static Matrix4x4 RotateQuaternion(Vector4D q)
			{
				Matrix4x4 ma = Matrix4x4::Matrix4x4(
					q.w, q.z, -q.y, q.x,
					-q.z, q.w, q.x, q.y,
					q.y, -q.x, q.w, q.z,
					-q.x, -q.y, -q.z, q.w
				);
				Matrix4x4 mb = Matrix4x4::Matrix4x4(
					q.w, q.z, -q.y, -q.x,
					-q.z, q.w, q.x, -q.y,
					q.y, -q.x, q.w, -q.z,
					q.x, q.y, q.z, q.w
				);
				return ma * mb;
			}
			static Matrix4x4 AsVVVVMatrix(FbxAMatrix& fm)
			{
				Matrix4x4 result = Matrix4x4::Matrix4x4(
					fm[0][0], fm[0][1], fm[0][2], fm[0][3],
					fm[1][0], fm[1][1], fm[1][2], fm[1][3],
					fm[2][0], fm[2][1], fm[2][2], fm[2][3],
					fm[3][0], fm[3][1], fm[3][2], fm[3][3]);
				return result;
			}
			static Matrix4x4 AsVVVVMatrix(FbxMatrix& fm)
			{
				Matrix4x4 result = Matrix4x4::Matrix4x4(
					fm[0][0], fm[0][1], fm[0][2], fm[0][3],
					fm[1][0], fm[1][1], fm[1][2], fm[1][3],
					fm[2][0], fm[2][1], fm[2][2], fm[2][3],
					fm[3][0], fm[3][1], fm[3][2], fm[3][3]);
				return result;
			}
			static double Dot(Vector4D a, Vector4D b)
			{
				return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
			}
			static Vector4D Slerp(Vector4D a, Vector4D b, double x)
			{
				if (x <= 0.0)
					return a;

				if (x >= 1.0)
					return b;

				double w1, w2;

				double cosTheta = a | b; // | is dot product
				double theta = Math::Acos(cosTheta);
				double sinTheta = Math::Sin(theta);
				Vector4D c = Vector4D::Vector4D(b);

				if(cosTheta < 0.0)
				{
					cosTheta = -cosTheta;
					c = Vector4D::Vector4D(-c.x, -c.y, -c.z, -c.w);
				}

				if (sinTheta > 0.0001)
				{
					sinTheta = 1 / sinTheta;
					w1 = Math::Sin((1 - x) * theta) * sinTheta;
					w2 = Math::Sin(x * theta) * sinTheta;
				}
				else
				{
					w1 = 1 - x;
					w2 = x;
				}

				return Vector4D::Vector4D(
					a.x*w1 + c.x*w2,
					a.y*w1 + c.y*w2,
					a.z*w1 + c.z*w2,
					a.w*w1 + c.w*w2
					);
			}
			static Matrix4x4 ReadSample(Stream^ input, int samp, double blend, int offs, int sampsize, int length)
			{
				Matrix4x4 cm, nm;

				input->Position = samp * sampsize + offs;
				cm = Matrix4x4::Matrix4x4(
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input)
				);
				input->Position = ((samp + 1) % length) * sampsize + offs;
				nm = Matrix4x4::Matrix4x4(
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input)
				);

				Vector3D cpos; Vector4D crot; Vector3D cscale;
				Vector3D npos; Vector4D nrot; Vector3D nscale;

				Matrix4x4Utils::Decompose(cm, cscale, crot, cpos);
				Matrix4x4Utils::Decompose(nm, nscale, nrot, npos);

				return
					VMath::Scale(VMath::Lerp(cscale, nscale, blend)) *
					FBXUtils::RotateQuaternion(FBXUtils::Slerp(crot, nrot, blend)) *
					VMath::Translate(VMath::Lerp(cpos, npos, blend));
			}
			static Matrix4x4 ReadSampleLinear(Stream^ input, int samp, double blend, int offs, int sampsize, int length)
			{
				Matrix4x4 cm, nm;
				input->Position = samp * sampsize + offs;
				cm = Matrix4x4::Matrix4x4(
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input)
				);
				input->Position = ((samp + 1) % length) * sampsize + offs;
				nm = Matrix4x4::Matrix4x4(
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input),
					FBXUtils::ReadFloat(input)
				);
				return Matrix4x4::Matrix4x4(
					VMath::Lerp(cm.m11, nm.m11, blend),
					VMath::Lerp(cm.m12, nm.m12, blend),
					VMath::Lerp(cm.m13, nm.m13, blend),
					VMath::Lerp(cm.m14, nm.m14, blend),
					VMath::Lerp(cm.m21, nm.m21, blend),
					VMath::Lerp(cm.m22, nm.m22, blend),
					VMath::Lerp(cm.m23, nm.m23, blend),
					VMath::Lerp(cm.m24, nm.m24, blend),
					VMath::Lerp(cm.m31, nm.m31, blend),
					VMath::Lerp(cm.m32, nm.m32, blend),
					VMath::Lerp(cm.m33, nm.m33, blend),
					VMath::Lerp(cm.m34, nm.m34, blend),
					VMath::Lerp(cm.m41, nm.m41, blend),
					VMath::Lerp(cm.m42, nm.m42, blend),
					VMath::Lerp(cm.m43, nm.m43, blend),
					VMath::Lerp(cm.m44, nm.m44, blend)
				);
			}

			//Time
			static FbxTime SecondsToTime(double t, bool relative, FbxAnimStack* animstack)
			{
				FbxTime time = FbxTime::FbxTime();
				if (relative)
				{
					double dur = animstack->LocalStop.Get().GetSecondDouble();
					time.SetSecondDouble(t * dur);
				}
				else time.SetSecondDouble(t);

				return time;
			}
			static FbxTime SecondsToTime(double t)
			{
				FbxTime time = FbxTime::FbxTime();
				time.SetSecondDouble(t);
				return time;
			}

			//Object path
			static String^ GetNodePath(FbxNode* node)
			{
				String^ np = "";
				FBXUtils::GetNodePath(node, np);
				return np->Trim('/');
			}
			static void GetNodePath(FbxNode* node, String^% path)
			{
				path = gcnew String(node->GetName()) + "/" + path;
				FbxNode* parent = node->GetParent();
				if(parent != NULL)
				{
					FBXUtils::GetNodePath(parent, path);
				}
			}
			static FbxNode* GetNodeFromPath(FbxScene* scene, String^ path)
			{
				String^ cpath = path->Trim('/');
				array<String^>^ splitted = cpath->Split('/');
				FbxNode* parent = scene->GetRootNode();
				if (path == "RootNode") return parent;
				FbxNode* curr;
				for(int i=0; i<splitted->Length; i++)
				{
					if (splitted[i] == "RootNode") continue;
					curr = parent->FindChild(FBX4V_CHARPOINTER(splitted[i]), false, false);
					if (curr == NULL)
					{
						throw gcnew Exception("Referenced node might not exist.");
					}
					parent = curr;
				}
				return curr;
			}
			
			static FbxNodeAttribute* GetAttributeFromPath(FbxScene* scene, String^ path)
			{
				String^ cpath = path;
				try
				{
					cpath = path->Trim('\\');
				}
				catch (const std::exception e)
				{
					cpath = path;
				}
				catch (Exception^ e)
				{
					cpath = path;
				}
				array<String^>^ splitted = cpath->Split('\\');
				if (splitted->Length != 2) return NULL;
				FbxNode* pnode = GetNodeFromPath(scene, splitted[0]);
				FbxNodeAttribute* attr = pnode->GetNodeAttributeByIndex(int::Parse(splitted[1]));
				if (attr == NULL) return NULL;
				return attr;
			}
		};
	}
}