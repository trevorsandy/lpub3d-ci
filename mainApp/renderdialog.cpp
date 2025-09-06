/****************************************************************************
**
** Copyright (C) 2019 - 2025 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public License (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "renderdialog.h"
#include "ui_renderdialog.h"
#include "lpub_preferences.h"
#include "render.h"
#include "paths.h"
#include "lpub.h"
#include "step.h"
#include "metagui.h"
#include "blenderpreferences.h"
#include "parmswindow.h"
#include "commonmenus.h"

#ifndef LC_DISABLE_RENDER_DIALOG

#define LP3D_CA 0.01
#define LP3D_CDF 1.0

#ifdef Q_OS_WIN
#include <TlHelp32.h>
#endif

void RenderPreviewWidget::resizeEvent(QResizeEvent* Event)
{
    mScaledImage = QImage();

    QWidget::resizeEvent(Event);
}

void RenderPreviewWidget::paintEvent(QPaintEvent* PaintEvent)
{
    Q_UNUSED(PaintEvent);

    QPainter Painter(this);

    if (!mImage.isNull())
    {
        QSize Size = size();

        if (mScaledImage.isNull())
            mScaledImage = mImage.scaled(Size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        Painter.drawImage((Size.width() - mScaledImage.width()) / 2, (Size.height() - mScaledImage.height()) / 2, mScaledImage);
    }
    else
        Painter.fillRect(rect(), Qt::transparent);
}

RenderDialog::RenderDialog(QWidget* Parent, int renderType, int importOnly)
    : QDialog(Parent),
      mRenderType(renderType),
      mOpenInBlender(importOnly),
      ui(new Ui::RenderDialog)
{
    ui->setupUi(this);

    mProcess = nullptr;

    mWidth      = RENDER_DEFAULT_WIDTH;
    mHeight     = RENDER_DEFAULT_HEIGHT;

    mHaveKeys   = false;
    mBannerLoaded = false;

    mViewerStepKey = lpub->viewerStepKey;

    static QRegularExpression valRx("^.*\\.png$", QRegularExpression::CaseInsensitiveOption);

    ui->OutputEdit->setText(Render::getRenderImageFile(renderType));
    ui->OutputEdit->setValidator(new QRegularExpressionValidator(valRx));
    ui->RenderOutputButton->setEnabled(false);

    mCsiKeyList = QString(lpub->ldrawFile.getViewerConfigKey(mViewerStepKey).split(";").last()).split("_");

    Step *currentStep = lpub->currentStep;

    bool showInput = mRenderType == BLENDER_RENDER || mRenderType == POVRAY_RENDER;

    ui->InputLabel->setVisible(showInput);
    ui->InputEdit->setVisible(showInput);
    ui->InputLine->setVisible(showInput);
    ui->InputBrowseButton->setVisible(showInput);
    ui->InputGenerateCheck->setVisible(showInput);

    // If the render dialog is launched from a blank/bom page, disable the controls
    if (mCsiKeyList.isEmpty() || (mCsiKeyList.size() == 1 && mCsiKeyList.at(0) == "")) {
        mCsiKeyList.clear();
        ui->OutputEdit->clear();
        ui->OutputEdit->setEnabled(false);
        ui->RenderSettingsButton->setEnabled(false);
        ui->RenderButton->setEnabled(false);
        ui->OutputBrowseButton->setEnabled(false);
        ui->InputEdit->clear();
    } else {
        mHaveKeys = true;
    }

    if (mHaveKeys && currentStep) {
        bool const displayModel = currentStep->displayStep >= DT_MODEL_DEFAULT || currentStep->subModel.viewerSubmodel;
        mMn = tr("STEP %1").arg(currentStep->stepNumber.number);
        if (displayModel) {
            mMn = currentStep->topOfStep().modelName;
            mMn = mMn.replace(mMn.indexOf(mMn.at(0)),1,mMn.at(0).toUpper());
        }
    }

    QString labelMessage = tr("Render image%1").arg(mMn.isEmpty() ? "" : tr(" for <b>%1</b>").arg(mMn));

    ui->RenderLabel->setText(labelMessage);

    resetOutputAct = ui->OutputEdit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
    resetOutputAct->setText(tr("Reset"));
    resetOutputAct->setEnabled(false);
    connect(ui->OutputEdit, SIGNAL(textEdited(const QString&)),
            this,           SLOT(  enableReset(const QString&)));
    connect(resetOutputAct, SIGNAL(triggered()),
            this,           SLOT(  resetEdit()));

    if (showInput) {
        ui->InputGenerateCheck->setToolTip(tr("Generate LDraw input file (csi_blender.ldr)%1")
                                               .arg(tr(" from %1").arg(mMn)));
        ui->InputEdit->setText(Render::getRenderModelFile(mRenderType, false/*save current model*/));
        ui->InputEdit->setEnabled(false);
        ui->InputBrowseButton->setEnabled(false);
        ui->InputBrowseButton->setToolTip(tr("Select LDraw model file - Uncheck Generate Model to enable."));
        resetInputAct = ui->InputEdit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
        resetInputAct->setText(tr("Reset"));
        resetInputAct->setEnabled(false);
        connect(ui->InputEdit,  SIGNAL(textEdited(const QString&)),
                this,           SLOT(  enableReset(const QString&)));
        connect(resetInputAct,  SIGNAL(triggered()),
                this,           SLOT(  resetEdit()));

        connect(ui->InputEdit,  SIGNAL(editingFinished()),
                this,           SLOT(validateInput()));
        mPopulatedFile = false;
    }

    if (mRenderType == POVRAY_RENDER) {

        setWindowTitle(tr("POV-Ray Image Render"));

        setWhatsThis(lpubWT(WT_DIALOG_POVRAY_RENDER,windowTitle()));

        ui->RenderSettingsButton->setToolTip(tr("POV-Ray render settings"));

        ui->RenderButton->setEnabled(mHaveKeys);

        ui->RenderButton->setToolTip(tr("Render LDraw Model"));

        mOutputBuffer    = nullptr;

        mQuality         = Preferences::povrayRenderQuality;

        mTransBackground = true;

        QImage Image(QPixmap(":/resources/1024px-Povray_logo_sphere.png").toImage());
        Image = Image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        ui->preview->SetImage(Image.scaled(mWidth, mHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));

        setMinimumSize(100, 100);

    } else if (mRenderType == BLENDER_RENDER) {

        setWindowTitle(mOpenInBlender ? tr("Blender LDraw Import") : tr("Blender Image Render"));

        setWhatsThis(lpubWT(WT_DIALOG_BLENDER_RENDER,windowTitle()));

        bool blenderConfigured = !Preferences::blenderImportModule.isEmpty();

        QString const blenderDir = QString("%1/Blender").arg(Preferences::lpub3d3rdPartyConfigDir);
        if (!QDir(QString("%1/addons/%2").arg(blenderDir, BLENDER_RENDER_ADDON_FOLDER)).isReadable()) {
            blenderConfigured = false;
            Preferences::setBlenderImportModule(QString());
        }

        if (blenderConfigured)
            mImportModule = Preferences::blenderImportModule == QLatin1String("TN")
                                ? tr("LDraw Import TN")
                                : tr("LDraw Import MM");

        ui->RenderButton->setEnabled(blenderConfigured && mHaveKeys);

        ui->RenderButton->setToolTip(blenderConfigured
                                        ? tr("Render LDraw Model")
                                        : tr("Blender not configured. Use Settings... to configure."));

        if (mOpenInBlender) {
            labelMessage = tr("Open%1 in Blender using %2")
                              .arg(mMn.isEmpty() ? "" : QString(" <b>%1</b>").arg(mMn), mImportModule);

            ui->InputLabel->setMinimumWidth(0);
            ui->InputBrowseButton->setMinimumWidth(0);
            ui->InputGenerateCheck->setMinimumWidth(0);

            ui->RenderButton->setText(tr("Open in Blender"));
            ui->RenderButton->setFixedWidth(ui->RenderButton->sizeHint().width() + 20);

            if (blenderConfigured)
                ui->RenderButton->setToolTip(tr("Import and open LDraw model in Blender"));

            ui->RenderLabel->setText(labelMessage);
            ui->RenderLabel->setAlignment(Qt::AlignTrailing | Qt::AlignVCenter);

            ui->RenderSettingsButton->setToolTip(tr("Blender import settings"));
            ui->RenderSettingsButton->setFixedWidth(ui->RenderButton->width());
            ui->outputLayout->setAlignment(Qt::AlignTrailing | Qt::AlignVCenter);

            ui->OutputLabel->hide();
            ui->OutputEdit->hide();
            ui->RenderProgress->hide();
            ui->OutputBrowseButton->hide();
            ui->RenderOutputButton->hide();
            ui->OutputLine->hide();
            ui->RenderLine->hide();

            adjustSize();
            setMinimumWidth(ui->preview->geometry().width());
            ui->preview->hide();
        } else {
            int scaledWidth = RENDER_DEFAULT_WIDTH;
            int scaledHeight = RENDER_DEFAULT_HEIGHT;

            ui->RenderSettingsButton->setToolTip(tr("Blender render settings"));

            if (QFileInfo::exists(Preferences::blenderLDrawConfigFile))
            {
                QSettings Settings(Preferences::blenderLDrawConfigFile, QSettings::IniFormat);
                if (Settings.value(QString("%1/cropImage").arg(IMPORTLDRAW), QString()).toBool())
                {
                    scaledWidth  = gui->GetImageWidth();
                    scaledHeight = gui->GetImageHeight();
                }
            }

            QImage Image(QPixmap(":/resources/blenderlogo1280x720.png").toImage());
            Image = Image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
            ui->preview->SetImage(Image.scaled(scaledWidth, scaledHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));

            connect(&mUpdateTimer, SIGNAL(timeout()), this, SLOT(UpdateElapsedTime()));
        }
    }

    connect(&mUpdateTimer, SIGNAL(timeout()), this, SLOT(Update()));

    mUpdateTimer.start(500);

    setSizeGripEnabled(true);
}

