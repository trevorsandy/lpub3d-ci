 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2019 Trevor SANDY. All rights reserved.
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
 * This file describes the data structure that represents an LPub callout
 * (the steps of a submodel packed together and displayed next to the
 * assembly where the submodel is used).  
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include <QtWidgets>

#include "lpub.h"
#include "callout.h"
#include "calloutbackgrounditem.h"
#include "pointer.h"
#include "calloutpointeritem.h"
#include "numberitem.h"
#include "ranges.h"
#include "range.h"
#include "step.h"
#include "placementdialog.h"
#include "commonmenus.h"
#include "paths.h"
#include "render.h"

//---------------------------------------------------------------------------

Callout::Callout(
  Meta                 &_meta,
  QGraphicsView        *_view)
{
  view          = _view;
  relativeType  = CalloutType;
  meta          = _meta;
  instances     = 1;
  shared        = false;
  if (meta.LPub.callout.begin.mode != CalloutBeginMeta::Unassembled) {
    meta.LPub.callout.pli.perStep.setValue(false);
  }
}

Callout::~Callout()
{
  Steps::list.clear();
  for (int i = 0; i < pointerList.size(); i++) {
    Pointer *p = pointerList[i];
    delete p;
  }
  pointerList.clear();
}

AllocEnc Callout::allocType()
{
  if (relativeType == CalloutType) {
    return meta.LPub.callout.alloc.value();
  } else {
    return meta.LPub.multiStep.alloc.value();
  }
}

AllocMeta &Callout::allocMeta()
{
  if (relativeType == CalloutType) {
    return meta.LPub.callout.alloc;
  } else {
    return meta.LPub.multiStep.alloc;
  }
}

void Callout::appendPointer(
  const Where       &here,
  PointerMeta       &pointerMeta,
  PointerAttribMeta &pointerAttrib)
{
    int pid = pointerList.size() + 1;
    PointerAttribMeta pam = pointerAttrib;
    Pointer *pointer = new Pointer(pid,here,pointerMeta);
    pam.setDefaultColor(meta.LPub.callout.border.value().color);
    pointer->setPointerAttribInches(pam);
    pointerList.append(pointer);
}

