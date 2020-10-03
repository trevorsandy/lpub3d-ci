 
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

#include <QMenu>
#include <QAction>
#include "resize.h"
#include "metaitem.h"
#include "scaledialog.h"
#include "step.h"
#include "ranges.h"
#include "color.h"
#include "commonmenus.h"
#include "pagebackgrounditem.h"
#include "lpub.h"

qreal AbstractResize::grabsize = 0.125; // inches

RectPlacement constAspectRatioResizePlacement[] =
{ TopLeftOutsideCorner, 
  TopRightOutsideCorner, 
  BottomRightOutsideCorner, 
  BottomLeftOutsideCorner};

void ResizeRect::placeGrabbers()
{
  QRectF rect = currentRect();
  int    left = rect.left();
  int    top  = rect.top();
  int    width = rect.width();
  int    height = rect.height();
  int    bottom = rect.bottom();
  
  points[0] = QPointF(left,top);
  points[1] = QPointF(left + width, top);
  points[2] = QPointF(left + width, top + height);
  points[3] = QPointF(left, top + height);
  
  if (grabbers[0] == nullptr) {
    for (int i = 0; i < numGrabbers; i++) {
      grabbers[i] = new Grabber(constAspectRatioResizePlacement[i],
                               this,myParentItem());
    }
  }
  for (int i = 0; i < numGrabbers; i++) {
    grabbers[i]->setPos(points[i].x()-grabSize()/2,points[i].y()-grabSize()/2);
  }
}

void ResizeConstAspectRect::resize(QPointF grabbed)
{
  // recalculate corners Y

  switch (selectedGrabber) {
    case TopLeftOutsideCorner:
    case TopRightOutsideCorner:
      points[0].setY(grabbed.y());
      points[1].setY(grabbed.y());
    break;
    case BottomRightOutsideCorner:
    case BottomLeftOutsideCorner:
      points[2].setY(grabbed.y());
      points[3].setY(grabbed.y());
    break;
    default:
    break;
  }

  qreal  rawWidth, rawHeight;

  switch (selectedGrabber) {
    case TopLeftOutsideCorner:
    case BottomLeftOutsideCorner:
      points[0].setX(grabbed.x());
      points[3].setX(grabbed.y());
    break;
    case TopRightOutsideCorner:
    case BottomRightOutsideCorner:
      points[1].setX(grabbed.x());
      points[2].setX(grabbed.x());
    break;
    default:
    break;
  }
 
  switch (selectedGrabber) {
    case TopLeftOutsideCorner:
      rawWidth = points[2].x() - grabbed.x();
      rawHeight = points[2].y() - grabbed.y();
    break;
    case TopRightOutsideCorner:
      rawWidth = grabbed.x()-points[3].x();
      rawHeight = points[3].y() - grabbed.y();
    break;
    case BottomRightOutsideCorner:
      rawWidth = grabbed.x() - points[0].x();
      rawHeight = grabbed.y() - points[0].y();
    break;
    default:
      rawWidth = points[1].x() - grabbed.x();
      rawHeight = grabbed.y() - points[1].y();
    break;
  }
  
  if (rawWidth > 0 && rawHeight > 0) {

    // Force aspect ratio to match original aspect ratio of picture
    // ratio = width/height
    // width = height * ratio
    
    qreal width = rawHeight * size[0]/size[1];
    qreal height = rawWidth * size[1]/size[0];
    
    if (width * rawHeight < rawWidth * height) {
      height = rawHeight;
    } else {
      width = rawWidth;
    }
    
    // Place the scaled box

    switch (selectedGrabber) {
      case TopLeftOutsideCorner:
        setNewPos(points[2].x()-width,points[2].y()-height);
      break;
      case TopRightOutsideCorner:
        setNewPos(points[3].x(),points[3].y()-height);
      break;
      case BottomRightOutsideCorner:
        setNewPos(points[0].x(),points[0].y());
      break;
      default:
        setNewPos(points[1].x()-width,points[1].y());
      break;
    }
    
    // Calculate corners
    
    points[0] = currentPos();
    points[1] = QPointF(currentPos().x() + width,currentPos().y());
    points[2] = currentPos() + QPointF(width,height);
    points[3] = QPointF(currentPos().x(),currentPos().y()+height);
  
    for (int i = 0; i < 4; i++) {
      grabbers[i]->setPos(points[i].x()-grabSize()/2,points[i].y()-grabSize()/2);
    }
    
    // Unscale from last time

    setScale(1.0/oldScale,1.0/oldScale);
    
    // Scale it to the new scale
    
    oldScale = width/size[0];
    setScale(oldScale,oldScale);
    sizeChanged = true;
  }
}


//------------------PixMap-------------------------//

