 
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
 * This class contains one or more individual range's.
 * By itself, this class represents step groups.  Callouts are derived
 * from ranges.
 *
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/

#ifndef rangesH
#define rangesH

#include "pli.h"
#include "meta.h"
#include "placement.h"
#include "submodelitem.h"
#include "pageattributepixmapitem.h"
#include "dividerpointeritem.h"
#include "textitem.h"

class Meta;
class Render;
class Pli;
class Placement;
class SubmodelItem;
class SubModel;
class RotateIcon;
class PageAttributePixmapItem;
class InsertPixmapItem;
class Pointer;
class PagePointer;

/*
 * This is a base class for multi-step and
 * callouts.
 */
class Step;
class PagePointer;
class AbstractStepsElement;
class Where;
class QGraphicsView;

class Steps : public Placement {
  public:
    Meta                           meta;
    Meta                           groupStepMeta;
    QList<AbstractStepsElement *>  list;                       // of range
    QMap<Positions,PagePointer *>  pagePointers;               // of pagePointers
    QList<DividerPointerItem *>    graphicsDividerPointerList; // of dividerPointers
    QList<TextItem*>               textItemList;               // of page Texts
    QGraphicsView                 *view;
    Pli                            pli;
    SubModel                       subModel;
    PlacementNum                   groupStepNumber;            // stepNumber for pli per page
    Where                          top;                        // needed for non-step pages
    Where                          bottom;
    bool                           isMirrored;
    bool                           placeSubModel;

    Steps();
    Steps(Meta &_meta,QGraphicsView *_view);
   ~Steps();

    QString modelName();
    QString path();
    QString csiName();
    QStringList submodelStack();
    void freeSteps();
    void append(AbstractStepsElement *re);

    virtual AllocEnc allocType();
    virtual AllocMeta &allocMeta();
      
    /* size ranges and place each range */

    void sizeIt();
    void sizeit(AllocEnc allocEnc, int x, int y);
    void sizeitFreeform(int xx, int yy);
    void addGraphicsItems(int ox, int oy, QGraphicsItem *parent);
    virtual void addGraphicsItems(AllocEnc, int ox, int oy, QGraphicsItem *parent);

    virtual int setCsiAnnotationMetas(bool = false);

    Boundary boundary(AbstractStepsElement *);

    int pageSizeDiv(int,bool = false,bool = true);

    const Where &bottomOfStep(AbstractStepsElement *me);
    const Where &topOfSteps();
    const Where &bottomOfSteps();
    void  setTopOfSteps(const Where &tos);
    void  setBottomOfSteps(const Where &bos);
    AbstractStepsElement *nextRange(const AbstractStepsElement *me);
};

class PageAttributePixmapItem;

class Page : public Steps {
  public:
    QList<InsertMeta> inserts;
    QList<InsertPixmapItem *> insertPixmaps;
    QList<PageAttributePixmapItem *> pageAttributePixmaps;
    QMap<Where, SceneObjectData> selectedSceneItems;     // page items whose depth is explicitly defined
    DisplayType displayPage;
    bool coverPage;
    bool frontCover;
    bool backCover;
    bool displayInstanceCount;
    int  instances;
    int  stepNumber;
    Page()
    {
      displayPage          = DT_DEFAULT;
      coverPage            = false;
      frontCover           = false;
      backCover            = false;
      displayInstanceCount = false;
      instances            = 1;
      stepNumber           = 0;
    }
    
    void addInsertPixmap(InsertPixmapItem *pixMap)
    {
      insertPixmaps.append(pixMap);
    }

    void addPageAttributePixmap(PageAttributePixmapItem *pixMap)
    {
      pageAttributePixmaps.append(pixMap);
    }

    void freePage()
    {
      displayPage          = DT_DEFAULT;
      coverPage            = false;
      frontCover           = false;
      backCover            = false;
      displayInstanceCount = false;
      placeSubModel        = false;
      instances            = 1;
      stepNumber           = 0;
      for (int i = 0; i < insertPixmaps.size(); i++) {
        InsertPixmapItem *pixmap = insertPixmaps[i];
        delete pixmap;
      }
      for (int i = 0; i < pageAttributePixmaps.size(); i++) {
        PageAttributePixmapItem *pixmapItem = pageAttributePixmaps[i];
        delete pixmapItem;
      }
      insertPixmaps.clear();
      pageAttributePixmaps.clear();
      inserts.clear();
      selectedSceneItems.clear();
      freeSteps();
    }
};

#endif
