#include "lc_global.h"
#include "lc_qpreferencesdialog.h"
#include "ui_lc_qpreferencesdialog.h"
#include "lc_qutils.h"
#include "lc_qcategorydialog.h"
#include "lc_basewindow.h"
#include "lc_library.h"
#include "lc_application.h"
#include "lc_qutils.h"
#include "lc_glextensions.h"
#include "pieceinf.h"

static const char* gLanguageLocales[] =
{
	"", "cs_CZ", "de_DE", "en_US", "fr_FR", "pt_PT", "es_ES"
};

lcQPreferencesDialog::lcQPreferencesDialog(QWidget* Parent, lcPreferencesDialogOptions* Options)
	: QDialog(Parent), mOptions(Options), ui(new Ui::lcQPreferencesDialog)
{
	ui->setupUi(this);

/*** LPub3D Mod - suppress Win/macOS preferences dialog settings ***/
/***
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
	ui->povrayLabel->hide();
	ui->povrayExecutable->hide();
	ui->povrayExecutableBrowse->hide();
	delete ui->povrayLabel;
	delete ui->povrayLayout;
#endif
***/
/*** LPub3D Mod end ***/

	connect(ui->AxesColorButton, SIGNAL(clicked()), this, SLOT(ColorButtonClicked()));
	connect(ui->OverlayColorButton, SIGNAL(clicked()), this, SLOT(ColorButtonClicked()));
	connect(ui->FadeStepsColor, SIGNAL(clicked()), this, SLOT(ColorButtonClicked()));
	connect(ui->HighlightNewPartsColor, SIGNAL(clicked()), this, SLOT(ColorButtonClicked()));
	connect(ui->gridStudColor, SIGNAL(clicked()), this, SLOT(ColorButtonClicked()));
	connect(ui->gridLineColor, SIGNAL(clicked()), this, SLOT(ColorButtonClicked()));
	connect(ui->ViewSphereColorButton, SIGNAL(clicked()), this, SLOT(ColorButtonClicked()));
	connect(ui->ViewSphereTextColorButton, SIGNAL(clicked()), this, SLOT(ColorButtonClicked()));
	connect(ui->ViewSphereHighlightColorButton, SIGNAL(clicked()), this, SLOT(ColorButtonClicked()));
	connect(ui->categoriesTree, SIGNAL(itemSelectionChanged()), this, SLOT(updateParts()));
	ui->shortcutEdit->installEventFilter(this);
	connect(ui->commandList, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(commandChanged(QTreeWidgetItem*)));
	connect(ui->mouseTree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(MouseTreeItemChanged(QTreeWidgetItem*)));

	ui->partsLibrary->setText(mOptions->LibraryPath);
	ui->ColorConfigEdit->setText(mOptions->ColorConfigPath);
	ui->MinifigSettingsEdit->setText(mOptions->MinifigSettingsPath);
	ui->povrayExecutable->setText(mOptions->POVRayPath);
	ui->lgeoPath->setText(mOptions->LGEOPath);
	ui->authorName->setText(mOptions->DefaultAuthor);
	ui->mouseSensitivity->setValue(mOptions->Preferences.mMouseSensitivity);
	const bool ColorThemeBlocked = ui->ColorTheme->blockSignals(true);
	ui->ColorTheme->setCurrentIndex(static_cast<int>(mOptions->Preferences.mColorTheme));
	ui->ColorTheme->blockSignals(ColorThemeBlocked);
	for (unsigned int LanguageIdx = 0; LanguageIdx < LC_ARRAY_COUNT(gLanguageLocales); LanguageIdx++)
	{
		if (mOptions->Language == gLanguageLocales[LanguageIdx])
		{
			ui->Language->setCurrentIndex(LanguageIdx);
			break;
		}
	}
	ui->checkForUpdates->setCurrentIndex(mOptions->CheckForUpdates);
	ui->fixedDirectionKeys->setChecked(mOptions->Preferences.mFixedAxes);
	ui->autoLoadMostRecent->setChecked(mOptions->Preferences.mAutoLoadMostRecent);
	ui->RestoreTabLayout->setChecked(mOptions->Preferences.mRestoreTabLayout);

	ui->antiAliasing->setChecked(mOptions->AASamples != 1);
	if (mOptions->AASamples == 8)
		ui->antiAliasingSamples->setCurrentIndex(2);
	else if (mOptions->AASamples == 4)
		ui->antiAliasingSamples->setCurrentIndex(1);
	else
		ui->antiAliasingSamples->setCurrentIndex(0);
	ui->edgeLines->setChecked(mOptions->Preferences.mDrawEdgeLines);

	if (QGLFormat::defaultFormat().sampleBuffers() && QGLFormat::defaultFormat().samples() > 1)
	{
		glGetFloatv(GL_SMOOTH_LINE_WIDTH_RANGE, mLineWidthRange);
		glGetFloatv(GL_SMOOTH_LINE_WIDTH_GRANULARITY, &mLineWidthGranularity);
	}
	else
	{
		glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, mLineWidthRange);
		mLineWidthGranularity = 1.0f;
	}

	ui->LineWidthSlider->setRange(0, (mLineWidthRange[1] - mLineWidthRange[0]) / mLineWidthGranularity);
	ui->LineWidthSlider->setValue((mOptions->Preferences.mLineWidth - mLineWidthRange[0]) / mLineWidthGranularity);

	ui->MeshLOD->setChecked(mOptions->Preferences.mAllowLOD);
	ui->FadeSteps->setChecked(mOptions->Preferences.mFadeSteps);
	ui->HighlightNewParts->setChecked(mOptions->Preferences.mHighlightNewParts);
	ui->gridStuds->setChecked(mOptions->Preferences.mDrawGridStuds);
	ui->gridLines->setChecked(mOptions->Preferences.mDrawGridLines);
	ui->gridLineSpacing->setText(QString::number(mOptions->Preferences.mGridLineSpacing));
	ui->AxisIconCheckBox->setChecked(mOptions->Preferences.mDrawAxes);

	ui->ViewSphereLocationCombo->setCurrentIndex((int)mOptions->Preferences.mViewSphereLocation);

	if (mOptions->Preferences.mViewSphereEnabled)
	{
		switch (mOptions->Preferences.mViewSphereSize)
		{
		case 200:
			ui->ViewSphereSizeCombo->setCurrentIndex(3);
			break;
		case 100:
			ui->ViewSphereSizeCombo->setCurrentIndex(2);
			break;
		case 50:
			ui->ViewSphereSizeCombo->setCurrentIndex(1);
			break;
		default:
			ui->ViewSphereSizeCombo->setCurrentIndex(0);
			break;
		}
	}
	else
		ui->ViewSphereSizeCombo->setCurrentIndex(0);

	ui->studLogo->setChecked(mOptions->StudLogo);
	if (ui->studLogo->isChecked())
		ui->studLogoCombo->setCurrentIndex(mOptions->StudLogo - 1);
	else
		ui->studLogoCombo->setCurrentIndex(mOptions->StudLogo);

	if (!gSupportsShaderObjects)
		ui->ShadingMode->removeItem(static_cast<int>(lcShadingMode::DefaultLights));
	ui->ShadingMode->setCurrentIndex(static_cast<int>(mOptions->Preferences.mShadingMode));

	QPixmap pix(12, 12);

	pix.fill(QColor(LC_RGBA_RED(mOptions->Preferences.mAxesColor), LC_RGBA_GREEN(mOptions->Preferences.mAxesColor), LC_RGBA_BLUE(mOptions->Preferences.mAxesColor)));
	ui->AxesColorButton->setIcon(pix);

	pix.fill(QColor(LC_RGBA_RED(mOptions->Preferences.mOverlayColor), LC_RGBA_GREEN(mOptions->Preferences.mOverlayColor), LC_RGBA_BLUE(mOptions->Preferences.mOverlayColor)));
	ui->OverlayColorButton->setIcon(pix);

	pix.fill(QColor(LC_RGBA_RED(mOptions->Preferences.mActiveViewColor), LC_RGBA_GREEN(mOptions->Preferences.mActiveViewColor), LC_RGBA_BLUE(mOptions->Preferences.mActiveViewColor)));
	ui->ActiveViewColorButton->setIcon(pix);
	
	pix.fill(QColor(LC_RGBA_RED(mOptions->Preferences.mFadeStepsColor), LC_RGBA_GREEN(mOptions->Preferences.mFadeStepsColor), LC_RGBA_BLUE(mOptions->Preferences.mFadeStepsColor)));
	ui->FadeStepsColor->setIcon(pix);

	pix.fill(QColor(LC_RGBA_RED(mOptions->Preferences.mHighlightNewPartsColor), LC_RGBA_GREEN(mOptions->Preferences.mHighlightNewPartsColor), LC_RGBA_BLUE(mOptions->Preferences.mHighlightNewPartsColor)));
	ui->HighlightNewPartsColor->setIcon(pix);

	pix.fill(QColor(LC_RGBA_RED(mOptions->Preferences.mGridStudColor), LC_RGBA_GREEN(mOptions->Preferences.mGridStudColor), LC_RGBA_BLUE(mOptions->Preferences.mGridStudColor)));
	ui->gridStudColor->setIcon(pix);

	pix.fill(QColor(LC_RGBA_RED(mOptions->Preferences.mGridLineColor), LC_RGBA_GREEN(mOptions->Preferences.mGridLineColor), LC_RGBA_BLUE(mOptions->Preferences.mGridLineColor)));
	ui->gridLineColor->setIcon(pix);

	pix.fill(QColor(LC_RGBA_RED(mOptions->Preferences.mViewSphereColor), LC_RGBA_GREEN(mOptions->Preferences.mViewSphereColor), LC_RGBA_BLUE(mOptions->Preferences.mViewSphereColor)));
	ui->ViewSphereColorButton->setIcon(pix);

	pix.fill(QColor(LC_RGBA_RED(mOptions->Preferences.mViewSphereTextColor), LC_RGBA_GREEN(mOptions->Preferences.mViewSphereTextColor), LC_RGBA_BLUE(mOptions->Preferences.mViewSphereTextColor)));
	ui->ViewSphereTextColorButton->setIcon(pix);

	pix.fill(QColor(LC_RGBA_RED(mOptions->Preferences.mViewSphereHighlightColor), LC_RGBA_GREEN(mOptions->Preferences.mViewSphereHighlightColor), LC_RGBA_BLUE(mOptions->Preferences.mViewSphereHighlightColor)));
	ui->ViewSphereHighlightColorButton->setIcon(pix);

	on_studLogo_toggled();
	on_antiAliasing_toggled();
	on_edgeLines_toggled();
	on_LineWidthSlider_valueChanged();
	on_FadeSteps_toggled();
	on_HighlightNewParts_toggled();
	on_gridStuds_toggled();
	on_gridLines_toggled();
	on_ViewSphereSizeCombo_currentIndexChanged(ui->ViewSphereSizeCombo->currentIndex());

	updateCategories();
	ui->categoriesTree->setCurrentItem(ui->categoriesTree->topLevelItem(0));

	updateCommandList();
	new lcQTreeWidgetColumnStretcher(ui->commandList, 0);
	commandChanged(nullptr);

	UpdateMouseTree();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
	ui->mouseTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
	ui->mouseTree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	ui->mouseTree->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