ResizePixmapItem::ResizePixmapItem()
{
  setTransformationMode(Qt::SmoothTransformation);
}

QRectF ResizePixmapItem::currentRect()
{
  return sceneBoundingRect();
}

QPointF ResizePixmapItem::currentPos()
{
  return pos();
}

void ResizePixmapItem::setNewPos(qreal x, qreal y)
{
  setPos(x,y);
}

void ResizePixmapItem::setScale(qreal x, qreal y)
{
  setTransform(QTransform::fromScale(x,y),true);
}

void ResizePixmapItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{     
  position = pos();
  positionChanged = false;
  if (event->button() == Qt::LeftButton){
    placeGrabbers();
  }
  QGraphicsItem::mousePressEvent(event);
} 
  
void ResizePixmapItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{ 
  positionChanged = true;
  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {
    placeGrabbers();
  }
  QGraphicsItem::mouseMoveEvent(event);
}

void ResizePixmapItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseReleaseEvent(event);
  if (event->button() == Qt::LeftButton){
    if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {
      change();
    }
  }
}

QVariant ResizePixmapItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
  if (grabbers[0] && change == ItemSelectedChange) {
    for (int i = 0; i < numGrabbers; i++) {
      grabbers[i]->setVisible(value.toBool());
    }
  }
  return QGraphicsItem::itemChange(change,value);
}

//------------------Text-------------------------//

ResizeTextItem::ResizeTextItem(QGraphicsItem *parent)
: QGraphicsTextItem(parent)
{
}

QRectF ResizeTextItem::currentRect()
{
  return sceneBoundingRect();
}

QPointF ResizeTextItem::currentPos()
{
  return pos();
}

void ResizeTextItem::setNewPos(qreal x, qreal y)
{
  setPos(x,y);
}

void ResizeTextItem::setScale(qreal x, qreal y)
{
  setTransform(QTransform::fromScale(x,y),true);
}

void ResizeTextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  position = pos();
  positionChanged = false;
  if (event->button() == Qt::LeftButton){
    placeGrabbers();
  }
  QGraphicsItem::mousePressEvent(event);
}

void ResizeTextItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  positionChanged = true;
  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {
    placeGrabbers();
  }
  QGraphicsItem::mouseMoveEvent(event);
}

void ResizeTextItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  if (event->button() == Qt::LeftButton){
    if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {
      change();
    }
  }
  QGraphicsItem::mouseReleaseEvent(event);
}

QVariant ResizeTextItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
  if (grabbers[0] && change == ItemSelectedChange) {
    for (int i = 0; i < numGrabbers; i++) {
      grabbers[i]->setVisible(value.toBool());
    }
  }
  return QGraphicsItem::itemChange(change,value);
}

//-----------------------------------------------------------
//-----------------------------------------------------------

InsertPixmapItem::InsertPixmapItem(
  QPixmap    &pixmap,
  InsertMeta meta,
  QGraphicsItem *parent)
  : meta(meta)
{
  setPixmap(pixmap);
  setParentItem(parent);
  
  size[0] = int(pixmap.width() * meta.value().picScale);
  size[1] = int(pixmap.height()* meta.value().picScale);

  setFlag(QGraphicsItem::ItemIsSelectable,true);
  setFlag(QGraphicsItem::ItemIsMovable,true);
  setData(ObjectId, InsertPixmapObj);
  setZValue(INSERTPIXMAP_ZVALUE_DEFAULT);

  margin.setValues(0.0,0.0);
}

void InsertPixmapItem::change()
{
  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {
    if (positionChanged || sizeChanged) {

      beginMacro(QString("Resize"));
      
      InsertData insertData = meta.value();
      
      qreal topLeft[2] = { sceneBoundingRect().left(),  sceneBoundingRect().top() };
      qreal size[2]    = { sceneBoundingRect().width(), sceneBoundingRect().height() };

      PlacementData pld;

      pld.placement    = TopLeft;
      pld.justification    = Center;
      pld.relativeTo      = PageType;
      pld.preposition   = Inside;

      calcOffsets(pld,insertData.offsets,topLeft,size);

      insertData.picScale *= oldScale;
      meta.setValue(insertData);
      
      changeInsertOffset(&meta);
      
      endMacro();
    }
  }
}

void InsertPixmapItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;
  QString pl = "Picture";

  QAction *deleteAction = menu.addAction("Delete this " + pl);
  deleteAction->setWhatsThis("Delete this " + pl.toLower());

  QAction *selectedAction   = menu.exec(event->screenPos());

  if (selectedAction == nullptr) {
    return;
  }

  Where here  = meta.here();

  if (selectedAction == deleteAction) {
      deleteMeta(meta.here());
  }
}
