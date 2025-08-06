/****************************************************************************
**
** Copyright (C) 2019 - 2025 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public Liceense (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef RENDERDIALOG_H
#define RENDERDIALOG_H

#ifndef LC_DISABLE_RENDER_DIALOG

#include <QDialog>
#include <QFile>
#include <QTimer>
#include <QElapsedTimer>

#include <QProcess>

namespace Ui {
class RenderDialog;
}

class QProcess;
class RenderProcess : public QProcess
{
    Q_OBJECT

public:
    explicit RenderProcess(QObject *parent = nullptr)
    : QProcess(parent)
    {
    }
    ~RenderProcess();
};

class RenderPreviewWidget : public QWidget
{
    Q_OBJECT

public:
    RenderPreviewWidget(QWidget* Parent)
        : QWidget(Parent)
    {
    }

    void SetImage(QImage Image)
    {
        mImage = Image;
        mScaledImage = QImage();
        update();
    }

protected:
    void resizeEvent(QResizeEvent* Event) override;
    void paintEvent(QPaintEvent* PaintEvent) override;

    QImage mImage;
    QImage mScaledImage;
};

class RenderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RenderDialog(
            QWidget* Parent = nullptr,
            int renderType = 0,
            int importOnly = 0);
    virtual ~RenderDialog();

private slots:
    void reject() override;
    void on_RenderSettingsButton_clicked();
    void on_RenderButton_clicked();
    void on_InputBrowseButton_clicked();
    void on_OutputBrowseButton_clicked();
    void on_RenderOutputButton_clicked();
    void on_InputGenerateCheck_toggled();
    void resetEdit();
    void enableReset(QString const &);
    void validateInput();
    void Update();

protected slots:
    void ReadStdOut();
    void WriteStdOut();
    void UpdateElapsedTime();

protected:
    QString GetOutputFileName() const;
    QString GetPOVFileName() const;
    QString GetLogFileName(bool = true) const;
    QString ReadStdErr(bool &hasError) const;
    void RenderPOVRay();
    void RenderBlender();
    void CloseProcess();
    bool PromptCancel();
    void ShowResult();
#ifdef Q_OS_WIN
    int TerminateChildProcess(const qint64 pid, const qint64 ppid);
#endif

    RenderProcess* mProcess;

    QAction   *resetInputAct;
    QAction   *resetOutputAct;

    void* mOutputBuffer;
    QTimer mUpdateTimer;
    QElapsedTimer mRenderTime;
    QFile mOutputFile;
    QImage mImage;
    QString mViewerStepKey;
    QString mModelFile;
    QString mImportModule;
    QString mMn;

    QStringList mCsiKeyList;
    QStringList mCsiKeyListData;
    QStringList mStdOutList;

    bool mPopulatedFile;
    bool mBannerLoaded;
    bool mHaveKeys;
    bool mTransBackground;
    int mWidth;
    int mHeight;
    int mResolution;
    int mQuality;
    double mScale;

    int mRenderType;
    int mImportOnly;
    int mPreviewWidth;
    int mPreviewHeight;
    int mBlendProgValue;
    int mBlendProgMax;

    Ui::RenderDialog* ui;
};

#endif // LC_DISABLE_RENDER_DIALOG

#endif // RENDERDIALOG_H_