#else
	ui->mouseTree->header()->setResizeMode(0, QHeaderView::Stretch);
	ui->mouseTree->header()->setResizeMode(1, QHeaderView::ResizeToContents);
	ui->mouseTree->header()->setResizeMode(2, QHeaderView::ResizeToContents);
#endif
	MouseTreeItemChanged(nullptr);

/*** LPub3D Mod - Update Default Camera ***/
	QPalette readOnlyPalette = QApplication::palette();
	if (mOptions->Preferences.mColorTheme == lcColorTheme::Dark)
		readOnlyPalette.setColor(QPalette::Base,QColor("#3E3E3E")); // THEME_DARK_PALETTE_MIDLIGHT
	else
		readOnlyPalette.setColor(QPalette::Base,QColor("#AEADAC")); // THEME_DEFAULT_PALETTE_LIGHT
	readOnlyPalette.setColor(QPalette::Text,QColor("#808080"));     // THEME_PALETTE_DISABLED_TEXT

	ui->defaultCameraProperties->setChecked(mOptions->Preferences.mDefaultCameraProperties);
	ui->cameraDefaultDistanceFactor->setValue(qreal(mOptions->Preferences.mDDF));
	ui->cameraDefaultPosition->setValue(qreal(mOptions->Preferences.mCDP));
	ui->cameraFoV->setValue(qreal(mOptions->Preferences.mCFoV));
	ui->cameraNearPlane->setValue(qreal(mOptions->Preferences.mCNear));
	ui->cameraFarPlane->setValue(qreal(mOptions->Preferences.mCFar));
	ui->cameraDistanceFactor->setText(QString::number(qreal(mOptions->Preferences.mCDP) /
													  -qreal(mOptions->Preferences.mDDF)));

	connect(ui->resetDefaultDistanceFactor, SIGNAL(clicked()), this, SLOT(cameraPropertyReset()));
	connect(ui->resetDefaultPosition, SIGNAL(clicked()), this, SLOT(cameraPropertyReset()));
	connect(ui->resetFoV, SIGNAL(clicked()), this, SLOT(cameraPropertyReset()));
	connect(ui->resetNearPlane, SIGNAL(clicked()), this, SLOT(cameraPropertyReset()));
	connect(ui->resetFarPlane, SIGNAL(clicked()), this, SLOT(cameraPropertyReset()));
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Native Renderer settings ***/
	ui->ViewpointsCombo->setCurrentIndex(mOptions->Preferences.mNativeViewpoint);
	ui->ProjectionCombo->setCurrentIndex(mOptions->Preferences.mNativeProjection);
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Timeline part icons ***/
	bool preferredIcons = mOptions->Preferences.mViewPieceIcons;
	ui->viewPieceIconsRadio->setChecked(preferredIcons);
	ui->viewColorIconsRadio->setChecked(!preferredIcons);
