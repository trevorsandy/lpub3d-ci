#pragma once

#include "lc_commands.h"
#include "lc_math.h"

enum class lcDragState
{
	None,
	Piece,
	Color
};

enum class lcCursor
{
	First,
	Hidden = First,
	Default,
	Brick,
	Light,
	Sunlight,     /*** LPub3D Mod - enable lights ***/
	Arealight,    /*** LPub3D Mod - enable lights ***/
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
	RotateStep,    /*** LPub3D Mod - Rotate Step ***/
	Count
};

enum class lcTrackButton
{
	None,
	Left,
	Middle,
	Right
};

enum class lcTrackTool
{
	None,
	Insert,
	PointLight,
	SunLight,      /*** LPub3D Mod - enable lights ***/
	AreaLight,     /*** LPub3D Mod - enable lights ***/
	SpotLight,
	Camera,
	Select,
	MoveX,
	MoveY,
	MoveZ,
	MoveXY,
	MoveXZ,
	MoveYZ,
	MoveXYZ,
	RotateX,
	RotateY,
	RotateZ,
	RotateXY,
	RotateXYZ,
	ScalePlus,
	ScaleMinus,
	Eraser,
	Paint,
	ColorPicker,
	Zoom,
	Pan,
	OrbitX,
	OrbitY,
	OrbitXY,
	Roll,
	ZoomRegion,
	RotateStep,    /*** LPub3D Mod - Rotate Step ***/
	Count
};

enum class lcViewType
{
	View,
	Preview,
	Minifig,
	Count
};

class lcGLWidget : public QObject
{
	Q_OBJECT

public:
	lcGLWidget(lcViewType ViewType, lcModel* Model);
	virtual ~lcGLWidget();

	lcGLWidget(const lcGLWidget&) = delete;
	lcGLWidget& operator=(const lcGLWidget&) = delete;

	static void UpdateProjectViews(const Project* Project);
	static void UpdateAllViews();

	lcModel* GetActiveModel() const;

	lcViewType GetViewType() const
	{
		return mViewType;
	}

	lcCamera* GetCamera() const
	{
		return mCamera;
	}

	bool IsLastFocused() const
	{
		return mLastFocusedView == this;
	}

	bool IsTracking() const
	{
		return mTrackButton != lcTrackButton::None;
	}

	int GetWidth() const
	{
		return mWidth;
	}

	int GetHeight() const
	{
		return mHeight;
	}

	void SetSize(int Width, int Height)
	{
		mWidth = Width;
		mHeight = Height;
	}

	QGLWidget* GetWidget() const
	{
		return mWidget;
	}

	void SetWidget(QGLWidget* Widget)
	{
		mWidget = Widget;
	}

	int GetMouseX() const
	{
		return mMouseX;
	}

	int GetMouseY() const
	{
		return mMouseY;
	}

/*** LPub3D Mod - Rotate step angles ***/
	lcTrackTool GetTrackTool() const
	{
		return mTrackTool;
	}
/*** LPub3D Mod end ***/
/*** LPub3D Mod - Camera Globe ***/
	void SetCameraGlobe(float Latitude, float Longitude, float Distance, lcVector3 &Target, bool ApplyZoomExtents = false);
/*** LPub3D Mod end ***/

	void SetFocus(bool Focus);
	void SetMousePosition(int MouseX, int MouseY);
	void SetMouseModifiers(Qt::KeyboardModifiers MouseModifiers);
	void SetContext(lcContext* Context);
	void MakeCurrent();
	void Redraw();
	void UpdateCursor();

	lcVector3 ProjectPoint(const lcVector3& Point) const;
	lcVector3 UnprojectPoint(const lcVector3& Point) const;
	void UnprojectPoints(lcVector3* Points, int NumPoints) const;
	lcMatrix44 GetProjectionMatrix() const;

	void ZoomExtents();

	void SetViewpoint(lcViewpoint Viewpoint);
	void SetViewpoint(const lcVector3& Position);
	void SetViewpoint(const lcVector3& Position, const lcVector3& Target, const lcVector3& Up);
	void SetCameraAngles(float Latitude, float Longitude);
	void SetDefaultCamera();
	void SetCamera(lcCamera* Camera, bool ForceCopy);
	void SetCamera(const QString& CameraName);
	void SetCameraIndex(int Index);

	void DrawBackground() const;
	void DrawViewport() const;
	void DrawAxes() const;

	virtual void OnDraw() { }
	virtual void OnInitialUpdate() { }
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
	void OnMouseWheel(float Direction);
	virtual void BeginDrag(lcDragState DragState) { Q_UNUSED(DragState); }
	virtual void EndDrag(bool Accept) { Q_UNUSED(Accept); }

	lcContext* mContext = nullptr;

signals:
	void FocusReceived();
	void CameraChanged();

protected:
	lcCursor GetCursor() const;
	void SetCursor(lcCursor Cursor);
	lcTool GetCurrentTool() const;
	void StartTracking(lcTrackButton TrackButton);
	lcVector3 GetCameraLightInsertPosition() const;

	int mMouseX = 0;
	int mMouseY = 0;
	int mMouseDownX = 0;
	int mMouseDownY = 0;
	Qt::KeyboardModifiers mMouseModifiers = Qt::NoModifier;

	bool mTrackUpdated = false;
	lcTrackTool mTrackTool = lcTrackTool::None;
	lcTrackButton mTrackButton = lcTrackButton::None;
	lcCursor mCursor = lcCursor::Default;

	std::unique_ptr<lcScene> mScene;
	std::unique_ptr<lcViewSphere> mViewSphere;

	lcModel* mModel = nullptr;
	lcPiece* mActiveSubmodelInstance = nullptr;
	lcMatrix44 mActiveSubmodelTransform;

	lcCamera* mCamera = nullptr;

	QGLWidget* mWidget = nullptr;
	int mWidth = 1;
	int mHeight = 1;
	bool mDeleteContext = true;
	lcViewType mViewType;

	static lcGLWidget* mLastFocusedView;
	static std::vector<lcGLWidget*> mViews;
};
