 
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

#include "textitem.h"
#include <QMenu>
#include <QAction>
#include <QGraphicsSceneContextMenuEvent>
#include <QFontDialog>
#include <QColor>
#include <QColorDialog>
#include "commonmenus.h"
#include "pagebackgrounditem.h"
#include "lpub.h"
#include "placementdialog.h"
#include "pairdialog.h"

TextItem::TextItem(InsertMeta meta,
  PageTypeEnc pageType,
  bool placement,
  QGraphicsItem *parent)
    : meta(meta),
      pageType( pageType),
      pagePlaced( false),
      textPlacement(placement),
      textChanged(false),
      isHovered(  false),
      mouseIsDown(false)
{
  if (meta.value().placementCommand) {
      QString line = gui->readLine(meta.here());
      emit gui->messageSig(LOG_ERROR, tr("Text placement command must come after an 'Add Text' command.<br>Line: %1")
                           .arg(QString("%1 %2%3").arg(meta.here().lineNumber).arg(meta.here().modelName, line.isEmpty() ? "" : line)));
      return;
  }

  InsertData data    = meta.value();
  richText           = data.type == InsertData::InsertRichText;
  setParentItem(parent);

  QString fontString = data.textFont;
  if (fontString.length() == 0) {
    fontString = "Arial,24,-1,255,75,0,0,0,0,0";
  }

  QFont font;
  font.fromString(fontString);
  setFont(font);

  QColor color(data.textColor);
  setDefaultTextColor(color);

  // unformat text - remove quote escapte
  QChar esc('\\');
  QStringList text;

  static QRegularExpression rx("\"");
  QRegularExpressionMatch match;
  QRegularExpressionMatchIterator matchIt;
  const QStringList goodsList = data.text.split("\\n");
  for (const QString &item : goodsList) {
    QString string = item.trimmed();
    int pos = 0;
    int adj = 0;
    matchIt = rx.globalMatch(string);
    while(matchIt.hasNext())
    {
      match = matchIt.next();
      pos = match.capturedStart() + adj;
      if (pos < string.size()) {
        QChar ch = string.at(pos-1);
        if (ch == esc) {
          string.remove(pos-1,1);
          adj--;
        }
      }
    }
    // if last character is esc \, append space ' ' so not to escape closing string double quote
    if (string.at(string.size()-1) == esc)
      string.append(QChar(' '));
    text << string;
  }

  if (richText)
    setHtml(text.join("\n"));
  else
    setPlainText(text.join("\n"));

  margin.setValues(0.0,0.0);

  setToolTip(tr("%1 [%2 x %3 px] - right-click to modify")
             .arg(richText ? tr("Rich Text") : tr("Text"))
             .arg(boundingRect().width())
             .arg(boundingRect().height()));

  setTextInteractionFlags(Qt::TextEditorInteraction);
  setAcceptHoverEvents(true);
  setFlag(QGraphicsItem::ItemIsMovable);
  setFlag(QGraphicsItem::ItemIsSelectable);
  setFlag(QGraphicsItem::ItemIsFocusable, true);
  setData(ObjectId, InsertTextObj);
  setZValue(INSERTTEXT_ZVALUE_DEFAULT);
}

void TextItem::formatText(const QString &input, QString &output)
{
    QChar esc('\\');
    QStringList text;

    static QRegularExpression rx("\"");
    QRegularExpressionMatch match;
    QRegularExpressionMatchIterator matchIt;
    const QStringList textList = input.split("\n");
    for (const QString &item : textList) {
      QString string = item.trimmed();
      int pos = 0;
      int adj = 0;
      matchIt = rx.globalMatch(string);
      while(matchIt.hasNext()) {
        match = matchIt.next();
        pos = match.capturedStart() + adj;
        if (pos < string.size()) {
          QChar ch = string.at(pos);
          if (ch != esc) {
            string.insert(pos,&esc,1);
            adj++;
          }
        }
      }
      // if last character is esc \, append space ' ' so not to escape closing string double quote
      if (string.at(string.size()-1) == esc)
        string.append(QChar(' '));
      text << string;
    }

   output = text.join("\\n");
}

