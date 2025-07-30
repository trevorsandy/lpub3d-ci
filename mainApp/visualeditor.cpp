/****************************************************************************
**
** Copyright (C) 2019 - 2025 Trevor SANDY. All rights reserved.
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
 * This class encapsulates the native renderer routines.
 *
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/

#include "lpub.h"
#include "editwindow.h"
#include "lpub_preferences.h"
#include "messageboxresizable.h"
#include "step.h"
#include "metagui.h"
#include "paths.h"
#include "lpub_qtcompat.h"

#include "lc_viewwidget.h"
#include "lc_previewwidget.h"
#include "pieceinf.h"
#include "lc_library.h"
#include "project.h"
#include "lc_mainwindow.h"

#include "lc_qutils.h"
#include "lc_profile.h"
#include "light.h"
#include "camera.h"
#include "piece.h"
#include "group.h"
#include "lc_view.h"
#include "application.h"
#include "lc_partselectionwidget.h"
#include "lc_setsdatabasedialog.h"

const QStringList VisualEditChangeTriggers = QStringList()
        << QLatin1String("Deleting")           << QLatin1String("Adding Piece")
        << QLatin1String("Move")               << QLatin1String("Rotate")
        << QLatin1String("Moving")             << QLatin1String("Rotating")
        << QLatin1String("Painting")           << QLatin1String("New Model")
        << QLatin1String("Cutting")            << QLatin1String("Pasting")
        << QLatin1String("Removing Step")      << QLatin1String("Inserting Step")
        << QLatin1String("Duplicating Pieces") << QLatin1String("New Model")
        << QLatin1String("Ungrouping")         << QLatin1String("Grouping")
        << QLatin1String("Inlining")
           ;

void Gui::create3DActions()
{
    gMainWindow->mActions[LC_VIEW_PREFERENCES]->setObjectName("PreferencesAct.4");
    lpub->actions.insert("PreferencesAct.4", Action(QStringLiteral("3DViewer.Preferences"), gMainWindow->mActions[LC_VIEW_PREFERENCES]));

    gMainWindow->mActions[LC_EDIT_TRANSFORM_RELATIVE_TRANSLATION]->setObjectName("EditRelativeTranslationAct.4");
    lpub->actions.insert("EditRelativeTranslationAct.4", Action(QStringLiteral("3DViewer.Tools.Transform.Relative Translation"), gMainWindow->mActions[LC_EDIT_TRANSFORM_RELATIVE_TRANSLATION]));

    gMainWindow->mActions[LC_EDIT_TRANSFORM_ABSOLUTE_TRANSLATION]->setObjectName("EditAbsoluteTranslationAct.4");
    lpub->actions.insert("EditAbsoluteTranslationAct.4", Action(QStringLiteral("3DViewer.Tools.Transform.Absolute Translation"), gMainWindow->mActions[LC_EDIT_TRANSFORM_ABSOLUTE_TRANSLATION]));

    gMainWindow->mActions[LC_EDIT_TRANSFORM_RELATIVE_ROTATION]->setObjectName("EditRelativeRotationAct.4");
    lpub->actions.insert("EditRelativeRotationAct.4", Action(QStringLiteral("3DViewer.Tools.Transform.Relative Rotation"), gMainWindow->mActions[LC_EDIT_TRANSFORM_RELATIVE_ROTATION]));

    gMainWindow->mActions[LC_EDIT_TRANSFORM_ABSOLUTE_ROTATION]->setObjectName("EditAbsoluteRotationAct.4");
    lpub->actions.insert("EditAbsoluteRotationAct.4", Action(QStringLiteral("3DViewer.Tools.Transform.Absolute Rotation"), gMainWindow->mActions[LC_EDIT_TRANSFORM_ABSOLUTE_ROTATION]));

    gMainWindow->mActions[LC_EDIT_ACTION_POINT_LIGHT]->setObjectName("PointLightAct.4");
    lpub->actions.insert("PointLightAct.4", Action(QStringLiteral("3DViewer.Tools.Lights.PointLight"), gMainWindow->mActions[LC_EDIT_ACTION_POINT_LIGHT]));

    gMainWindow->mActions[LC_EDIT_ACTION_DIRECTIONAL_LIGHT]->setObjectName("SunLightAct.4");
    lpub->actions.insert("SunLightAct.4", Action(QStringLiteral("3DViewer.Tools.Lights.SunLight"), gMainWindow->mActions[LC_EDIT_ACTION_DIRECTIONAL_LIGHT]));

    gMainWindow->mActions[LC_EDIT_ACTION_SPOTLIGHT]->setObjectName("SpotLightAct.4");
    lpub->actions.insert("SpotLightAct.4", Action(QStringLiteral("3DViewer.Tools.Lights.SpotLight"), gMainWindow->mActions[LC_EDIT_ACTION_SPOTLIGHT]));

    gMainWindow->mActions[LC_EDIT_ACTION_AREA_LIGHT]->setObjectName("AreaLightAct.4");
    lpub->actions.insert("AreaLightAct.4", Action(QStringLiteral("3DViewer.Tools.Lights.AreaLight"), gMainWindow->mActions[LC_EDIT_ACTION_AREA_LIGHT]));

    gMainWindow->mActions[LC_FILE_SAVEAS]->setObjectName("SaveAsAct.4");
    lpub->actions.insert("SaveAsAct.4", Action(QStringLiteral("3DViewer.Save As.LDraw"), gMainWindow->mActions[LC_FILE_SAVEAS]));

    gMainWindow->mActions[LC_FILE_SAVE_IMAGE]->setObjectName("SaveImageAct.4");
    lpub->actions.insert("SaveImageAct.4", Action(QStringLiteral("3DViewer.Save Image"), gMainWindow->mActions[LC_FILE_SAVE_IMAGE]));

    gMainWindow->mActions[LC_FILE_EXPORT_3DS]->setObjectName("Export3dsAct.4");
    lpub->actions.insert("Export3dsAct.4", Action(QStringLiteral("3DViewer.Export As.3DS"), gMainWindow->mActions[LC_FILE_EXPORT_3DS]));

    gMainWindow->mActions[LC_FILE_EXPORT_COLLADA]->setObjectName("ExportColladaAct.4");
    lpub->actions.insert("ExportColladaAct.4", Action(QStringLiteral("3DViewer.Export As.COLLADA"), gMainWindow->mActions[LC_FILE_EXPORT_COLLADA]));

    gMainWindow->mActions[LC_FILE_EXPORT_WAVEFRONT]->setObjectName("ExportWavefrontAct.4");
    lpub->actions.insert("ExportWavefrontAct.4", Action(QStringLiteral("3DViewer.Export As.Wavefront"), gMainWindow->mActions[LC_FILE_EXPORT_WAVEFRONT]));

    gMainWindow->mActions[LC_EDIT_SNAP_MOVE_TOGGLE]->setObjectName("EditSnapMoveToggleAct.4");
    lpub->actions.insert("EditSnapMoveToggleAct.4", Action(QStringLiteral("3DViewer.Tools.Snap.Move Snap Enabled"), gMainWindow->mActions[LC_EDIT_SNAP_MOVE_TOGGLE]));

    gMainWindow->mActions[LC_EDIT_SNAP_ANGLE_TOGGLE]->setObjectName("EditSnapAngleToggleAct.4");
    lpub->actions.insert("EditSnapAngleToggleAct.4", Action(QStringLiteral("3DViewer.Tools.Snap.Rotation Snap Enabled"), gMainWindow->mActions[LC_EDIT_SNAP_ANGLE_TOGGLE]));

    gMainWindow->mActions[LC_EDIT_CUT]->setObjectName("EditCutAct.4");
    lpub->actions.insert("EditCutAct.4", Action(QStringLiteral("3DViewer.Cut"), gMainWindow->mActions[LC_EDIT_CUT]));

    gMainWindow->mActions[LC_EDIT_COPY]->setObjectName("EditCopyAct.4");
    lpub->actions.insert("EditCopyAct.4", Action(QStringLiteral("3DViewer.Copy"), gMainWindow->mActions[LC_EDIT_COPY]));

    gMainWindow->mActions[LC_EDIT_PASTE]->setObjectName("EditPasteAct.4");
    lpub->actions.insert("EditPasteAct.4", Action(QStringLiteral("3DViewer.Paste"), gMainWindow->mActions[LC_EDIT_PASTE]));

    gMainWindow->mActions[LC_PIECE_DELETE]->setObjectName("PiectDeleteAct.4");
    lpub->actions.insert("PiectDeleteAct.4", Action(QStringLiteral("3DViewer.Delete"), gMainWindow->mActions[LC_PIECE_DELETE]));

    gMainWindow->mActions[LC_EDIT_FIND]->setObjectName("EditFindAct.4");
    lpub->actions.insert("EditFindAct.4", Action(QStringLiteral("3DViewer.Find"), gMainWindow->mActions[LC_EDIT_FIND]));

    gMainWindow->mActions[LC_EDIT_FIND_NEXT]->setObjectName("EditFindNextAct.4");
    lpub->actions.insert("EditFindNextAct.4", Action(QStringLiteral("3DViewer.Find Next"), gMainWindow->mActions[LC_EDIT_FIND_NEXT]));

    gMainWindow->mActions[LC_EDIT_FIND_PREVIOUS]->setObjectName("EditFindPreviousAct.4");
    lpub->actions.insert("EditFindPreviousAct.4", Action(QStringLiteral("3DViewer.Find Previous"), gMainWindow->mActions[LC_EDIT_FIND_PREVIOUS]));

    gMainWindow->mActions[LC_EDIT_CANCEL]->setObjectName("EditCancelAct.4");
    lpub->actions.insert("EditCancelAct.4", Action(QStringLiteral("3DViewer.Cancel"), gMainWindow->mActions[LC_EDIT_CANCEL]));

    gMainWindow->mActions[LC_PIECE_PAINT_SELECTED]->setObjectName("PaintSelectedAct.4");
    lpub->actions.insert("PaintSelectedAct.4", Action(QStringLiteral("3DViewer.Paint Selected"), gMainWindow->mActions[LC_PIECE_PAINT_SELECTED]));

    gMainWindow->mActions[LC_PIECE_RESET_PIVOT_POINT]->setObjectName("ResetPivotPointAct.4");
    lpub->actions.insert("ResetPivotPointAct.4", Action(QStringLiteral("3DViewer.Reset Pivot Point"), gMainWindow->mActions[LC_PIECE_RESET_PIVOT_POINT]));

    gMainWindow->mActions[LC_PIECE_EDIT_SELECTED_SUBMODEL]->setObjectName("EditSelectedSubmodelAct.4");
    lpub->actions.insert("EditSelectedSubmodelAct.4", Action(QStringLiteral("3DViewer.Edit Selected Submodel"), gMainWindow->mActions[LC_PIECE_EDIT_SELECTED_SUBMODEL]));

    gMainWindow->mActions[LC_PIECE_EDIT_END_SUBMODEL]->setObjectName("EndSubmodelEditAct.4");
    lpub->actions.insert("EndSubmodelEditAct.4", Action(QStringLiteral("3DViewer.End Submodel Editing"), gMainWindow->mActions[LC_PIECE_EDIT_END_SUBMODEL]));

    gMainWindow->mActions[LC_PIECE_INLINE_SELECTED_MODELS]->setObjectName("InlineSelectedSubmodelAct.4");
    lpub->actions.insert("InlineSelectedSubmodelAct.4", Action(QStringLiteral("3DViewer.Inline Selected Submodel"), gMainWindow->mActions[LC_PIECE_INLINE_SELECTED_MODELS]));

    gMainWindow->mActions[LC_MODEL_LIST]->setObjectName("SubmodelListAct.4");
    lpub->actions.insert("SubmodelListAct.4", Action(QStringLiteral("3DViewer.Submodel List"), gMainWindow->mActions[LC_MODEL_LIST]));

    gMainWindow->mActions[LC_MODEL_PROPERTIES]->setObjectName("SubmodelPropertiesAct.4");
    lpub->actions.insert("SubmodelPropertiesAct.4", Action(QStringLiteral("3DViewer.Submodel Properties"), gMainWindow->mActions[LC_MODEL_PROPERTIES]));

    gMainWindow->mActions[LC_EDIT_ACTION_RESET_TRANSFORM]->setObjectName("EditResetTransformAct.4");
    lpub->actions.insert("EditResetTransformAct.4", Action(QStringLiteral("3DViewer.Tools.Transform.Apply Transform"), gMainWindow->mActions[LC_EDIT_ACTION_RESET_TRANSFORM]));

    gMainWindow->mActions[LC_PIECE_HIDE_SELECTED]->setObjectName("HideSelectedAct.4");
    lpub->actions.insert("HideSelectedAct.4", Action(QStringLiteral("3DViewer.Hide Selected"), gMainWindow->mActions[LC_PIECE_HIDE_SELECTED]));

    gMainWindow->mActions[LC_PIECE_UNHIDE_SELECTED]->setObjectName("UnhideSelectedAct.4");
    lpub->actions.insert("UnhideSelectedAct.4", Action(QStringLiteral("3DViewer.Unhide Selected"), gMainWindow->mActions[LC_PIECE_UNHIDE_SELECTED]));

    gMainWindow->mActions[LC_PIECE_HIDE_UNSELECTED]->setObjectName("HideUnselectedAct.4");
    lpub->actions.insert("HideUnselectedAct.4", Action(QStringLiteral("3DViewer.Hide Unselected"), gMainWindow->mActions[LC_PIECE_HIDE_UNSELECTED]));

    gMainWindow->mActions[LC_PIECE_UNHIDE_ALL]->setObjectName("UnhideAllAct.4");
    lpub->actions.insert("UnhideAllAct.4", Action(QStringLiteral("3DViewer.Unhide All"), gMainWindow->mActions[LC_PIECE_UNHIDE_ALL]));

    gMainWindow->mActions[LC_EDIT_ACTION_SELECT]->setObjectName("EditSelectAct.4");
    lpub->actions.insert("EditSelectAct.4", Action(QStringLiteral("3DViewer.Tools.Select"), gMainWindow->mActions[LC_EDIT_ACTION_SELECT]));

    gMainWindow->mActions[LC_EDIT_ACTION_ROTATE]->setObjectName("EditRotateAct.4");
    lpub->actions.insert("EditRotateAct.4", Action(QStringLiteral("3DViewer.Tools.Rotate"), gMainWindow->mActions[LC_EDIT_ACTION_ROTATE]));

    gMainWindow->mActions[LC_EDIT_ACTION_MOVE]->setObjectName("EditMoveAct.4");
    lpub->actions.insert("EditMoveAct.4", Action(QStringLiteral("3DViewer.Tools.Move"), gMainWindow->mActions[LC_EDIT_ACTION_MOVE]));

    gMainWindow->mActions[LC_EDIT_ACTION_DELETE]->setObjectName("EditDeleteAct.4");
    lpub->actions.insert("EditDeleteAct.4", Action(QStringLiteral("3DViewer.Tools.Delete"), gMainWindow->mActions[LC_EDIT_ACTION_DELETE]));

    gMainWindow->mActions[LC_EDIT_ACTION_PAINT]->setObjectName("EditPaintAct.4");
    lpub->actions.insert("EditPaintAct.4", Action(QStringLiteral("3DViewer.Tools.Paint"), gMainWindow->mActions[LC_EDIT_ACTION_PAINT]));

    gMainWindow->mActions[LC_EDIT_ACTION_COLOR_PICKER]->setObjectName("EditColorPickerAct.4");
    lpub->actions.insert("EditColorPickerAct.4", Action(QStringLiteral("3DViewer.Tools.Color Picker"), gMainWindow->mActions[LC_EDIT_ACTION_COLOR_PICKER]));

    gMainWindow->mActions[LC_EDIT_ACTION_ROTATESTEP]->setObjectName("EditRotateStepAct.4");
    lpub->actions.insert("EditRotateStepAct.4", Action(QStringLiteral("3DViewer.Tools.RotateStep.Apply ROTSTEP"), gMainWindow->mActions[LC_EDIT_ACTION_ROTATESTEP]));

    gMainWindow->mActions[LC_EDIT_ACTION_BUILD_MOD]->setObjectName("EditBuildModAct.4");
    lpub->actions.insert("EditBuildModAct.4", Action(QStringLiteral("3DViewer.Tools.Build Modification.Edit Build Modification"), gMainWindow->mActions[LC_EDIT_ACTION_BUILD_MOD]));

    gMainWindow->mActions[LC_EDIT_ACTION_INSERT]->setObjectName("EditInsertAct.4");
    lpub->actions.insert("EditInsertAct.4", Action(QStringLiteral("3DViewer.Tools.Insert"), gMainWindow->mActions[LC_EDIT_ACTION_INSERT]));

    gMainWindow->mActions[LC_EDIT_ACTION_CAMERA]->setObjectName("EditCameraAct.4");
    lpub->actions.insert("EditCameraAct.4", Action(QStringLiteral("3DViewer.Tools.Camera"), gMainWindow->mActions[LC_EDIT_ACTION_CAMERA]));

    gMainWindow->mActions[LC_VIEW_LOOK_AT]->setObjectName("ViewLookAtAct.4");
    lpub->actions.insert("ViewLookAtAct.4", Action(QStringLiteral("3DViewer.Tools.Look At"), gMainWindow->mActions[LC_VIEW_LOOK_AT]));

    gMainWindow->mActions[LC_VIEW_ZOOM_EXTENTS]->setObjectName("ViewZoomExtentsAct.4");
    lpub->actions.insert("ViewZoomExtentsAct.4", Action(QStringLiteral("3DViewer.Tools.Zoom Extents"), gMainWindow->mActions[LC_VIEW_ZOOM_EXTENTS]));

    gMainWindow->mActions[LC_EDIT_ACTION_ZOOM]->setObjectName("EditZoomAct.4");
    lpub->actions.insert("EditZoomAct.4", Action(QStringLiteral("3DViewer.Tools.Zoom"), gMainWindow->mActions[LC_EDIT_ACTION_ZOOM]));

    gMainWindow->mActions[LC_EDIT_ACTION_ZOOM_REGION]->setObjectName("EditZoomRegionAct.4");
    lpub->actions.insert("EditZoomRegionAct.4", Action(QStringLiteral("3DViewer.Tools.Zoom Region"), gMainWindow->mActions[LC_EDIT_ACTION_ZOOM_REGION]));

    gMainWindow->mActions[LC_EDIT_ACTION_ROTATE_VIEW]->setObjectName("EditRotateViewAct.4");
    lpub->actions.insert("EditRotateViewAct.4", Action(QStringLiteral("3DViewer.Tools.Rotate View"), gMainWindow->mActions[LC_EDIT_ACTION_ROTATE_VIEW]));

    gMainWindow->mActions[LC_EDIT_ACTION_PAN]->setObjectName("EditPanAct.4");
    lpub->actions.insert("EditPanAct.4", Action(QStringLiteral("3DViewer.Tools.Pan"), gMainWindow->mActions[LC_EDIT_ACTION_PAN]));

    gMainWindow->mActions[LC_VIEW_SPLIT_HORIZONTAL]->setObjectName("ViewSplitHorizontalAct.4");
    lpub->actions.insert("ViewSplitHorizontalAct.4", Action(QStringLiteral("3DViewer.Split Horizontal"), gMainWindow->mActions[LC_VIEW_SPLIT_HORIZONTAL]));

    gMainWindow->mActions[LC_VIEW_SPLIT_VERTICAL]->setObjectName("ViewSplitVerticalAct.4");
    lpub->actions.insert("ViewSplitVerticalAct.4", Action(QStringLiteral("3DViewer.Split Vertical"), gMainWindow->mActions[LC_VIEW_SPLIT_VERTICAL]));

    gMainWindow->mActions[LC_VIEW_REMOVE_VIEW]->setObjectName("ViewRemoveViewAct.4");
    lpub->actions.insert("ViewRemoveViewAct.4", Action(QStringLiteral("3DViewer.Remove View"), gMainWindow->mActions[LC_VIEW_REMOVE_VIEW]));

    gMainWindow->mActions[LC_VIEW_RESET_VIEWS]->setObjectName("ViewResetViewsAct.4");
    lpub->actions.insert("ViewResetViewsAct.4", Action(QStringLiteral("3DViewer.Reset Views"), gMainWindow->mActions[LC_VIEW_RESET_VIEWS]));

    gMainWindow->mActions[LC_VIEW_VIEWPOINT_HOME]->setObjectName("ViewViewpointsHomeAct.4");
    lpub->actions.insert("ViewViewpointsHomeAct.4", Action(QStringLiteral("3DViewer.Tools.Viewpoints.Home"), gMainWindow->mActions[LC_VIEW_VIEWPOINT_HOME]));

    gMainWindow->mActions[LC_VIEW_VIEWPOINT_LAT_LON]->setObjectName("ViewViewpointsLatLonAct.4");
    lpub->actions.insert("ViewViewpointsLatLonAct.4", Action(QStringLiteral("3DViewer.Tools.Viewpoints.Latitude Longitude"), gMainWindow->mActions[LC_VIEW_VIEWPOINT_LAT_LON]));

    gMainWindow->mActions[LC_VIEW_VIEWPOINT_FRONT]->setObjectName("ViewViewpointsFrontAct.4");
    lpub->actions.insert("ViewViewpointsFrontAct.4", Action(QStringLiteral("3DViewer.Tools.Viewpoints.Front"), gMainWindow->mActions[LC_VIEW_VIEWPOINT_FRONT]));

    gMainWindow->mActions[LC_VIEW_VIEWPOINT_BACK]->setObjectName("ViewViewpointsBackAct.4");
    lpub->actions.insert("ViewViewpointsBackAct.4", Action(QStringLiteral("3DViewer.Tools.Viewpoints.Back"), gMainWindow->mActions[LC_VIEW_VIEWPOINT_BACK]));

    gMainWindow->mActions[LC_VIEW_VIEWPOINT_LEFT]->setObjectName("ViewViewpointsLeftAct.4");
    lpub->actions.insert("ViewViewpointsLeftAct.4", Action(QStringLiteral("3DViewer.Tools.Viewpoints.Left"), gMainWindow->mActions[LC_VIEW_VIEWPOINT_LEFT]));

    gMainWindow->mActions[LC_VIEW_VIEWPOINT_RIGHT]->setObjectName("ViewViewpointsRightAct.4");
    lpub->actions.insert("ViewViewpointsRightAct.4", Action(QStringLiteral("3DViewer.Tools.Viewpoints.Right"), gMainWindow->mActions[LC_VIEW_VIEWPOINT_RIGHT]));

    gMainWindow->mActions[LC_VIEW_VIEWPOINT_TOP]->setObjectName("ViewViewpointsTopAct.4");
    lpub->actions.insert("ViewViewpointsTopAct.4", Action(QStringLiteral("3DViewer.Tools.Viewpoints.Top"), gMainWindow->mActions[LC_VIEW_VIEWPOINT_TOP]));

    gMainWindow->mActions[LC_VIEW_VIEWPOINT_BOTTOM]->setObjectName("ViewViewpointsBottomAct.4");
    lpub->actions.insert("ViewViewpointsBottomAct.4", Action(QStringLiteral("3DViewer.Tools.Viewpoints.Bottom"), gMainWindow->mActions[LC_VIEW_VIEWPOINT_BOTTOM]));

    gMainWindow->mActions[LC_HELP_ABOUT]->setObjectName("HelpAboutAct.4");
    lpub->actions.insert("HelpAboutAct.4", Action(QStringLiteral("3DViewer.About"), gMainWindow->mActions[LC_HELP_ABOUT]));

    QString Front              = QStringLiteral(":/resources/front.png");
    QString Back               = QStringLiteral(":/resources/back.png");
    QString Left               = QStringLiteral(":/resources/right.png");
    QString Right              = QStringLiteral(":/resources/left.png");
    QString Top                = QStringLiteral(":/resources/top.png");
    QString Bottom             = QStringLiteral(":/resources/bottom.png");
    QString ViewpointHome      = QStringLiteral(":/resources/veiw_viewpoint_home.png");
    QString ViewpointHome16    = QStringLiteral(":/resources/veiw_viewpoint_home_16.png");
    QString ViewpointLatLong   = QStringLiteral(":/resources/veiw_viewpoint_latitude_longitude.png");
    QString ViewpointLatLong16 = QStringLiteral(":/resources/veiw_viewpoint_latitude_longitude_16.png");
    if (Preferences::darkTheme) {
        Front                  = QStringLiteral(":/resources/front_dark.png");
        Back                   = QStringLiteral(":/resources/back_dark.png");
        Left                   = QStringLiteral(":/resources/right_dark.png");
        Right                  = QStringLiteral(":/resources/left_dark.png");
        Top                    = QStringLiteral(":/resources/top_dark.png");
        Bottom                 = QStringLiteral(":/resources/bottom_dark.png");
        ViewpointHome          = QStringLiteral(":/resources/veiw_viewpoint_home_dark.png");
        ViewpointHome16        = QStringLiteral(":/resources/veiw_viewpoint_home_dark_16.png");
        ViewpointLatLong       = QStringLiteral(":/resources/veiw_viewpoint_latitude_longitude_dark.png");
        ViewpointLatLong16     = QStringLiteral(":/resources/veiw_viewpoint_latitude_longitude_dark_16.png");
    }

    gMainWindow->mActions[LC_EDIT_CANCEL]->setIcon(QIcon(":/resources/action_edit_cancel.png"));

    QIcon PaintSelectedIcon;
    PaintSelectedIcon.addFile(":/resources/action_paint_selected.png");
    PaintSelectedIcon.addFile(":/resources/action_paint_selected_16.png");
    gMainWindow->mActions[LC_PIECE_PAINT_SELECTED]->setIcon(PaintSelectedIcon);

    QIcon ResetPivotPointIcon;
    ResetPivotPointIcon.addFile(":/resources/resetpivotpoint.png");
    ResetPivotPointIcon.addFile(":/resources/resetpivotpoint16.png");
    gMainWindow->mActions[LC_PIECE_RESET_PIVOT_POINT]->setIcon(ResetPivotPointIcon);

    QIcon CreateBuildModIcon;
    CreateBuildModIcon.addFile(":/resources/buildmodcreate.png");
    CreateBuildModIcon.addFile(":/resources/buildmodcreate16.png");
    CreateBuildModAct = new QAction(CreateBuildModIcon,tr("Create Build Modification"),this);
    CreateBuildModAct->setObjectName("CreateBuildModAct.4");
    CreateBuildModAct->setEnabled(false);
    CreateBuildModAct->setStatusTip(tr("Create a new build modification for this step"));
    CreateBuildModAct->setShortcut(QStringLiteral("Shift+J"));
    lpub->actions.insert(CreateBuildModAct->objectName(), Action(QStringLiteral("3DViewer.Tools.Build Modification.Create Build Modification"), CreateBuildModAct));
    connect(CreateBuildModAct, SIGNAL(triggered()), this, SLOT(createBuildModification()));

    QIcon UpdateBuildModIcon;
    UpdateBuildModIcon.addFile(":/resources/buildmodupdate.png");
    UpdateBuildModIcon.addFile(":/resources/buildmodupdate16.png");
    UpdateBuildModAct = new QAction(UpdateBuildModIcon,tr("Update Build Modification"),this);
    UpdateBuildModAct->setObjectName("UpdateBuildModAct.4");
    UpdateBuildModAct->setEnabled(false);
    UpdateBuildModAct->setStatusTip(tr("Update the existing build modification in this step"));
    UpdateBuildModAct->setShortcut(QStringLiteral("Shift+K"));
    lpub->actions.insert(UpdateBuildModAct->objectName(), Action(QStringLiteral("3DViewer.Tools.Build Modification.Update Build Modification"), UpdateBuildModAct));
    connect(UpdateBuildModAct, SIGNAL(triggered()), this, SLOT(updateBuildModification()));

    QIcon ApplyBuildModIcon;
    ApplyBuildModIcon.addFile(":/resources/buildmodapply.png");
    ApplyBuildModIcon.addFile(":/resources/buildmodapply16.png");
    ApplyBuildModAct = new QAction(ApplyBuildModIcon,tr("Apply Build Modification..."),this);
    ApplyBuildModAct->setObjectName("ApplyBuildModAct.4");
    ApplyBuildModAct->setEnabled(false);
    ApplyBuildModAct->setStatusTip(tr("Add build modification apply action command to this step"));
    lpub->actions.insert(ApplyBuildModAct->objectName(), Action(QStringLiteral("3DViewer.Tools.Build Modification.Apply Build Modification"), ApplyBuildModAct));
    connect(ApplyBuildModAct, SIGNAL(triggered()), this, SLOT(applyBuildModification()));

    QIcon RemoveBuildModIcon;
    RemoveBuildModIcon.addFile(":/resources/buildmodremove.png");
    RemoveBuildModIcon.addFile(":/resources/buildmodremove16.png");
    RemoveBuildModAct = new QAction(RemoveBuildModIcon,tr("Remove Build Modification..."),this);
    RemoveBuildModAct->setObjectName("RemoveBuildModAct.4");
    RemoveBuildModAct->setEnabled(false);
    RemoveBuildModAct->setStatusTip(tr("Add build modification remove action command to this step"));
    lpub->actions.insert(RemoveBuildModAct->objectName(), Action(QStringLiteral("3DViewer.Tools.Build Modification.Remove Build Modification"), RemoveBuildModAct));
    connect(RemoveBuildModAct, SIGNAL(triggered()), this, SLOT(removeBuildModification()));

    QIcon DeleteBuildModActionIcon;
    DeleteBuildModActionIcon.addFile(":/resources/buildmodactiondelete.png");
    DeleteBuildModActionIcon.addFile(":/resources/buildmodactiondelete16.png");
    DeleteBuildModActionAct = new QAction(DeleteBuildModActionIcon,tr("Delete Build Modification Action..."),this);
    DeleteBuildModActionAct->setObjectName("DeleteBuildModActionAct.4");
    DeleteBuildModActionAct->setEnabled(false);
    DeleteBuildModActionAct->setStatusTip(tr("Delete build modification action command from this step"));
    lpub->actions.insert(DeleteBuildModActionAct->objectName(), Action(QStringLiteral("3DViewer.Tools.Build Modification.Delete Action"), DeleteBuildModActionAct));
    connect(DeleteBuildModActionAct, SIGNAL(triggered()), this, SLOT(deleteBuildModificationAction()));

    QIcon LoadBuildModIcon;
    LoadBuildModIcon.addFile(":/resources/buildmodload.png");
    LoadBuildModIcon.addFile(":/resources/buildmodload16.png");
    LoadBuildModAct = new QAction(LoadBuildModIcon,tr("Load Build Modification..."),this);
    LoadBuildModAct->setObjectName("LoadBuildModAct.4");
    LoadBuildModAct->setEnabled(false);
    LoadBuildModAct->setStatusTip(tr("Load the step containing the selected build modification"));
    lpub->actions.insert(LoadBuildModAct->objectName(), Action(QStringLiteral("3DViewer.Tools.Build Modification.Load Build Modification"), LoadBuildModAct));
    connect(LoadBuildModAct, SIGNAL(triggered()), this, SLOT(loadBuildModification()));

    QIcon DeleteBuildModIcon;
    DeleteBuildModIcon.addFile(":/resources/buildmoddelete.png");
    DeleteBuildModIcon.addFile(":/resources/buildmoddelete16.png");
    DeleteBuildModAct = new QAction(DeleteBuildModIcon,tr("Delete Build Modification..."),this);
    DeleteBuildModAct->setObjectName("DeleteBuildModAct.4");
    DeleteBuildModAct->setEnabled(false);
    DeleteBuildModAct->setStatusTip(tr("Delete selected build modification meta commands"));
    lpub->actions.insert(DeleteBuildModAct->objectName(), Action(QStringLiteral("3DViewer.Tools.Build Modification.Delete"), DeleteBuildModAct));
    connect(DeleteBuildModAct, SIGNAL(triggered()), this, SLOT(deleteBuildModification()));

    QIcon ApplyLightIcon;
    ApplyLightIcon.addFile(":/resources/applylightsettings.png");
    ApplyLightIcon.addFile(":/resources/applylightsettings_16.png");
    ApplyLightAct = new QAction(ApplyLightIcon,tr("Save Light Settings"),this);
    ApplyLightAct->setObjectName("ApplyLightAct.4");
    ApplyLightAct->setEnabled(false);
    ApplyLightAct->setStatusTip(tr("Save light settings to current step"));
    ApplyLightAct->setShortcut(QStringLiteral("Shift+I"));
    lpub->actions.insert(ApplyLightAct->objectName(), Action(QStringLiteral("3DViewer.Tools.Lights.Save Light Settings"), ApplyLightAct));
    connect(ApplyLightAct, SIGNAL(triggered()), this, SLOT(applyLightSettings()));

    QIcon SnapshotIcon;
    SnapshotIcon.addFile(":/resources/snapshot.png");
    SnapshotIcon.addFile(":/resources/snapshot16.png");
    SnapshotAct = new QAction(SnapshotIcon,tr("Snapshot"),this);
    SnapshotAct->setObjectName("SnapshotAct.4");
    SnapshotAct->setStatusTip(tr("Generate current image snapshot and save camera meta commands"));
    SnapshotAct->setShortcut(QStringLiteral("Shift+A"));
    lpub->actions.insert(SnapshotAct->objectName(), Action(QStringLiteral("3DViewer.Tools.Camera.Take Snapshot"), SnapshotAct));
    connect(SnapshotAct, SIGNAL(triggered()), this, SLOT(applyCameraSettings()));

    UseImageSizeAct = new QAction(tr("Use Image Size"),this);
    UseImageSizeAct->setObjectName("UseImageSizeAct.4");
    UseImageSizeAct->setStatusTip(tr("Use image width and height - you can also edit width and height in camera Properties tab"));
    UseImageSizeAct->setCheckable(true);
    UseImageSizeAct->setChecked(lcGetProfileInt(LC_PROFILE_USE_IMAGE_SIZE));
    lpub->actions.insert(UseImageSizeAct->objectName(), Action(QStringLiteral("3DViewer.Tools.Camera.Use Image Size"), UseImageSizeAct));
    connect(UseImageSizeAct, SIGNAL(triggered()), this, SLOT(useImageSize()));

    AutoCenterSelectionAct = new QAction(tr("Look At Selection"),this);
    AutoCenterSelectionAct->setObjectName("AutoCenterSelectionAct.4");
    AutoCenterSelectionAct->setStatusTip(tr("Automatically rotate view so selected pieces are at center"));
    AutoCenterSelectionAct->setCheckable(true);
    AutoCenterSelectionAct->setChecked(lcGetProfileInt(LC_PROFILE_AUTO_CENTER_SELECTION));
    lpub->actions.insert(AutoCenterSelectionAct->objectName(), Action(QStringLiteral("3DViewer.Tools.Camera.Look At Selection"), AutoCenterSelectionAct));
    connect(AutoCenterSelectionAct, SIGNAL(triggered()), this, SLOT(autoCenterSelection()));

    DefaultCameraPropertiesAct = new QAction(tr("Display Properties"),this);
    DefaultCameraPropertiesAct->setObjectName("DefaultCameraPropertiesAct.4");
    DefaultCameraPropertiesAct->setStatusTip(tr("Display default camera properties in Properties tab"));
    DefaultCameraPropertiesAct->setCheckable(true);
    DefaultCameraPropertiesAct->setChecked(GetPreferences().mDefaultCameraProperties);
    lpub->actions.insert(DefaultCameraPropertiesAct->objectName(), Action(QStringLiteral("3DViewer.Tools.Camera.Display Properties"), DefaultCameraPropertiesAct));
    connect(DefaultCameraPropertiesAct, SIGNAL(triggered()), this, SLOT(showDefaultCameraProperties()));

    QActionGroup* EnableRotateGroup = new QActionGroup(this);
    EnableBuildModRotateAct = new QAction(tr("Rotate Build Modification"),this);
    EnableBuildModRotateAct->setObjectName("EnableBuildModRotateAct.4");
    EnableBuildModRotateAct->setStatusTip(tr("This option allows you to choose your part selection and will transform the rotated parts"));
    EnableBuildModRotateAct->setChecked(false);
    EnableBuildModRotateAct->setCheckable(true);
    EnableRotateGroup->addAction(EnableBuildModRotateAct);
    lpub->actions.insert(EnableBuildModRotateAct->objectName(), Action(QStringLiteral("3DViewer.Tools.RotateStep.Build Modification Rotate"), EnableBuildModRotateAct));
    connect(EnableBuildModRotateAct, SIGNAL(triggered()), this, SLOT(enableVisualBuildModification()));

    EnableRotstepRotateAct = new QAction(tr("Rotate LPub ROTSTEP"),this);
    EnableRotstepRotateAct->setObjectName("EnableRotstepRotateAct.4");
    EnableRotstepRotateAct->setStatusTip(tr("This option will rotate the entire model and populate the LPub ROTSTEP command"));
    EnableRotstepRotateAct->setChecked(false);
    EnableRotstepRotateAct->setCheckable(true);
    EnableRotateGroup->addAction(EnableRotstepRotateAct);
    lpub->actions.insert(EnableRotstepRotateAct->objectName(), Action(QStringLiteral("3DViewer.Tools.RotateStep.ROTSTEP Rotate"), EnableRotstepRotateAct));
    connect(EnableRotstepRotateAct, SIGNAL(triggered()), this, SLOT(enableVisualBuildModification()));

    TransformAct = new QAction(tr("Transform"), this);
    TransformAct->setObjectName("TransformAct.4");
    TransformAct->setStatusTip(tr("Transform Options"));
    lpub->actions.insert(TransformAct->objectName(), Action(QStringLiteral("3DViewer.Tools.Transform"), TransformAct));
    TransformAct->setIcon(QIcon(":/resources/edit_transform_relative.png"));

    MovementSnapAct = new QAction(tr("Movement Snap"), this);
    MovementSnapAct->setObjectName("MoveAct.4");
    MovementSnapAct->setStatusTip(tr("Snap translations to fixed intervals"));
    MovementSnapAct->setIcon(QIcon(":/resources/edit_snap_move.png"));
    lpub->actions.insert(MovementSnapAct->objectName(), Action(QStringLiteral("3DViewer.Tools.Snap.Movement Snap"), MovementSnapAct));

    RotationSnapAct = new QAction(tr("Rotation Snap"), this);
    RotationSnapAct->setObjectName("RotationSnapAct.4");
    RotationSnapAct->setStatusTip(tr("Snap rotations to fixed intervals"));
    RotationSnapAct->setIcon(QIcon(":/resources/edit_snap_angle.png"));
    lpub->actions.insert(RotationSnapAct->objectName(), Action(QStringLiteral("3DViewer.Tools.Snap.Rotation Snap"), RotationSnapAct));

    gMainWindow->mActions[LC_PIECE_DELETE]->setIcon(QIcon(":/resources/delete.png"));

    gMainWindow->mActions[LC_FILE_SAVE_IMAGE]->setIcon(QIcon(":/resources/saveimage.png"));
    gMainWindow->mActions[LC_FILE_SAVE_IMAGE]->setShortcut(QStringLiteral("Alt+0"));
    gMainWindow->mActions[LC_FILE_SAVE_IMAGE]->setStatusTip(tr("Save an image of the current view - Alt+0"));

    gMainWindow->mActions[LC_FILE_EXPORT_3DS]->setIcon(QIcon(":/resources/3ds32.png"));
    gMainWindow->mActions[LC_FILE_EXPORT_COLLADA]->setIcon(QIcon(":/resources/dae32.png"));
    gMainWindow->mActions[LC_FILE_EXPORT_WAVEFRONT]->setIcon(QIcon(":/resources/obj32.png"));

    gMainWindow->mActions[LC_VIEW_REMOVE_VIEW]->setIcon(QIcon(":/resources/remove_view.png"));
    gMainWindow->mActions[LC_VIEW_RESET_VIEWS]->setIcon(QIcon(":/resources/reset_view.png"));
    gMainWindow->mActions[LC_VIEW_PREFERENCES]->setIcon(QIcon(":/resources/view_preferences.png"));

    gMainWindow->mActions[LC_HELP_HOMEPAGE]->setIcon(QIcon(":/resources/help_homepage.png"));
    gMainWindow->mActions[LC_HELP_ABOUT]->setIcon(QIcon(":/resources/leocad32.png"));

    gMainWindow->mActions[LC_EDIT_TRANSFORM_RELATIVE_TRANSLATION]->setIcon(QIcon(":/resources/edit_transform_absolute_translation.png"));
    gMainWindow->mActions[LC_EDIT_TRANSFORM_ABSOLUTE_TRANSLATION]->setIcon(QIcon(":/resources/edit_transform_relative_translation.png"));
    gMainWindow->mActions[LC_EDIT_TRANSFORM_RELATIVE_ROTATION]->setIcon(QIcon(":/resources/edit_transform_absolute_rotation.png"));
    gMainWindow->mActions[LC_EDIT_TRANSFORM_ABSOLUTE_ROTATION]->setIcon(QIcon(":/resources/edit_transform_relative_rotation.png"));

    QIcon FileSaveAsIcon;
    FileSaveAsIcon.addFile(":/resources/editldraw.png");
    FileSaveAsIcon.addFile(":/resources/editldraw16.png");
    gMainWindow->mActions[LC_FILE_SAVEAS]->setIcon(FileSaveAsIcon);

    QIcon EditToolsIcon;
    EditToolsIcon.addFile(":/resources/edittools.png");
    EditToolsIcon.addFile(":/resources/edittools16.png");
    gMainWindow->GetToolsMenu()->setIcon(EditToolsIcon);

    QIcon ViewCamerasIcon;
    ViewCamerasIcon.addFile(":/resources/viewcameras.png");
    ViewCamerasIcon.addFile(":/resources/viewcameras16.png");
    gMainWindow->GetCameraMenu()->setIcon(ViewCamerasIcon);

    QIcon ViewProjectionIcon;
    ViewProjectionIcon.addFile(":/resources/viewprojection.png");
    ViewProjectionIcon.addFile(":/resources/viewprojection16.png");
    gMainWindow->GetProjectionMenu()->setIcon(ViewProjectionIcon);

    QIcon ViewShadersIcon;
    ViewShadersIcon.addFile(":/resources/viewshaders.png");
    ViewShadersIcon.addFile(":/resources/viewshaders16.png");
    gMainWindow->GetShadingMenu()->setIcon(ViewShadersIcon);

    QIcon PieceIcon;
    PieceIcon.addFile(":/resources/previewpart.png");
    PieceIcon.addFile(":/resources/previewpart16.png");
    gMainWindow->GetPieceMenu()->setIcon(PieceIcon);

    QIcon EditIcon;
    EditIcon.addFile(":/resources/editplisubstituteparts.png");
    EditIcon.addFile(":/resources/editplisubstituteparts16.png");
    gMainWindow->GetEditMenu()->setIcon(EditIcon);

    QIcon EditActionBuildModIcon;
    EditActionBuildModIcon.addFile(":/resources/buildmodcreate.png");
    EditActionBuildModIcon.addFile(":/resources/buildmodcreate16.png");
    gMainWindow->mActions[LC_EDIT_ACTION_BUILD_MOD]->setIcon(EditActionBuildModIcon);

    QIcon EditActionReplaceIcon;
    EditActionReplaceIcon.addFile(":/resources/replace.png");
    EditActionReplaceIcon.addFile(":/resources/replace16.png");
    gMainWindow->mActions[LC_EDIT_REPLACE]->setIcon(EditActionReplaceIcon);

    QIcon EditActionSelectAllIcon;
    EditActionSelectAllIcon.addFile(":/resources/select_all.png");
    EditActionSelectAllIcon.addFile(":/resources/select_all16.png");
    gMainWindow->mActions[LC_EDIT_SELECT_ALL]->setIcon(EditActionSelectAllIcon);

    QIcon EditActionSelectNoneIcon;
    EditActionSelectNoneIcon.addFile(":/resources/select_none.png");
    EditActionSelectNoneIcon.addFile(":/resources/select_none16.png");
    gMainWindow->mActions[LC_EDIT_SELECT_NONE]->setIcon(EditActionSelectNoneIcon);

    QIcon EditActionSelectInvertIcon;
    EditActionSelectInvertIcon.addFile(":/resources/select_invert.png");
    EditActionSelectInvertIcon.addFile(":/resources/select_invert16.png");
    gMainWindow->mActions[LC_EDIT_SELECT_INVERT]->setIcon(EditActionSelectInvertIcon);

    QIcon EditActionSelectByNameIcon;
    EditActionSelectByNameIcon.addFile(":/resources/select_by_name.png");
    EditActionSelectByNameIcon.addFile(":/resources/select_by_name16.png");
    gMainWindow->mActions[LC_EDIT_SELECT_BY_NAME]->setIcon(EditActionSelectByNameIcon);

    QIcon ViewViewPointHomeIcon;
    ViewViewPointHomeIcon.addFile(ViewpointHome);
    ViewViewPointHomeIcon.addFile(ViewpointHome16);
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_HOME]->setIcon(ViewViewPointHomeIcon);

    QIcon ViewViewPointLatLonIcon;
    ViewViewPointLatLonIcon.addFile(ViewpointLatLong);
    ViewViewPointLatLonIcon.addFile(ViewpointLatLong16);
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_LAT_LON]->setIcon(ViewViewPointLatLonIcon);

    QIcon EditActionRotstepIcon;
    EditActionRotstepIcon.addFile(":/resources/edit_rotatestep.png");
    EditActionRotstepIcon.addFile(":/resources/edit_rotatestep_16.png");
    gMainWindow->mActions[LC_EDIT_ACTION_ROTATESTEP]->setIcon(EditActionRotstepIcon);

    QIcon ResetTransformIcon;
    ResetTransformIcon.addFile(":/resources/reset_viewer_image.png");
    ResetTransformIcon.addFile(":/resources/reset_viewer_image_16.png");
    gMainWindow->mActions[LC_EDIT_ACTION_RESET_TRANSFORM]->setIcon(ResetTransformIcon);

    QIcon ResetViewerImageIcon;
    ResetViewerImageIcon.addFile(":/resources/reset_viewer_image.png");
    ResetViewerImageIcon.addFile(":/resources/reset_viewer_image_16.png");
    ResetViewerImageAct = lpub->getAct("resetViewerImageAction.1");
    ResetViewerImageAct->setIcon(ResetViewerImageIcon);
    ResetViewerImageAct->setText(tr("Reset Current Display"));
    ResetViewerImageAct->setStatusTip(tr("Reset the current step assembly display"));
    connect(ResetViewerImageAct, SIGNAL(triggered()), this, SLOT(resetViewerImage()));

    // Light icons
    LightGroupAct = new QAction(tr("Lights"), this);
    LightGroupAct->setObjectName("LightGroupAct.4");
    LightGroupAct->setToolTip(tr("Lights - Pointlight"));
    LightGroupAct->setIcon(gMainWindow->mActions[LC_EDIT_ACTION_POINT_LIGHT]->icon());
    LightGroupAct->setStatusTip(gMainWindow->mActions[LC_EDIT_ACTION_POINT_LIGHT]->statusTip());
    LightGroupAct->setProperty("CommandId", QVariant(LC_EDIT_ACTION_POINT_LIGHT));
    lpub->actions.insert(LightGroupAct->objectName(), Action(QStringLiteral("3DViewer.Tools.Lights.Lights"), LightGroupAct));

    gMainWindow->mActions[LC_EDIT_ACTION_POINT_LIGHT]->setProperty("CommandId", QVariant(LC_EDIT_ACTION_POINT_LIGHT));
    gMainWindow->mActions[LC_EDIT_ACTION_DIRECTIONAL_LIGHT]->setProperty("CommandId", QVariant(LC_EDIT_ACTION_DIRECTIONAL_LIGHT));
    gMainWindow->mActions[LC_EDIT_ACTION_SPOTLIGHT]->setProperty("CommandId", QVariant(LC_EDIT_ACTION_SPOTLIGHT));
    gMainWindow->mActions[LC_EDIT_ACTION_AREA_LIGHT]->setProperty("CommandId", QVariant(LC_EDIT_ACTION_AREA_LIGHT));

    gMainWindow->mActions[LC_EDIT_ACTION_POINT_LIGHT]->setProperty("CommandTip", QVariant("Lights - Pointlight"));
    gMainWindow->mActions[LC_EDIT_ACTION_DIRECTIONAL_LIGHT]->setProperty("CommandTip", QVariant("Lights - Sunlight"));
    gMainWindow->mActions[LC_EDIT_ACTION_SPOTLIGHT]->setProperty("CommandTip", QVariant("Lights - Spotlight"));
    gMainWindow->mActions[LC_EDIT_ACTION_AREA_LIGHT]->setProperty("CommandTip", QVariant("Lights - Arealight"));
    connect(LightGroupAct, SIGNAL(triggered()),
            this,          SLOT(  groupActionTriggered()));

    connect(gMainWindow->mActions[LC_EDIT_ACTION_POINT_LIGHT], SIGNAL(triggered()),
            this,                                              SLOT(groupActionTriggered()));

    connect(gMainWindow->mActions[LC_EDIT_ACTION_DIRECTIONAL_LIGHT],  SIGNAL(triggered()),
            this,                                                     SLOT(groupActionTriggered()));

    connect(gMainWindow->mActions[LC_EDIT_ACTION_SPOTLIGHT], SIGNAL(triggered()),
            this,                                            SLOT(groupActionTriggered()));

    connect(gMainWindow->mActions[LC_EDIT_ACTION_AREA_LIGHT], SIGNAL(triggered()),
            this,                                             SLOT(groupActionTriggered()));

    FindAndReplaceGroupAct = new QAction(gMainWindow->mActions[LC_EDIT_FIND]->text(), this);
    FindAndReplaceGroupAct->setObjectName("FindAndReplaceGroupAct.4");
    FindAndReplaceGroupAct->setToolTip(tr("Find And Replace"));
    FindAndReplaceGroupAct->setIcon(gMainWindow->mActions[LC_EDIT_FIND]->icon());
    FindAndReplaceGroupAct->setStatusTip(gMainWindow->mActions[LC_EDIT_FIND]->statusTip());
    lpub->actions.insert(FindAndReplaceGroupAct->objectName(), Action(QStringLiteral("3DViewer.FindAndReplaceGroup"), FindAndReplaceGroupAct));
    connect(FindAndReplaceGroupAct, SIGNAL(triggered()), gMainWindow->mActions[LC_EDIT_FIND], SIGNAL(triggered()));

    SelectGroupAct =  new QAction(gMainWindow->mActions[LC_EDIT_SELECT_ALL]->text(), this);
    SelectGroupAct->setObjectName("SelectGroupAct.4");
    SelectGroupAct->setToolTip(tr("Select All"));
    SelectGroupAct->setIcon(gMainWindow->mActions[LC_EDIT_SELECT_ALL]->icon());
    SelectGroupAct->setStatusTip(gMainWindow->mActions[LC_EDIT_SELECT_ALL]->statusTip());
    lpub->actions.insert(SelectGroupAct->objectName(), Action(QStringLiteral("3DViewer.SelectGroup"), SelectGroupAct));
    connect(SelectGroupAct, SIGNAL(triggered()), gMainWindow->mActions[LC_EDIT_SELECT_ALL], SIGNAL(triggered()));

    ViewpointGroupAct = new QAction(tr("Viewpoints"), this);
    ViewpointGroupAct->setObjectName("ViewpointGroupAct.4");
    ViewpointGroupAct->setToolTip(tr("Viewpoints - Home"));
    ViewpointGroupAct->setIcon(gMainWindow->mActions[LC_VIEW_VIEWPOINT_HOME]->icon());
    ViewpointGroupAct->setStatusTip(gMainWindow->mActions[LC_VIEW_VIEWPOINT_HOME]->statusTip());
    ViewpointGroupAct->setProperty("CommandId", QVariant(LC_VIEW_VIEWPOINT_HOME));
    lpub->actions.insert(ViewpointGroupAct->objectName(), Action(QStringLiteral("3DViewer.Viewpoints"), ViewpointGroupAct));

    gMainWindow->mActions[LC_VIEW_VIEWPOINT_FRONT]->setIcon(QIcon(Front));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_BACK]->setIcon(QIcon(Back));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_LEFT]->setIcon(QIcon(Left));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_RIGHT]->setIcon(QIcon(Right));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_TOP]->setIcon(QIcon(Top));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_BOTTOM]->setIcon(QIcon(Bottom));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_LAT_LON]->setIcon(QIcon(ViewpointLatLong));

    gMainWindow->mActions[LC_VIEW_VIEWPOINT_FRONT]->setProperty("CommandId", QVariant(LC_VIEW_VIEWPOINT_FRONT));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_BACK]->setProperty("CommandId", QVariant(LC_VIEW_VIEWPOINT_BACK));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_LEFT]->setProperty("CommandId", QVariant(LC_VIEW_VIEWPOINT_LEFT));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_RIGHT]->setProperty("CommandId", QVariant(LC_VIEW_VIEWPOINT_RIGHT));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_TOP]->setProperty("CommandId", QVariant(LC_VIEW_VIEWPOINT_TOP));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_BOTTOM]->setProperty("CommandId", QVariant(LC_VIEW_VIEWPOINT_BOTTOM));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_HOME]->setProperty("CommandId", QVariant(LC_VIEW_VIEWPOINT_HOME));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_LAT_LON]->setProperty("CommandId", QVariant(LC_VIEW_VIEWPOINT_LAT_LON));

    gMainWindow->mActions[LC_VIEW_VIEWPOINT_FRONT]-> setProperty("CommandTip", QVariant("Viewpoints - Front"));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_BACK]->  setProperty("CommandTip", QVariant("Viewpoints - Back"));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_LEFT]->  setProperty("CommandTip", QVariant("Viewpoints - Left"));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_RIGHT]-> setProperty("CommandTip", QVariant("Viewpoints - Right"));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_TOP]->   setProperty("CommandTip", QVariant("Viewpoints - Top"));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_BOTTOM]->setProperty("CommandTip", QVariant("Viewpoints - Bottom"));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_HOME]->  setProperty("CommandTip", QVariant("Viewpoints - Home"));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_LAT_LON]->setProperty("CommandTip", QVariant("Viewpoints - Specify Latitude Longitude"));
    connect(ViewpointGroupAct,                               SIGNAL(triggered()),
            this,                                            SLOT(groupActionTriggered()));

    connect(gMainWindow->mActions[LC_VIEW_VIEWPOINT_FRONT],  SIGNAL(triggered()),
            this,                                            SLOT(groupActionTriggered()));

    connect(gMainWindow->mActions[LC_VIEW_VIEWPOINT_BACK],   SIGNAL(triggered()),
            this,                                            SLOT(groupActionTriggered()));

    connect(gMainWindow->mActions[LC_VIEW_VIEWPOINT_LEFT],   SIGNAL(triggered()),
            this,                                            SLOT(groupActionTriggered()));

    connect(gMainWindow->mActions[LC_VIEW_VIEWPOINT_RIGHT],  SIGNAL(triggered()),
            this,                                            SLOT(groupActionTriggered()));

    connect(gMainWindow->mActions[LC_VIEW_VIEWPOINT_TOP],    SIGNAL(triggered()),
            this,                                            SLOT(groupActionTriggered()));

    connect(gMainWindow->mActions[LC_VIEW_VIEWPOINT_BOTTOM], SIGNAL(triggered()),
            this,                                            SLOT(groupActionTriggered()));

    connect(gMainWindow->mActions[LC_VIEW_VIEWPOINT_HOME],   SIGNAL(triggered()),
            this,                                            SLOT(groupActionTriggered()));

    connect(gMainWindow->mActions[LC_VIEW_VIEWPOINT_LAT_LON],SIGNAL(triggered()),
            this,                                            SLOT(groupActionTriggered()));

    ViewpointZoomExtAct = new QAction(tr("Fit To View"),this);
    ViewpointZoomExtAct->setObjectName("ViewpointZoomExtAct.4");
    ViewpointZoomExtAct->setStatusTip(tr("Specify whether to zoom all items to fit the current view"));
    ViewpointZoomExtAct->setCheckable(true);
    ViewpointZoomExtAct->setChecked(lcGetProfileInt(LC_PROFILE_VIEWPOINT_ZOOM_EXTENT));
    lpub->actions.insert(ViewpointZoomExtAct->objectName(), Action(QStringLiteral("3DViewer.Tools.Viewpoints.Fit To View"), ViewpointZoomExtAct));
    connect(ViewpointZoomExtAct, SIGNAL(triggered()), this, SLOT(groupActionTriggered()));

    viewerZoomSliderAct = new QWidgetAction(nullptr);
    viewerZoomSliderWidget = new QSlider();
    viewerZoomSliderWidget->setSingleStep(1);
    viewerZoomSliderWidget->setTickInterval(10);
    viewerZoomSliderWidget->setTickPosition(QSlider::TicksBelow);
    viewerZoomSliderWidget->setMaximum(150);
    viewerZoomSliderWidget->setMinimum(1);
    viewerZoomSliderWidget->setValue(50);
    connect(viewerZoomSliderWidget, SIGNAL(valueChanged(int)), this, SLOT(ViewerZoomSlider(int)));

    QIcon ViewZoomExtentsIcon;
    ViewZoomExtentsIcon.addFile(":/resources/viewzoomextents.png");
    ViewZoomExtentsIcon.addFile(":/resources/viewzoomextents16.png");
    gMainWindow->mActions[LC_VIEW_ZOOM_EXTENTS]->setIcon(ViewZoomExtentsIcon);

    QIcon ViewLookAtIcon;
    ViewLookAtIcon.addFile(":/resources/lookat.png");
    ViewLookAtIcon.addFile(":/resources/lookat.png");
    gMainWindow->mActions[LC_VIEW_LOOK_AT]->setIcon(ViewLookAtIcon);
    gMainWindow->mActions[LC_VIEW_LOOK_AT]->setShortcut(QStringLiteral("Shift+O"));
    gMainWindow->mActions[LC_VIEW_LOOK_AT]->setStatusTip(tr("Position camera so selection is placed at the viewport center - Shift+O"));

    lpub->saveVisualEditorTransformSettings();

    enableVisualBuildModification();
}

void Gui::create3DMenus()
{
     /*
      * These menu items are displayed under the LPub3D Visual Editor top-level menu item
     */
     ViewerExportMenu = new QMenu(tr("&Export As..."), this);
     ViewerExportMenu->setIcon(QIcon(":/resources/exportas.png"));
     ViewerExportMenu->addAction(gMainWindow->mActions[LC_FILE_EXPORT_3DS]);
     ViewerExportMenu->addAction(gMainWindow->mActions[LC_FILE_EXPORT_COLLADA]);
     ViewerExportMenu->addAction(gMainWindow->mActions[LC_FILE_EXPORT_WAVEFRONT]);

     SnapXYMenu = new QMenu(tr("Snap XY"), this);
     for (int actionIdx = LC_EDIT_SNAP_MOVE_XY0; actionIdx <= LC_EDIT_SNAP_MOVE_XY9; actionIdx++)
         SnapXYMenu->addAction(gMainWindow->mActions[actionIdx]);

     SnapZMenu = new QMenu(tr("Snap Z"), this);
     for (int actionIdx = LC_EDIT_SNAP_MOVE_Z0; actionIdx <= LC_EDIT_SNAP_MOVE_Z9; actionIdx++)
         SnapZMenu->addAction(gMainWindow->mActions[actionIdx]);

     gMainWindow->mActions[LC_EDIT_ACTION_SELECT]->setMenu(gMainWindow->GetSelectionModeMenu());

     TransformAct->setMenu(gMainWindow->GetTransformMenu());

     SnapMenu = new QMenu(tr("Snap Menu"), this);
     SnapMenu->addAction(gMainWindow->mActions[LC_EDIT_SNAP_MOVE_TOGGLE]);
     SnapMenu->addSeparator();
     SnapMenu->addMenu(SnapXYMenu);
     SnapMenu->addMenu(SnapZMenu);
     MovementSnapAct->setMenu(SnapMenu);

     SnapAngleMenu = new QMenu(tr("Snap Angle Menu"), this);
     SnapAngleMenu->addAction(gMainWindow->mActions[LC_EDIT_SNAP_ANGLE_TOGGLE]);
     SnapAngleMenu->addSeparator();
     for (int actionIdx = LC_EDIT_SNAP_ANGLE0; actionIdx <= LC_EDIT_SNAP_ANGLE9; actionIdx++)
         SnapAngleMenu->addAction(gMainWindow->mActions[actionIdx]);
     RotationSnapAct->setMenu(SnapAngleMenu);

     BuildModMenu = new QMenu(tr("Build Modification"),this);
     BuildModMenu->addAction(CreateBuildModAct);
     BuildModMenu->addSeparator();
     BuildModMenu->addAction(UpdateBuildModAct);
     BuildModMenu->addSeparator();
     BuildModMenu->addAction(ApplyBuildModAct);
     BuildModMenu->addAction(RemoveBuildModAct);
     BuildModMenu->addAction(DeleteBuildModActionAct);
     BuildModMenu->addSeparator();
     BuildModMenu->addAction(LoadBuildModAct);
     BuildModMenu->addAction(DeleteBuildModAct);
     gMainWindow->mActions[LC_EDIT_ACTION_BUILD_MOD]->setMenu(BuildModMenu);

     LightMenu = new QMenu(tr("Lights"), this);
     LightMenu->addAction(ApplyLightAct);
     LightMenu->addSeparator();
     LightMenu->addAction(gMainWindow->mActions[LC_EDIT_ACTION_POINT_LIGHT]);
     LightMenu->addAction(gMainWindow->mActions[LC_EDIT_ACTION_DIRECTIONAL_LIGHT]);
     LightMenu->addAction(gMainWindow->mActions[LC_EDIT_ACTION_SPOTLIGHT]);
     LightMenu->addAction(gMainWindow->mActions[LC_EDIT_ACTION_AREA_LIGHT]);
     LightGroupAct->setMenu(LightMenu);

     ViewpointGroupAct->setMenu(gMainWindow->GetViewpointMenu());
     gMainWindow->GetViewpointMenu()->insertAction(gMainWindow->mActions[LC_VIEW_VIEWPOINT_FRONT], ViewpointZoomExtAct);
     gMainWindow->GetViewpointMenu()->insertSeparator(gMainWindow->mActions[LC_VIEW_VIEWPOINT_FRONT]);

     FindAndReplaceMenu = new QMenu(tr("Find And Replace"),this);
     FindAndReplaceMenu->addAction(gMainWindow->mActions[LC_EDIT_FIND_NEXT]);
     FindAndReplaceMenu->addAction(gMainWindow->mActions[LC_EDIT_FIND_PREVIOUS]);
     FindAndReplaceMenu->addAction(gMainWindow->mActions[LC_EDIT_REPLACE]);
     FindAndReplaceMenu->addAction(gMainWindow->mActions[LC_EDIT_REPLACE_NEXT]);
     FindAndReplaceGroupAct->setMenu(FindAndReplaceMenu);

     SelectMenu = new QMenu(tr("Select"),this);
     SelectMenu->addAction(gMainWindow->mActions[LC_EDIT_SELECT_NONE]);
     SelectMenu->addAction(gMainWindow->mActions[LC_EDIT_SELECT_INVERT]);
     SelectMenu->addAction(gMainWindow->mActions[LC_EDIT_SELECT_BY_NAME]);
     SelectGroupAct->setMenu(SelectMenu);

     // Viewer menus
     ViewerMenu = menuBar()->addMenu(tr("&Visual Editor"));
     // Render menu
     ViewerMenu->addAction(blenderRenderAct);
     ViewerMenu->addAction(blenderImportAct);
     ViewerMenu->addAction(povrayRenderAct);
     ViewerMenu->addSeparator();
     // Save Image menu
     ViewerMenu->addAction(gMainWindow->mActions[LC_FILE_SAVE_IMAGE]);
     // Save Step As menu
     ViewerMenu->addAction(gMainWindow->mActions[LC_FILE_SAVEAS]);
     // Export As menu
     ViewerMenu->addMenu(ViewerExportMenu);
     ViewerMenu->addSeparator();
     // Submodel Edit menus
     gMainWindow->GetEditMenu()->addAction(gMainWindow->mActions[LC_EDIT_CUT]);
     gMainWindow->GetEditMenu()->addAction(gMainWindow->mActions[LC_EDIT_COPY]);
     gMainWindow->GetEditMenu()->addAction(gMainWindow->mActions[LC_EDIT_PASTE]);
     // Submodel Find menus
     gMainWindow->GetEditMenu()->addSeparator();
     gMainWindow->GetEditMenu()->addAction(gMainWindow->mActions[LC_EDIT_FIND]);
     gMainWindow->GetEditMenu()->addAction(gMainWindow->mActions[LC_EDIT_FIND_NEXT]);
     gMainWindow->GetEditMenu()->addAction(gMainWindow->mActions[LC_EDIT_FIND_PREVIOUS]);
     gMainWindow->GetEditMenu()->addAction(gMainWindow->mActions[LC_EDIT_REPLACE]);
     gMainWindow->GetEditMenu()->addAction(gMainWindow->mActions[LC_EDIT_REPLACE_NEXT]);
     // Submodel Select menus
     gMainWindow->GetEditMenu()->addSeparator();
     gMainWindow->GetEditMenu()->addAction(gMainWindow->mActions[LC_EDIT_SELECT_ALL]);
     gMainWindow->GetEditMenu()->addAction(gMainWindow->mActions[LC_EDIT_SELECT_NONE]);
     gMainWindow->GetEditMenu()->addAction(gMainWindow->mActions[LC_EDIT_SELECT_INVERT]);
     gMainWindow->GetEditMenu()->addAction(gMainWindow->mActions[LC_EDIT_SELECT_BY_NAME]);
     ViewerMenu->addMenu(gMainWindow->GetEditMenu());
     ViewerMenu->addSeparator();
     // Camera menu
     ViewerMenu->addMenu(gMainWindow->GetCameraMenu());
     // Tools menu
     gMainWindow->GetToolsMenu()->addSeparator();
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_SELECT]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_BUILD_MOD]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ROTATE]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ROTATESTEP]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_MOVE]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_INSERT]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_DELETE]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_PAINT]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_COLOR_PICKER]);
     gMainWindow->GetToolsMenu()->addSeparator();
     gMainWindow->GetToolsMenu()->addAction(SnapshotAct);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_CAMERA]);
     gMainWindow->GetToolsMenu()->addAction(LightGroupAct);
     gMainWindow->GetToolsMenu()->addSeparator();
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_PAN]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_VIEW_LOOK_AT]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_VIEW_ZOOM_EXTENTS]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ZOOM_REGION]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ZOOM]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ROTATE_VIEW]);
     gMainWindow->GetToolsMenu()->addAction(ViewpointGroupAct);
     gMainWindow->GetToolsMenu()->addAction(ResetViewerImageAct);
     gMainWindow->GetToolsMenu()->addSeparator();
     gMainWindow->GetToolsMenu()->addAction(TransformAct);
     gMainWindow->GetToolsMenu()->addAction(MovementSnapAct);
     gMainWindow->GetToolsMenu()->addAction(RotationSnapAct);
     ViewerMenu->addMenu(gMainWindow->GetToolsMenu());
     // Piece menu
     gMainWindow->GetPieceMenu()->addAction(gMainWindow->mActions[LC_PIECE_DELETE]);
     gMainWindow->GetPieceMenu()->addAction(gMainWindow->mActions[LC_PIECE_PAINT_SELECTED]);
     gMainWindow->GetPieceMenu()->addAction(gMainWindow->mActions[LC_PIECE_RESET_PIVOT_POINT]);
     gMainWindow->GetPieceMenu()->addSeparator();
     // Piece menu - Submodels
     gMainWindow->GetPieceMenu()->addAction(gMainWindow->mActions[LC_PIECE_EDIT_SELECTED_SUBMODEL]);
     gMainWindow->GetPieceMenu()->addAction(gMainWindow->mActions[LC_PIECE_EDIT_END_SUBMODEL]);
     gMainWindow->GetPieceMenu()->addAction(gMainWindow->mActions[LC_PIECE_INLINE_SELECTED_MODELS]);
     gMainWindow->GetPieceMenu()->addSeparator();
     gMainWindow->GetPieceMenu()->addAction(gMainWindow->mActions[LC_MODEL_LIST]);
     gMainWindow->GetPieceMenu()->addAction(gMainWindow->mActions[LC_MODEL_PROPERTIES]);
     gMainWindow->GetPieceMenu()->addSeparator();
     // Piece menu - Show/Hide
     gMainWindow->GetPieceMenu()->addAction(gMainWindow->mActions[LC_PIECE_HIDE_SELECTED]);
     gMainWindow->GetPieceMenu()->addAction(gMainWindow->mActions[LC_PIECE_UNHIDE_SELECTED]);
     gMainWindow->GetPieceMenu()->addAction(gMainWindow->mActions[LC_PIECE_HIDE_UNSELECTED]);
     gMainWindow->GetPieceMenu()->addAction(gMainWindow->mActions[LC_PIECE_UNHIDE_ALL]);
     ViewerMenu->addMenu(gMainWindow->GetPieceMenu());
     // ViewPoint menu
     ViewerMenu->addMenu(gMainWindow->GetViewpointMenu());
     // Projection menu
     ViewerMenu->addMenu(gMainWindow->GetProjectionMenu());
     // Shading menu
     ViewerMenu->addMenu(gMainWindow->GetShadingMenu());
     ViewerMenu->addSeparator();
     // View menus
     ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_SPLIT_HORIZONTAL]);
     ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_SPLIT_VERTICAL]);
     ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_REMOVE_VIEW]);
     ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_RESET_VIEWS]);
     ViewerMenu->addSeparator();

     // Zoom slider menu item
     connect(gMainWindow->mActions[LC_VIEW_VIEWPOINT_HOME], SIGNAL(triggered()), this, SLOT(ResetViewerZoomSlider()));
     viewerZoomSliderAct->setDefaultWidget(viewerZoomSliderWidget);
     ViewerZoomSliderMenu = new QMenu(tr("Zoom Slider"),this);
     ViewerZoomSliderMenu->addAction(viewerZoomSliderAct);
     gMainWindow->mActions[LC_EDIT_ACTION_ZOOM]->setMenu(ViewerZoomSliderMenu);
     ViewerMenu->addSeparator();
     // Visual Editor Preferences menu
     ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_PREFERENCES]);
     // Visual Editor about menu
     ViewerMenu->addAction(gMainWindow->mActions[LC_HELP_ABOUT]);

     RotateStepActionMenu = new QMenu(tr("Rotate Step"),this);
     RotateStepActionMenu->addAction(EnableRotstepRotateAct);
     RotateStepActionMenu->addAction(EnableBuildModRotateAct);
     gMainWindow->mActions[LC_EDIT_ACTION_ROTATE]->setMenu(RotateStepActionMenu);

     SnapshotMenu = new QMenu(tr("Snapshot Settings"),this);
     SnapshotMenu->addAction(UseImageSizeAct);
     SnapshotMenu->addAction(AutoCenterSelectionAct);
     SnapshotAct->setMenu(SnapshotMenu);

     CameraMenu = new QMenu(tr("Camera Settings"),this);
     CameraMenu->addAction(DefaultCameraPropertiesAct);
     CameraMenu->addSeparator();
     CameraMenu->addMenu(gMainWindow->GetCameraMenu());
     gMainWindow->mActions[LC_EDIT_ACTION_CAMERA]->setMenu(CameraMenu);

     PaintMenu = new QMenu(tr("Paint"),this);
     PaintMenu->addAction(gMainWindow->mActions[LC_PIECE_PAINT_SELECTED]);
     gMainWindow->mActions[LC_EDIT_ACTION_PAINT]->setMenu(PaintMenu);
}