/*** LPub3D Mod end ***/

/*** LPub3D Mod - true fade ***/
	ui->ConditionalLines->setChecked(mOptions->Preferences.mConditionalLines);
/*** LPub3D Mod end ***/

/*** LPub3D Mod - set preferences dialog properties ***/
	ui->authorName->setReadOnly(true);
	ui->authorName->setPalette(readOnlyPalette);
	ui->partsLibrary->setReadOnly(true);
	ui->partsLibrary->setPalette(readOnlyPalette);
	ui->partsLibraryBrowse->hide();
	ui->partsArchiveBrowse->hide();
	ui->cameraDistanceFactor->setReadOnly(true);
	ui->cameraDistanceFactor->setPalette(readOnlyPalette);
	ui->ColorConfigEdit->setReadOnly(true);
	ui->ColorConfigEdit->setPalette(readOnlyPalette);
	ui->ColorConfigBrowseButton->hide();
	ui->povrayExecutable->setReadOnly(true);
	ui->povrayExecutable->setPalette(readOnlyPalette);
	ui->povrayExecutableBrowse->hide();
	ui->lgeoPath->setReadOnly(true);
	ui->lgeoPath->setPalette(readOnlyPalette);
	ui->MinifigSettingsEdit->hide();
	ui->MinifigSettingsBrowseButton->hide();
	ui->MinifigSettingsLabel->hide();
	ui->autoLoadMostRecent->hide();
	ui->lgeoPathBrowse->hide();
	ui->Language->hide();
	ui->checkForUpdates->hide();
	ui->RestoreTabLayout->hide();
	ui->ColorTheme->setDisabled(true);
	ui->label_29->hide();					//label Language
	ui->label_10->hide();					//label check for updates
	ui->fixedDirectionKeys->hide();
	ui->tabWidget->removeTab(4);			//hide tabKeyboard
	ui->tabWidget->removeTab(4);			//hide mouse
/*** LPub3D Mod end ***/

}

lcQPreferencesDialog::~lcQPreferencesDialog()
{
	delete ui;
}

void lcQPreferencesDialog::accept()
{
	int gridLineSpacing = ui->gridLineSpacing->text().toInt();
	if (gridLineSpacing < 1)
	{
/*** LPub3D Mod - set 3DViewer label ***/
		QMessageBox::information(this, "3DViewer", tr("Grid spacing must be greater than 0."));
/*** LPub3D Mod end ***/
		return;
	}

	mOptions->LibraryPath = ui->partsLibrary->text();
	mOptions->MinifigSettingsPath = ui->MinifigSettingsEdit->text();
	mOptions->ColorConfigPath = ui->ColorConfigEdit->text();
	mOptions->POVRayPath = ui->povrayExecutable->text();
	mOptions->LGEOPath = ui->lgeoPath->text();
	mOptions->DefaultAuthor = ui->authorName->text();
	mOptions->Preferences.mMouseSensitivity = ui->mouseSensitivity->value();
	mOptions->Preferences.mColorTheme = static_cast<lcColorTheme>(ui->ColorTheme->currentIndex());

	int Language = ui->Language->currentIndex();
	if (Language < 0 || Language > static_cast<int>(LC_ARRAY_COUNT(gLanguageLocales)))
		Language = 0;
	mOptions->Language = gLanguageLocales[Language];

	mOptions->CheckForUpdates = ui->checkForUpdates->currentIndex();
	mOptions->Preferences.mFixedAxes = ui->fixedDirectionKeys->isChecked();
	mOptions->Preferences.mAutoLoadMostRecent = ui->autoLoadMostRecent->isChecked();
	mOptions->Preferences.mRestoreTabLayout = ui->RestoreTabLayout->isChecked();

	if (!ui->antiAliasing->isChecked())
		mOptions->AASamples = 1;
	else if (ui->antiAliasingSamples->currentIndex() == 2)
		mOptions->AASamples = 8;
	else if (ui->antiAliasingSamples->currentIndex() == 1)
		mOptions->AASamples = 4;
	else
		mOptions->AASamples = 2;

	mOptions->Preferences.mDrawEdgeLines = ui->edgeLines->isChecked();
	mOptions->Preferences.mLineWidth = mLineWidthRange[0] + static_cast<float>(ui->LineWidthSlider->value()) * mLineWidthGranularity;
	mOptions->Preferences.mAllowLOD = ui->MeshLOD->isChecked();
	mOptions->Preferences.mFadeSteps = ui->FadeSteps->isChecked();
	mOptions->Preferences.mHighlightNewParts = ui->HighlightNewParts->isChecked();

	mOptions->Preferences.mDrawGridStuds = ui->gridStuds->isChecked();
	mOptions->Preferences.mDrawGridLines = ui->gridLines->isChecked();
	mOptions->Preferences.mGridLineSpacing = gridLineSpacing;

	mOptions->Preferences.mDrawAxes = ui->AxisIconCheckBox->isChecked();
	mOptions->Preferences.mViewSphereLocation = (lcViewSphereLocation)ui->ViewSphereLocationCombo->currentIndex();

	switch (ui->ViewSphereSizeCombo->currentIndex())
	{
	case 3:
		mOptions->Preferences.mViewSphereSize = 200;
		break;
	case 2:
		mOptions->Preferences.mViewSphereSize = 100;
		break;
	case 1:
		mOptions->Preferences.mViewSphereSize = 50;
		break;
	default:
		mOptions->Preferences.mViewSphereEnabled = 0;
		break;
	}

	mOptions->Preferences.mShadingMode = (lcShadingMode)ui->ShadingMode->currentIndex();

	if (ui->studLogoCombo->isEnabled())
		mOptions->StudLogo = ui->studLogoCombo->currentIndex() + 1;
	else
		mOptions->StudLogo = 0;

/*** LPub3D Mod - Update Default Camera ***/
	mOptions->Preferences.mDefaultCameraProperties = ui->defaultCameraProperties->isChecked();
	mOptions->Preferences.mDDF = float(ui->cameraDefaultDistanceFactor->value());
	mOptions->Preferences.mCDP = float(ui->cameraDefaultPosition->value());
	mOptions->Preferences.mCFoV = float(ui->cameraFoV->value());
	mOptions->Preferences.mCNear = float(ui->cameraNearPlane->value());
	mOptions->Preferences.mCFar = float(ui->cameraFarPlane->value());
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Native Renderer settings ***/
	mOptions->Preferences.mNativeViewpoint = ui->ViewpointsCombo->currentIndex();
	mOptions->Preferences.mNativeProjection = ui->ProjectionCombo->currentIndex();
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Timeline part icons ***/
	mOptions->Preferences.mViewPieceIcons = ui->viewPieceIconsRadio->isChecked();
/*** LPub3D Mod end ***/

/*** LPub3D Mod - true fade ***/
	mOptions->Preferences.mConditionalLines = ui->ConditionalLines->isChecked();
/*** LPub3D Mod end ***/

	QDialog::accept();
}

