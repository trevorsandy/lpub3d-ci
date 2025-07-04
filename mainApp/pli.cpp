
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
 * This class implements part list images.  It gathers and maintains a list
 * of part/colors that need to be displayed.  It provides mechanisms for
 * rendering the parts.  It provides methods for organizing the parts into
 * a reasonable looking box for display in your building instructions.
 *
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/
#include <QMenu>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsRectItem>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>

#include "lpub.h"
#include "pli.h"
#include "step.h"
#include "ranges.h"
#include "callout.h"
#include "resolution.h"
#include "render.h"
#include "paths.h"
#include "ldrawfiles.h"
#include "placementdialog.h"
#include "metaitem.h"
#include "color.h"
#include "commonmenus.h"
#include "lpub_preferences.h"
#include "ranges_element.h"
#include "range_element.h"
#include "dependencies.h"

#include "pieceinf.h"
#include "lc_viewwidget.h"
#include "lc_library.h"
#include "lc_previewwidget.h"

QCache<QString,QString> Pli::orientation;

QString PartTypeNames[NUM_PART_TYPES] = { QObject::tr("Fade Steps"), QObject::tr("Highlight Step"), QObject::tr("Normal") };

const Where &Pli::topOfStep()
{
  if (bom || step == nullptr || steps == nullptr) {
      return Gui::topOfPage();
    } else {
      if (step) {
          return step->topOfStep();
        } else {
          return steps->topOfSteps();
        }
    }
}
const Where &Pli::bottomOfStep()
{
  if (bom || step == nullptr || steps == nullptr) {
      return Gui::bottomOfPage();
    } else {
      if (step) {
          return step->bottomOfStep();
        } else {
          return steps->bottomOfSteps();
        }
    }
}
const Where &Pli::topOfSteps()
{
  return steps->topOfSteps();
}
const Where &Pli::bottomOfSteps()
{
  return steps->bottomOfSteps();
}
const Where &Pli::topOfCallout()
{
  return step->callout()->topOfCallout();
}
const Where &Pli::bottomOfCallout()
{
  return step->callout()->bottomOfCallout();
}

Pli::Pli(bool _bom) : bom(_bom)
{
  relativeType = PartsListType;
  initAnnotationString();
  steps = nullptr;
  step = nullptr;
  meta = nullptr;
  widestPart = 1;
  tallestPart = 1;
  background = nullptr;
  splitBom = false;
  perStep = false;

  ptn.append( { FADE_PART, FADE_SFX } );
  ptn.append( { HIGHLIGHT_PART, HIGHLIGHT_SFX } );
  ptn.append( { NORMAL_PART, QString() } );
  ia.sub[FADE_PART] = 0;
  ia.sub[HIGHLIGHT_PART] = 0;
  ia.sub[NORMAL_PART] = 0;

  isSubModel = false;
  fadeSteps = false;
  highlightStep = false;
  displayIcons = false;
  isSubModel = false;
  multistep = false;
  callout = false;
}

/****************************************************************************
 * Part List Images routines
 ***************************************************************************/

PliPart::~PliPart()
{
  instances.clear();
  leftEdge.clear();
  rightEdge.clear();
}

float PliPart::maxMargin()
{
  float margin1 = qMax(instanceMeta.margin.valuePixels(XX),
                       csiMargin.valuePixels(XX));

  // Use style margin
  if (styleMeta.style.value() != AnnotationStyle::none) {
      float margin2 = styleMeta.margin.valuePixels(XX);
      margin1 = qMax(margin1,margin2);
  }

  if (annotWidth) {
      float margin2 = styleMeta.margin.valuePixels(XX);
      margin1 = qMax(margin1,margin2);
    }

  return margin1;
}

void PliPart::addPartGroupToScene(
        LGraphicsScene *scene,
        Where &top,
        Where &bottom,
        int stepNumber)
{
    // create the part group item
    pliPartGroup = new PartGroupItem(groupMeta);
    pliPartGroup->top = top;
    pliPartGroup->bottom = bottom;
    pliPartGroup->stepNumber = stepNumber;

    // add the part group to the scene
    scene->addItem(pliPartGroup);

    // add part items to the group
    if (pixmap)
        pliPartGroup->addToGroup(pixmap);
    if (instanceText)
        pliPartGroup->addToGroup(instanceText);
    if (annotateText)
        pliPartGroup->addToGroup(annotateText);
    if (annotateElement)
        pliPartGroup->addToGroup(annotateElement);

/* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
    qDebug() << "\n"
             << "02/06 PLI PART GROUP ATTRIBUTES [" + groupMeta.value().type + "_" + groupMeta.value().color + "] - ADD TO SCENE"
             << "\n0. BOM:        " <<(groupMeta.value().bom ? "True" : "False")
             << "\n0. Bom Part:   " <<(groupMeta.value().bom ? groupMeta.value().bPart ? "Yes" : "No" : "N/A")
             << "\n1. Type:       " << groupMeta.value().type
             << "\n2. Color:      " << groupMeta.value().color
             << "\n3. ZValue:     " << groupMeta.value().zValue
             << "\n4. OffsetX:    " << groupMeta.value().offset[0]
             << "\n5. OffsetY:    " << groupMeta.value().offset[1]
             << "\n6. Group Model:" << groupMeta.value().group.modelName
             << "\n7. Group Line: " << groupMeta.value().group.lineNumber
             << "\n8. Meta Model: " << groupMeta.here().modelName
             << "\n9. Meta Line:  " << groupMeta.here().lineNumber
             ;
#endif
//*/

    // check if we have offset
    if (groupMeta.offset().x() == 0.0 && groupMeta.offset().y() == 0.0)
        return;

    // transform offset
    QTransform transform;
    transform.translate(groupMeta.offset().x(),groupMeta.offset().y());
    pliPartGroup->setTransform(transform);
    scene->update();
}

/****************************************************************************
 * Part List routines
 ***************************************************************************/

int Pli::pageSizeP(Meta *meta, int which) {
  int _size;

  // flip orientation for landscape
  if (meta->LPub.page.orientation.value() == Landscape) {
      which == 0 ? _size = 1 : _size = 0;
    } else {
      _size = which;
    }
  return meta->LPub.page.size.valuePixels(_size);
}

QString Pli::partLine(QString &line, Where &here, Meta &meta)
{
  QString attributes = QString(";%1;%2").arg(here.modelName).arg(here.lineNumber);
  // substitute part routine
  if (meta.LPub.pli.begin.sub.value().type) {
      SubData subData      = meta.LPub.pli.begin.sub.value();
      QStringList attrArgs = subData.attrs.split(";");
      // check if substitute type is not 0 and substitute lineNumber matches here.lineNumber (this line)
      if (subData.type && attrArgs.last().toInt() == here.lineNumber) {
          // remove substitues line number from substitute attributes
          attrArgs.removeLast();
          // capture first attribute from attrArgs(subOriginalType)
          QString subOriginalType = attrArgs.first();
          // then cut the first attribute from attrArgs
          attrArgs.removeFirst();
          // append substitute type and attributes, if any, to attributes - used by Pli::setParts()
          attributes.append(QString("|%1%2").arg(subData.type).arg(attrArgs.size() ? QString("|%1").arg(attrArgs.join(";")) : ""));
          // place subOriginalType at the end of attributes
          attributes.append(QString("|%1").arg(subOriginalType));
      }
  }
  return line + attributes;
}

void Pli::setParts(
    QStringList             &pliParts,
    QList<PliPartGroupMeta> &partGroups,
    Meta                    &meta,
    bool                    _bom,
    bool                    _split)
{
  bom      = _bom;
  splitBom = _split;
  pliMeta  = _bom ? meta.LPub.bom : meta.LPub.pli;

  bool displayAnnotation = pliMeta.annotation.display.value();
  bool enableStyle       = pliMeta.annotation.enableStyle.value();
  bool displayElement    = pliMeta.partElements.display.value();
  bool extendedStyle     = pliMeta.annotation.extendedStyle.value();
  bool fixedAnnotations  = pliMeta.annotation.fixedAnnotations.value();

  // setup Visual Editor entry
  switch (parentRelativeType) {
  case CalloutType:
      top     = topOfCallout();
      bottom  = bottomOfCallout();
      callout = true;
      break;
  default:
      if (step) {
          if (bom) {
              top    = topOfSteps();
              bottom = bottomOfSteps();
          } else {
              top    = topOfStep();
              bottom = bottomOfStep();
          }
      } else {
          top    = topOfSteps();
          bottom = bottomOfSteps();
      }
      multistep = parentRelativeType == StepGroupType;
      break;
  }

  // get bom part group last line
  Where where;
  if (bom && pliMeta.enablePliPartGroup.value()) {
      if (partGroups.size()) {
          where = partGroups.last().here();
      } else {
          Page *page = dynamic_cast<Page *>(steps);
          int nInserts = page->inserts.size();
          for (int i = 0; i < nInserts; i++) {
              if (page->inserts[i].value().type == InsertData::InsertBom) {
                  where = page->inserts[i].here();
                  break;
              }
          }
      }
  }

  for (int i = 0; i < pliParts.size(); i++) {
      QStringList segments = pliParts[i].split("|");
      QString part = segments.at(0);
      QStringList sections = part.split(";");
      QString line = sections[0];
      Where here(sections[1],lpub->ldrawFile.getSubmodelIndex(sections[1]),sections[2].toInt());

      QStringList tokens;

      split(line,tokens);

      if (tokens.size() == 15 && tokens[0] == "1") {
          QString &color = tokens[1];
          QString &type = tokens[14];

          QFileInfo info(type);

          QString baseName = info.completeBaseName();

          QString key = QString("%1_%2").arg(baseName, color);

          // extract the substitute original or ldraw type
          bool isSubstitute       = segments.size() > 2;
          bool validOriginalType  = isSubstitute && segments.last() != "undefined";
          QString subOriginalType = validOriginalType ? segments.last() : QString();
          bool isSubLdrawType     = validOriginalType ? QStringList(subOriginalType.split(":")).last().toInt() : false;

          if (lpub->ldrawFile.isMissingItem(type)) {
              emit lpub->messageSig(LOG_WARNING, QObject::tr("%1 [%2] was not found! See Load Status for details")
                                                             .arg(isSubstitute ? QObject::tr("Substitute part") : QObject::tr("Part"), type));
              continue;
          }

          QString description = titleDescription(type);

          QString sortCategory;
          partClass(sortCategory,description);  // populate sort category using part class and

          // initialize default style settings
          AnnotationStyleMeta styleMeta = pliMeta.defaultStyle;
          styleMeta.margin = pliMeta.annotate.margin;
          styleMeta.font   = pliMeta.annotate.font;
          styleMeta.color  = pliMeta.annotate.color;

          // initialize element id
          QString element = QString();

          // if display annotations is enabled
          if (displayAnnotation) {

              // populate part element id
              if (bom && displayElement) {

                  QString _colorid = color;
                  QString _typeid  = QFileInfo(isSubLdrawType ?
                                                   QStringList(subOriginalType.split(":")).first() : type).completeBaseName();

                  int which = 0; // Bricklink
                  if ( pliMeta.partElements.legoElements.value())
                      which = 1; // LEGO
                  // kept localElements for backwards compatability
                  if (pliMeta.partElements.userElements.value() || pliMeta.partElements.localElements.value() ) {
                      bool useLDrawKey = pliMeta.partElements.userElementsLDrawKey.value();
                      element = Annotations::getUserElement(_typeid.toLower(),_colorid.toLower(),useLDrawKey);
                  }
                  else
                  {
                      if (!Annotations::loadBLCodes()) {
                          QString URL(VER_LPUB3D_BLCODES_DOWNLOAD_URL);
                          lpub->downloadFile(URL, "BrickLink Elements");
                          QByteArray Buffer = lpub->getDownloadedFile();
                          Annotations::loadBLCodes(Buffer);
                      }
                      element = Annotations::getBLElement(_colorid.toLower(),_typeid.toLower(),which);
                  }
              }

              // if annotation style is enabled
              if (enableStyle) {

                  // if fixed Annotations is enabled
                  if (fixedAnnotations) {

                      // get part annotation style flag for fixed annotations - cirle(1), square(2), or rectangle(3)
                      AnnotationStyle fixedStyle = AnnotationStyle(Annotations::getAnnotationStyle(type));

                      // set style meta settings
                      if (fixedStyle) {
                          // get style category
                          bool styleCategory = false;
                          AnnotationCategory annotationCategory = AnnotationCategory(Annotations::getAnnotationCategory(type));
                          switch (annotationCategory)
                          {
                          case AnnotationCategory::axle:
                              styleCategory = pliMeta.annotation.axleStyle.value();
                              break;
                          case AnnotationCategory::beam:
                              styleCategory = pliMeta.annotation.beamStyle.value();
                              break;
                          case AnnotationCategory::cable:
                              styleCategory = pliMeta.annotation.cableStyle.value();
                              break;
                          case AnnotationCategory::connector:
                              styleCategory = pliMeta.annotation.connectorStyle.value();
                              break;
                          case AnnotationCategory::hose:
                              styleCategory = pliMeta.annotation.hoseStyle.value();
                              break;
                          case AnnotationCategory::panel:
                              styleCategory = pliMeta.annotation.panelStyle.value();
                              break;
                          default:
                              break;
                          }
                          // set style if category enabled
                          if (styleCategory) {
                              if (fixedStyle == AnnotationStyle::circle)
                                  styleMeta       = pliMeta.circleStyle;
                              else
                              if (fixedStyle == AnnotationStyle::square)
                                  styleMeta       = pliMeta.squareStyle;
                              else
                              if (fixedStyle == AnnotationStyle::rectangle)
                                  styleMeta       = pliMeta.rectangleStyle;
                          }
                      // if extended style annotation is enabled
                      } else if (extendedStyle) {
                          styleMeta = pliMeta.rectangleStyle;
                      }
                  // if extended style annotation is enabled
                  } else if (extendedStyle) {
                      styleMeta = pliMeta.rectangleStyle;
                  }
              }
          }

          bool found                 = false;
          PliPartGroupMeta groupMeta = pliMeta.pliPartGroup;
          if (!bom || where.lineNumber == 0)
              where = here;

          auto getGroupMeta = [this,&partGroups,&where,&baseName,
                               &found,&key,&color,&groupMeta]( )
          {
              if (!pliMeta.enablePliPartGroup.value())
                  return groupMeta;

              PliPartGroupData _gd;;

              // check if exists
              for (auto &_gm : partGroups) {
                  if (bom && (found = _gm.key() == key && _gm.bomPart())) {
                      groupMeta = _gm;
                      break;
                  } else if ((found = _gm.key() == key)) {
                      groupMeta = _gm;
                      break;
                  }
              }

              if (!found) {
                  _gd.bom              = bom;
                  _gd.type             = baseName;
                  _gd.color            = color;
                  _gd.group.modelName  = where.modelName;
                  _gd.group.lineNumber = where.lineNumber;
                  _gd.offset[0]        = 0.0;
                  _gd.offset[1]        = 0.0;
                  Where undefined;
                  groupMeta.setWhere(undefined);
                  groupMeta.setValue(_gd);
              }

              return groupMeta;
          };

          bool  useImageSize    = pliMeta.imageSize.value(0) > 0;
          float modelScale      = pliMeta.modelScale.value();
          qreal cameraFoV       = double(pliMeta.cameraFoV.value());
          bool  customViewpoint = pliMeta.cameraAngles.customViewpoint();
          bool noCA             = !customViewpoint && pliMeta.rotStep.value().type.toUpper() == QLatin1String("ABS");
          qreal cameraAngleXX   = noCA ? double(0.0f) : double(pliMeta.cameraAngles.value(0));
          qreal cameraAngleYY   = noCA ? double(0.0f) : double(pliMeta.cameraAngles.value(1));

          // extract substitute part arguments

          // segments[3] = 0=substituteTypeLine,colour, 1=typeOfSub, 2=subOriginalType
          // segments[4] = 0=substituteTypeLine,colour, 1=typeOfSub, 2=subAttributes, 3=subOriginalType
          QString subAddAttributes;
          Rc subType = OkRc;
          if (isSubstitute) {
              enum { sgSubType = 1, sgAttributes = 2, sgHasAttributes = 4 };
              subType = Rc(segments.at(sgSubType).toInt());
              if (segments.size() >= sgHasAttributes) {
                  QStringList attributes = segments.at(sgAttributes).split(";");
                  if (subType > PliBeginSub2Rc) {
                      modelScale = attributes.at(sModelScale+sAdj).toFloat();
                  }
                  if (subType > PliBeginSub3Rc) {
                      cameraFoV = attributes.at(sCameraFoV+sAdj).toDouble();
                  }
                  if (subType > PliBeginSub4Rc) {
                      cameraAngleXX = attributes.at(sCameraAngleXX+sAdj).toDouble();
                      cameraAngleYY = attributes.at(sCameraAngleYY+sAdj).toDouble();
                  }
                  if (subType > PliBeginSub5Rc) {
                      subAddAttributes = QString("_%1_%2_%3")
                                         .arg(attributes.at(sTargetX+sAdj),
                                              attributes.at(sTargetY+sAdj),
                                              attributes.at(sTargetZ+sAdj));
                  }
                  if (subType > PliBeginSub6Rc) {
                      subAddAttributes = QString("_%1_%2_%3_%4")
                                         .arg(attributes.at(sRotX+sAdj),
                                              attributes.at(sRotY+sAdj),
                                              attributes.at(sRotZ+sAdj),
                                              attributes.at(sTransform+sAdj));
                  }
                  if (subType > PliBeginSub7Rc) {
                      subAddAttributes = QString("_%1_%2_%3_%4_%5_%6_%7")
                                         .arg(attributes.at(sTargetX+sAdj),
                                              attributes.at(sTargetY+sAdj),
                                              attributes.at(sTargetZ+sAdj),
                                              attributes.at(sRotX+sAdj),
                                              attributes.at(sRotY+sAdj),
                                              attributes.at(sRotZ+sAdj),
                                              attributes.at(sTransform+sAdj));
                  }
              }
          }

          // assemble image name key
          QString nameKey = QString("%1_%2_%3_%4_%5_%6_%7_%8_%9")
              .arg(QString("%1-%2").arg(baseName).arg(Preferences::preferredRenderer), // 0
                   color)                                              // 1
              .arg(useImageSize ? double(pliMeta.imageSize.value(0)) :
                                  lpub->pageSize(meta.LPub.page, 0))   // 2
              .arg(double(resolution()))                               // 3
              .arg(resolutionType() == DPI ? "DPI" : "DPCM")           // 4
              .arg(double(modelScale))                                 // 5
              .arg(cameraFoV)                                          // 6
              .arg(cameraAngleXX)                                      // 7
              .arg(cameraAngleYY);                                     // 8

          // when subRotation and/or subTarget exist append to nameKey (sums to 12/[13]/[16] nodes)

          if (subType && !subAddAttributes.isEmpty()) {
              nameKey.append(subAddAttributes);                       // 9,10,11,[12],[13,14,15]
          } else {
              if (pliMeta.target.isPopulated())                       // 9,10,11
                  nameKey.append(QString("_%1_%2_%3")
                                 .arg(double(pliMeta.target.x()))
                                 .arg(double(pliMeta.target.y()))
                                 .arg(double(pliMeta.target.z())));
              if (pliMeta.rotStep.isPopulated())                      // 9,10,11,12/[12],[13,14,15]
                  nameKey.append(QString("_%1")
                                 .arg(renderer->getRotstepMeta(pliMeta.rotStep,true)));
          }

          // assemble image name
          QString partsDir = bom ? Paths::bomDir : Paths::partsDir;
          QString imageName = QDir::toNativeSeparators(QDir::currentPath() + QDir::separator() +
                                                       partsDir + QDir::separator() + nameKey + ".png");

          QString individual;
          if (pliMeta.individualParts.value()) individual = QString("%1-").arg(randomFour());

          if (bom && splitBom) {
              if ( ! tempParts.contains(individual+key)) {
                  PliPart *part = new PliPart(type,color);
                  part->subType         = subType;
                  part->subOriginalType = subOriginalType;
                  part->element         = element;
                  part->description     = description;
                  part->styleMeta       = styleMeta;
                  part->cameraView      = pliMeta.cameraAngles.cameraView();
                  part->instanceMeta    = pliMeta.instance;
                  part->csiMargin       = pliMeta.part.margin;
                  part->sortColour      = QString("%1").arg(color,5,'0');
                  part->sortCategory    = QString("%1").arg(sortCategory,80,' ');
                  part->sortElement     = pliMeta.partElements.legoElements.value() ? QString("%1").arg(element,12,'0'): QString("%1").arg(element,20,' ');
                  part->nameKey         = nameKey;
                  part->imageName       = imageName;
                  part->groupMeta       = getGroupMeta();
                  tempParts.insert(individual+key,part);
                }
              tempParts[individual+key]->instances.append(here);
            } else {
              if ( ! parts.contains(individual+key)) {
                  PliPart *part = new PliPart(type,color);
                  part->subType         = subType;
                  part->subOriginalType = subOriginalType;
                  part->element         = element;
                  part->description     = description;
                  part->styleMeta       = styleMeta;
                  part->cameraView      = pliMeta.cameraAngles.cameraView();
                  part->instanceMeta    = pliMeta.instance;
                  part->csiMargin       = pliMeta.part.margin;
                  part->sortColour      = QString("%1").arg(color,5,'0');
                  part->sortCategory    = QString("%1").arg(sortCategory,80,' ');
                  part->sortElement     = pliMeta.partElements.legoElements.value() ? QString("%1").arg(element,12,'0'): QString("%1").arg(element,20,' ');
                  part->nameKey         = nameKey;
                  part->imageName       = imageName;
                  part->groupMeta       = getGroupMeta();
                  parts.insert(individual+key,part);
                }
              parts[individual+key]->instances.append(here);
            }
/* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
          qDebug() << "\n"
                   << "01/05 PLI PART ATTRIBUTES [" + individual+key + "] - SET PART"
                   << "\n1. Found:        " << (found ? "Yes" : "No")
                   << "\n2. Bom Part:     " << (bom ? "Yes" : "No")
                   << "\n3. Individual ID:" << individual << (individual.isEmpty() ? "(NoID)" : "")
                   << "\n4. Type:         " << type
                   << "\n5. Color:        " << color
                   << "\n6. Element:      " << element
                   << "\n7. NameKey:      " << nameKey
                      ;
#endif
//*/
/* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
          qDebug() << "\n"
                   << "01/05 PLI PART GROUP ATTRIBUTES [" + individual+key + "] - SET PART"
                   << "\n0. Found:        " << (found ? "Yes" : "No")
                   << "\n0. Bom Part:     " << (bom ? groupMeta.value().bPart ? "Yes" : "No" : "N/A")
                   << "\n0. BOM:          " << (groupMeta.value().bom ? "True" : "False")
                   << "\n0. Individual ID:" << individual << (individual.isEmpty() ? "(NoID)" : "")
                   << "\n1. Type:         " << groupMeta.value().type
                   << "\n2. Color:        " << groupMeta.value().color
                   << "\n3. OffsetX:      " << groupMeta.value().offset[0]
                   << "\n4. OffsetY:      " << groupMeta.value().offset[1]
                   << "\n5. Group Model:  " << groupMeta.value().group.modelName
                   << "\n6. Group Line:   " << groupMeta.value().group.lineNumber
                   << "\n7. Meta Model:   " << groupMeta.here().modelName
                   << "\n8. Meta Line:    " << groupMeta.here().lineNumber
                      ;
#endif
//*/
        }
    } //instances

  // now sort then divide the list based on BOM occurrence
  if (bom && splitBom) {

      sortedKeys = tempParts.keys();

      sortParts(tempParts, true);

      int quotient    = tempParts.size() / Gui::GetBOMs();
      int remainder   = tempParts.size() % Gui::GetBOMs();
      int maxParts    = 0;
      int startIndex  = 0;
      int partIndex   = 0;   // using 0-based index

      if (Gui::GetBOMOccurrence() == Gui::GetBOMs()) {
          maxParts = Gui::GetBOMOccurrence() * quotient + remainder;
          startIndex = maxParts - quotient - remainder;
        } else {
          maxParts = Gui::GetBOMOccurrence() * quotient;
          startIndex = maxParts - quotient;
        }

      const QList keys = sortedKeys;
      for (QString const &key : keys) {
          PliPart *part;
          part = tempParts[key];

          if (partIndex >= startIndex && partIndex < maxParts) {
              parts.insert(key,part);
            }
          partIndex++;
        }
      tempParts.clear();
      sortedKeys.clear();
    }
}