void Callout::sizeIt()
{
  AllocEnc allocEnc = meta.LPub.callout.alloc.value();

  if (allocEnc == Vertical) {
    Steps::sizeit(allocEnc,XX,YY);
  } else {
    Steps::sizeit(allocEnc,YY,XX);
  }

//#ifdef QT_DEBUG_MODE
//  logNotice() << "\nCallout Size:"
//              << "\nSize XX W    [" << size[XX] << "]"
//              << "\nSize YY H    [" << size[YY] << "]"
//                ;
//  int dbg_adj_size[2];
//  dbg_adj_size[XX] = size[XX];
//  dbg_adj_size[YY] = size[YY];
//#endif

  BorderData borderData = meta.LPub.callout.border.valuePixels();

  float borderADjustmentX = borderData.margin[XX] + borderData.thickness;
  float borderADjustmentY = borderData.margin[YY] + borderData.thickness;

  size[XX] += int(borderADjustmentX);
  size[YY] += int(borderADjustmentY);

//#ifdef QT_DEBUG_MODE
//  logNotice() << "\nCallout Adjusted Size:"
//              << "\nBorderAdjustmentX XX W [" << borderADjustmentX << "] = BorderData.margin XX W" << borderData.margin[XX] << "+ BorderData.thickness" << borderData.thickness
//              << "\nBorderAdjustmentY YY H [" << borderADjustmentY << "] = BorderData.margin YY H" << borderData.margin[YY] << "+ BorderData.thickness" << borderData.thickness
//              << "\nAdjusted size XX W     [" << size[XX] << "] = Size[XX]" << dbg_adj_size[XX] << "+ BorderAdjustmentX XX W" << borderADjustmentX
//              << "\nAdjusted size YY H     [" << size[YY] << "] = Size[YY]" << dbg_adj_size[YY] << "+ BorderAdjustmentX YY H" << borderADjustmentY
//                ;
//  dbg_adj_size[XX] = size[XX];
//  dbg_adj_size[YY] = size[YY];
//#endif

  /* If we've got multiple instances of a submodel, we need to add
     the usage count in the lower right corner.  for assem, pli, and
     step number, determine if bottom right corner overlaps with the
     top left corner of the count.  If it fits, place the count
     inside.  If not, calculate the area created by adding to side,
     and compare the area created by adding to the bottom.  Add the
     smaller area to the callout, and place the count accordingly */

  if (instances > 1) {
    instanceCount.number    = instances;
    instanceCount.placement = meta.LPub.callout.instance.placement;
    instanceCount.margin    = meta.LPub.callout.instance.margin;
    instanceCount.font      = meta.LPub.callout.instance.font.valueFoo();
    instanceCount.sizeit("%1x");
    
    bool fit = true;

    /* Describe the instance count rectangle */

    int  instanceTop, instanceLeft, instanceWidth, instanceHeight;

    instanceWidth  = instanceCount.size[XX] + instanceCount.margin.valuePixels(XX);
    instanceHeight = instanceCount.size[YY] + instanceCount.margin.valuePixels(YY);

    instanceLeft   = int(size[XX] - instanceWidth  - (borderADjustmentX/2));
    instanceTop    = int(size[YY] - instanceHeight - (borderADjustmentY/2));

    for (int j = 0; j < list.size(); j++) {
      if (list[j] && list[j]->relativeType == RangeType) {
        Range *range = dynamic_cast<Range *>(list[j]);
        for (int i = 0; i < range->list.size(); i++) {
          if (range->list[i]->relativeType == StepType) {
            Step *step = dynamic_cast<Step *>(range->list[i]);

            /* if there is something to work with */
            
            int  right;
            int  bottom;
            
            // Overlap count with CSI?
              
            right  = step->loc[XX];
            right += step->csiPlacement.loc[XX] + step->csiPlacement.size[XX];
            right += range->loc[XX];
            bottom  = step->loc[YY];
            bottom += step->csiPlacement.loc[YY] + step->csiPlacement.size[YY];
            bottom += range->loc[YY];

            if (right > instanceLeft && bottom > instanceTop) {
              fit = false;
            } else {      
              right  = step->loc[XX];
              right += step->pli.loc[XX] + step->pli.size[XX];
              right += range->loc[XX];
              bottom  = step->loc[YY];
              bottom += step->pli.loc[YY] + step->pli.size[YY];
              bottom += range->loc[YY];

              // Overlap with PLI?
                
              if (right > instanceLeft && bottom > instanceTop) {
                fit = false;
              } else {
                right  = step->loc[XX];
                right += step->stepNumber.loc[XX] + step->stepNumber.size[XX];
                right += range->loc[XX];
                bottom  = step->loc[YY];
                bottom += step->stepNumber.loc[YY] + step->stepNumber.size[YY];
                bottom += range->loc[YY];

                // Overlap with Step Number?
                  
                fit = ! (right > instanceLeft && bottom > instanceTop);
                if (fit) {
                  for (int i = 0; i < step->list.size(); i++) {
                    Callout *callout = step->list[i];
                    if (callout) {
                      right  = step->loc[XX];
                      right += callout->loc[XX] + callout->size[XX];
                      bottom  = step->loc[XX];
                      bottom += callout->loc[YY] + callout->size[YY];
                      if (right > instanceLeft && bottom > instanceTop) {
                        fit = false;
                        break;
                      }
                    }
                  }
                }
              }
            }
          } // if step
        } // foreach step
      } // if range
    } // foreach range
    if (fit) {
      instanceCount.loc[XX] = instanceLeft + instanceCount.margin.valuePixels(XX);
      instanceCount.loc[YY] = instanceTop + instanceCount.margin.valuePixels(YY);
    } else {
      // enlarge the callout to make room

      int a = instanceWidth  * size[YY];
      int b = instanceHeight * size[XX];
      int max;

      if (a < b) {
        instanceCount.loc[XX] = size[XX];
        max = (instanceCount.margin.valuePixels(YY) > borderData.margin[YY]) ? instanceCount.margin.valuePixels(YY) : borderData.margin[YY];
        instanceCount.loc[YY] = instanceTop + int(max + borderData.thickness);
        max = (instanceCount.margin.valuePixels(XX) > borderData.margin[XX]) ? instanceCount.margin.valuePixels(XX) : borderData.margin[XX];
        size[XX] += instanceCount.size[XX] + max;
        size[YY] += int(borderData.margin[YY]);
      } else {
        max = (instanceCount.margin.valuePixels(XX) > borderData.margin[XX]) ? instanceCount.margin.valuePixels(XX) : borderData.margin[XX];
        instanceCount.loc[XX] = instanceLeft + int(max + borderData.thickness);
        instanceCount.loc[YY] = size[YY];
        max = (instanceCount.margin.valuePixels(YY) > borderData.margin[YY]) ? instanceCount.margin.valuePixels(YY) : borderData.margin[YY];
        size[YY] += instanceCount.size[YY] + max;
        size[XX] += int(borderData.margin[XX]);
      }
    }
  } else {
    size[XX] += int(borderData.margin[XX]);
    size[YY] += int(borderData.margin[YY]);
  }

//#ifdef QT_DEBUG_MODE // " <<  << " // " <<
//  dbg_adj_size[XX] = size[XX] - dbg_adj_size[XX];
//  dbg_adj_size[YY] = size[YY] - dbg_adj_size[YY];
//  logNotice() << "\nCallout 2nd Adjusted Size:"
//              << "\nAdjustment XX W (Instance Count/Border Margin) [" << dbg_adj_size[XX] << "]"
//              << "\nAdjustment YY H (Instance Count/Border Margin) [" << dbg_adj_size[YY] << "]"
//              << "\n2nd Adjusted size XX W                         [" << size[XX] << "]"
//              << "\n2nd Adjusted size YY H                         [" << size[YY] << "]"
//                ;
//#endif

  size[XX] += int(borderData.thickness);
  size[YY] += int(borderData.thickness);

//#ifdef QT_DEBUG_MODE
//  logNotice() << "\nCallout Final Size:"
//              << "\nAdjustment XX W (Border Thickness)  [" << borderData.thickness << "]"
//              << "\nAdjustment XX H (Border Thickness)  [" << borderData.thickness << "]"
//              << "\n----------------"
//              << "\nFinal Callout size XX W            [" << size[XX] << "]"
//              << "\nFinal Callout size YY H            [" << size[YY] << "]"
//                ;
//#endif
}