void lcQPreferencesDialog::on_partsLibraryBrowse_clicked()
{
	QString result = QFileDialog::getExistingDirectory(this, tr("Select Parts Library Folder"), ui->partsLibrary->text());

	if (!result.isEmpty())
		ui->partsLibrary->setText(QDir::toNativeSeparators(result));
}

void lcQPreferencesDialog::on_partsArchiveBrowse_clicked()
{
	QString result = QFileDialog::getOpenFileName(this, tr("Select Parts Library Archive"), ui->partsLibrary->text(), tr("Supported Archives (*.zip *.bin);;All Files (*.*)"));

	if (!result.isEmpty())
		ui->partsLibrary->setText(QDir::toNativeSeparators(result));
}

void lcQPreferencesDialog::on_ColorConfigBrowseButton_clicked()
{
	QString Result = QFileDialog::getOpenFileName(this, tr("Select Color Configuration File"), ui->ColorConfigEdit->text(), tr("Settings Files (*.ldr);;All Files (*.*)"));

	if (!Result.isEmpty())
		ui->ColorConfigEdit->setText(QDir::toNativeSeparators(Result));
}

void lcQPreferencesDialog::on_MinifigSettingsBrowseButton_clicked()
{
	QString Result = QFileDialog::getOpenFileName(this, tr("Select Minifig Settings File"), ui->MinifigSettingsEdit->text(), tr("Settings Files (*.ini);;All Files (*.*)"));

	if (!Result.isEmpty())
		ui->MinifigSettingsEdit->setText(QDir::toNativeSeparators(Result));
}

void lcQPreferencesDialog::on_povrayExecutableBrowse_clicked()
{
#ifdef Q_OS_WIN
	QString filter(tr("Executable Files (*.exe);;All Files (*.*)"));
#else
	QString filter(tr("All Files (*.*)"));
#endif

	QString result = QFileDialog::getOpenFileName(this, tr("Select POV-Ray Executable"), ui->povrayExecutable->text(), filter);

	if (!result.isEmpty())
		ui->povrayExecutable->setText(QDir::toNativeSeparators(result));
}

void lcQPreferencesDialog::on_lgeoPathBrowse_clicked()
{
	QString result = QFileDialog::getExistingDirectory(this, tr("Open LGEO Folder"), ui->lgeoPath->text());

	if (!result.isEmpty())
		ui->lgeoPath->setText(QDir::toNativeSeparators(result));
}

/*** LPub3D Mod - Suppress compatible signals warning ***/
void lcQPreferencesDialog::on_ColorTheme_currentIndexChanged(int Index)
{
	if (QMessageBox::question(this, tr("Reset Colors"), tr("Would you like to also reset the interface colors to match the color theme?")) == QMessageBox::Yes)
		mOptions->Preferences.SetInterfaceColors(static_cast<lcColorTheme>(Index));
}
/*** LPub3D Mod end ***/

void lcQPreferencesDialog::ColorButtonClicked()
{
	QObject* Button = sender();
	QString Title;
	quint32* Color = nullptr;
	QColorDialog::ColorDialogOptions DialogOptions;

	if (Button == ui->AxesColorButton)
	{
		Color = &mOptions->Preferences.mAxesColor;
		Title = tr("Select Axes Color");
		DialogOptions = 0;
	}
	else if (Button == ui->OverlayColorButton)
	{
		Color = &mOptions->Preferences.mOverlayColor;
		Title = tr("Select Overlay Color");
		DialogOptions = 0;
	}
	else if (Button == ui->ActiveViewColorButton)
	{
		Color = &mOptions->Preferences.mActiveViewColor;
		Title = tr("Select Active View Color");
		DialogOptions = 0;
	}
	else if (Button == ui->FadeStepsColor)
	{
		Color = &mOptions->Preferences.mFadeStepsColor;
		Title = tr("Select Fade Color");
		DialogOptions = QColorDialog::ShowAlphaChannel;
	}
	else if (Button == ui->HighlightNewPartsColor)
	{
		Color = &mOptions->Preferences.mHighlightNewPartsColor;
		Title = tr("Select Highlight Color");
		DialogOptions = QColorDialog::ShowAlphaChannel;
	}
	else if (Button == ui->gridStudColor)
	{
		Color = &mOptions->Preferences.mGridStudColor;
		Title = tr("Select Grid Stud Color");
		DialogOptions = QColorDialog::ShowAlphaChannel;
	}
	else if (Button == ui->gridLineColor)
	{
		Color = &mOptions->Preferences.mGridLineColor;
		Title = tr("Select Grid Line Color");
		DialogOptions = 0;
	}
	else if (Button == ui->ViewSphereColorButton)
	{
		Color = &mOptions->Preferences.mViewSphereColor;
		Title = tr("Select View Sphere Color");
		DialogOptions = 0;
	}
	else if (Button == ui->ViewSphereTextColorButton)
	{
		Color = &mOptions->Preferences.mViewSphereTextColor;
		Title = tr("Select View Sphere Text Color");
		DialogOptions = 0;
	}
	else if (Button == ui->ViewSphereHighlightColorButton)
	{
		Color = &mOptions->Preferences.mViewSphereHighlightColor;
		Title = tr("Select View Sphere Highlight Color");
		DialogOptions = 0;
	}
	else
		return;

	QColor oldColor = QColor(LC_RGBA_RED(*Color), LC_RGBA_GREEN(*Color), LC_RGBA_BLUE(*Color), LC_RGBA_ALPHA(*Color));
	QColor newColor = QColorDialog::getColor(oldColor, this, Title, DialogOptions);

	if (newColor == oldColor || !newColor.isValid())
		return;

	*Color = LC_RGBA(newColor.red(), newColor.green(), newColor.blue(), newColor.alpha());

	QPixmap pix(12, 12);

	pix.fill(newColor);
	((QToolButton*)Button)->setIcon(pix);
}

