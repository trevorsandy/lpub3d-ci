#include "lc_global.h"
#include "lc_previewwidget.h"
#include "pieceinf.h"
#include "piece.h"
#include "project.h"
#include "lc_model.h"
#include "lc_library.h"
#include "lc_viewwidget.h"
#include "lc_view.h"

lcPreviewDockWidget::lcPreviewDockWidget(QMainWindow* Parent)
	: QMainWindow(Parent)
{
	mPreview = new lcPreview();
	mViewWidget = new lcViewWidget(nullptr, mPreview);
	setCentralWidget(mViewWidget);
	setMinimumSize(200, 200);

	mLockAction = new QAction(QIcon(":/resources/action_preview_unlocked.png"),tr("Lock Preview"), this);
	mLockAction->setCheckable(true);
	mLockAction->setChecked(false);
	mLockAction->setShortcut(tr("Ctrl+L"));
	connect(mLockAction, SIGNAL(triggered()), this, SLOT(SetPreviewLock()));
	SetPreviewLock();

	mLabel = new QLabel();
/*** LPub3D Mod - preview widget for LPub3D ***/
	mToolBar = addToolBar(tr("Preview Toolbar"));
/*** LPub3D Mod end ***/
	mToolBar->setObjectName("PreviewToolbar");
	mToolBar->setStatusTip(tr("Preview Toolbar"));
	mToolBar->setMovable(false);
	mToolBar->addAction(mLockAction);
	mToolBar->addSeparator();
	mToolBar->addWidget(mLabel);
	if (mToolBar->isHidden())
		mToolBar->show();
}

bool lcPreviewDockWidget::SetCurrentPiece(const QString& PartType, int ColorCode)
{
	if (mLockAction->isChecked())
		return true;

	mLabel->setText(tr("Loading..."));
	if (mPreview->SetCurrentPiece(PartType, ColorCode))
	{
/*** LPub3D Mod - preview widget for LPub3D ***/
		QString PartLabel;
		if (mPreview->IsModel())
		{
			if (!mPreview->GetDescription().isEmpty())
				PartLabel = mPreview->GetDescription();
			else
				PartLabel = QFileInfo(PartType).completeBaseName();
			const QString PartSuffix = QString("-%1").arg(SUBMODEL_IMAGE_BASENAME);
			if (PartLabel.endsWith(PartSuffix,Qt::CaseInsensitive))
				PartLabel.chop(PartSuffix.size());
			PartLabel = PartLabel.replace(PartLabel.indexOf(PartLabel.at(0)),1,PartLabel.at(0).toUpper());
		}
		else
		{
			lcColor* Color = &gColorList[lcGetColorIndex(ColorCode)];
			QString ColorName = Color ? QString(Color->Name) : QLatin1String("No Color");
			PartLabel = QString("%1 (%2)%3")
								.arg(mPreview->GetDescription())
								.arg(QFileInfo(PartType).completeBaseName().toUpper())
								.arg(Color ? QString(", %1 (%2)").arg(ColorName).arg(ColorCode) : QString());
		}
		mLabel->setText(PartLabel);
		mLabel->setToolTip(PartLabel);
/*** LPub3D Mod end ***/
		return true;
	}
	return false;
}

void lcPreviewDockWidget::UpdatePreview()
{
/*** LPub3D Mod - preview widget for LPub3D ***/
	if (mLockAction->isChecked())
		return;
/*** LPub3D Mod end ***/

	mPreview->UpdatePreview();
}

void lcPreviewDockWidget::ClearPreview()
{
/*** LPub3D Mod - preview widget for LPub3D ***/
	if (mLockAction->isChecked())
		return;
/*** LPub3D Mod end ***/

	if (mPreview->GetModel()->GetPieces().size())
		mPreview->ClearPreview();

	mLabel->setText(QString());
}

void lcPreviewDockWidget::SetPreviewLock()
{
	bool Locked = mLockAction->isChecked();

	if (Locked && mPreview->GetModel()->GetPieces().empty())
	{
		mLockAction->setChecked(false);
		return;
	}

	QIcon LockIcon(Locked ? ":/resources/action_preview_locked.png" : ":/resources/action_preview_unlocked.png");
	QString StatusTip(Locked
		? tr("Unlock the preview display to enable updates")
		: tr("Lock the preview display to disable updates"));

	mLockAction->setToolTip(Locked ? tr("Unlock Preview") : tr("Lock Preview"));
	mLockAction->setIcon(LockIcon);
	mLockAction->setStatusTip(StatusTip);
}

/*** LPub3D Mod - preview widget for LPub3D ***/
lcPreview::lcPreview(bool SubstituteView)
	: lcView(lcViewType::Preview, nullptr, SubstituteView), mLoader(new Project(true/*IsPreview*/))
{
/*** LPub3D Mod end ***/
	mLoader->SetActiveModel(0);
	mModel = mLoader->GetActiveModel();
}

bool lcPreview::SetCurrentPiece(const QString& PartType, int ColorCode)
{
	lcPiecesLibrary* Library = lcGetPiecesLibrary();
	PieceInfo* Info = Library->FindPiece(PartType.toLatin1().constData(), nullptr, false, false);

	if (Info)
	{
		for (const std::unique_ptr<lcPiece>& ModelPiece : mModel->GetPieces())
		{
			if (Info == ModelPiece->mPieceInfo)
			{
				int ModelColorCode = ModelPiece->GetColorCode();
				if (ModelColorCode == ColorCode)
					return true;
			}
		}

		mIsModel = Info->IsModel();
		mDescription = Info->m_strDescription;

		mModel->SelectAllPieces();
		mModel->DeleteSelectedObjects();

		Library->LoadPieceInfo(Info, false, true);
		Library->WaitForLoadQueue();

		mModel->SetPreviewPieceInfo(Info, lcGetColorIndex(ColorCode));

		Library->ReleasePieceInfo(Info);
	}
	else
	{
/*** LPub3D Mod - preview widget for LPub3D (load submodel file) ***/
		lcPreferences& Preferences = lcGetPreferences();
		QString Path = Preferences.mPreviewLoadPath;
		if (Path.isEmpty())
			Path = QDir::currentPath();
		QString ModelPath = QDir::toNativeSeparators(QString("%1%2%3").arg(Path).arg(QDir::separator()).arg(PartType));
/*** LPub3D Mod end ***/

		if (!mLoader->Load(ModelPath, false))
			return false;

		mLoader->SetActiveModel(0);
		lcGetPiecesLibrary()->RemoveTemporaryPieces();
		mModel = mLoader->GetActiveModel();
		if (!mModel->GetProperties().mDescription.isEmpty())
			mDescription = mModel->GetProperties().mDescription;
		else
			mDescription = PartType;
		mIsModel = true;
	}

	ZoomExtents();

	return true;
}

void lcPreview::ClearPreview()
{
	mLoader = std::unique_ptr<Project>(new Project(true/*IsPreview*/));
	mLoader->SetActiveModel(0);
	mModel = mLoader->GetActiveModel();
	lcGetPiecesLibrary()->UnloadUnusedParts();
	Redraw();
}

void lcPreview::UpdatePreview()
{
	QString PartType;
	int ColorCode = -1;

	for (const std::unique_ptr<lcPiece>& ModelPiece : mModel->GetPieces())
	{
		if (ModelPiece->mPieceInfo)
		{
			PartType = ModelPiece->mPieceInfo->mFileName;
			ColorCode = ModelPiece->GetColorCode();
			break;
		}
	}

	ClearPreview();

	if (!PartType.isEmpty() && ColorCode > -1)
		SetCurrentPiece(PartType, ColorCode);
}