RenderDialog::~RenderDialog()
{
    delete ui;
}

void RenderDialog::on_RenderSettingsButton_clicked()
{
    if (mRenderType == POVRAY_RENDER) {

        POVRayRenderDialogGui *povrayRenderDialogGui =
            new POVRayRenderDialogGui(this);
        povrayRenderDialogGui->getRenderSettings(
                    mCsiKeyList,
                    mWidth,
                    mHeight,
                    mQuality,
                    mTransBackground);

    } else if (mRenderType == BLENDER_RENDER) {

        double renderPercentage = mCsiKeyList.at(K_MODELSCALE).toDouble();

        bool ok = BlenderPreferencesDialog::getBlenderPreferences(
                    mWidth,
                    mHeight,
                    renderPercentage,
                    false /*document model*/,
                    this);

        lpub->getAct("LightGroupAct.4")->setEnabled(ok);

        if (ok)
        {
            mCsiKeyList[K_MODELSCALE] = QString::number(renderPercentage);

            if (mOpenInBlender) {
                mImportModule = Preferences::blenderImportModule == QLatin1String("TN")
                        ? tr("LDraw Import TN")
                        : tr("LDraw Import MM");

                QString labelMessage = tr("Open%1 in Blender using %2")
                                          .arg(mMn.isEmpty() ? "" : QString(" <b>%1</b>").arg(mMn), mImportModule);
                ui->RenderLabel->setText(labelMessage);
                ui->RenderLabel->setAlignment(Qt::AlignTrailing | Qt::AlignVCenter);
            }

            ui->RenderButton->setEnabled(true);
            ui->RenderButton->setToolTip(tr("Import and open LDraw model in Blender"));
        } else
            ui->RenderButton->setToolTip(tr("Blender not configured. Click 'Settings' to configure."));
    }
}

void RenderDialog::on_RenderButton_clicked()
{
    if (mProcess)
    {
        PromptCancel();
        return;
    }

    LPub::loadBanner(mRenderType);
    mBannerLoaded = true;

    ui->RenderOutputButton->setEnabled(false);
    mPreviewWidth  = ui->preview->width();
    mPreviewHeight = ui->preview->height();

    mRenderTime.start();

    if (mRenderType == POVRAY_RENDER)
        RenderPOVRay();
    else
        RenderBlender();
}

