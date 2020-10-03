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

#include "lc_global.h"
#include "previewwidget.h"
#include "pieceinf.h"
#include "piece.h"
#include "project.h"
#include "lc_model.h"
#include "camera.h"
#include "texfont.h"
#include "lc_library.h"

#include "lc_qglwidget.h"

#include "lpubalert.h"
#include "paths.h"

PreviewWidget* gPreviewWidget;

PreviewDockWidget::PreviewDockWidget(QMainWindow *parent)
    :QMainWindow(parent)
{
    Preview    = new PreviewWidget();
    ViewWidget = new lcQGLWidget(nullptr, Preview, true/*isView*/, true/*isPreview*/);
    setCentralWidget(ViewWidget);
    setMinimumSize(200, 200);
    toolBar = addToolBar(tr("PreviewDescription"));
    toolBar->setObjectName("PreviewDescription");
    toolBar->setMovable(false);
    label = new QLabel("");
    toolBar->addWidget(label);
}

bool PreviewDockWidget::SetCurrentPiece(const QString &PartType, int ColorCode)
{
    label->setText("Loading...");
    if (Preview->SetCurrentPiece(PartType, ColorCode)) {
        label->setText(Preview->GetDescription());
        return true;
    }
    return false;
}

void PreviewDockWidget::ClearPreview()
{
    Preview->ClearPreview();
    label->setText("");
}

PreviewWidget::PreviewWidget(bool subPreview)
: mLoader(new Project(true/*isPreview*/)),
  mViewSphere(this/*Preview*/, subPreview),
  mIsPart(false),
  mIsSubPreview(subPreview)
{
    mTool        = LC_TOOL_SELECT;
    mTrackTool   = LC_TRACKTOOL_NONE;
    mTrackButton = lcTrackButton::None;

    mLoader->SetActiveModel(0);
    mModel  = mLoader->GetActiveModel();
    mCamera = nullptr;

    SetDefaultCamera();
}

PreviewWidget::~PreviewWidget()
{
    if (mCamera && mCamera->IsSimple())
        delete mCamera;

    if (mIsPart) {
        lcPiecesLibrary* Library = lcGetPiecesLibrary();
        for (lcPiece* Piece : mModel->GetPieces())
        {
            PieceInfo *Info = Piece->mPieceInfo;
            Library->ReleasePieceInfo(Info);
        }
    }
}

bool PreviewWidget::SetCurrentPiece(const QString &PartType, int ColorCode)
{
    lcPiecesLibrary  *Library = lcGetPiecesLibrary();
    PieceInfo* Info = Library->FindPiece(PartType.toLatin1().constData(), nullptr, false, false);
    if (Info) {
        mIsPart = true;
        mDescription = Info->m_strDescription;
        lcModel* ActiveModel = GetActiveModel();

        ActiveModel->SelectAllPieces();
        ActiveModel->DeleteSelectedObjects();

        Library->LoadPieceInfo(Info, false, true);
        Library->WaitForLoadQueue();

        float* Matrix =  lcMatrix44Identity();;
        lcMatrix44 Transform(lcVector4( Matrix[0],  Matrix[2], -Matrix[1], 0.0f), lcVector4(Matrix[8],  Matrix[10], -Matrix[9],  0.0f),
                             lcVector4(-Matrix[4], -Matrix[6],  Matrix[5], 0.0f), lcVector4(Matrix[12], Matrix[14], -Matrix[13], 1.0f));

        int CurrentStep = 1;
        lcPiece* Piece = new lcPiece(nullptr);

        Piece->SetPieceInfo(Info, PartType, false);
        Piece->Initialize(Transform, CurrentStep);
        Piece->SetColorCode(ColorCode);

        ActiveModel->AddPiece(Piece);

        emit lpubAlert->messageSig(LOG_DEBUG,
                                   QString("Preview PartType: %1, Name: %2, ColorCode: %3, ColorIndex: %4")
                                   .arg(Piece->GetID()).arg(Piece->GetName()).arg(ColorCode).arg( Piece->mColorIndex));
        Piece = nullptr;
    } else {
        QString ModelPath = QString("%1/%2/%3")
                .arg(QDir::currentPath())
                .arg(Paths::tmpDir)
                .arg(PartType);

        if (!mLoader->Load(ModelPath)) {
            emit lpubAlert->messageSig(LOG_DEBUG,QString("Failed to load '%1'.").arg(ModelPath));
            return false;
        }

        mLoader->SetActiveModel(0);
        lcGetPiecesLibrary()->RemoveTemporaryPieces();
        mModel = mLoader->GetActiveModel();
        if (ColorCode != LDRAW_MATERIAL_COLOUR)
            mModel->SetUnoffPartColorCode(ColorCode);
        if (!mModel->GetProperties().mDescription.isEmpty())
            mDescription = mModel->GetProperties().mDescription;
        else
            mDescription = PartType;
    }
    ZoomExtents();

    return true;
}