void TextItem::contextMenuEvent(
  QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;
  const QString name = tr("Text");

  InsertData data = meta.value();

  QAction *editTextAction  = lpub->getAct("textAction.1");
  commonMenus.addAction(editTextAction,menu,name);

  QAction *placementAction = nullptr;
  if (textPlacement) {
      placementAction      = lpub->getAct("placementAction.1");
      PlacementData placementData = placement.value();
      placementAction->setWhatsThis(commonMenus.naturalLanguagePlacementWhatsThis(TextType,placementData,name));
      commonMenus.addAction(placementAction,menu,name);

      data.offsets[XX] = 0.0f;
      data.offsets[YY] = 0.0f;
  }

  QAction *fontAction       = nullptr;
  QAction *colorAction      = nullptr;
  if (!richText) {
    fontAction              = lpub->getAct("fontAction.1");
    commonMenus.addAction(fontAction,menu,name);

    colorAction             = lpub->getAct("colorAction.1");
    commonMenus.addAction(colorAction,menu,name);
  }

  QAction *deleteTextAction = lpub->getAct("deleteTextAction.1");
  commonMenus.addAction(deleteTextAction,menu,name);

  QAction *selectedAction   = menu.exec(event->screenPos());

  if (selectedAction == nullptr) {
    return;
  }

  if (selectedAction == editTextAction) {

    updateText(meta.here(),
               data.text,
               data.textFont,
               data.textColor,
               data.offsets[XX],
               data.offsets[YY],
               parentRelativeType,
               richText,
               false/*append*/);

  } else if (selectedAction == fontAction) {

    FontMeta font;
    font.setValuePoints(data.textFont);

    QFont _font;
    QString fontName = font.valueFoo();
    _font.fromString(fontName);

    bool ok;

    _font = QFontDialog::getFont(&ok,_font);

    if (ok) {

        data.textFont = _font.toString();
        meta.setValue(data);

        beginMacro("UpdateFont");
        replaceMeta(meta.here(),meta.format(false,false));
        endMacro();

    } else {
        Gui::displayPage();
    }

  } else if (selectedAction == colorAction) {

    QColor color(data.textColor);

    color = QColorDialog::getColor(color);

    if (color.isValid())
        data.textColor = color.name();

    meta.setValue(data);
    beginMacro("UpdateColor");
    replaceMeta(meta.here(),meta.format(false,false));
    endMacro();

  } else if (selectedAction == placementAction) {
      placement.preamble = QString("0 !LPUB INSERT %1 PLACEMENT ")
                                   .arg(richText ? "RICH_TEXT" : "TEXT");
      PlacementData placementData = placement.value();
      placementData.pageType = pageType;
      bool ok;
      ok = PlacementDialog
           ::getPlacement(
                  parentRelativeType,
                  relativeType,
                  placementData,
                  tr("%1 Placement").arg(name));
      if (ok) {
        placement.setValue(placementData);
        QString line = gui->readLine(meta.here());
        if (line.contains(placement.preamble)) {
           line = placement.format(true,meta.global);
           replaceMeta(meta.here(),line);
        } else {
           Where walkFwd = meta.here() + 1;
           line = gui->readLine(walkFwd);
           if (line.contains(placement.preamble)) {
              line = placement.format(true,meta.global);
              replaceMeta(walkFwd,line);
           } else {
              bool local = LocalDialog::getLocal(VER_PRODUCTNAME_STR, tr("Change only this step?"),nullptr);
              line = placement.format(local,false);
              insertMeta(walkFwd,line);
           }
        }
      }
  } else if (selectedAction == deleteTextAction) {
    Where walkFwd = meta.here() + 1;
    QString placement = QString("0 !LPUB INSERT %1 PLACEMENT ")
                                .arg(richText ? "RICH_TEXT" : "TEXT");
    QString line = gui->readLine(walkFwd);
    beginMacro("DeleteText");
    if (line.contains(placement))
        deleteMeta(walkFwd);
    deleteMeta(meta.here());
    endMacro();
  }
}

