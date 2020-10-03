/****************************************************************************
**
** Copyright (C) 2020 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************************
 *
 * Preview widget is used to display a preview of the selected parts or model.
 * This module is called in the Substitue part dialogue and the context menu
 * of the LDraw Editor
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QMainWindow>
#include <QString>
#include "lc_global.h"
#include "lc_glwidget.h"
#include "lc_scene.h"
#include "lc_viewsphere.h"
#include "lc_commands.h"
#include "lc_application.h"
#include "camera.h"

class QLabel;
class Project;
class lcModel;
class lcPiece;
class lcQGLWidget;

class PreviewDockWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit PreviewDockWidget(QMainWindow *parent = nullptr);
    bool SetCurrentPiece(const QString &PartType, int ColorCode);
    void ClearPreview();

protected:
    QToolBar      *toolBar;
    QLabel        *label;
    PreviewWidget *Preview;
    lcQGLWidget   *ViewWidget;
};

class PreviewWidget : public lcGLWidget
{
public:
    enum class lcTrackButton
    {
        None,
        Left,
        Middle,
        Right
    };

    enum lcTrackTool
    {
        LC_TRACKTOOL_NONE,
        LC_TRACKTOOL_PAN,
        LC_TRACKTOOL_ORBIT_XY,
        LC_TRACKTOOL_COUNT
    };

    PreviewWidget(bool subPreview = false);
    ~PreviewWidget();

    lcTool GetTool() const
    {
        return mTool;
    }

    lcCamera* GetCamera() const
    {
        return mCamera;
    }

    QString GetDescription() const
    {
        return mDescription;
    }

    lcVector3 UnprojectPoint(const lcVector3& Point) const
    {
        int Viewport[4] = { 0, 0, mWidth, mHeight };
        return lcUnprojectPoint(Point, mCamera->mWorldView, GetProjectionMatrix(), Viewport);
    }

    void UnprojectPoints(lcVector3* Points, int NumPoints) const
    {
        int Viewport[4] = { 0, 0, mWidth, mHeight };
        lcUnprojectPoints(Points, NumPoints, mCamera->mWorldView, GetProjectionMatrix(), Viewport);
    }

    void ClearPreview();
    bool SetCurrentPiece(const QString& PartType, int ColorCode);
    lcMatrix44 GetProjectionMatrix() const;
    lcModel* GetActiveModel() const;
    lcCursor GetCursor() const;
    void SetCamera(lcCamera* Camera);
    void SetDefaultCamera();
    void ZoomExtents();

    // exclusively called from viewSphere
    void SetViewpoint(const lcVector3& Position);
    void StartOrbitTracking();
    bool IsTracking() const
    {
        return mTrackButton != lcTrackButton::None;
    }

    void OnInitialUpdate() override;
    void OnDraw() override;
    void OnUpdateCursor() override;
    void OnLeftButtonDown() override;
    void OnLeftButtonUp() override;
    void OnLeftButtonDoubleClick() override;
    void OnMiddleButtonDown() override;
    void OnMiddleButtonUp() override;
    void OnRightButtonDown() override;
    void OnRightButtonUp() override;
    void OnMouseMove() override;
    void OnMouseWheel(float Direction) override;

protected:
    void DrawAxes();
    void DrawViewport();

    lcTool GetCurrentTool() const;
    void StartTracking(lcTrackButton TrackButton);
    void StopTracking(bool Accept);
    void OnButtonDown(lcTrackButton TrackButton);

    Project* mLoader;
    lcModel* mModel;
    lcCamera* mCamera;
    lcViewSphere mViewSphere;

    lcScene mScene;

    lcTool mTool;
    lcTrackButton mTrackButton;
    lcTrackTool mTrackTool;

    QString mDescription;

    bool mIsPart;
    bool mIsSubPreview;
    bool mTrackUpdated;
    int mMouseDownX;
    int mMouseDownY;
};

extern class PreviewWidget* gPreviewWidget;

#endif // PREVIEWWIDGET_H
