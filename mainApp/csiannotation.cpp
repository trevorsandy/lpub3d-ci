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

/*********************************************************************
 *
 * This class creates a CSI Annotation icon
 *
 ********************************************************************/

#include <QGraphicsSceneContextMenuEvent>
#include "csiannotation.h"
#include "commonmenus.h"
#include "lpub_object.h"
#include "metaitem.h"
#include "color.h"
#include "step.h"
#include "lpub.h"

PlacementCsiPart::PlacementCsiPart(
    CsiPartMeta   &_csiPartMeta,
    QGraphicsItem *_parent)
{
  relativeType = CsiPartType;
  placement    = _csiPartMeta.placement;
  margin       = _csiPartMeta.margin;
  size[XX]     = _csiPartMeta.size.valuePixels(XX);
  size[YY]     = _csiPartMeta.size.valuePixels(YY);
  loc[XX]     += _csiPartMeta.loc.valuePixels(XX);
  loc[YY]     += _csiPartMeta.loc.valuePixels(YY);
  top          = _csiPartMeta.placement.here();
  bottom       = _csiPartMeta.placement.here();
  outline      = false;

  setData(ObjectId, AssemAnnotationPartObj);
  setZValue(ASSEMANNOTATIONPART_ZVALUE_DEFAULT);
  setParentItem(_parent);
}

bool PlacementCsiPart::hasOffset()
{
    bool zero;
    zero  = placement.value().offsets[XX] == 0.0f;
    zero &= placement.value().offsets[YY] == 0.0f;
    return !zero;
}

void PlacementCsiPart::paint( QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *w)
{
//#ifdef QT_DEBUG_MODE
    if (outline)
        setOutline(painter);
//#endif
    QGraphicsRectItem::paint(painter, o, w);
}

void PlacementCsiPart::toggleOutline()
{
    bool curState = outline;
    outline = ! curState;
    update();
}
void PlacementCsiPart::setOutline(QPainter *painter)
{
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
#else
    painter->setRenderHints(QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing);
#endif
    int ibt = int(1.0f/32.0f);

    /* BORDER */
    QPen borderPen;
    borderPen.setColor(QColor(Qt::blue));       // Qt::transparent
    borderPen.setCapStyle(Qt::SquareCap);
    borderPen.setJoinStyle(Qt::RoundJoin);
    borderPen.setStyle(Qt::DashLine);           // Qt::SolidLine
    borderPen.setWidth(ibt);
    painter->setPen(borderPen);

    /* BACKGROUND */
    painter->setBrush(QColor(Qt::transparent)); // Qt::blue

    QRectF irect(ibt/2,ibt/2,size[XX]-ibt,size[YY]-ibt);
    painter->drawRect(irect);
}

CsiAnnotation::CsiAnnotation(
    const Where       &_here,
    CsiAnnotationMeta &_caMeta)
{
    caMeta        = _caMeta;
    metaLine      = _here;
    partLine      = _here -1;

    if (caMeta.icon.value().hidden)
        return;

    PlacementData pld;

    // set PlacementCsiPart placement
    pld             = csiPartMeta.placement.value();
    pld.offsets[XX] = caMeta.icon.value().partOffset[XX];
    pld.offsets[YY] = caMeta.icon.value().partOffset[YY];
    csiPartMeta.placement.setValue(pld);
    csiPartMeta.size.setValuePixels(XX,caMeta.icon.value().partSize[XX]);
    csiPartMeta.size.setValuePixels(YY,caMeta.icon.value().partSize[YY]);

    // set CsiAnnotation Icon placement
    pld             = caMeta.placement.value();
    pld.offsets[XX] = caMeta.icon.value().iconOffset[XX];
    pld.offsets[YY] = caMeta.icon.value().iconOffset[YY];
    caMeta.placement.setValue(pld);
    setPlacement();

    margin.setValuesInches(0.0f,0.0f);
    placement       = caMeta.placement;
    relativeType    = CsiAnnotationType;
}