void Gui::create3DToolBars()
{
    /*
     * This toolbar is displayed under the LPub3D Visual Editor top-level menu item
     */
    exportToolBar->addSeparator();
    exportToolBar->addAction(blenderRenderAct);
    exportToolBar->addAction(blenderImportAct);
    exportToolBar->addAction(povrayRenderAct);
    exportToolBar->addSeparator();
    exportToolBar->addAction(gMainWindow->mActions[LC_FILE_SAVE_IMAGE]);
    exportToolBar->addAction(gMainWindow->mActions[LC_FILE_EXPORT_3DS]);
    exportToolBar->addAction(gMainWindow->mActions[LC_FILE_EXPORT_COLLADA]);
    exportToolBar->addAction(gMainWindow->mActions[LC_FILE_EXPORT_WAVEFRONT]);

    /*
     * These toolbars are displayed on the Visual Editor tab
     */
    gMainWindow->GetStandardToolBar()->addAction(gMainWindow->mActions[LC_EDIT_CUT]);
    gMainWindow->GetStandardToolBar()->addAction(gMainWindow->mActions[LC_EDIT_COPY]);
    gMainWindow->GetStandardToolBar()->addAction(gMainWindow->mActions[LC_EDIT_PASTE]);
    gMainWindow->GetStandardToolBar()->addSeparator();
    gMainWindow->GetStandardToolBar()->addAction(SelectGroupAct);
    gMainWindow->GetStandardToolBar()->addAction(FindAndReplaceGroupAct);
    gMainWindow->GetStandardToolBar()->addSeparator();
    gMainWindow->GetStandardToolBar()->addAction(TransformAct);
    gMainWindow->GetStandardToolBar()->addAction(MovementSnapAct);
    gMainWindow->GetStandardToolBar()->addAction(RotationSnapAct);

    gMainWindow->GetToolsToolBar()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_SELECT]);
    gMainWindow->GetToolsToolBar()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_BUILD_MOD]);
    gMainWindow->GetToolsToolBar()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ROTATE]);
    gMainWindow->GetToolsToolBar()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ROTATESTEP]);
    gMainWindow->GetToolsToolBar()->addSeparator();
    gMainWindow->GetToolsToolBar()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_MOVE]);
    gMainWindow->GetToolsToolBar()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_INSERT]);
    gMainWindow->GetToolsToolBar()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_DELETE]);
    gMainWindow->GetToolsToolBar()->addSeparator();
    gMainWindow->GetToolsToolBar()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_PAINT]);
    gMainWindow->GetToolsToolBar()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_COLOR_PICKER]);
    gMainWindow->GetToolsToolBar()->addSeparator();
    gMainWindow->GetToolsToolBar()->addAction(gMainWindow->mActions[LC_PIECE_RESET_PIVOT_POINT]);

    gMainWindow->GetVisualizationToolBar()->addAction(SnapshotAct);
    gMainWindow->GetVisualizationToolBar()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_CAMERA]);
    gMainWindow->GetVisualizationToolBar()->addAction(LightGroupAct);
    gMainWindow->GetVisualizationToolBar()->addSeparator();
    gMainWindow->GetVisualizationToolBar()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_PAN]);
    gMainWindow->GetVisualizationToolBar()->addAction(gMainWindow->mActions[LC_VIEW_LOOK_AT]);
    gMainWindow->GetVisualizationToolBar()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ZOOM_REGION]);
    gMainWindow->GetVisualizationToolBar()->addAction(gMainWindow->mActions[LC_VIEW_ZOOM_EXTENTS]);
    gMainWindow->GetVisualizationToolBar()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ZOOM]);
    gMainWindow->GetVisualizationToolBar()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ROTATE_VIEW]);
    gMainWindow->GetVisualizationToolBar()->addAction(ViewpointGroupAct);
    gMainWindow->GetVisualizationToolBar()->addAction(ResetViewerImageAct);

    gMainWindow->GetPartsToolBar()->setWindowTitle("Tools Toolbar");
}