void PreviewWidget::ClearPreview()
{
    mLoader = new Project(true/*isPreview*/);
    mLoader->SetActiveModel(0);
    mModel = mLoader->GetActiveModel();
    lcGetPiecesLibrary()->UnloadUnusedParts();
    Redraw();
}

void PreviewWidget::SetDefaultCamera()
{
    if (!mCamera || !mCamera->IsSimple())
        mCamera = new lcCamera(true);
    mCamera->SetViewpoint(LC_VIEWPOINT_HOME);
}

void PreviewWidget::SetCamera(lcCamera* Camera) // called by lcModel::DeleteModel()
{
    if (!mCamera || !mCamera->IsSimple())
        mCamera = new lcCamera(true);

    mCamera->CopyPosition(Camera);
}

lcModel* PreviewWidget::GetActiveModel() const
{
    return mModel;
}

void PreviewWidget::ZoomExtents()
{
    lcModel* ActiveModel = GetActiveModel();
    if (ActiveModel) {
        ActiveModel->ZoomExtents(mCamera, float(mWidth) / float(mHeight));
        Redraw();
    }
}

lcMatrix44 PreviewWidget::GetProjectionMatrix() const
{
    float AspectRatio = (float)mWidth / (float)mHeight;

    if (mCamera->IsOrtho())
    {
        float OrthoHeight = mCamera->GetOrthoHeight() / 2.0f;
        float OrthoWidth = OrthoHeight * AspectRatio;

        return lcMatrix44Ortho(-OrthoWidth, OrthoWidth, -OrthoHeight, OrthoHeight, mCamera->m_zNear, mCamera->m_zFar * 4);
    }
    else
        return lcMatrix44Perspective(mCamera->m_fovy, AspectRatio, mCamera->m_zNear, mCamera->m_zFar);
}

void PreviewWidget::StartOrbitTracking() // called by viewSphere
{
    mTrackTool = LC_TRACKTOOL_ORBIT_XY;

    OnUpdateCursor();

    OnButtonDown(lcTrackButton::Left);
}

void PreviewWidget::SetViewpoint(const lcVector3& Position)
{
    if (!mCamera || !mCamera->IsSimple())
    {
        lcCamera* OldCamera = mCamera;

        mCamera = new lcCamera(true);

        if (OldCamera)
            mCamera->CopySettings(OldCamera);
    }

    mCamera->SetViewpoint(Position);

    Redraw();
}

void PreviewWidget::DrawViewport()
{
    mContext->SetWorldMatrix(lcMatrix44Identity());
    mContext->SetViewMatrix(lcMatrix44Translation(lcVector3(0.375, 0.375, 0.0)));
    mContext->SetProjectionMatrix(lcMatrix44Ortho(0.0f, mWidth, 0.0f, mHeight, -1.0f, 1.0f));

    mContext->SetDepthWrite(false);
    glDisable(GL_DEPTH_TEST);

    if (true/*we have an active view*/)
    {
        mContext->SetMaterial(lcMaterialType::UnlitColor);
        mContext->SetColor(lcVector4FromColor(lcGetPreferences().mPreviewActiveColor));
        float Verts[8] = { 0.0f, 0.0f, mWidth - 1.0f, 0.0f, mWidth - 1.0f, mHeight - 1.0f, 0.0f, mHeight - 1.0f };

        mContext->SetVertexBufferPointer(Verts);
        mContext->SetVertexFormatPosition(2);
        mContext->DrawPrimitives(GL_LINE_LOOP, 0, 4);
    }

    mContext->SetDepthWrite(true);
    glEnable(GL_DEPTH_TEST);
}