void Pli::clear()
{
  parts.clear();
}

QHash<int, QString>     annotationString;
QList<QString>          titleAnnotations;

bool Pli::initAnnotationString()
{
  if (annotationString.empty()) {
      annotationString[1] = "B";  // blue
      annotationString[2] = "G";  // green
      annotationString[3] = "DC"; // dark cyan
      annotationString[4] = "R";  // red
      annotationString[5] = "M";  // magenta
      annotationString[6] = "Br"; // brown
      annotationString[9] = "LB"; // light blue
      annotationString[10]= "LG"; // light green
      annotationString[11]= "C";  // cyan
      annotationString[12]= "LR"; // cyan
      annotationString[13]= "P";  // pink
      annotationString[14]= "Y";  // yellow
      annotationString[22]= "Ppl";// purple
      annotationString[25]= "O";  // orange

      annotationString[32+1] = "TB";  // blue
      annotationString[32+2] = "TG";  // green
      annotationString[32+3] = "TDC"; // dark cyan
      annotationString[32+4] = "TR";  // red
      annotationString[32+5] = "TM";  // magenta
      annotationString[32+6] = "TBr"; // brown
      annotationString[32+9] = "TLB"; // light blue
      annotationString[32+10]= "TLG"; // light green
      annotationString[32+11]= "TC";  // cyan
      annotationString[32+12]= "TLR"; // cyan
      annotationString[32+13]= "TP";  // pink
      annotationString[32+14]= "TY";  // yellow
      annotationString[32+22]= "TPpl";// purple
      annotationString[32+25]= "TO";  // orange

      titleAnnotations = Annotations::getTitleAnnotations();
    }
  return true;
}

void Pli::getAnnotation(
    QString &annotateStr,
    const int style,
    const QString &type,
    const QString &description)
{
  annotateStr.clear();

  bool enableAnnotations = pliMeta.annotation.display.value();
  if (! enableAnnotations)
    return;

  if (style) {
    annotateStr = Annotations::getStyleAnnotation(type);
    if (! annotateStr.isEmpty())
      return;
  }

  // pick up annotations
  annotateStr = description;

  bool title = pliMeta.annotation.titleAnnotation.value();
  bool freeform = pliMeta.annotation.freeformAnnotation.value();
  bool titleAndFreeform = pliMeta.annotation.titleAndFreeformAnnotation.value();

  if(title || titleAndFreeform) {
      if (titleAnnotations.size() == 0 && ! titleAndFreeform) {
          qDebug() << "Annotations enabled but no annotation source found.";
          return;
        }
      if (titleAnnotations.size() > 0) {
          QString annotation,sClean;
          static QRegularExpression rx;
          QRegularExpressionMatch match;
          for (int i = 0; i < titleAnnotations.size(); i++) {
              annotation = titleAnnotations[i];
              rx.setPattern(annotation);
              match = rx.match(annotateStr);
              if (match.hasMatch()) {
                  sClean = match.captured(1);
                  rx.setPattern("\\s");
                  sClean.remove(rx);            //remove spaces
                  annotateStr = sClean;
                  return;
                }
            }
        }
      if (titleAndFreeform) {
          annotateStr = Annotations::freeformAnnotation(type.toLower());
          return;
        }
    } else if (freeform) {
        annotateStr = Annotations::freeformAnnotation(type.toLower());
        return;
      }
    annotateStr.clear();
    return;
}

QString Pli::orient(QString &color, QString type)
{
  type = type.toLower();

  float a = 1, b = 0, c = 0;
  float d = 0, e = 1, f = 0;
  float g = 0, h = 0, i = 1;

  QString *cached = orientation[type];

  if (! cached) {
      QString filePath(Preferences::pliControlFile);

      if (!filePath.isEmpty()) {
          QFile file(filePath);
          if (file.open(QFile::ReadOnly | QFile::Text)) {
              QTextStream in(&file);

              while ( ! in.atEnd()) {
                  QString line = in.readLine(0);
                  QStringList tokens;

                  split(line,tokens);

                  if (tokens.size() != 15) {
                      continue;
                  }

                  QString token14 = tokens[14].toLower();

                  if (tokens.size() == 15 && tokens[0] == "1" && token14 == type) {
                      cached = new QString(line);
                      orientation.insert(type,cached);
                      a = tokens[5].toFloat();
                      b = tokens[6].toFloat();
                      c = tokens[7].toFloat();
                      d = tokens[8].toFloat();
                      e = tokens[9].toFloat();
                      f = tokens[10].toFloat();
                      g = tokens[11].toFloat();
                      h = tokens[12].toFloat();
                      i = tokens[13].toFloat();
                      break;
                  }
              }
              file.close();
          } else {
              emit gui->messageSig(LOG_ERROR, QObject::tr("Failed to open PLI control file: %1:<br>%2")
                                   .arg(filePath, file.errorString()));
          }
      }
  } else {
      QStringList tokens;

      split(*cached, tokens);

      if (tokens.size() == 15 && tokens[0] == "1") {
          a = tokens[5].toFloat();
          b = tokens[6].toFloat();
          c = tokens[7].toFloat();
          d = tokens[8].toFloat();
          e = tokens[9].toFloat();
          f = tokens[10].toFloat();
          g = tokens[11].toFloat();
          h = tokens[12].toFloat();
          i = tokens[13].toFloat();
        }
  }

  return QString ("1 %1 0 0 0 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11")
      .arg(color)
      .arg(a) .arg(b) .arg(c)
      .arg(d) .arg(e) .arg(f)
      .arg(g) .arg(h) .arg(i)
      .arg(type);
}

int Pli::createSubModelIcons()
{
    int rc = 0;
    QString key;
    QString type;
    QString color;
    Meta    _meta;
    meta       = &_meta;
    bom        = false;
    splitBom   = false;
    isSubModel = true;
    pliMeta    = meta->LPub.pli;
    int iconCount = lpub->ldrawFile.subFileOrder().size();

    auto setSubmodel = [this,&type,&color] (const int i)
    {
        color = "0";

        type = lpub->ldrawFile.subFileOrder()[i];

        QFileInfo info(type);

        QString baseName = info.completeBaseName();

        QString key = QString("%1_%2").arg(baseName, color);

        float modelScale  = pliMeta.modelScale.value();

        bool customViewpoint = pliMeta.cameraAngles.customViewpoint();

        bool noCA = !customViewpoint && pliMeta.rotStep.value().type.toUpper() == QLatin1String("ABS");

        // assemble icon name key
        QString nameKey = QString("%1_%2_%3_%4_%5_%6_%7_%8_%9")
                .arg(baseName,
                     color)
                .arg(lpub->pageSize(meta->LPub.page, 0))
                .arg(double(resolution()))
                .arg(resolutionType() == DPI ? "DPI" : "DPCM")
                .arg(double(modelScale))
                .arg(double(pliMeta.cameraFoV.value()))
                .arg(noCA ? double(0.0f) : double(pliMeta.cameraAngles.value(0)))
                .arg(noCA ? double(0.0f) : double(pliMeta.cameraAngles.value(1)));
        if (pliMeta.target.isPopulated())
            nameKey.append(QString("_%1_%2_%3")
                           .arg(double(pliMeta.target.x()))
                           .arg(double(pliMeta.target.y()))
                           .arg(double(pliMeta.target.z())));
        if (pliMeta.rotStep.isPopulated())
            nameKey.append(QString("_%1")
                           .arg(renderer->getRotstepMeta(pliMeta.rotStep,true)));

        if ( ! parts.contains(key)) {
            AnnotationStyleMeta styleMeta = pliMeta.defaultStyle;
            styleMeta.margin = pliMeta.annotate.margin;
            styleMeta.font   = pliMeta.annotate.font;
            styleMeta.color  = pliMeta.annotate.color;

            PliPart *part = new PliPart(type,color);
            part->styleMeta    = styleMeta;
            part->instanceMeta = pliMeta.instance;
            part->csiMargin    = pliMeta.part.margin;
            part->sortColour   = QString("%1").arg(color,5,'0');
            part->nameKey      = nameKey;
            parts.insert(key,part);
        }
        return key;
    };

    if (Render::useLDViewSCall()) {

        for (int i = 0; i < iconCount; i++) {
            setSubmodel(i);
        }
        rc = partSizeLDViewSCall();

    } else {

        emit gui->progressBarPermResetSig();
        emit gui->progressBarPermSetRangeSig(1, iconCount);

        for (int i = 0; i < iconCount; i++) {

            emit gui->progressBarPermSetValueSig(i + 1);
            emit gui->progressLabelPermSetTextSig(QObject::tr("Rendering submodel icon %1 of %2...").arg(i + 1).arg(iconCount));

            key = setSubmodel(i);
            if ((createPartImage(parts[key]->nameKey,type,color,nullptr) != 0)) {
                emit gui->messageSig(LOG_ERROR, QObject::tr("Failed to create submodel icon for key %1").arg(parts[key]->nameKey));
                rc = -1;
                continue;
            }
        }
        emit gui->progressBarPermSetValueSig(iconCount);
    }

    return rc;
}