void RenderDialog::RenderPOVRay()
{
    QFuture<QStringList> getFileContent = QtConcurrent::run([&]()
    {
        QStringList contents, fileContents;
        QFile file(mModelFile);
        if (!file.open(QFile::ReadOnly | QFile::Text)) {
            emit gui->messageSig(LOG_ERROR, tr("Cannot read external file %1<br>%2")
                                               .arg(mModelFile, file.errorString()));
            return fileContents;
        }

        QTextStream in(&file);
        while ( ! in.atEnd()) {
            QString sLine = in.readLine(0);
            contents << sLine.trimmed();
        }
        file.close();

        fileContents = gui->getModelFileContent(&contents, QFileInfo(mModelFile).fileName());

        return fileContents;
    });

    QString message;

    lpub->getAct("povrayRenderAct.4")->setEnabled(false);

    ui->RenderLabel->setText(tr("Generating POV-Ray scene file..."));

    QImage Image(mPreviewWidth, mPreviewHeight, QImage::Format_RGB32);
    Image.fill(QColor(255, 255, 255));
    ui->preview->SetImage(Image);

    QApplication::processEvents();

    mModelFile = Render::getRenderModelFile(mRenderType);

    QString errorEncountered;
    QStringList csiParts = gui->getViewerStepUnrotatedContents(mViewerStepKey);
    if (csiParts.isEmpty())
    {
        if (!ui->InputGenerateCheck->isChecked() && QFileInfo::exists(mModelFile)) {
            QApplication::setOverrideCursor(Qt::WaitCursor);
            while (!getFileContent.isFinished())
                QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            QGuiApplication::restoreOverrideCursor();
            csiParts = getFileContent.result();
            if (csiParts.isEmpty())
                errorEncountered = tr("Could not get LDraw content for %1.").arg(mModelFile);
        } else {
            Where here;
            int stepNumber = 0;
            QString model;
            if (lpub->extractStepKey(here, stepNumber))
                model = tr(" for %1%2")
                           .arg(here.modelName,
                                stepNumber ? QString(" STEP %1").arg(stepNumber) : "");
            errorEncountered = tr("Could not find LDraw content%1.").arg(model);
        }

        if (!errorEncountered.isEmpty()) {
            ui->RenderLabel->setText(tr("Error encountered."));
            emit gui->messageSig(LOG_ERROR, errorEncountered);
            lpub->getAct("povrayRenderAct.4")->setEnabled(true);
            return;
        }
    }

    const QStringList povrayLights = gui->get3DViewerPOVLightList();

    // Camera angle keys
    QString caKey =
            mCsiKeyList.at(K_LATITUDE)+" "+          // latitude
            mCsiKeyList.at(K_LONGITUDE);             // longitude

    // Target keys
    QString mKey =
            mCsiKeyList.at(K_TARGETX)+" "+           // target[X]
            mCsiKeyList.at(K_TARGETY)+" "+           // target[Y]
            mCsiKeyList.at(K_TARGETZ);               // target[Z]

    // Rotstep keys
    QString rsKey =
            mCsiKeyList.at(K_ROTSX)+" "+             // rots[X]
            mCsiKeyList.at(K_ROTSY)+" "+             // rots[Y]
            mCsiKeyList.at(K_ROTSZ)+" "+             // rots[Z]
            mCsiKeyList.at(K_ROTSTYPE);              // type [REL|ABS]

    // RotateParts #1 - 5 parms, rotate parts for ldvExport - apply rotstep and camera angles
    if ((Render::rotatePartsRD(csiParts, mModelFile, rsKey, caKey, Options::CSI)) != 0) {
        lpub->getAct("povrayRenderAct.4")->setEnabled(true);
        ui->RenderLabel->setText(tr("Error encountered."));
        return ;
    }

    // Camera distance keys
    QString cdKey =
            QString::number(mWidth)+" "+             // imageWidth
            QString::number(mHeight)+" "+            // imageHeight
            mCsiKeyList.at(K_MODELSCALE)+" "+        // modelScale
            mCsiKeyList.at(K_RESOLUTION)+" "+        // resolution
            mCsiKeyList.at(K_RESOLUTIONTYPE);        // resolutionType (DPI,DPCM)

    /* perspective projection settings */
    bool pp    = Preferences::perspectiveProjection;

    /* set camera angles */
    bool noCA  = Preferences::applyCALocally;

    /* determine camera distance - use LDView POV file generation values */
    int cd = int((double(Render::getPovrayRenderCameraDistance(cdKey))*0.455)*1700/1000);

    /* set LDV arguments */
    QString CA = QString("-ca%1") .arg(0.01);        // Effectively defaults to orthographic projection.
    QString cg = QString("-cg%1,%2,%3")
            .arg(noCA ? double(0.0f) : mCsiKeyList.at(K_LATITUDE).toDouble())
            .arg(noCA ? double(0.0f) : mCsiKeyList.at(K_LONGITUDE).toDouble())
            .arg(QString::number(pp ? cd * LP3D_CDF : cd,'f',0));

    QString m  = mKey == "0 0 0" ? QString() : mKey.trimmed();
    QString w  = QString("-SaveWidth=%1")  .arg(mWidth);
    QString h  = QString("-SaveHeight=%1") .arg(mHeight);
    QString f  = QString("-ExportFile=%1") .arg(GetPOVFileName());
    QString l  = QString("-LDrawDir=%1") .arg(QDir::toNativeSeparators(Preferences::ldrawLibPath));

    QStringList Arguments;
    Arguments << CA;
    Arguments << cg;

    /*
        K_STEPNUMBER = 0,  // 0  not used
        K_IMAGEWIDTH,      // 1  not used
        K_RESOLUTION,      // 2
        K_RESOLUTIONTYPE,  // 3
        K_MODELSCALE,      // 4
        K_FOV,             // 5  not used
        K_LATITUDE,        // 6
        K_LONGITUDE,       // 7
        K_TARGETX,         // 8
        K_TARGETY,         // 9
        K_TARGETZ,         // 10
        K_ROTSX,           // 11
        K_ROTSY,           // 12
        K_ROTSZ,           // 13
        K_ROTSTYPE         // 14
    */

    // replace CA with FOV
    if (pp) {
        QString df = QString("-FOV=%1").arg(mCsiKeyList.at(K_FOV).toDouble());
        Arguments.replace(Arguments.indexOf(CA),df);
    }

    // Set alternate target position or use specified image size
    if (!m.isEmpty()) {
        Arguments.removeAt(Arguments.indexOf(cg)); // remove camera globe
        QString dz = QString("-DefaultZoom=%1")
                .arg(mCsiKeyList.at(K_MODELSCALE).toDouble());
        QString dl = QString("-DefaultLatLong=%1,%2")
                .arg(noCA ? double(0.0f) : mCsiKeyList.at(K_LATITUDE).toDouble())
                .arg(noCA ? double(0.0f) : mCsiKeyList.at(K_LONGITUDE).toDouble());
        Render::addArgument(Arguments, dz, "-DefaultZoom");
        Render::addArgument(Arguments, dl, "-DefaultLatLong");
        Arguments << m;
    }

    Arguments << w;
    Arguments << h;
    Arguments << f;
    Arguments << l;

    if (povrayLights.size() && povrayLights[0] != "")
    {
        QString p = QString("-PovExporter/PovLights=\"%1\"").arg(povrayLights.join(";"));
        Arguments << p;
    }

    if (!Preferences::altLDConfigPath.isEmpty()) {
        Arguments << "-LDConfig=" + Preferences::altLDConfigPath;
    }

    Arguments << QDir::toNativeSeparators(mModelFile);

    message = tr("LDV CSI POV File Generation Arguments: %1 %2")
                 .arg(Preferences::ldviewExe, Arguments.join(" "));
    emit gui->messageSig(LOG_INFO, message);

    // generate POV file
    if (!Render::doLDVCommand(Arguments,POVRAY_RENDER)) {
        lpub->getAct("povrayRenderAct.4")->setEnabled(true);
        ui->RenderLabel->setText(tr("Error encountered."));
        return ;
    } else {
        ui->RenderLabel->setText(tr("Rendering POV-Ray scene..."));
    }

    message = tr("LDV POV file %1 generated. %2").arg(GetPOVFileName(), Gui::elapsedTime(mRenderTime.elapsed()));
    emit gui->messageSig(LOG_INFO, message);

    /* set POV-Ray arguments */
    Arguments.clear();

    if (mTransBackground) {
        Arguments << QString("+UA");
    }

    Arguments << QString("+D");
    Arguments << QString("-O-");
    Arguments << Render::getPovrayRenderQuality(mQuality);
    Arguments << QString("+W%1").arg(mWidth);
    Arguments << QString("+H%1").arg(mHeight);
    Arguments << QString("+I\"%1\"").arg(Render::fixupDirname(GetPOVFileName()));
    Arguments << QString("+SM\"%1\"").arg(Render::fixupDirname(GetOutputFileName()));

    bool hasSTL       = Preferences::lgeoStlLib;
    bool hasLGEO      = Preferences::lgeoPath != "";
    bool hasPOVRayIni = Preferences::povrayIniPath != "";
    bool hasPOVRayInc = Preferences::povrayIncPath != "";

    if(hasPOVRayInc) {
        QString povinc = QString("+L\"%1\"").arg(Render::fixupDirname(QDir::toNativeSeparators(Preferences::povrayIncPath)));
        Arguments << povinc;
    }
    if(hasPOVRayIni) {
        QString povini = QString("+L\"%1\"").arg(Render::fixupDirname(QDir::toNativeSeparators(Preferences::povrayIniPath)));
        Arguments << povini;
    }
    if(hasLGEO) {
        QString lgeoLg = QString("+L\"%1\"").arg(Render::fixupDirname(QDir::toNativeSeparators(Preferences::lgeoPath + "/lg")));
        QString lgeoAr = QString("+L\"%1\"").arg(Render::fixupDirname(QDir::toNativeSeparators(Preferences::lgeoPath + "/ar")));
        Arguments << lgeoLg;
        Arguments << lgeoAr;
        if (hasSTL) {
            QString lgeoStl = QString("+L\"%1\"").arg(Render::fixupDirname(QDir::toNativeSeparators(Preferences::lgeoPath + "/stl")));
            Arguments << lgeoStl;
        }
    }

    message = tr("POV-Ray CSI Render Arguments: %1 %2").arg(Preferences::povrayExe, Arguments.join(" "));
    emit gui->messageSig(LOG_INFO, message);

    QStringList povrayEnv = Render::splitParms(lpub->page.meta.LPub.assem.povrayEnvVars.value());
    povrayEnv.prepend("POV_IGNORE_SYSCONF_MSG=1");
    emit gui->messageSig(LOG_INFO, QObject::tr("POVRay Model Renderer Environment Variables: %1")
                                               .arg(povrayEnv.join(" ")));

    RenderProcess *Process = new RenderProcess(this);
    povrayEnv << QProcess::systemEnvironment();
    Process->setEnvironment(povrayEnv);
    Process->setWorkingDirectory(QDir::currentPath() + QDir::separator() + Paths::tmpDir); // pov win console app will not write to dir different from cwd or source file dir
    Process->setStandardErrorFile(GetLogFileName(false/*stdOut*/));
    Process->start(Preferences::povrayExe, Arguments);

    mImage = QImage(mWidth, mHeight, QImage::Format_ARGB32);
    mImage.fill(Qt::transparent);
    ui->preview->SetImage(mImage);

    if (Process->waitForStarted())
    {
        mProcess = Process;
        ui->RenderButton->setText(tr("Cancel"));
        ui->RenderProgress->setValue(ui->RenderProgress->minimum());
        mStdOutList.clear();
    }
    else
    {
        delete Process;
        lpub->getAct("povrayRenderAct.4")->setEnabled(true);
        message = tr("Error starting POV-Ray.");
        emit gui->messageSig(LOG_ERROR, message);
        ui->RenderLabel->setText(tr("Error encountered."));
        CloseProcess();
    }
}