void PreviewWidget::DrawAxes()
{
//	glClear(GL_DEPTH_BUFFER_BIT);

    const float Verts[28 * 3] =
    {
         0.00f,  0.00f,  0.00f, 20.00f,  0.00f,  0.00f, 12.00f,  3.00f,  0.00f, 12.00f,  2.12f,  2.12f,
        12.00f,  0.00f,  3.00f, 12.00f, -2.12f,  2.12f, 12.00f, -3.00f,  0.00f, 12.00f, -2.12f, -2.12f,
        12.00f,  0.00f, -3.00f, 12.00f,  2.12f, -2.12f,  0.00f, 20.00f,  0.00f,  3.00f, 12.00f,  0.00f,
         2.12f, 12.00f,  2.12f,  0.00f, 12.00f,  3.00f, -2.12f, 12.00f,  2.12f, -3.00f, 12.00f,  0.00f,
        -2.12f, 12.00f, -2.12f,  0.00f, 12.00f, -3.00f,  2.12f, 12.00f, -2.12f,  0.00f,  0.00f, 20.00f,
         0.00f,  3.00f, 12.00f,  2.12f,  2.12f, 12.00f,  3.00f,  0.00f, 12.00f,  2.12f, -2.12f, 12.00f,
         0.00f, -3.00f, 12.00f, -2.12f, -2.12f, 12.00f, -3.00f,  0.00f, 12.00f, -2.12f,  2.12f, 12.00f,
    };

    const GLushort Indices[78] =
    {
         0,  1,  0, 10,  0, 19,
         1,  2,  3,  1,  3,  4,  1,  4,  5,  1,  5,  6,  1,  6,  7,  1,  7,  8,  1,  8,  9,  1,  9,  2,
        10, 11, 12, 10, 12, 13, 10, 13, 14, 10, 14, 15, 10, 15, 16, 10, 16, 17, 10, 17, 18, 10, 18, 11,
        19, 20, 21, 19, 21, 22, 19, 22, 23, 19, 23, 24, 19, 24, 25, 19, 25, 26, 19, 26, 27, 19, 27, 20
    };

    lcMatrix44 TranslationMatrix = lcMatrix44Translation(lcVector3(30.375f, 30.375f, 0.0f));
    lcMatrix44 WorldViewMatrix = mCamera->mWorldView;
    WorldViewMatrix.SetTranslation(lcVector3(0, 0, 0));

    mContext->SetMaterial(lcMaterialType::UnlitColor);
    mContext->SetWorldMatrix(lcMatrix44Identity());
    mContext->SetViewMatrix(lcMul(WorldViewMatrix, TranslationMatrix));
    mContext->SetProjectionMatrix(lcMatrix44Ortho(0, mWidth, 0, mHeight, -50, 50));

    mContext->SetVertexBufferPointer(Verts);
    mContext->SetVertexFormatPosition(3);
    mContext->SetIndexBufferPointer(Indices);

    mContext->SetColor(0.0f, 0.0f, 0.0f, 1.0f);
    mContext->DrawIndexedPrimitives(GL_LINES, 6, GL_UNSIGNED_SHORT, 0);

    mContext->SetColor(0.8f, 0.0f, 0.0f, 1.0f);
    mContext->DrawIndexedPrimitives(GL_TRIANGLES, 24, GL_UNSIGNED_SHORT, 6 * 2);
    mContext->SetColor(0.0f, 0.8f, 0.0f, 1.0f);
    mContext->DrawIndexedPrimitives(GL_TRIANGLES, 24, GL_UNSIGNED_SHORT, (6 + 24) * 2);
    mContext->SetColor(0.0f, 0.0f, 0.8f, 1.0f);
    mContext->DrawIndexedPrimitives(GL_TRIANGLES, 24, GL_UNSIGNED_SHORT, (6 + 48) * 2);

    mContext->SetMaterial(lcMaterialType::UnlitTextureModulate);
    mContext->SetViewMatrix(TranslationMatrix);
    mContext->BindTexture2D(gTexFont.GetTexture());
    glEnable(GL_BLEND);

    float TextBuffer[6 * 5 * 3];
/*** Native viewer camera globe mod, switch Y and Z axis with -Y(LC -Z) in the up direction ***/
    lcVector3 PosX = lcMul30(lcVector3(25.0f, 0.0f, 0.0f), WorldViewMatrix);
    gTexFont.GetGlyphTriangles(PosX.x, PosX.y, PosX.z, 'X', TextBuffer);
    lcVector3 PosY = lcMul30(lcVector3(0.0f, 25.0f, 0.0f), WorldViewMatrix);
    gTexFont.GetGlyphTriangles(PosY.x, PosY.y, PosY.z, 'Z', TextBuffer + 5 * 6);
    lcVector3 PosZ = lcMul30(lcVector3(0.0f, 0.0f, 25.0f), WorldViewMatrix);
    gTexFont.GetGlyphTriangles(PosZ.x, PosZ.y, PosZ.z, 'Y', TextBuffer + 5 * 6 * 2);
/*** Camera globe mod end ***/

    mContext->SetVertexBufferPointer(TextBuffer);
    mContext->SetVertexFormat(0, 3, 0, 2, 0, false);

    mContext->SetColor(lcVector4FromColor(lcGetPreferences().mAxesColor));
    mContext->DrawPrimitives(GL_TRIANGLES, 0, 6 * 3);

    glDisable(GL_BLEND);
}