int Pli::createPartImage(
    QString  &nameKey /*old Value: partialKey*/,
    QString  &type,
    QString  &color,
    QPixmap  *pixmap,
    int subType)
{

    int rc = 0;
    fadeSteps = Preferences::enableFadeSteps ;
    displayIcons = lcGetPreferences().mViewPieceIcons;
    fadeColour = LDrawColor::code(Preferences::validFadeStepsColour);
    highlightStep = Preferences::enableHighlightStep /*&& !Gui::suppressColourMeta()*/;
    bool fadePartOK = fadeSteps && !highlightStep && displayIcons;
    bool highlightPartOK = highlightStep && !fadeSteps && displayIcons;
    bool isColorPart = LDrawColourParts::isLDrawColourPart(type);
    int stepNumber = step ? step->stepNumber.number : 0/*BOM page*/;

    // set key substitute flag when there is a name change
    int keySub = 0;
    if (subType > PliBeginSub2Rc)
        keySub = subType;

    // create name key list
    QStringList nameKeys = nameKey.split("_");

    // treat parts with '_' in the name - decode
    QString altNameKey;
    if (nameKeys.at(nType).count(";")) {
        // keySub nameKey uses encoded nameKey when '_' in name
        if (keySub || bom)
            altNameKey = nameKey;
        nameKeys[nType].replace(";", "_");
        nameKey.replace(";", "_");
    } else if (keySub || bom)
        altNameKey = nameKey;

    // populate rotStep string from nameKeys - if exist
    bool hr;
    QString rotStep;
    if ((hr = nameKeys.size() == nHasRotstep) || nameKeys.size() == nHasTargetAndRotstep) {
        rotStep = QString("_%1_%2_%3_%4")
                          .arg(nameKeys.at(hr ? nRotX : nRot_X),          // rotX
                               nameKeys.at(hr ? nRotY : nRot_Y),          // rotY
                               nameKeys.at(hr ? nRotZ : nRot_Z),          // rotZ
                               nameKeys.at(hr ? nRotTrans : nRot_Trans)); // Transform
        if (Preferences::debugLogging)
            emit gui->messageSig(LOG_DEBUG, QObject::tr("Substitute type ROTSTEP meta: %1").arg(rotStep));
    }

    // populate targetPosition string from nameKeys - if exist
    QString targetPosition;
    if (nameKeys.size() >= nHasTarget) {
        targetPosition = QString("_%1_%2_%3")
                        .arg(nameKeys.at(nTargetX),                       // targetX
                             nameKeys.at(nTargetY),                       // targetY
                             nameKeys.at(nTargetZ));                      // targetZ

        if (Preferences::debugLogging)
            emit gui->messageSig(LOG_DEBUG, QObject::tr("Substitute type TARGET meta: %1").arg(targetPosition));
    }

    PliType pliType = isSubModel ? SUBMODEL: bom ? BOM : PART;

    for (int pT = 0; pT < ptn.size(); pT++ ) {
        int ptRc = 0;
//#ifdef QT_DEBUG_MODE
//        QString CurrentPartType = PartTypeNames[pT];
//#endif
        if (((pT == FADE_PART) && !fadePartOK) || ((pT == HIGHLIGHT_PART) && !highlightPartOK))
            continue;

        QElapsedTimer timer;
        timer.start();
        bool showElapsedTime = false;

        // moved from enum to save weight
        ia.baseName[pT] = QFileInfo(type).completeBaseName();
        ia.partColor[pT] = (pT == FADE_PART && fadeSteps && Preferences::fadeStepsUseColour) ? fadeColour : color;

        // assemble Visual Editor name key - create unique file when a value that impacts the image changes
        QString keyPart1 =  QString("%1_%2").arg(ia.baseName[pT], ia.partColor[pT]); /*baseName + colour*/

        QString keyPart2 = QString("%1_%2_%3_%4_%5_%6_%7_%8")
                                   .arg(stepNumber)
                                   .arg(nameKeys.at(nPageWidth),     // pageSizeP
                                        nameKeys.at(nResolution),    // resolution
                                        nameKeys.at(nResType),       // resolutionType - "DPI" : "DPCM"
                                        nameKeys.at(nModelScale),    // modelScale
                                        nameKeys.at(nCameraFoV),     // cameraFoV
                                        nameKeys.at(nCameraAngleXX), // cameraAngles.value(X)
                                        nameKeys.at(nCameraAngleYY));// cameraAngles.value(Y)

        if (!targetPosition.isEmpty())
            keyPart2.append(QString("%1").arg(targetPosition));

        if (!rotStep.isEmpty())
            keyPart2.append(QString("%1").arg(rotStep));

        emit gui->messageSig(LOG_INFO, QObject::tr("Generate PLI image for [%1] parts...").arg(PartTypeNames[pT]));

        // assemble image name using nameKey - create unique file when a value that impacts the image changes
        QString partsDir = bom ? Paths::bomDir : Paths::partsDir;
        QString imageDir = isSubModel ? Paths::submodelDir : partsDir;
        ldrNames  = QStringList() << QDir::toNativeSeparators(QString("%1/%2/pli.ldr").arg(QDir::currentPath(), Paths::tmpDir));
        imageName = QDir::toNativeSeparators(QString("%1/%2/%3%4.png").arg(QDir::currentPath(), imageDir, nameKey, ptn[pT].typeName));
        QString renderImageName = imageName;
        if (keySub || bom)
            renderImageName = QDir::toNativeSeparators(QString("%1/%2/%3%4.png").arg(QDir::currentPath(), imageDir, altNameKey, ptn[pT].typeName));

        QFile part(imageName);

        // Populate viewerPliPartiKey variable
        viewerPliPartKey = QString("%1;%2;%3")
                                  .arg(ia.baseName[pT],
                                       ia.partColor[pT])
                                  .arg(stepNumber);
/* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
        const QString stepType = step ? step->calledOut ? "called out" : step->multiStep ? "step group" : "single step" : "BOM";
        const int stepTypeLineNum = step ? step->calledOut ? step->topOfCallout().lineNumber : step->multiStep ? step->topOfSteps().lineNumber : step->topOfStep().lineNumber : 0;
        emit gui->messageSig(LOG_DEBUG,
                             QObject::tr("Create PLI ViewerStep "
                                         "Key: '%1' ["
                                         "PartName: %2, "
                                         "PartColour: %3, "
                                         "StepNumber: %4], "
                                         "Type: [%5], "
                                         "StepsLineNumber: [%6]")
                             .arg(viewerPliPartKey)
                             .arg(ia.baseName[pT])
                             .arg(ia.partColor[pT])
                             .arg(stepNumber)
                             .arg(stepType)
                             .arg(stepTypeLineNum));
#endif
//*/
        // Set Visual Editor PLI part entry
        bool nativeRenderer = Preferences::preferredRenderer == RENDERER_NATIVE;

        if ((! Gui::exportingObjects() || nativeRenderer) && pT == NORMAL_PART) {
            StudStyleMeta* ssm = meta->LPub.studStyle.value() ? &meta->LPub.studStyle : &pliMeta.studStyle;
            AutoEdgeColorMeta* aecm = meta->LPub.autoEdgeColor.enable.value() ? &meta->LPub.autoEdgeColor : &pliMeta.autoEdgeColor;
            HighContrastColorMeta* hccm = meta->LPub.studStyle.value() ? &meta->LPub.highContrast : &pliMeta.highContrast;
            // set viewer display options
            QStringList rotate            = rotStep.isEmpty()        ? QString("0 0 0 REL").split(" ") : rotStep.split("_");
            QStringList target            = targetPosition.isEmpty() ? QString("0 0 0 REL").split(" ") : targetPosition.split("_");
            bool customViewpoint          = pliMeta.cameraAngles.customViewpoint();
            bool noCA                     = !customViewpoint && rotate.at(3).toUpper() == QLatin1String("ABS");
            viewerOptions                 = new NativeOptions();
            viewerOptions->CameraDistance = renderer->ViewerCameraDistance(*meta,pliMeta.modelScale.value());
            viewerOptions->CameraName     = pliMeta.cameraName.value();
            viewerOptions->ImageFileName  = imageName;
            viewerOptions->ImageType      = Options::PLI;
            viewerOptions->Viewpoint      = static_cast<int>(pliMeta.cameraAngles.cameraView());
            viewerOptions->CustomViewpoint= customViewpoint;
            viewerOptions->Latitude       = noCA ? 0.0f : pliMeta.cameraAngles.value(0);
            viewerOptions->Longitude      = noCA ? 0.0f : pliMeta.cameraAngles.value(1);
            viewerOptions->ModelScale     = nameKeys.at(5).toFloat();
            viewerOptions->PageHeight     = pageSizeP(meta, 1);
            viewerOptions->PageWidth      = pageSizeP(meta, 0);
            viewerOptions->Position       = Vector3(pliMeta.position.x(),pliMeta.position.y(),pliMeta.position.z());
            viewerOptions->Resolution     = nameKeys.at(3).toFloat();
            viewerOptions->RotStep        = Vector3(rotate.at(0).toFloat(),rotate.at(1).toFloat(),rotate.at(2).toFloat());
            viewerOptions->RotStepType    = rotate.at(3);
            viewerOptions->AutoEdgeColor  = aecm->enable.value();
            viewerOptions->EdgeContrast   = aecm->contrast.value();
            viewerOptions->EdgeSaturation = aecm->saturation.value();
            viewerOptions->StudStyle      = ssm->value();
            viewerOptions->LightDarkIndex = hccm->lightDarkIndex.value();
            viewerOptions->StudCylinderColor = hccm->studCylinderColor.value();
            viewerOptions->StudCylinderColorEnabled = hccm->studCylinderColorEnabled.value();
            viewerOptions->PartEdgeColor  = hccm->partEdgeColor.value();
            viewerOptions->PartEdgeColorEnabled = hccm->partEdgeColorEnabled.value();
            viewerOptions->BlackEdgeColor = hccm->blackEdgeColor.value();
            viewerOptions->BlackEdgeColorEnabled = hccm->blackEdgeColorEnabled.value();
            viewerOptions->DarkEdgeColor  = hccm->darkEdgeColor.value();
            viewerOptions->DarkEdgeColorEnabled = hccm->darkEdgeColorEnabled.value();
            viewerOptions->Target         = Vector3(target.at(0).toFloat(),target.at(1).toFloat(),target.at(2).toFloat());
            viewerOptions->UpVector       = Vector3(pliMeta.upvector.x(),pliMeta.upvector.y(),pliMeta.upvector.z());
            viewerOptions->ViewerStepKey  = viewerPliPartKey;
            viewerOptions->ZFar           = pliMeta.cameraZFar.value();
            viewerOptions->ZNear          = pliMeta.cameraZNear.value();
            viewerOptions->DDF            = meta->LPub.cameraDDF.value();
            if (viewerOptsList.contains(keyPart1))
                viewerOptsList.remove(keyPart1);
            viewerOptsList.insert(keyPart1,viewerOptions);
        }

        // Check if viewer PLI part does exist in repository
        bool addViewerPliPartContent = !lpub->ldrawFile.viewerStepContentExist(viewerPliPartKey);

        // Generate and renderer  PLI Part file
        if ( ! part.exists() || addViewerPliPartContent) {

            showElapsedTime = true;

            // define ldr file name
            QFileInfo typeInfo(type);
            QString typeName = typeInfo.fileName();
            if (pT != NORMAL_PART && (isSubModel || isColorPart))
                typeName = typeInfo.completeBaseName() + ptn[pT].typeName + "." + typeInfo.suffix();

            QFuture<QStringList> future = QtConcurrent::run([this,pT,keySub,&typeName,&nameKeys,&rotStep,&type] () {
                // generate PLI Part file
                QStringList futureFile = configurePLIPart(pT,typeName,nameKeys,keySub);
                // add ROTSTEP command
                futureFile.prepend(QString("0 // ROTSTEP %1").arg(rotStep.isEmpty() ? "REL 0 0 0" : rotStep.replace("_"," ")));
                // prepare content for Native renderer
                if (Preferences::inlineNativeContent) {
                    // header and closing meta - this call returns an updated pliFile
                    if (renderer->setLDrawHeaderAndFooterMeta(futureFile,type,Options::PLI)) {
                        // consolidate pli part and MPD subfile(s) into single file
                        if (renderer->createNativeModelFile(futureFile,fadeSteps,highlightStep) != 0) {
                            emit gui->messageSig(LOG_ERROR,QString("Failed to consolidate Native PLI part"));
                            imageName = QString(":/resources/missingimage.png");
                            futureFile.clear();
                        }
                    }
                }
                return futureFile;
            });

            const QStringList pliFile = future.result();
            rc = pliFile.isEmpty();

            // unrotated part
            QStringList pliFileU = QStringList()
                    << QString("1 %1 0 0 0 1 0 0 0 1 0 0 0 1 %2").arg(color, typeName.toLower());

            // rotated part - without header
            QStringList pliFileR;
            for(const QString &line : pliFile)
              if (line[0] == '1') {
                pliFileR << line;
                break;
              }

            // store rotated and unrotated Part. Unrotated part used to generate LDView pov file
            if (targetPosition.isEmpty())
                keyPart2.append(QString("_0_0_0"));
            if (rotStep.isEmpty())
                keyPart2.append(QString("_0_0_0_REL"));
            QString pliPartKey = QString("%1;%3").arg(keyPart1, keyPart2);
            lpub->ldrawFile.insertViewerStep(viewerPliPartKey,pliFile,pliFileR,pliFileU,ldrNames.first(),imageName,pliPartKey,multistep,callout,Options::PLI);

            if (! rc && ! part.exists()) {

                // create a temporary DAT to feed the renderer
                part.setFileName(ldrNames.first());

                if ( ! part.open(QIODevice::WriteOnly)) {
                    emit gui->messageSig(LOG_ERROR,QObject::tr("Cannot open file for writing %1:\n%2.")
                                         .arg(ldrNames.first(), part.errorString()));
                    continue;
                }

                QTextStream out(&part);
                Q_FOREACH (QString line, pliFile)
                    out << line << lpub_endl;
                part.close();

                // feed DAT to renderer
                if ((renderer->renderPli(ldrNames,renderImageName,*meta,pliType,keySub) != 0)) {
                    emit gui->messageSig(LOG_ERROR,QObject::tr("%1 PLI [%2] render failed for<br>[%3]")
                                         .arg(rendererNames[Render::getRenderer()],
                                              PartTypeNames[pT],
                                              imageName));
                    imageName = QString(":/resources/missingimage.png");
                    ptRc = -1;
                }
            }
        }

        // create icon path key - using actual color code
        QString colourCode, imageKey;
        if (pT != NORMAL_PART) {
            colourCode = QString("%1").arg(pT == FADE_PART ?
                                           QString("%1%2").arg(LPUB3D_COLOUR_FADE_PREFIX, Preferences::fadeStepsUseColour ? fadeColour : ia.partColor[pT]) :
                                           QString("%1%2").arg(LPUB3D_COLOUR_HIGHLIGHT_PREFIX, ia.partColor[pT]));
            if (isSubModel || isColorPart) {
                imageKey = QString("%1%2_%3").arg(ia.baseName[pT], ptn[pT].typeName, colourCode);
            } else {
                imageKey = QString("%1_%2").arg(ia.baseName[pT], colourCode);
            }
        } else {
            colourCode = ia.partColor[pT];
            imageKey = QString("%1_%2").arg(ia.baseName[pT], colourCode);
        }

        if (Preferences::modeGUI)
            emit gui->setPliIconPathSig(imageKey,imageName);

        if (pixmap && (pT == NORMAL_PART))
            pixmap->load(imageName);

        if (showElapsedTime) {
            if (!ptRc) {
                emit gui->messageSig(LOG_INFO,QObject::tr("%1 PLI [%2] render took %3 to render image [%4].")
                                                          .arg(rendererNames[Render::getRenderer()],
                                                               PartTypeNames[pT],
                                                               Gui::elapsedTime(timer.elapsed(),false),
                                                               imageName));
            } else {
               rc = ptRc;
            }
        }
    }

  return rc;
}

// LDView performance improvement
int Pli::createPartImagesLDViewSCall(QStringList &ldrNames, bool isNormalPart, int sub) {
    int rc = 0;

    emit gui->messageSig(LOG_INFO, QObject::tr("Generate PLI images using LDView Single Call..."));

    if (! ldrNames.isEmpty()) {
        // feed DAT to renderer
        PliType pliType = isSubModel ? SUBMODEL: bom ? BOM : PART;
        if ((renderer->renderPli(ldrNames,QString(),*meta,pliType,sub) != 0)) {
            rc = -1;
        }
    }

    if (isNormalPart) {
        // 3. populate parts with image pixmap and size
        const QList keys = parts.keys();
        Q_FOREACH (const QString &key, keys) {
            PliPart *part;
            // get part info
            part = parts[key];
            // load image files into pixmap
            // instantiate pixmps //ERROR
            QPixmap *pixmap = new QPixmap();
            if (pixmap == nullptr) {
                rc = -1;
                continue;
            }

            if (! pixmap->load(part->imageName)) {
                emit gui->messageSig(LOG_ERROR,QObject::tr("Could not load PLI pixmap image.<br>%1 was not found.")
                                     .arg(part->imageName));
                part->imageName = QString(":/resources/missingimage.png");
                pixmap->load(part->imageName);
                rc = -1;
                continue;
            }

            // transfer image info to part
            QImage image = pixmap->toImage();

            part->pixmap = new PGraphicsPixmapItem(this,part,*pixmap,parentRelativeType,part->type, part->color);

            delete pixmap;

            // size the PLI
            part->pixmapWidth  = image.width();
            part->pixmapHeight = image.height();

            part->width  = image.width();

            /* Add instance count area */

            QString descr;

            descr = QString("%1x") .arg(part->instances.size(),0,10);

            QString font = pliMeta.instance.font.valueFoo();
            QString color = pliMeta.instance.color.value();

            int textWidth = 0, textHeight = 0;

            if (!pliMeta.individualParts.value()) {

                part->instanceText =
                    new InstanceTextItem(this,part,descr,font,color,parentRelativeType);

                part->instanceText->size(textWidth,textHeight);

                part->textHeight = textHeight;
            }

            // if text width greater than image width
            // the bounding box is wider

            if (textWidth > part->width) {
                part->width = textWidth;
            }

            /* Add annotation area */
            const int style = part->styleMeta.style.value();
            getAnnotation(descr, style, part->type,part->description);

            if (descr.size()) {

                part->text = descr;

                font   = part->styleMeta.font.valueFoo();
                color  = part->styleMeta.color.value();

                part->annotateText =
                        new AnnotateTextItem(this,part,descr,font,color,parentRelativeType);

                part->annotateText->size(part->annotWidth,part->annotHeight);

                if (part->annotWidth > part->width) {
                    part->width = part->annotWidth;
                }

                part->partTopMargin = part->styleMeta.margin.valuePixels(YY);

                int hMax = int(part->annotHeight + part->partTopMargin);
                for (int h = 0; h < hMax; h++) {
                    part->leftEdge  << part->width - part->annotWidth;
                    part->rightEdge << part->width;
                }
            } else {
                part->annotateText = nullptr;
                part->annotWidth  = 0;
                part->annotHeight = 0;
                part->partTopMargin = 0;
            }

            part->topMargin = part->csiMargin.valuePixels(YY);
            getLeftEdge(image,part->leftEdge);
            getRightEdge(image,part->rightEdge);

            /*
             * Lets see if we can slide the text up in the bottom left corner of
             * part image (or part element if display option selected)
             */

            bool overlapped = false;

            int overlap;
            for (overlap = 1; overlap < textHeight && ! overlapped; overlap++) {
                if (part->leftEdge[part->leftEdge.size() - overlap] < textWidth) {
                    overlapped = true;
                  }
              }

            part->textMargin = part->instanceMeta.margin.valuePixels(YY);

            int hMax = int(textHeight + part->textMargin);
            for (int h = overlap; h < hMax; h++) {
                part->leftEdge << 0;
                part->rightEdge << textWidth;
              }

            if (bom && pliMeta.partElements.display.value()) {

                /* Add BOM Elements area */

                if (part->element.size()) {

                    int elementMargin;

                    if (pliMeta.annotation.elementStyle.value()) {
                        font   = pliMeta.elementStyle.font.valueFoo();
                        color  = pliMeta.elementStyle.color.value();
                        elementMargin = pliMeta.elementStyle.margin.valuePixels(YY);
                    } else {
                        font   = pliMeta.annotate.font.valueFoo();
                        color  = pliMeta.annotate.color.value();
                        elementMargin = pliMeta.annotate.margin.valuePixels(YY);
                    }

                    part->annotateElement =
                            new AnnotateTextItem(this,part,part->element,font,color,parentRelativeType,true);

                    int elementWidth, elementHeight;

                    part->annotateElement->size(elementWidth,elementHeight);

                    part->elementHeight = elementHeight;

                    if (elementWidth > part->width) {
                        part->width = elementWidth;
                    }

                    /*
                     * Lets see if we can slide the BOM Element up in the bottom left corner of
                     * part image
                     */

                    overlapped = false;

                    for (overlap = 1; overlap < elementHeight && ! overlapped; overlap++) {
                        if (part->leftEdge[part->leftEdge.size() - overlap] < elementWidth) {
                            overlapped = true;
                        }
                    }

                    part->partBotMargin = elementMargin;

                    hMax = elementHeight + part->partBotMargin;
                    for (int h = overlap; h < hMax; h++) {
                        part->leftEdge << 0;
                        part->rightEdge << elementWidth;
                    }
                } else {
                    part->annotateElement = nullptr;
                    part->elementHeight  = 0;
                    part->partBotMargin = part->textMargin;
                }
            } else {
                part->partBotMargin = part->textMargin;
            }

            part->height = part->leftEdge.size();

            part->sortSize = QString("%1%2")
                    .arg(part->width, 8,10,QChar('0'))
                    .arg(part->height,8,10,QChar('0'));

            if (part->width > widestPart) {
                widestPart = part->width;
            }
            if (part->height > tallestPart) {
                tallestPart = part->height;
            }
        }
    }
    return rc;
}

QStringList Pli::configurePLIPart(int pT, QString &typeName, QStringList &nameKeys, int keySub) {
    QString updatedColour = ia.partColor[pT];
    QStringList out;

    if (fadeSteps && (pT == FADE_PART)) {
        updatedColour = QString("%1%2").arg(LPUB3D_COLOUR_FADE_PREFIX, ia.partColor[pT]);
        out << QString("0 // %1 part custom colours").arg(VER_PRODUCTNAME_STR);
        out << Gui::createColourEntry(ia.partColor[pT], PartType(pT));
        out << QString("0 !FADE %1").arg(Preferences::fadeStepsOpacity);
    }
    if (highlightStep && (pT == HIGHLIGHT_PART)) {
        updatedColour = QString("%1%2").arg(LPUB3D_COLOUR_HIGHLIGHT_PREFIX, ia.partColor[pT]);
        out << QString("0 // %1 part custom colours").arg(VER_PRODUCTNAME_STR);
        out << Gui::createColourEntry(ia.partColor[pT], PartType(pT));
        out << QString("0 !SILHOUETTE %1 %2")
                       .arg(Preferences::highlightStepLineWidth)
                       .arg(Preferences::highlightStepColour);
    }

    if (keySub) {
        bool good = false, ok = false;
        // get subRotation string - if exist
        bool hr;
        RotStepMeta rotStepMeta;
        if ((hr = nameKeys.size() == nHasRotstep) ||
                  nameKeys.size() == nHasTargetAndRotstep) {
            RotStepData rotStepData;
            rotStepData.rots[0] = nameKeys.at(hr ? nRotX : nRot_X).toDouble(&good);
            rotStepData.rots[1] = nameKeys.at(hr ? nRotY : nRot_Y).toDouble(&ok);
            good &= ok;
            rotStepData.rots[2] = nameKeys.at(hr ? nRotZ : nRot_Z).toDouble(&ok);
            good &= ok;
            if (!good) {
                emit gui->messageSig(LOG_NOTICE,QObject::tr("Malformed ROTSTEP values from nameKey [%1], using '0 0 0'.")
                                     .arg(QString("%1_%2_%3")
                                     .arg(nameKeys.at(hr ? nRotX : nRot_X),
                                          nameKeys.at(hr ? nRotY : nRot_Y),
                                          nameKeys.at(hr ? nRotZ : nRot_Z))));
                rotStepData.rots[0] = 0.0f;
                rotStepData.rots[1] = 0.0f;
                rotStepData.rots[2] = 0.0f;
            }
            rotStepData.type    = nameKeys.at(hr ? nRotTrans : nRot_Trans);
            rotStepMeta.setValue(rotStepData);
        }

        QStringList rotatedType = QStringList() << orient(updatedColour, typeName);
        QString addLine = "1 color 0 0 0 1 0 0 0 1 0 0 0 1 foo.ldr";

        float latitude  = nameKeys.at(nCameraAngleXX).toFloat(&good);
        float longitude = nameKeys.at(nCameraAngleYY).toFloat(&ok);
        good &= ok;
        if (!good) {
            emit gui->messageSig(LOG_NOTICE,QObject::tr("Malformed Camera Angle values from nameKey [%1], using 'latitude 30', 'longitude -45'.")
                                 .arg(QString("%1 %2").arg(nameKeys.at(nCameraAngleXX), nameKeys.at(nCameraAngleYY))));
            latitude = 30.0; longitude = -45.0;
        }
        FloatPairMeta cameraAngles;
         cameraAngles.setValues(latitude,longitude);

        bool nativeRenderer  = Preferences::preferredRenderer == RENDERER_NATIVE;
        // RotateParts #3 - 5 parms, do not apply camera angles for native renderer
        if ((renderer->rotateParts(addLine,rotStepMeta,rotatedType,cameraAngles,!nativeRenderer/*applyCA*/)) != 0)
            emit gui->messageSig(LOG_ERROR,QObject::tr("Failed to rotate type: %1.").arg(typeName));

        out << rotatedType;
    } else {
        out << orient(updatedColour, typeName);
    }

    if (highlightStep && (pT == HIGHLIGHT_PART))
        out << QString("0 !SILHOUETTE");
    if (fadeSteps && (pT == FADE_PART))
        out << QString("0 !FADE");

    if ((pT == FADE_PART) || (pT == HIGHLIGHT_PART))
        out << QString("0 NOFILE");

    return out;
}

