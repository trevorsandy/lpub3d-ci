#pragma once

#include "lc_application.h"
#include "lc_shortcuts.h"
#include "lc_commands.h"
#include "lc_model.h"

/*** LPub3D Mod - status and logging ***/
#include <QToolBar>
#include <QStatusBar>
#include <declarations.h>
/*** LPub3D Mod end ***/

class lcPartSelectionWidget;
class lcPreviewDockWidget;
class PiecePreview;
class lcQPartsTree;
class lcColorList;
class lcPropertiesWidget;
class lcTimelineWidget;
class lcElidedLabel;
#ifdef QT_NO_PRINTER
class QPrinter;
#endif

#define LC_MAX_RECENT_FILES 4

class lcTabBar : public QTabBar
{
public:
	lcTabBar(QWidget* Parent = nullptr)
		: QTabBar(Parent), mMousePressTab(-1)
	{
	}

protected:
	void mousePressEvent(QMouseEvent* Event) override;
	void mouseReleaseEvent(QMouseEvent* Event) override;

	int mMousePressTab;
};

class lcModelTabWidget : public QWidget
{
	Q_OBJECT

public:
	lcModelTabWidget(lcModel* Model)
	{
		mModel = Model;
		mActiveView = nullptr;
	}

	QWidget* GetAnyViewWidget()
	{
		QWidget* Widget = layout()->itemAt(0)->widget();

		while (Widget->metaObject() == &QSplitter::staticMetaObject)
			Widget = ((QSplitter*)Widget)->widget(0);

		return Widget;
	}

	lcView* GetActiveView() const
	{
		return mActiveView;
	}

	void SetActiveView(lcView* ActiveView)
	{
		mActiveView = ActiveView;
	}

	void RemoveView(const lcView* View)
	{
		if (View == mActiveView)
			mActiveView = nullptr;
	}

	lcModel* GetModel() const
	{
		return mModel;
	}

protected:
	lcModel* mModel;
	lcView* mActiveView;
};

/*** LPub3D Mod - move lcElidedLabel from lcMainWindow source to header ***/
class lcElidedLabel : public QFrame
{
public:
    explicit lcElidedLabel(QWidget* Parent = nullptr)
        : QFrame(Parent)
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    }

    void setText(const QString& Text)
    {
        mText = Text;
        update();
    }

protected:
    void paintEvent(QPaintEvent* event) override;

    QString mText;
};
/*** LPub3D Mod end ***/

class lcMainWindow : public QMainWindow
{
	Q_OBJECT

public:
/*** LPub3D Mod - define lcMainWindow parent ***/
	lcMainWindow(QMainWindow *parent = 0);
/*** LPub3D Mod end ***/
	~lcMainWindow();

	void CreateWidgets();

	lcTool GetTool() const
	{
		return mTool;
	}

	lcTransformType GetTransformType() const
	{
		return mTransformType;
	}

	bool GetAddKeys() const
	{
		return mAddKeys;
	}

	float GetMoveXYSnap() const
	{
		const float SnapXYTable[] = { 0.0f, 1.0f, 5.0f, 8.0f, 10.0f, 20.0f, 40.0f, 60.0f, 80.0f, 160.0f };
		return mMoveSnapEnabled ? SnapXYTable[mMoveXYSnapIndex] : 0.0f;
	}

	float GetMoveZSnap() const
	{
		const float SnapZTable[] = { 0.0f, 1.0f, 5.0f, 8.0f, 10.0f, 20.0f, 24.0f, 48.0f, 96.0f, 192.0f };
		return mMoveSnapEnabled ? SnapZTable[mMoveZSnapIndex] : 0.0f;
	}

	float GetAngleSnap() const
	{
		const float AngleTable[] = { 0.0f, 1.0f, 5.0f, 15.0f, 22.5f, 30.0f, 45.0f, 60.0f, 90.0f, 180.0f };
		return mAngleSnapEnabled ? AngleTable[mAngleSnapIndex] : 0.0f;
	}

	QString GetMoveXYSnapText() const
	{
		QString SnapXYText[] = { tr("0"), tr("1/20S"), tr("1/4S"), tr("1F"), tr("1/2S"), tr("1S"), tr("2S"), tr("3S"), tr("4S"), tr("8S") };
		return mMoveSnapEnabled ? SnapXYText[mMoveXYSnapIndex] : tr("None");
	}

	QString GetMoveZSnapText() const
	{
		QString SnapZText[] = { tr("0"), tr("1/20S"), tr("1/4S"), tr("1F"), tr("1/2S"), tr("1S"), tr("1B"), tr("2B"), tr("4B"), tr("8B") };
		return mMoveSnapEnabled ? SnapZText[mMoveZSnapIndex] : tr("None");
	}