void lcQPreferencesDialog::on_studLogo_toggled()
{
   ui->studLogoCombo->setEnabled(ui->studLogo->isChecked());
}

void lcQPreferencesDialog::on_antiAliasing_toggled()
{
	ui->antiAliasingSamples->setEnabled(ui->antiAliasing->isChecked());
}

void lcQPreferencesDialog::on_edgeLines_toggled()
{
	ui->LineWidthSlider->setEnabled(ui->edgeLines->isChecked());
	ui->LineWidthLabel->setEnabled(ui->edgeLines->isChecked());
}

void lcQPreferencesDialog::on_LineWidthSlider_valueChanged()
{
	float Value = mLineWidthRange[0] + static_cast<float>(ui->LineWidthSlider->value()) * mLineWidthGranularity;
	ui->LineWidthLabel->setText(QString::number(Value));
}

void lcQPreferencesDialog::on_FadeSteps_toggled()
{
	ui->FadeStepsColor->setEnabled(ui->FadeSteps->isChecked());
}

void lcQPreferencesDialog::on_HighlightNewParts_toggled()
{
	ui->HighlightNewPartsColor->setEnabled(ui->HighlightNewParts->isChecked());
}

void lcQPreferencesDialog::on_gridStuds_toggled()
{
	ui->gridStudColor->setEnabled(ui->gridStuds->isChecked());
}

void lcQPreferencesDialog::on_gridLines_toggled()
{
	ui->gridLineColor->setEnabled(ui->gridLines->isChecked());
	ui->gridLineSpacing->setEnabled(ui->gridLines->isChecked());
}

void lcQPreferencesDialog::on_ViewSphereSizeCombo_currentIndexChanged(int Index)
{
	bool Enabled = Index != 0;

	ui->ViewSphereLocationCombo->setEnabled(Enabled);
	ui->ViewSphereColorButton->setEnabled(Enabled);
	ui->ViewSphereTextColorButton->setEnabled(Enabled);
	ui->ViewSphereHighlightColorButton->setEnabled(Enabled);
}

void lcQPreferencesDialog::updateCategories()
{
	QTreeWidgetItem* CategoryItem;
	QTreeWidget* CategoriesTree = ui->categoriesTree;

	CategoriesTree->clear();

	for (int CategoryIndex = 0; CategoryIndex < static_cast<int>(mOptions->Categories.size()); CategoryIndex++)
	{
		CategoryItem = new QTreeWidgetItem(CategoriesTree, QStringList(mOptions->Categories[CategoryIndex].Name));
		CategoryItem->setData(0, CategoryRole, QVariant(CategoryIndex));
	}

	CategoryItem = new QTreeWidgetItem(CategoriesTree, QStringList(tr("Unassigned")));
	CategoryItem->setData(0, CategoryRole, QVariant(-1));
}

void lcQPreferencesDialog::updateParts()
{
	lcPiecesLibrary *Library = lcGetPiecesLibrary();
	QTreeWidget *tree = ui->partsTree;

	tree->clear();

	QList<QTreeWidgetItem*> selectedItems = ui->categoriesTree->selectedItems();

	if (selectedItems.empty())
		return;

	QTreeWidgetItem *categoryItem = selectedItems.first();
	int categoryIndex = categoryItem->data(0, CategoryRole).toInt();

	if (categoryIndex != -1)
	{
		lcArray<PieceInfo*> singleParts, groupedParts;

		Library->GetCategoryEntries(mOptions->Categories[categoryIndex].Keywords.constData(), false, singleParts, groupedParts);

		for (int partIndex = 0; partIndex < singleParts.GetSize(); partIndex++)
		{
			PieceInfo *info = singleParts[partIndex];

			QStringList rowList(info->m_strDescription);
			rowList.append(info->mFileName);

			new QTreeWidgetItem(tree, rowList);
		}
	}
	else
	{
		for (const auto& PartIt : Library->mPieces)
		{
			PieceInfo* Info = PartIt.second;

			for (categoryIndex = 0; categoryIndex < static_cast<int>(mOptions->Categories.size()); categoryIndex++)
			{
				if (Library->PieceInCategory(Info, mOptions->Categories[categoryIndex].Keywords.constData()))
					break;
			}

			if (categoryIndex == static_cast<int>(mOptions->Categories.size()))
			{
				QStringList rowList(Info->m_strDescription);
				rowList.append(Info->mFileName);

				new QTreeWidgetItem(tree, rowList);
			}
		}
	}

	tree->resizeColumnToContents(0);
	tree->resizeColumnToContents(1);
}

void lcQPreferencesDialog::on_newCategory_clicked()
{
	lcLibraryCategory category;

	lcQCategoryDialog dialog(this, &category);
	if (dialog.exec() != QDialog::Accepted)
		return;

	mOptions->CategoriesModified = true;
	mOptions->CategoriesDefault = false;
	mOptions->Categories.emplace_back(std::move(category));

	updateCategories();
	ui->categoriesTree->setCurrentItem(ui->categoriesTree->topLevelItem(static_cast<int>(mOptions->Categories.size()) - 1));
}

void lcQPreferencesDialog::on_editCategory_clicked()
{
	QList<QTreeWidgetItem*> selectedItems = ui->categoriesTree->selectedItems();

	if (selectedItems.empty())
		return;

	QTreeWidgetItem *categoryItem = selectedItems.first();
	int categoryIndex = categoryItem->data(0, CategoryRole).toInt();

	if (categoryIndex == -1)
		return;

	lcQCategoryDialog dialog(this, &mOptions->Categories[categoryIndex]);
	if (dialog.exec() != QDialog::Accepted)
		return;

	mOptions->CategoriesModified = true;
	mOptions->CategoriesDefault = false;

	updateCategories();
	ui->categoriesTree->setCurrentItem(ui->categoriesTree->topLevelItem(categoryIndex));
}