void Pli::partClass(
    QString &pclass,
    const QString &description)
{
  pclass = description;

  if (pclass.length()) {
      static QRegularExpression rx("^(\\w+)\\s+([0-9a-zA-Z]+).*$");
      QRegularExpressionMatch match = rx.match(description);
      if (match.hasMatch()) {
          pclass = match.captured(1);
          if (rx.captureCount() == 2 && match.captured(1).contains("Technic", Qt::CaseInsensitive)) {
              pclass += match.captured(2);
          }
        } else {
          pclass = "NoCat";
        }
    } else {
      pclass = "NoCat";
    }
}

int Pli::placePli(
    QList<QString> &keys,
    int    xConstraint,
    int    yConstraint,
    bool   packSubs,
    bool   sortType,
    int   &pliCols,
    int   &pliWidth,
    int   &pliHeight)
{

  // Place the first row
  BorderData borderData;
  borderData = pliMeta.border.valuePixels();
  int left = 0;
  int nPlaced = 0;
  int tallest = 0;
  int topMargin = int(borderData.margin[1]+borderData.thickness);
  int botMargin = topMargin;

  pliCols = 0;

  pliWidth = 0;
  pliHeight = 0;

  for (int i = 0; i < keys.size(); i++) {
      parts[keys[i]]->placed = false;
      if (parts[keys[i]]->height > yConstraint) {
          yConstraint = parts[keys[i]]->height;
          // return -2;
      }
  }

  QList< QPair<int, int> > margins;

  while (nPlaced < keys.size()) {

      int i;
      PliPart *part = nullptr;

      for (i = 0; i < keys.size(); i++) {
          QString key = keys[i];
          part = parts[key];
          if ( ! part->placed && left + part->width < xConstraint) {
              break;
          }
      }

      if (i == keys.size()) {
          return -1;
      }

      /* Start new col */

      PliPart *prevPart = parts[keys[i]];

      pliCols++;

      int width = prevPart->width /* + partMarginX */;
      int widest = i;

      prevPart->left = left;
      prevPart->bot  = 0;
      prevPart->placed = true;
      prevPart->col = pliCols;
      nPlaced++;

      QPair<int, int> margin;


      margin.first = qMax(prevPart->instanceMeta.margin.valuePixels(XX),
                          prevPart->csiMargin.valuePixels(XX));

      // Compare BOM Element Margin
      if (bom && pliMeta.partElements.display.value()) {
          int elementMargin = qMax(prevPart->styleMeta.margin.valuePixels(XX),
                                   prevPart->csiMargin.valuePixels(XX));
          if (elementMargin > margin.first)
              margin.first = elementMargin;
      }

      tallest = qMax(tallest,prevPart->height);

      int right = left + prevPart->width;
      int bot = prevPart->height;

      botMargin = qMax(botMargin,prevPart->csiMargin.valuePixels(YY));

      // leftEdge is the number of pixels between the left edge of the image
      // and the leftmost pixel in the image

      // rightEdge is the number of pixels between the left edge of the image
      // and the rightmost pixel in the image

      // lets see if any unplaced part fits under the right side
      // of the first part of the column

      bool fits = false;
      for (i = 0; i < keys.size() && ! fits; i++) {
          part = parts[keys[i]];

          if ( ! part->placed) {
              int xMargin = qMax(prevPart->csiMargin.valuePixels(XX),
                                 part->csiMargin.valuePixels(XX));
              int yMargin = qMax(prevPart->csiMargin.valuePixels(YY),
                                 part->csiMargin.valuePixels(YY));

              // Do they overlap?

              int top;
              for (top = 0; top < part->height; top++) {
                  int ltop  = prevPart->height - part->height - yMargin + top;
                  if (ltop >= 0 && ltop < prevPart->height) {
                      if (prevPart->rightEdge[ltop] + xMargin >
                          prevPart->width - part->width + part->leftEdge[top]) {
                          break;
                      }
                  }
              }

              if (top == part->height) {
                  fits = true;
                  break;
              }
          }
      }

      if (fits) {
          part->left = prevPart->left + prevPart->width - part->width;
          part->bot  = 0;
          part->placed = true;
          part->col = pliCols;
          nPlaced++;
      }

      // allocate new row

      while (nPlaced < parts.size()) {

          int overlap = 0;

          bool overlapped = false;

          // new possible upstairs neighbors

          for (i = 0; i < keys.size() && ! overlapped; i++) {
              PliPart *part = parts[keys[i]];

              if ( ! part->placed) {

                  int splitMargin = qMax(prevPart->topMargin,part->csiMargin.valuePixels(YY));

                  // dropping part down into prev part (top part is right edge, bottom left)

                  for (overlap = 1; overlap < prevPart->height && ! overlapped; overlap++) {
                      if (overlap > part->height) { // in over our heads?
                          // slide the part from the left to right until it bumps into previous
                          // part
                          for (int right = 0, left = 0;
                               right < part->height;
                               right++,left++) {
                              if (part->rightEdge[right] + splitMargin >
                                  prevPart->leftEdge[left+overlap-part->height]) {
                                  overlapped = true;
                                  break;
                              }
                          }
                      } else {
                          // slide the part from the left to right until it bumps into previous
                          // part
                          for (int right = part->height - overlap - 1, left = 0;
                               right < part->height && left < overlap;
                               right++,left++) {
                              if (right >= 0 && part->rightEdge[right] + splitMargin >
                                  prevPart->leftEdge[left]) {
                                  overlapped = true;
                                  break;
                              }
                          }
                      }
                  }

                  // overlap = 0;

                  if (bot + part->height + splitMargin - overlap <= yConstraint) {
                      bot += splitMargin;
                      break;
                  } else {
                      overlapped = false;
                  }
              }
          }

          if (i == keys.size()) {
              break; // we can't go more Vertical in this column
          }

          PliPart *part = parts[keys[i]];

          margin.first    = part->csiMargin.valuePixels(XX);
          int splitMargin = qMax(prevPart->topMargin,part->csiMargin.valuePixels(YY));

          prevPart = parts[keys[i]];

          prevPart->left = left;
          prevPart->bot  = bot - overlap;
          prevPart->placed = true;
          prevPart->col = pliCols;
          nPlaced++;

          if (sortType) {
              if (prevPart->width > width) {
                  widest = i;
                  width = prevPart->width;
              }
          }

          int height = prevPart->height + splitMargin;

          // try to do sub columns

          if (packSubs && /* DISABLES CODE */ (0) /* && overlap == 0 */ ) {
              int subLeft = left + prevPart->width;
              int top = bot + prevPart->height - overlap + prevPart->topMargin;

              // allocate new sub_col

              while (nPlaced < keys.size() && i < parts.size()) {
                  PliPart *part = parts[keys[i]];
                  int subMargin = 0;

                  for (i = 0; i < keys.size(); i++) {
                      part = parts[keys[i]];
                      if ( ! part->placed) {
                          subMargin = qMax(prevPart->csiMargin.valuePixels(XX),part->csiMargin.valuePixels(XX));
                          if (subLeft + subMargin + part->width <= right &&
                              bot + part->height + part->topMargin <= top) {
                              break;
                          }
                      }
                  }

                  if (i == parts.size()) {
                      break;
                  }

                  int subWidth = part->width;
                  part->left = subLeft + subMargin;
                  part->bot  = bot;
                  part->placed = true;
                  nPlaced++;

                  int subBot = bot + part->height + part->topMargin;

                  // try to place sub_row

                  while (nPlaced < parts.size()) {
                      for (i = 0; i < parts.size(); i++) {
                          part = parts[keys[i]];
                          subMargin = qMax(prevPart->csiMargin.valuePixels(XX),part->csiMargin.valuePixels(XX));

                          if ( ! part->placed &&
                               subBot + part->height + splitMargin <= top &&
                               subLeft + subMargin + part->width <= right) {
                              break;
                          }
                      }

                      if (i == parts.size()) {
                          break;
                      }

                      part->left = subLeft + subMargin;
                      part->bot  = subBot;
                      part->placed = true;
                      nPlaced++;

                      subBot += part->height + splitMargin;
                  }

                  subLeft += subWidth;
              }
          } /* DISABLED CODE */

          bot -= overlap;

          // FIMXE:: try to pack something under bottom of the row.

          bot += height;
          if (bot > tallest) {
              tallest = bot;
          }
      }

      topMargin = qMax(topMargin, part ? part->topMargin : 0);

      left += width;

      part = parts[keys[widest]];

      if (part->annotWidth) {
          margin.second = qMax(part->styleMeta.margin.valuePixels(XX),part->csiMargin.valuePixels(XX));
      } else {
          margin.second = part->csiMargin.valuePixels(XX);
      }

      margins.append(margin);
  }

  pliWidth = left;

  int margin;
  int totalCols = margins.size();
  int lastMargin = 0;
  for (int col = 0; col < totalCols; col++) {
      lastMargin = margins[col].second;

      if (col == 0) {
          int bmargin = int(borderData.thickness + borderData.margin[0]);
          margin = qMax(bmargin,margins[col].first);
      } else {
          margin = qMax(margins[col].first,margins[col].second);
      }

      for (int i = 0; i < parts.size(); i++) {
          if (parts[keys[i]]->col >= col+1) {
              parts[keys[i]]->left += margin;
          }
      }

      pliWidth += margin;
  }

  if (lastMargin < borderData.margin[0]+borderData.thickness) {
      lastMargin = int(borderData.margin[0]+borderData.thickness);
  }

  pliWidth += lastMargin;

  pliHeight = tallest;

  for (int i = 0; i < parts.size(); i++) {
      parts[keys[i]]->bot += botMargin;
  }

  pliHeight += botMargin + topMargin;

  return 0;
}

void Pli::placeCols(
    QList<QString> &keys)
{
  // Place the first row
  BorderData borderData;
  borderData = pliMeta.border.valuePixels();

  float topMargin = parts[keys[0]]->topMargin;
  topMargin = qMax(borderData.margin[1]+borderData.thickness,topMargin);
  float botMargin = parts[keys[0]]->csiMargin.valuePixels(YY);
  botMargin = qMax(borderData.margin[1]+borderData.thickness,botMargin);

  int height = 0;
  int width;

  PliPart *part = parts[keys[0]];

  float borderMargin = borderData.thickness + borderData.margin[XX];

  width = int(qMax(borderMargin, part->maxMargin()));

  for (int i = 0; i < keys.size(); i++) {
      part = parts[keys[i]];
      part->left = width;
      part->bot  = int(botMargin);
      part->col = i;

      width += part->width;

      if (part->height > height) {
          height = part->height;
        }

      if (i < keys.size() - 1) {
          PliPart *nextPart = parts[keys[i+1]];
          width += int(qMax(part->maxMargin(),nextPart->maxMargin()));
        }
    }
  part = parts[keys[keys.size()-1]];
  width += int(qMax(part->maxMargin(),borderMargin));

  size[0] = width;
  size[1] = int(topMargin + height + botMargin);
}

void Pli::getLeftEdge(
    QImage     &image,
    QList<int> &edge)
{
  QImage alpha =
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
      image.convertToFormat(QImage::Format_Grayscale8);
#else
      image.alphaChannel();
#endif

  for (int y = 0; y < alpha.height(); y++) {
      int x;
      for (x = 0; x < alpha.width(); x++) {
          QColor c = alpha.pixel(x,y);
          if (c.blue()) {
              edge << x;
              break;
            }
        }
      if (x == alpha.width()) {
          edge << x - 1;
        }
    }
}

void Pli::getRightEdge(
    QImage     &image,
    QList<int> &edge)
{
  QImage alpha =
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
      image.convertToFormat(QImage::Format_Grayscale8);
#else
      image.alphaChannel();
#endif

  for (int y = 0; y < alpha.height(); y++) {
      int x;
      for (x = alpha.width() - 1; x >= 0; x--) {
          QColor c = alpha.pixel(x,y);
          if (c.blue()) {
              edge << x;
              break;
            }
        }
      if (x < 0) {
          edge << 0;
        }
    }
}

bool Pli::loadTheViewer() {
    if (! Gui::exporting()) {
        if (! renderer->LoadViewer(viewerOptions)) {
            emit gui->messageSig(LOG_ERROR,QObject::tr("Could not load Visual Editor with Pli part key: %1")
                                 .arg(viewerPliPartKey));
            return false;
        }
    }
    return true;
}

void Pli::sortParts(QHash<QString, PliPart *> &parts, bool setSplit)
{
    // initialize
    bool ascending = true;
    bool unsorted = true;

    // sort direction lambda
    auto setSortDirection = [this, &ascending](const int sort)
    {
        switch (sort) {
        case SortPrimary:
            ascending = tokenMap[pliMeta.sortOrder.primaryDirection.value()] != SortDescending;
            break;
        case SortSecondary:
            ascending = tokenMap[pliMeta.sortOrder.secondaryDirection.value()] != SortDescending;
            break;
        case SortTetriary:
            ascending = tokenMap[pliMeta.sortOrder.tertiaryDirection.value()] != SortDescending;
            break;
        }
    };

    // sort
    while (unsorted) {

        unsorted = false;

        for (int firstPart = 0; firstPart < parts.size() - 1; firstPart++) {
            for (int nextPart = firstPart+1; nextPart < parts.size(); nextPart++) {

                QString firstValue, nextValue;

                bool sortedBy[SortByOptions];
                sortedBy[PartSize]     = false;
                sortedBy[PartColour]   = false;
                sortedBy[PartCategory] = false;
                sortedBy[PartElement]  = false;
                bool canSort           = false;

                // get sortedBy lambda
                auto isSortedBy = [&sortedBy](const int option)
                {
                    return sortedBy[option];
                };

                // set sortedBy lambda
                auto setSortedBy = [&canSort,&sortedBy](const int option)
                {
                    canSort = sortedBy[option] = true;
                };

                // set part Values lambda
                auto setPartValues = [this, &parts, &firstValue, &nextValue, &firstPart, &nextPart](
                        const int option)
                {
                    switch (option) {
                    case PartColour:
                        firstValue = parts[sortedKeys[firstPart]]->sortColour;
                        nextValue = parts[sortedKeys[nextPart]]->sortColour;
                        break;
                    case PartCategory:
                        firstValue = parts[sortedKeys[firstPart]]->sortCategory;
                        nextValue = parts[sortedKeys[nextPart]]->sortCategory;
                        break;
                    case PartSize:
                        firstValue = parts[sortedKeys[firstPart]]->sortSize;
                        nextValue = parts[sortedKeys[nextPart]]->sortSize;
                        break;
                    case PartElement:
                        firstValue = parts[sortedKeys[firstPart]]->sortElement;
                        nextValue = parts[sortedKeys[nextPart]]->sortElement;
                        break;
                    }
                };

                // process options for the primary sort
                int option = tokenMap[pliMeta.sortOrder.primary.value()];
                if (option != NoSort) {
                    setSortDirection(SortPrimary);
                    setPartValues(option);
                    setSortedBy(option);
                }

                // process options secondary sort
                option = tokenMap[pliMeta.sortOrder.secondary.value()];
                if (!setSplit &&
                    firstValue == nextValue &&
                    !isSortedBy(option) &&
                    option != NoSort) {
                    setSortDirection(SortSecondary);
                    setPartValues(option);
                    setSortedBy(option);
                }

                // process options tertiary sort
                option = tokenMap[pliMeta.sortOrder.tertiary.value()];
                if (!setSplit &&
                    firstValue == nextValue &&
                    !isSortedBy(option) &&
                    option != NoSort) {
                    setSortDirection(SortTetriary);
                    setPartValues(option);
                    setSortedBy(option);
                }

                // sort the part values
                if (canSort && (ascending ? firstValue > nextValue : firstValue < nextValue)) {
                    QString moved = sortedKeys[firstPart];
                    sortedKeys[firstPart] = sortedKeys[nextPart];
                    sortedKeys[nextPart] = moved;
                    unsorted = true;
                }

                // restore primary sort direction
                setSortDirection(tokenMap[pliMeta.sortOrder.primary.value()]);
            }
        }
    }
}

int Pli::sortPli()
{
    // Create and render parts to populate part size
    //QFuture<int> PartsFuture = QtConcurrent::run([this] {
    //    return partSize();
    //});

    //int rc = PartsFuture.result();
    int rc = partSize();
    if (rc)
        emit gui->messageSig(LOG_ERROR, QObject::tr("There was a problem sizing parts for this part list instance"));

    rc = !parts.size();

    if (rc) {
        emit gui->messageSig(LOG_ERROR, QObject::tr("No valid parts were found for this part list instance"));
        return rc;
    }

    sortedKeys = parts.keys();

    if (! bom)
        pliMeta.sort.setValue(true);

    QFuture<void> future = QtConcurrent::run([this] {
        sortParts(parts);
    });
    future.waitForFinished();;

    return rc;
}

