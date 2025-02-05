#pragma once

/*** LPub3D Mod - libLC conversion ***/
#include <QMainWindow>
/*** LPub3D Mod end ***/
/*** LPub3D Mod - Options moved to application.h ***/
//#include "lc_math.h"
#include "application.h"
/*** LPub3D Mod end ***/

class Project;
class lcPiecesLibrary;
enum class lcViewSphereLocation;

enum class lcShadingMode
{
	Wireframe,
	Flat,
	DefaultLights,
	Full
};

enum class lcColorTheme
{
	Dark,
	System
};

enum class lcAxisIconLocation
{
	TopLeft,
	TopRight,
	BottomLeft,
	BottomRight
};

/*** LPub3D Mod - preview widget for LPub3D ***/
enum class lcPreviewLocation
{
	TopLeft,
	TopRight,
	BottomLeft,
	BottomRight
};

enum class lcPreviewPosition
{
	Dockable,
	Floating,
	Viewport	 // not implemented
};
/*** LPub3D Mod end ***/

class lcPreferences
{
public:
	void LoadDefaults();
	void SaveDefaults();
	void SetInterfaceColors(lcColorTheme ColorTheme);

	int mMouseSensitivity;
	lcShadingMode mShadingMode;
	bool mBackgroundGradient;
	quint32 mBackgroundSolidColor;
	quint32 mBackgroundGradientColorTop;
	quint32 mBackgroundGradientColorBottom;
	bool mDrawAxes;
	lcAxisIconLocation mAxisIconLocation;
	quint32 mAxesColor;
	quint32 mTextColor;
	quint32 mMarqueeBorderColor;
	quint32 mMarqueeFillColor;
	quint32 mOverlayColor;
	quint32 mActiveViewColor;
	quint32 mInactiveViewColor;
	bool mDrawEdgeLines;
	bool mDrawConditionalLines;
	float mLineWidth;
	bool mAllowLOD;
	float mMeshLODDistance;
	bool mFadeSteps;
	quint32 mFadeStepsColor;
	bool mHighlightNewParts;
	quint32 mHighlightNewPartsColor;
	bool mGridEnabled = true;
	bool mDrawGridStuds;
	quint32 mGridStudColor;
	bool mDrawGridLines;
	int mGridLineSpacing;
	quint32 mGridLineColor;
	bool mDrawGridOrigin;
	bool mFixedAxes;
	bool mViewSphereEnabled;
	lcViewSphereLocation mViewSphereLocation;
	int mViewSphereSize;
	quint32 mViewSphereColor;
	quint32 mViewSphereTextColor;
	quint32 mViewSphereHighlightColor;
	bool mAutoLoadMostRecent;
	bool mRestoreTabLayout;
	lcColorTheme mColorTheme;
	quint32 mObjectSelectedColor;
	quint32 mObjectFocusedColor;
	quint32 mCameraColor;
	quint32 mLightColor;
	quint32 mControlPointColor;
	quint32 mControlPointFocusedColor;


	int mPreviewViewSphereEnabled;
	int mPreviewViewSphereSize;
	lcViewSphereLocation mPreviewViewSphereLocation;
	int mDrawPreviewAxis;

	bool mStudCylinderColorEnabled;
	quint32 mStudCylinderColor;
	bool mPartEdgeColorEnabled;
	quint32 mPartEdgeColor;
	bool mBlackEdgeColorEnabled;
	quint32 mBlackEdgeColor;
	bool mDarkEdgeColorEnabled;
	quint32 mDarkEdgeColor;
	float mPartEdgeContrast;
	float mPartColorValueLDIndex;
	bool  mAutomateEdgeColor;

/*** LPub3D Mod - Build mod object selected colour ***/
	quint32 mBMObjectSelectedColor;
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Zoom extents ***/
	int mZoomExtents;
/*** LPub3D Mod - ***/

/*** LPub3D Mod - parts load order ***/
	bool mPreferOfficialParts;
/*** LPub3D Mod - ***/

/*** LPub3D Mod - preview widget for LPub3D ***/
	lcPreviewLocation mPreviewLocation;
	lcPreviewPosition mPreviewPosition;
	int mPreviewSize;
	int mPreviewEnabled;
	QString mPreviewLoadPath;
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Update Default Camera ***/
	bool mDefaultCameraProperties;
	float mDDF;
	float mCDP;
	float mCFoV;
	float mCNear;
	float mCFar;
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Native Renderer settings ***/
	int mNativeViewpoint;
	int mNativeProjection;
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Timeline part icons ***/
	bool mViewPieceIcons;
/*** LPub3D Mod end ***/

/*** LPub3D Mod - lpub fade highlight ***/
	bool mLPubFadeHighlight;
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Selected Parts ***/
	bool mBuildModificationEnabled;
/*** LPub3D Mod end ***/

/*** LPub3D Mod - line width max granularity ***/
	float mLineWidthMaxGranularity;
/*** LPub3D Mod - ***/
};