bool CsiAnnotation::setPlacement()
{
    QString placement, justification, preposition;
    if (caMeta.icon.value().placements.size() == 2) {
        placement   = placementNames[PlacementEnc(caMeta.icon.value().placements.at(0).toInt())];
        preposition = prepositionNames[PrepositionEnc(caMeta.icon.value().placements.at(1).toInt())];
    }
    else
    if (caMeta.icon.value().placements.size() == 3) {
        placement     = placementNames[PlacementEnc(caMeta.icon.value().placements.at(0).toInt())];
        justification = placementNames[PlacementEnc(caMeta.icon.value().placements.at(1).toInt())];
        preposition   = prepositionNames[PrepositionEnc(caMeta.icon.value().placements.at(2).toInt())];
    }
    if (preposition == "INSIDE" && justification == "CENTER") {
        justification = "";
    }

    int i;
    for (i = 0; i < NumSpots; i++) {
        if (placementOptions[i][0] == placement &&
            placementOptions[i][1] == justification &&
            placementOptions[i][2] == preposition) {
            break;
          }
    }
    if (i == NumSpots) {
        return false;
    }
    RectPlacement  _placementR = RectPlacement(i);;
    PlacementType  _relativeTo = caMeta.placement.value().relativeTo;
    caMeta.placement.setValue(_placementR,_relativeTo);
    return true;
}

bool CsiAnnotation::setCsiPartLoc(int csiSize[])
{
    float partOffset[2] =
    { caMeta.icon.value().partOffset[XX],
      caMeta.icon.value().partOffset[YY]};

    if (partOffset[XX] != 0.0f || partOffset[YY] != 0.0f) {
        csiPartMeta.loc.setValuePixels(XX,int(csiSize[XX] * partOffset[XX]));
        csiPartMeta.loc.setValuePixels(YY,int(csiSize[YY] * partOffset[YY]));
    } else {
        csiPartMeta.loc.setValue(XX,0.0f);
        csiPartMeta.loc.setValue(YY,0.0f);
        return false;
    }
    return true;
}

bool CsiAnnotation::setAnnotationLoc(float iconOffset[])
{
    if (iconOffset[XX] != 0.0f || iconOffset[YY] != 0.0f) {
        loc[XX] += iconOffset[XX];
        loc[YY] += iconOffset[YY];
    } else {
        loc[XX] = 0;
        loc[YY] = 0;
        return false;
    }
    return true;
}

CsiAnnotationItem::CsiAnnotationItem(
   QGraphicsItem  *_parent)
  : ResizeTextItem(_parent)
  , alignment( Qt::AlignCenter | Qt::AlignVCenter )
  , isHovered(false)
  , mouseIsDown(false)
{
   relativeType         = CsiAnnotationType;
   placementCsiPart     = nullptr;

   setAcceptHoverEvents(true);
   setData(ObjectId, AssemAnnotationObj);
   setZValue(ASSEMANNOTATION_ZVALUE_DEFAULT);
}