int Pli::partSize()
{
    isSubModel = false; // not sizing icon images

    // set the current step - enable access from other parts of the application - e.g. Renderer
    if (step)
        lpub->setCurrentStep(step);

    if (Render::useLDViewSCall()) {
      if (partSizeLDViewSCall() != 0)
          return -1;
    } else {

      widestPart = 0;
      tallestPart = 0;

      bool populateBomProgress = bom && Preferences::modeGUI && !Gui::exporting();
      int partCounter = 0;

      const QList keys = parts.keys();
      Q_FOREACH (const QString &key, keys) {

          if (populateBomProgress)
              emit gui->progressBarPermSetValueSig(++partCounter);

          // get part info
          PliPart *part;
          part = parts[key];

          QFileInfo info(part->type);
          PieceInfo* pieceInfo = lcGetPiecesLibrary()->FindPiece(info.fileName().toUpper().toLatin1().constData(), nullptr, false, false);

          if (pieceInfo ||
              lpub->ldrawFile.isUnofficialPart(part->type) ||
              lpub->ldrawFile.isSubmodel(part->type)) {

              QPixmap *pixmap = new QPixmap();
              if (pixmap == nullptr) {
                  return -1;
                }

              // treat parts with '_' in the name - encode
              QString nameKey = part->nameKey;
              if (part->type.count("_")) {
                  const QString type = QFileInfo(part->type).completeBaseName();
                  nameKey.replace(type, QString(type).replace("_", ";"));
              }

              if (createPartImage(nameKey,part->type,part->color,pixmap,part->subType) != 0) {
                  emit gui->messageSig(LOG_ERROR, QObject::tr("Failed to create PLI part for key %1")
                                       .arg(part->nameKey));
                  imageName = QString(":/resources/missingimage.png");
                  pixmap->load(imageName);
                  return -1;
              }

              QImage image = pixmap->toImage();

              part->pixmap = new PGraphicsPixmapItem(this,part,*pixmap,parentRelativeType,part->type, part->color);

              delete pixmap;

              part->pixmapWidth  = image.width();
              part->pixmapHeight = image.height();

              part->width  = image.width();

              /* Add instance count area */

              QString descr;

              descr = QString("%1x") .arg(part->instances.size(),0,10);

              QString font = pliMeta.instance.font.valueFoo();
              QString color = pliMeta.instance.color.value();

              int textWidth = 0, textHeight = 0;

              if (!pliMeta.individualParts.value()) {

                  part->instanceText =
                        new InstanceTextItem(this,part,descr,font,color,parentRelativeType);

                  part->instanceText->size(textWidth,textHeight);

                  part->textHeight = textHeight;
              }

              // if text width greater than image width
              // the bounding box is wider

              if (textWidth > part->width) {
                  part->width = textWidth;
                }

              /* Add annotation area */
              const int style = part->styleMeta.style.value();
              getAnnotation(descr, style, part->type,part->description);

              if (descr.size()) {

                  part->text = descr;

                  font   = part->styleMeta.font.valueFoo();
                  color  = part->styleMeta.color.value();

                  part->annotateText =
                      new AnnotateTextItem(this,part,descr,font,color,parentRelativeType);

                  part->annotateText->size(part->annotWidth,part->annotHeight);

                  if (part->annotWidth > part->width) {
                      part->width = part->annotWidth;
                    }

                  part->partTopMargin = part->styleMeta.margin.valuePixels(YY);           // annotationStyle margin

                  int hMax = int(part->annotHeight + part->partTopMargin);
                  for (int h = 0; h < hMax; h++) {
                      part->leftEdge  << part->width - part->annotWidth;
                      part->rightEdge << part->width;
                    }
                } else {
                  part->annotateText = nullptr;
                  part->annotWidth  = 0;
                  part->annotHeight = 0;
                  part->partTopMargin = 0;
                }

              part->topMargin = part->csiMargin.valuePixels(YY);
              getLeftEdge(image,part->leftEdge);
              getRightEdge(image,part->rightEdge);

              /*
               * Lets see if we can slide the text up in the bottom left corner of
               * part image (or part element if display option selected)
               */

              bool overlapped = false;

              int overlap;
              for (overlap = 1; overlap < textHeight && ! overlapped; overlap++) {
                  if (part->leftEdge[part->leftEdge.size() - overlap] < textWidth) {
                      overlapped = true;
                    }
                }

              part->textMargin = part->instanceMeta.margin.valuePixels(YY);

              int hMax = int(textHeight + part->textMargin);
              for (int h = overlap; h < hMax; h++) {
                  part->leftEdge << 0;
                  part->rightEdge << textWidth;
                }

              if (bom && pliMeta.partElements.display.value()) {

                  /* Add BOM Elements area */

                  if (part->element.size()) {

                      int elementMargin;

                      if (pliMeta.annotation.elementStyle.value()) {
                          font   = pliMeta.elementStyle.font.valueFoo();
                          color  = pliMeta.elementStyle.color.value();
                          elementMargin = pliMeta.elementStyle.margin.valuePixels(YY);
                      } else {
                          font   = pliMeta.annotate.font.valueFoo();
                          color  = pliMeta.annotate.color.value();
                          elementMargin = pliMeta.annotate.margin.valuePixels(YY);
                      }

                      part->annotateElement =
                              new AnnotateTextItem(this,part,part->element,font,color,parentRelativeType,true);

                      int elementWidth, elementHeight;

                      part->annotateElement->size(elementWidth,elementHeight);

                      part->elementHeight = elementHeight;

                      if (elementWidth > part->width) {
                          part->width = elementWidth;
                      }

                      /*
                       * Lets see if we can slide the BOM Element up in the bottom left corner of
                       * part image
                       */

                      overlapped = false;

                      for (overlap = 1; overlap < elementHeight && ! overlapped; overlap++) {
                          if (part->leftEdge[part->leftEdge.size() - overlap] < elementWidth) {
                              overlapped = true;
                          }
                      }

                      part->partBotMargin = elementMargin;

                      hMax = elementHeight + part->partBotMargin;
                      for (int h = overlap; h < hMax; h++) {
                          part->leftEdge << 0;
                          part->rightEdge << elementWidth;
                      }
                  } else {
                      part->annotateElement = nullptr;
                      part->elementHeight  = 0;
                      part->partBotMargin = part->textMargin;
                  }
              } else {
                  part->partBotMargin = part->textMargin;
              }

              part->height = part->leftEdge.size();

              part->sortSize = QString("%1%2")
                  .arg(part->width, 8,10,QChar('0'))
                  .arg(part->height,8,10,QChar('0'));

              if (part->width > widestPart) {
                  widestPart = part->width;
                }
              if (part->height > tallestPart) {
                  tallestPart = part->height;
                }
            } else {
              emit gui->messageSig(LOG_NOTICE, QObject::tr("Part [%1] was not found - part removed from list").arg(parts[key]->type));
              delete parts[key];
              parts.remove(key);
            }
        }
    }

  return 0;
}

//LDView performance improvement
int Pli::partSizeLDViewSCall() {

    int rc = 0;
    int iaSub = 0;
    widestPart = 0;
    tallestPart = 0;

    fadeSteps = Preferences::enableFadeSteps ;
    displayIcons = lcGetPreferences().mViewPieceIcons;
    fadeColour = LDrawColor::code(Preferences::validFadeStepsColour);
    highlightStep = Preferences::enableHighlightStep /*&& !Gui::suppressColourMeta()*/;
    bool fadePartOK = fadeSteps && !highlightStep && displayIcons;
    bool highlightPartOK = highlightStep && !fadeSteps && displayIcons;
    int stepNumber = step ? step->stepNumber.number : 0/*BOM page*/;
    bool populateBomProgress = bom && Preferences::modeGUI && !Gui::exporting();
    int partCounter = 0;

    // 1. generate ldr files
    const QList keys = parts.keys();
    Q_FOREACH (const QString &key, keys) {

        if (populateBomProgress)
            emit gui->progressBarPermSetValueSig(++partCounter);

        // get part info

        PliPart *pliPart;

        pliPart = parts[key];

        QFileInfo info(pliPart->type);
        PieceInfo* pieceInfo = lcGetPiecesLibrary()->FindPiece(info.fileName().toUpper().toLatin1().constData(), nullptr, false, false);

        if (pieceInfo ||
            lpub->ldrawFile.isSubmodel(pliPart->type) ||
            lpub->ldrawFile.isUnofficialPart(pliPart->type)) {

            if (pliPart->color == LDRAW_MAIN_MATERIAL_COLOUR || isSubModel) {
                pliPart->color = "0";
            }

            bool isColorPart = LDrawColourParts::isLDrawColourPart(pliPart->type);

            // treat parts with '_' in the name - encode
            QString nameKey = pliPart->nameKey;
            if (pliPart->type.count("_")) {
                const QString type = QFileInfo(pliPart->type).completeBaseName();
                nameKey.replace(type, QString(type).replace("_", ";"));
            }

            // set key substitute flag when there is a namekey change
            int keySub = 0;
            if (pliPart->subType > PliBeginSub2Rc)
                keySub = pliPart->subType;

            // append nameKey with 'SUB' for LDView Single Call
            if (keySub)
                nameKey.append("_SUB"); // 14th node

            // create name key list
            QStringList nameKeys = nameKey.split("_");

            // treat parts with '_' in the name - decode
            QString altNameKey;
            if (nameKeys.at(nType).count(";")) {
                // keySub nameKey uses encoded nameKey when '_' in name
                if (keySub || bom)
                  altNameKey = nameKey;
                nameKeys[nType].replace(";", "_");
                nameKey.replace(";", "_");
            } else if (bom)
                altNameKey = nameKey;

            // populate rotStep string from nameKeys - if exist
            bool hr;
            QString rotStep;
            if ((hr = nameKeys.size() == nHasRotstep) || nameKeys.size() == nHasTargetAndRotstep) {
                rotStep = QString("_%1_%2_%3_%4")
                                  .arg(nameKeys.at(hr ? nRotX : nRot_X),          // rotX
                                       nameKeys.at(hr ? nRotY : nRot_Y),          // rotY
                                       nameKeys.at(hr ? nRotZ : nRot_Z),          // rotZ
                                       nameKeys.at(hr ? nRotTrans : nRot_Trans)); // Transform
                if (Preferences::debugLogging)
                    emit gui->messageSig(LOG_DEBUG, QObject::tr("Substitute type ROTSTEP meta: %1").arg(rotStep));
            }

            // populate targetPosition string from nameKeys - if exist
            QString targetPosition;
            if (nameKeys.size() >= nHasTarget) {
                targetPosition = QString("_%1_%2_%3")
                                .arg(nameKeys.at(nTargetX),                       // targetX
                                     nameKeys.at(nTargetY),                       // targetY
                                     nameKeys.at(nTargetZ));                      // targetZ
                if (Preferences::debugLogging)
                    emit gui->messageSig(LOG_DEBUG, QObject::tr("Substitute type TARGET meta: %1").arg(targetPosition));
            }

            emit gui->messageSig(LOG_INFO, QObject::tr("Processing PLI part for nameKey [%1]").arg(nameKey));

            for (int pT = 0; pT < ptn.size(); pT++ ) {

//#ifdef QT_DEBUG_MODE
//                QString CurrentPartType = PartTypeNames[pT];
//#endif
                if (((pT == FADE_PART) && !fadePartOK) || ((pT == HIGHLIGHT_PART) && !highlightPartOK))
                     continue;

                // pass substitute key to single call list
                if (keySub && !ia.sub[pT])
                    ia.sub[pT] = keySub;
                ia.baseName[pT] = QFileInfo(pliPart->type).completeBaseName();
                ia.partColor[pT] = (pT == FADE_PART && fadeSteps && Preferences::fadeStepsUseColour) ? fadeColour : pliPart->color;

                // assemble Visual Editor name key - create unique file when a value that impacts the image changes
                QString keyPart1 =  QString("%1_%2").arg(ia.baseName[pT], ia.partColor[pT]); /*baseName + colour*/

                QString keyPart2 = QString("%1_%2_%3_%4_%5_%6_%7_%8")
                                           .arg(stepNumber)
                                           .arg(nameKeys.at(nPageWidth),     // pageSizeP
                                                nameKeys.at(nResolution),    // resolution
                                                nameKeys.at(nResType),       // resolutionType - "DPI" : "DPCM"
                                                nameKeys.at(nModelScale),    // modelScale
                                                nameKeys.at(nCameraFoV),     // cameraFoV
                                                nameKeys.at(nCameraAngleXX), // cameraAngles.value(X)
                                                nameKeys.at(nCameraAngleYY));// cameraAngles.value(Y)

                if (!targetPosition.isEmpty())
                    keyPart2.append(QString("_%1").arg(targetPosition));

                if (!rotStep.isEmpty())
                    keyPart2.append(QString("_%1").arg(rotStep));

                // assemble ldr name
                QString key = !ptn[pT].typeName.isEmpty() ? nameKey + ptn[pT].typeName : nameKey;
                QString altKey = !ptn[pT].typeName.isEmpty() ? altNameKey + ptn[pT].typeName : altNameKey;
                QString ldrName = QDir::toNativeSeparators(QString("%1/%2/%3.ldr").arg(QDir::currentPath(), Paths::tmpDir, key));
                QString ldrAltName = QDir::toNativeSeparators(QString("%1/%2/%3.ldr").arg(QDir::currentPath(), Paths::tmpDir, altKey));
                QString partsDir = bom ? Paths::bomDir : Paths::partsDir;
                QString imageDir = isSubModel ? Paths::submodelDir : partsDir;
                // remove _SUB for imageName
                if (keySub && key.endsWith("_SUB"))
                    key.replace("_SUB","");
                QString imageName = QDir::toNativeSeparators(QString("%1/%2/%3.png").arg(QDir::currentPath(), imageDir, key));

                // create icon path key - using actual color code
                QString colourCode, imageKey;
                if (pT != NORMAL_PART) {
                    colourCode = QString("%1").arg(pT == FADE_PART ?
                                                   QString("%1%2").arg(LPUB3D_COLOUR_FADE_PREFIX, ia.partColor[pT]) :
                                                   QString("%1%2").arg(LPUB3D_COLOUR_HIGHLIGHT_PREFIX, ia.partColor[pT]));
                    if (isSubModel || isColorPart) {
                        imageKey = QString("%1%2_%3").arg(ia.baseName[pT], ptn[pT].typeName, colourCode);
                    } else {
                        imageKey = QString("%1_%2").arg(ia.baseName[pT], colourCode);
                    }
                } else {
                    colourCode = ia.partColor[pT];
                    imageKey = QString("%1_%2").arg(ia.baseName[pT], colourCode);
                }

                // store imageName
                ia.imageKeys[pT] << imageKey;
                ia.imageNames[pT] << imageName;

                QFile part(imageName);

                // Populate viewerPliPartiKey variable
                viewerPliPartKey = QString("%1;%2;%3")
                                           .arg(ia.baseName[pT],
                                                ia.partColor[pT])
                                           .arg(stepNumber);

#ifdef QT_DEBUG_MODE
                const QString stepType = step ? step->calledOut ? QObject::tr("called out") : step->multiStep ? QObject::tr("step group") : QObject::tr("single step") : QObject::tr("BOM");
                const int stepTypeLineNum = step ? step->calledOut ? step->topOfCallout().lineNumber : step->multiStep ? step->topOfSteps().lineNumber : step->topOfStep().lineNumber : 0;
                emit gui->messageSig(LOG_DEBUG,
                                     QObject::tr("Create PLI ViewerStep "
                                                 "Key: '%1' ["
                                                 "PartName: %2, "
                                                 "PartColour: %3, "
                                                 "StepNumber: %4], "
                                                 "Type: [%5], "
                                                 "StepsLineNumber: [%6]")
                                     .arg(viewerPliPartKey,
                                          ia.baseName[pT],
                                          ia.partColor[pT])
                                     .arg(stepNumber)
                                     .arg(stepType)
                                     .arg(stepTypeLineNum));
#endif

                // Set Visual Editor PLI part entry
                if (! Gui::exportingObjects() && pT == NORMAL_PART) {
                    StudStyleMeta* ssm = meta->LPub.studStyle.value() ? &meta->LPub.studStyle : &pliMeta.studStyle;
                    AutoEdgeColorMeta* aecm = meta->LPub.autoEdgeColor.enable.value() ? &meta->LPub.autoEdgeColor : &pliMeta.autoEdgeColor;
                    HighContrastColorMeta* hccm = meta->LPub.studStyle.value() ? &meta->LPub.highContrast : &pliMeta.highContrast;
                    // set viewer display options
                    QStringList rotate            = rotStep.isEmpty()        ? QString("0 0 0 REL").split(" ") : rotStep.split("_");
                    QStringList target            = targetPosition.isEmpty() ? QString("0 0 0 REL").split(" ") : targetPosition.split("_");
                    bool customViewpoint          = pliMeta.cameraAngles.customViewpoint();
                    bool noCA                     = !customViewpoint && rotate.at(3).toUpper() == QLatin1String("ABS");
                    viewerOptions                 = new NativeOptions();
                    viewerOptions->CameraDistance = renderer->ViewerCameraDistance(*meta,pliMeta.modelScale.value());
                    viewerOptions->CameraName     = pliMeta.cameraName.value();
                    viewerOptions->ImageFileName  = imageName;
                    viewerOptions->ImageType      = Options::PLI;
                    viewerOptions->Viewpoint      = static_cast<int>(pliMeta.cameraAngles.cameraView());
                    viewerOptions->CustomViewpoint= customViewpoint;
                    viewerOptions->Latitude       = noCA ? 0.0f : pliMeta.cameraAngles.value(0);
                    viewerOptions->Longitude      = noCA ? 0.0f : pliMeta.cameraAngles.value(1);
                    viewerOptions->ModelScale     = nameKeys.at(5).toFloat();
                    viewerOptions->PageHeight     = pageSizeP(meta, 1);
                    viewerOptions->PageWidth      = pageSizeP(meta, 0);
                    viewerOptions->Position       = Vector3(pliMeta.position.x(),pliMeta.position.y(),pliMeta.position.z());
                    viewerOptions->Resolution     = nameKeys.at(3).toFloat();
                    viewerOptions->RotStep        = Vector3(rotate.at(0).toFloat(),rotate.at(1).toFloat(),rotate.at(2).toFloat());
                    viewerOptions->RotStepType    = rotate.at(3);
                    viewerOptions->AutoEdgeColor  = aecm->enable.value();
                    viewerOptions->EdgeContrast   = aecm->contrast.value();
                    viewerOptions->EdgeSaturation = aecm->saturation.value();
                    viewerOptions->StudStyle      = ssm->value();
                    viewerOptions->LightDarkIndex = hccm->lightDarkIndex.value();
                    viewerOptions->StudCylinderColor = hccm->studCylinderColor.value();
                    viewerOptions->StudCylinderColorEnabled = hccm->studCylinderColorEnabled.value();
                    viewerOptions->PartEdgeColor  = hccm->partEdgeColor.value();
                    viewerOptions->PartEdgeColorEnabled = hccm->partEdgeColorEnabled.value();
                    viewerOptions->BlackEdgeColor = hccm->blackEdgeColor.value();
                    viewerOptions->BlackEdgeColorEnabled = hccm->blackEdgeColorEnabled.value();
                    viewerOptions->DarkEdgeColor  = hccm->darkEdgeColor.value();
                    viewerOptions->DarkEdgeColorEnabled = hccm->darkEdgeColorEnabled.value();
                    viewerOptions->Target         = Vector3(target.at(0).toFloat(),target.at(1).toFloat(),target.at(2).toFloat());
                    viewerOptions->UpVector       = Vector3(pliMeta.upvector.x(),pliMeta.upvector.y(),pliMeta.upvector.z());
                    viewerOptions->ViewerStepKey  = viewerPliPartKey;
                    viewerOptions->ZFar           = pliMeta.cameraZFar.value();
                    viewerOptions->ZNear          = pliMeta.cameraZNear.value();
                    viewerOptions->DDF            = meta->LPub.cameraDDF.value();
                    if (viewerOptsList.contains(keyPart1))
                        viewerOptsList.remove(keyPart1);
                    viewerOptsList.insert(keyPart1,viewerOptions);
                }

                // Check if viewer PLI part does exist in repository
                bool addViewerPliPartContent = !lpub->ldrawFile.viewerStepContentExist(viewerPliPartKey);

                if ( ! part.exists() || addViewerPliPartContent) {

                    // store ldrName - long name includes nameKey
                    ia.ldrNames[pT] << ldrName;
                    ia.ldrAltNames[pT] << ldrAltName;

                    // define ldr file name
                    QFileInfo typeInfo = QFileInfo(pliPart->type);
                    QString typeName = typeInfo.fileName();
                    bool isColorPart = LDrawColourParts::isLDrawColourPart(typeInfo.fileName());
                    if (pT != NORMAL_PART && (isSubModel || isColorPart))
                        typeName = typeInfo.completeBaseName() + ptn[pT].typeName + "." + typeInfo.suffix();

                    QFuture<QStringList> future = QtConcurrent::run([this,pT,keySub,&typeName,&nameKeys,&rotStep,&pliPart,&imageName] () {
                        // generate PLI Part file
                        QStringList futureFile = configurePLIPart(pT,typeName,nameKeys,keySub);
                        // add ROTSTEP command
                        futureFile.prepend(QString("0 // ROTSTEP %1").arg(rotStep.isEmpty() ? "REL 0 0 0" : rotStep.replace("_"," ")));
                        // prepare content for Native renderer
                        if (Preferences::inlineNativeContent) {
                            // header and closing meta - this call returns an updated pliFile
                            if (renderer->setLDrawHeaderAndFooterMeta(futureFile,pliPart->type,Options::PLI)) {
                                // consolidate pli part and MPD subfile(s) into single file
                                if (renderer->createNativeModelFile(futureFile,fadeSteps,highlightStep) != 0) {
                                    emit gui->messageSig(LOG_ERROR,QObject::tr("Failed to consolidate Native PLI part"));
                                    imageName = QString(":/resources/missingimage.png");
                                    futureFile.clear();
                                }
                            }
                        }
                        return futureFile;
                    });

                    const QStringList pliFile = future.result();

                    // unrotated part
                    QStringList pliFileU = QStringList()
                            << QString("1 %1 0 0 0 1 0 0 0 1 0 0 0 1 %2").arg(colourCode, typeName.toLower());

                    // rotated part - without header
                    QStringList pliFileR;
                    for(const QString &line : pliFile)
                      if (line[0] == '1') {
                        pliFileR << line;
                        break;
                      }

                    // store rotated and unrotated Part. Unrotated part used to generate LDView pov file
                    if (targetPosition.isEmpty())
                        keyPart2.append(QString("_0_0_0"));
                    if (rotStep.isEmpty())
                        keyPart2.append(QString("_0_0_0_REL"));
                    QString pliPartKey = QString("%1;%3").arg(keyPart1, keyPart2);
                    lpub->ldrawFile.insertViewerStep(viewerPliPartKey,pliFile,pliFileR,pliFileU,ia.ldrNames[pT].first(),imageName,pliPartKey,multistep,callout,Options::PLI);

                    if (! rc && ! part.exists()) {

                        // create a DAT files to feed the renderer
                        part.setFileName(ldrName);
                        if ( ! part.open(QIODevice::WriteOnly)) {
                            emit gui->messageSig(LOG_ERROR,QObject::tr("Cannot open ldr DAT file for writing part:\n%1:\n%2.")
                                                                       .arg(ldrName, part.errorString()));
                            return -1;
                        }

                        QTextStream out(&part);
                        Q_FOREACH (QString line, pliFile)
                            out << line << lpub_endl;
                        part.close();
                    }

                } else {
                    ia.ldrNames[pT] << QStringList();
                    ia.ldrAltNames[pT] << QStringList();
                } // part already exist
            }     // for every part type
        }         // part is valid
        else
        {
            delete parts[key];
            parts.remove(key);
        }
    }            // for every part

    if (isSubModel) {
        emit gui->progressBarPermResetSig();
        emit gui->progressBarPermSetRangeSig(1, ptn.size());
    }

    // 2. generate part image
    for (int pT = 0; pT < ptn.size(); pT++ ) {   // for every part type

        int ptRc = 0;
        if (isSubModel) {
            emit gui->progressLabelPermSetTextSig(QObject::tr("Rendering submodel icon %1 of %2...").arg(pT + 1).arg(ptn.size()));
            emit gui->progressBarPermSetValueSig(pT);
        } else if (populateBomProgress)
            emit gui->progressBarPermSetValueSig(++partCounter);

//#ifdef QT_DEBUG_MODE
//        QString CurrentPartType = PartTypeNames[pT];
//#endif

        if (((pT == FADE_PART) && !fadePartOK) || ((pT == HIGHLIGHT_PART) && !highlightPartOK))
             continue;

        QElapsedTimer timer;
        timer.start();

        QStringList renderLdrNames = ia.ldrNames[pT];
        if (ia.sub[pT]) {
            iaSub = ia.sub[pT]; // keySub
            renderLdrNames = ia.ldrAltNames[pT];
        } else if (bom)
            renderLdrNames = ia.ldrAltNames[pT];
        if ((createPartImagesLDViewSCall(renderLdrNames,(isSubModel ? false : pT == NORMAL_PART),iaSub) != 0)) {
            emit gui->messageSig(LOG_ERROR,QObject::tr("LDView Single Call PLI render failed."));
            continue;
        }

        if (Preferences::modeGUI)
            for (int i = 0; i < ia.imageKeys[pT].size() && displayIcons; i++) {                      // normal, fade, highlight image full paths
                emit gui->setPliIconPathSig(ia.imageKeys[pT][i],ia.imageNames[pT][i]);
            }

        if (!ia.ldrNames[pT].isEmpty()) {
            if (!ptRc) {
                emit gui->messageSig(LOG_INFO, QObject::tr("%1 PLI (Single Call) for [%2] render took %3 to render %4.")
                                                           .arg(rendererNames[Render::getRenderer()],
                                                                PartTypeNames[pT],
                                                                Gui::elapsedTime(timer.elapsed(),false),
                                                                QString("%1 %2")
                                                                .arg(ia.ldrNames[pT].size())
                                                                .arg(ia.ldrNames[pT].size() == 1 ? QObject::tr("image") : QObject::tr("images"))));
            } else {
                rc = ptRc;
            }
        }
    } // for every part type

    if (isSubModel) {
        emit gui->progressBarPermSetValueSig(ptn.size());
    }

    return rc;
}