// Callouts that have round corners are tricky, trying to get the pointer to start/end on the
// rounded corner.  To avoid trying to know the shape of the curve, we make sure the pointer
// is below (think zDepth) the callout.  If we make the pointer start at the center of the curved
// corner rather than the edge, then the callout hides the starting point of the pointer, and the
// pointer always appears to start right at the edge of the callout (no matter the shape of the
// corner's curve.
void Callout::addGraphicsItems(
  int            offsetX,
  int            offsetY,
  QRect         &csiRect,
  QGraphicsItem *parent,
  bool           movable)
{
  PlacementData placementData = placement.value();
  
  if (placementData.relativeTo == PageType ||
      placementData.relativeTo == StepGroupType ||
      placementData.relativeTo == CalloutType) {
    offsetX = 0;
    offsetY = 0;
  }

  int newLoc[2] = { offsetX + loc[XX], offsetY + loc[YY] };

  // Add an invisible rectangle underneath the callout background item called underpinning.
  // Pointers will be added with underpinning as the parent.  This makes sure the start of the
  // pointer is covered by the callout background.

  // If we have the pointers use callout background as parent, the pointer is on top of the
  // background.  So by using underpinnings, the callout end of the pointer is under the
  // background.  This allows us to have the pointers look correct for round cornered callouts.

  underpinnings = new QGraphicsRectItem(
      qreal(newLoc[XX]),qreal(newLoc[YY]),qreal(size[XX]),qreal(size[YY]),parent);
  QPen pen;
  QColor none(0,0,0,0);
  pen.setColor(none);
  underpinnings->setPen(pen);
  underpinnings->setPos(newLoc[XX],newLoc[YY]);
  underpinnings->setData(ObjectId, CalloutUnderpinningObj);
  underpinnings->setZValue(meta.LPub.page.scene.calloutUnderpinning.zValue());

  QRect calloutRect(newLoc[XX],newLoc[YY],size[XX],size[YY]);

  // This is the background for the entire callout.

  background = new CalloutBackgroundItem(
                     this,
                     calloutRect,
                     csiRect,
                     parentRelativeType,
                    &meta,
                     meta.submodelStack.size(),
                     path(),
                     parent,
                     view);

  background->setPos(newLoc[XX],newLoc[YY]);
  background->setFlag(QGraphicsItem::ItemIsMovable, movable);

  int saveLocX = loc[XX];
  int saveLocY = loc[YY];

  BorderData borderData = meta.LPub.callout.border.valuePixels();

  if (instanceCount.number > 1) {
    loc[XX] = int(borderData.margin[0])/2;
    loc[YY] = int(borderData.margin[1])/2;
  } else {
    loc[XX] = int(borderData.margin[0]);
    loc[YY] = int(borderData.margin[1]);
  }
 
//#ifdef QT_DEBUG_MODE
//  logNotice() << "\nCALLOUT PLACEMENT SUMMARY (addGraphicsItems) - "
//              << "\nPLACEMENT DATA -       "
//              << " \nPlacement:            "   << PlacNames[placement.value().placement]     << " (" << placement.value().placement << ")"
//              << " \nJustification:        "   << PlacNames[placement.value().justification] << " (" << placement.value().justification << ")"
//              << " \nRelativeTo:           "   << RelNames[placement.value().relativeTo]     << " (" << placement.value().relativeTo << ")"
//              << " \nPreposition:          "   << PrepNames[placement.value().preposition]   << " (" << placement.value().preposition << ")"
//              << " \nRectPlacement:        "   << RectNames[placement.value().rectPlacement] << " (" << placement.value().rectPlacement << ")"
//              << " \nOffset[0](Placement): "   << placement.value().offsets[0]
//              << " \nOffset[1](Placement): "   << placement.value().offsets[1]
//              << "\nOTHER DATA -           "
//              << " \nOffxetX:              "   << offsetX
//              << " \nOffxetY:              "   << offsetY
//              << " \nAlloc:                "   << (meta.LPub.callout.alloc.value() == Vertical ? "Vertical" : "Horizontal")
//              << " \nBorder Thickness      "   << borderData.thickness
//              << " \nRelative Type:        "   << RelNames[relativeType]       << " (" << relativeType << ")"
//              << " \nParent Relative Type: "   << RelNames[parentRelativeType] << " (" << parentRelativeType << ")"
//              << " \nSize[XX]:             "   << size[XX]
//              << " \nSize[YY]:             "   << size[YY]
//              << " \nnewLoc[XX]:           "   << newLoc[XX]
//              << " \nnewLoc[YY]:           "   << newLoc[YY]
//              << " \nLoc[XX](bdr.margin):  "   << loc[XX]
//              << " \nLoc[YY](bdr.margin):  "   << loc[YY]
//              << " \nsaveLocX:             "   << saveLocX
//              << " \nsaveLocY:             "   << saveLocY
//              << " \nRelative To Size[0]:  "   << relativeToSize[0]
//              << " \nRelative To Size[1]:  "   << relativeToSize[1]
//                ;
//#endif

  if (meta.LPub.callout.alloc.value() == Vertical) {
    addGraphicsItems(Vertical,0,int(borderData.thickness),background, movable);
  } else {
    addGraphicsItems(Horizontal,int(borderData.thickness),0,background, movable);
  }

  loc[XX] = saveLocX;
  loc[YY] = saveLocY;
}