void RenderDialog::RenderBlender()
{
    QString const option = mOpenInBlender ? tr("import") : tr("render");

    if (!mPopulatedFile && ui->InputGenerateCheck->isChecked()) {

        ui->RenderLabel->setText(tr("Saving Blender %1 model...").arg(option));

        mModelFile = Render::getRenderModelFile(mRenderType);
    }

    mBlendProgValue = 0;
    mBlendProgMax   = 0;

    if (!QFileInfo(Preferences::blenderLDrawConfigFile).isReadable() &&
            !Preferences::blenderImportModule.isEmpty())
        BlenderPreferences::saveSettings();

    QString defaultBlendFile = QString("%1/Blender/config/%2")
                                       .arg(Preferences::lpub3d3rdPartyConfigDir,
                                            VER_BLENDER_DEFAULT_BLEND_FILE);
    bool searchCustomDir = Preferences::enableFadeSteps || Preferences::enableHighlightStep;
    int renderPercentage = mHaveKeys ? qRound(mCsiKeyList.at(K_MODELSCALE).toDouble() * 100) : 100;

    QString message;
    QStringList Arguments;
    QString pythonExpression;
    pythonExpression.append(QString("\"import bpy; bpy.ops.render_scene.lpub3d_render_ldraw("
                                    "'EXEC_DEFAULT', "
                                    "resolution_width=%1, resolution_height=%2, "
                                    "render_percentage=%3, model_file=r'%4', "
                                    "image_file=r'%5'")
                                    .arg(mWidth).arg(mHeight)
                                    .arg(renderPercentage)
                                    .arg(QDir::toNativeSeparators(mModelFile).replace("\\","\\\\"),
                                         QDir::toNativeSeparators(ui->OutputEdit->text()).replace("\\","\\\\")));
    if (Preferences::blenderImportModule == QLatin1String("MM"))
        pythonExpression.append(", use_ldraw_import_mm=True");
    if (searchCustomDir)
        pythonExpression.append(", search_additional_paths=True");
    if (mOpenInBlender) {
        pythonExpression.append(", import_only=True");

        Arguments << QLatin1String("--window-geometry");
        Arguments << QLatin1String("200 100 1440 900");
    } else {
        Arguments << QLatin1String("--background");
        lpub->getAct("blenderRenderAct.4")->setEnabled(false);
    }

    pythonExpression.append(", cli_render=True)\"");

    if (Preferences::defaultBlendFile)
        Arguments << QDir::toNativeSeparators(defaultBlendFile);
    Arguments << QString("--python-expr");
    Arguments << pythonExpression;

    QString scriptDir, scriptName, scriptCommand, shellProgram;

    QStringList configPathList = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
    scriptDir = configPathList.first();

    QFile script;
    if (QFileInfo::exists(scriptDir)) {

#ifdef Q_OS_WIN
        scriptName =  QLatin1String("render_ldraw_model.bat");
#else
        scriptName =  QLatin1String("render_ldraw_model.sh");
#endif
        scriptCommand = QString("\"%1\" %2").arg(Preferences::blenderExe, Arguments.join(" "));

        message = tr("Blender %1 command: %2").arg(option, scriptCommand);
#ifdef QT_DEBUG_MODE
        qDebug() << qPrintable(message);
#else
        emit gui->messageSig(LOG_INFO, message);
#endif
        if (mOpenInBlender)
            scriptCommand.append(QString(" > %1").arg(QDir::toNativeSeparators(GetLogFileName(true/*stdOut*/))));

        script.setFileName(QString("%1/%2").arg(scriptDir, scriptName));
        if (script.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&script);
#ifdef Q_OS_WIN
            stream << QLatin1String("@ECHO OFF &SETLOCAL") << lpub_endl;
#else
            stream << QLatin1String("#!/bin/bash") << lpub_endl;
#endif
            stream << scriptCommand << lpub_endl;
            script.close();
            message = tr("Blender %1 script: %2").arg(option, QDir::toNativeSeparators(script.fileName()));
#ifdef QT_DEBUG_MODE
            qDebug() << qPrintable(message);
#else
            emit gui->messageSig(LOG_INFO, message);
#endif
        } else {
            emit gui->messageSig(LOG_ERROR, tr("Cannot write Blender render script file [%1] %2.")
                                               .arg(script.fileName(),
                                               script.errorString()));
            lpub->getAct("blenderRenderAct.4")->setEnabled(true);
            ui->RenderLabel->setText(tr("Error encountered."));
            return;
        }
    } else {
        emit gui->messageSig(LOG_ERROR, tr("Cannot create Blender render script temp path."));
        lpub->getAct("blenderRenderAct.4")->setEnabled(true);
        ui->RenderLabel->setText(tr("Error encountered."));
        return;
    }

    QThread::sleep(2);

