#pragma once

#include "CmPrerequisites.h"
#include "CmMatrix4.h"
#include "CmVector3.h"
#include "CmQuaternion.h"
#include "CmRTTIType.h"
#include "CmSceneManager.h"
#include "CmGameObject.h"

#include "boost/static_assert.hpp"

namespace CamelotEngine
{
	class CM_EXPORT SceneObject : public GameObject
	{
		friend class SceneManager;
	public:
		static HSceneObject create(const String& name);
		void destroy();

	private:
		HSceneObject mThisHandle;

		SceneObject(const String& name);
		~SceneObject();

		static HSceneObject createInternal(const String& name);
		void destroyInternal();

		/************************************************************************/
		/* 								Transform	                     		*/
		/************************************************************************/
	public:
		void setPosition(const Vector3& position);
		const Vector3& getPosition() const { return mPosition; }
		const Vector3& getWorldPosition() const;

		void setRotation(const Quaternion& rotation);
		const Quaternion& getRotation() const { return mRotation; }
		const Quaternion& getWorldRotation() const;

		void setScale(const Vector3& scale);
		const Vector3& getScale() const { return mScale; }
		const Vector3& getWorldScale() const;

		void lookAt(const Vector3& location, const Vector3& up = Vector3::UNIT_Y);

		const Matrix4& getWorldTfrm() const;
		const Matrix4& getLocalTfrm() const;

		/** Moves the object's position by the vector offset provided along world axes.
        */
        void move(const Vector3& vec);

        /** Moves the object's position by the vector offset provided along it's own axes (relative to orientation).
        */
        void moveRelative(const Vector3& vec);

		/**
		 * @brief	Gets the Z (forward) axis of the object, in world space.
		 *
		 * @return	Forward axis of the object.
		 */
		Vector3 getForward() const { return getWorldRotation() * Vector3::NEGATIVE_UNIT_Z; }

		/**
		 * @brief	Gets the Y (up) axis of the object, in world space.
		 *
		 * @return	Up axis of the object.
		 */
		Vector3 getUp() const { return getWorldRotation() * Vector3::UNIT_Y; }

		/**
		 * @brief	Gets the X (right) axis of the object, in world space.
		 *
		 * @return	Right axis of the object.
		 */
		Vector3 getRight() const { return getWorldRotation() * Vector3::UNIT_X; }

		/**
		 * @brief	Rotates the game object so it's forward axis faces the provided
		 * 			direction.
		 * 			
		 * @note	Local forward axis is considered to be negative Z.
		 *
		 * @param	forwardDir	The forward direction to face, in world space.
		 */
		void setForward(const Vector3& forwardDir);

		/** Rotate the object around an arbitrary axis.
        */
        void rotate(const Vector3& axis, const Radian& angle);

        /** Rotate the object around an arbitrary axis using a Quaternion.
        */
        void rotate(const Quaternion& q);

		/**
		 * @brief	Rotates around local Z axis.
		 *
		 * @param	angle	Angle to rotate by.
		 */
		void roll(const Radian& angle);

		/**
		 * @brief	Rotates around Y axis.
		 *
		 * @param	angle	Angle to rotate by.
		 */
		void yaw(const Radian& angle);

		/**
		 * @brief	Rotates around X axis
		 *
		 * @param	angle	Angle to rotate by.
		 */
		void pitch(const Radian& angle);

	private:
		String mName;

		Vector3 mPosition;
		Quaternion mRotation;
		Vector3 mScale;

		mutable Vector3 mWorldPosition;
		mutable Quaternion mWorldRotation;
		mutable Vector3 mWorldScale;

		mutable Matrix4 mCachedLocalTfrm;
		mutable bool mIsCachedLocalTfrmUpToDate;

		mutable Matrix4 mCachedWorldTfrm;
		mutable bool mIsCachedWorldTfrmUpToDate;

		Matrix4 mCustomWorldTfrm; // TODO
		bool mIsCustomTfrmModeActive; // TODO

		void markTfrmDirty() const;
		void updateLocalTfrm() const;
		void updateWorldTfrm() const;

