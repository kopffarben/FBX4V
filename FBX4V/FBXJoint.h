#pragma once

#include "Stdafx.h"

using namespace VVVV::SkeletonInterfaces;
using namespace VVVV::Utils::VMath;

namespace VVVV {
	namespace FBX {
		public ref class FBXJoint : IJoint
		{
		public:
			FBXJoint(int id, String^ name)
			{
				this->FId = id;
				this->FName = name;
				this->FChildren = gcnew List<IJoint^>();

				this->FBaseTransform = VMath::IdentityMatrix;
				this->FAnimationTransform = VMath::IdentityMatrix;
				this->FConstraints = gcnew List<Vector2D>();
				this->FConstraints->Add(Vector2D::Vector2D(-1.0, 1.0));
				this->FConstraints->Add(Vector2D::Vector2D(-1.0, 1.0));
				this->FConstraints->Add(Vector2D::Vector2D(-1.0, 1.0));
				this->Invalidate();
			}

			FbxNode* AttachedNode;

			property String^ Name
			{
				virtual String^ get() { return this->FName; }
				virtual void set(String^ value) { this->FName = value; }
			}
			property int Id
			{
				virtual int get() { return this->FId; }
				virtual void set(int value) { this->FId = value; }
			}
			property List<Vector2D>^ Constraints
			{
				virtual List<Vector2D>^ get() { return this->FConstraints; }
				virtual void set(List<Vector2D>^ value) { this->FConstraints = value; }
			}
			property List<IJoint^>^ Children
			{
				virtual List<IJoint^>^ get() { return this->FChildren; }
			}

			property Matrix4x4 BaseTransform
			{
				virtual Matrix4x4 get() { return this->FBaseTransform; }
				virtual void set(Matrix4x4 value)
				{
					this->FBaseTransform = value;
					this->Invalidate();
				}
			}
			property Matrix4x4 AnimationTransform
			{
				virtual Matrix4x4 get() { return this->FAnimationTransform; }
				virtual void set(Matrix4x4 value)
				{
					this->FAnimationTransform = value;
					this->Invalidate();
				}
			}
			property IJoint^ Parent
			{
				virtual IJoint^ get() { return this->FParent; }
				virtual void set(IJoint^ value)
				{
					this->FParent = value;
					this->Invalidate();
				}
			}


			property Matrix4x4 CombinedTransform
			{
				virtual Matrix4x4 get()
				{
					this->UpdateValues();
					return this->FCombinedTransform;
				}
			}
			property Vector3D Rotation
			{
				virtual Vector3D get()
				{
					this->UpdateValues();
					return this->FRotation;
				}
			}
			property Vector3D Translation
			{
				virtual Vector3D get()
				{
					this->UpdateValues();
					return this->FTranslation;
				}
			}
			property Vector3D Scale
			{
				virtual Vector3D get()
				{
					this->UpdateValues();
					return this->FScale;
				}
			}

			virtual void CalculateCombinedTransforms()
			{
				this->UpdateValues();
			}
			virtual void AddChild(IJoint^ joint)
			{
				joint->Parent = this;
				this->Children->Add(joint);
			}
			virtual void ClearAll()
			{
				this->Children->Clear();
			}
			virtual IJoint^ DeepCopy()
			{
				FBXJoint^ copy = gcnew FBXJoint(this->Id, this->Name);
				copy->BaseTransform = Matrix4x4::Matrix4x4(this->BaseTransform);
				copy->AnimationTransform = Matrix4x4::Matrix4x4(this->AnimationTransform);

				for each (IJoint^ child in this->Children)
				{
					copy->AddChild(child->DeepCopy());
				}
				for(int i=0; i<3; i++)
				{
					copy->Constraints[i] = this->Constraints[i];
				}

				return copy;
			}

			bool IsInvalid() { return this->Invalid; }
			void Invalidate()
			{
				if(!this->IsInvalid())
				{
					this->Invalid = true;
					for each (IJoint^ j in this->Children)
					{
						safe_cast<FBXJoint^>(j)->Invalidate();
					}
				}
			}

		private:
			String^ FName;
			int FId;
			IJoint^ FParent;
			List<IJoint^>^ FChildren;
			List<Vector2D>^ FConstraints;
			bool Invalid;

			Matrix4x4 FBaseTransform;
			Matrix4x4 FAnimationTransform;
			Matrix4x4 FCombinedTransform;
			Vector3D FTranslation;
			Vector3D FRotation;
			Vector3D FScale;

			void UpdateValues()
			{
				if(this->IsInvalid())
				{
					Matrix4x4Utils::Decompose(this->AnimationTransform, this->FScale, this->FRotation, this->FTranslation);
					if(this->Parent != nullptr)
					{
						this->FCombinedTransform = this->AnimationTransform * this->BaseTransform * this->Parent->CombinedTransform;
					}
					else
					{
						this->FCombinedTransform = this->AnimationTransform * this->BaseTransform;
					}
					this->Invalid = false;
				}
			}
		};
	}
}