void CsiAnnotationItem::addGraphicsItems(
   CsiAnnotation        *_ca,
   Step                 *_step,
   PliPart              *_part,
   CsiItem              *_csiItem,
   bool                  _movable)
{
    icon                = _ca->caMeta.icon;
    partLine            = _ca->partLine;
    metaLine            = _ca->metaLine;
    placement           = _ca->placement;
    margin              = _part->styleMeta.margin;
    border              = _part->styleMeta.border;
    background          = _part->styleMeta.background;
    style               = _part->styleMeta.style;
    submodelLevel       = _csiItem->submodelLevel;
    parentRelativeType  = _csiItem->parentRelativeType;
    subModelColor       = _step->pli.pliMeta.subModelColor;
    stepNumber          = _step->stepNumber.number;
    positionChanged     = false;
    switch (parentRelativeType) {
      case CalloutType:
        topOf           = _step->topOfCallout();
        bottomOf        = _step->bottomOfCallout();
        break;
      default:
        topOf           = _step->topOfStep();
        bottomOf        = _step->bottomOfStep();
        break;
    }

    setParentItem(_csiItem);

    QString textString  = _part->text;
    QString fontString  = _part->styleMeta.font.valueFoo();
    QString colorString = _part->styleMeta.color.value();

    QString toolTip = tr("CSI Part Annotation %1 %2 (%3) \"%4\" - right-click to modify")
                         .arg(_part->type, LDrawColor::name(_part->color), _part->color, _part->description);

    setText(textString,fontString,toolTip);

    QColor color(colorString);
    setDefaultTextColor(color);

    textRect  = QRectF(0,0,document()->size().width(),document()->size().height());

    if (style.value() == AnnotationStyle::none) {
        styleRect = textRect;
    } else {
        // set rectangle size and dimensions parameters
        bool fixedStyle  = style.value() != AnnotationStyle::rectangle;
        bool isRectangle = style.value() == AnnotationStyle::rectangle;
        UnitsMeta rectSize;
        if (isRectangle) {
            if ((_part->styleMeta.size.valueInches(XX) > STYLE_SIZE_DEFAULT  ||
                 _part->styleMeta.size.valueInches(XX) < STYLE_SIZE_DEFAULT) ||
                (_part->styleMeta.size.valueInches(YY) > STYLE_SIZE_DEFAULT  ||
                 _part->styleMeta.size.valueInches(YY) < STYLE_SIZE_DEFAULT)) {
                rectSize = _part->styleMeta.size;
            } else {
                int widthInPx  = int(textRect.width());
                int heightInPx = int(textRect.height());
                rectSize.setValuePixels(XX,widthInPx);
                rectSize.setValuePixels(YY,heightInPx);
            }
        }
        QRectF _styleRect = QRectF(0,0,fixedStyle ? _part->styleMeta.size.valuePixels(XX) : isRectangle ? rectSize.valuePixels(XX) : textRect.width(),
                                       fixedStyle ? _part->styleMeta.size.valuePixels(YY) : isRectangle ? rectSize.valuePixels(YY) : textRect.height());
        styleRect = boundingRect().adjusted(0,0,_styleRect.width()-textRect.width(),_styleRect.height()-textRect.height());

        // scale down the font as needed
        scaleDownFont();

        // center document text in style size
        setTextWidth(-1);
        setTextWidth(styleRect.width());
        QTextBlockFormat format;
        format.setAlignment(alignment);
        QTextCursor cursor = textCursor();
        cursor.select(QTextCursor::Document);
        cursor.mergeBlockFormat(format);
        cursor.clearSelection();
        setTextCursor(cursor);

        // adjust text horizontal alignment
        textOffset.setX(border.valueInches().thickness/2);
        // adjust text vertical alignment
        textOffset.setY((styleRect.height()-textRect.height())/2);
    }
    size[XX] = int(styleRect.size().width());
    size[YY] = int(styleRect.size().height());

    sizeIt();

    // set PlacementCsiPart location based on csi size

    _ca->setCsiPartLoc(_csiItem->size);

    // place PlacementCsiPart relative to CSI

    placementCsiPart = new PlacementCsiPart(_ca->csiPartMeta,_csiItem);
    placementCsiPart->top = topOf;
    placementCsiPart->top = bottomOf;
    placementCsiPart->stepNumber = stepNumber;
    if (! placementCsiPart->hasOffset())
        _csiItem->placeRelative(placementCsiPart);

    placementCsiPart->setPos(placementCsiPart->loc[XX],
                             placementCsiPart->loc[YY]);

    // place CsiAnnotation Icon relative to PlacementCsiPart

    bool hasLoc = _ca->setAnnotationLoc(placement.value().offsets);
    if (hasLoc) {
        loc[XX] = _ca->loc[XX];
        loc[YY] = _ca->loc[YY];
    } else {
        placementCsiPart->placeRelative(this);
        _ca->assign(this);
    }
    setPos(loc[XX],loc[YY]);

    setData(ObjectId, AssemAnnotationObj);
    setZValue(ASSEMANNOTATION_ZVALUE_DEFAULT);

    setFlag(QGraphicsItem::ItemIsMovable, _movable);
    setFlag(QGraphicsItem::ItemIsSelectable, _movable);
}