bool Gui::eventFilter(QObject *object, QEvent *event)
{
    if ( object == gMainWindow)
        if (event->type() == QEvent::Show || event->type() == QEvent::Hide)
            emit visualEditorVisibleSig(gMainWindow->isVisible());
    return QMainWindow::eventFilter(object, event);
}

void Gui::initiaizeVisualEditor()
{
    connect(gMainWindow, SIGNAL(       UpdateUndoRedoSig(const QString&,const QString&)),
            this,          SLOT(UpdateVisualEditUndoRedo(const QString&,const QString&)));

    connect(gMainWindow, SIGNAL(TogglePreviewWidgetSig(bool)),
            this,        SLOT(  togglePreviewWidget(bool)));

    connect(this,        SIGNAL(clearViewerWindowSig()),
            gMainWindow, SLOT(  NewProject()));

    connect(gMainWindow, SIGNAL(SetActiveModelSig(const QString&)),
            this,          SLOT(   SetActiveModel(const QString&)));

    connect(this,        SIGNAL(setSelectedPiecesSig(const QVector<int>&)),
            gMainWindow, SLOT(  SetSelectedPieces(const QVector<int>&)));

    connect(gMainWindow, SIGNAL(SelectedPartLinesSig(const QVector<TypeLine>&,PartSource)),
            this,        SLOT(  SelectedPartLines(const QVector<TypeLine>&,PartSource)));

    connect(gMainWindow, SIGNAL(SetRotStepCommand()),
            this,        SLOT(  SetRotStepCommand()));

    connect(gMainWindow, SIGNAL(SetRotStepAngleX(float,bool)),
            this,        SLOT(  SetRotStepAngleX(float,bool)));

    connect(gMainWindow, SIGNAL(SetRotStepAngleY(float,bool)),
            this,        SLOT(  SetRotStepAngleY(float,bool)));

    connect(gMainWindow, SIGNAL(SetRotStepAngleZ(float,bool)),
            this,        SLOT(  SetRotStepAngleZ(float,bool)));

    connect(gMainWindow, SIGNAL(SetRotStepType(const QString&,bool)),
            this,        SLOT(  SetRotStepType(const QString&,bool)));

    connect(gMainWindow, SIGNAL(SetRotStepAngles(const QVector<float>&,bool)),
            this,        SLOT(  SetRotStepAngles(const QVector<float>&,bool)));

    enable3DActions(false);

    gMainWindow->installEventFilter(gui);

    emit visualEditorVisibleSig(gMainWindow->isVisible());
}

void Gui::enable3DActions(bool enable)
{
    if (enable) {
        enableVisualBuildModification();
        enableVisualBuildModActions();
        gui->RaiseVisualEditDockWindow();
    } else if (lpub->page.coverPage && lpub->page.meta.LPub.coverPageViewEnabled.value()) {
        gui->RaisePreviewDockWindow();
    }

    SnapshotAct->setEnabled(enable);
    LightGroupAct->setEnabled(enable);
    ViewpointGroupAct->setEnabled(enable);

    blenderRenderAct->setEnabled(enable);
    blenderImportAct->setEnabled(enable);
    povrayRenderAct->setEnabled(enable);
    ViewerExportMenu->setEnabled(enable);

    ResetViewerImageAct->setEnabled(enable &&
        static_cast<Options::Mt>(lcGetActiveProject()->GetImageType()) == Options::CSI);

    GetStandardToolBar()->setEnabled(enable);
    GetVisualizationToolBar()->setEnabled(enable);
    GetToolsToolBar()->setEnabled(enable);
    GetTimelineToolBar()->setEnabled(enable);
    GetPropertiesToolBar()->setEnabled(enable);
    GetPartsToolBar()->setEnabled(enable);
    GetColorsToolBar()->setEnabled(enable);

    GetCameraMenu()->setEnabled(enable);
    GetEditMenu()->setEnabled(enable);
    GetToolsMenu()->setEnabled(enable);
    GetPieceMenu()->setEnabled(enable);
    GetViewpointMenu()->setEnabled(enable);
    GetProjectionMenu()->setEnabled(enable);
    GetShadingMenu()->setEnabled(enable);

    //Window
    gMainWindow->mActions[LC_VIEW_SPLIT_HORIZONTAL]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_SPLIT_VERTICAL]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_REMOVE_VIEW]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_RESET_VIEWS]->setEnabled(enable);
    //Edit
    gMainWindow->mActions[LC_EDIT_UNDO]->setEnabled(enable);
    gMainWindow->mActions[LC_EDIT_REDO]->setEnabled(enable);
    //File
    gMainWindow->mActions[LC_FILE_SAVE_IMAGE]->setEnabled(enable);
    gMainWindow->mActions[LC_FILE_SAVE_IMAGE]->setEnabled(enable);
    //Export
    gMainWindow->mActions[LC_FILE_EXPORT_3DS]->setEnabled(enable);
    gMainWindow->mActions[LC_FILE_EXPORT_BRICKLINK]->setEnabled(enable);
    gMainWindow->mActions[LC_FILE_EXPORT_COLLADA]->setEnabled(enable);
    gMainWindow->mActions[LC_FILE_EXPORT_CSV]->setEnabled(enable);
    gMainWindow->mActions[LC_FILE_EXPORT_HTML]->setEnabled(enable);
    gMainWindow->mActions[LC_FILE_EXPORT_POVRAY]->setEnabled(enable);
    gMainWindow->mActions[LC_FILE_EXPORT_WAVEFRONT]->setEnabled(enable);
    //Find
    gMainWindow->mActions[LC_EDIT_FIND]->setEnabled(enable);
    gMainWindow->mActions[LC_EDIT_FIND_NEXT]->setEnabled(enable);
    gMainWindow->mActions[LC_EDIT_FIND_PREVIOUS]->setEnabled(enable);
    //Tools
    //gMainWindow->mActions[LC_EDIT_ACTION_ROTATESTEP]->setEnabled(enable);
    //gMainWindow->mActions[LC_EDIT_ACTION_BUILD_MOD]->setEnabled(enable);
    //gMainWindow->mActions[LC_EDIT_ACTION_RESET_TRANSFORM]->setEnabled(enable);
    gMainWindow->mActions[LC_EDIT_ACTION_CAMERA]->setEnabled(enable);
    gMainWindow->mActions[LC_EDIT_ACTION_SELECT]->setEnabled(enable);
    gMainWindow->mActions[LC_EDIT_ACTION_ROTATE]->setEnabled(enable);
    gMainWindow->mActions[LC_EDIT_ACTION_PAN]->setEnabled(enable);
    gMainWindow->mActions[LC_EDIT_ACTION_ROTATE_VIEW]->setEnabled(enable);
    gMainWindow->mActions[LC_EDIT_ACTION_ROLL]->setEnabled(enable);
    gMainWindow->mActions[LC_EDIT_ACTION_ZOOM_REGION]->setEnabled(enable);
    gMainWindow->mActions[LC_EDIT_ACTION_MOVE]->setEnabled(enable);
    gMainWindow->mActions[LC_EDIT_ACTION_INSERT]->setEnabled(enable);
    gMainWindow->mActions[LC_EDIT_ACTION_PAINT]->setEnabled(enable);
    gMainWindow->mActions[LC_EDIT_ACTION_COLOR_PICKER]->setEnabled(enable);
    gMainWindow->mActions[LC_EDIT_ACTION_DELETE]->setEnabled(enable);
    //Shading
    gMainWindow->mActions[LC_VIEW_SHADING_WIREFRAME]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_SHADING_FLAT]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_SHADING_DEFAULT_LIGHTS]->setEnabled(enable);
    //View
    gMainWindow->mActions[LC_VIEW_ZOOM_EXTENTS]->setEnabled(enable);
    gMainWindow->mActions[LC_EDIT_ACTION_ZOOM]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_LOOK_AT]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_SPLIT_HORIZONTAL]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_SPLIT_VERTICAL]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_REMOVE_VIEW]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_RESET_VIEWS]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_FRONT]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_BACK]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_LEFT]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_RIGHT]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_TOP]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_BOTTOM]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_HOME]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_LAT_LON]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_CAMERA_NONE]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_PROJECTION_PERSPECTIVE]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_PROJECTION_ORTHO]->setEnabled(enable);
}

void Gui::halt3DViewer(bool enable)
{
    if (!gMainWindow)
        return;

    if(enable) {
        GetToolsToolBar()->setEnabled(false);
        gMainWindow->menuBar()->setEnabled(false);
    } else {
        GetToolsToolBar()->setEnabled(true);
        gMainWindow->menuBar()->setEnabled(true);
    }

    emit messageSig(LOG_INFO, QString("Exporting content - Visual Editor %1").arg(enable ? "stopped" :"resumed"));
}