void TextItem::focusInEvent(QFocusEvent *event)
{
  textChanged = false;
  QGraphicsTextItem::focusInEvent(event);
}

void TextItem::focusOutEvent(QFocusEvent *event)
{
  QGraphicsTextItem::focusOutEvent(event);
  if (textChanged) {
    InsertData insertData = meta.value();
    // remove offset from insertData if textPlacement enabled
    if (textPlacement) {
        insertData.offsets[XX] = 0.0f;
        insertData.offsets[YY] = 0.0f;
    }

    QString input,output;
    if (richText)
       input = toHtml();
    else
       input = toPlainText();

    formatText(input, output);

    insertData.text = output;
    meta.setValue(insertData);

    beginMacro(QString("EditText"));
    changeInsertOffset(&meta);
    endMacro();
  }
}

void TextItem::keyPressEvent(QKeyEvent *event)
{
  textChanged = true;
  QGraphicsTextItem::keyPressEvent(event);
}

void TextItem::keyReleaseEvent(QKeyEvent *event)
{
  textChanged = true;
  QGraphicsTextItem::keyReleaseEvent(event);
}

void TextItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  positionChanged = true;
  QGraphicsItem::mouseMoveEvent(event);
}

void TextItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = !this->isSelected() && !mouseIsDown;
    QGraphicsItem::hoverEnterEvent(event);
}

void TextItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = false;
    QGraphicsItem::hoverLeaveEvent(event);
}

void TextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  position = pos();
  positionChanged = false;
  mouseIsDown = true;
  QGraphicsItem::mousePressEvent(event);
//  update();
}

void TextItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  mouseIsDown = false;
  QGraphicsItem::mouseReleaseEvent(event);

  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable) && positionChanged) {

    InsertData insertData = meta.value();

    qreal topLeft[2] = { sceneBoundingRect().left(),  sceneBoundingRect().top() };
    qreal size[2]    = { sceneBoundingRect().width(), sceneBoundingRect().height() };

    if (textPlacement)
        placement.preamble = QString("0 !LPUB INSERT %1 PLACEMENT ")
                                     .arg(richText ? "RICH_TEXT" : "TEXT");

    PlacementData pld = placement.value();

    calcOffsets(pld,insertData.offsets,topLeft,size);

    if (textPlacement) {
        // apply offset to placementData
        pld.offsets[XX] = insertData.offsets[XX];
        pld.offsets[YY] = insertData.offsets[YY];

        placement.setValue(pld);

        bool canReplace = false;
        Where walk = meta.here();
        QString line = gui->readLine(walk);
        if ((canReplace = line.contains(placement.preamble))) {
           line = placement.format(true,meta.global);
        } else {
           walk++;
           line = gui->readLine(walk);
           if ((canReplace = line.contains(placement.preamble))) {
              line = placement.format(true,meta.global);
           }
        }
        if (canReplace) {
          beginMacro(QString("MoveTextPlacement"));

          replaceMeta(walk,line);

          endMacro();
        }
    } else {
        QString input,output;
        if (richText)
           input = toHtml();
        else
           input = toPlainText();

        formatText(input, output);

        insertData.text = output;
        meta.setValue(insertData);

        beginMacro(QString("MoveText"));

        changeInsertOffset(&meta);

        endMacro();
    }
  }
}

void TextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  QPen pen;
  pen.setColor(isHovered ? QColor(Preferences::sceneGuideColor) : Qt::black);
  pen.setWidth(0/*cosmetic*/);
  pen.setStyle(isHovered ? Qt::PenStyle(Preferences::sceneGuidesLine) : Qt::NoPen);
  painter->setPen(pen);
  painter->setBrush(Qt::transparent);
  painter->drawRect(this->boundingRect());
  QGraphicsTextItem::paint(painter,option,widget);
}