void Callout::addGraphicsItems(
  AllocEnc       allocEnc,
  int            offsetX,
  int            offsetY,
  QGraphicsItem *parent,
  bool           movable)
{
  int margin;

  if (instanceCount.number > 1) {

    PlacementData placementData = instanceCount.placement.value();
    switch (placementData.placement) {
      case TopLeft:
      case Left:
      case BottomLeft:
        margin   = meta.LPub.callout.instance.margin.valuePixels(XX);
        offsetX += instanceCount.size[XX] + margin;
      break;
      default:
      break;
    }

    switch (placementData.placement) {
      case TopLeft:
      case Top:
      case TopRight:
        margin   = meta.LPub.callout.instance.margin.valuePixels(YY);
        offsetY += instanceCount.size[YY] + margin;
      break;
      default:
      break;
    }

    CalloutInstanceItem *item = new CalloutInstanceItem(
      this,&meta,"x%d",instanceCount.number,parent);
    item->setPos(offsetX + instanceCount.loc[0], offsetY + instanceCount.loc[1]);
    item->setData(ObjectId, CalloutInstanceObj);
    item->setZValue(meta.LPub.page.scene.calloutInstance.zValue());
  }

  Steps::addGraphicsItems(allocEnc,
                          offsetX,
                          offsetY,
                          parent);
  background->setFlag(QGraphicsItem::ItemIsMovable,movable);
}