lcTool PreviewWidget::GetCurrentTool() const
{
    const lcTool ToolFromTrackTool[] =
    {
        LC_TOOL_SELECT,             // LC_TRACKTOOL_NONE
        LC_TOOL_PAN,                // LC_TRACKTOOL_PAN
        LC_TOOL_ROTATE_VIEW,        // LC_TRACKTOOL_ORBIT_XY
    };

    return ToolFromTrackTool[mTrackTool];
}

void PreviewWidget::StartTracking(lcTrackButton TrackButton)
{
    mTrackButton = TrackButton;
    mTrackUpdated = false;
    mMouseDownX = mInputState.x;
    mMouseDownY = mInputState.y;
    lcTool Tool = GetCurrentTool();  // Either LC_TRACKTOOL_NONE (LC_TOOL_SELECT) or LC_TRACKTOOL_ORBIT_XY (LC_TOOL_ROTATE_VIEW)
    lcModel* ActiveModel = GetActiveModel();

    switch (Tool)
    {
    case LC_TOOL_SELECT:
        break;

    case LC_TOOL_PAN:
    case LC_TOOL_ROTATE_VIEW:
        ActiveModel->BeginMouseTool();
        break;

    case LC_NUM_TOOLS:
    default:
        break;
    }

    OnUpdateCursor();
}

void PreviewWidget::StopTracking(bool Accept)
{
    if (mTrackButton == lcTrackButton::None)
        return;

    lcTool Tool = GetCurrentTool();  // Either LC_TRACKTOOL_NONE (LC_TOOL_SELECT) or LC_TRACKTOOL_ORBIT_XY (LC_TOOL_ROTATE_VIEW)
    lcModel* ActiveModel = GetActiveModel();

    switch (Tool)
    {
    case LC_TOOL_SELECT:
        break;

    case LC_TOOL_PAN:
    case LC_TOOL_ROTATE_VIEW:
        ActiveModel->EndMouseTool(Tool, Accept);
        break;

    case LC_NUM_TOOLS:
    default:
        break;
    }

    mTrackButton = lcTrackButton::None;

    mTrackTool = LC_TRACKTOOL_NONE;

    OnUpdateCursor();
}