void lcQPreferencesDialog::on_deleteCategory_clicked()
{
	QList<QTreeWidgetItem*> selectedItems = ui->categoriesTree->selectedItems();

	if (selectedItems.empty())
		return;

	QTreeWidgetItem *categoryItem = selectedItems.first();
	int categoryIndex = categoryItem->data(0, CategoryRole).toInt();

	if (categoryIndex == -1)
		return;

	QString question = tr("Are you sure you want to delete the category '%1'?").arg(mOptions->Categories[categoryIndex].Name);
/*** LPub3D Mod - set 3DViewer label ***/
	if (QMessageBox::question(this, "3DViewer", question, QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
		return;
/*** LPub3D Mod end ***/

	mOptions->CategoriesModified = true;
	mOptions->CategoriesDefault = false;
	mOptions->Categories.erase(mOptions->Categories.begin() + categoryIndex);

	updateCategories();
}

void lcQPreferencesDialog::on_importCategories_clicked()
{
	QString FileName = QFileDialog::getOpenFileName(this, tr("Import Categories"), "", tr("Text Files (*.txt);;All Files (*.*)"));

	if (FileName.isEmpty())
		return;

	std::vector<lcLibraryCategory> Categories;
	if (!lcLoadCategories(FileName, Categories))
	{
/*** LPub3D Mod - set 3DViewer label ***/
		QMessageBox::warning(this, "3DViewer", tr("Error loading categories file."));
/*** LPub3D Mod end ***/
		return;
	}

	mOptions->Categories = Categories;
	mOptions->CategoriesModified = true;
	mOptions->CategoriesDefault = false;
}

void lcQPreferencesDialog::on_exportCategories_clicked()
{
	QString FileName = QFileDialog::getSaveFileName(this, tr("Export Categories"), "", tr("Text Files (*.txt);;All Files (*.*)"));

	if (FileName.isEmpty())
		return;

	if (!lcSaveCategories(FileName, mOptions->Categories))
	{
/*** LPub3D Mod - set 3DViewer label ***/
		QMessageBox::warning(this, "3DViewer", tr("Error saving categories file."));
/*** LPub3D Mod end ***/
		return;
	}
}

void lcQPreferencesDialog::on_resetCategories_clicked()
{
/*** LPub3D Mod - set 3DViewer label ***/
	if (QMessageBox::question(this, "3DViewer", tr("Are you sure you want to load the default categories?"), QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
		return;
/*** LPub3D Mod end ***/

	lcResetCategories(mOptions->Categories);

	mOptions->CategoriesModified = true;
	mOptions->CategoriesDefault = true;

	updateCategories();
}

bool lcQPreferencesDialog::eventFilter(QObject *object, QEvent *event)
{
	Q_UNUSED(object);

	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

		int nextKey = keyEvent->key();
		if (nextKey == Qt::Key_Control || nextKey == Qt::Key_Shift || nextKey == Qt::Key_Meta || nextKey == Qt::Key_Alt)
			return true;

		Qt::KeyboardModifiers state = keyEvent->modifiers();
		QString text = QKeySequence(nextKey).toString();
		if ((state & Qt::ShiftModifier) && (text.isEmpty() || !text.at(0).isPrint() || text.at(0).isLetter() || text.at(0).isSpace()))
			nextKey |= Qt::SHIFT;
		if (state & Qt::ControlModifier)
			nextKey |= Qt::CTRL;
		if (state & Qt::MetaModifier)
			nextKey |= Qt::META;
		if (state & Qt::AltModifier)
			nextKey |= Qt::ALT;

		QKeySequence ks(nextKey);
		ui->shortcutEdit->setText(ks.toString(QKeySequence::NativeText));
		keyEvent->accept();

		return true;
	}

	if (event->type() == QEvent::Shortcut || event->type() == QEvent::KeyRelease || event->type() == QEvent::ShortcutOverride)
	{
		event->accept();
		return true;
	}

	return QDialog::eventFilter(object, event);
}

void lcQPreferencesDialog::updateCommandList()
{
	ui->commandList->clear();
	QMap<QString, QTreeWidgetItem*> sections;

	for (int actionIdx = 0; actionIdx < LC_NUM_COMMANDS; actionIdx++)
	{
		if (!gCommands[actionIdx].ID[0])
			continue;

		const QString identifier = tr(gCommands[actionIdx].ID);

		int pos = identifier.indexOf(QLatin1Char('.'));
		int subPos = identifier.indexOf(QLatin1Char('.'), pos + 1);
		if (subPos == -1)
			subPos = pos;

		const QString parentSection = identifier.left(pos);

		if (subPos != pos)
		{
			if (!sections.contains(parentSection))
			{
				QTreeWidgetItem *categoryItem = new QTreeWidgetItem(ui->commandList, QStringList(parentSection));
				QFont f = categoryItem->font(0);
				f.setBold(true);
				categoryItem->setFont(0, f);
				sections.insert(parentSection, categoryItem);
				ui->commandList->expandItem(categoryItem);
			}
		}

		const QString section = identifier.left(subPos);
		const QString subId = identifier.mid(subPos + 1);

		if (!sections.contains(section))
		{
			QTreeWidgetItem *parent = sections[parentSection];
			QTreeWidgetItem *categoryItem;
			QString subSection;

			if (pos != subPos)
				subSection = identifier.mid(pos + 1, subPos - pos - 1);
			else
				subSection = section;

			if (parent)
				categoryItem = new QTreeWidgetItem(parent, QStringList(subSection));
			else
				categoryItem = new QTreeWidgetItem(ui->commandList, QStringList(subSection));

			QFont f = categoryItem->font(0);
			f.setBold(true);
			categoryItem->setFont(0, f);
			sections.insert(section, categoryItem);
			ui->commandList->expandItem(categoryItem);
		}

		QTreeWidgetItem *item = new QTreeWidgetItem;
		QKeySequence sequence(mOptions->KeyboardShortcuts.mShortcuts[actionIdx]);
		item->setText(0, qApp->translate("Menu", gCommands[actionIdx].MenuName).remove('&').remove(QLatin1String("...")));
		item->setText(1, sequence.toString(QKeySequence::NativeText));
		item->setData(0, Qt::UserRole, QVariant::fromValue(actionIdx));

		if (mOptions->KeyboardShortcuts.mShortcuts[actionIdx] != gCommands[actionIdx].DefaultShortcut)
			setShortcutModified(item, true);

		sections[section]->addChild(item);
	}
}

void lcQPreferencesDialog::setShortcutModified(QTreeWidgetItem *treeItem, bool modified)
{
	QFont font = treeItem->font(0);
	font.setItalic(modified);
	treeItem->setFont(0, font);
	font.setBold(modified);
	treeItem->setFont(1, font);
}

void lcQPreferencesDialog::commandChanged(QTreeWidgetItem *current)
{
	if (!current || !current->data(0, Qt::UserRole).isValid())
	{
		ui->shortcutEdit->setText(QString());
		ui->shortcutGroup->setEnabled(false);
		return;
	}

	ui->shortcutGroup->setEnabled(true);

	int shortcutIndex = qvariant_cast<int>(current->data(0, Qt::UserRole));
	QKeySequence key(mOptions->KeyboardShortcuts.mShortcuts[shortcutIndex]);
	ui->shortcutEdit->setText(key.toString(QKeySequence::NativeText));
}

void lcQPreferencesDialog::on_KeyboardFilterEdit_textEdited(const QString& Text)
{
	if (Text.isEmpty())
	{
		std::function<void(QTreeWidgetItem*)> ShowItems = [&ShowItems](QTreeWidgetItem* ParentItem)
		{
			for (int ChildIdx = 0; ChildIdx < ParentItem->childCount(); ChildIdx++)
				ShowItems(ParentItem->child(ChildIdx));

			ParentItem->setHidden(false);
		};

		ShowItems(ui->commandList->invisibleRootItem());
	}
	else
	{
		std::function<bool(QTreeWidgetItem*,bool)> ShowItems = [&ShowItems, &Text](QTreeWidgetItem* ParentItem, bool ForceVisible)
		{
			ForceVisible |= (bool)ParentItem->text(0).contains(Text, Qt::CaseInsensitive) | (bool)ParentItem->text(1).contains(Text, Qt::CaseInsensitive);
			bool Visible = ForceVisible;

			for (int ChildIdx = 0; ChildIdx < ParentItem->childCount(); ChildIdx++)
				Visible |= ShowItems(ParentItem->child(ChildIdx), ForceVisible);

			ParentItem->setHidden(!Visible);

			return Visible;
		};

		ShowItems(ui->commandList->invisibleRootItem(), false);
	}
}

void lcQPreferencesDialog::on_shortcutAssign_clicked()
{
	QTreeWidgetItem *current = ui->commandList->currentItem();

	if (!current || !current->data(0, Qt::UserRole).isValid())
		return;

	int shortcutIndex = qvariant_cast<int>(current->data(0, Qt::UserRole));
	mOptions->KeyboardShortcuts.mShortcuts[shortcutIndex] = ui->shortcutEdit->text();

	current->setText(1, ui->shortcutEdit->text());

	setShortcutModified(current, mOptions->KeyboardShortcuts.mShortcuts[shortcutIndex] != gCommands[shortcutIndex].DefaultShortcut);

	mOptions->KeyboardShortcutsModified = true;
	mOptions->KeyboardShortcutsDefault = false;
}

void lcQPreferencesDialog::on_shortcutRemove_clicked()
{
	ui->shortcutEdit->setText(QString());

	on_shortcutAssign_clicked();
}

void lcQPreferencesDialog::on_shortcutsImport_clicked()
{
	QString FileName = QFileDialog::getOpenFileName(this, tr("Import shortcuts"), "", tr("Text Files (*.txt);;All Files (*.*)"));

	if (FileName.isEmpty())
		return;

	lcKeyboardShortcuts Shortcuts;
	if (!Shortcuts.Load(FileName))
	{
/*** LPub3D Mod - set 3DViewer label ***/
		QMessageBox::warning(this, "3DViewer", tr("Error loading keyboard shortcuts file."));
/*** LPub3D Mod end ***/
		return;
	}

	mOptions->KeyboardShortcuts = Shortcuts;

	mOptions->KeyboardShortcutsModified = true;
	mOptions->KeyboardShortcutsDefault = false;
}

void lcQPreferencesDialog::on_shortcutsExport_clicked()
{
	QString FileName = QFileDialog::getSaveFileName(this, tr("Export shortcuts"), "", tr("Text Files (*.txt);;All Files (*.*)"));

	if (FileName.isEmpty())
		return;

	if (!mOptions->KeyboardShortcuts.Save(FileName))
	{
/*** LPub3D Mod - set 3DViewer label ***/
		QMessageBox::warning(this, "3DViewer", tr("Error saving keyboard shortcuts file."));
/*** LPub3D Mod end ***/
		return;
	}
}

void lcQPreferencesDialog::on_shortcutsReset_clicked()
{
/*** LPub3D Mod - set 3DViewer label ***/
	if (QMessageBox::question(this, "3DViewer", tr("Are you sure you want to load the default keyboard shortcuts?"), QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
		return;
/*** LPub3D Mod end ***/

	mOptions->KeyboardShortcuts.Reset();
	updateCommandList();

	mOptions->KeyboardShortcutsModified = true;
	mOptions->KeyboardShortcutsDefault = true;
}

void lcQPreferencesDialog::UpdateMouseTree()
{
	ui->mouseTree->clear();

	for (int ToolIdx = 0; ToolIdx < LC_NUM_TOOLS; ToolIdx++)
		UpdateMouseTreeItem(ToolIdx);
}

void lcQPreferencesDialog::UpdateMouseTreeItem(int ItemIndex)
{
	auto GetShortcutText = [](Qt::MouseButton Button, Qt::KeyboardModifiers Modifiers)
	{
		QString Shortcut = QKeySequence(Modifiers).toString(QKeySequence::NativeText);

		switch (Button)
		{
		case Qt::LeftButton:
			Shortcut += tr("Left Button");
			break;

#if (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
		case Qt::MiddleButton:
			Shortcut += tr("Middle Button");
			break;
#endif

		case Qt::RightButton:
			Shortcut += tr("Right Button");
			break;

		default:
			Shortcut.clear();
		}
		return Shortcut;
	};

	QString Shortcut1 = GetShortcutText(mOptions->MouseShortcuts.mShortcuts[ItemIndex].Button1, mOptions->MouseShortcuts.mShortcuts[ItemIndex].Modifiers1);
	QString Shortcut2 = GetShortcutText(mOptions->MouseShortcuts.mShortcuts[ItemIndex].Button2, mOptions->MouseShortcuts.mShortcuts[ItemIndex].Modifiers2);

	QTreeWidgetItem* Item = ui->mouseTree->topLevelItem(ItemIndex);

	if (Item)
	{
		Item->setText(1, Shortcut1);
		Item->setText(2, Shortcut2);
	}
	else
		new QTreeWidgetItem(ui->mouseTree, QStringList() << tr(gToolNames[ItemIndex]) << Shortcut1 << Shortcut2);
}

void lcQPreferencesDialog::on_mouseAssign_clicked()
{
	QTreeWidgetItem* Current = ui->mouseTree->currentItem();

	if (!Current)
		return;

	int ButtonIndex = ui->mouseButton->currentIndex();
	Qt::MouseButton Button = Qt::NoButton;
	Qt::KeyboardModifiers Modifiers = Qt::NoModifier;

	if (ButtonIndex)
	{
		switch (ButtonIndex)
		{
		case 1:
			Button = Qt::LeftButton;
			break;

#if (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
		case 2:
			Button = Qt::MiddleButton;
			break;
#endif

		case 3:
			Button = Qt::RightButton;
			break;
		}

		if (ui->mouseControl->isChecked())
			Modifiers |= Qt::ControlModifier;

		if (ui->mouseShift->isChecked())
			Modifiers |= Qt::ShiftModifier;

		if (ui->mouseAlt->isChecked())
			Modifiers |= Qt::AltModifier;

		for (int ToolIdx = 0; ToolIdx < LC_NUM_TOOLS; ToolIdx++)
		{
			if (ToolIdx == ButtonIndex)
				continue;

			if (mOptions->MouseShortcuts.mShortcuts[ToolIdx].Button2 == Button && mOptions->MouseShortcuts.mShortcuts[ToolIdx].Modifiers2 == Modifiers)
			{
				if (QMessageBox::question(this, tr("Override Shortcut"), tr("This shortcut is already assigned to '%1', do you want to replace it?").arg(tr(gToolNames[ToolIdx])), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
					return;

				mOptions->MouseShortcuts.mShortcuts[ToolIdx].Button2 = Qt::NoButton;
				mOptions->MouseShortcuts.mShortcuts[ToolIdx].Modifiers2 = Qt::NoModifier;
			}

			if (mOptions->MouseShortcuts.mShortcuts[ToolIdx].Button1 == Button && mOptions->MouseShortcuts.mShortcuts[ToolIdx].Modifiers1 == Modifiers)
			{
				if (QMessageBox::question(this, tr("Override Shortcut"), tr("This shortcut is already assigned to '%1', do you want to replace it?").arg(tr(gToolNames[ToolIdx])), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
					return;

				mOptions->MouseShortcuts.mShortcuts[ToolIdx].Button1 = mOptions->MouseShortcuts.mShortcuts[ToolIdx].Button2;
				mOptions->MouseShortcuts.mShortcuts[ToolIdx].Modifiers1 = mOptions->MouseShortcuts.mShortcuts[ToolIdx].Modifiers2;
				mOptions->MouseShortcuts.mShortcuts[ToolIdx].Button2 = Qt::NoButton;
				mOptions->MouseShortcuts.mShortcuts[ToolIdx].Modifiers2 = Qt::NoModifier;
			}
		}
	}

	int ItemIndex = ui->mouseTree->indexOfTopLevelItem(Current);
	mOptions->MouseShortcuts.mShortcuts[ItemIndex].Button2 = mOptions->MouseShortcuts.mShortcuts[ItemIndex].Button1;
	mOptions->MouseShortcuts.mShortcuts[ItemIndex].Modifiers2 = mOptions->MouseShortcuts.mShortcuts[ItemIndex].Modifiers1;
	mOptions->MouseShortcuts.mShortcuts[ItemIndex].Button1 = Button;
	mOptions->MouseShortcuts.mShortcuts[ItemIndex].Modifiers1 = Modifiers;

	mOptions->MouseShortcutsModified = true;
	mOptions->MouseShortcutsDefault = false;

	UpdateMouseTreeItem(ItemIndex);
}

void lcQPreferencesDialog::on_mouseRemove_clicked()
{
	QTreeWidgetItem* Current = ui->mouseTree->currentItem();

	if (!Current)
		return;

	int ItemIndex = ui->mouseTree->indexOfTopLevelItem(Current);
	mOptions->MouseShortcuts.mShortcuts[ItemIndex].Button1 = mOptions->MouseShortcuts.mShortcuts[ItemIndex].Button2;
	mOptions->MouseShortcuts.mShortcuts[ItemIndex].Modifiers1 = mOptions->MouseShortcuts.mShortcuts[ItemIndex].Modifiers2;
	mOptions->MouseShortcuts.mShortcuts[ItemIndex].Button2 = Qt::NoButton;
	mOptions->MouseShortcuts.mShortcuts[ItemIndex].Modifiers2 = Qt::NoModifier;

	mOptions->MouseShortcutsModified = true;
	mOptions->MouseShortcutsDefault = false;

	UpdateMouseTreeItem(ItemIndex);
	MouseTreeItemChanged(Current);
}

void lcQPreferencesDialog::on_mouseReset_clicked()
{
/*** LPub3D Mod - set 3DViewer label ***/
	if (QMessageBox::question(this, "3DViewer", tr("Are you sure you want to load the default mouse shortcuts?"), QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
		return;
/*** LPub3D Mod end ***/

	mOptions->MouseShortcuts.Reset();
	UpdateMouseTree();

	mOptions->MouseShortcutsModified = true;
	mOptions->MouseShortcutsDefault = true;
}

void lcQPreferencesDialog::MouseTreeItemChanged(QTreeWidgetItem* Current)
{
	if (!Current)
	{
		ui->MouseShortcutGroup->setEnabled(false);
		return;
	}

	ui->MouseShortcutGroup->setEnabled(true);

	int ToolIndex = ui->mouseTree->indexOfTopLevelItem(Current);

	Qt::MouseButton Button = mOptions->MouseShortcuts.mShortcuts[ToolIndex].Button1;

	switch (Button)
	{
	case Qt::LeftButton:
		ui->mouseButton->setCurrentIndex(1);
		break;

#if (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
	case Qt::MiddleButton:
		ui->mouseButton->setCurrentIndex(2);
		break;
#endif

	case Qt::RightButton:
		ui->mouseButton->setCurrentIndex(3);
		break;

	default:
		ui->mouseButton->setCurrentIndex(0);
		break;
	}

	Qt::KeyboardModifiers Modifiers = mOptions->MouseShortcuts.mShortcuts[ToolIndex].Modifiers1;
	ui->mouseControl->setChecked((Modifiers & Qt::ControlModifier) != 0);
	ui->mouseShift->setChecked((Modifiers & Qt::ShiftModifier) != 0);
	ui->mouseAlt->setChecked((Modifiers & Qt::AltModifier) != 0);
}

/*** LPub3D Mod - Native Renderer settings ***/
void lcQPreferencesDialog::on_ViewpointsCombo_currentIndexChanged(int index)
{
	if (index <=6)
		ui->ProjectionCombo->setCurrentIndex(2); // Default
}
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Update Default Camera ***/
void lcQPreferencesDialog::on_cameraDefaultDistanceFactor_valueChanged(double value)
{
	qreal cdp = ui->cameraDefaultPosition->value();
	ui->cameraDistanceFactor->setText(QString::number(cdp / -value));
}

void lcQPreferencesDialog::on_cameraDefaultPosition_valueChanged(double value)
{
	qreal ddf = ui->cameraDefaultDistanceFactor->value();
	ui->cameraDistanceFactor->setText(QString::number(value / -ddf));
}

void lcQPreferencesDialog::cameraPropertyReset()
{
	if (sender() == ui->resetDefaultDistanceFactor) {
		ui->cameraDefaultDistanceFactor->setValue(qreal(5.0));
	} else if (sender() == ui->resetDefaultPosition) {
		ui->cameraDefaultPosition->setValue(qreal(1250.0));
	} else if (sender() == ui->resetFoV) {
		ui->cameraFoV->setValue(qreal(30.0));
	} else if (sender() == ui->resetNearPlane) {
		ui->cameraNearPlane->setValue(qreal(25));
	} else if (sender() == ui->resetFarPlane) {
		ui->cameraFarPlane->setValue(qreal(50000.0));
	}
}
/*** LPub3D Mod end ***/