void Callout::sizeitFreeform(
  int xx,
  int yy)
{
  Steps::sizeitFreeform(xx,yy);

  size[XX] += 2*int(meta.LPub.callout.border.valuePixels().thickness);
  size[YY] += 2*int(meta.LPub.callout.border.valuePixels().thickness);

  if (instanceCount.number > 1) {
    instanceCount.sizeit("%1x");
    PlacementData placementData = instanceCount.placement.value();
    if (placementData.preposition == Outside) {

      switch (placementData.placement) {
        case TopLeft:
        case Left:
        case BottomLeft:
        case TopRight:
        case Right:
        case BottomRight:
          size[XX] += margin.valuePixels(XX) + instanceCount.size[XX];
        break;
        default:
        break;
      }
      switch (placementData.placement) {
        case TopLeft:
        case Top:
        case TopRight:
        case BottomLeft:
        case Bottom:
        case BottomRight:
          size[YY] += margin.valuePixels(YY) + instanceCount.size[YY];
        break;
        default:
        break;
      }
      placementData.preposition = Inside;
      instanceCount.placement.setValue(placementData);

      placeRelative(&instanceCount);

      placementData = instanceCount.placement.value();
      placementData.preposition = Inside;
      instanceCount.placement.setValue(placementData);
    } else {
      size[YY] += margin.valuePixels(YY);
      placeRelative(&instanceCount);
    }
  }
}

CalloutInstanceItem::CalloutInstanceItem(
  Callout             *_callout,
  Meta                *_meta,
  const char          *_format,
  int                  _value,
  QGraphicsItem       *_parent)
{
  callout = _callout;
  QString toolTip("Times used - right-click to modify");
  setAttributes(PageNumberType,
                CalloutType,
                _meta->LPub.callout.instance,
                _format,
                _value,
                toolTip,
                _parent);
}

void Callout::addGraphicsPointerItem(
  Pointer *pointer,
  QGraphicsItem *parent)
{
  CalloutPointerItem *t = 
    new CalloutPointerItem(
          this,
          pointer,
           parent,
          view);
  graphicsCalloutPointerList.append(t);
}

void Callout::updatePointers(QPoint &delta)
{
  for (int i = 0; i < graphicsCalloutPointerList.size(); i++) {
    CalloutPointerItem *pointer = graphicsCalloutPointerList[i];
    pointer->updatePointer(delta);
  }
}

void Callout::drawTips(QPoint &delta, int type)
{
  for (int i = 0; i < graphicsCalloutPointerList.size(); i++) {
    CalloutPointerItem *pointer = graphicsCalloutPointerList[i];
    int initiator = type ? type : relativeType;
    pointer->drawTip(delta,initiator);
  }
}

void CalloutInstanceItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;
  QString ci = "Callout Instance Count";
  QAction *fontAction   = commonMenus.fontMenu(menu,ci);
  QAction *colorAction  = commonMenus.colorMenu(menu,ci);
  QAction *marginAction = commonMenus.marginMenu(menu,ci);

  QAction *selectedAction   = menu.exec(event->screenPos());

  if (selectedAction == nullptr) {
    return;
  }

  if (selectedAction == fontAction) {

    changeFont(callout->topOfCallout(), callout->bottomOfCallout(),&font);

  } else if (selectedAction == colorAction) {

    changeColor(callout->topOfCallout(), callout->bottomOfCallout(),&color);

  } else if (selectedAction == marginAction) {

    changeMargins("Times Used Margin",
                  callout->topOfCallout(), callout->bottomOfCallout(),
                  &margin, false);
  }
}

void CalloutInstanceItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseReleaseEvent(event);
  QPointF newPosition;
  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable) && positionChanged) {
    // back annotate the movement of the PLI into the LDraw file.
    newPosition = pos() - position;
    PlacementData placementData = placement.value();
    placementData.offsets[0] += newPosition.x()/relativeToSize[0];
    placementData.offsets[1] += newPosition.y()/relativeToSize[1];
    placement.setValue(placementData);
    changePlacementOffset(callout->topOfCallout(),&placement,CalloutType,false);
  }
}


QString Callout::wholeSubmodel(
  Meta    & meta,
  QString &modelName,
  QString & addLine,
  int depth)
{
  QStringList argv;
  split(addLine,argv);
  QString mirrored;
  QString assembled;
  bool isMirrored = LDrawFile::mirrored(argv);

  if (isMirrored) {
    mirrored = "mirrored_";
  }

  if (meta.LPub.callout.begin.value() == CalloutBeginMeta::Rotated) {
    assembled = "rotated_";
  } else {
    assembled = "assembled_";
  }
  const QString wholeName = "whole_" + assembled + mirrored + modelName;
  //if (gui->subFileSize(wholeName)) {
    //return wholeName;
  //}


  int numLines = gui->subFileSize(modelName);
  QStringList csiParts;
  Where walk(modelName,0);

  for ( ; walk < numLines; walk++) {
    QString line = gui->readLine(walk);
    QStringList tokens;
    split(line,tokens);
    int num_tokens = tokens.size();

    if (num_tokens > 0 && tokens[0] == "0") {
      if (num_tokens == 2 && tokens[1] == "STEP") {
        continue;
      }
      if (num_tokens > 1 && tokens[1] == "ROTSTEP") {
        continue;
      }
      if (num_tokens > 2 && (tokens[1] == "LPUB" || tokens[1] == "!LPUB") && tokens[2] == "MULTI_STEP") {
        continue;
      }
    } else if (num_tokens == 15 && tokens[0] == "1") {
      if (gui->isSubmodel(tokens[14])) {
        tokens[14] = wholeSubmodel(meta,tokens[14],line,depth + 1);
        if (isGhost(line))
            tokens.prepend(GHOST_META);
        line = tokens.join(" ");
      }
    }
    csiParts << line;
  }

  if ( ! isMirrored && meta.LPub.callout.begin.value() == CalloutBeginMeta::Rotated && depth == 0) {
    //Render::rotateParts(addLine,meta.rotStep,csiParts,false);
    RotStepMeta emptyRotStep; FloatPairMeta emptyCameraAngles;
    Render::rotateParts(addLine,emptyRotStep,csiParts,emptyCameraAngles,false);
    //logDebug() << "Called rotateParts for Called out 'Rotated' parts;";
  }

  gui->insertGeneratedModel(wholeName,csiParts);
  //logDebug() << "Called insertGeneratedModel with passed parms wholeName: " << wholeName << ", cisParts: " << csiParts;

  return wholeName;
}