void PreviewWidget::OnButtonDown(lcTrackButton TrackButton)
{
    switch (mTrackTool)
    {
    case LC_TRACKTOOL_NONE:
        break;

    case LC_TRACKTOOL_PAN:
        StartTracking(TrackButton);
        break;

    case LC_TRACKTOOL_ORBIT_XY:
        StartTracking(TrackButton);
        break;

    case LC_TRACKTOOL_COUNT:
        break;
    }
}

lcCursor PreviewWidget::GetCursor() const
{
    const lcCursor CursorFromTrackTool[] =
    {
        lcCursor::Select,           // LC_TRACKTOOL_NONE
        lcCursor::Pan,              // LC_TRACKTOOL_PAN
        lcCursor::RotateView,       // LC_TRACKTOOL_ORBIT_XY
    };

    static_assert(LC_ARRAY_COUNT(CursorFromTrackTool) == LC_TRACKTOOL_COUNT, "Tracktool array size mismatch.");

    if (mTrackTool >= 0 && mTrackTool < LC_ARRAY_COUNT(CursorFromTrackTool))
        return CursorFromTrackTool[mTrackTool];

    return lcCursor::Select;
}

void PreviewWidget::OnInitialUpdate()
{
    MakeCurrent();

    mContext->SetDefaultState();
}

void PreviewWidget::OnDraw()
{
    if (!mModel)
        return;

    lcPreferences& Preferences = lcGetPreferences();
    const bool DrawInterface = mWidget != nullptr;

    mScene.SetAllowLOD(Preferences.mAllowLOD && mWidget != nullptr);
    mScene.SetLODDistance(Preferences.mMeshLODDistance);

    mScene.Begin(mCamera->mWorldView);

    mScene.SetDrawInterface(DrawInterface);

    mModel->GetScene(mScene, mCamera, false /*HighlightNewParts*/, false/*mFadeSteps*/);

    mScene.End();

    mContext->SetDefaultState();

    mContext->SetViewport(0, 0, mWidth, mHeight);

    mModel->DrawBackground(this);

    mContext->SetProjectionMatrix(GetProjectionMatrix());

    mContext->SetLineWidth(Preferences.mLineWidth);

    mScene.Draw(mContext);

    if (DrawInterface)
    {
        mContext->SetLineWidth(1.0f);

        if (Preferences.mDrawPreviewAxis && !mIsSubPreview)
            DrawAxes();

        if (Preferences.mDrawPreviewViewSphere && !mIsSubPreview)
            mViewSphere.Draw();
        DrawViewport();
    }

    mContext->ClearResources();
}

void PreviewWidget::OnUpdateCursor()
{
    SetCursor(GetCursor());
}

void PreviewWidget::OnLeftButtonDown()
{
    if (mTrackButton != lcTrackButton::None)
    {
        StopTracking(false);
        return;
    }

    if (mViewSphere.OnLeftButtonDown())
        return;

    lcTrackTool OverrideTool = LC_TRACKTOOL_ORBIT_XY;

    if (OverrideTool != LC_TRACKTOOL_NONE)
    {
        mTrackTool = OverrideTool;
        OnUpdateCursor();
    }

    OnButtonDown(lcTrackButton::Left);
}

void PreviewWidget::OnLeftButtonUp()
{
    StopTracking(mTrackButton == lcTrackButton::Left);

    if (mViewSphere.OnLeftButtonUp()) {
        ZoomExtents();
        return;
    }
}

void PreviewWidget::OnMiddleButtonDown()
{
    if (mTrackButton != lcTrackButton::None)
    {
        StopTracking(false);
        return;
    }

#if (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
    lcTrackTool OverrideTool = LC_TRACKTOOL_NONE;

    if (OverrideTool != LC_TRACKTOOL_NONE)
    {
        mTrackTool = OverrideTool;
        OnUpdateCursor();
    }
#endif
    OnButtonDown(lcTrackButton::Middle);
}

void PreviewWidget::OnMiddleButtonUp()
{
    StopTracking(mTrackButton == lcTrackButton::Middle);
}