#ifdef Q_OS_WIN
    shellProgram = QLatin1String(WINDOWS_SHELL);
#else
    shellProgram = QLatin1String(UNIX_SHELL);
#endif

    mProcess = new RenderProcess(this);

    connect(mProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(ReadStdOut()));

    QStringList blenderEnvironment = Render::splitParms(lpub->page.meta.LPub.assem.blenderEnvVars.value());

    blenderEnvironment.prepend("LDRAW_DIRECTORY=" + Preferences::ldrawLibPath);

    emit gui->messageSig(LOG_INFO,QObject::tr("Blender Model Renderer Environment Variables: %1")
                         .arg(blenderEnvironment.join(" ")));

    blenderEnvironment << QProcess::systemEnvironment();

    mProcess->setEnvironment(blenderEnvironment);

    mProcess->setWorkingDirectory(QDir::toNativeSeparators(QString("%1/%2").arg(QDir::currentPath(), Paths::blenderRenderDir)));

    mProcess->setStandardErrorFile(GetLogFileName(false/*stdOut*/));

    message = tr("Blender process output: %1").arg(GetLogFileName(true/*stdOut*/));
#ifdef QT_DEBUG_MODE
    qDebug() << qPrintable(message);
#else
    emit gui->messageSig(LOG_INFO, message);
#endif
    if (mOpenInBlender) {
        QFileInfo info(GetLogFileName(true/*stdOut*/));
        if (info.exists())
            QFile::remove(info.absoluteFilePath());
#ifdef Q_OS_WIN
        mProcess->startDetached(shellProgram, QStringList() << "/C" << script.fileName());
#else
        mProcess->startDetached(shellProgram, QStringList() << script.fileName());
#endif
        if (mProcess)
        {
            static QRegularExpression errorRx("(?:\\w)*ERROR: ", QRegularExpression::CaseInsensitiveOption);
            static QRegularExpression warningRx("(?:\\w)*WARNING: ", QRegularExpression::CaseInsensitiveOption);
            mProcess->kill();
            CloseProcess();
            if (mStdOutList.size())
                WriteStdOut();
            if (info.exists())
            {
                QFile log(info.absoluteFilePath());
                QTime waiting = QTime::currentTime().addSecs(3);
                while (!log.size() || QTime::currentTime() < waiting)
                    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
                if (log.size()) {
                    if (log.open(QFile::ReadOnly | QFile::Text))
                    {
                        QByteArray ba = log.readAll();
                        bool const error = QString(ba).contains(errorRx);
                        bool const warning = QString(ba).contains(warningRx);
                        if (error || warning)
                        {
                            QMessageBox::Icon icon = error ? QMessageBox::Critical : QMessageBox::Warning;
                            QString const &items = error ? tr("errors%1").arg(warning ? tr(" and warnings") : "") : warning ? tr("warnings") : "";
                            QString const &title = tr("Open in Blender standard output");
                            QString const &body = tr("Open in Blender encountered %1. See Show Details...").arg(items);
                            BlenderPreferences::showMessage(this, body, title, QString(), QString(ba), 0, icon);
                        }
                    }
                }
            }
            if (mBannerLoaded && lpub->currentStep) {
                lpub->currentStep->loadTheViewer();
                mBannerLoaded = false;
            }
            close();
            return;
        }
    } else {
        ui->RenderProgress->setRange(mBlendProgValue, mBlendProgMax);
        ui->RenderProgress->setValue(1);
#ifdef Q_OS_WIN
        mProcess->start(shellProgram, QStringList() << "/C" << script.fileName());
#else
        mProcess->start(shellProgram, QStringList() << script.fileName());
#endif
    }

    if (mProcess->waitForStarted())
    {
        ui->RenderButton->setText(tr("Cancel"));
        ui->RenderProgress->setValue(ui->RenderProgress->minimum());
        ui->RenderLabel->setText(tr("Loading LDraw Model... %1")
                                 .arg(Gui::elapsedTime(mRenderTime.elapsed())));
        QApplication::processEvents();
        emit gui->messageSig(LOG_INFO, tr("Blender render process [%1] running...").arg(mProcess->processId()));
    }
    else
    {
        lpub->getAct("blenderRenderAct.4")->setEnabled(true);
        message = tr("Error starting Blender render process");
        emit gui->messageSig(LOG_ERROR, message);
        ui->RenderLabel->setText(tr("Error encountered."));
        CloseProcess();
    }
}