int Pli::sizePli(Meta *_meta, PlacementType _parentRelativeType, bool _perStep)
{
  int rc = !parts.size();
  if (rc)
      return rc;

  parentRelativeType = _parentRelativeType;
  perStep = _perStep;
  meta = _meta;

  // Create and render PLI parts
  //QFuture<int> PartsFuture = QtConcurrent::run([this] {
  //    return sortPli();
  //});

  rc = sortPli(); // PartsFuture.result();
  if (rc)
      return rc;

  QFuture<int> future = QtConcurrent::run([this] {
      ConstrainData constrainData = pliMeta.constrain.value();
      return resizePli(meta,constrainData);
  });

  return future.result();
}

int Pli::sizePli(ConstrainData::PliConstrain constrain, unsigned size)
{
  int rc = !parts.size();
  if (rc)
      return rc;

  if (meta) {
      QFuture<int> future = QtConcurrent::run([&] {
          ConstrainData constrainData;
          constrainData.type = constrain;
          if (constrainData.type == ConstrainData::PliConstrainWidth )
              constrainData.constraint.width = size;
          else if (constrainData.type == ConstrainData::PliConstrainHeight)
              constrainData.constraint.height = size;
          return resizePli(meta,constrainData);
      });

      return future.result();
  }

  return rc;
}

int Pli::resizePli(
    Meta *meta,
    ConstrainData &constrainData)
{
  // preserve LOCAL (pushed) flag
  bool pushed = placement.pushed;
  switch (parentRelativeType) {
    case StepGroupType:
      placement = meta->LPub.multiStep.pli.placement;
      break;
    case CalloutType:
      placement = meta->LPub.callout.pli.placement;
      break;
    default:
      placement = meta->LPub.pli.placement;
      break;
  }

  // preserve LOCAL (pushed) flag
  placement.pushed = pushed;

  // Fill the part list image using constraint
  //   Constrain Height
  //   Constrain Width
  //   Constrain Columns
  //   Constrain Area
  //   Constrain Square

  #define X_CONSTRAIN 10000000

  bool packSubs = pliMeta.pack.value();
  bool sortType = pliMeta.sort.value();
  int height, pliWidth = 0,pliHeight = 0, pliCols = 0;

  if (constrainData.type == ConstrainData::PliConstrainHeight) {
      int rc;
      rc = placePli(sortedKeys,
                    X_CONSTRAIN,
                    int(constrainData.constraint.height),
                    packSubs,
                    sortType,
                    pliCols,
                    pliWidth,
                    pliHeight);
      if (rc == -2) {
          constrainData.type = ConstrainData::PliConstrainArea;
      }
  } else if (constrainData.type == ConstrainData::PliConstrainColumns) {
      if (parts.size() <= constrainData.constraint.columns) {
          placeCols(sortedKeys);
          pliWidth = Placement::size[0];
          pliHeight = Placement::size[1];
          pliCols = parts.size();
      } else {
          int maxHeight = 0;
          for (int i = 0; i < parts.size(); i++) {
              maxHeight += parts[sortedKeys[i]]->height + parts[sortedKeys[i]]->csiMargin.valuePixels(1);
          }

          maxHeight += maxHeight;

          int constraintCols = int(constrainData.constraint.columns);

          if (constraintCols) {
              for (height = maxHeight/(4*constraintCols); height <= maxHeight; height++) {
                  int rc = placePli(sortedKeys,
                                    X_CONSTRAIN,
                                    height,
                                    packSubs,
                                    sortType,
                                    pliCols,
                                    pliWidth,
                                    pliHeight);
                  if (rc == 0 && pliCols == constraintCols) {
                      break;
                  }
              }
          }
      }
  } else if (constrainData.type == ConstrainData::PliConstrainWidth) {
      int height = 0;
      for (int i = 0; i < parts.size(); i++) {
          height += parts[sortedKeys[i]]->height;
      }

      int good_height = height;

      for ( ; height > 0; height -= 4) {
          int rc = placePli(sortedKeys,
                            X_CONSTRAIN,
                            height,
                            packSubs,
                            sortType,
                            pliCols,
                            pliWidth,
                            pliHeight);
          if (rc) {
              break;
          }

          int w = 0;

          for (int i = 0; i < parts.size(); i++) {
              int t;
              t = parts[sortedKeys[i]]->left + parts[sortedKeys[i]]->width;
              if (t > w) {
                  w = t;
              }
          }

          if (w < constrainData.constraint.width) {
              good_height = height;
          }
      }

      placePli(sortedKeys,
               X_CONSTRAIN,
               good_height,
               packSubs,
               sortType,
               pliCols,
               pliWidth,
               pliHeight);
  } else if (constrainData.type == ConstrainData::PliConstrainArea) {

      int height = 0;
      for (int i = 0; i < parts.size(); i++) {
          height += parts[sortedKeys[i]]->height;
      }

      int min_area = height*height;
      int good_height = height;
      // step by 1/10 of inch or centimeter
      int step = int(toPixels(0.1f,DPI));

      for ( ; height > 0; height -= step) {
          int rc = placePli(sortedKeys,
                            X_CONSTRAIN,
                            height,
                            packSubs,
                            sortType,
                            pliCols,
                            pliWidth,
                            pliHeight);

          if (rc) {
              break;
          }

          int h = 0;
          int w = 0;

          for (int i = 0; i < parts.size(); i++) {
              int t;
              t = parts[sortedKeys[i]]->bot + parts[sortedKeys[i]]->height;
              if (t > h) {
                  h = t;
              }
              t = parts[sortedKeys[i]]->left + parts[sortedKeys[i]]->width;
              if (t > w) {
                  w = t;
              }
          }

          if (w*h < min_area) {
              min_area = w*h;
              good_height = height;
          }
      }

      placePli(sortedKeys,
               X_CONSTRAIN,
               good_height,
               packSubs,
               sortType,
               pliCols,
               pliWidth,
               pliHeight);
  } else if (constrainData.type == ConstrainData::PliConstrainSquare) {

      int height = 0;
      for (int i = 0; i < parts.size(); i++) {
          height += parts[sortedKeys[i]]->height;
      }

      int min_delta = height;
      int good_height = height;
      int step = int(toPixels(0.1f,DPI));

      for ( ; height > 0; height -= step) {
          int rc = placePli(sortedKeys,
                            X_CONSTRAIN,
                            height,
                            packSubs,
                            sortType,
                            pliCols,
                            pliWidth,
                            pliHeight);

          if (rc) {
              break;
          }

          int h = pliWidth;
          int w = pliHeight;
          int delta = 0;

          if (w < h) {
              delta = h - w;
          } else if (h < w) {
              delta = w - h;
          }

          if (delta < min_delta) {
              min_delta = delta;
              good_height = height;
          }
      }

      placePli(sortedKeys,
               X_CONSTRAIN,
               good_height,
               packSubs,
               sortType,
               pliCols,
               pliWidth,
               pliHeight);
  }

  size[0] = pliWidth;
  size[1] = pliHeight;

  constrainData.constraint.width = pliWidth;
  constrainData.constraint.height = pliHeight;
  constrainData.constraint.columns = pliCols;
  pliMeta.constrain.setValue(constrainData);

  return 0;
}

void Pli::positionChildren(
    int height,
    qreal scaleX,
    qreal scaleY)
{
  QString key;

  Q_FOREACH (key, sortedKeys) {
      PliPart *part = parts[key];
      if (part == nullptr) {
          continue;
        }

      bool showElement = bom && pliMeta.partElements.display.value() && part->annotateElement;

      float x,y;
      x = part->left;
      y = height - part->bot;

      if (part->annotateText) {
          part->annotateText->setParentItem(background);
          part->annotateText->setPos(
                (x + part->width - part->annotWidth)/scaleX,
                (y - part->height /*+ part->annotHeight*/)/scaleY);
        }

      if (part->pixmap == nullptr) {
          break;
      }

      part->pixmap->setParentItem(background);
      part->pixmap->setPos(
            x/scaleX,
            (y - part->height + part->annotHeight + part->partTopMargin)/scaleY);
      part->pixmap->setTransformationMode(Qt::SmoothTransformation);

      if (!pliMeta.individualParts.value()) {
          part->instanceText->setParentItem(background);
          part->instanceText->setPos(
                x/scaleX,
                (y - (showElement ? (part->textHeight + part->elementHeight - part->textMargin) : part->textHeight))/scaleY);
      }

      // Position the BOM Element
      if (showElement) {
          part->annotateElement->setParentItem(background);
          part->annotateElement->setPos(
                x/scaleX,
                (y - part->elementHeight)/scaleY);
      }

    }
}

int Pli::addPli(
    int       submodelLevel,
    QGraphicsItem *parent)
{
  if (parts.size()) {
      background =
          new PliBackgroundItem(
            this,
            size[0],
          size[1],
          parentRelativeType,
          submodelLevel,
          parent);

      if ( ! background) {
          return -1;
        }

      background->size[0] = size[0];
      background->size[1] = size[1];

      positionChildren(size[1],1.0,1.0);
    } else {
      background = nullptr;
    }
  return 0;
}

void Pli::setPos(float x, float y)
{
  if (background) {
      background->setPos(x,y);
    }
}
void Pli::setFlag(QGraphicsItem::GraphicsItemFlag flag, bool value)
{
  if (background) {
      background->setFlag(flag,value);
    }
}

/*
 * Single step per page                   case 3 top/bottom of step
 * step in step group pli per step = true case 3 top/bottom of step
 * step in callout                        case 3 top/bottom of step
 * step group global pli                  case 2 topOfSteps/bottomOfSteps
 * BOM on single step per page            case 2 topOfSteps/bottomOfSteps
 * BOM on step group page                 case 1
 * BOM on cover page                      case 2 topOfSteps/bottomOfSteps
 * BOM on numbered page
 */

bool Pli::autoRange(Where &top, Where &bottom)
{
  if (bom || ! perStep) {
      top = topOfSteps();
      bottom = bottomOfSteps();
      return steps->list.size() && (perStep || bom);
    } else {
      top = topOfStep();
      bottom = bottomOfStep();
      return false;
    }
}

QString PGraphicsPixmapItem::pliToolTip(
    QString type,
    QString color,
    bool isSub)
{
  QString originalType =
          isSub && !part->subOriginalType.isEmpty() ?
              QObject::tr(" (Substitute for %1)")
              .arg(QStringList(part->subOriginalType.split(":")).first()) : QString();

  QString toolTip =
          QObject::tr("%1 (%2) %3 \"%4\" - right-click to modify")
                      .arg(LDrawColor::name(color),
                           color,
                           type,
                           QString("%1%2")
                           .arg(part->description,
                                originalType));
  return toolTip;
}

const QString Pli::titleDescription(const QString &part)
{
  PieceInfo* pieceInfo = lcGetPiecesLibrary()->FindPiece(QFileInfo(part).fileName().toUpper().toLatin1().constData(), nullptr, false, false);
  if (pieceInfo)
      return pieceInfo->m_strDescription;

  Where here(part,0);
  QString title = gui->readLine(here);
  title = title.right(title.length() - 2);
  return title;
}

PliBackgroundItem::PliBackgroundItem(
    Pli           *_pli,
    int            width,
    int            height,
    PlacementType  _parentRelativeType,
    int            submodelLevel,
    QGraphicsItem *parent):
    isHovered(false),
    mouseIsDown(false)
{
  pli       = _pli;
  grabHeight = height;

  grabber = nullptr;
  grabbersVisible = false;

  parentRelativeType = _parentRelativeType;

  QPixmap *pixmap = new QPixmap(width,height);

  QString toolTip;

  if (_pli->bom) {
      toolTip = QObject::tr("Bill Of Materials");
    } else {
      toolTip = QObject::tr("Part List");
    }
  toolTip += QObject::tr(" [%1 x %2 px] - right-click to modify")
                         .arg(width)
                         .arg(height);

  if (parentRelativeType == StepGroupType /* && pli->perStep == false */) {
      if (pli->bom) {
          placement = pli->meta->LPub.bom.placement;
        } else {
          placement = pli->meta->LPub.multiStep.pli.placement;
        }
    } else {
      placement = pli->pliMeta.placement;
    }

  //gradient settings
  if (pli->pliMeta.background.value().gsize[0] == 0.0f &&
      pli->pliMeta.background.value().gsize[1] == 0.0f) {
      pli->pliMeta.background.value().gsize[0] = pixmap->width();
      pli->pliMeta.background.value().gsize[1] = pixmap->width();
      QSize gSize(pli->pliMeta.background.value().gsize[0],
          pli->pliMeta.background.value().gsize[1]);
      int h_off = gSize.width() / 10;
      int v_off = gSize.height() / 8;
      pli->pliMeta.background.value().gpoints << QPointF(gSize.width() / 2, gSize.height() / 2)
                                              << QPointF(gSize.width() / 2 - h_off, gSize.height() / 2 - v_off);
    }
  setBackground( pixmap,
                 PartsListType,
                 pli->meta,
                 pli->pliMeta.background,
                 pli->pliMeta.border,
                 pli->pliMeta.margin,
                 pli->pliMeta.subModelColor,
                 submodelLevel,
                 toolTip);

  setData(ObjectId, PartsListBackgroundObj);
  setZValue(PARTSLISTBACKGROUND_ZVALUE_DEFAULT);
  setPixmap(*pixmap);
  setParentItem(parent);
  if (parentRelativeType != SingleStepType && pli->perStep) {
      setFlag(QGraphicsItem::ItemIsMovable,false);
    }
  setFlag(QGraphicsItem::ItemIsSelectable,true);
  setFlag(QGraphicsItem::ItemIsFocusable, true);
  setAcceptHoverEvents(true);
}