void PreviewWidget::OnLeftButtonDoubleClick()
{
    ZoomExtents();
    Redraw();
}

void PreviewWidget::OnRightButtonDown()
{
    if (mTrackButton != lcTrackButton::None)
    {
        StopTracking(false);
        return;
    }

    lcTrackTool OverrideTool = LC_TRACKTOOL_PAN;

    if (OverrideTool != LC_TRACKTOOL_NONE)
    {
        mTrackTool = OverrideTool;
        OnUpdateCursor();
    }

    OnButtonDown(lcTrackButton::Middle);
}

void PreviewWidget::OnRightButtonUp()
{
    if (mTrackButton != lcTrackButton::None)
        StopTracking(mTrackButton == lcTrackButton::Right);
}

void PreviewWidget::OnMouseMove()
{
    lcModel* ActiveModel = GetActiveModel();

    if (!ActiveModel)
        return;

    if (mTrackButton == lcTrackButton::None)
    {
        if (mViewSphere.OnMouseMove())
        {
            lcTrackTool NewTrackTool = mViewSphere.IsDragging() ? LC_TRACKTOOL_ORBIT_XY : LC_TRACKTOOL_NONE;

            if (NewTrackTool != mTrackTool)
            {
                mTrackTool = NewTrackTool;
                OnUpdateCursor();
            }

            return;
        }

        return;
    }

    mTrackUpdated = true;
    const float MouseSensitivity = 0.5f / (21.0f - lcGetPreferences().mMouseSensitivity);

    switch (mTrackTool)
    {
    case LC_TRACKTOOL_NONE:
        break;

    case LC_TRACKTOOL_PAN:
        {
            lcVector3 Points[4] =
            {
                lcVector3((float)mInputState.x, (float)mInputState.y, 0.0f),
                lcVector3((float)mInputState.x, (float)mInputState.y, 1.0f),
                lcVector3(mMouseDownX, mMouseDownY, 0.0f),
                lcVector3(mMouseDownX, mMouseDownY, 1.0f)
            };

            UnprojectPoints(Points, 4);

            const lcVector3& CurrentStart = Points[0];
            const lcVector3& CurrentEnd = Points[1];
            const lcVector3& MouseDownStart = Points[2];
            const lcVector3& MouseDownEnd = Points[3];
            lcVector3 Center = ActiveModel->GetSelectionOrModelCenter();

            lcVector3 PlaneNormal(mCamera->mPosition - mCamera->mTargetPosition);
            lcVector4 Plane(PlaneNormal, -lcDot(PlaneNormal, Center));
            lcVector3 Intersection, MoveStart;

            if (!lcLineSegmentPlaneIntersection(&Intersection, CurrentStart, CurrentEnd, Plane) || !lcLineSegmentPlaneIntersection(&MoveStart, MouseDownStart, MouseDownEnd, Plane))
            {
                Center = MouseDownStart + lcNormalize(MouseDownEnd - MouseDownStart) * 10.0f;
                Plane = lcVector4(PlaneNormal, -lcDot(PlaneNormal, Center));

                if (!lcLineSegmentPlaneIntersection(&Intersection, CurrentStart, CurrentEnd, Plane) || !lcLineSegmentPlaneIntersection(&MoveStart, MouseDownStart, MouseDownEnd, Plane))
                    break;
            }

            ActiveModel->UpdatePanTool(mCamera, MoveStart - Intersection);
            Redraw();
        }
    break;

    case LC_TRACKTOOL_ORBIT_XY:
        ActiveModel->UpdateOrbitTool(mCamera, 0.1f * MouseSensitivity * (mInputState.x - mMouseDownX), 0.1f * MouseSensitivity * (mInputState.y - mMouseDownY));
        Redraw();
        break;

    case LC_TRACKTOOL_COUNT:
        break;
    }
}

void PreviewWidget::OnMouseWheel(float Direction)
{
    mModel->Zoom(mCamera, (int)(((mInputState.Modifiers & Qt::ControlModifier) ? 100 : 10) * Direction));
    Redraw();
}
