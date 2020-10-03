 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2020 Trevor SANDY. All rights reserved.
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
 * The class described in this file is a refined version of the background
 * graphics item class that is used specifically for callouts.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "lpub.h"
#include <QtWidgets>
#include "callout.h"
#include "calloutbackgrounditem.h"
#include "pointer.h"
#include "calloutpointeritem.h"
#include <QGraphicsView>
#include "commonmenus.h"

CalloutBackgroundItem::CalloutBackgroundItem(
  Callout       *_callout,
  QRect         &_calloutRect,
  QRect         &_csiRect,
  PlacementType  parentRelativeType,
  Meta          *meta,
  int            submodelLevel,
  QString        _path,
  QGraphicsItem *parent,
  QGraphicsView *_view)
{
  callout     = _callout;
  view        = _view;
  calloutRect = _calloutRect;
  csiRect     = _csiRect;

  QPixmap *pixmap = new QPixmap(_calloutRect.width(),_calloutRect.height());
  QString toolTip = QString("Callout " + _path + " [%1 x %2 px] - right-click to modify")
                            .arg(_calloutRect.width())
                            .arg(_calloutRect.height());;

  //gradient settings
  if (meta->LPub.callout.background.value().gsize[0] == 0 &&
      meta->LPub.callout.background.value().gsize[1] == 0) {
      meta->LPub.callout.background.value().gsize[0] = pixmap->width();
      meta->LPub.callout.background.value().gsize[1] = pixmap->width();
      QSize gSize(meta->LPub.callout.background.value().gsize[0],
          meta->LPub.callout.background.value().gsize[1]);
      int h_off = gSize.width() / 10;
      int v_off = gSize.height() / 8;
      meta->LPub.callout.background.value().gpoints << QPointF(gSize.width() / 2, gSize.height() / 2)
                                                    << QPointF(gSize.width() / 2 - h_off, gSize.height() / 2 - v_off);
    }

  setBackground(pixmap,
                CalloutType,
                parentRelativeType,
                meta->LPub.callout.placement,
                meta->LPub.callout.background,
                meta->LPub.callout.border,
                meta->LPub.callout.margin,
                meta->LPub.callout.subModelColor,
                submodelLevel,
                toolTip);
  setPixmap(*pixmap);
  delete pixmap;
  setParentItem(parent);

  calloutMeta = meta->LPub.callout;
  perStep = &calloutMeta.pli.perStep;
  alloc   = &calloutMeta.alloc;
  page    = &meta->LPub.page;

  setFlag(QGraphicsItem::ItemIsSelectable,true);
  setFlag(QGraphicsItem::ItemIsMovable,true);
  setData(ObjectId, CalloutBackgroundObj);
  setZValue(CALLOUTBACKGROUND_ZVALUE_DEFAULT);
}