void PliBackgroundItem::placeGrabbers()
{
  if (grabbersVisible) {
      if (grabber) {
        scene()->removeItem(grabber);
        grabber = nullptr;
      }
    grabbersVisible = false;
    return;
  }

  QRectF rect = currentRect();
  point = QPointF(rect.left() + rect.width()/2,rect.bottom());
  if (grabber == nullptr) {
      grabber = new Grabber(BottomInside,this,myParentItem());
      grabber->setData(ObjectId, PliGrabberObj);
      grabber->top        = pli->top;
      grabber->bottom     = pli->bottom;
      grabber->stepNumber = pli->step ? pli->step->stepNumber.number : 0;
      grabbersVisible     = true;
    }
  grabber->setPos(point.x()-grabSize()/2,point.y()-grabSize()/2);
}

void PliBackgroundItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  position = pos();
  mouseIsDown = true;
  positionChanged = false;
  // we only want to toggle the grabbers off on second left mouse click
  if (event->button() != Qt::LeftButton) {
    grabbersVisible = false;
  }
  QGraphicsItem::mousePressEvent(event);
  placeGrabbers();
}

void PliBackgroundItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  positionChanged = true;
  QGraphicsItem::mouseMoveEvent(event);
  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {
    placeGrabbers();
  }
}

void PliBackgroundItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  mouseIsDown = false;
  QGraphicsItem::mouseReleaseEvent(event);

  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {

      QPointF newPosition;

      // back annotate the movement of the PLI into the LDraw file.
      newPosition = pos() - position;
      if (newPosition.x() || newPosition.y()) {
          positionChanged = true;
          PlacementData placementData = placement.value();
          placementData.offsets[XX] += newPosition.x()/pli->relativeToSize[XX];
          placementData.offsets[YY] += newPosition.y()/pli->relativeToSize[YY];
          placement.setValue(placementData);

          Where here, bottom;
          if (pli->autoRange(here, bottom))
              here = bottom;
          changePlacementOffset(here,&placement,pli->parentRelativeType);
        }
    }
}

void PliBackgroundItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = !this->isSelected() && !mouseIsDown;
    QGraphicsItem::hoverEnterEvent(event);
}

void PliBackgroundItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = false;
    QGraphicsItem::hoverLeaveEvent(event);
}

void PliBackgroundItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen pen;
    pen.setColor(isHovered ? QColor(Preferences::sceneGuideColor) : Qt::black);
    pen.setWidth(0/*cosmetic*/);
    pen.setStyle(isHovered ? Qt::PenStyle(Preferences::sceneGuidesLine) : Qt::NoPen);
    painter->setPen(pen);
    painter->setBrush(Qt::transparent);
    painter->drawRect(this->boundingRect());
    QGraphicsPixmapItem::paint(painter,option,widget);
}

void PliBackgroundItem::contextMenuEvent(
        QGraphicsSceneContextMenuEvent *event)
{
    if (pli) {
        QMenu menu;
        QString name = pli->bom ? QObject::tr("Bill Of Materials") : QObject::tr("Parts List");

        QAction *placementAction       = lpub->getAct("placementAction.1");
        PlacementData placementData    = pli->placement.value();
        placementAction->setWhatsThis(commonMenus.naturalLanguagePlacementWhatsThis(PartsListType,placementData,name));
        commonMenus.addAction(placementAction,menu,name);

        QAction *constrainAction       = lpub->getAct("constrainAction.1");
        commonMenus.addAction(constrainAction,menu,name);

        QAction *backgroundAction      = lpub->getAct("backgroundAction.1");
        commonMenus.addAction(backgroundAction,menu,name);

        QAction *subModelColorAction   = lpub->getAct("subModelColorAction.1");
        commonMenus.addAction(subModelColorAction,menu,name);

        QAction *borderAction          = lpub->getAct("borderAction.1");
        commonMenus.addAction(borderAction,menu,name);

        QAction *marginAction          = lpub->getAct("marginAction.1");
        commonMenus.addAction(marginAction,menu,name);

        QAction *pliPartGroupAction    = nullptr;
        if (pli->pliMeta.enablePliPartGroup.value()) {
            pliPartGroupAction         = lpub->getAct("partGroupsOffAction.1");
        } else {
            pliPartGroupAction         = lpub->getAct("partGroupsOnAction.1");
        }
        commonMenus.addAction(pliPartGroupAction,menu,name);

        QAction *sortAction            = lpub->getAct("sortAction.1");
        commonMenus.addAction(sortAction,menu,name);

        QAction *annotationAction      = lpub->getAct("annotationAction.1");
        if (pli->bom)
            annotationAction->setIcon(QIcon(":/resources/bomannotation.png"));
        commonMenus.addAction(annotationAction,menu,name);

        QAction *cameraAnglesAction    = lpub->getAct("cameraAnglesAction.1");
        commonMenus.addAction(cameraAnglesAction,menu,name);

        QAction *scaleAction           = lpub->getAct("scaleAction.1");
        commonMenus.addAction(scaleAction,menu,name);

        QAction *cameraFoVAction       = lpub->getAct("cameraFoVAction.1");
        commonMenus.addAction(cameraFoVAction,menu,name);

        QAction *rendererAction        = lpub->getAct("preferredRendererAction.1");
        commonMenus.addAction(rendererAction,menu,name);

        QAction *rendererArgumentsAction       = nullptr;
        QAction *povrayRendererArgumentsAction = nullptr;
        bool usingPovray = Preferences::preferredRenderer == RENDERER_POVRAY;
        if (Preferences::preferredRenderer != RENDERER_NATIVE) {
            rendererArgumentsAction    = lpub->getAct("rendererArgumentsAction.1");
            commonMenus.addAction(rendererArgumentsAction,menu,name);
            if (usingPovray) {
                povrayRendererArgumentsAction = lpub->getAct("povrayRendererArgumentsAction.1");
                commonMenus.addAction(povrayRendererArgumentsAction,menu,name);
            }
        }

        QAction *splitBomAction        = nullptr;
        QAction *deleteBomAction       = nullptr;
        QAction *refreshPartsCacheAction = nullptr;
        if (!pli->bom) {
            refreshPartsCacheAction      = lpub->getAct("refreshPartsCacheAction.1");
            commonMenus.addAction(refreshPartsCacheAction,menu,name);
        } else {
            splitBomAction             = lpub->getAct("splitBomAction.1");
            commonMenus.addAction(splitBomAction,menu);

            deleteBomAction            = lpub->getAct("deleteBomAction.1");
            commonMenus.addAction(deleteBomAction,menu,name);
        }

        QAction *selectedAction        = menu.exec(event->screenPos());

        if (selectedAction == nullptr) {
            return;
        }

        Where top = pli->top;
        Where bottom = pli->bottom;

        name = pli->bom ? QObject::tr("BOM") : QObject::tr("PLI");

        if (selectedAction == sortAction) {
            changePliSort(QObject::tr("%1 Sort Order and Direction").arg(name),
                          top,
                          bottom,
                          &pli->pliMeta.sortOrder);
        } else if (selectedAction == annotationAction) {
            changePliAnnotation(QObject::tr("%1 Annotaton").arg(name),
                                top,
                                bottom,
                                &pli->pliMeta.annotation);
        } else if (selectedAction == constrainAction) {
            changeConstraint(QObject::tr("%1 Constraint").arg(name),
                             top,
                             bottom,
                             &pli->pliMeta.constrain);
        } else if (selectedAction == placementAction) {
            if (pli->step) {
                if (pli->step->showStepNumber) {
                    pli->placement.setStepNumberShown(true);
                    if (pli->step->stepNumber.placement.value().relativeTo == PartsListType)
                        pli->placement.setValue(StepNumberType);
                }
                pli->placement.setSubModelShown(pli->step->placeSubModel);
                pli->placement.setRotateIconShown(pli->step->placeRotateIcon);
            } else if (pli->steps && !pli->perStep) {
                if(pli->steps->groupStepMeta.LPub.multiStep.showGroupStepNumber.value()) {
                    pli->placement.setStepNumberShown(true);
                    if (pli->steps->groupStepNumber.placement.value().relativeTo == PartsListType)
                        pli->placement.setValue(StepNumberType);
                }
                pli->placement.setSubModelShown(pli->steps->placeSubModel);
                // To place rotate icon at page level can be done if requested
                pli->placement.setRotateIconShown(false);
            }
            pli->placement.setPartsListPerStep(pli->perStep);
            if (pli->bom) {
                pli->pliMeta.placement.setPartsListPerStep(pli->perStep);
                changePlacement(parentRelativeType,
                                BomType,
                                QObject::tr("%1 Placement").arg(name),
                                top,
                                bottom,
                                &pli->pliMeta.placement,true,1,0,false);
            } else if (pli->perStep) {
                changePlacement(parentRelativeType,
                                PartsListType,
                                QObject::tr("%1 Placement").arg(name),
                                top,
                                bottom,
                                &pli->placement);
            } else {
                changePlacement(parentRelativeType,
                                PartsListType,
                                QObject::tr("%1 Placement").arg(name),
                                top,
                                bottom,
                                &pli->placement,true,1,0,false);
            }
        } else if (selectedAction == marginAction) {
            changeMargins(QObject::tr("%1 Margins").arg(name),
                          top,
                          bottom,
                          &pli->margin);
        } else if (selectedAction == pliPartGroupAction) {
            togglePartGroups(
                        top,
                        bottom,
                        pli->bom,
                        &pli->pliMeta.enablePliPartGroup);
        } else if (selectedAction == backgroundAction) {
            changeBackground(QObject::tr("%1 Background").arg(name),
                             top,
                             bottom,
                             &pli->pliMeta.background);
        } else if (selectedAction == subModelColorAction) {
            changeSubModelColor(QObject::tr("%1 Background Color").arg(name),
                                top,
                                bottom,
                                &pli->pliMeta.subModelColor);
        } else if (selectedAction == borderAction) {
          changeBorder(QObject::tr("%1 Border").arg(name),
                       top,
                       bottom,
                       &pli->pliMeta.border);
        } else if (selectedAction == scaleAction) {
            changeFloatSpin(QObject::tr("%1 Scale").arg(name),
                            QObject::tr("Model Size"),
                            top,
                            bottom,
                            &pli->pliMeta.modelScale,
                            0.01f,1,true, // step, append, checklocal
                            DoubleSpinScale);
        } else if (selectedAction == cameraFoVAction) {
            changeCameraFOV(QObject::tr("%1 Field Of View").arg(name),
                            QObject::tr("FOV"),
                            top,
                            bottom,
                            &pli->pliMeta.cameraFoV,
                            &pli->pliMeta.cameraZNear,
                            &pli->pliMeta.cameraZFar);
        } else if (selectedAction == cameraAnglesAction) {
            changeCameraAngles(QObject::tr("%1 Camera Angles").arg(name),
                               top,
                               bottom,
                               &pli->pliMeta.cameraAngles);
        }  else if (selectedAction == rendererAction) {
            if (pli->bom) {
                changePreferredRenderer(QObject::tr("%1 Preferred Renderer").arg(name),
                                        top,
                                        bottom,
                                        &pli->meta->LPub.bom.preferredRenderer);
            } else {
                changePreferredRenderer(QObject::tr("%1 Preferred Renderer").arg(name),
                                        top,
                                        bottom,
                                        &pli->meta->LPub.pli.preferredRenderer);
            }
        } else if (selectedAction == deleteBomAction) {
            deleteBOM();
        } else if (selectedAction == splitBomAction) {
            insertSplitBOM();
        }  else if (selectedAction == rendererArgumentsAction) {
            const QString rendererLabel = QObject::tr("Add %1 Arguments")
                                                     .arg(usingPovray ? QObject::tr("POV Generation"):
                                                                        QObject::tr("%1 Renderer").arg(rendererNames[Render::getRenderer()]));
            StringMeta rendererArguments =
                    Render::getRenderer() == RENDERER_LDVIEW ? pli->pliMeta.ldviewParms :
                    Render::getRenderer() == RENDERER_LDGLITE ? pli->pliMeta.ldgliteParms :
                                  /*POV scene file generator*/  pli->pliMeta.ldviewParms ;
            setRendererArguments(top,
                                 bottom,
                                 rendererLabel,
                                 &rendererArguments);
        } else if (selectedAction == povrayRendererArgumentsAction) {
            setRendererArguments(top,
                                 bottom,
                                 rendererNames[Render::getRenderer()],
                                 &pli->pliMeta.povrayParms);
        } else if (selectedAction == refreshPartsCacheAction) {
            if (pli->step)
                clearStepCache(pli->step,Options::SMI);
            else if (pli->steps && !pli->perStep)
                clearPageCache(pli->parentRelativeType,dynamic_cast<Page *>(pli->steps), Options::PLI);
        }
    }
}

/*
 * Code for resizing the PLI - part of the resize class described in
 * resize.h
 */

void PliBackgroundItem::resize(QPointF grabbed)
{
  if (!pli)
      return;

  // recalculate corners Y

  point = grabbed;

  // Figure out desired height of PLI

  if (pli->parentRelativeType == CalloutType) {
      QPointF absPos = pos();
      absPos = mapToScene(absPos);
      grabHeight = int(grabbed.y() - absPos.y());
    } else {
      grabHeight = int(grabbed.y() - pos().y());
    }

  ConstrainData constrainData;
  constrainData.type = ConstrainData::PliConstrainHeight;
  constrainData.constraint.height = grabHeight;

  pli->resizePli(pli->meta, constrainData);

  qreal width = pli->size[0];
  qreal height = pli->size[1];
  qreal scaleX = width/size[0];
  qreal scaleY = height/size[1];

  pli->positionChildren(int(height),scaleX,scaleY);

  point = QPoint(int(pos().x()+width/2),int(pos().y()+height));
  grabber->setPos(point.x()-grabSize()/2,point.y()-grabSize()/2);

  resetTransform();
  setTransform(QTransform::fromScale(scaleX,scaleY),true);

  QList<QGraphicsItem *> kids = childItems();

  for (int i = 0; i < kids.size(); i++) {
      kids[i]->resetTransform();
      kids[i]->setTransform(QTransform::fromScale(1.0/scaleX,1.0/scaleY),true);
    }

  sizeChanged = true;
}

void PliBackgroundItem::change()
{
  ConstrainData constrainData;

  constrainData.type = ConstrainData::PliConstrainHeight;
  constrainData.constraint.height = int(grabHeight);

  pli->pliMeta.constrain.setValue(constrainData);

  Where top, bottom;
  bool useBot;

  // for single step with BOM, we have to do something special

  useBot = pli->autoRange(top,bottom);
  int append = 1;
  if (pli->bom && pli->steps->relativeType == SingleStepType && pli->steps->list.size() == 1) {
      Range *range = dynamic_cast<Range *>(pli->steps->list[0]);
      if (range->list.size() == 1) {
          append = 0;
        }
    }

  changeConstraint(top,bottom,&pli->pliMeta.constrain,append,useBot);
}

QRectF PliBackgroundItem::currentRect()
{
  if (pli->parentRelativeType == CalloutType) {
      QRectF foo (pos().x(),pos().y(),size[0],size[1]);
      return foo;
    } else {
      return sceneBoundingRect();
    }
}

void AnnotateTextItem::contextMenuEvent(
    QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;

  QString name =  isElement ? QObject::tr("Part Element") : QObject::tr("Part Annotation");

  QAction *fontAction       = lpub->getAct("fontAction.1");
  commonMenus.addAction(fontAction,menu,name);

  QAction *colorAction      = lpub->getAct("colorAction.1");
  commonMenus.addAction(colorAction,menu,name);

  QAction *borderAction     = lpub->getAct("borderAction.1");
  commonMenus.addAction(borderAction,menu,name);

  QAction *backgroundAction = lpub->getAct("backgroundAction.1");
  commonMenus.addAction(backgroundAction,menu,name);

//  QAction *marginAction     = lpub->getAct("marginAction.1");
//  commonMenus.addAction(marginAction,menu,name);

//  QAction *sizeAction       = lpub->getAct("sizeAction.1");
//  commonMenus.addAction(sizeAction,menu,name);

  QAction *selectedAction   = menu.exec(event->screenPos());

  if (selectedAction == nullptr) {
      return;
  }

  Where top = pli->top;
  Where bottom = pli->bottom;

  if (selectedAction == fontAction) {
      changeFont(top,
                 bottom,
                 &font);
  } else if (selectedAction == colorAction) {
      changeColor(top,
                  bottom,
                  &color);
  } else if (selectedAction == backgroundAction) {
      changeBackground(QObject::tr("%1 Background").arg(name),
                       top,
                       bottom,
                       &background,
                       true,1,true,false);  // no picture
  } else if (selectedAction == borderAction) {
      bool corners = style.value() != AnnotationStyle::circle;
      changeBorder(QObject::tr("%1 Border").arg(name),
                   top,
                   bottom,
                   &border,
                   true/*useTop*/,1/*append*/,true/*local*/,false/*rotateArrow*/,corners);
  }
//    else if (selectedAction == marginAction) {
//      changeMargins(QObject::tr("%1 Margins")arg(name),
//                    top,
//                    bottom,
//                    &margin);
//  } else if (selectedAction == sizeAction) {
//      changeSize(QObject::tr("%1 Size").arg(name),
//                   "Width",
//                   "Height",
//                   top,
//                   bottom,
//                   &styleMeta->size);
//  }
}

void InstanceTextItem::contextMenuEvent(
    QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;

  QString name = QObject::tr("Parts Count");

  QAction *fontAction       = lpub->getAct("fontAction.1");
  commonMenus.addAction(fontAction,menu,name);

  QAction *colorAction      = lpub->getAct("colorAction.1");
  commonMenus.addAction(colorAction,menu,name);

  QAction *marginAction     = lpub->getAct("marginAction.1");
  commonMenus.addAction(marginAction,menu,name);

  QAction *selectedAction   = menu.exec(event->screenPos());

  if (selectedAction == nullptr) {
      return;
  }

  Where top = pli->top;
  Where bottom = pli->bottom;

  if (selectedAction == fontAction) {
    changeFont(top,bottom,&pli->pliMeta.instance.font,1,false);
  } else if (selectedAction == colorAction) {
    changeColor(top,bottom,&pli->pliMeta.instance.color,1,false);
  } else if (selectedAction == marginAction) {
    changeMargins(QObject::tr("%1 Margins"),top,bottom,&pli->pliMeta.instance.margin,true,1,false);
  }
}

PGraphicsPixmapItem::PGraphicsPixmapItem(
  Pli     *_pli,
  PliPart *_part,
  QPixmap &pixmap,
  PlacementType _parentRelativeType,
  QString &type,
  QString &color) :
    isHovered(false),
    mouseIsDown(false),
    canUpdatePreview(false)
{
  parentRelativeType = _parentRelativeType;
  pli = _pli;
  part = _part;
  bool isSub = _part->subType;
  setPixmap(pixmap);
  setFlag(QGraphicsItem::ItemIsSelectable,true);
  setFlag(QGraphicsItem::ItemIsFocusable, true);
  setAcceptHoverEvents(true);
  setToolTip(pliToolTip(type,color,isSub));
  setData(ObjectId, PartsListPixmapObj);
  setZValue(PARTSLISTPARTPIXMAP_ZVALUE_DEFAULT);
}

