#include "BsMainEditorWindow.h"
#include "BsDockManager.h"
#include "BsCamera.h"
#include "CmSceneObject.h"
#include "CmRenderTexture.h"

// DEBUG ONLY
#include "CmTestTextSprite.h"
#include "CmDebugCamera.h"

using namespace CamelotFramework;
using namespace BansheeEngine;

namespace BansheeEditor
{
	MainEditorWindow::MainEditorWindow(CM::RenderWindowPtr renderWindow)
		:EditorWindowBase(renderWindow), mDockManager(cm_new<DockManager>(mGUI.get()))
	{
		// DEBUG ONLY

		HSceneObject sceneCameraGO = SceneObject::create("SceneCamera");
		HCamera sceneCamera = sceneCameraGO->addComponent<Camera>();

		RenderTexturePtr sceneRenderTarget = RenderTexture::create(TEX_TYPE_2D, 800, 600);

		sceneCamera->initialize(sceneRenderTarget, 0.0f, 0.0f, 1.0f, 1.0f, 0);
		sceneCameraGO->setPosition(Vector3(0,50,1240));
		sceneCameraGO->lookAt(Vector3(0,50,-300));
		sceneCamera->setNearClipDistance(5);
		sceneCamera->setAspectRatio(800.0f / 600.0f);

		GameObjectHandle<DebugCamera> debugCamera = sceneCameraGO->addComponent<DebugCamera>();

		GameObjectHandle<TestTextSprite> textSprite = mSceneObject->addComponent<TestTextSprite>();
		textSprite->initialize(mCamera->getViewport().get(), renderWindow.get());

		textSprite->init(mCamera, "Testing in a new row, does this work?", sceneRenderTarget);
	}

	MainEditorWindow::~MainEditorWindow()
	{
		cm_delete(mDockManager);
	}

	void MainEditorWindow::movedOrResized()
	{
		EditorWindowBase::movedOrResized();

		UINT32 widgetWidth = (UINT32)std::max(0, (INT32)getWidth() - 2);
		UINT32 widgetHeight = (UINT32)std::max(0, (INT32)getHeight() - 2);

		mDockManager->setArea(1, 1, widgetWidth, widgetHeight);
	}
}