void CsiAnnotationItem::scaleDownFont() {
  qreal widthRatio  = styleRect.width()  / textRect.width();
  qreal heightRatio = styleRect.height() / textRect.height();
  if (widthRatio < 1 || heightRatio < 1) {
    QFont font = this->QGraphicsTextItem::font();
    qreal saveFontSizeF = font.pointSizeF();
    font.setPointSizeF(font.pointSizeF()*qMin(widthRatio,heightRatio));
    setFont(font);
    textRect = QRectF(0,0,document()->size().width(),document()->size().height());

    if (textRect.width() > styleRect.width() || textRect.height() > styleRect.height()) {
      scaleDownFont();
    }

    emit gui->messageSig(LOG_INFO,QMessageBox::tr("CSI annotation font size was adjusted from %1 to %2.")
                                                  .arg(saveFontSizeF).arg(font.pointSizeF()));
  }
}

void CsiAnnotationItem::sizeIt()
{
    size[XX] += int(border.valuePixels().margin[XX]);
    size[YY] += int(border.valuePixels().margin[YY]);
    size[XX] += int(border.valuePixels().thickness);
    size[YY] += int(border.valuePixels().thickness);
}

void CsiAnnotationItem::setAnnotationStyle(QPainter *painter)
{
    // set painter and render hints
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
#else
    painter->setRenderHints(QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing);
#endif
    // set the background then set the border and paint both in one go.

    /* BACKGROUND */
    QColor brushColor;
    BackgroundData backgroundData = background.value();

    switch(backgroundData.type) {
    case BackgroundData::BgColor:
        brushColor = LDrawColor::color(backgroundData.string);
        break;
    case BackgroundData::BgSubmodelColor:
        brushColor = LDrawColor::color(subModelColor.value(0));
        break;
    default:
        brushColor = Qt::transparent;
        break;
    }
    painter->setBrush(brushColor);

    /* BORDER */
    QPen borderPen;
    QColor borderPenColor;
    BorderData borderData = border.valuePixels();
    if (borderData.type == BorderData::BdrNone) {
        borderPenColor = Qt::transparent;
    } else {
        borderPenColor =  LDrawColor::color(borderData.color);
    }
    borderPen.setColor(borderPenColor);
    borderPen.setCapStyle(Qt::RoundCap);
    borderPen.setJoinStyle(Qt::RoundJoin);
    if (borderData.line == BorderData::BdrLnNone) {
          borderPen.setStyle(Qt::NoPen);
    }
    else if (borderData.line == BorderData::BdrLnSolid) {
        borderPen.setStyle(Qt::SolidLine);
    }
    else if (borderData.line == BorderData::BdrLnDash) {
        borderPen.setStyle(Qt::DashLine);
    }
    else if (borderData.line == BorderData::BdrLnDot) {
        borderPen.setStyle(Qt::DotLine);
    }
    else if (borderData.line == BorderData::BdrLnDashDot) {
        borderPen.setStyle(Qt::DashDotLine);
    }
    else if (borderData.line == BorderData::BdrLnDashDotDot) {
        borderPen.setStyle(Qt::DashDotDotLine);
    }
     borderPen.setWidth(int(borderData.thickness));

    painter->setPen(borderPen);

    // draw icon shape - background and border
    int bt = int(borderData.thickness);
    QRectF bgRect(bt/2,bt/2,size[XX]-bt,size[YY]-bt);
    if (style.value() != AnnotationStyle::circle) {
        if (borderData.type == BorderData::BdrRound) {
            // set icon border dimensions
            qreal rx = double(borderData.radius);
            qreal ry = double(borderData.radius);
            qreal dx = size[XX];
            qreal dy = size[YY];
            if (int(dx) && int(dy)) {
                if (dx > dy) {
                    rx *= dy;
                    rx /= dx;
                } else {
                    ry *= dx;
                    ry /= dy;
                }
            }
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
            painter->drawRoundedRect(bgRect,int(rx),int(ry));
#else
            painter->drawRoundRect(bgRect,int(rx),int(ry));
#endif
        } else {
            painter->drawRect(bgRect);
        }
    } else {
        painter->drawEllipse(bgRect);
    }
}

void CsiAnnotationItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *w)
{
    if (style.value() != AnnotationStyle::none) {
        setAnnotationStyle(painter);
        QRectF textBounds = boundingRect();
        textBounds.translate(textOffset);
        painter->translate(textBounds.left(), textBounds.top());
    }
    QPen pen;
    pen.setColor(isHovered ? QColor(Preferences::sceneGuideColor) : Qt::black);
    pen.setWidth(0/*cosmetic*/);
    pen.setStyle(isHovered ? Qt::PenStyle(Preferences::sceneGuidesLine) : Qt::NoPen);
    painter->setPen(pen);
    painter->setBrush(Qt::transparent);
    painter->drawRect(this->boundingRect());
    QGraphicsTextItem::paint(painter, o, w);
}

void CsiAnnotationItem::change() {
    updateCsiAnnotationIconMeta(metaLine, &icon);
}

void CsiAnnotationItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = !this->isSelected() && !mouseIsDown;
    QGraphicsItem::hoverEnterEvent(event);
}

void CsiAnnotationItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = false;
    QGraphicsItem::hoverLeaveEvent(event);
}

void CsiAnnotationItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  mouseIsDown = true;
  position = pos();
  positionChanged = false;
  QGraphicsItem::mousePressEvent(event);
  //placeGrabbers();
}

void CsiAnnotationItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  positionChanged = true;
  QGraphicsItem::mouseMoveEvent(event);
  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {
      //placeGrabbers();
  }
}

void CsiAnnotationItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    mouseIsDown = false;
    QGraphicsItem::mouseReleaseEvent(event);

    if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {

        // back annotate the movement of the PLI into the LDraw file.

        if (positionChanged) {

            beginMacro(QString("DragCsiAnnotation"));

            QPointF newPosition;
            newPosition = pos() - position;

            if (newPosition.x() || newPosition.y()) {

                positionChanged = true;

                PlacementData placementData    = placement.value();
                if (relativeToSize[XX] > 1 || relativeToSize[YY] > 1) {
                    placementData.offsets[XX] += newPosition.x()+loc[XX];
                    placementData.offsets[YY] += newPosition.y()+loc[YY];
                } else {
                    placementData.offsets[XX] += newPosition.x()/relativeToSize[XX];
                    placementData.offsets[YY] += newPosition.y()/relativeToSize[YY];
                }
                placement.setValue(placementData);

                CsiAnnotationIconData caiData = icon.value();
                caiData.iconOffset[XX]        = placementData.offsets[XX];
                caiData.iconOffset[YY]        = placementData.offsets[YY];
                icon.setValue(caiData);

                change();
            }
            endMacro();
        }
    }
}

void CsiAnnotationItem::contextMenuEvent(
    QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;
  const QString name = tr("CSI Part Annotation");

  QAction *placementAction    = lpub->getAct("placementAction.1");
  PlacementData placementData = placement.value();
  placementAction->setWhatsThis(commonMenus.naturalLanguagePlacementWhatsThis(CsiAnnotationType,placementData,name));
  commonMenus.addAction(placementAction,menu,name);

  QAction *hideAction         = lpub->getAct("hideCsiAnnotationAction.1");
  commonMenus.addAction(hideAction,menu,name);

  QAction *toggleCsiPartRectAction = lpub->getAct("toggleCsiPartRectAction.1");
  commonMenus.addAction(toggleCsiPartRectAction,menu);

  QAction *selectedAction   = menu.exec(event->screenPos());

  if (selectedAction == nullptr) {
      return;
  }
  else if (selectedAction == toggleCsiPartRectAction) {
      placementCsiPart->toggleOutline();
      gui->pagescene()->update();
  } else if (selectedAction == placementAction) {
      changeCsiAnnotationPlacement(
              parentRelativeType,
              CsiAnnotationType,
              tr("%1 Placement").arg(name),
              metaLine,
              metaLine,
             &placement,
             &icon,true,1,false);
   } else if (selectedAction == hideAction) {
              CsiAnnotationIconData caid = icon.value();
              caid.hidden = true;
              icon.setValue(caid);
              updateCsiAnnotationIconMeta(metaLine, &icon);
   }
}