#ifdef Q_OS_WIN
int RenderDialog::TerminateChildProcess(const qint64 pid, const qint64 ppid)
{
//    emit gui->messageSig(LOG_DEBUG, QString("PID %1, Parent PID: %2").arg(pid).arg(ppid));
    DWORD pID        = DWORD(pid);
    DWORD ppID       = DWORD(ppid);
    HANDLE hSnapshot = INVALID_HANDLE_VALUE, hProcess = INVALID_HANDLE_VALUE;
    PROCESSENTRY32 pe32;
    static QRegularExpression rx("^(?:cmd\\.exe|conhost\\.exe|blender\\.exe)$", QRegularExpression::CaseInsensitiveOption);

    if ((hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, ppID)) == INVALID_HANDLE_VALUE) {
        emit gui->messageSig(LOG_ERROR, QString("%1 failed: %1").arg("CreateToolhelp32Snapshot").arg(GetLastError()));
        return -1;
    }
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hSnapshot, &pe32) == FALSE) {
        emit gui->messageSig(LOG_ERROR, QString("%1 failed: %2").arg("Process32First").arg(GetLastError()));
        CloseHandle(hSnapshot);
        return -2;
    }
    do {
        if (QString::fromWCharArray(pe32.szExeFile).contains(rx)) {
            if ((pe32.th32ProcessID == pID && pe32.th32ParentProcessID == ppID) || // parent:   cmd.exe
                (pe32.th32ParentProcessID == pID)) {                      // children: conhost.exe, blender.exe
                if ((hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID)) == INVALID_HANDLE_VALUE) {
                    emit gui->messageSig(LOG_ERROR, tr("%1 failed: %2").arg("OpenProcess").arg(GetLastError()));
                    return -3;
                } else {
                    TerminateProcess(hProcess, 9);
                    CloseHandle(hProcess);
                    emit gui->messageSig(LOG_INFO, tr("%1 Process Terminated: PID: %2  Parent PID: %3  Name: %4")
                                         .arg(pe32.th32ParentProcessID == pID ? tr("Child") : tr("Parent"))
                                         .arg(pe32.th32ProcessID)
                                         .arg(pe32.th32ParentProcessID)
                                         .arg(QString::fromWCharArray(pe32.szExeFile)));
                }
            }
        }
    } while (Process32Next(hSnapshot, &pe32));
    CloseHandle(hSnapshot);

    return 0;
}
#endif

void RenderDialog::ReadStdOut()
{
    QString StdOut = QString(mProcess->readAllStandardOutput());
    mStdOutList.append(StdOut);
    QString renderType;
    bool progress = false;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QRegularExpression rxRenderProgress;
    rxRenderProgress.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
#else
    QRegExp rxRenderProgress;
    rxRenderProgress.setCaseSensitivity(Qt::CaseInsensitive);
#endif
    int blenderVersionNum = QString(Preferences::blenderVersion.mid(1, 1)).toInt();
    bool const blenderVersion3OrGreater = blenderVersionNum > 2;
    if (blenderVersion3OrGreater)
    {
        rxRenderProgress.setPattern("Sample (\\d+)\\/(\\d+)");
        renderType = QLatin1String("Sample");
    } else {
        rxRenderProgress.setPattern("(\\d+)\\/(\\d+) Tiles");
        renderType = QLatin1String("Tile");
    }
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QRegularExpressionMatch expression = rxRenderProgress.match(StdOut);
    if ((progress = expression.hasMatch()))
    {
        mBlendProgValue = expression.captured(1).toInt();
        mBlendProgMax   = expression.captured(2).toInt();
    }
#else
    if ((progress = StdOut.contains(rxRenderProgress)))
    {
        mBlendProgValue = rxRenderProgress.cap(1).toInt();
        mBlendProgMax   = rxRenderProgress.cap(2).toInt();
    }
#endif
    if (progress) {
        ui->RenderProgress->setMaximum(mBlendProgMax);
        ui->RenderProgress->setValue(mBlendProgValue);
        emit gui->messageSig(LOG_INFO, tr("Rendered %1 %2/%3")
                             .arg(renderType)
                             .arg(mBlendProgValue)
                             .arg(mBlendProgMax));
    }
}

QString RenderDialog::ReadStdErr(bool &hasError) const
{
    hasError = mRenderType == BLENDER_RENDER ? false : true;
    QFile file;
    QStringList returnLines;
    static QRegularExpression rx("^POV-Ray finished$", QRegularExpression::CaseInsensitiveOption);

    file.setFileName(GetLogFileName(false/*stdOut*/));

    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        QString message = tr("Failed to open log file: %1:\n%2")
                             .arg(file.fileName(), file.errorString());
        return message;
    }

    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine(0);
        returnLines << line.trimmed() + "<br>";
        if (mRenderType == POVRAY_RENDER) {
            if (line.contains(rx))
                hasError = false;
        } else if (mRenderType == BLENDER_RENDER) {
            if (!hasError && !line.isEmpty())
                hasError = true;
        }
    }

    return returnLines.join(" ");
}

void RenderDialog::WriteStdOut()
{
    QFile file(GetLogFileName(true/*stdOut*/));

    if (file.open(QFile::WriteOnly | QIODevice::Truncate | QFile::Text))
    {
        QTextStream Out(&file);

        for (QString& Line : mStdOutList)
            Out << Line;

        file.close();

        ui->RenderOutputButton->setEnabled(true);
    }
    else
    {
       emit gui->messageSig(LOG_INFO, tr("Error writing to stdout file '%1':\n%2")
                                         .arg(file.fileName(), file.errorString()));
    }
}

