#pragma once

#include "lc_context.h"

enum class lcCursor
{
	Default,
	Brick,
	Light,
	Sunlight,   /*** LPub3D Mod - enable lights ***/
	Arealight,  /*** LPub3D Mod - enable lights ***/
	Spotlight,
	Camera,
	Select,
	SelectAdd,
	SelectRemove,
	Move,
	Rotate,
	RotateX,
	RotateY,
	Delete,
	Paint,
	ColorPicker,
	Zoom,
	ZoomRegion,
	Pan,
	Roll,
	RotateView,
	RotateStep,  /*** LPub3D Mod - Rotate Step ***/
	Count
};

struct lcInputState
{
	int x;
	int y;
	Qt::KeyboardModifiers Modifiers;
};

class lcGLWidget
{
public:
	lcGLWidget()
	{
		mCursor = lcCursor::Default;
		mWidget = nullptr;
		mInputState.x = 0;
		mInputState.y = 0;
		mInputState.Modifiers = Qt::NoModifier;
		mWidth = 1;
		mHeight = 1;
		mContext = new lcContext();
		mDeleteContext = true;
	}

	virtual ~lcGLWidget()
	{
		if (mDeleteContext)
			delete mContext;
	}

	void SetContext(lcContext* Context)
	{
		if (mDeleteContext)
			delete mContext;

		mContext = Context;
		mDeleteContext = false;
	}

	void MakeCurrent();
	void Redraw();
	void SetCursor(lcCursor Cursor);

	virtual void OnDraw() { }
	virtual void OnInitialUpdate() { }
	virtual void OnUpdateCursor() { }
	virtual void OnLeftButtonDown() { }
	virtual void OnLeftButtonUp() { }
	virtual void OnLeftButtonDoubleClick() { }
	virtual void OnMiddleButtonDown() { }
	virtual void OnMiddleButtonUp() { }
	virtual void OnRightButtonDown() { }
	virtual void OnRightButtonUp() { }
	virtual void OnBackButtonDown() { }
	virtual void OnBackButtonUp() { }
	virtual void OnForwardButtonDown() { }
	virtual void OnForwardButtonUp() { }
	virtual void OnMouseMove() { }
	virtual void OnMouseWheel(float Direction) { Q_UNUSED(Direction); }

	lcInputState mInputState;
	int mWidth;
	int mHeight;
	lcCursor mCursor;
	void* mWidget;
	lcContext* mContext;
	bool mDeleteContext;
};