void Gui::create3DDockWindows()
{
    //Timeline
    gMainWindow->GetTimelineToolBar()->setWindowTitle(tr("Timeline"));
    gMainWindow->GetTimelineToolBar()->setObjectName("TimelineToolbar");
    gMainWindow->GetTimelineToolBar()->setAllowedAreas(
                Qt::TopDockWidgetArea  | Qt::BottomDockWidgetArea |
                Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, gMainWindow->GetTimelineToolBar());
    viewMenu->addAction(gMainWindow->GetTimelineToolBar()->toggleViewAction());

    // split the space covered commandEditDockWindow into two parts, move visualEditDockWindow into the second part.
    splitDockWidget(commandEditDockWindow, gMainWindow->GetTimelineToolBar(), Qt::Vertical);

    // Preview
    if (GetPreferences().mPreviewPosition == lcPreviewPosition::Dockable)
        createPreviewWidget();

    //Properties
    gMainWindow->GetPropertiesToolBar()->setWindowTitle(tr("Properties"));
    gMainWindow->GetPropertiesToolBar()->setObjectName("PropertiesToolbar");
    gMainWindow->GetPropertiesToolBar()->setAllowedAreas(
                Qt::TopDockWidgetArea  | Qt::BottomDockWidgetArea |
                Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, gMainWindow->GetPropertiesToolBar());
    viewMenu->addAction(gMainWindow->GetPropertiesToolBar()->toggleViewAction());

    tabifyDockWidget(gMainWindow->GetTimelineToolBar(), gMainWindow->GetPropertiesToolBar());

    //Colors Selection
    gMainWindow->GetColorsToolBar()->setWindowTitle(tr("Colors"));
    gMainWindow->GetColorsToolBar()->setObjectName("ColorsToolbar");
    gMainWindow->GetColorsToolBar()->setAllowedAreas(
                Qt::TopDockWidgetArea  | Qt::BottomDockWidgetArea |
                Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, gMainWindow->GetColorsToolBar());
    viewMenu->addAction(gMainWindow->GetColorsToolBar()->toggleViewAction());

    tabifyDockWidget(gMainWindow->GetTimelineToolBar(), gMainWindow->GetColorsToolBar());

    //Part Selection
    gMainWindow->GetPartsToolBar()->setWindowTitle(tr("Parts"));
    gMainWindow->GetPartsToolBar()->setObjectName("PartsToolbar");
    gMainWindow->GetPartsToolBar()->setAllowedAreas(
                Qt::TopDockWidgetArea  | Qt::BottomDockWidgetArea |
                Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, gMainWindow->GetPartsToolBar());
    viewMenu->addAction(gMainWindow->GetPartsToolBar()->toggleViewAction());

    tabifyDockWidget(gMainWindow->GetTimelineToolBar(), gMainWindow->GetPartsToolBar());

    //Visual Editor
    visualEditDockWindow = new QDockWidget(tr("Visual Editor"), this);
    visualEditDockWindow->setObjectName("ModelDockWindow");
    visualEditDockWindow->setAllowedAreas(
                Qt::TopDockWidgetArea  | Qt::BottomDockWidgetArea |
                Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    visualEditDockWindow->setWidget(gMainWindow);
    addDockWidget(Qt::RightDockWidgetArea, visualEditDockWindow);
    viewMenu->addAction(visualEditDockWindow->toggleViewAction());

    tabifyDockWidget(gMainWindow->GetTimelineToolBar(), visualEditDockWindow);

    // Status Bar and Window Flags
    connect(visualEditDockWindow,                SIGNAL (topLevelChanged(bool)), this, SLOT (toggleLCStatusBar(bool)));
    connect(visualEditDockWindow,                SIGNAL (topLevelChanged(bool)), this, SLOT (enableWindowFlags(bool)));
    connect(gMainWindow->GetTimelineToolBar(),   SIGNAL (topLevelChanged(bool)), this, SLOT (enableWindowFlags(bool)));
    connect(gMainWindow->GetPropertiesToolBar(), SIGNAL (topLevelChanged(bool)), this, SLOT (enableWindowFlags(bool)));
    connect(gMainWindow->GetColorsToolBar(),     SIGNAL (topLevelChanged(bool)), this, SLOT (enableWindowFlags(bool)));
    connect(gMainWindow->GetPartsToolBar(),      SIGNAL (topLevelChanged(bool)), this, SLOT (enableWindowFlags(bool)));
}

bool Gui::createPreviewWidget()
{
    gMainWindow->CreatePreviewWidget();

    if (gMainWindow->GetPreviewWidget()) {
        previewDockWindow = new QDockWidget(tr("Preview"), this);
        previewDockWindow->setWindowTitle(tr("Preview"));
        previewDockWindow->setObjectName("PreviewDockWindow");
        previewDockWindow->setAllowedAreas(
                    Qt::TopDockWidgetArea  | Qt::BottomDockWidgetArea |
                    Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        previewDockWindow->setWidget(gMainWindow->GetPreviewWidget());
        addDockWidget(Qt::RightDockWidgetArea, previewDockWindow);
        viewMenu->addAction(previewDockWindow->toggleViewAction());

        tabifyDockWidget(gMainWindow->GetTimelineToolBar(), previewDockWindow);

        connect(previewDockWindow, SIGNAL (topLevelChanged(bool)), this, SLOT (enableWindowFlags(bool)));

        return true;
    } else {
        emit gui->messageSig(LOG_ERROR, tr("Preview failed."));
    }
    return false;
}

void Gui::PreviewPiece(const QString &partType, int colorCode, bool dockable, QRect parentRect, QPoint position)
{
    if (dockable) {
        if (gMainWindow)
            gMainWindow->PreviewPiece(partType, colorCode, false);
        return;
    } else {
        gui->preview = new lcPreview();
        if (gui->preview) {
            lcViewWidget* viewWidget = new lcViewWidget(nullptr, gui->preview);
            if (viewWidget) {
                viewWidget->setAttribute(Qt::WA_DeleteOnClose, true);
                if (gui->preview->SetCurrentPiece(partType, colorCode)) {
                    viewWidget->SetPreviewPosition(parentRect, position);
                    return;
                }
            }
        }
    }

    emit gui->messageSig(LOG_WARNING, tr("Part preview for '%1' failed.").arg(partType));
}

bool Gui::PreviewPiece(const QString &type, int colorCode)
{
    if (!gMainWindow)
        return false;

    // Set preview project path
    const QFileInfo typeInfo(type);
    const QFileInfo typePath(typeInfo.absolutePath());
    lcPreferences& Preferences = lcGetPreferences();
    if (typePath.exists() && typePath.isDir()) {
        Preferences.mPreviewLoadPath = QFileInfo(type).absolutePath();
    } else {
        PieceInfo* pieceInfo = lcGetPiecesLibrary()->FindPiece(typeInfo.completeBaseName().toLatin1().constData(), nullptr, false, false);
        if (! pieceInfo)
            emit gui->messageSig(LOG_ERROR, tr("Preview file path '%1' is invalid.").arg(typePath.absolutePath()));
    }

    // Load preview
    if (/*gMainWindow->GetPreviewWidget() && */Preferences.mPreviewPosition != lcPreviewPosition::Floating) {
        gMainWindow->PreviewPiece(QFileInfo(type).fileName(), colorCode, false/*UNUSED*/);
        gui->RaisePreviewDockWindow();
    }
    // Or load visual editor if preview is floating
    else {
        // Create empty project
        Project* NewProject = new Project();
        gApplication->SetProject(NewProject);
        Gui::UpdateAllViews();

        if (!gMainWindow->OpenProject(type))
            return false;
        else
            gui->RaiseVisualEditDockWindow();
    }
    return true;
}

void Gui::previewModel(QString const &modelFileName)
{
    if (Preferences::modeGUI && ! Gui::exporting()) {
        emit gui->messageSig(LOG_INFO_STATUS, tr("Render model preview..."));
        if (!gui->PreviewPiece(modelFileName, LDRAW_MATERIAL_COLOUR))
            emit gui->messageSig(LOG_WARNING, tr("Could not load cover page preview file '%1'.").arg(modelFileName));
    }
}

void Gui::updatePreview()
{
    if (gui->previewDockWindow) {
        gMainWindow->GetPreviewWidget()->UpdatePreview();
        gui->RaisePreviewDockWindow();
    } else if (gui->preview) {
        gui->preview->UpdatePreview();
    }
}

void Gui::togglePreviewWidget(bool visible)
{
    if (gui->previewDockWindow) {
        if (visible)
            gui->previewDockWindow->show();
        else
            gui->previewDockWindow->hide();
        const QList<QAction*> viewActions = gui->viewMenu->actions();
        for (QAction *viewAct : viewActions) {
            if (viewAct->text() == QLatin1String("Preview")) {
                viewAct->setChecked(visible);
                viewAct->setVisible(visible);
//* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
                emit messageSig(LOG_DEBUG, QString("%1 window %2.")
                                .arg(viewAct->text()).arg(visible ? "Displayed" : "Hidden"));
#endif
//*/
                break;
            }
        }
    } else if (visible) {
        createPreviewWidget();
    }
}

void Gui::enableWindowFlags(bool detached)
{
    if (detached) {
        QDockWidget *dockWidget = qobject_cast<QDockWidget *>(sender());
        dockWidget->setWindowFlags(Qt::CustomizeWindowHint |
                                   Qt::Window |
                                   Qt::WindowMinimizeButtonHint |
                                   Qt::WindowMaximizeButtonHint |
                                   Qt::WindowCloseButtonHint);
        dockWidget->show();
    }
}

void Gui::UpdateVisualEditUndoRedo(const QString& UndoText, const QString& RedoText)
{
    visualEditUndo = !UndoText.isEmpty();
    if (visualEditUndo)
    {
        undoAct->setEnabled(true);
        undoAct->setText(QString(tr("&Undo %1")).arg(UndoText));
        undoAct->setStatusTip(tr("Undo %1").arg(UndoText));
    }
    else
    {
        undoAct->setEnabled(undoStack->canUndo());
        undoAct->setText(tr("&Undo"));
        undoAct->setStatusTip(tr("Undo last change"));
    }

    visualEditRedo = !RedoText.isEmpty();
    if (visualEditRedo)
    {
        redoAct->setEnabled(true);
        redoAct->setText(QString(tr("&Redo %1")).arg(RedoText));
#ifdef __APPLE__
        redoAct->setStatusTip(tr("Redo %1 - Ctrl+Shift+Z").arg(RedoText));
#else
        redoAct->setStatusTip(tr("Redo %1").arg(RedoText));
#endif
    }
    else
    {
        redoAct->setEnabled(undoStack->canRedo());
        redoAct->setText(tr("&Redo"));
#ifdef __APPLE__
        redoAct->setStatusTip(tr("Redo last change - Ctrl+Shift+Z"));
#else
        redoAct->setStatusTip(tr("Redo last change"));
#endif
    }

    visualEditUndoRedoText = visualEditUndo ? UndoText :
                             visualEditRedo ? RedoText : QString();

}

void Gui::statusBarMsg(QString msg)
{
    if (!Preferences::modeGUI)
        return;
    if(!visualEditDockWindow->isFloating())
        gMainWindow->mStatusBarLabel->setText(msg);
}

void Gui::toggleLCStatusBar(bool topLevel)
{
    Q_UNUSED(topLevel);

    if(visualEditDockWindow->isFloating())
        gMainWindow->statusBar()->show();
    else
        gMainWindow->statusBar()->hide();
}

void Gui::ViewerZoomSlider(int value)
{
    float z = value;
    if (value > mViewerZoomLevel)
        z = 10.0f;
    else
        z = -10.0f;
    gMainWindow->GetActiveView()->Zoom(z);
    mViewerZoomLevel = value;
}

void Gui::ResetViewerZoomSlider()
{
   viewerZoomSliderWidget->setValue(50);
}

void Gui::enableApplyLightAction()
{
    ApplyLightAct->setEnabled(lcGetActiveProject()->GetImageType() == Options::CSI);
}

void Gui::applyLightSettings()
{
    int it = lcGetActiveProject()->GetImageType();
    if (it != Options::CSI)
        return;

    Step *currentStep = lpub->currentStep;

    if (currentStep) {

        Meta meta;
        LightData lightData = meta.LeoCad.light.value();
        LightMeta lightMeta = meta.LeoCad.light;
        lightMeta.setValue(lightData);

        lcView* ActiveView = gMainWindow->GetActiveView();

        lcModel* ActiveModel = ActiveView->GetActiveModel();

        QString metaString;
        bool local = false, global = false;
        bool newCommand = true;
        Where top = currentStep->topOfStep();
        Where bottom = currentStep->bottomOfStep();

        auto notEqual = [] (const float v1, const float v2)
        {
            return qAbs(v1 - v2) > 0.1f;
        };

        beginMacro(QLatin1String("LightSettings"));

        // Delete existing LIGHT commands starting at the bottom of the current step
        for (Where walk = bottom - 1; walk >= top.lineNumber; --walk)
            if(readLine(walk).startsWith(lightMeta.preamble))
                deleteLine(walk);

        for (const std::unique_ptr<lcLight>& Light : ActiveModel->GetLights()) {

            emit messageSig(LOG_INFO, tr("Setting Light [%1]").arg(Light->GetName()));

            const bool POVRayLight = Light->GetLightFormat() == lcLightFormat::POVRayLight;

            // LeoCAD to LDraw to POVRay: 0 1 2 => 0 -2 1 => 2 0 1
            const int X = POVRayLight ? 2 : 0 ;
            const int Y = POVRayLight ? 0 : 1 ;
            const int Z = POVRayLight ? 1 : 2 ;

            QString Type = "Undefined";
            switch(Light->GetLightType())
            {
            case lcLightType::Point:
                Type = lightData.typeNames[LightData::Point];
                break;
            case lcLightType::Area:
                Type = lightData.typeNames[LightData::Area];
                break;
            case lcLightType::Directional:
                Type = lightData.typeNames[LightData::Sun];
                break;
            case lcLightType::Spot:
                Type = lightData.typeNames[LightData::Spot];
                break;
            default:
                break;
            }

            // Populate existing settings
            const QString lightKey = QString("%1 %2").arg(Type, Light->GetName());
            if (currentStep->lightList.contains(lightKey))
                lightMeta.setValue(currentStep->lightList[lightKey]);

            // Type and Name
            lightMeta.type.setValue(Type);
            metaString = lightMeta.type.format(local, global);
            metaString.append(QString(" NAME \"%1\"").arg(Light->GetName()));
            currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);

            // Position
            if (notEqual(Light->GetPosition()[X], lightData.position[X]) ||
                notEqual(Light->GetPosition()[Y], lightData.position[Y]) ||
                notEqual(Light->GetPosition()[Z], lightData.position[Z])) {
                lightMeta.position.setValues(Light->GetPosition()[X],
                                             Light->GetPosition()[Y],
                                             Light->GetPosition()[Z]);
                metaString = lightMeta.position.format(local, global);
                currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
            }

            // Target Position
            if (notEqual(Light->GetTargetPosition()[X], lightData.target[X]) ||
                notEqual(Light->GetTargetPosition()[Y], lightData.target[Y]) ||
                notEqual(Light->GetTargetPosition()[Z], lightData.target[Z])) {
                lightMeta.target.setValues(Light->GetTargetPosition()[X],
                                           Light->GetTargetPosition()[Y],
                                           Light->GetTargetPosition()[Z]);
                metaString = lightMeta.target.format(local, global);
                currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
            }

            // Rotation
            if (notEqual(Light->GetRotation()[0].x, lightData.rotation1[X]) ||
                notEqual(Light->GetRotation()[0].y, lightData.rotation1[Y]) ||
                notEqual(Light->GetRotation()[0].z, lightData.rotation1[Z]) ||
                notEqual(Light->GetRotation()[1].x, lightData.rotation2[X]) ||
                notEqual(Light->GetRotation()[1].y, lightData.rotation2[Y]) ||
                notEqual(Light->GetRotation()[1].z, lightData.rotation2[Z]) ||
                notEqual(Light->GetRotation()[2].x, lightData.rotation3[X]) ||
                notEqual(Light->GetRotation()[2].y, lightData.rotation3[Y]) ||
                notEqual(Light->GetRotation()[2].z, lightData.rotation3[Z])) {
                lightMeta.rotation.setValues(Light->GetRotation()[0].x,Light->GetRotation()[0].y,Light->GetRotation()[0].z,
                                             Light->GetRotation()[1].x,Light->GetRotation()[1].y,Light->GetRotation()[1].z,
                                             Light->GetRotation()[2].x,Light->GetRotation()[2].y,Light->GetRotation()[2].z);
                metaString = lightMeta.rotation.format(local, global);
                currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
            }

            // Colour
            if (notEqual(Light->GetColor()[0], lightData.color[0]) ||
                notEqual(Light->GetColor()[1], lightData.color[1]) ||
                notEqual(Light->GetColor()[2], lightData.color[2])) {
                lightMeta.color.setValues(Light->GetColor()[0],
                                          Light->GetColor()[1],
                                          Light->GetColor()[2]);
                metaString = lightMeta.color.format(local, global);
                currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
            }

            const float lightPower = POVRayLight ? Light->GetPOVRayPower() : Light->GetBlenderPower();

            if (POVRayLight)
            {
                // POVRay Power
                if (notEqual(lightPower, lightData.povrayPower)) {
                    lightMeta.povrayPower.setValue(lightPower);
                    metaString = lightMeta.povrayPower.format(local, global);
                    currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
                }
            }
            else
            {
                // Blender Power
                if (notEqual(lightPower, lightData.blenderPower)) {
                    lightMeta.blenderPower.setValue(lightPower);
                    metaString = lightMeta.blenderPower.format(local, global);
                    currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
                }

                // Specular
                if (notEqual(Light->GetBlenderSpecular(), lightData.specular)) {
                    lightMeta.specular.setValue(Light->GetBlenderSpecular());
                    metaString = lightMeta.specular.format(local, global);
                    currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
                }

                // Diffuse
                if (notEqual(Light->GetBlenderDiffuse(), lightData.diffuse)) {
                    lightMeta.diffuse.setValue(Light->GetBlenderDiffuse());
                    metaString = lightMeta.diffuse.format(local, global);
                    currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
                }

                // Cutoff Distance
                if (Light->GetEnableBlenderCutoff() && (notEqual(Light->GetBlenderCutoffDistance(), lightData.cutoffDistance))) {
                    lightMeta.cutoffDistance.setValue(Light->GetBlenderCutoffDistance());
                    metaString = lightMeta.cutoffDistance.format(local, global);
                    currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
                }
            }

            // Spot Penumbra Angle
            const float spotPenumbraAngle = Light->GetSpotPenumbraAngle();
            if (notEqual(spotPenumbraAngle, lightData.spotPenumbraAngle)) {
                lightMeta.spotPenumbraAngle.setValue(spotPenumbraAngle);
                metaString = lightMeta.spotPenumbraAngle.format(local, global);
                currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
            }

            lcLightAreaShape areaShapeInt = lcLightAreaShape::Square;

            switch (Light->GetLightType())
            {
            case lcLightType::Point:
                if (!POVRayLight)
                {
                    // Point Radius
                    if (notEqual(Light->GetPointBlenderRadius(), lightData.pointRadius)) {
                        lightMeta.pointRadius.setValue(Light->GetPointBlenderRadius());
                        metaString = lightMeta.pointRadius.format(local, global);
                        currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
                    }
                }
                break;
            case lcLightType::Spot:
                if (POVRayLight)
                {
                    // POVRay Fade Power
                    if (notEqual(Light->GetPOVRayFadePower(), lightData.fadePower)) {
                        lightMeta.fadePower.setValue(Light->GetPOVRayFadePower());
                        metaString = lightMeta.fadePower.format(local, global);
                        currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
                    }

                    // POVRay Fade Distance
                    if (notEqual(Light->GetPOVRayFadeDistance(), lightData.fadeDistance)) {
                        lightMeta.fadeDistance.setValue(Light->GetPOVRayFadeDistance());
                        metaString = lightMeta.fadeDistance.format(local, global);
                        currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
                    }

                    // Spot Tightness
                    if (notEqual(Light->GetSpotPOVRayTightness(), lightData.spotTightness)) {
                        lightMeta.spotTightness.setValue(Light->GetSpotPOVRayTightness());
                        metaString = lightMeta.spotTightness.format(local, global);
                        currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
                    }
                }
                else
                {
                    // Spot Radius
                    if (notEqual(Light->GetSpotBlenderRadius(), lightData.spotRadius)) {
                        lightMeta.spotRadius.setValue(Light->GetSpotBlenderRadius());
                        metaString = lightMeta.spotRadius.format(local, global);
                        currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
                    }

                    // Spot Cone Angle
                    if (notEqual(Light->GetSpotConeAngle(), lightData.spotConeAngle)) {
                        lightMeta.spotConeAngle.setValue(Light->GetSpotConeAngle());
                        metaString = lightMeta.spotConeAngle.format(local, global);
                        currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
                    }

                    // Spot Blend
                    const float spotBlend = spotPenumbraAngle / Light->GetSpotConeAngle();
                    if (notEqual(spotBlend, lightData.spotBlend)) {
                        lightMeta.spotBlend.setValue(spotBlend);
                        metaString = lightMeta.spotBlend.format(local, global);
                        currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
                    }
                }
                break;
            case lcLightType::Directional:
                {
                    if (!POVRayLight)
                    {
                        // Blender Sun (Directional) Angle
                        if (notEqual(Light->GetDirectionalBlenderAngle(), lightData.sunAngle)) {
                            lightMeta.sunAngle.setValue(Light->GetDirectionalBlenderAngle());
                            metaString = lightMeta.sunAngle.format(local, global);
                            currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
                        }
                    }
                }
                break;
            case lcLightType::Area:
                if (Light->GetAreaShape() == lcLightAreaShape::Rectangle || Light->GetAreaShape() == lcLightAreaShape::Ellipse)
                {
                    // Area Size X (Width)
                    if (notEqual(Light->GetAreaSizeX(), lightData.areaSizeX)) {
                        lightMeta.areaSizeX.setValue(Light->GetAreaSizeX());
                        metaString = lightMeta.areaSizeX.format(local, global);
                        currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
                    }

                    // Area Size Y (Height)
                    if (notEqual(Light->GetAreaSizeY(), lightData.areaSizeY)) {
                        lightMeta.areaSizeY.setValue(Light->GetAreaSizeY());
                        metaString = lightMeta.areaSizeY.format(local, global);
                        currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
                    }
                }
                else
                {
                    // Area Size (Width == Height)
                    if (notEqual(Light->GetAreaSizeX(), lightData.areaSize)) {
                        lightMeta.areaSize.setValue(Light->GetAreaSizeX());
                        metaString = lightMeta.areaSize.format(local, global);
                        currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
                    }
                }

                if (POVRayLight)
                {
                    // Area Grid X notEqual(Light->GetDirectionalBlenderAngle(), lightData.sunAngle)
                    if (int(Light->GetAreaPOVRayGridX()) != lightData.areaGridX) {
                        lightMeta.areaGridX.setValue(Light->GetAreaPOVRayGridX());
                        metaString = lightMeta.areaGridX.format(local, global);
                        currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
                    }

                    // Area Grid Y
                    if (int(Light->GetAreaPOVRayGridY()) != lightData.areaGridY) {
                        lightMeta.areaGridY.setValue(Light->GetAreaPOVRayGridY());
                        metaString = lightMeta.areaGridY.format(local, global);
                        currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
                    }
                }

                // Area Shape
                switch(Light->GetAreaShape())
                {
                case lcLightAreaShape::Disk:
                    lightData.areaShape = QLatin1String("DISK");
                    areaShapeInt = lcLightAreaShape::Disk;
                    break;
                case lcLightAreaShape::Rectangle:
                    lightData.areaShape = QLatin1String("RECTANGLE");
                    areaShapeInt = lcLightAreaShape::Rectangle;
                    break;
                case lcLightAreaShape::Ellipse:
                    lightData.areaShape = QLatin1String("ELLIPSE");
                    areaShapeInt = lcLightAreaShape::Ellipse;
                    break;
                default:
                    lightData.areaShape = QLatin1String("SQUARE");
                    break;
                }

                if (Light->GetAreaShape() != areaShapeInt)
                {
                    metaString = lightMeta.areaShape.format(local, global);
                    currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
                }
                break;
            default:
                break;
            }

            // POVRay Light
            if (POVRayLight)
            {
                lightMeta.setLatLong();

                metaString = lightMeta._povrayLight.format(local, global);
                currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
            }

            // Shadowless
            if (!Light->GetCastShadow())
            {
                metaString = lightMeta._shadowless.format(local, global);
                currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
            }
        }

        endMacro();
    }
}

void Gui::applyCameraSettings()
{
    using namespace Options;

    Step *currentStep = lpub->currentStep;

    if (currentStep) {

        lcView* ActiveView = gMainWindow->GetActiveView();

        if (!ActiveView)
            return;

        SettingsMeta cameraMeta;

        if (AutoCenterSelectionAct->isChecked())
            ActiveView->LookAt();

        lcCamera* Camera = ActiveView->GetCamera();

        auto validCameraFoV = [this, &cameraMeta, &Camera] ()
        {
            float result = qRound(Camera->m_fovy);

            if (Preferences::preferredRenderer != RENDERER_NATIVE)
                  // e.g.            30.0  +                 0.01         - 30.0
                   result = Camera->m_fovy + cameraMeta.cameraFoV.value() - GetPreferences().mCFoV;

            return qRound(result);
        };

        auto notEqual = [] (const float v1, const float v2)
        {
            return qAbs(v1 - v2) > 0.1f;
        };

        emit messageSig(LOG_INFO, QString("Setting %1 Camera").arg(Camera->GetName().isEmpty() ? "Default" : Camera->GetName()));

        QString imageFileName;

        int it = lcGetActiveProject()->GetImageType();
        switch(it) {
        case PLI:
            cameraMeta.cameraAngles   = currentStep->pli.pliMeta.cameraAngles;
            cameraMeta.cameraDistance = currentStep->pli.pliMeta.cameraDistance;
            cameraMeta.modelScale     = currentStep->pli.pliMeta.modelScale;
            cameraMeta.cameraFoV      = currentStep->pli.pliMeta.cameraFoV;
            cameraMeta.cameraZNear    = currentStep->pli.pliMeta.cameraZNear;
            cameraMeta.cameraZFar     = currentStep->pli.pliMeta.cameraZFar;
            cameraMeta.isOrtho        = currentStep->pli.pliMeta.isOrtho;
            cameraMeta.imageSize      = currentStep->pli.pliMeta.imageSize;
            cameraMeta.target         = currentStep->pli.pliMeta.target;
            cameraMeta.position       = currentStep->pli.pliMeta.position;
            cameraMeta.upvector       = currentStep->pli.pliMeta.upvector;
            break;
        case SMI:
            cameraMeta.cameraAngles   = currentStep->subModel.subModelMeta.cameraAngles;
            cameraMeta.cameraDistance = currentStep->subModel.subModelMeta.cameraDistance;
            cameraMeta.modelScale     = currentStep->subModel.subModelMeta.modelScale;
            cameraMeta.cameraFoV      = currentStep->subModel.subModelMeta.cameraFoV;
            cameraMeta.cameraZNear    = currentStep->subModel.subModelMeta.cameraZNear;
            cameraMeta.cameraZFar     = currentStep->subModel.subModelMeta.cameraZFar;
            cameraMeta.isOrtho        = currentStep->subModel.subModelMeta.isOrtho;
            cameraMeta.imageSize      = currentStep->subModel.subModelMeta.imageSize;
            cameraMeta.target         = currentStep->subModel.subModelMeta.target;
            cameraMeta.position       = currentStep->subModel.subModelMeta.position;
            cameraMeta.upvector       = currentStep->subModel.subModelMeta.upvector;
            break;
        default: /*CSI:*/
            cameraMeta                = currentStep->csiStepMeta;
            imageFileName             = currentStep->pngName;
            break;
        }

        QString metaString;
        bool newCommand = false;
        bool clearStepCache = false;
        lcVector3 ldrawVector;
        Where undefined = Where();
        Where top = currentStep->topOfStep();
        float Latitude, Longitude, Distance;

        bool applyTarget   = Camera->mTargetPosition != lcVector3(0.0f, 0.0f, 0.0f);
        bool applyPosition = Camera->mPosition       != lcVector3(0.0f, -0.0f, 0.0f);
        bool applyUpVector = Camera->mUpVector       != lcVector3(0.0f, 0.0f, -0.0f) && applyPosition;
        bool applyZPlanes  = applyUpVector;
        bool applyAngles   = !applyPosition && !applyUpVector;
        if (applyAngles) {
            Camera->GetAngles(Latitude, Longitude, Distance);
            applyAngles    = (notEqual(qRound(Latitude), qRound(cameraMeta.cameraAngles.value(0))) ||
                              notEqual(qRound(Longitude),qRound(cameraMeta.cameraAngles.value(1))));
        }
        bool applyDistance = applyAngles && notEqual(qRound(Distance), cameraMeta.cameraDistance.value());

        beginMacro(QLatin1String("CameraSettings"));

        // execute first in last out

        if (applyUpVector) {
            clearStepCache = true;
            if (QFileInfo::exists(imageFileName))
                clearStepCSICache(imageFileName);

            // Camera Globe, Switch Y and Z axis with -Y(LC -Z) in the up direction
            ldrawVector = lcVector3LeoCADToLDraw(Camera->mUpVector);
            cameraMeta.upvector.setValues(ldrawVector[0], ldrawVector[1], ldrawVector[2]);
            metaString = cameraMeta.upvector.format(true/*local*/,false/*global*/);
            newCommand = cameraMeta.upvector.here() == undefined;
            currentStep->mi(it)->setMetaAlt(newCommand ? top : cameraMeta.upvector.here(), metaString, newCommand);
        }

        if (applyTarget) {
            clearStepCache = true;
            if (QFileInfo::exists(imageFileName))
                clearStepCSICache(imageFileName);

            // Camera Globe, Switch Y and Z axis with -Y(LC -Z) in the up direction
            lcVector3 ldrawTarget = lcVector3LeoCADToLDraw(Camera->mTargetPosition);
            cameraMeta.target.setValues(ldrawTarget[0], ldrawTarget[1], ldrawTarget[2]);
            metaString = cameraMeta.target.format(true/*local*/,false/*global*/);
            newCommand = cameraMeta.target.here() == undefined;
            currentStep->mi(it)->setMetaAlt(newCommand ? top : cameraMeta.target.here(), metaString, newCommand);
        }

        if (applyPosition) {
            clearStepCache = true;
            if (QFileInfo::exists(imageFileName))
                clearStepCSICache(imageFileName);

            // Camera Globe, Switch Y and Z axis with -Y(LC -Z) in the up direction
            ldrawVector = lcVector3LeoCADToLDraw(Camera->mPosition);
            cameraMeta.position.setValues(ldrawVector[0], ldrawVector[1], ldrawVector[2]);
            metaString = cameraMeta.position.format(true/*local*/,false/*global*/);
            newCommand = cameraMeta.position.here() == undefined;
            currentStep->mi(it)->setMetaAlt(newCommand ? top : cameraMeta.position.here(), metaString, newCommand);
        }

        if (UseImageSizeAct->isChecked()) {
            cameraMeta.imageSize.setFormats(3,0,"###9");
            cameraMeta.imageSize.setValues(lcGetActiveProject()->GetImageWidth(),
                                           lcGetActiveProject()->GetImageHeight());
            metaString = cameraMeta.imageSize.format(true,false);
            newCommand = cameraMeta.imageSize.here() == undefined;
            currentStep->mi(it)->setMetaAlt(newCommand ? top : cameraMeta.imageSize.here(), metaString, newCommand);
        }

        if (notEqual(Camera->GetScale(), cameraMeta.modelScale.value())) {
            clearStepCache = true;
            cameraMeta.modelScale.setValue(Camera->GetScale());
            metaString = cameraMeta.modelScale.format(true,false);
            newCommand = cameraMeta.modelScale.here() == undefined;
            currentStep->mi(it)->setMetaAlt(newCommand ? top : cameraMeta.modelScale.here(), metaString, newCommand);
        }

        if (applyDistance && !UseImageSizeAct->isChecked()) {
            clearStepCache = true;
            cameraMeta.cameraDistance.setValue(qRound(Distance));
            metaString = cameraMeta.cameraDistance.format(true,false);
            newCommand = cameraMeta.cameraDistance.here() == undefined;
            currentStep->mi(it)->setMetaAlt(newCommand ? top : cameraMeta.cameraDistance.here(), metaString, newCommand);
        }

        if (applyAngles) {
            clearStepCache = true;
            cameraMeta.cameraAngles.setFormats(5,2,"###9");
            cameraMeta.cameraAngles.setValues(qRound(Latitude), qRound(Longitude));
            metaString = cameraMeta.cameraAngles.format(true,false);
            newCommand = cameraMeta.cameraAngles.here() == undefined;
            currentStep->mi(it)->setMetaAlt(newCommand ? top : cameraMeta.cameraAngles.here(), metaString, newCommand);
        }

        if (applyZPlanes && notEqual(cameraMeta.cameraZNear.value(), Camera->m_zNear)) {
            clearStepCache = true;
            cameraMeta.cameraZNear.setValue(Camera->m_zNear);
            metaString = cameraMeta.cameraZNear.format(true,false);
            newCommand = cameraMeta.cameraZNear.here() == undefined;
            currentStep->mi(it)->setMetaAlt(newCommand ? top : cameraMeta.cameraZNear.here(), metaString, newCommand);
        }

        if (applyZPlanes && notEqual(cameraMeta.cameraZFar.value(), Camera->m_zFar)) {
            clearStepCache = true;
            cameraMeta.cameraZFar.setValue(Camera->m_zFar);
            metaString = cameraMeta.cameraZFar.format(true,false);
            newCommand = cameraMeta.cameraZFar.here() == undefined;
            currentStep->mi(it)->setMetaAlt(newCommand ? top : cameraMeta.cameraZFar.here(), metaString, newCommand);
        }

        float fovy = validCameraFoV();
        if (notEqual(cameraMeta.cameraFoV.value(), fovy)) {
            clearStepCache = true;
            cameraMeta.cameraFoV.setValue(fovy);
            metaString = cameraMeta.cameraFoV.format(true,false);
            newCommand = cameraMeta.cameraFoV.here() == undefined;
            currentStep->mi(it)->setMetaAlt(newCommand ? top : cameraMeta.cameraFoV.here(), metaString, newCommand);
        }

        if (Camera->IsOrtho() != cameraMeta.isOrtho.value()) {
            clearStepCache = true;
            cameraMeta.isOrtho.setValue(Camera->IsOrtho());
            metaString = cameraMeta.isOrtho.format(true,false);
            newCommand = cameraMeta.isOrtho.here() == undefined;
            currentStep->mi(it)->setMetaAlt(newCommand ? top : cameraMeta.isOrtho.here(), metaString, newCommand);
        }

        if (!Camera->GetName().isEmpty()) {
            cameraMeta.cameraName.setValue(Camera->GetName());
            metaString = cameraMeta.cameraName.format(true,false);
            newCommand = cameraMeta.cameraName.here() == undefined;
            currentStep->mi(it)->setMetaAlt(newCommand ? top : cameraMeta.cameraName.here(), metaString, newCommand);
        }

        if (clearStepCache && QFileInfo::exists(imageFileName))
            clearStepCSICache(imageFileName);

        endMacro();
    }
}

bool Gui::loadBanner(const int &type, const QString &bannerPath)
{
    if (!gMainWindow)
        return false;

    const bool bom = type == GENERATE_BOM;
    const bool error = type == ERROR_ENCOUNTERED;
    QString banner = bom ? tr("BOM") : error ? tr("Error") : tr("Export");
    QString description = bom ? tr("Bill of Material (%1)").arg(banner) : banner;
    QList<QString> bannerData;
    bannerData << "0 Author: Trevor SANDY";
    bannerData << "0 !LDRAW_ORG Unofficial_Model";
    if (error) {
        bannerData << "0 !LEOCAD GROUP BEGIN Minifig Error";
        bannerData << "1 320 -5.805 -96 -0.8715 0.642788 0 -0.766044 0 1 0 0.766044 0 0.642788 3624.dat";
        bannerData << "1 14 -5.805 -96 -0.8715 0.642788 0 -0.766044 0 1 0 0.766044 0 0.642788 3626bp01.dat";
        bannerData << "1 320 -5.805 -72 -0.8715 0.642788 0 -0.766044 0 1 0 0.766044 0 0.642788 973.dat";
        bannerData << "1 0 -5.805 -40 -0.8715 0.642788 0 -0.766044 0 1 0 0.766044 0 0.642788 3815.dat";
        bannerData << "1 20 4.1916 -63.12 11.042 0.633425 0.618968 -0.464382 -0.170055 0.696808 0.696808 0.754887 -0.362405 0.546634 3819.dat";
        bannerData << "1 20 -15.8017 -63.12 -12.7851 0.633425 -0.109309 -0.766044 0.170055 0.985435 -0 0.754887 -0.13027 0.642788 3818.dat";
        bannerData << "1 14 23.763 -57.699 2.4644 0.639017 -0.069521 -0.766044 0.108156 0.994134 0 0.761551 -0.082852 0.642788 3820.dat";
        bannerData << "1 14 -13.3852 -45.247 -25.4625 0.633425 -0.618968 -0.464382 0.170055 0.696808 -0.696808 0.754887 0.362405 0.546634 3820.dat";
        bannerData << "1 23 39.9124 -90.0903 -4.5972 -0.963225 0.041837 0.265419 -0.260218 0.100936 -0.96026 -0.066965 -0.994013 -0.086337 3900.dat";
        bannerData << "1 0 -5.805 -28 -0.8715 0.642788 0 -0.766044 0 1 0 0.766044 0 0.642788 3817.dat";
        bannerData << "1 0 -5.805 -28 -0.8715 0.642788 0 -0.766044 0 1 0 0.766044 0 0.642788 3816.dat";
        bannerData << "0 !LEOCAD GROUP END";
        bannerData << "1 7 -27.4663 -8 -45.3551 0.642788 0 -0.766044 0 1 0 0.766044 0 0.642788 3570p01.dat";
    } else if (bom) {
        bannerData << "0 !LEOCAD GROUP BEGIN Minifig BOM";
        bannerData << "1 320 30.7866 -24 86.6357 0.906308 0 -0.422618 0 1 0 0.422618 0 0.906308 3624.dat";
        bannerData << "1 14 30.7866 -24 86.6357 0.906308 0 -0.422618 0 1 0 0.422618 0 0.906308 3626bp01.dat";
        bannerData << "1 320 30.7866 0 86.6357 0.906308 0 -0.422618 0 1 0 0.422618 0 0.906308 973.dat";
        bannerData << "1 0 30.7866 32 86.6357 0.906308 0 -0.422618 0 1 0 0.422618 0 0.906308 3815.dat";
        bannerData << "1 20 44.8815 8.88 93.2084 0.893107 0.349773 -0.282876 -0.170055 0.844683 0.507536 0.416463 -0.405179 0.813872 3819.dat";
        bannerData << "1 20 16.6916 8.8801 80.0631 0.893107 -0.154122 -0.422619 0.170055 0.985435 -0 0.416463 -0.071868 0.906308 3818.dat";
        bannerData << "1 14 58.8216 19.0033 79.4534 0.893107 0.047303 -0.447351 -0.170055 0.956163 -0.238398 0.416463 0.288989 0.862 3820.dat";
        bannerData << "1 14 13.5239 26.7531 67.5522 0.893107 -0.407817 -0.189856 0.170055 0.696808 -0.696807 0.416463 0.590037 0.691675 3820.dat";
        bannerData << "1 0 30.7866 44 86.6357 0.906308 0 -0.422618 0 1 0 0.422618 0 0.906308 3817.dat";
        bannerData << "1 0 30.7866 44 86.6357 0.906308 0 -0.422618 0 1 0 0.422618 0 0.906308 3816.dat";
        bannerData << "0 !LEOCAD GROUP END";
        bannerData << "1 71 -26.3683 -24 -56.5914 0.34202 0 -0.939693 0 1 0 0.939693 0 0.34202 3005ptb.dat";
        bannerData << "1 71 -19.5279 -24 -37.7975 0.34202 0 -0.939693 0 1 0 0.939693 0 0.34202 3005pti.dat";
        bannerData << "1 71 -12.6875 -24 -19.0037 0.34202 0 -0.939693 0 1 0 0.939693 0 0.34202 3005ptl.dat";
        bannerData << "1 71 -5.8471 -24 -0.2098 0.34202 0 -0.939693 0 1 0 0.939693 0 0.34202 3005ptl.dat";
        bannerData << "1 71 -19.5279 0 -37.7975 0.34202 0 -0.939693 0 1 0 0.939693 0 0.34202 3005pto.dat";
        bannerData << "1 71 -12.6875 0 -19.0037 0.34202 0 -0.939693 0 1 0 0.939693 0 0.34202 3005ptf.dat";
        bannerData << "1 72 -5.8471 0 -0.2098 0.34202 0 -0.939693 0 1 0 0.939693 0 0.34202 3005.dat";
        bannerData << "1 72 -26.3683 0 -56.5914 0.34202 0 -0.939693 0 1 0 0.939693 0 0.34202 3005.dat";
        bannerData << "1 71 -40.0492 24 -94.1791 0.34202 0 -0.939693 0 1 0 0.939693 0 0.34202 3005ptm.dat";
        bannerData << "1 71 -33.2087 24 -75.3853 0.34202 0 -0.939693 0 1 0 0.939693 0 0.34202 3005pta.dat";
        bannerData << "1 71 -26.3683 24 -56.5914 0.34202 0 -0.939693 0 1 0 0.939693 0 0.34202 3005ptt.dat";
        bannerData << "1 71 -19.5279 24 -37.7975 0.34202 0 -0.939693 0 1 0 0.939693 0 0.34202 3005pte.dat";
        bannerData << "1 71 -12.6875 24 -19.0037 0.34202 0 -0.939693 0 1 0 0.939693 0 0.34202 3005ptr.dat";
        bannerData << "1 71 -5.8471 24 -0.2098 0.34202 0 -0.939693 0 1 0 0.939693 0 0.34202 3005pti.dat";
        bannerData << "1 71 0.9933 24 18.5841 0.34202 0 -0.939693 0 1 0 0.939693 0 0.34202 3005pta.dat";
        bannerData << "1 71 7.8337 24 37.378 0.34202 0 -0.939693 0 1 0 0.939693 0 0.34202 3005ptl.dat";
        bannerData << "1 72 -19.5279 -48 -37.7975 0.34202 0 -0.939693 0 1 0 0.939693 0 0.34202 3062a.dat";
        bannerData << "1 72 -12.6875 -48 -19.0037 0.34202 0 -0.939693 0 1 0 0.939693 0 0.34202 3062a.dat";
        bannerData << "1 72 0.9933 0 18.5841 0.34202 0 -0.939693 0 1 0 0.939693 0 0.34202 3005.dat";
        bannerData << "1 72 -33.2087 0 -75.3853 0.34202 0 -0.939693 0 1 0 0.939693 0 0.34202 3005.dat";
        bannerData << "1 72 -16.1077 48 -28.4006 0.34202 0 -0.939693 0 1 0 0.939693 0 0.34202 6111.dat";
    } else {
        bannerData << "0 !LEOCAD GROUP BEGIN Minifig Export";
        bannerData << "1 320 66.3171 -69 30.467 0.97437 0 0.224951 0 1 0 -0.224951 0 0.97437 3624.dat";
        bannerData << "1 14 66.3171 -69 30.467 0.97437 0 0.224951 0 1 0 -0.224951 0 0.97437 3626bp01.dat";
        bannerData << "1 320 66.3171 -45 30.467 0.97437 0 0.224951 0 1 0 -0.224951 0 0.97437 973.dat";
        bannerData << "1 0 66.3171 -13 30.467 0.97437 0 0.224951 0 1 0 -0.224951 0 0.97437 3815.dat";
        bannerData << "1 20 81.4705 -36.1201 26.9685 0.960178 0.165696 0.224951 -0.170055 0.985435 0 -0.221674 -0.038254 0.97437 3819.dat";
        bannerData << "1 20 51.1636 -36.1201 33.9654 0.960178 -0.279308 -0.006704 0.170055 0.565223 0.80722 -0.221674 -0.776215 0.590212 3818.dat";
        bannerData << "1 14 87.1702 -18.2469 15.3895 0.960178 0.276229 0.041899 -0.170055 0.696808 -0.696807 -0.221674 0.661934 0.716034 3820.dat";
        bannerData << "1 14 41.1228 -34.3034 14.4235 0.960178 -0.202241 0.19276 0.170055 0.970464 0.171119 -0.221674 -0.131525 0.96621 3820.dat";
        bannerData << "1 0 66.3171 -1 30.467 0.97437 -0.224951 0 0 0 1 -0.224951 -0.97437 0 3817.dat";
        bannerData << "1 0 66.3171 -1 30.467 0.97437 -0.224951 0 0 0 1 -0.224951 -0.97437 0 3816.dat";
        bannerData << "0 !LEOCAD GROUP END";
        bannerData << "1 71 3.7112 0 -12.0337 0.891007 0 -0.45399 0 1 0 0.45399 0 0.891007 3020.dat";
        bannerData << "1 71 25.9016 -8 10.4962 0.891007 0 -0.45399 0 1 0 0.45399 0 0.891007 3024.dat";
        bannerData << "1 71 25.9016 -16 10.4962 0.891007 0 -0.45399 0 1 0 0.45399 0 0.891007 3024.dat";
        bannerData << "1 71 -27.5591 -8 -16.7433 0.891007 0 -0.45399 0 1 0 0.45399 0 0.891007 3024.dat";
        bannerData << "1 71 -27.5591 -16 -16.7433 0.891007 0 -0.45399 0 1 0 0.45399 0 0.891007 3024.dat";
        bannerData << "1 71 -27.5591 -32 -16.7433 0.891007 0 -0.45399 0 1 0 0.45399 0 0.891007 6091.dat";
        bannerData << "1 71 25.9016 -32 10.4962 0.891007 0 -0.45399 0 1 0 0.45399 0 0.891007 6091.dat";
        bannerData << "1 71 25.9016 -32 10.4962 0.891007 0 -0.45399 0 1 0 0.45399 0 0.891007 30039.dat";
        bannerData << "1 2 -27.5591 -32 -16.7433 0.891007 0 -0.45399 0 1 0 0.45399 0 0.891007 30039.dat";
        bannerData << "1 71 -0.8287 -24 -3.1235 0.891007 0 -0.45399 0 1 0 0.45399 0 0.891007 3937.dat";
        bannerData << "1 72 8.2511 -8 -20.9438 0.891007 0 -0.45399 0 1 0 0.45399 0 0.891007 3023.dat";
        bannerData << "1 72 8.2511 -8 -20.9438 -0.891007 0 0.45399 0 1 0 -0.45399 0 -0.891007 85984.dat";
        bannerData << "1 71 0.872 -23.272 -6.4615 -0.891007 -0.17025 0.420859 0 0.927021 0.375008 -0.45399 0.334135 -0.825982 3938.dat";
        bannerData << "1 72 4.9537 -45.523 -14.4722 -0.891007 -0.17025 0.420859 0 0.927021 0.375008 -0.45399 0.334135 -0.825982 4865a.dat";
        switch (type) {
        case EXPORT_PNG:
            banner = "PNG";
            description = tr("Export Portable Network Graphics (%1)").arg(banner);
            bannerData << "1 25 -1.3668 -4.0043 -50.5269 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptp.dat";
            bannerData << "1 25 16.4534 -4.0043 -41.447 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptn.dat";
            bannerData << "1 25 34.2736 -4.0043 -32.3671 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptg.dat";
            break;
        case EXPORT_JPG:
            banner = "JPEG";
            description = tr("Export Joint Photographic Experts Group (%1)").arg(banner);
            bannerData << "1 92 -1.3668 -4.0043 -50.5269 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptj.dat";
            bannerData << "1 92 16.4534 -4.0043 -41.447 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptp.dat";
            bannerData << "1 92 34.2736 -4.0043 -32.3671 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptg.dat";
            break;
        case EXPORT_BMP:
            banner = "BMP";
            description = tr("Export Bitmap Image (%1)").arg(banner);
            bannerData << "1 320 -1.3668 -4.0043 -50.5269 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptb.dat";
            bannerData << "1 320 16.4534 -4.0043 -41.447 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptm.dat";
            bannerData << "1 320 34.2736 -4.0043 -32.3671 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptp.dat";
            break;
        case EXPORT_WAVEFRONT:
            banner = "OBJ";
            description = tr("Export Wavefront 3D Object (%1)").arg(banner);
            bannerData << "1 320 -1.3668 -4.0043 -50.5269 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bpto.dat";
            bannerData << "1 320 16.4534 -4.0043 -41.447 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptb.dat";
            bannerData << "1 320 34.2736 -4.0043 -32.3671 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptj.dat";
            break;
        case EXPORT_COLLADA:
            banner = "DAE";
            description = tr("Export COLLADA Digital Asset Exchange (%1)").arg(banner);
            bannerData << "1 320 -1.3668 -4.0043 -50.5269 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptd.dat";
            bannerData << "1 320 16.4534 -4.0043 -41.447 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bpta.dat";
            bannerData << "1 320 34.2736 -4.0043 -32.3671 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bpte.dat";
            break;
        case EXPORT_3DS_MAX:
            banner = "3DS";
            description = tr("Export Autodesk 3DS Max (%1)").arg(banner);
            bannerData << "1 320 -1.3668 -4.0043 -50.5269 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptx.dat";
            bannerData << "1 320 16.4534 -4.0043 -41.447 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptd.dat";
            bannerData << "1 320 34.2736 -4.0043 -32.3671 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bpts.dat";
            break;
        case EXPORT_STL:
            banner = "STL";
            description = tr("Export Stereolithography Standard Tessellation Language (%1)").arg(banner);
            bannerData << "1 320 -1.3668 -4.0043 -50.5269 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bpts.dat";
            bannerData << "1 320 16.4534 -4.0043 -41.447 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptt.dat";
            bannerData << "1 320 34.2736 -4.0043 -32.3671 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptl.dat";
            break;
        case POVRAY_RENDER:
        case EXPORT_POVRAY:
            banner = "POV";
            description = tr("Export POV-Ray Scene Description (%1)").arg(banner);
            bannerData << "1 320 -1.3668 -4.0043 -50.5269 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptp.dat";
            bannerData << "1 320 16.4534 -4.0043 -41.447 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bpto.dat";
            bannerData << "1 320 34.2736 -4.0043 -32.3671 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptv.dat";
            break;
        case EXPORT_HTML_PARTS:
            banner = "HTM";
            description = tr("Export Hypertext Markup Language (%1)").arg(banner);
            bannerData << "1 320 -1.3668 -4.0043 -50.5269 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bpth.dat";
            bannerData << "1 320 16.4534 -4.0043 -41.447 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptt.dat";
            bannerData << "1 320 34.2736 -4.0043 -32.3671 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptm.dat";
            break;
        case EXPORT_CSV:
            banner = "CSV";
            description = tr("Export Comma-Separated Values (%1)").arg(banner);
            bannerData << "1 320 -1.3668 -4.0043 -50.5269 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptc.dat";
            bannerData << "1 320 16.4534 -4.0043 -41.447 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bpts.dat";
            bannerData << "1 320 34.2736 -4.0043 -32.3671 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptv.dat";
            break;
        case EXPORT_BRICKLINK:
            banner = "XML";
            description = tr("Export The LEGO Group Bricklink (%1)").arg(banner);
            bannerData << "1 320 -1.3668 -4.0043 -50.5269 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptx.dat";
            bannerData << "1 320 16.4534 -4.0043 -41.447 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptm.dat";
            bannerData << "1 320 34.2736 -4.0043 -32.3671 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptl.dat";
            break;
        case EXPORT_LDRAW_MODEL:
            banner = "LDraw";
            description = tr("Export Current (%1)").arg(banner);
            bannerData << "1 462 -19.1869 -4.0043 -59.6067 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptl.dat";
            bannerData << "1 462 -1.3668 -4.0043 -50.5269 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptd.dat";
            bannerData << "1 462 16.4534 -4.0043 -41.447 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptr.dat";
            bannerData << "1 462 34.2736 -4.0043 -32.3671 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bpta.dat";
            bannerData << "1 462 52.0937 -4.0043 -23.2873 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptw.dat";
            break;
        case BLENDER_RENDER:
            banner = "Blender";
            description = tr("Render Image (%1)").arg(banner);
            bannerData << "1 462 -37.0071 -4.0043 -68.6865 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptb.dat";
            bannerData << "1 462 -19.1869 -4.0043 -59.6067 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptl.dat";
            bannerData << "1 462 -1.3668 -4.0043 -50.5269 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bpte.dat";
            bannerData << "1 462 16.4534 -4.0043 -41.447 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptn.dat";
            bannerData << "1 462 34.2736 -4.0043 -32.3671 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptd.dat";
            bannerData << "1 462 52.0937 -4.0043 -23.2873 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bpte.dat";
            bannerData << "1 462 69.9138 -4.0043 -14.2077 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptr.dat";
            break;
        default:
            banner = "PDF";
            description = tr("Export Portable Document Format (%1)").arg(banner);
            bannerData << "1 320 -1.3668 -4.0043 -50.5269 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptp.dat";
            bannerData << "1 320 16.4534 -4.0043 -41.447 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptd.dat";
            bannerData << "1 320 34.2736 -4.0043 -32.3671 0.891007 -0.411364 -0.192061 0 0.42305 -0.906106 0.45399 0.807347 0.37694 3070bptf.dat";
        }
    }

    const QString bannerFileName = QString("%1 %2.ldr").arg(banner, VISUAL_BANNER_SUFFIX);

    bannerData << "0";
    bannerData.prepend("0 Name: " + bannerFileName);
    bannerData.prepend("0 " + description + " " + VISUAL_BANNER_SUFFIX);

    const QString bannerFilePath = QDir::toNativeSeparators(QString("%1/%2").arg(bannerPath, bannerFileName));

    QFile bannerFile(bannerFilePath);
    if ( ! bannerFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) {
        emit gui->messageSig(LOG_ERROR,tr("Could not open banner file '%1' for writing:\n%2")
                             .arg(bannerFilePath, bannerFile.errorString()));
        return false;
    }
    QTextStream out(&bannerFile);
    for (int i = 0; i < bannerData.size(); i++)
        out << bannerData[i] << lpub_endl;
    bannerFile.close();

    if (!gui->PreviewPiece(bannerFilePath, LDRAW_MATERIAL_COLOUR)) {
        emit gui->messageSig(LOG_WARNING, tr("Could not load banner file '%1'.").arg(bannerFilePath));
        return false;
    }

    return true;
}

void Gui::groupActionTriggered()
{
    QAction* Action = static_cast<QAction*>(sender());
    lcCommandId commandId;
    QString commandTip;
    bool ok;

    if (Action == ViewpointZoomExtAct)
    {
        lcSetProfileInt(LC_PROFILE_VIEWPOINT_ZOOM_EXTENT, ViewpointZoomExtAct->isChecked());
    }
    else if (Action == ViewpointGroupAct)
    {
        commandId = lcCommandId(ViewpointGroupAct->property("CommandId").toInt(&ok));
        if (ok)
            gMainWindow->mActions[commandId]->trigger();
    }
    else if (Action == LightGroupAct)
    {
        commandId = lcCommandId(LightGroupAct->property("CommandId").toInt(&ok));
        if (ok)
            gMainWindow->mActions[commandId]->trigger();
    }
    else
    {
        commandId = lcCommandId(Action->property("CommandId").toInt(&ok));
        commandTip = Action->property("CommandTip").toString();
        if (ok)
        {
            if (commandId == LC_EDIT_ACTION_POINT_LIGHT ||
                commandId == LC_EDIT_ACTION_DIRECTIONAL_LIGHT ||
                commandId == LC_EDIT_ACTION_SPOTLIGHT ||
                commandId == LC_EDIT_ACTION_AREA_LIGHT) {
                LightGroupAct->setIcon(gMainWindow->mActions[commandId]->icon());
                LightGroupAct->setToolTip(tr(commandTip.toLatin1()));
                LightGroupAct->setStatusTip(gMainWindow->mActions[commandId]->statusTip());
                LightGroupAct->setProperty("CommandId", QVariant(commandId));
            } else {
                ViewpointGroupAct->setIcon(gMainWindow->mActions[commandId]->icon());
                ViewpointGroupAct->setToolTip(tr(commandTip.toLatin1()));
                ViewpointGroupAct->setStatusTip(gMainWindow->mActions[commandId]->statusTip());
                ViewpointGroupAct->setProperty("CommandId", QVariant(commandId));
                if (Action == gMainWindow->mActions[LC_VIEW_VIEWPOINT_LAT_LON] && commandId == LC_VIEW_VIEWPOINT_HOME)
                    gMainWindow->mActions[LC_VIEW_VIEWPOINT_LAT_LON]->setProperty("CommandId", QVariant(LC_VIEW_VIEWPOINT_LAT_LON));
            }
        }
    }
}

void Gui::restoreLightAndViewpointDefaults() {
    LightGroupAct->setToolTip(tr("Lights - Pointlight"));
    LightGroupAct->setIcon(gMainWindow->mActions[LC_EDIT_ACTION_POINT_LIGHT]->icon());
    LightGroupAct->setStatusTip(gMainWindow->mActions[LC_EDIT_ACTION_POINT_LIGHT]->statusTip());
    LightGroupAct->setProperty("CommandId", QVariant(LC_EDIT_ACTION_POINT_LIGHT));

    ViewpointGroupAct->setToolTip(tr("Viewpoints - Home"));
    ViewpointGroupAct->setIcon(gMainWindow->mActions[LC_VIEW_VIEWPOINT_HOME]->icon());
    ViewpointGroupAct->setStatusTip(gMainWindow->mActions[LC_VIEW_VIEWPOINT_HOME]->statusTip());
    ViewpointGroupAct->setProperty("CommandId", QVariant(LC_VIEW_VIEWPOINT_HOME));
}

void Gui::enableVisualBuildModification()
{
    if (!lpub->currentStep || !Preferences::modeGUI || Gui::exporting())
        return;

#ifdef QT_DEBUG_MODE
    LDrawFile *ldrawFile = &lpub->ldrawFile;
    Step *currentStep = lpub->currentStep;
    Q_UNUSED(ldrawFile)
    Q_UNUSED(currentStep)
#endif

    bool displayStep = lpub->currentStep->displayStep != DT_DEFAULT;
    bool buildModEnabled = Preferences::buildModEnabled && !displayStep;

    EnableBuildModRotateAct->setVisible(!displayStep);
    EnableBuildModRotateAct->setEnabled(buildModEnabled);

    if (sender() == EnableBuildModRotateAct)
        buildModEnabled &= EnableBuildModRotateAct->isChecked();
    else if (sender() == EnableRotstepRotateAct)
        buildModEnabled &= !EnableRotstepRotateAct->isChecked();

    gApplication->mPreferences.mBuildModificationEnabled = buildModEnabled;

    if (buildModEnabled && !Gui::curFile.isEmpty()) {
        using namespace Options;
        switch (lcGetActiveProject()->GetImageType())
        {
        case static_cast<int>(CSI):
        case static_cast<int>(SMI):
            break;

        default:
            return;
        }
    }

    if(!Gui::ContinuousPage())
        gMainWindow->UpdateDefaultCameraProperties();

    EnableBuildModRotateAct->setChecked(buildModEnabled);
    EnableRotstepRotateAct->setChecked(!buildModEnabled);

    gMainWindow->mActions[LC_EDIT_ACTION_BUILD_MOD]->setEnabled(false);
    CreateBuildModAct->setEnabled(false);
    UpdateBuildModAct->setEnabled(false);

    QIcon RotateIcon;
    QString RotateText,RotateStatusTip,RelTranslateStatusTip,AbsTranslateStatusTip;
    if (buildModEnabled) {
        RotateText = tr("Rotate Build Modification");
        RotateStatusTip = tr("Rotate selected build modification pieces - Shift+L");
        RelTranslateStatusTip = tr("Switch to relative translation mode when applying transforms");
        AbsTranslateStatusTip = tr("Switch to absolute translation mode when applying transforms");
        RotateIcon.addFile(":/resources/rotatebuildmod.png");
        RotateIcon.addFile(":/resources/rotatebuildmod16.png");
        if (gMainWindow->GetTransformType() == lcTransformType::RelativeRotation ||
            gMainWindow->GetTransformType() == lcTransformType::AbsoluteRotation)
            gMainWindow->SetTransformType(lcTransformType::RelativeTranslation);
    } else {
        RotateText = tr("Rotate ROTSTEP");
        RotateStatusTip = tr("Select model to enable axes for %1 ROTSTEP command - Shift+L").arg(VER_PRODUCTNAME_STR);
        RelTranslateStatusTip = tr("Relative translation mode is disabled when build modifications are disabled");
        AbsTranslateStatusTip = tr("Absolute translation mode is disabled when build modifications are disabled");
        RotateIcon.addFile(":/resources/rotaterotstep.png");
        RotateIcon.addFile(":/resources/rotaterotstep16.png");
        if (gMainWindow->GetTransformType() == lcTransformType::RelativeTranslation ||
            gMainWindow->GetTransformType() == lcTransformType::AbsoluteTranslation)
            gMainWindow->SetTransformType(lcTransformType::RelativeRotation);
    }

    gMainWindow->mActions[LC_EDIT_ACTION_ROTATE]->setIcon(RotateIcon);
    gMainWindow->mActions[LC_EDIT_ACTION_ROTATE]->setText(RotateText);
    gMainWindow->mActions[LC_EDIT_ACTION_ROTATE]->setStatusTip(RotateStatusTip);

    gMainWindow->mActions[LC_EDIT_ACTION_ROTATESTEP]->setEnabled(false);

    gMainWindow->mActions[LC_EDIT_TRANSFORM_RELATIVE_TRANSLATION]->setEnabled(buildModEnabled);
    gMainWindow->mActions[LC_EDIT_TRANSFORM_RELATIVE_TRANSLATION]->setStatusTip(RelTranslateStatusTip);
    gMainWindow->mActions[LC_EDIT_TRANSFORM_ABSOLUTE_TRANSLATION]->setEnabled(buildModEnabled);
    gMainWindow->mActions[LC_EDIT_TRANSFORM_ABSOLUTE_TRANSLATION]->setStatusTip(AbsTranslateStatusTip);
}

void Gui::enableVisualBuildModActions()
{
    Step *currentStep = lpub->currentStep;

    if (!currentStep || !Preferences::buildModEnabled || !Preferences::modeGUI || Gui::exporting())
        return;

    if (currentStep)
        if (currentStep->displayStep != DT_DEFAULT)
            return;

#ifdef QT_DEBUG_MODE
    LDrawFile *ldrawFile = &lpub->ldrawFile;
    Q_UNUSED(ldrawFile)
#endif

    using namespace Options;
    if (static_cast<Mt>(lcGetActiveProject()->GetImageType()) != CSI)
        return;

    // build modification actions
    bool appliedMod = false, removedMod = false;
    bool beginModStep = false, beginMod = false, buildModEnabled = false;

    // count build mods declared at or before the current page
    int modCount = buildModsCount();

    // number of declared build mods at or before current page is 1
    bool oneMod = modCount == 1;

    bool applyModDialogTitle = ApplyBuildModAct->text().endsWith("...");

    // set action labels and initialize action lineNumber propert
    bool updateTitle = oneMod || !applyModDialogTitle;

    // get the current step index
    int modStepIndex = getBuildModStepIndex(currentStep->topOfStep());

    // build mods declared at or before last step of current page is greater than 0
    bool haveMod = static_cast<bool>(modCount);

    // to enable actions, is the earliest build mod declared before or at the current step ?
    if ((haveMod &= lpub->ldrawFile.getBuildModFirstStepIndex() <= modStepIndex)) {

        // get the build mod declaration and/or actions for the current step
        QList<QVector<int> > buildModStepActions = getBuildModStepActions(currentStep->topOfStep());

        // assign step actions
        if (buildModStepActions.size()) {

            for (int i = 0; i < buildModStepActions.size(); ++i) {
                QVector<int> buildModStepAction = buildModStepActions.at(i);

                // get the current build mod key
                const QString buildModKey = getBuildModsList().at(buildModStepAction[BM_ACTION_KEY_INDEX]);

                // get the current build mod action
                const Rc modAction = static_cast<Rc>(buildModStepAction[BM_ACTION_CODE]);

                // get the current build mod begin step index
                const int modBeginStepIndex = getBuildModStepIndex(buildModKey);

                // is the current build mod action the same as the last action occurrence ?
                bool sameAsPrev = getBuildModActionPrevIndex(buildModKey, modStepIndex, modAction) < modStepIndex;

                // are we in the begin step for the current action ?
                beginModStep = modStepIndex == modBeginStepIndex;

                // set mod action
                switch (modAction)
                {
                    case BuildModBeginRc:
                        beginMod = haveMod;
                        break;
                    case BuildModApplyRc:
                        appliedMod = haveMod && !sameAsPrev;
                        break;
                    case BuildModRemoveRc:
                        removedMod = haveMod && !sameAsPrev;
                        break;
                    case BuildModNoActionRc:
                    default:
                        break;
                } // set mod action
            } // for each action - begin, apply, remove
        } // have step declaration and/or step actions
    } // haveMod

    // set delete action description
    bool deleteBuildModAction = appliedMod || removedMod;
    QString deleteBuildModActionText, deleteBuildModActionStatusTip;
    if (deleteBuildModAction) {
        if (appliedMod) {
            deleteBuildModActionText      = tr("Delete Build Modification Apply Action");
            deleteBuildModActionStatusTip = tr("Delete build modification apply action command from this step");
        } else {
            deleteBuildModActionText      = tr("Delete Build Modification Remove Action");
            deleteBuildModActionStatusTip = tr("Delete build modification remove action command from this step");
        }
    } else {
        deleteBuildModActionText      = tr("Delete Build Modification Action");
        deleteBuildModActionStatusTip = tr("Delete build modification action command from this step");
    }

    // enable actions
    ApplyBuildModAct->setEnabled(         haveMod && !appliedMod && !removedMod && !beginModStep);
    RemoveBuildModAct->setEnabled(        haveMod && !appliedMod && !removedMod && !beginModStep);
    LoadBuildModAct->setEnabled(          haveMod && !beginMod);
    DeleteBuildModAct->setEnabled(        beginMod);
    DeleteBuildModActionAct->setEnabled(  deleteBuildModAction);
    DeleteBuildModActionAct->setText(     deleteBuildModActionText);
    DeleteBuildModActionAct->setStatusTip(deleteBuildModActionStatusTip);

    // set action text and build mod combo enable
    const QList<QAction*> modActions = {
        ApplyBuildModAct,
        RemoveBuildModAct,
        DeleteBuildModActionAct,
        DeleteBuildModAct,
        LoadBuildModAct
    };

    for(QAction* action : modActions) {
        if (updateTitle) {
            QString text = action->text();
            if (oneMod && applyModDialogTitle)
                text.chop(3);
            if (!oneMod && !applyModDialogTitle)
                text.append("...");
            action->setText(text);
        }
        buildModEnabled |= action->isEnabled();
    }

    buildModEnabled |= CreateBuildModAct->isEnabled();
    buildModEnabled |= UpdateBuildModAct->isEnabled();

    // enable build mod combo action
    gMainWindow->mActions[LC_EDIT_ACTION_BUILD_MOD]->setEnabled(buildModEnabled);
    if (!buildModEnabled) {
        QIcon BuildModIcon;
        BuildModIcon.addFile(":/resources/buildmodcreate.png");
        BuildModIcon.addFile(":/resources/buildmodcreate16.png");
        gMainWindow->mActions[LC_EDIT_ACTION_BUILD_MOD]->setIcon(BuildModIcon);
        gMainWindow->mActions[LC_EDIT_ACTION_BUILD_MOD]->setShortcut(QStringLiteral("Shift+J"));
        gMainWindow->mActions[LC_EDIT_ACTION_BUILD_MOD]->setText(tr("Build Modification"));
        gMainWindow->mActions[LC_EDIT_ACTION_BUILD_MOD]->setStatusTip(tr("Create update or manage build modifications"));
    }
}

void Gui::enableVisualBuildModEditAction()
{
    Step *currentStep = lpub->currentStep;

    if (!currentStep)
        return;

    bool buildModEnabled = VisualEditChangeTriggers.contains(visualEditUndoRedoText) && mBuildModRange.size();

    // get and set the build mod action and, if action is build mod begin, action line number
    Rc buildModStepAction = OkRc;
    if (buildModEnabled) {
        QList<QVector<int> > buildModStepActions = getBuildModStepActions(currentStep->topOfStep());
        if (buildModStepActions.size()) {
            for (int i = 0; i < buildModStepActions.size(); ++i) {
                QVector<int> stepAction = buildModStepActions.at(i);
                if ((buildModStepAction = static_cast<Rc>(stepAction[BM_ACTION_CODE])) == BuildModBeginRc) {
                    break;
                }
            }
        }
    }

    QIcon BuildModIcon;
    QKeySequence BuildModShortcut;
    QAction *BuildModAct = gMainWindow->mActions[LC_EDIT_ACTION_BUILD_MOD];
    if (buildModStepAction == BuildModBeginRc) {
        disconnect(BuildModAct, SIGNAL(triggered()), this, SLOT(createBuildModification()));
        connect(BuildModAct, SIGNAL(triggered()), this, SLOT(updateBuildModification()));
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
        BuildModShortcut = QKeySequence(Qt::SHIFT | Qt::Key_K);
#else
        BuildModShortcut = QKeySequence(Qt::SHIFT + Qt::Key_K);
#endif
        BuildModIcon.addFile(":/resources/buildmodupdate.png");
        BuildModIcon.addFile(":/resources/buildmodupdate16.png");
        BuildModAct->setText(tr("Update Build Modification"));
        BuildModAct->setStatusTip(tr("Update the existing build modification in this step"));
        UpdateBuildModAct->setEnabled(buildModEnabled);
    } else {
        disconnect(BuildModAct, SIGNAL(triggered()), this, SLOT(updateBuildModification()));
        connect(BuildModAct, SIGNAL(triggered()), this, SLOT(createBuildModification()));
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
        BuildModShortcut = QKeySequence(Qt::SHIFT | Qt::Key_J);
#else
        BuildModShortcut = QKeySequence(Qt::SHIFT + Qt::Key_J);
#endif
        BuildModIcon.addFile(":/resources/buildmodcreate.png");
        BuildModIcon.addFile(":/resources/buildmodcreate16.png");
        if (buildModEnabled) {
            BuildModAct->setText(tr("Create Build Modification"));
            BuildModAct->setStatusTip(tr("Create a new build modification for this step"));
        }
        CreateBuildModAct->setEnabled(buildModEnabled);
    }

    gMainWindow->mActions[LC_EDIT_ACTION_BUILD_MOD]->setIcon(BuildModIcon);
    gMainWindow->mActions[LC_EDIT_ACTION_BUILD_MOD]->setShortcut(BuildModShortcut);
    gMainWindow->mActions[LC_EDIT_ACTION_BUILD_MOD]->setEnabled(buildModEnabled);
}

void Gui::showDefaultCameraProperties()
{
    gApplication->mPreferences.mDefaultCameraProperties = DefaultCameraPropertiesAct->isChecked();
    lcSetProfileInt(LC_PROFILE_DEFAULT_CAMERA_PROPERTIES, DefaultCameraPropertiesAct->isChecked());
}

void Gui::useImageSize()
{
    lcSetProfileInt(LC_PROFILE_USE_IMAGE_SIZE, UseImageSizeAct->isChecked());
}

void Gui::autoCenterSelection()
{
    lcSetProfileInt(LC_PROFILE_AUTO_CENTER_SELECTION, AutoCenterSelectionAct->isChecked());
}

void Gui::createStatusBar()
{
    if (!Preferences::modeGUI)
        return;

    QStatusBar* StatusBar = new QStatusBar(this);
    setStatusBar(StatusBar);

    if (!gMainWindow->mStatusBarLabel)
        gMainWindow->mStatusBarLabel = new lcElidedLabel();
    StatusBar->addWidget(gMainWindow->mStatusBarLabel, 1);
    gMainWindow->mStatusBarLabel->setText(tr("Ready"));

    gui->progressLabelPerm = new QLabel(this);
    gui->progressLabelPerm->setAlignment(Qt::AlignRight);
    gui->progressLabelPerm->hide();
    gui->progressBarPerm = new QProgressBar(this);
    gui->progressBarPerm->setMaximumHeight(15);
    gui->progressBarPerm->setMaximumWidth(300);
    gui->progressBarPerm->hide();
    StatusBar->addPermanentWidget(gui->progressLabelPerm);
    StatusBar->addPermanentWidget(gui->progressBarPerm);

    if (!gMainWindow->mStatusPositionLabel)
        gMainWindow->mStatusPositionLabel = new QLabel();
    StatusBar->addPermanentWidget(gMainWindow->mStatusPositionLabel);

    if (!gMainWindow->mStatusSnapLabel)
        gMainWindow->mStatusSnapLabel = new QLabel();
    StatusBar->addPermanentWidget(gMainWindow->mStatusSnapLabel);
}

void Gui::displayVisualEditorStatusWidgets(bool b)
{
    gMainWindow->mStatusPositionLabel->setVisible(b);
}

void Gui::readVisualEditorSettings(QSettings &Settings)
{
    if (gMainWindow)
        gMainWindow->mPartSelectionWidget->LoadState(Settings);
}

void Gui::writeVisualEditorSettings(QSettings& Settings)
{
    if (gMainWindow)
        gMainWindow->mPartSelectionWidget->SaveState(Settings);
}

void Gui::SetActiveModel(const QString &modelName)
{
    QString const &stepKey = lcGetActiveProject()->GetStepKey();

    if (!stepKey.isEmpty()) {
        if (stepKey == Gui::getCurrentStep()->viewerStepKey)
            return;
        lpub->setCurrentStep(stepKey);
    } else {
        if (modelName == VIEWER_MODEL_DEFAULT)
            return;
        if (Gui::getCurrentStep()) {
            if (modelName == Gui::getCurrentStep()->topOfStep().modelName)
                return;
        } else {
            emit gui->messageSig(LOG_WARNING, QString("Model '%1' could not be set active.").arg(modelName));
            return;
        }
    }

    displayFile(&lpub->ldrawFile, Gui::getCurrentStep()->topOfStep());
}

int Gui::GetImageWidth()
{
    return lcGetActiveProject()->GetImageWidth();
}

int Gui::GetImageHeight()
{
    return lcGetActiveProject()->GetImageHeight();
}

QStringList Gui::get3DViewerPOVLightList() const
{
    Step *currentStep = lpub->currentStep;

    QStringList lightStringList;

    if (!currentStep)
        return lightStringList;

    lcView* ActiveView   = gMainWindow->GetActiveView();
    lcModel* ActiveModel = ActiveView->GetActiveModel();

    if (ActiveModel)
    {
        // Save lights
        for (const std::unique_ptr<lcLight>& Light : ActiveModel->GetLights())
        {
            const bool POVRayLight = Light->GetLightFormat() == lcLightFormat::POVRayLight;

            // Switch XYZ coordinates from LeoCAD where +Z is up to LDraw where -Y is up, so  (0 1 2) => (0 -2 1)
            // Then switch from LDraw to POVRay where +Y is up, so (0 -2 1) => (2 0 1)
            // FYI, from LeoCAD directly to POVRay would be (0 1 2) => (1 0 2)
            const int X = POVRayLight ? 2 : 0 ;
            const int Y = POVRayLight ? 0 : 1 ;
            const int Z = POVRayLight ? 1 : 2 ;

            LightData lightData;
            lcVector3 areaWidth, areaHeight;

            QString Type = "Undefined";
            switch(Light->GetLightType())
            {
            case lcLightType::Point:
                Type = lightData.typeNames[LightData::Point];
                break;
            case lcLightType::Area:
                Type = lightData.typeNames[LightData::Area];
                break;
            case lcLightType::Directional:
                Type = lightData.typeNames[LightData::Sun];
                break;
            case lcLightType::Spot:
                Type = lightData.typeNames[LightData::Spot];
                break;
            default:
                break;
            }

            const QString lightKey = QString("%1 %2").arg(Type, Light->GetName());
            if (currentStep->lightList.contains(lightKey))
                lightData = currentStep->lightList[lightKey];

            lightData.type = Type;
            lightData.name = Light->GetName();
            lightData.position[X] = Light->GetPosition().x;
            lightData.position[Y] = Light->GetPosition().y;
            lightData.position[Z] = Light->GetPosition().z;
            lightData.target[X] = Light->GetTargetPosition().x;
            lightData.target[Y] = Light->GetTargetPosition().y;
            lightData.target[Z] = Light->GetTargetPosition().z;
            lightData.color[0] = Light->GetColor()[0];
            lightData.color[1] = Light->GetColor()[1];
            lightData.color[2] = Light->GetColor()[2];
            lightData.povrayPower = Light->GetPOVRayPower();

            switch (Light->GetLightType())
            {
            case lcLightType::Point:
            case lcLightType::Directional:
                break;

            case lcLightType::Spot:
                lightData.spotConeAngle = Light->GetSpotConeAngle();
                lightData.fadePower = Light->GetPOVRayFadePower();
                lightData.fadeDistance = Light->GetPOVRayFadeDistance();
                lightData.spotTightness = Light->GetSpotPOVRayTightness();
                break;

            case lcLightType::Area:
                lightData.areaGridX = int(Light->GetAreaPOVRayGridX());
                lightData.areaGridY = int(Light->GetAreaPOVRayGridY());
                lightData.areaWidth = Light->GetAreaSizeX();
                lightData.areaHeight = Light->GetAreaSizeY();
                areaWidth = lcVector3(Light->GetWorldMatrix()[0]) * Light->GetAreaSizeX();
                areaHeight = lcVector3(Light->GetWorldMatrix()[1]) * Light->GetAreaSizeY();

                switch(Light->GetAreaShape())
                {
                case lcLightAreaShape::Square:
                    lightData.areaShape = QLatin1String("SQUARE");
                    break;
                case lcLightAreaShape::Disk:
                    lightData.areaShape = QLatin1String("DISK");
                    break;
                case lcLightAreaShape::Rectangle:
                    lightData.areaShape = QLatin1String("RECTANGLE");
                    break;
                case lcLightAreaShape::Ellipse:
                    lightData.areaShape = QLatin1String("ELLIPSE");
                    break;
                case lcLightAreaShape::Count:
                    break;
                }
                break;

            case lcLightType::Count:
                break;
            }

            LightMeta lightMeta;
            lightMeta.setValue(lightData);
            lightStringList.append(lightMeta.value().getPOVLightMacroString());
        }
    }

    return lightStringList;
}

void Gui::SaveCurrent3DViewerModel(const QString &_ModelFile)
{
    Step *currentStep = lpub->currentStep;

    if (!currentStep)
        return;

//* local ldrawFile and step used for debugging
#ifdef QT_DEBUG_MODE
    LDrawFile *ldrawFile = &lpub->ldrawFile;
    Q_UNUSED(ldrawFile)
#endif
//*/

    Project* Project = lcGetActiveProject();
    lcView* ActiveView = gMainWindow->GetActiveView();
    if (!Project || !ActiveView)
        return;
    lcModel* ActiveModel = ActiveView->GetActiveModel();

    QString ModelFile = _ModelFile;

    bool ModelBannerLoaded = false;

    if (ActiveModel)
    {
        // Get the model file name
        if (ModelFile.isEmpty()) {
            LPub::loadBanner(EXPORT_LDRAW_MODEL);
            ModelBannerLoaded = true;
            ModelFile = QFileInfo(currentStep->topOfStep().modelName).completeBaseName();
            ModelFile = ModelFile.replace(ModelFile.indexOf(ModelFile.at(0)),1,ModelFile.at(0).toUpper());
            if (!(currentStep->displayStep >= DT_MODEL_DEFAULT || currentStep->subModel.viewerSubmodel))
                ModelFile += tr("-Step-%1").arg(currentStep->stepNumber.number);
            bool IsMPDModel = Project->GetModels().size() > 1;
            ModelFile.append(IsMPDModel ? QString(".mpd") : QString(".ldr"));
            QString Filter = IsMPDModel ? tr("Supported Files (*.mpd);;All Files (*.*)") : tr("Supported Files (*.ldr *.dat *.mpd);;All Files (*.*)");
            ModelFile = QFileDialog::getSaveFileName(gui, tr("Save Model"), ModelFile, Filter);
            if (ModelFile.isEmpty())
                return;
        }

        // Get the current camera
        lcCamera* Camera = ActiveView->GetCamera();

        bool IsDefaultCamera = true;

        // If default camera, name it, add it to cameras and set it to the active view so it can be exported
        if (Camera)
        {
            if ((IsDefaultCamera = Camera->GetName().isEmpty()))
            {
                Camera->CreateName(ActiveModel->GetCameras());
                ActiveModel->AddCamera(Camera);
                ActiveView->SetCamera(Camera->GetName());
            }
        }

        // Save the current model
        if (!Project->Save(ModelFile))
            emit gui->messageSig(LOG_ERROR, tr("Failed to save current model to file [%1]").arg(ModelFile));

        if (currentStep->displayStep >= DT_MODEL_DEFAULT || currentStep->subModel.viewerSubmodel)
        {
            QFile vf(ModelFile);
            if (vf.open(QFile::ReadOnly | QFile::Text)) {
                QTextStream in(&vf);
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
                in.setEncoding(QStringConverter::Utf8);
#else
                in.setCodec(QTextCodec::codecForName("UTF-8"));
#endif
                QStringList vcl;
                QLatin1String lpubMeta("0 !LPUB ");
                while ( ! in.atEnd()) {
                    QString line = in.readLine(0);
                    if (line.startsWith(lpubMeta))
                        vcl << line.trimmed();
                }
                vf.close();

                QFile rf(ModelFile);
                if (rf.open(QFile::WriteOnly | QIODevice::Truncate | QFile::Text)) {
                    QTextStream out(&rf);
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
                    out.setEncoding(QStringConverter::Utf8);
#else
                    out.setCodec(QTextCodec::codecForName("UTF-8"));
#endif
                    QStringList const scl = lpub->ldrawFile.getViewerStepRotatedContents(currentStep->viewerStepKey);
                    static QRegularExpression insertRx("^0 // ROTSTEP ");
                    bool inserted = false;
                    for (QString const &sl : scl) {
                        if (!inserted) {
                            if ((inserted = sl.contains(insertRx))) {
                                for (QString &vl : vcl)
                                    out << vl << lpub_endl;
                                continue;
                            }
                        }
                        out << sl << lpub_endl;
                    }
                    rf.close();
                } else {
                    emit gui->messageSig(LOG_ERROR, tr("Cannot write render file: [%1]<br>%2.")
                                         .arg(ModelFile, rf.errorString()));
                }
            } else {
                emit gui->messageSig(LOG_ERROR, tr("Cannot read viewer file: [%1]<br>%2.")
                                     .arg(ModelFile, vf.errorString()));
            }
        }

        // Reset the camera
        if (Camera && IsDefaultCamera)
        {
            for (size_t CameraIdx = 0; CameraIdx < ActiveModel->GetCameras().size(); )
            {
                QString const Name = ActiveModel->GetCameras()[CameraIdx]->GetName();
                if (Name == Camera->GetName())
                    ActiveModel->RemoveCameraIndex(CameraIdx);
                else
                    CameraIdx++;
            }

            Camera = new lcCamera(true);
            ActiveView->SetCamera(Camera, false);
        }

        if (ModelBannerLoaded)
            currentStep->loadTheViewer();
    }
}

/*********************************************
 *
 * RotStep Meta
 *
 ********************************************/
void Gui::SetRotStepAngles(const QVector<float> &Angles, bool display)
{
    mRotStepAngleX = Angles[0];
    mRotStepAngleY = Angles[1];
    mRotStepAngleZ = Angles[2];
    if (display)
        ShowStepRotationStatus();
}

void Gui::SetRotStepAngleX(float AngleX, bool display)
{
    mRotStepAngleX = AngleX;
    if (display)
        ShowStepRotationStatus();
}

void Gui::SetRotStepAngleY(float AngleY, bool display)
{
    mRotStepAngleY = AngleY;
    if (display)
        ShowStepRotationStatus();
}

void Gui::SetRotStepAngleZ(float AngleZ, bool display)
{
    mRotStepAngleZ = AngleZ;
    if (display)
        ShowStepRotationStatus();
}

void Gui::SetRotStepType(const QString& RotStepType, bool display)
{
    mRotStepType = RotStepType;
    if (display)
        ShowStepRotationStatus();
}

void Gui::SetRotStepCommand()
{
    Step *currentStep = Gui::getCurrentStep();

    if (currentStep) {
        using namespace Options;
        int it = lcGetActiveProject()->GetImageType();

        RotStepData rotStepData = currentStep->rotStepMeta.value();

        if (! Gui::exporting() && Preferences::modeGUI) {
            const QString type = it == static_cast<int>(CSI) ? QLatin1String("Step") :
                                 it == static_cast<int>(PLI) ? QLatin1String("Part") :
                                 it == static_cast<int>(SMI) ? QLatin1String("Submodel Preview") :
                                                               QLatin1String("Item");
            const QString question = tr("Apply command ROTSTEP %1 %2 %3 %4 to %5 %6 ?<br><br>"
                                        "Current command is <i>ROTSTEP %7 %8 %9 %10</i>")
                                        .arg(QString::number(double(mRotStepAngleX),'f',0),
                                             QString::number(double(mRotStepAngleY),'f',0),
                                             QString::number(double(mRotStepAngleZ),'f',0))
                                        .arg(mRotStepType)
                                        .arg(type)
                                        .arg(QString::number(currentStep->stepNumber.number))
                                        .arg(QString::number(double(rotStepData.rots[0]),'f',0),
                                             QString::number(double(rotStepData.rots[1]),'f',0),
                                             QString::number(double(rotStepData.rots[2]),'f',0))
                                        .arg(rotStepData.type);
            if (QMessageBox::question(this, tr("%1 Rotate Step Command").arg(VER_PRODUCTNAME_STR), question,
                                      QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
                return;

            ShowStepRotationStatus();
        }

        lpub->ldrawFile.setViewerStepModified(currentStep->viewerStepKey);

        mStepRotation[0] = mRotStepAngleX;
        mStepRotation[1] = mRotStepAngleY;
        mStepRotation[2] = mRotStepAngleZ;

        RotStepData saveRotStepData = rotStepData;

        rotStepData.type    = mRotStepType;
        rotStepData.rots[0] = double(mStepRotation[0]);
        rotStepData.rots[1] = double(mStepRotation[1]);
        rotStepData.rots[2] = double(mStepRotation[2]);

        currentStep->rotStepMeta.setValue(rotStepData);

        QString metaString = currentStep->rotStepMeta.format(false/*no LOCAL tag*/,false);

        currentStep->rotStepMeta.setValue(saveRotStepData);

        Where top = currentStep->rotStepMeta.here();

        bool newCommand = top == Where() || top < currentStep->topOfStep();

        if (newCommand) {
            top = currentStep->topOfStep();
            QString line = gui->readLine(top);
            Rc rc = lpub->page.meta.parse(line,top);
            if (rc == RotStepRc || rc == StepRc) {
                currentStep->mi(it)->replaceMeta(top, metaString);
            } else {
                bool firstStep = top.modelName == gui->topLevelFile() &&
                                 currentStep->stepNumber.number == 1 + Gui::sa;
                if (firstStep)
                    currentStep->mi(it)->scanPastLPubMeta(top);
                currentStep->mi(it)->insertMeta(top, metaString);
            }
        } else {
            currentStep->mi(it)->replaceMeta(top, metaString);
        }
    } else {
        emit messageSig(LOG_WARNING,tr("No current step was detected. Cannot create a build modification."), true/*showMessage*/);
    }
}

void Gui::ShowStepRotationStatus()
{
    QString rotLabel = QString("%1 X: %2 Y: %3 Z: %4 Transform: %5")
                               .arg(lcGetPreferences().mBuildModificationEnabled ? QLatin1String("Rotate Build Modification") : QLatin1String("ROTSTEP"),
                                    QString::number(double(mRotStepAngleX), 'f', 2),
                                    QString::number(double(mRotStepAngleY), 'f', 2),
                                    QString::number(double(mRotStepAngleZ), 'f', 2),
                                    mRotStepType.toUpper() == QLatin1String("REL") ? QLatin1String("Relative") :
                                    mRotStepType.toUpper() == QLatin1String("ABS") ? QLatin1String("Absolute") :
                                                              QLatin1String("None"));
    statusBarMsg(rotLabel);
}

/*********************************************
 *
 * RotStep Meta
 *
 ********************************************/

void Gui::loadTheme() {
  Application::instance()->setTheme();
  emit gui->setTextEditHighlighterSig();
  Gui::ReloadVisualEditor();
}

void Gui::ReloadVisualEditor() {
  Step *currentStep = Gui::getCurrentStep();
  if (currentStep) {
    if (currentStep->displayStep == DT_MODEL_COVER_PAGE_PREVIEW) {
      Where topOfStep = currentStep->topOfStep();
      Gui::showLine(topOfStep);
      const QString fileName = Preferences::preferredRenderer == RENDERER_NATIVE ? SUBMODEL_IMAGE_BASENAME : SUBMODEL_COVER_PAGE_PREVIEW_BASENAME;
      const QString modelFileName = QDir::toNativeSeparators(QString("%1/%2/%3.ldr").arg(QDir::currentPath(), Paths::tmpDir, fileName));
      if (!gui->PreviewPiece(modelFileName, LDRAW_MATERIAL_COLOUR))
        emit gui->messageSig(LOG_WARNING, tr("Could not reload cover page preview (%1) file '%2'.").arg(topOfStep.modelName, modelFileName));
    } else if (currentStep->viewerOptions) {
      Render::LoadViewer(currentStep->viewerOptions);
    }
  } else {
    Project* NewProject = new Project();
    gApplication->SetProject(NewProject);
    Gui::UpdateAllViews();
  }
}

/*********************************************
 *
 * Native viewer convenience calls
 *
 ********************************************/

 lcPiecesLibrary* Gui::GetPiecesLibrary()
 {
     return lcGetPiecesLibrary();
 }

 lcView* Gui::GetActiveView()
 {
     if (gMainWindow)
         return gMainWindow->GetActiveView();
     else
         return new lcView(lcViewType::View, gApplication->mProject->GetMainModel());
 }

 lcModel* Gui::GetActiveModel()
 {
     return gApplication->mProject->GetMainModel();
 }

 Project* Gui::GetActiveProject()
 {
     return gApplication->mProject;
 }

 lcPartSelectionWidget* Gui::GetPartSelectionWidget()
 {
     if (gMainWindow)
         return gMainWindow->GetPartSelectionWidget();
     return nullptr;
 }

 lcPreferences& Gui::GetPreferences()
 {
     return lcGetPreferences();
 }

 bool Gui::GetSubmodelIconsLoaded()
 {
     if (gMainWindow)
         return gMainWindow->mSubmodelIconsLoaded;
     return false;
 }

 void Gui::SetSubmodelIconsLoaded(bool value)
 {
     if (gMainWindow)
         gMainWindow->mSubmodelIconsLoaded = value;
 }

 int Gui::GetLPubStepPieces()
 {
     if (GetActiveModel())
         return GetActiveModel()->GetPieces().size();
     return 0;
 }

 void Gui::UpdateAllViews()
 {
     lcView::UpdateAllViews();
 }

 void Gui::UnloadOfficialPiecesLibrary()
 {
     lcGetPiecesLibrary()->UnloadOfficialLib();
 }

 void Gui::UnloadUnofficialPiecesLibrary()
 {
     lcGetPiecesLibrary()->UnloadUnofficialLib();
 }

 bool Gui::ReloadPiecesLibrary()
 {
     return lcGetPiecesLibrary()->Reload();
 }

 bool Gui::ReloadUnofficialPiecesLibrary()
 {
     return lcGetPiecesLibrary()->ReloadUnofficialLib();
 }

 void Gui::LoadColors()
 {
     lcGetPiecesLibrary()->LoadColors();
 }

 void Gui::LoadDefaults()
 {
     gApplication->mPreferences.LoadDefaults();
 }

 QToolBar* Gui::GetStandardToolBar()
 {
     if (gMainWindow)
         return gMainWindow->GetStandardToolBar();
     return nullptr;
 }

 QToolBar* Gui::GetToolsToolBar()
 {
     if (gMainWindow)
         return gMainWindow->GetToolsToolBar();
     return nullptr;
 }

 QToolBar* Gui::GetVisualizationToolBar()
 {
     if (gMainWindow)
         return gMainWindow->GetVisualizationToolBar();
     return nullptr;
 }

 QDockWidget* Gui::GetTimelineToolBar()
 {
     if (gMainWindow)
         return gMainWindow->GetTimelineToolBar();
     return nullptr;
 }

 QDockWidget* Gui::GetPropertiesToolBar()
 {
     if (gMainWindow)
         return gMainWindow->GetPropertiesToolBar();
     return nullptr;
 }

 QDockWidget* Gui::GetPartsToolBar()
 {
     if (gMainWindow)
         return gMainWindow->GetPartsToolBar();
     return nullptr;
 }

 QDockWidget* Gui::GetColorsToolBar()
 {
     if (gMainWindow)
         return gMainWindow->GetColorsToolBar();
     return nullptr;
 }

 QMenu* Gui::GetCameraMenu()
 {
     if (gMainWindow)
         return gMainWindow->GetCameraMenu();
     return nullptr;
 }

 QMenu* Gui::GetEditMenu()
 {
     if (gMainWindow)
         return gMainWindow->GetEditMenu();
     return nullptr;
 }

  QMenu* Gui::GetToolsMenu()
 {
     if (gMainWindow)
         return gMainWindow->GetToolsMenu();
     return nullptr;
 }

 QMenu* Gui::GetPieceMenu()
 {
     if (gMainWindow)
         return gMainWindow->GetPieceMenu();
     return nullptr;
 }

 QMenu* Gui::GetViewpointMenu()
 {
     if (gMainWindow)
         return gMainWindow->GetViewpointMenu();
     return nullptr;
 }

 QMenu* Gui::GetProjectionMenu()
 {
     if (gMainWindow)
         return gMainWindow->GetProjectionMenu();
     return nullptr;
 }

 QMenu* Gui::GetShadingMenu()
 {
     if (gMainWindow)
         return gMainWindow->GetShadingMenu();
     return nullptr;
 }

/*********************************************
 *
 * build modificaitons
 *
 ********************************************/
 QMutex createBuildModMutex;

 void Gui::createBuildModification()
 {
     QMutexLocker createBuildModLocker(&createBuildModMutex);

     if (!Preferences::buildModEnabled)
         return;

     Step *currentStep = lpub->currentStep;

     if (!currentStep)
         return;

     bool Update = !buildModificationKey.isEmpty();

     if ((!Update && !CreateBuildModAct->isEnabled()) || (Update && !UpdateBuildModAct->isEnabled()))
         return;

     using namespace Options;
     Mt imageType = static_cast<Mt>(lcGetActiveProject()->GetImageType());
     if (imageType != CSI) {
         const QString model = imageType == CSI ? tr("a part instance") :
                               imageType == SMI ? tr("a submodel preview") :
                               tr("not an assembly");
         emit gui->messageSig(LOG_WARNING,tr("Build modifications can only be created for an assembly.<br>"
                                             "The active model is %1.").arg(model), LOG_SHOW_DIALOG);
         return;
     }

     //actionTriggered = true;

     if (buildModificationKey.isEmpty() && ! mBuildModRange.first()) {
         emit gui->messageSig(LOG_INFO,tr("No build modification detected for this step.<br>There is nothing to create."),LOG_SHOW_DIALOG);
         return;
     }

     lcView* ActiveView = GetActiveView();
     lcModel* ActiveModel = ActiveView->GetActiveModel();

     if (ActiveModel) {
         const QString Action = Update ? QLatin1String("Update") : QLatin1String("Create");

         QString BuildModKey = buildModificationKey;

         gui->progressPermInit();
         gui->progressBarPermSetRange(0, 0);   // Busy indicator
         gui->progressLabelPermSetText(tr("%1 Build Modification...").arg(Action));

         if (mBuildModRange.first() || Update) {

             emit gui->messageSig(LOG_INFO, tr("%1 Build Modification for Step %2...")
                                               .arg(Action)
                                               .arg(currentStep->stepNumber.number));

             // 'load...' default lines from modelFile and 'save...' buildMod lines from Visual Editor
             std::vector<std::unique_ptr<lcCamera>> Cameras;
             std::vector<std::unique_ptr<lcLight>>  Lights;
             std::vector<std::unique_ptr<lcGroup>>  Groups;
             std::vector<lcGroup*>  LPubGroups;
             std::vector<lcGroup*>  ViewerGroups;
             std::vector<std::unique_ptr<lcPiece>>  LPubPieces;   // Pieces in the buildMod - may include removed viewer pieces
             std::vector<lcPieceControlPoint> ControlPoints;
             lcModelProperties  ViewerProperties;
             QStringList  LPubFileLines,   ViewerFileLines,
                          LPubModContents, ViewerModContents,
                          ModStepKeys;

             bool FadeSteps        = Preferences::enableFadeSteps;
             bool HighlightStep    = Preferences::enableHighlightStep && !Gui::suppressColourMeta();

             int CurrentStep       = 1;
             int AddedPieces       = 0;

             lcPiece  *Piece       = nullptr;
             lcCamera *Camera      = nullptr;
             lcLight  *Light       = nullptr;
             ViewerProperties      = ActiveModel->GetProperties();
             ViewerFileLines       = ActiveModel->GetFileLines();
             lcPiecesLibrary *Library = lcGetPiecesLibrary();

             QString ModStepKey    = lpub->viewerStepKey;
             ModStepKeys           = ModStepKey.split(";");

             // When Update, initialize BuildMod StepPieces, and Begin and End range with the existing values
             int BuildModBegin     = BM_INIT;
             int BuildModAction    = BM_INIT;
             int BuildModEnd       = BM_INIT;
             int ModBeginLineNum   = BM_INIT;
             int ModActionLineNum  = BM_INIT;
             int ModEndLineNum     = BM_INIT;
             int ModStepPieces     = BM_INIT;
             int ModelIndex        = BM_NONE;
             if (Update) {
                 BuildModBegin     = getBuildModBeginLineNumber(BuildModKey);
                 BuildModAction    = getBuildModActionLineNumber(BuildModKey);
                 BuildModEnd       = getBuildModEndLineNumber(BuildModKey);
                 ModBeginLineNum   = BuildModBegin;
                 ModActionLineNum  = BuildModAction;
                 ModEndLineNum     = BuildModEnd;
                 ModStepPieces     = getBuildModStepPieces(BuildModKey);  // All pieces in the previous step
                 ModelIndex        = getSubmodelIndex(getBuildModStepKeyModelName(BuildModKey));
             } else {
                 ModBeginLineNum   = mBuildModRange.at(BM_BEGIN_LINE_NUM);
                 ModActionLineNum  = mBuildModRange.at(BM_ACTION_LINE_NUM);
                 ModEndLineNum     = mBuildModRange.at(BM_BEGIN_LINE_NUM);
                 ModelIndex        = mBuildModRange.at(BM_MODEL_INDEX);
             }
             int ModStepIndex      = getBuildModStepIndex(currentStep->topOfStep());
             int ModStepLineNum    = ModStepKeys[BM_STEP_LINE_KEY].toInt();
             int ModStepNum        = ModStepKeys[BM_STEP_NUM_KEY].toInt();
             int ModDisplayPageNum = Gui::displayPageNum;
             QString ModelName     = getSubmodelName(ModelIndex);
             buildModificationKey     = QString(); // clear the change key
             int Top               = currentStep->topOfStep().lineNumber;
             int Bottom            = currentStep->bottomOfStep().lineNumber;

             const QByteArray LPMeta("0 !LPUB");

             static QRegularExpression rx("(\\r\\n)|\\r|\\n");

             // Check that the the build mod and current step shares the same submodel
             if (ModStepKeys[BM_STEP_MODEL_KEY].toInt() != ModelIndex)
                 emit gui->messageSig(LOG_ERROR, tr("%1 BuildMod model (%2) '%3' and current Step model (%4) are not the same")
                                                    .arg(Action)
                                                    .arg(ModelIndex).arg(ModelName, ModStepKeys[BM_STEP_MODEL_KEY]));

             if (Gui::abortProcess()) {
                 showLine(currentStep->topOfStep());
                 gui->progressPermStatusRemove();
                 return;
             }

             // Check that the build mod is in the current step
             if (ModBeginLineNum && (ModBeginLineNum < Top || (Bottom && ModBeginLineNum > Bottom))) {
                 const QString message = tr("%1 BuildMod at line '%2' is %3 the current step.<br>"
                                            "Top of current step '%4', Bottom of current step '%5'.<br><br>"
                                            "Do you want to continue ?")
                                            .arg(Action)
                                            .arg(ModBeginLineNum)
                                            .arg(ModBeginLineNum < Top ? tr("before") : tr("after"))
                                            .arg(Top)
                                            .arg(Bottom);
                 if (QMessageBox::warning(this,tr("%1 BuildMod Warning").arg(VER_PRODUCTNAME_STR),message,
                                          QMessageBox::No|QMessageBox::Yes,QMessageBox::No) == QMessageBox::No) {
                     showLine(currentStep->topOfStep());
                     gui->progressPermStatusRemove();
                     return;
                 }
             }

             // Check if there is an existing build modification in this Step
             static QRegularExpression lineRx("^0 !?LPUB BUILD_MOD BEGIN ");
             if (Gui::stepContains(currentStep->top, lineRx) && !Update) {

                 QMessageBox box;
                 box.setWindowIcon(QIcon());
                 box.setIconPixmap (QPixmap(LPUB3D_MESSAGE_ICON));
                 box.setTextFormat (Qt::RichText);
                 box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
                 QString title = "<b>" + tr ("You specified create action for an existing Build Modification") + "</b>";
                 QString text = tr("<br>This action will replace the existing Build Modification."
                                   "<br>You can cancel and select 'Update Build Modification...' from the build modification submenu."
                                   "<br>Do you want to continue with this create action ?");
                 box.setText (title);
                 box.setInformativeText (text);
                 box.setStandardButtons (QMessageBox::No | QMessageBox::Yes);
                 box.setDefaultButton   (QMessageBox::No);

                 if (box.exec() == QMessageBox::No) {
                     showLine(currentStep->topOfStep());
                     gui->progressPermStatusRemove();
                     return;
                 }
             }

             // Process a step group
             auto GetGroup = [&Groups](const QString& Name, bool CreateIfMissing)
             {
                 lcGroup* nullGroup = nullptr;
                 for (const std::unique_ptr<lcGroup>& Group : Groups)
                     if (Group->mName == Name)
                         return Group.get();

                 if (CreateIfMissing)
                 {
                     lcGroup* Group = new lcGroup();
                     Group->mName = Name;
                     Groups.emplace_back(Group);

                     return Group;
                 }

                 return nullGroup;
             };

             // Configure LPub fade previous steps and/or highlight current step lines
             auto ConfigureFadeHighlightPartLine = [&] (int LineTypeIndex, QTextStream &Stream)
             {
                 if (!FadeSteps && !HighlightStep)
                     return;

                 if (!Stream.string()->isEmpty()) {
                     QStringList ModLines = Stream.string()->split(rx, SkipEmptyParts);
                     QString PartLine = ModLines.last();

                     QStringList argv;
                     split(PartLine, argv);

                     if (argv.size() == 15 && argv[0] == "1") {

                         QString NameMod, ColourPrefix;
                         if (FadeSteps) {
                             NameMod = FADE_SFX;
                             ColourPrefix = LPUB3D_COLOUR_FADE_PREFIX;
                         } else if (HighlightStep) {
                             NameMod = HIGHLIGHT_SFX;
                             ColourPrefix = LPUB3D_COLOUR_HIGHLIGHT_PREFIX;
                         }

                         // Colour code
                         if (argv[1].startsWith(ColourPrefix)) {
                             int LineNumber;
                             QString LPubPartLine, NewColorCode;
                             bool NewLine = !gui->getSelectedLine(ModelIndex, LineTypeIndex, VIEWER_MOD, LineNumber);

                             if (NewLine) {
                                 NewColorCode = argv[1];

                             } else {
                                 LPubPartLine = LPubModContents.at(LineTypeIndex);
                                 QStringList dargv;
                                 split(LPubPartLine, dargv);

                                 if (dargv.size() == 15 && dargv[0] == "1") {
                                     NewColorCode = dargv[1];

                                 } else {
                                     NewColorCode = argv[1].right(argv[1].size() - ColourPrefix.size());
                                     if (FadeSteps && Preferences::fadeStepsUseColour)
                                         NewColorCode = LDRAW_MAIN_MATERIAL_COLOUR;
                                 }
                             }

                             argv[1] = NewColorCode;
                         }

                         // fileName
                         QString FileName = QString(argv[argv.size()-1]).toLower();
                         if (FileName.contains(NameMod)) {

                             QString NewFileName = FileName.replace(NameMod,"");
                             argv[argv.size()-1] = NewFileName;
                         }

                         PartLine = joinLine(argv);

                         ModLines.removeLast();
                         ModLines.append(PartLine);

                         Stream.string()->clear();

                         for (int i = 0; i < ModLines.size(); i++)
                             Stream << ModLines.at(i);
                     }
                 }
             };

             // Process piece insert
             auto InsertPiece = [] (std::vector<std::unique_ptr<lcPiece>>& LPubPieces, lcPiece* Piece, size_t PieceIdx)
             {
                 PieceInfo* Info = Piece->mPieceInfo;

                 if (!Info->IsModel())
                 {
                     const lcMesh* Mesh = Info->GetMesh();

                     if (Mesh && Mesh->mVertexCacheOffset == -1)
                         lcGetPiecesLibrary()->mBuffersDirty = true;
                 }

                 LPubPieces.insert(LPubPieces.begin() + PieceIdx, std::unique_ptr<lcPiece>(Piece));
             };

             // Process piece add
             auto AddPiece = [&InsertPiece](std::vector<std::unique_ptr<lcPiece>>& LPubPieces, lcPiece* Piece)
             {
                 for (size_t PieceIdx = 0; PieceIdx < LPubPieces.size(); PieceIdx++)
                 {
                     if (LPubPieces[PieceIdx]->GetStepShow() > Piece->GetStepShow())
                     {
                         InsertPiece(LPubPieces, Piece, PieceIdx);
                         return;
                     }
                 }

                 InsertPiece(LPubPieces, Piece, LPubPieces.size());
             };

             // Process piece transform - add transformation and rotation delta to unrotated line
             auto TransformPiece = [&] (const QString & PieceString, const int PieceLineIndex, const bool NewPiece)
             {
                 if (NewPiece)
                     return PieceString;

                 if (PieceLineIndex == BM_INVALID_INDEX) {
                     emit lpub->messageSig(LOG_WARNING, tr("%1 BuildMod invalid piece line index for piece string %1").arg(Action, PieceString));
                     return PieceString;
                 }

                 QString ULine = lpub->ldrawFile.getViewerStepContentLine(ModStepKey, PieceLineIndex, false/*rotated*/);
                 QString RLine = lpub->ldrawFile.getViewerStepContentLine(ModStepKey, PieceLineIndex);
                 QString VLine = QString(PieceString).replace(rx,"");

                 QStringList UTokens, RTokens, VTokens;

                 split(ULine,UTokens); // Unrotated lpub line
                 split(RLine,RTokens); // Rotated lpub line
                 split(VLine,VTokens); // Rotated viewer line

                 enum TRc { X, Y, Z, N };

                 if (UTokens.size() < 15 || VTokens.size() < 15 || RTokens.size() < 15)
                   return PieceString;

                 // Result string
                 QString Result;

                 // Translation arrays
                 double TU[N];
                 double TR[N];
                 double TV[N];

                 // Rotation arrays
                 double RU[N][N];
                 double RR[N][N];
                 double RV[N][N];

                 if (UTokens[0] == "1" && VTokens[0] == "1" && RTokens[0] == "1") {
                   int C = 2;
                   TU[X] = UTokens[C]  .toFloat();
                   TU[Y] = UTokens[C+1].toFloat();
                   TU[Z] = UTokens[C+2].toFloat();
                   TR[X] = RTokens[C]  .toFloat();
                   TR[Y] = RTokens[C+1].toFloat();
                   TR[Z] = RTokens[C+2].toFloat();
                   TV[X] = VTokens[C]  .toFloat();
                   TV[Y] = VTokens[C+1].toFloat();
                   TV[Z] = VTokens[C+2].toFloat();
                   C += 3;
                   for (int y = 0; y < 3; y++) {
                     for (int x = 0; x < 3; x++) {
                       int I = C++;
                       RU[y][x] = UTokens[I].toDouble();
                       RR[y][x] = RTokens[I].toDouble();
                       RV[y][x] = VTokens[I].toDouble();
                     }
                   }

                   // Calculate translation delta
                   double TDeltaX = TV[X] - TR[X];
                   double TDeltaY = TV[Y] - TR[Y];
                   double TDeltaZ = TV[Z] - TR[Z];

                   // Add delta to unrotated translation
                   TV[X] = TU[X] + TDeltaX;
                   TV[Y] = TU[Y] + TDeltaY;
                   TV[Z] = TU[Z] + TDeltaZ;

                   // Calculate rotation delta
                   double RDelta[3][3];

                   for (int i = 0; i < 3; i++) {
                     for (int j = 0; j < 3; j++) {
                       RDelta[i][j] = 0.0;
                     }
                   }

                   for (int i = 0; i < 3; i++) {
                     for (int j = 0; j < 3; j++) {
                       RDelta[i][j] = RV[i][j] - RR[i][j];
                     }
                   }

                   // Add delta to unrotated rotation
                   for (int i = 0; i < 3; i++) {
                     for (int j = 0; j < 3; j++) {
                       RV[i][j] = RU[i][j] + RDelta[i][j];
                     }
                   }

                   // Result
                   Result = QString("1 %1 "         // Line Type and Colour
                                    "%2 %3 %4 "     // Translation
                                    "%5 %6 %7 "     // Rotation X
                                    "%8 %9 %10 "    // Rotation Y
                                    "%11 %12 %13 "  // Rotation Z
                                    "%14\r\n")      // Part Type
                                    .arg(VTokens[1])
                                    .arg(TV[X])    .arg(TV[Y])    .arg(TV[Z])
                                    .arg(RV[X][X]) .arg(RV[X][Y]) .arg(RV[X][Z])
                                    .arg(RV[Y][X]) .arg(RV[Y][Y]) .arg(RV[Y][Z])
                                    .arg(RV[Z][X]) .arg(RV[Z][Y]) .arg(RV[Z][Z])
                                    .arg(VTokens[VTokens.size()-1]);
                 }
/* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
                 qDebug() << qPrintable(QString("DEBUG: TransformPiece PieceLineIndex: %1 Lines:\n"
                                                "       Unrotated: %2\n"
                                                "       Rotated:   %3\n"
                                                "       Piece:     %4\n"
                                                "       Result:    %5")
                                                .arg(PieceLineIndex).arg(ULine).arg(RLine).arg(VLine).arg(Result));
#endif
//*/
                 return Result;
             };

             // Load LPub content - unrotated from ldrawFile
             QByteArray ByteArray;
             for (int i = 0; i < lpub->ldrawFile.contents(ModelName).size(); i++) {
                 if (i > ModActionLineNum)
                     break;
                 QString ModLine = lpub->ldrawFile.contents(ModelName).at(i);
                 if (i >= ModBeginLineNum)
                     LPubModContents.append(ModLine);
                 ByteArray.append(ModLine.toUtf8());
                 ByteArray.append(QString("\n").toUtf8());
             }
             QBuffer Buffer(&ByteArray);
             Buffer.open(QIODevice::ReadOnly);
             Buffer.seek(0);

             // Get pieces in current step
             const int ViewerPieces = ActiveModel->GetPieces().size();

             // Load LPubFileLines, LPubGroups and LPubPieces, Camera, Lights and LSynth Control Points with LDraw content using QBA buffer
             while (!Buffer.atEnd())
             {
                 qint64 Pos = Buffer.pos();
                 QString OriginalLine = Buffer.readLine();
                 QString Line = OriginalLine.trimmed();
                 QTextStream LineStream(&Line, QIODevice::ReadOnly);

                 QString Token;
                 LineStream >> Token;

                 if (Token == QLatin1String("0"))
                 {
                     LineStream >> Token;
                     if (Token == QLatin1String("FILE"))
                     {
                         QString Name = LineStream.readAll().trimmed();

                         if (ViewerProperties.mFileName != Name)
                         {
                             Buffer.seek(Pos);
                             break;
                         }

                         continue;
                     }
                     else if (Token == QLatin1String("NOFILE"))
                     {
                         break;
                     }
                     else if (Token == QLatin1String("STEP"))
                     {
                         CurrentStep++;
                         LPubFileLines.append(OriginalLine);
                         continue;
                     }

                     if (Token != QLatin1String("!LPUB"))
                     {
                         LPubFileLines.append(OriginalLine);
                         continue;
                     }

                     LineStream >> Token;

                     if (Token == QLatin1String("MODEL"))
                     {
                         ViewerProperties.ParseLDrawLine(LineStream);
                     }
                     else if (Token == QLatin1String("PIECE"))
                     {
                         if (!Piece)
                             Piece = new lcPiece(nullptr);

                         Piece->ParseLDrawLine(LineStream);
                     }
                     else if (Token == QLatin1String("CAMERA"))
                     {
                         if (!Camera)
                             Camera = new lcCamera(false);

                         Camera->SetLCMeta(false);

                         if (Camera->ParseLDrawLine(LineStream))
                         {
                             Camera->CreateName(Cameras);
                             Cameras.emplace_back(Camera);
                             Camera = nullptr;
                         }
                     }
                     else if (Token == QLatin1String("LIGHT"))
                     {
                         if (!Light)
                             Light = new lcLight(lcVector3(0.0f, 0.0f, 0.0f), lcLightType::Point);

                         Light->SetLCMeta(false);

                         if (Light->ParseLDrawLine(LineStream))
                         {
                             Light->CreateName(Lights);
                             Lights.emplace_back(Light);
                             Light = nullptr;
                         }
                     }
                     else if (Token == QLatin1String("GROUP"))
                     {
                         LineStream >> Token;

                         if (Token == QLatin1String("BEGIN"))
                         {
                             QString Name = LineStream.readAll().trimmed();
                             lcGroup* Group = GetGroup(Name, true);
                             if (!LPubGroups.empty())
                                 Group->mGroup = LPubGroups[LPubGroups.size() - 1];
                             else
                                 Group->mGroup = nullptr;
                             LPubGroups.emplace_back(Group);
                         }
                         else if (Token == QLatin1String("END"))
                         {
                             if (!LPubGroups.empty())
                                 LPubGroups.pop_back();
                         }
                     }
                     else if (Token == QLatin1String("SYNTH"))
                     {
                         LineStream >> Token;

                         if (Token == QLatin1String("BEGIN"))
                         {
                             ControlPoints.clear();
                         }
                         else if (Token == QLatin1String("END"))
                         {
                             ControlPoints.clear();
                         }
                         else if (Token == QLatin1String("CONTROL_POINT"))
                         {
                             float Numbers[13];
                             for (int TokenIdx = 0; TokenIdx < 13; TokenIdx++)
                                 LineStream >> Numbers[TokenIdx];

                             ControlPoints.emplace_back();

                             lcPieceControlPoint& PieceControlPoint = ControlPoints.back();
                             PieceControlPoint.Transform = lcMatrix44(lcVector4(Numbers[3], Numbers[9], -Numbers[6], 0.0f), lcVector4(Numbers[5], Numbers[11], -Numbers[8], 0.0f),
                                                                      lcVector4(-Numbers[4], -Numbers[10], Numbers[7], 0.0f), lcVector4(Numbers[0], Numbers[2], -Numbers[1], 1.0f));
                             PieceControlPoint.Scale = Numbers[12];
                         }
                     }

                     continue;

                 }
                 else if (Token == QLatin1String("1"))
                 {
                     int ColorCode;
                     LineStream >> ColorCode;
                     float IncludeMatrix[12];

                     for (int TokenIdx = 0; TokenIdx < 12; TokenIdx++)
                         LineStream >> IncludeMatrix[TokenIdx];

                     lcMatrix44 IncludeTransform(lcVector4(IncludeMatrix[3], IncludeMatrix[6], IncludeMatrix[9],  0.0f), lcVector4(IncludeMatrix[4], IncludeMatrix[7], IncludeMatrix[10], 0.0f),
                                                 lcVector4(IncludeMatrix[5], IncludeMatrix[8], IncludeMatrix[11], 0.0f), lcVector4(IncludeMatrix[0], IncludeMatrix[1], IncludeMatrix[2],  1.0f));

                     QString PartId = LineStream.readAll().trimmed();
                     QByteArray CleanId = PartId.toLatin1().toUpper().replace('\\', '/');

                     if (Library->IsPrimitive(CleanId.constData()))
                     {
                         LPubFileLines.append(OriginalLine);
                     }
                     else
                     {
                         if (!Piece)
                             Piece = new lcPiece(nullptr);

                         if (!LPubGroups.empty())
                             Piece->SetGroup(LPubGroups[LPubGroups.size() - 1]);

                         PieceInfo* Info = Library->FindPiece(PartId.toLatin1().constData(), lcGetActiveProject(), true, true);

                         float* Matrix = IncludeTransform.GetFloats();
                         lcMatrix44 Transform(lcVector4( Matrix[0],  Matrix[2], -Matrix[1], 0.0f), lcVector4(Matrix[8],  Matrix[10], -Matrix[9],  0.0f),
                                              lcVector4(-Matrix[4], -Matrix[6],  Matrix[5], 0.0f), lcVector4(Matrix[12], Matrix[14], -Matrix[13], 1.0f));

                         Piece->SetFileLine(LPubFileLines.size());
                         Piece->SetPieceInfo(Info, PartId, false);
                         Piece->Initialize(Transform, quint32(CurrentStep));
                         Piece->SetColorCode(quint32(ColorCode));
                         Piece->SetControlPoints(ControlPoints);
                         AddPiece(LPubPieces, Piece);

                         Piece = nullptr;
                     }
                 }
                 else
                     LPubFileLines.append(OriginalLine);

                 Library->WaitForLoadQueue();
                 Library->mBuffersDirty = true;
                 Library->UnloadUnusedParts();

                 delete Piece;
                 delete Camera;
             } // Load LPub content buffer

             Buffer.close();

             // Load Visual Editor content from ViewerFileLines, ViewerGroups, Camera, Lights and LSynth Control Points
             bool NewPiece      = false;
             bool SelectedOnly  = false;
             bool PieceInserted = false;
             bool PieceModified = true;
             QLatin1String LineEnding("\r\n");

             QString ViewerModContentsString;
             QTextStream Stream(&ViewerModContentsString, QIODevice::ReadWrite);

             lcStep Step       = 1;
             int CurrentLine   = ModBeginLineNum/*0*/;
             int AddedSteps    = 0;

             int LineIndex     = NEW_PART;
             int LineNumber    = 0;
             int PieceAdjustment = 0;
             int EndModLineNum = ModActionLineNum;

             // Do we have a difference between the number of LPub pieces and Viewer pieces ?
             // If pieces have been added or removed, we capture the delta in PieceAdjustment
             // and AddedPieces.
             PieceAdjustment = AddedPieces = Update ? ViewerPieces - ModStepPieces : ViewerPieces - getViewerStepPartCount(ModStepKey);

             // Adjust EndModLineNum to accomodate removed pieces
             if (PieceAdjustment < 0)
                 EndModLineNum -= PieceAdjustment;

             if (Preferences::debugLogging) {
                 const QString message =
                         QString("%1 BuildMod - %2Pieces [%3], Viewer Pieces Count [%4], %5 Pieces Count [%6]")
                                 .arg(Action)
                                 .arg(PieceAdjustment == 0 ? "" : PieceAdjustment > 0 ? "Added " : "Removed ")
                                 .arg(PieceAdjustment  < 0 ? -PieceAdjustment : PieceAdjustment)
                                 .arg(ViewerPieces).arg(VER_PRODUCTNAME_STR).arg(ModStepPieces);
                 emit gui->messageSig(LOG_DEBUG, message);
             }

             // Viewer current step pieces
             for (const std::unique_ptr<lcPiece>& Piece : ActiveModel->GetPieces())
             {
                 LineIndex  = Piece->GetLineTypeIndex();

                 // We have a new piece (NewPiece set to true) when the piece LineIndex is -1
                 NewPiece   = !getSelectedLine(ModelIndex, LineIndex, VIEWER_MOD, LineNumber);

                 // If PieceInserted, we must increment the 'original' line number for the next piece
                 if (PieceInserted)
                 {
                     LineNumber   += PieceInserted;
                 }

                 // Added pieces have a line number of 0 so we must reset the LineNumber to EndModLineNum
                 // and set PieceInserted to true so we can increment the line number for the next piece.
                 // For each NewPiece, we'll decrement PieceAdjustment.
                 if ((PieceInserted = NewPiece))
                 {
                     PieceAdjustment--;
                     LineNumber = EndModLineNum;
                 }

                 // Set PieceModified. Only modified pieces are added to ViewerModContents
                 PieceModified = LineNumber <= ModActionLineNum || Piece->PieceModified() || NewPiece;

                 if (Preferences::debugLogging)
                 {
                     const QString message =
                             QString("%1 BuildMod Viewer Piece - LineIndex [%2], ID [%3], Name [%4], LineNumber [%5], Modified: [%6]")
                                     .arg(Action,
                                          LineIndex < 0 ? QString::number(LineIndex) + "], Added Piece [#"+QString::number(AddedPieces) : QString::number(LineIndex),
                                          Piece->GetID(),
                                          Piece->GetName())
                                     .arg(LineNumber)
                                     .arg(PieceModified ? QLatin1String("Yes") : QLatin1String("No"));
                     emit gui->messageSig(LOG_DEBUG, message);
                 }

                 // If PieceAdjustment is not 0, we increment EndModLineNum as we process each piece
                 if (PieceAdjustment > 0)
                      EndModLineNum = LineNumber + 1;

                 // No more modified pieces so we exit the loop
                 if (LineNumber > EndModLineNum)
                     break;

                 // Process the current piece
                 if (LineNumber >= ModBeginLineNum && PieceModified)
                 {

                     // Use LPubFileLines
                     while (LineNumber/*Piece->GetFileLine()*/ > CurrentLine && CurrentLine < LPubFileLines.size()/*ViewerFileLines.size()*/)
                     {
                         QString Line = LPubFileLines[CurrentLine]/*ViewerFileLines[CurrentLine]*/;
                         QTextStream LineStream(&Line, QIODevice::ReadOnly);

                         QString Token;
                         LineStream >> Token;
                         bool Skip = false;

                         if (Token == QLatin1String("0"))
                         {
                             LineStream >> Token;

                             if (Token == QLatin1String("STEP"))
                             {
                                 if (Piece->GetStepShow() > Step)
                                     Step++;
                                 else
                                     Skip = true;
                             }
                         }

                         if (!Skip)
                         {
                             Stream << LPubFileLines[CurrentLine]/*ViewerFileLines[CurrentLine]*/;
                             if (AddedSteps > 0)
                                 AddedSteps--;
                         }
                         CurrentLine++;
                     }

                     // Use Viewer Pieces

                     // Skip previous and hidden steps
                     while (Piece->GetStepShow() > Step)
                     {
                         Stream << QLatin1String("0 STEP\r\n");
                         AddedSteps++;
                         Step++;
                     }

                     // Process piece groups
                     lcGroup* PieceGroup = Piece->GetGroup();

                     if (PieceGroup)
                     {
                         if (ViewerGroups.empty() || (!ViewerGroups.empty() && PieceGroup != ViewerGroups[ViewerGroups.size() - 1]))
                         {
                             std::deque<lcGroup*> PieceParents;

                             for (lcGroup* Group = PieceGroup; Group; Group = Group->mGroup)
                                 PieceParents.push_front(Group);

                             std::deque<lcGroup*>::iterator ParentsToAdd = PieceParents.begin();

                             while (!ViewerGroups.empty())
                             {
                                 lcGroup* Group = ViewerGroups[ViewerGroups.size() - 1];
                                 const std::deque<lcGroup*>::iterator ParentFound = std::find(PieceParents.begin(), PieceParents.end(), Group);

                                 if (ParentFound == PieceParents.end())
                                 {
                                     ViewerGroups.pop_back();
                                     Stream << QLatin1String(LPMeta + " GROUP END\r\n");
                                 }
                                 else
                                 {
                                     ParentsToAdd = ParentFound + 1;
                                     break;
                                 }
                             }

                             for (std::deque<lcGroup*>::iterator ParentIt = ParentsToAdd; ParentIt != PieceParents.end(); ParentIt++)
                             {
                                 lcGroup* Group = *ParentIt;
                                 ViewerGroups.emplace_back(Group);
                                 Stream << QLatin1String(LPMeta + " GROUP BEGIN ") << Group->mName << LineEnding;
                             }
                         }
                     }
                     else
                     {
                         while (ViewerGroups.size())
                         {
                             ViewerGroups.pop_back();
                             Stream << QLatin1String(LPMeta + " GROUP END\r\n");
                         }
                     }

                     // Process piece synth info
                     if (Piece->mPieceInfo->GetSynthInfo())
                     {
                         Stream << QLatin1String(LPMeta + " SYNTH BEGIN\r\n");

                         const std::vector<lcPieceControlPoint>& ControlPoints = Piece->GetControlPoints();
                         for (size_t ControlPointIdx = 0; ControlPointIdx < ControlPoints.size(); ControlPointIdx++)
                         {
                             const lcPieceControlPoint& ControlPoint = ControlPoints[ControlPointIdx];

                             Stream << QLatin1String(LPMeta + " SYNTH CONTROL_POINT");

                             const float* FloatMatrix = ControlPoint.Transform.GetFloats();
                             float Numbers[13] = { FloatMatrix[12], -FloatMatrix[14], FloatMatrix[13], FloatMatrix[0], -FloatMatrix[8], FloatMatrix[4], -FloatMatrix[2], FloatMatrix[10], -FloatMatrix[6], FloatMatrix[1], -FloatMatrix[9], FloatMatrix[5], ControlPoint.Scale };

                             for (int NumberIdx = 0; NumberIdx < 13; NumberIdx++)
                                 Stream << ' ' << lcFormatValue(Numbers[NumberIdx], NumberIdx < 3 ? 4 : 6);

                             Stream << LineEnding;
                         }
                     }

                     QString PieceString;
                     QTextStream PieceStream(&PieceString, QIODevice::ReadWrite);

                     // Write piece lines to piece string
                     Piece->SaveLDraw(PieceStream);

                     // Set piece lines to stream
                     Stream << TransformPiece(PieceString, LineIndex, NewPiece);;

                     // Process fade previous steps and highlight current step colour lines
                     if (FadeSteps || HighlightStep)
                         ConfigureFadeHighlightPartLine(Piece->GetLineTypeIndex(), Stream);

                     // End piece synth info
                     if (Piece->mPieceInfo->GetSynthInfo())
                         Stream << QLatin1String(LPMeta + " SYNTH END\r\n");
                 } // Process the current piece
             } // Viewer current step pieces

             // Process to bottom of current step
             while (CurrentLine < LPubFileLines.size()/*ViewerFileLines.size()*/)
             {
                 QString Line = LPubFileLines[CurrentLine]/*ViewerFileLines[CurrentLine]*/;
                 QTextStream LineStream(&Line, QIODevice::ReadOnly);

                 QString Token;
                 LineStream >> Token;
                 bool Skip = false;

                 if (Token == QLatin1String("0"))
                 {
                     LineStream >> Token;

                     if (Token == QLatin1String("STEP") && AddedSteps-- > 0)
                         Skip = true;
                 }

                 if (!Skip)
                     Stream << LPubFileLines[CurrentLine]/*ViewerFileLines[CurrentLine]*/;
                 CurrentLine++;
             } // Process to bottom of current step

             // Process step group end
             while (ViewerGroups.size())
             {
                 ViewerGroups.pop_back();
                 Stream << QLatin1String(LPMeta + " GROUP END\r\n");
             }

             // Process camera lines
             for (const std::unique_ptr<lcCamera>& Camera : Cameras)
                 if (!SelectedOnly || Camera->IsSelected())
                     Camera->SaveLDraw(Stream);

             // Process light lines
             for (const std::unique_ptr<lcLight>& Light : Lights)
                 if (!SelectedOnly || Light->IsSelected())
                     Light->SaveLDraw(Stream);

             // Set step stream (of lines) to content list
             ViewerModContents = QString(ViewerModContentsString).split(rx, SkipEmptyParts);

             int BuildModPieces = ViewerModContents.size();
/* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
             QStringList Items;
             for (const int &LineIndex: currentStep->lineTypeIndexes)
                 Items << QString::number(LineIndex);
             if (Items.size())
                 qDebug() << qPrintable(QString("DEBUG: LPubStepLineTypeIndexes: %1").arg(Items.join(" ")));

             for (const QString &Line : LPubModContents)
                 qDebug() << qPrintable(QString("DEBUG: LPubModContents:   %1").arg(Line));

             for (const QString &Line : ViewerModContents)
                 qDebug() << qPrintable(QString("DEBUG: ViewerModContents: %1").arg(Line));
#endif
//*/
             // Generate build modification key
             if (!Update) {
                 int buildModCount = lpub->ldrawFile.getBuildModsCount(ModelName) + 1;
                 BuildModKey = QString("%1 Mod %2").arg(ModelName).arg(buildModCount);
                 while (lpub->ldrawFile.getBuildModExists(ModelName, BuildModKey))
                   BuildModKey = QString("%1 Mod %2").arg(ModelName).arg(++buildModCount);
             }

             // Delete meta commands uses the 'original' BuildMod values
             int SaveModBeginLineNum  = BM_INIT;
             int SaveModActionLineNum = BM_INIT;
             int SaveModEndLineNum    = BM_INIT;
             int SaveModPieces        = BM_INIT;
             if (Update) {
                 SaveModBeginLineNum  = BuildModBegin;
                 SaveModActionLineNum = BuildModAction;
                 SaveModEndLineNum    = BuildModEnd;
                 SaveModPieces        = SaveModEndLineNum - SaveModActionLineNum - 1/*Meta Line*/;
             } else {
                 SaveModBeginLineNum  = ModBeginLineNum;
                 SaveModActionLineNum = ModActionLineNum;
                 SaveModEndLineNum    = ModEndLineNum;
                 SaveModPieces        = BuildModPieces - (AddedPieces > 0 ? AddedPieces : 0);
             }

             if (Preferences::debugLogging) {
                 const QString message = QString("%1 BuildMod Save LineNumbers - "
                                                 "Begin: %2, Action: %3, End: %4, ModPieces: %5")
                                                 .arg(Action)
                                                 .arg(SaveModBeginLineNum)
                                                 .arg(SaveModActionLineNum)
                                                 .arg(SaveModEndLineNum)
                                                 .arg(SaveModPieces);
                 emit gui->messageSig(LOG_DEBUG, message);
             }

             // BuildMod meta command lines are written in a bottom up manner

             // Set ModBeginLineNum to the top of the BuildMod step plus the number of 'AddedPieces' introduced by the BuildMod command
             // This is the position for BUILD_MOD END
             ModBeginLineNum += SaveModPieces;

             // Set ModActionLineNum to SaveModBeginLineNum - initial BuildMod insertion line
             // This is the position of BUILD_MOD MOD_END
             ModActionLineNum = SaveModBeginLineNum;

             BuildModData buildModData;

             Where modHere;

             buildModData = currentStep->buildModMeta.value();

             QString metaString;
             buildModData.buildModKey = QString();

             if (!Update)
                 currentStep->buildModMeta.preamble = QLatin1String(LPMeta + " BUILD_MOD ");

             beginMacro(QLatin1String("BuildModCreate|") + lpub->viewerStepKey);

             if (BuildModPieces) {
                 Meta meta;
                 // Delete old BUILD_MOD END meta command
                 Where endMod = Where(ModelName, SaveModEndLineNum);
                 QString modLine = readLine(endMod);
                 Rc rc = meta.parse(modLine, endMod);
                 if (rc == BuildModEndRc)
                     deleteLine(endMod);

                 // Delete old BUILD_MOD content from bottom up including END_MOD meta command
                 endMod = Where(ModelName, SaveModActionLineNum);
                 modLine = readLine(endMod);
                 rc = meta.parse(modLine, endMod);
                 if (rc == BuildModEndModRc)
                     for (modHere = endMod; modHere >= SaveModBeginLineNum; --modHere)
                         deleteLine(modHere);

                 // Write BUILD_MOD END meta command at the BuildMod insert position
                 modHere = Where(ModelName, ModBeginLineNum);
                 buildModData.action      = BuildModEndRc;
                 currentStep->buildModMeta.setValue(buildModData);
                 metaString = currentStep->buildModMeta.format(false/*local*/,false/*global*/);
                 insertLine(modHere, metaString, nullptr);

                 // Write BUILD_MOD END_MOD meta command above LPub content first line - last to first
                 modHere = Where(ModelName, ModActionLineNum);
                 buildModData.action      = BuildModEndModRc;
                 currentStep->buildModMeta.setValue(buildModData);
                 metaString = currentStep->buildModMeta.format(false,false);
                 insertLine(modHere, metaString, nullptr);

                 // Write buildMod content last to first
                 for (int i = BuildModPieces - 1; i >= 0; --i) {
                     metaString = ViewerModContents.at(i);
                     insertLine(modHere, metaString, nullptr);
                 }

                 // Write BUILD_MOD BEGIN meta command above buildMod content first line
                 buildModData.action      = BuildModBeginRc;
                 buildModData.buildModKey = BuildModKey;
                 currentStep->buildModMeta.setValue(buildModData);
                 metaString = currentStep->buildModMeta.format(false,false);
                 insertLine(modHere, metaString, nullptr);

                 clearWorkingFiles(getPathsFromViewerStepKey(lpub->viewerStepKey));
             }

             // BuildMod attribute values are calculated in a top down manner

             // Reset to BUILD_MOD BEGIN
             ModBeginLineNum = SaveModBeginLineNum;

             // Reset to BUILD_MOD END_MOD command line number - add 1 for meta command
             ModActionLineNum = ModBeginLineNum + BuildModPieces + 1;

             // Set to BUILD_MOD END command line number - add 1 for meta command
             ModEndLineNum = ModActionLineNum + SaveModPieces + 1;

             // Set to updated number of pieces in the current Step
             ModStepPieces = ViewerPieces;

             QVector<int> ModAttributes =
             {
                 ModBeginLineNum,            // 0 BM_BEGIN_LINE_NUM
                 ModActionLineNum,           // 1 BM_ACTION_LINE_NUM
                 ModEndLineNum,              // 2 BM_END_LINE_NUM
                 ModDisplayPageNum,          // 3 BM_DISPLAY_PAGE_NUM
                 ModStepPieces,              // 4 BM_STEP_PIECES
                 ModelIndex,                 // 5 BM_MODEL_NAME_INDEX
                 ModStepLineNum,             // 6 BM_MODEL_LINE_NUM
                 ModStepNum                  // 7 BM_MODEL_STEP_NUM
             };

             insertBuildMod(BuildModKey,
                            ModAttributes,
                            ModStepIndex);   // Unique ID

             if (Preferences::debugLogging) {
                 const QString message =
                         QString("%1 BuildMod - StepIndex: %2, "
                                 "Action: Apply(64), "
                                 "Attributes: %3 %4 %5 %6 %7 %8 %9 %10, "
                                 "ModKey: %11, "
                                 "Level: %12")
                                 .arg(Action)   // 01
                                 .arg(ModStepIndex)                   // 02     ModAttributes:
                                 .arg(ModBeginLineNum)                // 03 - 0 BM_BEGIN_LINE_NUM
                                 .arg(ModActionLineNum)               // 04 - 1 BM_ACTION_LINE_NUM
                                 .arg(ModEndLineNum)                  // 05 - 2 BM_END_LINE_NUM
                                 .arg(ModDisplayPageNum)              // 06 - 3 BM_DISPLAY_PAGE_NUM
                                 .arg(ModStepPieces)                  // 07 - 4 BM_STEP_PIECES
                                 .arg(ModelIndex)                     // 08 - 5 BM_MODEL_NAME_INDEX
                                 .arg(ModStepLineNum)                 // 09 - 6 BM_MODEL_LINE_NUM
                                 .arg(ModStepNum)                     // 10 - 7 BM_MODEL_STEP_NUM
                                 .arg(ModStepKey,                     // 11
                                      BuildModKey);                   // 12
                 emit gui->messageSig(LOG_DEBUG, message);
             }

             endMacro();

             // Reset the build modification range
             clearBuildModRange();

         } // mBuildModRange || Update

         gui->progressPermStatusRemove();

         emit gui->messageSig(LOG_INFO_STATUS, tr("Build modification '%1' created at step %2")
                                                  .arg(BuildModKey).arg(lpub->currentStep->stepNumber.number));
     }
 }

void Gui::applyBuildModification()
{
    Step *currentStep = lpub->currentStep;

    if (!currentStep || Gui::exporting())
        return;

//* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
     LDrawFile *ldrawFile = &lpub->ldrawFile;
     Q_UNUSED(ldrawFile)
#endif
//*/

    QString buildModKey;
    QStringList buildModKeys;
    if (buildModsCount() == 1) {
        buildModKeys = getBuildModsList();
    } else {
        BuildModDialogGui *buildModDialogGui = new BuildModDialogGui();
        buildModDialogGui->getBuildMod(buildModKeys, BuildModApplyRc);
    }

    if (buildModKeys.size())
        buildModKey = buildModKeys.first();

    if (buildModKey.isEmpty())
        return;

    emit gui->messageSig(LOG_INFO_STATUS, tr("Processing build modification 'Apply' action..."));

    Where topOfStep = currentStep->topOfStep();

    // get the last action for this build modification
    Rc buildModAction = static_cast<Rc>(getBuildModAction(buildModKey, getBuildModStepIndex(topOfStep)));

    const QString model(topOfStep.modelName);
    const QString line(QString::number(topOfStep.lineNumber));
    const QString step(QString::number(currentStep->stepNumber.number));
    QString text, type, title;
    if (getBuildModStepKeyModelIndex(buildModKey) == getSubmodelIndex(model) && getBuildModStepKeyStepNum(buildModKey) > step.toInt()) {
        text  = tr("Build modification '%1' was created after this step (%2), model '%3', at line %4.<br>"
                   "Applying a build modification before it is created is not supported.<br><br>No action taken.<br>")
                   .arg(buildModKey, step, model, line);
        type  = tr("apply build modification error message");
        title = tr("Build Modification");

        Preferences::MsgID msgID(Preferences::BuildModEditErrors, Where(QLatin1String("Apply_Before_") + model,line).nameToString());
        Preferences::showMessage(msgID, text, title, type);

        return;

    } else if (getBuildModStepKey(buildModKey) == currentStep->viewerStepKey) {
        text  = tr("Build modification '%1' was created in this step (%2), model '%3', at line %4.<br>"
                   "It was automatically applied to the step it was created in.<br><br>No action taken.<br>")
                   .arg(buildModKey, step, model, line);
        type  = tr("apply build modification error message");
        title = tr("Build Modification");

        Preferences::MsgID msgID(Preferences::BuildModEditErrors, Where(QLatin1String("Begin_") + model,line).nameToString());
        Preferences::showMessage(msgID, text, title, type);

        return;

    } else if (buildModAction == BuildModApplyRc) {
        text  = tr("Build modification '%1' was already applied to step (%2), model '%3'.<br><br>No action taken.<br>")
                .arg(buildModKey, step, model);
        type  = tr("apply build modification error message");
        title = tr("Build Modification");

        Preferences::MsgID msgID(Preferences::BuildModEditErrors, Where(QLatin1String("Already_Applied_") + model,line).nameToString());
        Preferences::showMessage(msgID, text, title, type);

        return;

    } /* else {
        text  = tr("This action will apply build modification '%1' "
                   "beginning at step (%2), in model '%3'.<br><br>Are you sure ?<br>")
                   .arg(buildModKey, step, model);
        type  = tr("apply build modification message");
        title = tr("Build Modification");

        Preferences::MsgID msgID(Preferences::BuildModEditErrors, Where(model,line).nameToString());
        switch (Preferences::showMessage(msgID, text, title, type, true / *option* /))
        {
        default:
        case QMessageBox::Cancel:
            return; // cancel request
        case QMessageBox::Ok:
            break;  // apply changes
        }
    } */

    using namespace Options;
    Mt it = static_cast<Mt>(lcGetActiveProject()->GetImageType());
    if (it == CSI) {
        BuildModData buildModData = currentStep->buildModActionMeta.value();
        buildModData.action       = BuildModApplyRc;
        buildModData.buildModKey  = buildModKey;
        currentStep->buildModActionMeta.setValue(buildModData);

        beginMacro(QLatin1String("BuildModApply|") + currentStep->viewerStepKey);

        QString metaString        = currentStep->buildModActionMeta.format(false/*local*/,false/*global*/);

        Where top                 = topOfStep;

        Meta meta;

        QString line = lpub->ldrawFile.readLine(top.modelName, top.lineNumber + 1);
        Rc rc = meta.parse(line,top);
        if (rc == StepGroupEndRc)
            top++;
        line = lpub->ldrawFile.readLine(top.modelName, top.lineNumber + 1);
        rc = meta.parse(line,top);
        if (rc == StepGroupBeginRc)
            top++;

        currentStep->mi(it)->setMetaAlt(top, metaString, true/*newCommand*/, true/*removeActionCommand*/);

        clearWorkingFiles(getPathsFromViewerStepKey(currentStep->viewerStepKey));

        endMacro();

        emit gui->messageSig(LOG_INFO_STATUS, tr("Build modification '%1' applied at step %1")
                                                 .arg(buildModKey).arg(lpub->currentStep->stepNumber.number));
    }
}

void Gui::removeBuildModification()
{
    Step *currentStep = lpub->currentStep;

    if (!currentStep || Gui::exporting())
        return;

//* local ldrawFile and step used for debugging
#ifdef QT_DEBUG_MODE
     LDrawFile *ldrawFile = &lpub->ldrawFile;
     Q_UNUSED(ldrawFile)
#endif
//*/

     QString buildModKey;
     QStringList buildModKeys;
     if (buildModsCount() == 1) {
         buildModKeys = getBuildModsList();
     } else {
         BuildModDialogGui *buildModDialogGui = new BuildModDialogGui();
         buildModDialogGui->getBuildMod(buildModKeys, BuildModRemoveRc);
     }

     if (buildModKeys.size())
         buildModKey = buildModKeys.first();

     if (buildModKey.isEmpty()) {
         return;
     }

    emit gui->messageSig(LOG_INFO_STATUS, tr("Processing build modification 'Remove' action..."));

    Where topOfStep = currentStep->topOfStep();

    // get the last action for this build modification
    Rc buildModAction = static_cast<Rc>(getBuildModAction(buildModKey, getBuildModStepIndex(topOfStep)));

    const QString model(topOfStep.modelName);
    const QString line(QString::number(topOfStep.lineNumber));
    const QString step(QString::number(currentStep->stepNumber.number));
    QString text, type, title;
    if (getBuildModStepKeyModelIndex(buildModKey) == getSubmodelIndex(model) && getBuildModStepKeyStepNum(buildModKey) > step.toInt()) {
        text  = tr("Build modification '%1' was created after this step (%2), model '%3', at line %4.<br>"
                   "Removing a build modification before it is created is not supported.<br><br>No action taken.<br>")
                   .arg(buildModKey, step, model, line);
        type  = tr("remove build modification error message");
        title = tr("Build Modification");

        return;

        Preferences::MsgID msgID(Preferences::BuildModEditErrors, Where(QLatin1String("Remove_Before_") + model,line).nameToString());
        Preferences::showMessage(msgID, text, title, type);
    } else if (getBuildModStepKey(buildModKey) == lpub->viewerStepKey) {
        text  = tr("Build modification '%1' was created in this step (%2), in model '%3' at line %4.<br><br>"
                   "It cannot be removed from the step it was created in.<br><br>"
                   "Select 'Delete Build Modification' to delete from '%3', step %2 at line %4")
                   .arg(buildModKey, step, model, line);
        type  = tr("remove build modification error message");
        title = tr("Build Modification");

        Preferences::MsgID msgID(Preferences::BuildModEditErrors, Where(QLatin1String("Not_Applied_") + model,line).nameToString());
        Preferences::showMessage(msgID, text, title, type);

        return;

    } else if (buildModAction == BuildModRemoveRc) {
        text  = tr("Build modification '%1' was already removed from step (%2), model '%3.<br><br>No action taken.<br>")
                   .arg(buildModKey, step, model);
        type  = tr("remove build modification error message");
        title = tr("Build Modification");

        Preferences::MsgID msgID(Preferences::BuildModEditErrors, Where(QLatin1String("Already_Removed_") + model,line).nameToString());
        Preferences::showMessage(msgID, text, title, type);

        return;

    } /* else {
        text  = tr("This action will remove build modification '%1' "
                   "beginning at step (%2) in model '%3'.<br><br>Are you sure ?<br>")
                   .arg(buildModKey, step, model);
        type  = tr("remove build modification message");
        title = tr("Build Modification");

        Preferences::MsgID msgID(Preferences::BuildModEditErrors, Where(model,line).nameToString());
        switch (Preferences::showMessage(msgID, text, title, type, true / *option* /))
        {
        default:
        case QMessageBox::Cancel:
            return;
        case QMessageBox::Ok:
            break;
        }
    } */

    using namespace Options;
    Mt it = static_cast<Mt>(lcGetActiveProject()->GetImageType());
    if (it == CSI) {
        BuildModData buildModData = currentStep->buildModActionMeta.value();
        buildModData.action       = BuildModRemoveRc;
        buildModData.buildModKey  = buildModKey;
        currentStep->buildModActionMeta.setValue(buildModData);
        currentStep->buildModActionMeta.preamble = QLatin1String("0 !LPUB BUILD_MOD ");

        beginMacro(QLatin1String("BuildModRemove|") + currentStep->viewerStepKey);

        QString metaString = currentStep->buildModActionMeta.format(false/*local*/,false/*global*/);

        Where top          = topOfStep;

        Meta meta;

        QString line = lpub->ldrawFile.readLine(top.modelName, top.lineNumber + 1);
        Rc rc = meta.parse(line,top);
        if (rc == StepGroupEndRc)
            top++;
        line = lpub->ldrawFile.readLine(top.modelName, top.lineNumber + 1);
        rc = meta.parse(line,top);
        if (rc == StepGroupBeginRc)
            top++;

        currentStep->mi(it)->setMetaAlt(top , metaString, true/*newCommand*/, true/*removeActionCommand*/);

        clearWorkingFiles(getPathsFromViewerStepKey(currentStep->viewerStepKey));

        endMacro();

        emit gui->messageSig(LOG_INFO_STATUS, tr("Build modification '%1' removed at step %2")
                                            .arg(buildModKey).arg(lpub->currentStep->stepNumber.number));
    }
}

void Gui::deleteBuildModificationAction()
{
    Step *currentStep = lpub->currentStep;

    if (!currentStep || Gui::exporting())
        return;

//* local ldrawFile and step used for debugging
#ifdef QT_DEBUG_MODE
     LDrawFile *ldrawFile = &lpub->ldrawFile;
     Q_UNUSED(ldrawFile)
#endif
//*/

    QString buildModKey = currentStep->buildModActionMeta.value().buildModKey;

    if (buildModKey.isEmpty()) {
        setBuildModificationKey(BuildModRemoveRc);
        if (buildModificationKey.isEmpty())
            setBuildModificationKey(BuildModApplyRc);
        if (buildModificationKey.isEmpty()) {
            QStringList buildModKeys;
            if (buildModsCount() == 1) {
                buildModKeys = getBuildModsList();
            } else {
                BuildModDialogGui *buildModDialogGui = new BuildModDialogGui();
                buildModDialogGui->getBuildMod(buildModKeys, BM_DELETE);
            }
            if (buildModKeys.size())
                buildModKey = buildModKeys.first();
        } else {
            buildModKey = buildModificationKey;
            buildModificationKey.clear();
        }
    }

    if (buildModKey.isEmpty())
        return;

    Where topOfStep = currentStep->topOfStep();

    // was the last action defined in this step ?
    Rc buildModStepAction =  static_cast<Rc>(getBuildModStepAction(topOfStep));

    // determine current step action
    QString macroString, actionString;
    switch (buildModStepAction)
    {
        case BuildModApplyRc:
            actionString = QLatin1String(BUILD_MOD_APPLY);
            macroString = QLatin1String("DeleteBuildModApplyAction|");
            break;
        case BuildModRemoveRc:
            actionString = QLatin1String(BUILD_MOD_REMOVE);
            macroString = QLatin1String("DeleteBuildModRemoveAction|");
            break;
        case BuildModNoActionRc:
        default:
            break;
    }

    emit gui->messageSig(LOG_INFO_STATUS, tr("Processing build modification 'Remove Action' action..."));

    const QString model(topOfStep.modelName);
    const QString line(QString::number(topOfStep.lineNumber));
    const QString step(QString::number(currentStep->stepNumber.number));
    QString text, type, title;
    if (getBuildModStepKeyModelIndex(buildModKey) == getSubmodelIndex(model) && getBuildModStepKeyStepNum(buildModKey) > step.toInt()) {
            text  = tr("Build modification '%1' was created after this step (%2), model '%3', at line %4.<br>"
                       "Removing a build modification before it is created is not supported.<br><br>No action taken.<br>")
                       .arg(buildModKey, step, model, line);
            type  = tr("remove build modification error message");
            title = tr("Build Modification");

            return;

            Preferences::MsgID msgID(Preferences::BuildModEditErrors, Where(QLatin1String("Remove_Before_") + model,line).nameToString());
            Preferences::showMessage(msgID, text, title, type);
    } else if (getBuildModStepKey(buildModKey) == lpub->viewerStepKey) {
        text  = tr("Build modification '%1' was created in this step (%2), in model '%3' at line %4.<br><br>"
                   "It cannot be removed from the step it was created in.<br><br>"
                   "Select 'Delete Build Modification' to delete from '%3', "
                   "step %2 at line %4")
                   .arg(buildModKey, step, model, line);
        type  = tr("remove build modification error message");
        title = tr("Build Modification");

        Preferences::MsgID msgID(Preferences::BuildModEditErrors, Where(QLatin1String("Not_Applied_") + model,line).nameToString());
        Preferences::showMessage(msgID, text, title, type);

        return;
    }

    using namespace Options;
    Mt it = static_cast<Mt>(lcGetActiveProject()->GetImageType());
    if (it == CSI) {
        BuildModData buildModData = currentStep->buildModActionMeta.value();
        buildModData.action       = buildModStepAction;
        buildModData.buildModKey  = buildModKey;
        currentStep->buildModActionMeta.setValue(buildModData);
        currentStep->buildModActionMeta.preamble = QLatin1String("0 !LPUB BUILD_MOD ");

        beginMacro(macroString + currentStep->viewerStepKey);

        QString metaString = currentStep->buildModActionMeta.format(false/*local*/,false/*global*/);

        currentStep->mi(it)->setMetaAlt(currentStep->buildModActionMeta.here(), metaString, false/*newCommand*/, true/*removeCommand*/);

        clearBuildModAction(buildModKey, getBuildModStepIndex(topOfStep));

        clearWorkingFiles(getPathsFromViewerStepKey(currentStep->viewerStepKey));

        endMacro();

        emit gui->messageSig(LOG_INFO_STATUS, tr("Build modification '%1' %2 action deleted at step %1")
                                            .arg(buildModKey, actionString.toLower()).arg(lpub->currentStep->stepNumber.number));
    }
}

void Gui::loadBuildModification()
{
    Step *currentStep = lpub->currentStep;

    if (!currentStep || Gui::exporting())
        return;

    QString buildModKey;
    QStringList buildModKeys;
    if (buildModsCount() == 1) {
        buildModKeys = getBuildModsList();
    } else {
        BuildModDialogGui *buildModDialogGui = new BuildModDialogGui();
        buildModDialogGui->getBuildMod(buildModKeys, BM_CHANGE);
    }

    if (buildModKeys.size())
        buildModKey = buildModKeys.first();

    if (buildModKey.isEmpty())
        return;

    emit gui->messageSig(LOG_INFO_STATUS, tr("Processing build modification 'Load' action..."));

    /*
    QString model = "undefined", line = "undefined", step = "undefined";
    QStringList keys = getViewerStepKeys(true/ *get Name* /, false/ *pliPart* /, getBuildModStepKey(buildModKey));
    if (keys.size() > 2) { model = keys[0]; line = keys[1]; step = keys[2]; }
    QString text  = tr("This action will load build modification '%1' "
                       ", step (%2), model '%3' into the Visual Editor "
                       "to allow editing.<br><br>Are you sure ?<br>")
                       .arg(buildModKey).arg(step).arg(model);
    QString type  = tr("load build modification");
    QString title = tr("Build Modification");
    Preferences::MsgID msgID(Preferences::BuildModEditErrors, Where(model,line).nameToString());
    switch (Preferences::showMessage(msgID, text, title, type, true / *option* /))
    {
    default:
    case QMessageBox::Cancel:
        return;
    case QMessageBox::Ok:
        break;
    }
    */

    using namespace Options;
    Mt it = static_cast<Mt>(lcGetActiveProject()->GetImageType());
    if (it == CSI) {

        buildModificationKey.clear();

        const int buildModDisplayPageNum = getBuildModDisplayPageNumber(buildModKey);

        const QString buildModStepKey = getBuildModStepKey(buildModKey);

        const int displayPageNumber = Gui::displayPageNum;

        if (buildModDisplayPageNum && ! buildModStepKey.isEmpty()) {

            int stepNumber = currentStep->stepNumber.number;

            if (buildModDisplayPageNum != displayPageNumber) {
                cyclePageDisplay(buildModDisplayPageNum);
                currentStep = lpub->currentStep;
                stepNumber = currentStep->stepNumber.number;
            }

            bool setBuildModStep = currentStep && currentStep->viewerStepKey != buildModStepKey;

            if (isViewerStepMultiStep(buildModStepKey) && setBuildModStep) {
                if (lpub->setCurrentStep(buildModStepKey)) {
                    currentStep = lpub->currentStep;
                    if (currentStep) {
                        currentStep->loadTheViewer();
                        showLine(currentStep->topOfStep());
                        if (gMainWindow)
                            gMainWindow->UpdateDefaultCameraProperties();
                        stepNumber = currentStep->stepNumber.number;
                    }
                }
            }

            buildModificationKey = buildModKey;

            emit gui->messageSig(LOG_INFO_STATUS, tr("Step %1 with build modification '%1' loaded")
                                                .arg(stepNumber).arg(buildModKey));
        }
    }
}

void Gui::setUndoRedoBuildModAction(bool Undo)
{
    QStringList keys = Undo ? undoStack->undoText().split("|") : undoStack->redoText().split("|");
    Rc buildModAction = keys.first() == QLatin1String("BuildModApply") ? BuildModApplyRc :
                        keys.first() == QLatin1String("BuildModRemove") ? BuildModRemoveRc :
                        BuildModNoActionRc;
    if (buildModAction != BuildModNoActionRc) {
        int buildModStepIndex = getBuildModStepIndex(lpub->currentStep->topOfStep());
        setBuildModificationKey(buildModAction);
        if (!buildModificationKey.isEmpty()) {
            Rc undoRedoBuildModAction = buildModAction == BuildModRemoveRc ? BuildModApplyRc : BuildModRemoveRc;
            setBuildModAction(buildModificationKey, buildModStepIndex, undoRedoBuildModAction);
            buildModificationKey.clear();
        }
    }
}

bool Gui::setBuildModificationKey(Rc key)
{
    Step *currentStep = lpub->currentStep;

    if (!currentStep)
        return false;

    using namespace Options;
    Mt it = static_cast<Mt>(lcGetActiveProject()->GetImageType());
    if (it == CSI) {

        Rc action = key;

        buildModificationKey.clear();

        if (key == OkRc)
            action = BuildModBeginRc;

        Rc rc;

        Meta meta;

        Where walk = currentStep->topOfStep();

        QString line = readLine(walk);
        rc =  meta.parse(line,walk,false);
        if (rc == StepRc || rc == RotStepRc)
            walk++;   // Advance past STEP meta

        // Parse the step lines
        for ( ;
              walk.lineNumber < subFileSize(walk.modelName);
              walk.lineNumber++) {
            line = readLine(walk);
            Where here(walk.modelName,walk.lineNumber);
            rc =  meta.parse(line,here,false);

            switch (rc) {
            case BuildModBeginRc:
                if (action == BuildModBeginRc) {
                    buildModificationKey = meta.LPub.buildMod.key();
                    return true;
                }
                break;

            case BuildModApplyRc:
                if (key == BuildModApplyRc) {
                    buildModificationKey = meta.LPub.buildMod.key();
                    return true;
                }
                break;

            case BuildModRemoveRc:
                if (action == BuildModRemoveRc) {
                    buildModificationKey = meta.LPub.buildMod.key();
                    return true;
                }
                break;

            // Search until next step/rotstep meta
            case RotStepRc:
            case StepRc:
            default:
                return false;;
            }
        }
    }

    return false;
}

void Gui::updateBuildModification()
{
    Step *currentStep = lpub->currentStep;

    if (!currentStep)
        return;

    buildModificationKey = currentStep->buildModMeta.value().buildModKey;

    if (buildModificationKey.isEmpty())
        setBuildModificationKey();

    if (!buildModificationKey.isEmpty())
        createBuildModification();
}

void Gui::deleteBuildModification()
{
    Step *currentStep = lpub->currentStep;

    if (!currentStep)
        return;

//* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
     LDrawFile *ldrawFile = &lpub->ldrawFile;
     Q_UNUSED(ldrawFile)
#endif
//*/

    if (!buildModsCount())
        return;

    QString buildModKey = currentStep->buildModMeta.value().buildModKey;

    if (buildModKey.isEmpty()) {
        setBuildModificationKey(BuildModBeginRc);
        if (buildModificationKey.isEmpty()) {
            QStringList buildModKeys;
            if (buildModsCount() == 1) {
                buildModKeys = getBuildModsList();
            } else {
                BuildModDialogGui *buildModDialogGui = new BuildModDialogGui();
                buildModDialogGui->getBuildMod(buildModKeys, BM_DELETE);
            }
            if (buildModKeys.size())
                buildModKey = buildModKeys.first();
        } else {
            buildModKey = buildModificationKey;
            buildModificationKey.clear();
        }
    }

    if (buildModKey.isEmpty())
        return;

    emit gui->messageSig(LOG_INFO_STATUS, tr("Processing build modification 'Delete' action..."));

    const QString step(QString::number(currentStep->stepNumber.number));
    /*
    const QString model(currentStep->topOfStep().modelName);
    const QString line(currentStep->topOfStep().lineNumber);

    QString text  = tr("This action will permanently delete build modification '%1' "
                       "from '%3' at step (%2)' and cannot be completelly undone using the Undo menu action.<br><br>"
                       "The modified CSI image and Visual Editor entry will be parmanently deleted.<br>"
                       "However, you can use 'Reload' menu action to restore all deleted content.<br><br>"
                       "Do you want to continue ?<br>")
                       .arg(buildModKey).arg(step).arg(model);
    QString type  = tr("delete build modification");
    QString title = tr("Build Modification");

    Preferences::MsgID msgID(Preferences::BuildModEditErrors, Where(model,line).nameToString());
    switch (Preferences::showMessage(msgID, text, title, type, true / *option* /))
    {
    default:
    case QMessageBox::Cancel:
        return;
    case QMessageBox::Ok:
        break;
    }
    */

    const QString buildModStepKey(currentStep->viewerStepKey);
    const bool multiStepPage(currentStep->multiStep);
    const int pageNum = getBuildModDisplayPageNumber(buildModKey);

    // Delete options
    QMessageBoxResizable box;
    box.setWindowIcon(QIcon());
    box.setIconPixmap (QPixmap(LPUB3D_MESSAGE_ICON));
    box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    const QString title = tr("Build Modification Image");
    const QString text = tr("Click 'Submodel',%1 or 'Step' to reset the respective image cache.").arg(multiStepPage? tr(" 'Page',") : "");

    box.setWindowTitle(QString("%1 %2").arg(VER_PRODUCTNAME_STR, title));
    box.setText (tr("Delete build modification <b>%1</b><br><br>"
                    "Select your option to reset the image cache.").arg(buildModKey));
    box.setInformativeText(text);

    QPushButton *clearSubmodelButton = box.addButton(tr("Modified"), QMessageBox::AcceptRole);
    clearSubmodelButton->setToolTip(tr("Reset modified assembly images starting from step %1").arg(step));

    QPushButton *clearPageButton = box.addButton(tr("Page %1").arg(pageNum), QMessageBox::AcceptRole);
    clearPageButton->setToolTip(tr("Reset page %1 step group images").arg(pageNum));
    clearPageButton->setVisible(multiStepPage);

    QPushButton *clearStepButton = box.addButton(tr("Step %1").arg(step), QMessageBox::AcceptRole);
    clearStepButton->setToolTip(tr("Reset step %1 image only").arg(step));

    QPushButton *cancelButton = box.addButton(QMessageBox::Cancel);
    cancelButton->setDefault(true);

    box.exec();

    const bool clearSubmodel = box.clickedButton() == clearSubmodelButton;
    const bool clearPage     = box.clickedButton() == clearPageButton;
    const bool clearStep     = box.clickedButton() == clearStepButton;

    if (box.clickedButton() == cancelButton)
        return;

    const QString clearOption = clearSubmodel ?
                QLatin1String("_cm") : clearPage ?
                QLatin1String("_cp") : clearStep ?
                QLatin1String("_cs") : QString();

    using namespace Options;
    Mt it = static_cast<Mt>(lcGetActiveProject()->GetImageType());
    if (it == CSI) {
        const int modBeginLineNum  = getBuildModBeginLineNumber(buildModKey);
        const int modActionLineNum = getBuildModActionLineNumber(buildModKey);
        const int modEndLineNum    = getBuildModEndLineNumber(buildModKey);
        QString modelName          = getBuildModStepKeyModelName(buildModKey);

        if (modelName.isEmpty() || !modBeginLineNum || !modActionLineNum || !modEndLineNum) {
            emit gui->messageSig(LOG_ERROR, tr("There was a problem receiving build modification attributes for key [%1]<br>"
                                          "Delete build modification cannot continue.").arg(buildModKey));
            return;
        }

        beginMacro(QLatin1String("BuildModDelete|") + buildModStepKey + clearOption);

        // delete existing APPLY/REMOVE (action) commands, starting from the bottom of the step
        Rc rc;
        Meta meta;
        QString modKey, modLine;
        Where here, topOfStep, bottomOfStep;
        QMap<int/*stepIndex*/, int/*ActionRc*/> actionsMap = getBuildModActions(buildModKey);
        QList<int> stepIndexes = actionsMap.keys();
        std::sort(stepIndexes.begin(), stepIndexes.end(), std::greater<int>()); // sort stepIndexes descending
        for (int &stepIndex : stepIndexes) {
            QMap<int, int>::iterator i = actionsMap.find(stepIndex);
            if (i.key()) {                                                      // skip first step at index 0 - deleted later
                if (getBuildModStepIndexWhere(i.key(), topOfStep)) {            // initialize topOfStep Where
                    if (! getBuildModStepIndexWhere(i.key() + 1, bottomOfStep)) // bottom of step is top of next step
                        // handle last step
                        bottomOfStep = Where(topOfStep.modelName, subFileSize(topOfStep.modelName));
                    for (Where walk = bottomOfStep; walk > topOfStep.lineNumber; --walk) {
                        here = walk;
                        modLine = readLine(here);
                        rc = meta.parse(modLine, here);
                        switch (rc) {
                        case BuildModApplyRc:
                        case BuildModRemoveRc:
                            modKey = meta.LPub.buildMod.key().toLower();
                            if (modKey == buildModKey.toLower())
                                deleteLine(here);
                            break;
                        default:
                            break;
                        }
                    }
                }
            }
        }

        // delete existing BUILD_MOD commands from bottom up, starting at END
        here = Where(modelName, modEndLineNum);
        modLine = readLine(here);
        rc = meta.parse(modLine, here);
        if (rc == BuildModEndRc)
            deleteLine(here);

        // delete existing BUILD_MOD commands from bottom up, starting at END_MOD
        here = Where(modelName, modActionLineNum);
        modLine = readLine(here);
        rc = meta.parse(modLine, here);
        if (rc == BuildModEndModRc)
            for (Where walk = here; walk >= modBeginLineNum; --walk)
                deleteLine(walk);

        // delete step image to trigger image regen
        if (clearSubmodel) {
            clearWorkingFiles(getPathsFromViewerStepKey(buildModStepKey));
        } else if (clearPage) {
            PlacementType relativeType = multiStepPage ? StepGroupType : SingleStepType;
            clearPageCache(relativeType, &lpub->page, CSI);
        } else if (clearStep) {
            QString csiPngName = getViewerStepImagePath(buildModStepKey);
            clearStepCSICache(csiPngName);
            // delete viewer step to trigger viewer update
            if (!deleteViewerStep(buildModStepKey))
                emit messageSig(LOG_ERROR,tr("Failed to delete viewer step entry for key %1.").arg(buildModStepKey));
        }

        // delete BuildMod
        if (!deleteBuildMod(buildModKey))
            emit messageSig(LOG_ERROR,tr("Failed to delete build modification '%1'.").arg(buildModKey));

        if (Gui::abortProcess()) {
            showLine(currentStep->topOfStep());
            return;
        }

        endMacro();

        emit gui->messageSig(LOG_INFO_STATUS, tr("Build modification '%1' deleted at step %2.")
                                                 .arg(buildModKey).arg(lpub->currentStep->stepNumber.number));
    }
}

/*********************************************
 *
 * save viewer model if modified
 *
 ********************************************/
QMutex saveBuildModMutex;

bool Gui::saveBuildModification()
{
    QMutexLocker saveBuildModLocker(&saveBuildModMutex);

    Step *currentStep = lpub->currentStep;

    if (!currentStep || !Preferences::buildModEnabled)
        return true;     // continue

    using namespace Options;
    if (static_cast<Mt>(lcGetActiveProject()->GetImageType()) != CSI)
        return true;     // continue

    if (!VisualEditChangeTriggers.contains(visualEditUndoRedoText))
        return true;     // continue

    clearVisualEditUndoRedoText();

    QMessageBox box;
    box.setWindowIcon(QIcon());
    box.setIconPixmap (QPixmap(LPUB3D_MESSAGE_ICON));
    box.setTextFormat (Qt::RichText);
    box.setWindowTitle(tr ("%1 Save Model Change").arg(VER_PRODUCTNAME_STR));
    box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    QString title = setBuildModificationKey()
             ? tr("Save build modification <b>'%1'</b> model changes to step %2 ?")
                  .arg(getBuildModificationKey()).arg(currentStep->stepNumber.number)
             : tr("Save model changes as build modification to step %1 ?")
                  .arg(currentStep->stepNumber.number);
    box.setText (title);
    box.setStandardButtons (QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    box.setDefaultButton   (QMessageBox::Cancel);
    switch (box.exec())
    {
    default:
    case QMessageBox::Cancel:
        return false; // cancel request

    case QMessageBox::Yes:
        updateBuildModification();
        break;       // apply changes

    case QMessageBox::No:
        break;       // discard changes and continue
    }

    return true;
}

/*********************************************
 *
 * clear the buildMod Range for the current step
 *
 ********************************************/

void Gui::clearBuildModRange()
{
    mBuildModRange = { 0/*BM_BEGIN_LINE_NUM*/, 0/*BM_ACTION_LINE_NUM*/, -1/*BM_MODEL_INDEX*/ };
}

/*********************************************
 *
 * clear the visual editor undo/redo text
 *
 ********************************************/

void Gui::clearVisualEditUndoRedoText()
{
    visualEditUndoRedoText.clear();
    if (Preferences::buildModEnabled) {
        CreateBuildModAct->setEnabled(false);
        UpdateBuildModAct->setEnabled(false);
    }
}

/*********************************************
 *
 * set step from specified line
 *
 ********************************************/

void Gui::setStepForLine()
{
    if (!lpub->currentStep || !gMainWindow || !gMainWindow->isVisible() || Gui::exporting())
        return;

    if (lpub->page.coverPage)
        return;

    disconnect(this,           SIGNAL(highlightSelectedLinesSig(const QVector<int>&,bool)),
               editWindow,       SLOT(   highlightSelectedLines(const QVector<int>&,bool)));

    enableVisualBuildModActions();
    lpub->currentStep->viewerOptions->ZoomExtents = true;
    lpub->currentStep->loadTheViewer();

    connect(this,           SIGNAL(highlightSelectedLinesSig(const QVector<int>&,bool)),
            editWindow,       SLOT(   highlightSelectedLines(const QVector<int>&,bool)));
}

/*********************************************
 *
 * slelcted Line
 *
 ********************************************/

bool Gui::getSelectedLine(int modelIndex, int lineIndex, int source, int &lineNumber)
{

    lineNumber        = EDITOR_LINE   ;             // 0
    bool currentModel = modelIndex == QString(lpub->viewerStepKey[0]).toInt();
    bool newLine      = lineIndex  == NEW_PART;     //-1
    bool fromViewer   = source      > EDITOR_LINE;  // 0
/* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
    emit messageSig(LOG_DEBUG, QString("currentModel: %1 [modelIndex: %2 == ViewerStepKey(index,line,step)[0]: %3]")
                    .arg(currentModel ? "True" : "False")
                    .arg(modelIndex)
                    .arg(lpub->viewerStepKey));
    emit messageSig(LOG_DEBUG, QString("newLine: %1 [lineIndex: %2 == NEW_PART: -1]")
                    .arg(newLine ? "True" : "False")
                    .arg(lineIndex));
    emit messageSig(LOG_DEBUG, QString("fromViewer: %1 [source: %2 > EDITOR_LINE: 0]")
                    .arg(fromViewer ? "True" : "False")
                    .arg(source));
#endif
//*/
    if (newLine) {
        emit gui->messageSig(LOG_TRACE, tr("New viewer part modelName [%1]")
                                           .arg(gui->getSubmodelName(modelIndex,false)));
        return false;

    } else if (currentModel) {

        Step *currentStep = lpub->currentStep;

        if (!currentStep)
            return false;
//* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_TRACE, tr("%1 Step lineIndex count: %2 item(s)")
                                           .arg(VER_PRODUCTNAME_STR)
                                           .arg(currentStep->lineTypeIndexes.size()));
//      for (int i = 0; i < currentStep->lineTypeIndexes.size(); ++i)
//          emit messageSig(LOG_TRACE, tr(" -%1 Part lineNumber [%2] at step line lineIndex [%3] - specified lineIndex [%4]")
//                                        .arg(.arg(VER_PRODUCTNAME_STR)).arg(currentStep->lineTypeIndexes.at(i)).arg(i).arg(lineIndex));
#endif
//*/

        if (fromViewer)      // input relativeIndes
            lineNumber = currentStep->getLineTypeRelativeIndex(lineIndex);
        else                 // input lineTypeIndex
            lineNumber = currentStep->getLineTypeIndex(lineIndex);

    } else if (modelIndex != NEW_MODEL) {

        if (fromViewer)      // input relativeIndes
            lineNumber = getLineTypeRelativeIndex(modelIndex,lineIndex); // return lineTypeIndex - part lineNumber
        else                 // input lineTypeIndex
            lineNumber = getLineTypeIndex(modelIndex,lineIndex);         // return relativeIndex - step line lineIndex
    } else
        return false;        // lineNumber = EDITOR_LINE;
/* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
    emit messageSig(LOG_DEBUG, tr("selected lineNumber: [%1] > NOT_FOUND: [-1]")
                                  .arg(lineNumber));
#endif
//*/
    return lineNumber > NOT_FOUND; // -1
}

/*********************************************
 *
 * Selected part index(es)
 *
 ********************************************/

void Gui::SelectedPartLines(const QVector<TypeLine> &indexes, const PartSource source)
{
    if (! Gui::exporting() && Preferences::modeGUI) {
        Step *currentStep = lpub->currentStep;

        if (!currentStep || (source == EDITOR_LINE && !indexes.size()))
            return;

        using namespace Options;
        if (static_cast<Mt>(lcGetActiveProject()->GetImageType()) != CSI)
            return;

        QVector<int> lines;
        bool validTrigger = source < VIEWER_CLR && source > EDITOR_CLR;
        bool validLine    = false;
        bool modsEnabled  = false;
        bool fromViewer   = source > EDITOR_LINE;
        int lineNumber    = 0;
        int lineIndex     = NEW_PART;
        int modelIndex    = NEW_MODEL;
        QString modelName = QLatin1String("undefined");
//* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
        const QString SourceNames[] =
        {
            QLatin1String("EDITOR_CLR (-2)"), // -2
            QLatin1String("NOT_FOUND (-1)"),  // -1 OUT_OF_BOUNDS (-1), NEW_PART (-1), NEW_MODEL (-1)
            QLatin1String("EDITOR_LINE (0)"), //  0 VIEWER_NONE
            QLatin1String("VIEWER_LINE (1)"), //  1
            QLatin1String("VIEWER_MOD (2)"),  //  2
            QLatin1String("VIEWER_DEL (3)"),  //  3
            QLatin1String("VIEWER_SEL (4)"),  //  4
            QLatin1String("VIEWER_CLR (5)")   //  5
        };
        QString fromSource;
        if (!fromViewer && source == EDITOR_LINE)
            fromSource = QLatin1String("EDITOR_LINE (0)");
        else
            fromSource = SourceNames[source+2];
        emit gui->messageSig(LOG_DEBUG, QString("Selection Source: %1 (%2)")
                                                .arg(fromSource, QString::number(source)));
#endif
//*/
        if (indexes.size()) {
            modelName  = getSubmodelName(indexes.at(0).modelIndex,false);
            modelIndex = indexes.at(0).modelIndex;
        } else if (!lpub->viewerStepKey.isEmpty()) {
            modelName  = currentStep->topOfStep().modelName;
            modelIndex = getSubmodelIndex(modelName);
        }
//* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
        if (modelIndex != NEW_MODEL && source > VIEWER_LINE)
            emit gui->messageSig(LOG_TRACE, tr("Submodel lineIndex count: %1 item(s)")
                                               .arg(lpub->ldrawFile.getLineTypeRelativeIndexCount(modelIndex)));
#endif
//*/
        for (int i = 0; i < indexes.size() && validTrigger; ++i) {
            lineIndex = indexes.at(i).lineIndex;
            // New part lines are added in createBuildModification() routine
            if (lineIndex != NEW_PART) {
                validLine = getSelectedLine(modelIndex, lineIndex, source, lineNumber);
                if (lineNumber != OUT_OF_BOUNDS)
                    lines.append(lineNumber);
            }

            if (validLine) {
                bool buildModChange = source == VIEWER_MOD || source == VIEWER_DEL;
                if (fromViewer && buildModChange && Preferences::buildModEnabled) {
                    if (mBuildModRange.first()) {
                        if (lineNumber < mBuildModRange.first())
                            mBuildModRange[BM_BEGIN_LINE_NUM] = lineNumber;
                        else if (lineNumber > mBuildModRange.last())
                            mBuildModRange[BM_ACTION_LINE_NUM] = lineNumber;
                        mBuildModRange[BM_MODEL_INDEX] = modelIndex;
                    } else {
                        mBuildModRange = { lineNumber, lineNumber, modelIndex };
                    }
                    if (!modsEnabled) {
                        enableVisualBuildModEditAction();
                    }
                }
            }
//* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
            QString Message;
            if (fromViewer) {
                if (lineIndex == NEW_PART) {
                    fromSource = QLatin1String("VIEWER_NEW_PART (-1)");
                    Message = tr("New %1 part specified at step %2, modelName: [%3]")
                                 .arg(fromSource)
                                 .arg(currentStep->stepNumber.number)
                                 .arg(modelName);
                } else if (validLine) {
                    Message = tr("Selected %1 part modelName [%2] lineNumber: [%3] at step line index [%4]")
                                 .arg(fromSource, modelName).arg(lineNumber).arg(lineIndex < 0 ? "undefined" : QString::number(lineIndex));
                } else {
                    fromSource = indexes.size() ? QLatin1String("VIEWER_OUT_OF_BOUNDS (-1)") : QLatin1String("VIEWER_INVALID (-1)");
                    Message = tr("%1 part lineNumber [%2] for step line index [%3]")
                                 .arg(fromSource)
                                 .arg(lineNumber).arg(lineIndex < 0 ? "undefined" : QString::number(lineNumber));
                }
            } else if (validLine) { // valid and not from viewer
                Message = tr("Selected %1 part modelName [%2] lineNumber: [%3] at step line index [%4]")
                             .arg(fromSource, modelName).arg(lineIndex).arg(lineNumber < 0 ? "undefined" : QString::number(lineIndex));
            } else {                // invalid and not from viewer
                fromSource = indexes.size() ? QLatin1String("OUT_OF_BOUNDS (-1)") : QLatin1String("INVALID (-1)");
                Message = tr("%1 part lineNumber [%2] for step line index [%3]") // index and number flipped
                             .arg(fromSource)
                             .arg(lineIndex).arg(lineNumber < 0 ? "undefined" : QString::number(lineNumber));
            }
            emit gui->messageSig(LOG_TRACE, Message);
#endif
//*/
        } // indexes present and source is not VIEWER_CLR

        if (fromViewer) {
            if (editWindow->isVisible() && (source == VIEWER_SEL || source == VIEWER_MOD || source == VIEWER_CLR)) {
                emit highlightSelectedLinesSig(lines, source == VIEWER_CLR);
            }
            // delete action with no selected lines
            if (source == VIEWER_DEL && Preferences::buildModEnabled) {
                if (!modsEnabled) {
                    enableVisualBuildModEditAction();
                }
//* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
                emit gui->messageSig(LOG_TRACE, tr("Delete %1 part(s) specified at step %2, modelName: [%3]")
                                                   .arg(fromSource)
                                                   .arg(currentStep->stepNumber.number)
                                                   .arg(modelName));
#endif
//*/
            }
        } else { // indexes from editor
            if (lines.size() && gMainWindow->isVisible())
                emit setSelectedPiecesSig(lines);
        }
    } // not exporting
}

void Gui::importLDD()
{
    if (!maybeSave() || !saveBuildModification())
        return;

    QString LoadFileName = QFileDialog::getOpenFileName(this, tr("Import"), QString(), tr("LEGO Diginal Designer Files (*.lxf);;All Files (*.*)"));
    if (LoadFileName.isEmpty())
        return;

    Project* Importer = new Project();

    if (Importer->ImportLDD(LoadFileName))
    {
        QString FileName = QFileDialog::getSaveFileName(this,tr("Save Diginal Designer File As"),
                                                        tr("%1.%2.mpd").arg(QFileInfo(LoadFileName).completeBaseName()),
                                                        tr("LDraw Files (*.mpd *.ldr *.dat);;All Files (*.*)"));
        if (FileName.isEmpty()) {
            QTemporaryDir tempDir;
            if (tempDir.isValid())
                FileName =  QString("%1/%2.%3.mpd").arg(tempDir.path(),QFileInfo(LoadFileName).completeBaseName());
        }
        saveImport(FileName, Importer);
    }
    else
        delete Importer;
}

void Gui::importInventory()
{
    if (!maybeSave() || !saveBuildModification())
        return;

    lcSetsDatabaseDialog Dialog(this);
    if (Dialog.exec() != QDialog::Accepted)
        return;

    Project* Importer = new Project();

    if (Importer->ImportInventory(Dialog.GetSetInventory(), Dialog.GetSetName(), Dialog.GetSetDescription()))
    {
        QString FileName = QFileDialog::getSaveFileName(this,tr("Save Inventory File As"),
                                                        tr("%1.%2.mpd").arg(Dialog.GetSetName()),
                                                        tr("LDraw Files (*.mpd *.ldr *.dat);;All Files (*.*)"));
        if (FileName.isEmpty()) {
            QTemporaryDir tempDir;
            if (tempDir.isValid())
                FileName =  QString("%1/%2.%3.mpd").arg(tempDir.path(),Dialog.GetSetName());
        }
        saveImport(FileName, Importer);
    }
    else
        delete Importer;
}

bool Gui::saveImport(const QString& FileName, Project *Importer)
{
    QFile File(FileName);

    if (!File.open(QIODevice::WriteOnly))
    {
        emit gui->messageSig(LOG_ERROR, tr("Error writing to file '%1':\n%2").arg(FileName, File.errorString()));
        return false;
    }

    QTextStream Stream(&File);
    bool firstStep = true;
    QLatin1String LineEnding("\r\n");
    QLatin1String Extension(".ldr");
    for (const std::unique_ptr<lcModel>& Model : Importer->GetModels())
    {
        if (firstStep) {
            firstStep = false;
            Stream << QLatin1String("0 FILE Import-") << Model->GetProperties().mFileName << Extension << LineEnding;
            Stream << QLatin1String("0 ") << Model->GetProperties().mDescription << LineEnding;
            Stream << QLatin1String("0 Name: Import-") << Model->GetProperties().mModelName << LineEnding;
            Stream << QLatin1String("0 Author: ") << Model->GetProperties().mAuthor << LineEnding;
            Stream << QLatin1String("0 !LPUB INSERT TEXT \"This page is intentionally left blank\\nto avoid loading the imported parts at the first step.\" \"Arial,36,-1,255,75,0,0,0,0,0\" \"Black\" OFFSET 0.0219 0.4814\r\n");
            Stream << QLatin1String("0 !LPUB INSERT PAGE\r\n");
            Stream << QLatin1String("0 STEP\r\n");
            Stream << QLatin1String("1 16 0 0 0 1 0 0 0 1 0 0 0 1 ") << Model->GetProperties().mFileName << Extension << LineEnding;
            Stream << QLatin1String("0 NOFILE\r\n");
        }
        Stream << QLatin1String("0 FILE ") << Model->GetProperties().mFileName << Extension << LineEnding;
        Model->SaveLDraw(Stream, false, Model->GetCurrentStep());
        Model->SetSaved();
        Stream << QLatin1String("0 NOFILE\r\n");
    }
    File.close();

    int saveLoadMessageFlag = Preferences::ldrawFilesLoadMsgs;
    Preferences::ldrawFilesLoadMsgs = SHOW_WARNING;
    if (!loadFile(FileName)) {
        Preferences::ldrawFilesLoadMsgs = saveLoadMessageFlag;
        return false;
    }
    Preferences::ldrawFilesLoadMsgs = saveLoadMessageFlag;

    return true;
}

void Gui::resetViewerImage(bool zoomExtents)
{
    Step *currentStep = lpub->currentStep;

    if (!currentStep || !Preferences::modeGUI || Gui::exporting())
        return;

    gMainWindow->mActions[LC_EDIT_ACTION_RESET_TRANSFORM]->setEnabled(false);

    using namespace Options;
    switch (lcGetActiveProject()->GetImageType())
    {
    case static_cast<int>(CSI):
        lpub->saveVisualEditorTransformSettings();
        enableVisualBuildModActions();
        currentStep->viewerOptions->IsReset     = true;
        currentStep->viewerOptions->ZoomExtents = zoomExtents;
        currentStep->loadTheViewer();
        break;

    case static_cast<int>(SMI):
        if (currentStep->placeSubModel) {
            lcPreferences& Preferences = lcGetPreferences();
            SubModel& subModel = currentStep->subModel;
            if (!Preferences.mPreviewEnabled) {
                if (gui->saveBuildModification()) {
                    lpub->saveVisualEditorTransformSettings();
                    subModel.viewerOptions->ZoomExtents = zoomExtents;
                    subModel.viewerOptions->IsReset     = true;
                }
            }
            subModel.loadTheViewer();
        }
        break;

    default:
        break;
    }
}

/*********************************************
 *
 * Default camera settings
 *
 *********************************************/

float MetaDefaults::getCameraFOV()
{
    return (Preferences::preferredRenderer == RENDERER_NATIVE ?
            gApplication ? lcGetPreferences().mCFoV : CAMERA_FOV_NATIVE_DEFAULT :
            Preferences::preferredRenderer == RENDERER_LDVIEW && Preferences::perspectiveProjection ?
            CAMERA_FOV_LDVIEW_P_DEFAULT :
            CAMERA_FOV_DEFAULT);
}

float MetaDefaults::getFOVMinRange()
{
    return (Preferences::preferredRenderer == RENDERER_NATIVE ?
            CAMERA_FOV_NATIVE_MIN_DEFAULT :
            CAMERA_FOV_MIN_DEFAULT);
}

float MetaDefaults::getFOVMaxRange()
{
    return (Preferences::preferredRenderer == RENDERER_NATIVE ?
            CAMERA_FOV_NATIVE_MAX_DEFAULT :
            Preferences::preferredRenderer == RENDERER_LDVIEW && Preferences::perspectiveProjection ?
            CAMERA_FOV_LDVIEW_P_MAX_DEFAULT :
            CAMERA_FOV_MAX_DEFAULT);
}

float MetaDefaults::getNativeCameraZNear()
{
    return gApplication ? lcGetPreferences().mCNear : CAMERA_ZNEAR_NATIVE_DEFAULT;
}

float MetaDefaults::getNativeCameraZFar()
{
    return gApplication ? lcGetPreferences().mCFar : CAMERA_ZFAR_NATIVE_DEFAULT;
}

float MetaDefaults::getCameraDDF()
{
    return gApplication ? lcGetPreferences().mDDF : CAMERA_DISTANCE_FACTOR_NATIVE_DEFAULT;
}

QString MetaDefaults::getPreferredRenderer(int which)
{
    int preferredRenderer = which < 0
        ? Preferences::preferredRenderer
        : which;
    const QString rendererNames[NUM_RENDERERS] =
    {
        QLatin1String("Native"),  // RENDERER_NATIVE
        QLatin1String("LDView"),  // RENDERER_LDVIEW
        QLatin1String("LDGLite"), // RENDERER_LDGLITE
        QLatin1String("POVRay"),  // RENDERER_POVRAY
        QLatin1String("Blender")  // RENDERER_BLENDER
    };
    return rendererNames[preferredRenderer];
}

QString MetaDefaults::getPreferredUnits()
{
    return Preferences::preferCentimeters ? QObject::tr("dots per centimetre (DPCM) unit") :
                                            QObject::tr("dots per inch (DPI) unit");
}

float MetaDefaults::getAssemblyCameraLatitude()
{
    return (float)Preferences::assemblyCameraLatitude;
}

float MetaDefaults::getAssemblyCameraLongitude()
{
    return (float)Preferences::assemblyCameraLongitude;
}

float MetaDefaults::getPartCameraLatitude()
{
    return (float)Preferences::partCameraLatitude;
}

float MetaDefaults::getPartCameraLongitude()
{
    return (float)Preferences::partCameraLongitude;
}

float MetaDefaults::getSubmodelCameraLatitude()
{
    return (float)Preferences::submodelCameraLatitude;
}

float MetaDefaults::getSubmodelCameraLongitude()
{
    return (float)Preferences::submodelCameraLongitude;
}

QString MetaDefaults::getDefaultCSIKeys()
{
    QString const keys = QString("0_1280_%1_%2_1_%3_%4_%5_0_0_0_0_0_0_REL")
                                 .arg(double(resolution()))
                                 .arg(resolutionType() == DPI ? "DPI" : "DPCM")
                                 .arg(double(getCameraFOV()))
                                 .arg(double(getAssemblyCameraLatitude()))
                                 .arg(double(getAssemblyCameraLongitude()));
    return keys;
}