void RenderDialog::Update()
{
    if (!mProcess)
        return;

    if (mProcess->state() == QProcess::NotRunning)
    {
        if (mRenderType == BLENDER_RENDER)
            emit gui->messageSig(LOG_INFO, tr("Blender process finished."));
        ShowResult();
        CloseProcess();
    }

    if (mRenderType == POVRAY_RENDER) {

        if (!mOutputBuffer)
        {
            mOutputFile.setFileName(GetOutputFileName());

            if (!mOutputFile.open(QFile::ReadWrite))
                return;

            mOutputBuffer = mOutputFile.map(0, mOutputFile.size());

            if (!mOutputBuffer)
            {
                mOutputFile.close();
                return;
            }
        }

        struct lcSharedMemoryHeader
        {
            quint32 Version;
            quint32 Width;
            quint32 Height;
            quint32 PixelsWritten;
            quint32 PixelsRead;
        };

        lcSharedMemoryHeader* Header = (lcSharedMemoryHeader*)mOutputBuffer;

        if (Header->PixelsWritten == Header->PixelsRead)
            return;

        int Width = int(Header->Width);
        int Height = int(Header->Height);
        int PixelsWritten = int(Header->PixelsWritten);

        if (!Header->PixelsRead)
            mImage = QImage(Width, Height, QImage::Format_ARGB32);

        quint8* Pixels = (quint8*)(Header + 1);

        for (int y = 0; y < Height; y++)
        {
            for (int x = 0; x < Width; x++)
            {
                mImage.setPixel(x, y, qRgba(Pixels[0], Pixels[1], Pixels[2], Pixels[3]));
                Pixels += 4;
            }
        }

        Header->PixelsRead = quint32(PixelsWritten);

        ui->RenderProgress->setMaximum(mImage.width() * mImage.height());
        ui->RenderProgress->setValue(int(Header->PixelsRead));

        if (PixelsWritten == Width * Height)
            ui->RenderProgress->setValue(ui->RenderProgress->maximum());

        ui->preview->SetImage(mImage.scaled(mPreviewWidth, mPreviewHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void RenderDialog::ShowResult()
{
    bool hasError;

    const QString StdErrLog = ReadStdErr(hasError);

    if (mProcess->exitStatus() != QProcess::NormalExit || mProcess->exitCode() != 0 || hasError)
    {
        emit gui->messageSig(LOG_NOTICE, tr("Render failed. See %1 for details.")
                                            .arg(GetLogFileName(false/*stdOut*/)));
        ui->RenderLabel->setText(tr("Image generation failed."));
        ui->RenderProgress->setRange(0,1);
        ui->RenderProgress->setValue(0);
        QString const &title = mRenderType == BLENDER_RENDER ? tr("Blender Render") : tr("POV-Ray Render");
        QString const &body = tr("An error occurred while rendering. See Show Details...");
        BlenderPreferences::showMessage(this, body, title, QString(), StdErrLog, 0, QMessageBox::Critical);
        return;
    } else {
        ui->RenderProgress->setValue(ui->RenderProgress->maximum());
    }

    QString message;

    bool Success = false;

    QString FileName = ui->OutputEdit->text();

    if (mRenderType == POVRAY_RENDER) {

        lpub->getAct("povrayRenderAct.4")->setEnabled(true);

        ui->preview->SetImage(mImage.scaled(mPreviewWidth, mPreviewHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));

        emit gui->messageSig(LOG_INFO,tr("Writing POV-Ray rendered image '%1'...").arg(ui->OutputEdit->text()));

        if (!FileName.isEmpty())
        {
            QImageWriter Writer(FileName);

            Success = Writer.write(mImage);

            if (!Success)
                emit gui->messageSig(LOG_ERROR,tr("Error writing to image file '%1':\n%2").arg(FileName, Writer.errorString()));
            else if (Preferences::povrayAutoCrop)
                Render::clipImage(FileName);
        }

    } else if (mRenderType == BLENDER_RENDER) {

        lpub->getAct("blenderRenderAct.4")->setEnabled(true);

        Success = QFileInfo::exists(FileName);
        if (Success) {
            setMinimumSize(100, 100);
            QImageReader reader(FileName);
            mImage = reader.read();
            mImage = mImage.convertToFormat(QImage::Format_ARGB32_Premultiplied);

            mPreviewWidth  = mImage.width();
            mPreviewHeight = mImage.height();

            ui->preview->SetImage(mImage.scaled(mPreviewWidth, mPreviewHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }

    QString imageType = mRenderType == BLENDER_RENDER ? QLatin1String("Blender") : QLatin1String("POV-Ray");

    if (!Success) {
        ui->RenderLabel->setStyleSheet("QLabel { color : red; }");
        ui->RenderLabel->setText(tr("Image render failed."));
    }

    WriteStdOut();

    if (mBannerLoaded && lpub->currentStep) {
        lpub->currentStep->loadTheViewer();
        mBannerLoaded = false;
    }

    message = QString("%1 CSI %2. %3")
                      .arg(imageType,
                           Success ? mOpenInBlender ? tr("completed") :
                                                   tr("generated %1").arg(FileName) :
                                                   tr("failed (unknown reason)"),
                           Gui::elapsedTime(mRenderTime.elapsed()));
    emit gui->messageSig(Success ? LOG_INFO : LOG_ERROR, message);
}

QString RenderDialog::GetOutputFileName() const
{
    return QDir(QDir::tempPath()).absoluteFilePath("lpub3d-render-map.out");
}

QString RenderDialog::GetPOVFileName() const
{
    return QDir::toNativeSeparators(mModelFile + ".pov");
}

QString RenderDialog::GetLogFileName(bool stdOut) const
{
    QString logFile = mRenderType == POVRAY_RENDER ? stdOut ? "stdout-povray-render" : "stderr-povray-render" :
                                                     stdOut ? "stdout-blender-render"  : "stderr-blender-render";
    return QDir::toNativeSeparators(QDir::currentPath() + QDir::separator() + logFile);
}

void RenderDialog::UpdateElapsedTime()
{
    if (mProcess && !mOpenInBlender)
    {
        QString const renderType = Preferences::blenderVersion.startsWith("v3") ? QLatin1String("Samples") : QLatin1String("Tiles");
        ui->RenderLabel->setText(tr("%1: %2/%3, %4")
                                  .arg(renderType)
                                  .arg(mBlendProgValue)
                                  .arg(mBlendProgMax)
                                  .arg(Gui::elapsedTime(mRenderTime.elapsed())));
    }
}

void RenderDialog::CloseProcess()
{
    delete mProcess;
    mProcess = nullptr;

    if (mRenderType == POVRAY_RENDER) {

        mOutputFile.unmap((uchar*)mOutputBuffer);
        mOutputBuffer = nullptr;
        mOutputFile.close();

        QFile::remove(GetOutputFileName());

        QFile::remove(GetPOVFileName());

    } else if (mRenderType == BLENDER_RENDER) {
        emit gui->messageSig(LOG_INFO, tr("Blender process closed"));
    }

    if (!mOpenInBlender)
        ui->RenderButton->setText(tr("Render"));
}

bool RenderDialog::PromptCancel()
{
    if (mProcess)
    {
        if (QMessageBox::question(this,
                                  tr("Cancel Render"),
                                  tr("Are you sure you want to cancel the current Render?"),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            if (mRenderType == POVRAY_RENDER)
                lpub->getAct("povrayRenderAct.4")->setEnabled(true);
            else if (mRenderType == BLENDER_RENDER && !mOpenInBlender)
                lpub->getAct("blenderRenderAct.4")->setEnabled(true);
#ifdef Q_OS_WIN
            TerminateChildProcess(mProcess->processId(),
                                  QCoreApplication::applicationPid());
#endif
            mProcess->kill();
            CloseProcess();
            if (mStdOutList.size()) {
                WriteStdOut();
                ui->RenderOutputButton->setEnabled(true);
            }
            if (mRenderType == BLENDER_RENDER)
                ui->RenderLabel->setText(tr("Tiles: %1/%2, Render Cancelled.")
                                            .arg(mBlendProgValue)
                                            .arg(mBlendProgMax));

            if (mBannerLoaded && lpub->currentStep) {
                lpub->currentStep->loadTheViewer();
                mBannerLoaded = false;
            }
        }
        else
            return false;
    }

    return true;
}

void RenderDialog::reject()
{
    if (PromptCancel())
        QDialog::reject();
}

void RenderDialog::on_InputBrowseButton_clicked()
{
    if (ui->InputEdit->text().isEmpty())
        mModelFile = QFileDialog::getOpenFileName(this, tr("Select LDraw Input File"), QFileInfo(Gui::getCurFile()).absoluteFilePath(), tr("Supported LDraw Files (*.mpd *.ldr *.dat);;All Files (*.*)"));

    QFile modelFile(mModelFile);
    if (modelFile.open( QIODevice::WriteOnly|QIODevice::Append ))
        mPopulatedFile = modelFile.pos() > 0;

    ui->InputEdit->setText(QDir::toNativeSeparators(mModelFile));
}

void RenderDialog::on_OutputBrowseButton_clicked()
{
    QString Result = QFileDialog::getSaveFileName(this, tr("Select Image Output File"), ui->OutputEdit->text(), tr("Supported Image Files (*.bmp *.png *.jpg);;BMP Files (*.bmp);;PNG Files (*.png);;JPEG Files (*.jpg);;All Files (*.*)"));

    if (!Result.isEmpty()) {
        ui->OutputEdit->setText(QDir::toNativeSeparators(Result));
        validateInput();
    }
}

void RenderDialog::on_InputGenerateCheck_toggled()
{
    if (mPopulatedFile)
        return;

    bool const generateModelFile = ui->InputGenerateCheck->isChecked();
    ui->InputEdit->setEnabled(!generateModelFile);
    ui->InputBrowseButton->setEnabled(!generateModelFile);
    if (generateModelFile) {
        mModelFile = Render::getRenderModelFile(mRenderType, false/*save current model*/);
        ui->InputEdit->setText(mModelFile);
    } else {
        ui->InputEdit->clear();
        ui->InputEdit->setPlaceholderText(tr("Enter LDraw file"));
    }
}

void RenderDialog::enableReset(const QString &displayText)
{
  mModelFile = Render::getRenderModelFile(mRenderType, false/*save current model*/);

  if (sender() == ui->OutputEdit)
        resetOutputAct->setEnabled(QDir::toNativeSeparators(displayText.toLower()) != mModelFile.toLower());

  if (sender() == ui->InputEdit)
    resetInputAct->setEnabled(QDir::toNativeSeparators(displayText.toLower()) != mModelFile.toLower());
}

void RenderDialog::resetEdit()
{
    if (sender() == resetOutputAct) {
        resetOutputAct->setEnabled(false);
        mCsiKeyList = QString(lpub->ldrawFile.getViewerConfigKey(mViewerStepKey).split(";").last()).split("_");
        ui->OutputEdit->setText(Render::getRenderImageFile(mRenderType));
        ui->RenderProgress->setRange(0,1);
        ui->RenderProgress->setValue(0);
        ui->RenderLabel->setText(QString());
        ui->RenderOutputButton->setEnabled(false);
        if (mRenderType == BLENDER_RENDER ) {
            ui->preview->hide();
            adjustSize();
            setMinimumWidth(int(ui->preview->geometry().width()/*mWidth*/));
        }
    }

    if (sender() == resetInputAct) {
        mModelFile = Render::getRenderModelFile(mRenderType, false/*save current model*/);
        ui->InputEdit->setText(mModelFile);
        ui->InputEdit->setEnabled(false);
        ui->InputGenerateCheck->setChecked(true);
        ui->InputBrowseButton->setEnabled(false);
    }
}

void RenderDialog::validateInput()
{
    mModelFile = QDir::toNativeSeparators(ui->InputEdit->text());
    QFileInfo fileInfo(mModelFile);

    if (!fileInfo.exists()) {
        mModelFile = Render::getRenderModelFile(mRenderType, false/*save current model*/);
        emit gui->messageSig(LOG_WARNING, tr("Input file %1 was not found. Using default.").arg(mModelFile), true/*show message*/);
        ui->InputEdit->setText(mModelFile);
        ui->InputEdit->setEnabled(false);
        ui->InputGenerateCheck->setChecked(true);
        ui->InputBrowseButton->setEnabled(false);
    } else {
        QString mMn = fileInfo.fileName(), labelMessage;
        mMn = mMn.replace(mMn.indexOf(mMn.at(0)),1,mMn.at(0).toUpper());
        if (mOpenInBlender)
            labelMessage = tr("Open%1 in Blender using %2")
                               .arg(mMn.isEmpty() ? "" : tr(" <b>%1</b>").arg(mMn), mImportModule);
        else
            labelMessage = tr("Render image%1").arg(mMn.isEmpty() ? "" : tr(" for <b>%1</b>").arg(mMn));
        ui->RenderLabel->setText(labelMessage);
    }

    if (!mHaveKeys) {
        mCsiKeyList = MetaDefaults::getDefaultCSIKeys().split("_");
        mHaveKeys = true;
        ui->OutputEdit->clear();
        ui->OutputEdit->setEnabled(true);
        ui->RenderSettingsButton->setEnabled(true);
        ui->RenderButton->setEnabled(true);
        ui->OutputBrowseButton->setEnabled(true);
    }
}

RenderProcess::~RenderProcess() {
    if(state() == QProcess::Running ||
       state() == QProcess::Starting)
    {
        terminate();
        waitForFinished();
    }
}

void RenderDialog::on_RenderOutputButton_clicked()
{
    QString renderType = QLatin1String("Blender");
    QFileInfo fileInfo(GetLogFileName(true/*stdOut*/));
    QString message = tr("Blender standard output log not found: %1.").arg(fileInfo.absoluteFilePath());
    if (mRenderType == POVRAY_RENDER)
    {
        renderType = QLatin1String("POV-Ray");
        fileInfo.setFile(GetLogFileName(false/*stdOut*/));
        message = tr("POV-Ray standard error log not found: %1.").arg(fileInfo.absoluteFilePath());
    }
    if (!fileInfo.exists()) {
        emit gui->messageSig(LOG_ERROR, message);
        return;
    }
    QString title = tr("%1 Render Log").arg(renderType);
    QString status = tr("View %1 render process standard output log").arg(renderType);
    gui->displayParmsFile(fileInfo.absoluteFilePath());
    gui->parmsWindow->setWindowTitle(tr(title.toLatin1(),status.toLatin1()));
    gui->parmsWindow->show();
}

#endif // LC_DISABLE_RENDER_DIALOG