void PGraphicsPixmapItem::previewPart(bool useDockable) {
    if (!part)
        return;

    lcPreferences& Preferences = lcGetPreferences();
    bool dockable = Preferences.mPreviewPosition == lcPreviewPosition::Dockable && useDockable;
    QPoint position;
    if (!dockable) {
        QPointF sceneP;
        switch (Preferences.mPreviewLocation)
        {
        case lcPreviewLocation::TopRight:
            sceneP = pli->background->mapToScene(pli->background->boundingRect().topRight());
            break;
        case lcPreviewLocation::TopLeft:
            sceneP = pli->background->mapToScene(pli->background->boundingRect().topLeft());
            break;
        case lcPreviewLocation::BottomRight:
            sceneP = pli->background->mapToScene(pli->background->boundingRect().bottomRight());
            break;
        default:
            sceneP = pli->background->mapToScene(pli->background->boundingRect().bottomLeft());
            break;
        }
        QGraphicsView *view = pli->background->scene()->views().first();
        QPoint viewP = view->mapFromScene(sceneP);
        position = view->viewport()->mapToGlobal(viewP);
    } else {
        gui->RaisePreviewDockWindow();
    }

    gui->showLine(pli->topOfStep());

    gui->PreviewPiece(part->type, part->color.toInt(), dockable, QRect(), position);
}

void PGraphicsPixmapItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = !this->isSelected() && !mouseIsDown;
    QGraphicsItem::hoverEnterEvent(event);
}

void PGraphicsPixmapItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = false;
    QGraphicsItem::hoverLeaveEvent(event);
}

void PGraphicsPixmapItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseDoubleClickEvent(event);
    if ( event->button() == Qt::LeftButton )
    {
        lcPreferences& Preferences = lcGetPreferences();
        if (Preferences.mPreviewEnabled && Preferences.mPreviewPosition == lcPreviewPosition::Floating)
        {
            previewPart();
            canUpdatePreview = true;
        }
    }
}

void PGraphicsPixmapItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if ( event->button() == Qt::LeftButton ) {
        lcPreferences& Preferences = lcGetPreferences();
        if (!Preferences.mPreviewEnabled) {
            if (gui->saveBuildModification()) {
                QString type = QFileInfo(part->type).completeBaseName();
                QString viewerPliPartKey = QString("%1;%2;%3")
                        .arg(type, part->color)
                        .arg(pli->step ? pli->step->stepNumber.number : 0/*BOM page*/);
                QString partKey = gui->getViewerStepKey();
                bool havePartKey = !partKey.isEmpty();
                QString viewerOptKey = QString("%1_%2").arg(type, part->color);
                pli->viewerOptions = pli->viewerOptsList[viewerOptKey];
                pli->viewerOptions->ImageWidth  = part->pixmapWidth;
                pli->viewerOptions->ImageHeight = part->pixmapHeight;
                if (havePartKey && partKey != viewerPliPartKey) {
                    pli->loadTheViewer();
                    canUpdatePreview = true;
                }
            }
        } else if (Preferences.mPreviewPosition == lcPreviewPosition::Dockable) {
            previewPart(true/*Dockable*/);
            canUpdatePreview = true;
        }
    }

    mouseIsDown = true;
    QGraphicsItem::mousePressEvent(event);
    update();
}

void PGraphicsPixmapItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    mouseIsDown = false;
    QGraphicsItem::mouseReleaseEvent(event);
    update();
}

void PGraphicsPixmapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen pen;
    pen.setColor(isHovered ? QColor(Preferences::sceneGuideColor) : Qt::black);
    pen.setWidth(0/*cosmetic*/);
    pen.setStyle(isHovered ? Qt::PenStyle(Preferences::sceneGuidesLine) : Qt::NoPen);
    painter->setPen(pen);
    painter->setBrush(Qt::transparent);
    painter->drawRect(this->boundingRect());
    QGraphicsPixmapItem::paint(painter,option,widget);
}

void PGraphicsPixmapItem::contextMenuEvent(
    QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;
  QFontMetrics currentMetrics(gui->font());
  QString elidedPartName = currentMetrics.elidedText(part->type, Qt::ElideRight, currentMetrics.averageCharWidth()*15);
  QString name = QObject::tr("Part %1").arg(elidedPartName);
  // Text elided to 15 chars
  //QString name = QObject::tr("Part %1")
  //                           .arg(this->part->type.size() > 15 ?
  //                                    this->part->type.left(12) + "..." +
  //                                    this->part->type.right(3) : this->part->type);

  QAction *substitutePartAction       = nullptr;
  QAction *removeSubstitutePartAction = nullptr;
  if (this->part->subType) {
      substitutePartAction            = lpub->getAct("changeSubstitutePartAction.1");
      commonMenus.addAction(substitutePartAction,menu,name);

      removeSubstitutePartAction      = lpub->getAct("removeSubstitutePartAction.1");
      commonMenus.addAction(removeSubstitutePartAction,menu,name);
  } else {
      substitutePartAction            = lpub->getAct("substitutePartAction.1");
      commonMenus.addAction(substitutePartAction,menu,name);
  }

  QAction *hideAction                 = lpub->getAct("hideAction.1");
  commonMenus.addAction(hideAction,menu,name);

  QAction *marginAction               = lpub->getAct("marginAction.1");
  commonMenus.addAction(marginAction,menu,name);

  QAction *resetPartGroupAction       = nullptr;
  if (pli->pliMeta.enablePliPartGroup.value()) {
      resetPartGroupAction            = lpub->getAct("resetPartGroupAction.1");
      commonMenus.addAction(resetPartGroupAction,menu,name);
  }

  QAction *resetViewerImageAction     = nullptr;
  if (canUpdatePreview) {
      menu.addSeparator();
      resetViewerImageAction          = lpub->getAct("resetViewerImageAction.1");
      commonMenus.addAction(resetViewerImageAction,menu,name);
  }

  QAction *copyPliImagePathAction     = nullptr;
#ifndef QT_NO_CLIPBOARD
  menu.addSeparator();
  copyPliImagePathAction              = lpub->getAct("copyToClipboardAction.1");
  commonMenus.addAction(copyPliImagePathAction,menu,name);
#endif

  QAction *previewPartAction          = lpub->getAct("previewPartAction.1");
  lcPreferences& Preferences = lcGetPreferences();
  previewPartAction->setEnabled(Preferences.mPreviewEnabled);
  commonMenus.addAction(previewPartAction,menu,name);

// Manipulate individual PLI images
  //QAction *cameraAnglesAction  = lpub->getAct("cameraAnglesAction.1");
  //commonMenus.addAction(cameraAnglesAction,menu,name);

  //QAction *scaleAction  = lpub->getAct("scaleAction.1");
  //commonMenus.addAction(scaleAction,menu,name);

  //QAction *cameraFoVAction  = lpub->getAct("cameraFoVAction.1");
  //commonMenus.addAction(cameraFoVAction,menu,name);

  QAction *selectedAction             = menu.exec(event->screenPos());

  if (selectedAction == nullptr) {
      return;
  }

  Where top = pli->top;
  Where bottom = pli->bottom;

  if (selectedAction == marginAction) {

      changeMargins(QObject::tr("%1 Margins").arg(name),
                    top,
                    bottom,
                    &pli->pliMeta.part.margin);
  } else if (selectedAction == previewPartAction) {
      previewPart(true /*previewPartAction*/);
  } else if (selectedAction == hideAction) {
      hidePLIParts(this->part->instances);
  } else if (selectedAction == resetPartGroupAction) {
      resetPartGroup(part->groupMeta.here());
  } else if (selectedAction == removeSubstitutePartAction) {
      QStringList attributes;
      attributes.append(this->part->type);
      attributes.append(this->part->color);
      substitutePLIPart(attributes,this->part->instances,sRemove);
  } else if (selectedAction == substitutePartAction) {
      QStringList defaultList;
      if (this->part->subType/*sUpdate*/) {
          float modelScale = this->pli->pliMeta.modelScale.value();
          bool customViewpoint = this->pli->pliMeta.cameraAngles.customViewpoint();
          bool noCA = !customViewpoint && this->pli->pliMeta.rotStep.value().type.toUpper() == QLatin1String("ABS");
          defaultList.append(QString::number(double(modelScale)));
          defaultList.append(QString::number(double(this->pli->pliMeta.cameraFoV.value())));
          defaultList.append(QString::number(noCA ? double(0.0f) : double(this->pli->pliMeta.cameraAngles.value(0))));
          defaultList.append(QString::number(noCA ? double(0.0f) : double(this->pli->pliMeta.cameraAngles.value(1))));
          defaultList.append(QString(QString("%1 %2 %3")
                                     .arg(double(this->pli->pliMeta.target.x()))
                                     .arg(double(this->pli->pliMeta.target.y()))
                                     .arg(double(this->pli->pliMeta.target.z()))).split(" "));
          defaultList.append(QString(renderer->getRotstepMeta(this->pli->pliMeta.rotStep,true)).split("_"));
      }

      QStringList attributes;
      // treat parts with '_' in the name - encode
      if (this->part->type.count("_")) {
          QString nameKey = this->part->nameKey;
          const QString type = QFileInfo(this->part->type).completeBaseName();
          nameKey.replace(type, QString(type).replace("_", ";"));
          attributes = nameKey.split("_");
          attributes.replace(0,type);
      } else {
          attributes = this->part->nameKey.split("_");
      }
      attributes.removeAt(nResType);
      attributes.removeAt(nResolution);
      attributes.removeAt(nPageWidth);
      attributes.replace(nType,this->part->type);
      if (attributes.size() == nAdjustedBaseAttributes        /*BaseAttributes - removals*/)
          attributes.append(QString("0 0 0 0 0 0 REL").split(" "));
      else if (attributes.size() == nAdjustedTarget           /*Target - removals*/)
          attributes.append(QString("0 0 0 REL").split(" ")); /*13 items total without substituted part [new substitution]*/
      if (!this->part->subOriginalType.isEmpty())
          attributes.append(this->part->subOriginalType);           /*14 items total with substituted part [update substitution]*/
      substitutePLIPart(attributes,this->part->instances,this->part->subType ? sUpdate : sSubstitute,defaultList);
  } else if (selectedAction == resetViewerImageAction) {
      if (!Preferences.mPreviewEnabled) {
          if (gui->saveBuildModification()) {
              QString type = QFileInfo(part->type).completeBaseName();
              QString viewerOptKey = QString("%1_%2").arg(type, part->color);
              lpub->saveVisualEditorTransformSettings();
              pli->viewerOptions = pli->viewerOptsList[viewerOptKey];
              pli->viewerOptions->ImageWidth  = part->pixmapWidth;
              pli->viewerOptions->ImageHeight = part->pixmapHeight;
              pli->viewerOptions->IsReset     = true;
              pli->loadTheViewer();
          }
      } else if (Preferences.mPreviewPosition == lcPreviewPosition::Dockable) {
          gui->updatePreview();
      }
  } else if (selectedAction == copyPliImagePathAction) {
      QObject::connect(copyPliImagePathAction, SIGNAL(triggered()), gui, SLOT(updateClipboard()));
      copyPliImagePathAction->setData(pli->imageName);
      emit copyPliImagePathAction->triggered();
  } /*else if (selectedAction == cameraAnglesAction) {
      changeCameraAngles(QObject::tr("%1 Camera Angles").arg(name),
                      top,
                      bottom,
                      &pli->pliMeta.cameraAngles);
  } else if (selectedAction == scaleAction) {
      changeFloatSpin(name,
                      "Model Size",
                      top,
                      bottom,
                      &pli->pliMeta.modelScale);
  } else if (selectedAction == cameraFoVAction) {
      changeFloatSpin(name,
                      "Camera FOV",
                      top,
                      bottom,
                      &pli->pliMeta.cameraFoV);
  } */
}

//-----------------------------------------
//-----------------------------------------

//-----------------------------------------
//-----------------------------------------
//-----------------------------------------

InstanceTextItem::InstanceTextItem(
  Pli                *_pli,
  PliPart            *_part,
  QString            &text,
  QString            &fontString,
  QString            &colorString,
  PlacementType      _parentRelativeType,
  PGraphicsTextItem *_parent)
    : PGraphicsTextItem(_parent),
      isHovered(false),
      mouseIsDown(false)
{
  parentRelativeType = _parentRelativeType;
  QString toolTip(tr("%1 Times Used - right-click to modify").arg(_part->type));
  setText(_pli,_part,text,fontString,toolTip);
  QColor color(colorString);
  setDefaultTextColor(color);

  setFlag(QGraphicsItem::ItemIsSelectable,true);
  setFlag(QGraphicsItem::ItemIsFocusable, true);
  setAcceptHoverEvents(true);

  setData(ObjectId, PartsListInstanceObj);
  setZValue(PARTSLISTINSTANCE_ZVALUE_DEFAULT);
}

void InstanceTextItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = !this->isSelected() && !mouseIsDown;
    QGraphicsItem::hoverEnterEvent(event);
}

void InstanceTextItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = false;
    QGraphicsItem::hoverLeaveEvent(event);
}

void InstanceTextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    mouseIsDown = true;
    QGraphicsItem::mousePressEvent(event);
    update();
}

void InstanceTextItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    mouseIsDown = false;
    QGraphicsItem::mouseReleaseEvent(event);
    update();
}

void InstanceTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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

//-----------------------------------------
//-----------------------------------------

AnnotateTextItem::AnnotateTextItem(
  Pli           *_pli,
  PliPart       *_part,
  QString       &_text,
  QString       &_fontString,
  QString       &_colorString,
  PlacementType  _parentRelativeType,
  bool           _element,
  PGraphicsTextItem *_parent)
    : PGraphicsTextItem( _parent ),
      alignment( Qt::AlignCenter | Qt::AlignVCenter ),
      isHovered(false),
      mouseIsDown(false)
{
  parentRelativeType = _parentRelativeType;
  isElement          = _element;

  QString fontString = _fontString;

  QString toolTip;

  UnitsMeta styleSize;

  if (isElement) {
      bool enableStyle        = _pli->pliMeta.annotation.enableStyle.value();
      bool enableElementStyle = _pli->pliMeta.annotation.elementStyle.value();
      if (enableStyle && enableElementStyle) {
         border     = _pli->pliMeta.elementStyle.border;
         background = _pli->pliMeta.elementStyle.background;
         style      = _pli->pliMeta.elementStyle.style;
         styleSize  = _pli->pliMeta.elementStyle.size;
      } else {
         border     = _pli->pliMeta.defaultStyle.border;     // Type::BdrNone, Line::BdrLnNone
         background = _pli->pliMeta.defaultStyle.background; // BackgroundData::BgTransparent
         style      = _pli->pliMeta.defaultStyle.style;      // AnnotationStyle::none
         styleSize  = _pli->pliMeta.defaultStyle.size;       // 0.28f,0.28f (42px @ 150DPI)
      }
      font       = _pli->pliMeta.elementStyle.font;
      color      = _pli->pliMeta.elementStyle.color;
      margin     = _pli->pliMeta.elementStyle.margin;
      toolTip = tr("%1 Element Annotation %2 %3 (%4) \"%5\" - right-click to modify")
                       .arg(_pli->pliMeta.partElements.legoElements.value() ? tr("LEGO") : tr("BrickLink"),
                            _part->type,
                            LDrawColor::name(_part->color),
                            _part->color,
                            _part->description);
  } else {
      border     = _part->styleMeta.border;
      background = _part->styleMeta.background;
      style      = _part->styleMeta.style;
      font       = _part->styleMeta.font;
      color      = _part->styleMeta.color;
      margin     = _part->styleMeta.margin;
      styleSize  = _part->styleMeta.size;
      toolTip    = tr("%1 Part Annotation %2 %3 (%4) \"%5\" - right-click to modify")
                      .arg(_pli->bom ? tr("BOM") : tr("PLI"),
                           _part->type,
                           LDrawColor::name(_part->color),
                           _part->color,
                           _part->description);
  }

  canSetAnnotationStyle.setValue(background.value().type != BackgroundData::BgTransparent ||
                                 (border.valuePixels().type != BorderData::BdrNone &&
                                  border.valuePixels().line != BorderData::BdrLnNone));

  setText(_pli,_part,_text,fontString,toolTip);

  QColor color(_colorString);
  setDefaultTextColor(color);

  textRect  = QRectF(0,0,document()->size().width(),document()->size().height());

  if (style.value() == AnnotationStyle::none) {
      styleRect = textRect;
  } else {
      // set rectangle size and dimensions parameters
      bool fixedStyle  = _part->styleMeta.style.value() != AnnotationStyle::rectangle && !isElement;
      bool isRectangle = _part->styleMeta.style.value() == AnnotationStyle::rectangle;
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
      QRectF _styleRect = QRectF(0,0,fixedStyle ? styleSize.valuePixels(XX) : isRectangle ? rectSize.valuePixels(XX) : textRect.width(),
                                     fixedStyle ? styleSize.valuePixels(YY) : isRectangle ? rectSize.valuePixels(YY) : textRect.height());
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

  subModelColor = pli->pliMeta.subModelColor;
  if (pli->background)
    submodelLevel = pli->background->submodelLevel;

  setFlag(QGraphicsItem::ItemIsSelectable,true);
  setFlag(QGraphicsItem::ItemIsFocusable, true);
  setAcceptHoverEvents(true);

  setData(ObjectId, PartsListAnnotationObj);
  setZValue(PARTSLISTANNOTATION_ZVALUE_DEFAULT);
}

void AnnotateTextItem::scaleDownFont() {
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

    emit gui->messageSig(LOG_INFO,QObject::tr("PLI annotation font size was adjusted from %1 to %2.")
                                              .arg(saveFontSizeF).arg(font.pointSizeF()));
  }
}

void AnnotateTextItem::size(int &x, int &y)
{
    x = int(styleRect.width());
    y = int(styleRect.height());
}

void AnnotateTextItem::setAnnotationStyle(QPainter *painter)
{
    QPixmap *pixmap = new QPixmap(int(styleRect.width()),int(styleRect.height()));

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
    qreal bt = borderData.thickness;
    QRectF bgRect(bt/2,bt/2,pixmap->width()-bt,pixmap->height()-bt);
    if (style.value() != AnnotationStyle::circle) {
        if (borderData.type == BorderData::BdrRound) {
            // set icon border dimensions
            qreal rx = double(borderData.radius);
            qreal ry = double(borderData.radius);
            qreal dx = pixmap->width();
            qreal dy = pixmap->height();
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

void AnnotateTextItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = !this->isSelected() && !mouseIsDown;
    QGraphicsItem::hoverEnterEvent(event);
}

void AnnotateTextItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = false;
    QGraphicsItem::hoverLeaveEvent(event);
}

void AnnotateTextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    mouseIsDown = true;
    QGraphicsItem::mousePressEvent(event);
    update();
}

void AnnotateTextItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    mouseIsDown = false;
    QGraphicsItem::mouseReleaseEvent(event);
    update();
}

void AnnotateTextItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (canSetAnnotationStyle.value()) {
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
    QGraphicsTextItem::paint(painter, option, widget);
}

PartGroupItem::PartGroupItem(PliPartGroupMeta meta)
: meta(meta)
{
    setHandlesChildEvents(false);
    setFlag(QGraphicsItem::ItemIsSelectable,true);
    setFlag(QGraphicsItem::ItemIsMovable,true);

    setData(ObjectId, PartsListGroupObj);
    setZValue(PARTSLISTPARTGROUP_ZVALUE_DEFAULT);
}