	QString GetAngleSnapText() const
	{
		return mAngleSnapEnabled ? QString::number(GetAngleSnap()) : tr("None");
	}

	bool GetRelativeTransform() const
	{
		return mRelativeTransform;
	}

	bool GetSeparateTransform() const
	{
		return mLocalTransform;
	}

	lcSelectionMode GetSelectionMode() const
	{
		return mSelectionMode;
	}

	PieceInfo* GetCurrentPieceInfo() const
	{
		return mCurrentPieceInfo;
	}

	lcView* GetActiveView() const
	{
		const lcModelTabWidget* const CurrentTab = mModelTabWidget ? (lcModelTabWidget*)mModelTabWidget->currentWidget() : nullptr;
		return CurrentTab ? CurrentTab->GetActiveView() : nullptr;
	}

	lcModel* GetActiveModel() const;
	lcModelTabWidget* GetTabForView(lcView* View) const;

	lcModel* GetCurrentTabModel() const
	{
		const lcModelTabWidget* const CurrentTab = (lcModelTabWidget*)mModelTabWidget->currentWidget();
		return CurrentTab ? CurrentTab->GetModel() : nullptr;
	}

	lcPartSelectionWidget* GetPartSelectionWidget() const
	{
		return mPartSelectionWidget;
	}

	lcPreviewDockWidget* GetPreviewWidget() const
	{
		return mPreviewWidget;
	}

	QMenu* GetToolsMenu() const
	{
		return mToolsMenu;
	}

/*** LPub3D Mod - expand toolbars ***/
	QMenu* GetEditMenu() const
	{
		return mEditMenu;
	}

	QMenu* GetPieceMenu() const
	{
		return mPieceMenu;
	}
/*** LPub3D Mod end ***/

	QMenu* GetViewpointMenu() const
	{
		return mViewpointMenu;
	}

	QMenu* GetCameraMenu() const
	{
		return mCameraMenu;
	}

	QMenu* GetProjectionMenu() const
	{
		return mProjectionMenu;
	}

	QMenu* GetShadingMenu() const
	{
		return mShadingMenu;
	}

/*** LPub3D Mod - Expose dockwidgets toolbars and menus ***/
	QMenu* GetTransformMenu() const
	{
		return mTransformMenu;
	}

	QMenu* GetSelectionModeMenu()
	{
		return mSelectionModeMenu;
	}

	QToolBar* GetStandardToolBar()
	{
		return mStandardToolBar;
	}

	QToolBar* GetToolsToolBar()
	{
		return mToolsToolBar;
	}

	QToolBar* GetVisualizationToolBar()
	{
		return mVisualizationToolBar;
	}

	QDockWidget* GetPartsToolBar()
	{
		return mPartsToolBar;
	}

	QDockWidget* GetPropertiesToolBar()
	{
		return mPropertiesToolBar;
	}

	QDockWidget* GetTimelineToolBar()
	{
		return mTimelineToolBar;
	}

	QDockWidget* GetColorsToolBar()
	{
		return mColorsToolBar;
	}
/*** LPub3D Mod end ***/

	QByteArray GetTabLayout();
	void RestoreTabLayout(const QByteArray& TabLayout);
	void RemoveAllModelTabs();
	void CloseCurrentModelTab();
	void SetCurrentModelTab(lcModel* Model);
	void ResetCameras();
	void AddView(lcView* View);
	void RemoveView(lcView* View);

	void SetTool(lcTool Tool);
	void SetTransformType(lcTransformType TransformType);
	void SetColorIndex(int ColorIndex);
	void SetMoveSnapEnabled(bool Enabled);
	void SetAngleSnapEnabled(bool Enabled);
	void SetMoveXYSnapIndex(int Index);
	void SetMoveZSnapIndex(int Index);
	void SetAngleSnapIndex(int Index);
	void SetRelativeTransform(bool RelativeTransform);
	void SetSeparateTransform(bool SelectionTransform);
	void SetShadingMode(lcShadingMode ShadingMode);
	void SetSelectionMode(lcSelectionMode SelectionMode);
	void ToggleViewSphere();
	void ToggleAxisIcon();
	void ToggleGrid();
	void ToggleFadePreviousSteps();

/***    void NewProject();                     // LPub3D Mod - moved to public slots ***/
	bool OpenProject(const QString& FileName);
	void OpenRecentProject(int RecentFileIndex);
	void MergeProject();
	void ImportLDD();
	void ImportInventory();
	bool SaveProject(const QString& FileName);
	bool SaveProjectIfModified();
	bool SetModelFromFocus();
	void SetModelFromSelection();
	void HandleCommand(lcCommandId CommandId);