struct lcCommandLineOptions
{
	bool ParseOK = true;
	bool Exit = false;
	bool SaveImage = false;
	bool SaveWavefront = false;
	bool Save3DS = false;
	bool SaveCOLLADA = false;
	bool SaveCSV = false;
	bool SaveHTML = false;
	bool SetCameraAngles = false;
	bool SetCameraPosition = false;
	bool Orthographic = false;
	bool SetFoV = false;
	bool SetZPlanes = false;
	bool SetFadeStepsColor = false;
	bool SetHighlightColor = false;
	bool FadeSteps = false;
	bool ImageHighlight = false;
	bool AutomateEdgeColor = false;
/*** LPub3D Mod - DrawConditionalLines ***/
	bool DrawConditionalLines = false;
/*** LPub3D Mod end ***/
	int ImageWidth;
	int ImageHeight;
	int AASamples;
	lcShadingMode ShadingMode;
	float LineWidth;
	lcStudStyle StudStyle;
	lcStep ImageStart;
	lcStep ImageEnd;
	lcVector3 CameraPosition[3];
	lcVector2 CameraLatLon;
	float FoV;
	float PartEdgeContrast;
	float PartColorValueLDIndex;
	lcVector2 ZPlanes;
	lcViewpoint Viewpoint;
	bool StudCylinderColorEnabled;
	quint32 StudCylinderColor;
	bool PartEdgeColorEnabled;
	quint32 PartEdgeColor;
	bool BlackEdgeColorEnabled;
	quint32 BlackEdgeColor;
	bool DarkEdgeColorEnabled;
	quint32 DarkEdgeColor;
	quint32 FadeStepsColor;
	quint32	HighlightColor;
	QString ImageName;
	QString ModelName;
	QString CameraName;
	QString ProjectName;
	QString SaveWavefrontName;
	QString Save3DSName;
	QString SaveCOLLADAName;
	QString SaveCSVName;
	QString SaveHTMLName;
	QList<QPair<QString, bool>> LibraryPaths;
	QString StdOut;
	QString StdErr;
};

enum class lcStartupMode
{
	ShowWindow,
	Success,
	Error
};

/*** LPub3D Mod - change QApplication to QObject ***/
class lcApplication : public QObject
/*** LPub3D Mod end ***/
{
	Q_OBJECT

public:
/*** LPub3D Mod - initialization - move arguments to LP3D Application ***/
	lcApplication(const lcCommandLineOptions &Options);
/*** LPub3D Mod end ***/
	~lcApplication();

	lcApplication(const lcApplication&) = delete;
	lcApplication(lcApplication&&) = delete;
	lcApplication& operator=(const lcApplication&) = delete;
	lcApplication& operator=(lcApplication&&) = delete;

	void SetProject(Project* Project);
	static lcCommandLineOptions ParseCommandLineOptions();
/*** LPub3D Mod - add parent, remove ShowWindow ***/
	lcStartupMode Initialize(const QList<QPair<QString, bool>>& LibraryPaths, QMainWindow *parent = 0);
/*** LPub3D Mod end ***/

/*** LPub3D Mod - process command line ***/
	int Process3DViewerCommandLine();
/*** LPub3D Mod end ***/
	void Shutdown();
	void ShowPreferencesDialog();
	void SaveTabLayout() const;

	bool LoadPartsLibrary(const QList<QPair<QString, bool>>& LibraryPaths, bool OnlyUsePaths);

	void SetClipboard(const QByteArray& Clipboard);
	void ExportClipboard(const QByteArray& Clipboard);

/*** LPub3D Mod - load color entry ***/
	bool LPubHighlightParts();
/*** LPub3D Mod end ***/

/*** LPub3D Mod - lpub fade highlight ***/
	bool LPubFadeParts();
	bool LPubFadeHighlight();
	bool LPubDisplayModelStep();
	bool UseLPubFadeColour();

	QString LPubFadeColour();
/*** LPub3D Mod end ***/

/*** LPub3D Mod - global options ***/
	lcCommandLineOptions mOptions;
/*** LPub3D Mod end ***/
	Project* mProject = nullptr;
	lcPiecesLibrary* mLibrary = nullptr;
	lcPreferences mPreferences;
	QByteArray mClipboard;

protected:
	void UpdateStyle();
	QString GetTabLayoutKey() const;

	QString mDefaultStyle;

/*** LPub3D Mod - LcLib Preferences ***/
	friend class Preferences;
/*** LPub3D Mod end ***/
};

extern lcApplication* gApplication;

inline lcPiecesLibrary* lcGetPiecesLibrary()
{
	return gApplication->mLibrary;
}

inline Project* lcGetActiveProject()
{
	return gApplication->mProject;
}

inline lcPreferences& lcGetPreferences()
{
	return gApplication->mPreferences;
}
