/****************************************************************************
**
** Copyright (C) 2019 - 2022 Trevor SANDY. All rights reserved.
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
 * This class provides the options object passed between LPub3D, the Native
 * renderer and the Visual Editor.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef OPTIONS_H
#define OPTIONS_H

#include <QString>
#include <QStringList>

namespace Options
{
  enum Mt { PLI, CSI ,SMP ,MON    };
}

class Vector3
{

private:
  bool _populated;

public:
  Vector3()
  {
    _populated  = false;
  }
  Vector3(const float _x, const float _y, const float _z)
      : x(_x), y(_y), z(_z)
  {
    _populated  = x != 0.0f || y != 0.0f || z != 0.0f;
  }
  bool isPopulated() const
  {
    return _populated;
  }

  friend bool operator==(const Vector3& a, const Vector3& b);
  friend bool operator!=(const Vector3& a, const Vector3& b);

  float x, y, z;
};

inline bool operator==(const Vector3 &a, const Vector3 &b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

inline bool operator!=(const Vector3 &a, const Vector3 &b)
{
    return a.x != b.x || a.y != b.y || a.z != b.z;
}

class NativeOptions
{

public:
  NativeOptions()
      :ImageType         (Options::CSI),
       ImageWidth        (800 ),
       ImageHeight       (600),
       PageWidth         (800),
       PageHeight        (600),
       StudStyle         (0 /*Plain*/),
       IniFlag           (-1 /*NONE*/),
       ExportMode        (-1 /*EXPORT_NONE*/),
       LineWidth         (1.0f),
       Resolution        (150.0f),
       ModelScale        (1.0f),
       CameraDistance    (0.0f),
       FoV               (0.0f),
       ZNear             (0.0f),
       ZFar              (0.0f),
       Latitude          (0.0f),
       Longitude         (0.0f),
       EdgeContrast      (0.0f),
       EdgeSaturation    (0.0f),
       LightDarkIndex    (0.0f),
       IsOrtho           (false),
       ZoomExtents       (false),
       AutoEdgeColor     (false),
       HighlightNewParts (false),
       TransBackground   (true),
       StudCylinderColor (0),
       PartEdgeColor     (0),
       BlackEdgeColor    (0),
       DarkEdgeColor     (0),
       RotStep           (Vector3(0, 0, 0)),
       Target            (Vector3(0, 0, 0)),
       Position          (Vector3(0, 0, 0)),
       UpVector          (Vector3(0, 0, 0))
  { }
//  virtual ~NativeOptions(){}
  QStringList ExportArgs;   // New
  QString ViewerStepKey;
  QString ImageFileName;
  QString RotStepType;
  QString CameraName;
  QString InputFileName;    // New
  QString OutputFileName;   // New
  QString ExportFileName;   // New
  Options::Mt ImageType;
  int ImageWidth;
  int ImageHeight;
  int PageWidth;
  int PageHeight;
  int StudStyle;
  int IniFlag;              // New
  int ExportMode;           // New
  float LineWidth;          // New
  float Resolution;
  float ModelScale;
  float CameraDistance;
  float FoV;
  float ZNear;
  float ZFar;
  float Latitude;
  float Longitude;
  float EdgeContrast;
  float EdgeSaturation;
  float LightDarkIndex;
  bool IsOrtho;
  bool ZoomExtents;
  bool AutoEdgeColor;
  bool HighlightNewParts;    // New
  bool TransBackground;      // New
  quint32 StudCylinderColor;
  quint32 PartEdgeColor;
  quint32 BlackEdgeColor;
  quint32 DarkEdgeColor;
  Vector3 RotStep;
  Vector3 Target;
  Vector3 Position;
  Vector3 UpVector;
};

// Page Options Routines

class Meta;
class Where;
class PgSizeData;
class PliPartGroupMeta;
class SubmodelStack;

class BuildModFlags
{

public:
    QString key;
    int  level;
    int  action;
    int  state;
    bool ignore;
    BuildModFlags()
        : level(0),
          action(0),
          state(-1),
          ignore(false)
    { }
    void clear()
    {
        level = 0;
        action = 0;
        state = -1;
        ignore = false;
    }
};

class FindPageFlags
{

public:
    bool stepGroup;
    bool partIgnore;
    bool coverPage;
    bool stepPage;
    bool bfxStore1;
    bool bfxStore2;
    bool parseNoStep;
    bool callout;
    bool noStep;
    bool noStep2;
    bool stepGroupBfxStore2;
    bool pageSizeUpdate;
    bool addCountPage;
    bool parseBuildMods;
    bool parseStepGroupBM;
    bool countPageContains;
    bool resetIncludeRc;
    bool includeFileFound;
    int  includeFileRc;
    int  countInstances;
    int  includeLineNum;
    int  partsAdded;
    int  numLines;

    BuildModFlags buildMod;
    QList<BuildModFlags> buildModStack;

    FindPageFlags()
        :
          stepGroup         (false),
          partIgnore        (false),
          coverPage         (false),
          stepPage          (false),
          bfxStore1         (false),
          bfxStore2         (false),
          parseNoStep       (false),
          callout           (false),
          noStep            (false),
          noStep2           (false),
          stepGroupBfxStore2(false),
          pageSizeUpdate    (false),
          addCountPage      (false),
          parseBuildMods    (false),
          parseStepGroupBM  (false),
          countPageContains (false),
          resetIncludeRc    (false),
          includeFileFound  (false),
          includeFileRc     (-1),    // EndOfIncludeFileRc
          countInstances    (2),     // CountAtModel
          includeLineNum    (0),
          partsAdded        (0),
          numLines          (0)
    { }
    ~FindPageFlags()
    {
        buildModStack.clear();
    }
};