	void AddRecentFile(const QString& FileName);
	void RemoveRecentFile(int FileIndex);

	void SplitHorizontal();
	void SplitVertical();
	void RemoveActiveView();
	void ResetViews();

	void TogglePrintPreview();
	void ToggleFullScreen();

/*** LPub3D Mod - selected Line ***/
	void UpdateSelectedObjects(bool SelectionChanged, int SelectionType = VIEWER_LINE);
/*** LPub3D Mod end ***/
	void UpdateTimeline(bool Clear, bool UpdateItems);
	void UpdatePaste(bool Enabled);
	void UpdateCurrentStep();
	void SetAddKeys(bool AddKeys);
	void UpdateLockSnap();
	void UpdateSnap();
	void UpdateColor();
	void UpdateUndoRedo(const QString& UndoText, const QString& RedoText);
	void UpdateShadingMode();
	void UpdateSelectionMode();
	void UpdateModels();
	void UpdateInUseCategory();
	void UpdateCategories();
	void UpdateTitle();
	void UpdateModified(bool Modified);
	void UpdateRecentFiles();
	void UpdateShortcuts();
/*** LPub3D Mod - Update Default Camera ***/
	void UpdateDefaultCameraProperties();
/*** LPub3D Mod end ***/
/*** LPub3D Mod - Timeline part icons ***/
	bool IsLPub3DSubModel(QString &Piece);
	const QString GetPliIconsPath(QString &key);
/*** LPub3D Mod end ***/
 /*** LPub3D Mod - Rotate step angles ***/
	void GetRotStepMetaAngles();
/*** LPub3D Mod end ***/
/*** LPub3D Mod - parse and set rotstep line on model file load ***/
	void ParseAndSetRotStep(QTextStream& LineStream);
/*** LPub3D Mod end ***/
/*** LPub3D Mod - rotstep transform ***/
	lcVector3 GetRotStepTransformAmount();
/*** LPub3D Mod end ***/
	lcVector3 GetTransformAmount();

	QString mRecentFiles[LC_MAX_RECENT_FILES];
	int mColorIndex;
	QAction* mActions[LC_NUM_COMMANDS];
/*** LPub3D Mod - Timeline part icons ***/
	bool mSubmodelIconsLoaded;
/*** LPub3D Mod end ***/

public slots:
	void ProjectFileChanged(const QString& Path);
	void PreviewPiece(const QString& PartId, int ColorCode, bool ShowPreview);
	void TogglePreviewWidget(bool Visible);
	void SetCurrentPieceInfo(PieceInfo* Info);
/*** LPub3D Mod - relocate new project ***/
	void NewProject();               // move from public:
/*** LPub3D Mod end ***/

/*** LPub3D Mod - transform command ***/
	void ApplyRotStepMeta(lcCommandId CommandId);
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Selected Parts ***/
	int GetImageType();
	void SetSelectedPieces(const QVector<int> &LineTypeIndexes);
/*** LPub3D Mod end ***/

/*** LPub3D Mod - preview widget for LPub3D ***/
	void CreatePreviewWidget();
/*** LPub3D Mod end ***/

/*** LPub3D Mod - signals ***/
signals:
/*** LPub3D Mod - rotate step signals ***/
	void SetRotStepAngles(const QVector<float>&, bool=false);
	void SetRotStepAngleX(float,  bool=false);
	void SetRotStepAngleY(float,  bool=false);
	void SetRotStepAngleZ(float,  bool=false);
	void SetRotStepType(const QString&, bool=false);
	void SetRotStepCommand();
/*** LPub3D Mod end ***/
/*** LPub3D Mod - export image completion ***/
	void updateSig();
/*** LPub3D Mod end ***/
/*** LPub3D Mod - Selected Parts ***/
	void SetActiveModelSig(const QString &);
	void SelectedPartLinesSig(const QVector<TypeLine> &, PartSource);
/*** LPub3D Mod end ***/
/*** LPub3D Mod - Move unod/redo trigger to LPub3D ***/
	void UpdateUndoRedoSig(const QString&, const QString&);
/*** LPub3D Mod end ***/
/*** LPub3D Mod - preview widget for LPub3D ***/
	void TogglePreviewWidgetSig(bool);
/*** LPub3D Mod end ***/

protected slots:
	void CameraMenuAboutToShow();
	void ProjectionMenuAboutToShow();
	void ViewFocusReceived();
	void UpdateDockWidgetActions();
	void UpdateGamepads();
	void ModelTabContextMenuRequested(const QPoint& Point);
	void ModelTabCloseOtherTabs();
	void ModelTabClosed(int Index);
	void ModelTabChanged(int Index);
	void ClipboardChanged();
	void ActionTriggered();
	void ColorChanged(int ColorIndex);
	void PartListPicked(PieceInfo* Info);
	void ColorButtonClicked();
	void Print(QPrinter* Printer);
	void EnableWindowFlags(bool);

protected:
	void closeEvent(QCloseEvent *event) override;
	void dragEnterEvent(QDragEnterEvent* Event) override;
	void dropEvent(QDropEvent* Event) override;
	QMenu* createPopupMenu() override;