void CalloutBackgroundItem::contextMenuEvent(
  QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;
  QString name = "Callout ";

  PlacementData placementData = callout->meta.LPub.callout.placement.value();
  QAction *placementAction  = commonMenus.placementMenu(menu,name,
                              commonMenus.naturalLanguagePlacementWhatsThis(CalloutType,placementData,name));

  QAction *allocAction = nullptr;
  QAction *perStepAction = nullptr;

  if (calloutMeta.begin.value() == CalloutBeginMeta::Unassembled) {

    if (callout->allocType() == Vertical) {
      allocAction = commonMenus.displayRowsMenu(menu,name);
    } else {
      allocAction = commonMenus.displayColumnsMenu(menu, name);
    }

    if (callout->meta.LPub.callout.pli.perStep.value()) {
      perStepAction = commonMenus.noPartsList(menu,name);
    } else {
      perStepAction = commonMenus.partsList(menu,name);
    }
  }

  QAction *editBackgroundAction    = commonMenus.backgroundMenu(menu,name);
  QAction *editBorderAction        = commonMenus.borderMenu(menu,name);
  QAction *editSubModelColorAction = commonMenus.subModelColorMenu(menu,name);
  QAction *marginAction            = commonMenus.marginMenu(menu,name);
  QAction *rotateAction            = nullptr;

  QAction *unCalloutAction;

  if (calloutMeta.begin.value() == CalloutBeginMeta::Unassembled) {
    unCalloutAction = menu.addAction("Unpack Callout");
    unCalloutAction->setIcon(QIcon(":/resources/unpackcallout.png"));
  } else {
    unCalloutAction = menu.addAction("Remove Callout");
    unCalloutAction->setIcon(QIcon(":/resources/remove.png"));
    if (calloutMeta.begin.value() == CalloutBeginMeta::Assembled) {
      rotateAction = menu.addAction("Rotate");
      rotateAction->setIcon(QIcon(":/resources/rotate.png"));
    } else {
      rotateAction = menu.addAction("Unrotate");
      rotateAction->setIcon(QIcon(":/resources/unrotate.png"));
    }
  }

  QAction *addPointerAction = menu.addAction("Add Pointer");
  addPointerAction->setWhatsThis("Add pointer from this callout to the step where it is used");
  addPointerAction->setIcon(QIcon(":/resources/addpointer.png"));

  QAction *selectedAction = menu.exec(event->screenPos());

  if (selectedAction == nullptr) {
      return;
  } else if (selectedAction == addPointerAction) {
    int pid = callout->pointerList.size()+1;
    Pointer *pointer = new Pointer(pid,callout->topOfCallout(),calloutMeta.pointer);
    /*
    float _loc = 0, _x1 = 0, _y1 = 0;
    float           _x2 = 0, _y2 = 0;
    float           _x3 = 0, _y3 = 0;
    float           _x4 = 0, _y4 = 0;
    float _base = -1, _segments = 1;
    pointer->pointerMeta.setValue(
    PlacementEnc(TopLeft),
    _loc,
    _base,
    _segments,
    _x1,_y1,_x2,_y2,_x3,_y3,_x4,_y4);
    */
    pointer->setPointerAttribInches(calloutMeta.pointerAttrib);
    CalloutPointerItem *calloutPointer = 
      new CalloutPointerItem(callout,pointer,this,view);
    calloutPointer->defaultPointer();

  } else if (selectedAction == perStepAction) {
    changeBool(callout->topOfCallout(),
               callout->bottomOfCallout(),
               &callout->meta.LPub.callout.pli.perStep,true,0,false,false);

  } else if (selectedAction == placementAction) {
    changePlacement(parentRelativeType, 
                    relativeType,
                    "Placement",
                    callout->topOfCallout(),
                    callout->bottomOfCallout(),
                    &placement,false,0,false,false);

  } else if (selectedAction == editSubModelColorAction) {
      changeSubModelColor("Submodel Color",
                       callout->topOfCallout(),
                       callout->bottomOfCallout(),
                       &subModelColor,0,false,false);

  } else if (selectedAction == editBackgroundAction) {
    changeBackground("Background",
                     callout->topOfCallout(), 
                     callout->bottomOfCallout(),
                     &background,false,0,false);

  } else if (selectedAction == editBorderAction) {
    changeBorder("Border",
                 callout->topOfCallout(), 
                 callout->bottomOfCallout(),
                 &border,false,0,false);

  } else if (selectedAction == marginAction) {
    changeMargins("Callout Margins",
                  callout->topOfCallout(), 
                  callout->bottomOfCallout(), 
                  &margin,false,0,false);

  } else if (selectedAction == unCalloutAction) {
    removeCallout(callout->modelName(),
                  callout->topOfCallout(),
                  callout->bottomOfCallout());
  } else if (selectedAction == rotateAction) {
    changeRotation(callout->topOfCallout());
  } else if (selectedAction == allocAction) {
    changeAlloc(callout->topOfCallout(),
                callout->bottomOfCallout(),
                callout->allocMeta(),
                0); // append - default is no
  }
}

/*
 * As the callout moves, the CSI stays in place, yet since the callout is
 * grouped with the pointer, the pointer is moved.  
 */

void CalloutBackgroundItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  positionChanged = false;
  position = pos();
  QGraphicsItem::mousePressEvent(event);
}

/*
 * When moving a callout, we want the tip of any pointers to stay still.  We
 * need to figure out how much the callout moved, and then compensate 
 */

void CalloutBackgroundItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsPixmapItem::mouseMoveEvent(event);
  // when sliding the callout up, we get negative Y
  // when sliding left of callout base we get negativeX?
  if ((flags() & QGraphicsItem::ItemIsMovable) && isSelected()) {
    QPoint delta(int(position.x() - pos().x() + 0.5),
                 int(position.y() - pos().y() + 0.5));

    if (delta.x() || delta.y()) {
      callout->drawTips(delta,CalloutType);
      positionChanged = true;
    }
  }
}
void CalloutBackgroundItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{

  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable) && positionChanged) {
    gui->beginMacro(QString("DraggingCallout"));

    QPointF delta(position.x() - pos().x(),
                  position.y() - pos().y());

    if (delta.x() || delta.y()) {

      QPoint deltaI(int(delta.x()+0.5),int(delta.y()+0.5));
      for (int i = 0; i < callout->graphicsCalloutPointerList.size(); i++) {
        CalloutPointerItem *pointer = callout->graphicsCalloutPointerList[i];
        pointer->updatePointer(deltaI);
      }
      PlacementData placementData = placement.value();

      float w = delta.x()/gui->pageSize(callout->meta.LPub.page, 0);
      float h = delta.y()/gui->pageSize(callout->meta.LPub.page, 1);

      if (placementData.relativeTo == CsiType) {
        w = delta.x()/csiRect.width();
        h = delta.y()/csiRect.height();
      }
  
      placementData.offsets[0] -= w;
      placementData.offsets[1] -= h;
      placement.setValue(placementData);

      changePlacementOffset(callout->topOfCallout(),&placement,CalloutType,false,0);  
    }
    QGraphicsItem::mouseReleaseEvent(event);
    gui->endMacro();
  } else {
    QGraphicsItem::mouseReleaseEvent(event);
  } 
}