class FindPageOptions
{

public:
    FindPageOptions(
            int             &_pageNum,
            Where           &_current,
            PgSizeData      &_pageSize,
            FindPageFlags   &_flags,
            QList<SubmodelStack> &_modelStack,

            bool             _pageDisplayed,
            bool             _updateViewer,
            bool             _isMirrored,
            bool             _printing,
            int              _stepNumber,
            int              _contStepNumber,
            int              _groupStepNumber = 0,
            QString          _renderParentModel = "")
        :
          pageNum           (_pageNum),
          current           (_current),
          pageSize          (_pageSize),
          flags             (_flags),
          modelStack        (_modelStack),

          pageDisplayed     (_pageDisplayed),
          updateViewer      (_updateViewer),
          isMirrored        (_isMirrored),
          printing          (_printing),
          stepNumber        (_stepNumber),
          contStepNumber    (_contStepNumber),
          groupStepNumber   (_groupStepNumber),
          renderParentModel (_renderParentModel)
    {  }
    int            &pageNum;
    Where          &current;
    PgSizeData     &pageSize;
    FindPageFlags  &flags;
    QList<SubmodelStack> &modelStack;
    bool            pageDisplayed;
    bool            updateViewer;
    bool            isMirrored;
    bool            printing;
    int             stepNumber;
    int             contStepNumber;
    int             groupStepNumber;
    QString         renderParentModel;
};

class DrawPageFlags
{
public:
    bool printing;
    bool updateViewer;
    bool csiAnnotation;
    bool buildModActionChange;
    DrawPageFlags()
        : printing(false)
        , updateViewer(true)
        , csiAnnotation(false)
        , buildModActionChange(false)
    { }
};

class DrawPageOptions
{

public:
    DrawPageOptions(
            Where                       &_current,
            QStringList                 &_csiParts,
            QStringList                 &_pliParts,
            QStringList                 &_bfxParts,
            QStringList                 &_ldrStepFiles,
            QStringList                 &_csiKeys,
            QHash<QString, QStringList> &_bfx,
            QList<PliPartGroupMeta>     &_pliPartGroups,
            QVector<int>                &_lineTypeIndexes,
            QHash<QString, QVector<int>>&_bfxLineTypeIndexes,

            int                          _stepNum,
            int                          _groupStepNumber,
            bool                         _updateViewer,
            bool                         _isMirrored,
            bool                         _printing,
            bool                         _bfxStore2,
            bool                         _assembledCallout = false,
            bool                         _calledOut        = false)
        :
          current                       (_current),
          csiParts                      (_csiParts),
          pliParts                      (_pliParts),
          bfxParts                      (_bfxParts),
          ldrStepFiles                  (_ldrStepFiles),
          csiKeys                       (_csiKeys),
          bfx                           (_bfx),
          pliPartGroups                 (_pliPartGroups),
          lineTypeIndexes               (_lineTypeIndexes),
          bfxLineTypeIndexes            (_bfxLineTypeIndexes),

          stepNum                       (_stepNum),
          groupStepNumber               (_groupStepNumber),
          updateViewer                  (_updateViewer),
          isMirrored                    (_isMirrored),
          printing                      (_printing),
          bfxStore2                     (_bfxStore2),
          assembledCallout              (_assembledCallout),
          calledOut                     (_calledOut)
    {  }
    Where                       &current;
    QStringList                 &csiParts;
    QStringList                 &pliParts;
    QStringList                 &bfxParts;
    QStringList                 &ldrStepFiles;
    QStringList                 &csiKeys;
    QHash<QString, QStringList> &bfx;
    QList<PliPartGroupMeta>     &pliPartGroups;
    QVector<int>                &lineTypeIndexes;
    QHash<QString, QVector<int>>&bfxLineTypeIndexes;

    int                          stepNum;
    int                          groupStepNumber;
    bool                         updateViewer;
    bool                         isMirrored;
    bool                         printing;
    bool                         bfxStore2;
    bool                         assembledCallout;
    bool                         calledOut;
};

class PartLineAttributes
{

public:
    PartLineAttributes(
            QStringList           &_csiParts,
            QVector<int>          &_lineTypeIndexes,
            QStringList           &_buildModCsiParts,
            QVector<int>          &_buildModLineTypeIndexes,
            int                    _buildModLevel,
            bool                   _buildModIgnore,
            bool                   _buildModItems)
        :
          csiParts                (_csiParts),
          lineTypeIndexes         (_lineTypeIndexes),
          buildModCsiParts        (_buildModCsiParts),
          buildModLineTypeIndexes (_buildModLineTypeIndexes),
          buildModLevel           (_buildModLevel),
          buildModIgnore          (_buildModIgnore),
          buildModItems           (_buildModItems)
    {  }
    QStringList           &csiParts;
    QVector<int>          &lineTypeIndexes;
    QStringList           &buildModCsiParts;
    QVector<int>          &buildModLineTypeIndexes;
    int                    buildModLevel;
    bool                   buildModIgnore;
    bool                   buildModItems;
};

#endif // OPTIONS_H