	void CreateActions();
	void CreateMenus();
	void CreateToolBars();
	void CreateStatusBar();
	lcView* CreateView(lcModel* Model);
	void SetActiveView(lcView* ActiveView);
	void ToggleDockWidget(QWidget* DockWidget);
	void SplitView(Qt::Orientation Orientation);
	void ShowUpdatesDialog();
	void ShowAboutDialog();
	void ShowHTMLDialog();
	void ShowRenderDialog(int Command);
	void ShowInstructionsDialog();
	void ShowPrintDialog();
/*** LPub3D Mod - preview widget for LPub3D ***/
	//void CreatePreviewWidget(); // moved to public
/*** LPub3D Mod end ***/

	bool OpenProjectFile(const QString& FileName);

	lcModelTabWidget* GetTabWidgetForModel(const lcModel* Model) const
	{
		for (int TabIdx = 0; TabIdx < mModelTabWidget->count(); TabIdx++)
		{
			lcModelTabWidget* TabWidget = (lcModelTabWidget*)mModelTabWidget->widget(TabIdx);

			if (TabWidget->GetModel() == Model)
				return TabWidget;
		}

		return nullptr;
	}

	QTimer mGamepadTimer;
	QDateTime mLastGamepadUpdate;

	bool mAddKeys;
	lcTool mTool;
	lcTransformType mTransformType;
	bool mMoveSnapEnabled;
	bool mAngleSnapEnabled;
	int mMoveXYSnapIndex;
	int mMoveZSnapIndex;
	int mAngleSnapIndex;
	bool mRelativeTransform;
	bool mLocalTransform;
	PieceInfo* mCurrentPieceInfo;
	lcSelectionMode mSelectionMode;
	int mModelTabWidgetContextMenuIndex;

	QAction* mActionFileRecentSeparator;

	QTabWidget* mModelTabWidget;
	QToolBar* mStandardToolBar;
	QToolBar* mToolsToolBar;
/*** LPub3D Mod - expand toolbars ***/
	QToolBar* mVisualizationToolBar;
/*** LPub3D Mod end ***/
/*** LPub3D Mod - rotate step ***/
	lcVector3 mExistingTransform;
	lcVector3 mExistingRotStep;
	QString mRotStepType;
/*** LPub3D Mod end ***/

	QToolBar* mTimeToolBar;
	QDockWidget* mPreviewToolBar;
	QDockWidget* mPartsToolBar;
	QDockWidget* mColorsToolBar;
	QDockWidget* mPropertiesToolBar;
	QDockWidget* mTimelineToolBar;

	lcPartSelectionWidget* mPartSelectionWidget;
	lcColorList* mColorList;
	QToolButton* mColorButton;
	lcPropertiesWidget* mPropertiesWidget;
	lcTimelineWidget* mTimelineWidget;
	QLineEdit* mTransformXEdit;
	QLineEdit* mTransformYEdit;
	QLineEdit* mTransformZEdit;
	lcPreviewDockWidget* mPreviewWidget;

	lcElidedLabel* mStatusBarLabel;
	QLabel* mStatusSnapLabel;
    QLabel* mStatusPositionLabel;
/*** LPub3D Mod - disable time status label ***/
	//QLabel* mStatusTimeLabel;
/*** LPub3D Mod end ***/

	QMenu* mTransformMenu;
	QMenu* mToolsMenu;
/*** LPub3D Mod - expand toolbars ***/
	QMenu* mEditMenu;
	QMenu* mPieceMenu;
/*** LPub3D Mod end ***/
	QMenu* mViewpointMenu;
	QMenu* mCameraMenu;
	QMenu* mProjectionMenu;
	QMenu* mShadingMenu;
	QMenu* mSelectionModeMenu;
/*** LPub3D Mod - transform command ***/
	friend class Gui;
/*** LPub3D Mod end ***/
};

extern class lcMainWindow* gMainWindow;