		/************************************************************************/
		/* 								Hierarchy	                     		*/
		/************************************************************************/
	public:
		/**
		 * @brief	Changes the parent of this object. Also removes the object from the current parent,
		 * 			and assigns it to the new parent.
		 *
		 * @param [in]	parent	New parent.
		 */
		void setParent(const HSceneObject& parent);

		/**
		 * @brief	Gets the parent of this object.
		 *
		 * @return	Parent object, or nullptr if this SceneObject is at root level.
		 */
		HSceneObject getParent() const { return mParent; }

		/**
		 * @brief	Gets a child of this item.
		 *
		 * @param	idx	The zero based index of the child.
		 *
		 * @return	SceneObject of the child.
		 * 			
		 * @throws ERR_INVALIDPARAMS If the index is out of range.
		 */
		HSceneObject getChild(unsigned int idx) const;

		/**
		 * @brief	Find the index of the specified child. Don't persist this value as
		 * 			it may change whenever you add/remove children.
		 *
		 * @param	child	The child to look for.
		 *
		 * @return	The zero-based index of the found child, or -1 if no match was found.
		 */
		int indexOfChild(const HSceneObject& child) const;

		/**
		 * @brief	Gets the number of all child GameObjects.
		 */
		UINT32 getNumChildren() const { return (UINT32)mChildren.size(); }

	private:
		HSceneObject mParent;
		vector<HSceneObject>::type mChildren;

		/**
		 * @brief	Adds a child to the child array. This method doesn't check for null or duplicate values.
		 *
		 * @param [in]	object	New child.
		 */
		void addChild(const HSceneObject& object);
		
		/**
		 * @brief	Removes the child from the object. 
		 *
		 * @param [in]	object	Child to remove.
		 * 					
		 * @throws INTERNAL_ERROR If the provided child isn't a child of the current object.
		 */
		void removeChild(const HSceneObject& object);

		/************************************************************************/
		/* 								Component	                     		*/
		/************************************************************************/
	public:
		template <typename T>
		GameObjectHandle<T> addComponent()
		{
			BOOST_STATIC_ASSERT_MSG((boost::is_base_of<CamelotEngine::Component, T>::value), 
				"Specified type is not a valid Component.");

			GameObjectHandle<T> newComponent = GameObjectHandle<T>::_create(CM_NEW(T, PoolAlloc) T(mThisHandle),
				&MemAllocDeleter<GameObject, PoolAlloc>::deleter);
			mComponents.push_back(newComponent);

			gSceneManager().notifyComponentAdded(newComponent);

			return newComponent;
		}

		/**
		 * @brief	Searches for a component with the specific type and returns the first one
		 * 			it finds. 
		 * 			
		 * @note	Don't call this too often as it is relatively slow. It is more efficient 
		 * 			to call it once and store the result for further use.
		 *
		 * @tparam	typename T	Type of the component.
		 *
		 * @return	Component if found, nullptr otherwise.
		 */
		template <typename T>
		GameObjectHandle<T> getComponent()
		{
			BOOST_STATIC_ASSERT_MSG((boost::is_base_of<CamelotEngine::Component, T>::value), 
				"Specified type is not a valid Component.");

			return static_object_cast<T>(getComponent(T::getRTTIStatic()->getRTTIId()));
		}

		/**
		 * @brief	Searches for a component with the specified type id and returns the first one it
		 * 			finds.
		 * 			
		 * 			@note	Don't call this too often as it is relatively slow. It is more efficient to
		 * 			call it once and store the result for further use.
		 *
		 * @param	typeId	Identifier for the type.
		 *
		 * @return	Component if found, nullptr otherwise.
		 */
		HComponent getComponent(UINT32 typeId) const;

		/**
		 * @brief	Removes the component from this SceneObject, and deallocates it.
		 *
		 * @param [in]	component	The component to destroy.
		 */
		void destroyComponent(const HComponent& component);

		/**
		 * @brief	Returns all components on this SceneObject.
		 */
		vector<HComponent>::type& getComponents() { return mComponents; }

	private:
		vector<HComponent>::type mComponents;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SceneObjectRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const;
	};
}