 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2025 Trevor SANDY. All rights reserved.
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
 * This class represents one step including a step number, and assembly
 * image, possibly a parts list image and zero or more callouts needed for
 * the step.
 *
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/

#ifndef stepH
#define stepH

#include <QGraphicsRectItem>
#include <QString>
#include <QStringList>
#include <QHash>

#include "range_element.h"
#include "pli.h"
#include "meta.h"
#include "csiitem.h"
#include "callout.h"
#include "pagepointer.h"
#include "submodelitem.h"
#include "rotateiconitem.h"

class Meta;
class Callout;
class Range;
class MetaItem;
class ImageMatt;
class PagePointer;
class QGraphicsView;
class CsiAnnotationItem;

class Step : public AbstractRangeElement
{
  public:
    bool                  calledOut;
    bool                  multiStep;
    bool                  placeSubModel;
    bool                  placeRotateIcon;
    bool                  placeCsiAnnotation;
    DividerType           dividerType;
    QList<Callout *>      list;
    QList<CsiAnnotation*> csiAnnotations;
    QVector<int>          lineTypeIndexes;
    Pli                   pli;
    SubModel              subModel;
    CsiItem              *csiItem;
    Placement             csiPlacement;
    QPixmap               csiPixmap;
    RotateIconMeta        rotateIconMeta;
    RotateIconItem        rotateIcon;
    UnitsMeta             stepSize;
    PlacementNum          stepNumber;
    NumberPlacementMeta   numberPlacemetMeta;
    RotStepMeta           rotStepMeta;
    SettingsMeta          csiStepMeta;
    StringMeta            ldviewParms;
    StringMeta            ldgliteParms;
    StringMeta            povrayParms;
    JustifyStepMeta       justifyStep;
    BuildModMeta          buildModMeta;
    BuildModMeta          buildModActionMeta;
    DisplayType           displayStep;
    bool                  showStepNumber;
    int                   submodelLevel;
    bool                  pliPerStep;
    bool                  csiOutOfDate;
    bool                  bfxLoadStep;
    bool                  buildModActionStep;
    bool                  adjustOnItemOffset;
    bool                  justifyYAxisOutsidePlacement;
    bool                  updateViewer;
    QString               ldrName;
    QString               pngName;
    QString               csiKey;
    QString               viewerStepKey;
    NativeOptions        *viewerOptions;
    PlacementHeader       plPageHeader;
    PlacementFooter       plPageFooter;

    QMap<QString, LightData> lightList;

    QString               foo;

    MultiStepStepBackgroundItem *stepBackground;

    Step(
      Where                 &topOfStep,
      AbstractStepsElement *_parent,
      int                    number,
      Meta                  &_meta,
      bool                   calledOut,
      bool                   multiStep);

    virtual ~Step();

    void append(
      Callout *callout)
    {
      list.append(callout);
      callout->parentStep = this;
    }

    Step  *nextStep();
    Range *range();
    Page  *page();

    bool loadTheViewer();
    int  getLineTypeRelativeIndex(int lineTypeIndx);
    int  getLineTypeIndex(int relativeTypeIndx);

    MetaItem *mi(int which = -1)
    {
        switch(which) {
        case Options::PLI:
            return static_cast<MetaItem *>(pli.background);
        case Options::CSI:
            return static_cast<MetaItem *>(csiItem);
        case Options::SMI:
            return static_cast<MetaItem *>(subModel.background);
        default:
           return static_cast<MetaItem *>(csiItem);
        }
    }

    void getStepLocation(Where &top, Where &bottom);

    int setCsiAnnotationMetas(Meta &, bool = false);
    int setCsiAnnotationMetas(Meta &, int &, bool);

    void appendCsiAnnotation(
            const Where       &here,
            CsiAnnotationMeta &caMeta);

    int  createCsi(
           QString      const &addLine,
           QStringList  const &csiParts,
           QVector<int> const &lineTypeIndexes,
           QPixmap            *pixmap,
           Meta               &meta);

    QStringList configureModelStep(
        const QStringList &csiParts,
        Where             &current);

    int  sizeit(int  rows[],
                int  cols[],
                int  rowsMargin[][2],
                int  colsMargin[][2],
                int  x,
                int  y);

    bool adjustSize(
      Placement &pl1, // placement with offset
      int  rows[],    // accumulate sub-row heights here
      int  cols[]);   // accumulate sub-col widths here

    bool collide(int square[NumPlaces][NumPlaces],
                 int tbl[],
                 int x,
                 int y);

    void maxMargin(MarginsMeta &margin, int tbl[2], int r[][2], int c[][2]);
    int maxMargin(int &top, int &bot, int y = YY);

    void placeit(int rows[],
                     int margin[],
                     int y,
                     bool shared = false);

    void placeInside();

    void sizeitFreeform(
      int xx,
      int yy,
      int relativeBase,
      int relativeJustification,
      int &left,
      int &right);

    virtual void addGraphicsItems(int ox, int oy, Meta *, PlacementType, QGraphicsItem *, bool);

};

extern class Step* gStep;

#endif // stepH
