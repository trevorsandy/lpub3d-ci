
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
 * The traverse function is the one function that traverses the LDraw model
 * higherarchy seaching for pages to render.  It tracks the partial assembly
 * contents, parts list contents, step group contents, and callouts.
 *
 * It can count pages in the design, gather page contents for translation
 * into graphical representation of pages for the user.  In the future it
 * will gather Bill of Materials contents.
 *
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/

#include "lpub.h"
#include <QtWidgets>
#include <QGraphicsItem>
#include <QString>
#include <QFileInfo>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtConcurrent>
#endif

#include "lpub_preferences.h"
#include "ranges.h"
#include "callout.h"
#include "pointer.h"
#include "range.h"
#include "reserve.h"
#include "step.h"
#include "paths.h"
#include "metaitem.h"
#include "pointer.h"
#include "pagepointer.h"
#include "ranges_item.h"
#include "separatorcombobox.h"
#include "waitingspinnerwidget.h"
#include "lc_application.h"

// Set to enable write parts output file for debugging // TODO: delete from header
#ifndef WRITE_PARTS_DEBUG
//#define WRITE_PARTS_DEBUG
#endif

#define FIRST_STEP 1
#define FIRST_PAGE 1

QString Gui::AttributeNames[] =
{
    "Line",
    "Border"
};

QString Gui::PositionNames[] =
{
    "BASE_TOP",
    "BASE_BOTTOM",
    "BASE_LEFT",
    "BASE_RIGHT"
};

//struct PAMItem
//{
//    PointerAttribMeta pam;
//    Positions pos;
//};

/*********************************************
 *
 * remove_group
 *
 * this removes members of a group from the
 * ldraw file held in the the ldr string
 *
 ********************************************/
void Gui::remove_group(
    QStringList  in,     // csiParts
    QVector<int> tin,    // typeIndexes
    QString      group,  // steps->meta.LPub.remove.group.value()
    QStringList  &out,   // newCSIParts
    QVector<int> &tiout, // newTypeIndexes
    Meta         *meta)
{
  Q_UNUSED(meta)

  bool    grpMatch = false;
  bool    exclude  = false;
  bool    tinNull  = false;
  int     grpLevel = 0;
  Rc      grpType  = OkRc;
  QString line, grpData;
  QStringList lids;

  if ((tinNull = in.size() != tin.size())) {
    QString const message(tr("CSI part list size [%1] does not match line index size [%2].")
                             .arg(in.size()).arg(tin.size()));
    emit gui->messageSig(LOG_NOTICE, message);
  }

  auto parseGroupMeta = [&line](Rc &grpType)
  {
    QHash<Rc, QRegularExpression>::const_iterator i = groupRegExMap.constBegin();
    QRegularExpressionMatch match;
    while (i != groupRegExMap.constEnd()) {
      match = i.value().match(line);
      if (match.hasMatch()) {
        grpType = i.key();
        return match.captured(i.value().captureCount());
      }
      ++i;
    }
    return QString();
  };

  for (int i = 0; i < in.size(); i++) {

    line = in.at(i);

    grpData = parseGroupMeta(grpType);

    switch (grpType)
    {
      case MLCadGroupRc:
        if ((exclude = grpData == group))
          i++;
        break;
      case LDCadGroupRc:
        lids = grpData.split(" ");
        if ((exclude = lids.size() && lpub->ldrawFile.ldcadGroupMatch(group, lids)))
          i++;
        break;
      case LeoCadGroupBeginRc:
        if ((exclude = grpData == group)) {
          grpMatch = true;
          i++;
        }
        else if ((exclude = grpMatch)) {
          grpLevel++;
          i++;
        }
        break;
      case LeoCadGroupEndRc:
        if ((exclude = grpMatch)) {
          if (grpLevel == 0) {
            grpMatch = false;
          } else {
            grpLevel--;
          }
        }
        break;
      default:
        break;
    }

    if (grpMatch)
      i++;
    else
    if (!exclude) {
      out << line;
      if (!tinNull)
        tiout << tin.at(i);
    }
  }

  return;
}

/*********************************************
 *
 * remove_part
 *
 * this removes members of a part from the
 * ldraw file held in the the ldr string
 *
 ********************************************/

void Gui::remove_parttype(
    QStringList   in,    // csiParts
    QVector<int>  tin,   // typeIndexes
    QString       model, // part type
    QStringList  &out,   // newCSIParts
    QVector<int> &tiout) // newTypeIndexes
{
  bool exclude  = false;
  bool tinNull  = false;

  if ((tinNull = in.size() != tin.size())) {
    QString const message(tr("CSI part list size [%1] does not match line index size [%2].")
                            .arg(in.size()).arg(tin.size()));
    emit gui->messageSig(LOG_NOTICE, message);
  }

  for (int i = 0; i < in.size(); i++) {
    QString line = in.at(i);
    QStringList tokens;
    split(line,tokens);

    if (tokens.size() == 15 && tokens[0] == "1") {
      exclude = tokens[14].toLower() == model.toLower();
    }

    if (!exclude) {
      out << line;
      if (!tinNull)
        tiout << tin.at(i);
    }
  }

  return;
}

/*********************************************
 *
 * remove_name
 *
 ********************************************/

void Gui::remove_partname(
    QStringList   in,    // csiParts
    QVector<int>  tin,   // typeIndexes
    QString       name,  // partName
    QStringList  &out,   // newCSIParts
    QVector<int> &tiout) // newCSIParts
{
  bool tinNull  = false;
  name = name.toLower();

  if ((tinNull = in.size() != tin.size())) {
    QString const message(tr("CSI part list size [%1] does not match line index size [%2].")
                             .arg(in.size()).arg(tin.size()));
    emit gui->messageSig(LOG_NOTICE, message);
  }

  for (int i = 0; i < in.size(); i++) {
    QString line = in.at(i);
    QStringList tokens;

    split(line,tokens);

    if (tokens.size() == 4 && tokens[0] == "0" &&
      (tokens[1] == "!LPUB" || tokens[1] == "LPUB") &&
      tokens[2] == "NAME") {
      if (tokens[3].toLower() == name.toLower()) {
        for ( ; i < in.size(); i++) {
          line = in.at(i);
          split(line,tokens);
          if (tokens.size() == 15 && tokens[0] == "1") {
            break;
          } else {
            out << line;
            if (!tinNull)
              tiout << tin.at(i);
          }
        }
      } else {
        out << line;
        if (!tinNull)
          tiout << tin.at(i);
      }
    } else {
      out << line;
      if (!tinNull)
        tiout << tin.at(i);
    }
  }

  return;
}

/*********************************************
 *
 * set_divider_pointers
 *
 * this processes step_group or callout divider
 * pointers and pointer attributes
 *
 ********************************************/

void Gui::set_divider_pointers(
        Meta &curMeta,
        Where &current,
        Range *range,
        LGraphicsView *view,
        DividerType dividerType,
        int stepNum,
        Rc rct) {

    Rc pRc  = (rct == CalloutDividerRc ? CalloutDividerPointerRc :
                                         StepGroupDividerPointerRc);
    Rc paRc = (rct == CalloutDividerRc ? CalloutDividerPointerAttribRc :
                                         StepGroupDividerPointerAttribRc);
    PointerAttribMeta pam = (rct == CalloutDividerRc ? curMeta.LPub.callout.divPointerAttrib :
                                                       curMeta.LPub.multiStep.divPointerAttrib);

    Where walk(current.modelName,current.lineNumber);
    walk++;

    int numLines = gui->subFileSize(walk.modelName);

    bool rd = dividerType == RangeDivider;

    int sn  = stepNum - 1; // set the previous STEP's step number

    for ( ; walk.lineNumber < numLines; walk++) {
        QString stepLine = gui->readLine(walk);
        Rc mRc = curMeta.parse(stepLine,walk);
        if (mRc == StepRc || mRc == RotStepRc) {
            break;
        } else if (mRc == pRc) {
            PointerMeta pm = (rct == CalloutDividerRc ? curMeta.LPub.callout.divPointer :
                                                        curMeta.LPub.multiStep.divPointer);
            range->appendDividerPointer(walk,pm,pam,view,sn,rd);
        } else if (mRc == paRc) {
            QStringList argv;
            split(stepLine,argv);
            pam.setValueInches(pam.parseAttributes(argv,walk));
            Pointer          *p = nullptr;
            int i               = pam.value().id - 1;
            int validIndex      = rd ? range->rangeDividerPointerList.size() - 1 :
                                       range->stepDividerPointerList.size() - 1; /*0-index*/
            if (i <= validIndex) {
                p = rd ? range->rangeDividerPointerList[i] :
                         range->stepDividerPointerList[i];
            } else {
                emit gui->parseErrorSig(tr("Invalid Divider pointer attribute index.<br>"
                                           "Expected value &#60;= %1, received %2")
                                           .arg(validIndex).arg(i),current);
                break;
            }
            if (p && pam.value().id == p->id) {
                pam.setOtherDataInches(p->getPointerAttribInches());
                p->setPointerAttribInches(pam);
                if (rd)
                    range->rangeDividerPointerList.replace(i,p);
                else
                    range->stepDividerPointerList.replace(i,p);
            }
        }
    }
}

/*
 * This function, drawPage, is handed the parse state going into the page
 * that is to be displayed.  It gathers up a step group, or a single step,
 * including any called out models (think recursion), but ignores non-called
 * out submodels.  It stops parsing the LDraw files when it hits end of
 * page, at which point, it calls a function to convert the parsed and
 * retained results into Qt GraphicsItems for display to the user.
 *
 * This drawPage function is only called by the findPage function.  The findPage
 * function and this drawPage function used to be one function, but doing
 * this processing in one function was problematic.  The design issue is that
 * at start of step, or multistep, you do not know the page number, because
 * the step could contain submodels that are not called out, which produce at
 * least one page each.
 *
 * findPage (is below this drawPage function in this file), is lightweight
 * in that it is much smaller that the original combined function traverse.
 * Its design goal is to find the page the user wants displayed, and present
 * the parse state of the start of page to this function drawPage.
 *
 * depends on the current page number of the parse, and the page number the
 * user wants to see.  If the current page number is lower than the "display"
 * page number, the state of meta, the parts in the submodel, the filename
 * and linenumber of the first line of page is saved.  When findPage hits end
 * of page for the "display" page, it hands the saved start of page state to
 * drawPage.  drawPage parses from start of page, creating a tree of data
 * structures representing the content of the page.  At end of page, the
 * tree is converted into Qt GraphicsItems for display.
 *
 * One thing to note is that findPage does the bulk of the LDraw file parsing
 * and is as lightweight (e.g. small) as I could make it.  Since callouts do
 * not have pages of their own (they are on the page of their parent step),
 * findPage ignores callouts.  Since findPage deals with non-callout submodels,
 * drawPage ignores non-called out submodels, and only deals with callout
 * submodels.
 *
 * After drawPage finishes gathering the page and converting the tree to
 * graphics items, it returns to findPage, which discards the parse state,
 * but continues parsing through to the last page, so we know how many pages
 * are in the building instructions.
 *
 */

Range *Gui::newRange(
    Steps  *steps,
    bool    calledOut)
{
  Range *range;

  if (calledOut) {
      range = new Range(steps,
                        steps->meta.LPub.callout.alloc.value(),
                        steps->meta.LPub.callout.freeform);
    } else {
      range = new Range(steps,
                        steps->meta.LPub.multiStep.alloc.value(),
                        steps->meta.LPub.multiStep.freeform);
    }
  return range;
}

int Gui::drawPage(
        Steps          *steps,
        QString const   &addLine,
        DrawPageOptions &opts)
{
    Gui::setPageProcessRunning(PROC_DRAW_PAGE);
    QElapsedTimer pageRenderTimer;
    pageRenderTimer.start();

    LGraphicsView *view = opts.printing ? &gui->KexportView : gui->KpageView;

    static QRegularExpression partTypeLineRx("(^[1-5]\\s+)|(\\bBEGIN SUB\\b)");
    QStringList configuredCsiParts; // fade and highlight configuration
    QString  line;
    Callout *callout         = nullptr;
    Range   *range           = nullptr;
    Step    *step            = nullptr;

    int      instances       = 1;
    bool     global          = true;
    bool     pliIgnore       = false;
    bool     partIgnore      = false;
    bool     excludedPart    = false;
    bool     synthBegin      = false;
    bool     multiStep       = false;
    bool     multiStepPage   = false;
    bool     partsAdded      = false;
    bool     coverPage       = false;
    bool     bfxStore1       = false;
    bool     bfxLoad         = false;
    bool     firstGroupStep  = true;
    bool     noStep          = false;
    bool     rotateIcon      = false;
    bool     assemAnnotation = false;
    bool     displayInstanceCount = false;
    int      countInstances  = steps->meta.LPub.countInstance.value();

    // Build mod update flags
    int      buildModStepIndex  = -1;
    bool     buildModItems      = false;
    bool     buildModInsert     = false;
    bool     buildModExists     = false;
    bool     buildModChange     = false;
    bool     buildModPliIgnore  = false;
    bool     buildModActionStep = false;
    bool     buildModTypeIgnore = false;

    QVector<int>  buildModLineTypeIndexes;
    QStringList   buildModCsiParts;
    BuildModFlags buildMod;
    BuildModMeta  buildModMeta;
    BuildModMeta  buildModActionMeta;

    QMap<int, QString> buildModKeys;
    QMap<int, QVector<int>> buildModAttributes;
    QMap<int,int> buildModActions;

    DividerType dividerType  = NoDivider;

    PagePointer *pagePointer = nullptr;
    QMap<Positions, PagePointer *> pagePointers;
    QMap<Where, SceneObjectData> selectedSceneItems;

    QList<InsertMeta> inserts;
    QMap<QString, LightData> lightList;
    QStringList calloutParts;

    Where topOfStep = opts.current;

    enum draw_page_stat { begin, end };

    steps->setTopOfSteps(topOfStep/*opts.current*/);
    steps->isMirrored = opts.isMirrored;
    lpub->page.coverPage = false;

    Rc gprc    = OkRc;
    Rc rc      = OkRc;

    TraverseRc returnValue = HitNothing;

    // include file vars
    Where includeHere;
    Rc    includeFileRc   = EndOfIncludeFileRc;
    bool includeFileFound = false;
    bool resetIncludeRc   = false;

    // set page header/footer width
    float pW;
    int which;
    if (callout) {
        which = callout->meta.LPub.page.orientation.value() == Landscape ? 1 : 0;
        pW = callout->meta.LPub.page.size.value(which);
        callout->meta.LPub.page.pageHeader.size.setValue(0,pW);
        callout->meta.LPub.page.pageFooter.size.setValue(0,pW);
    } else {
        which = steps->meta.LPub.page.orientation.value() == Landscape ? 1 : 0;
        pW = steps->meta.LPub.page.size.value(which);
        steps->meta.LPub.page.pageHeader.size.setValue(0,pW);
        steps->meta.LPub.page.pageFooter.size.setValue(0,pW);
    }

    auto drawPageStatus = [&opts, &multiStep, &coverPage, &topOfStep] (draw_page_stat status) {
        int charWidth = QFontMetrics(gui->font()).averageCharWidth();
        QFontMetrics currentMetrics(gui->font());
        QString elidedModelName = currentMetrics.elidedText(opts.current.modelName,
                                                            Qt::ElideRight, charWidth * 30/*characters*/);
        Where where = topOfStep;
        bool fin = status == end;
        static QRegularExpression multiStepRx(" MULTI_STEP BEGIN$");
        bool stepGroup = fin ? multiStep : Gui::stepContains(where, multiStepRx);
        QString const message = tr("%1 %2 draw-page for page %3, step %4, model '%5'%6")
                .arg(fin ? tr("Processed") : tr("Processing")).arg(stepGroup ? "multi-step" : opts.calledOut ? tr("called out") : coverPage ? tr("cover page") : tr("single-step"),
                     Gui::displayPageNum).arg(opts.stepNum).arg(elidedModelName).arg(fin ? "" : "...");
        emit gui->messageSig(LOG_STATUS, message);
        emit gui->messageSig(fin ? LOG_TRACE : LOG_INFO, message);
    };

    auto drawPageElapsedTime = [&partsAdded, &pageRenderTimer, &coverPage]() {
        QString pageRenderMessage = QString("%1 ").arg(VER_PRODUCTNAME_STR);
        if (!lpub->page.coverPage && partsAdded) {
            pageRenderMessage += tr("using %1 ").arg(rendererNames[Render::getRenderer()]);
            QString renderAttributes;
            if (Render::getRenderer() == RENDERER_LDVIEW) {
                if (Preferences::enableLDViewSingleCall)
                    renderAttributes += tr("Single Call");
                if (Preferences::enableLDViewSnaphsotList)
                    renderAttributes += tr(", Snapshot List");
            }
            if (!renderAttributes.isEmpty())
                pageRenderMessage += QString("(%1) ").arg(renderAttributes);
            pageRenderMessage += tr("render ");
        }
        pageRenderMessage += tr("rendered page %1 - %2")
                .arg(QString("%1%2").arg(Gui::displayPageNum)
                .arg(coverPage ? tr(" (Cover Page)") : ""),
                     Gui::elapsedTime(pageRenderTimer.elapsed()));
        emit gui->messageSig(LOG_TRACE, pageRenderMessage);
        Gui::revertPageProcess();
    };

    auto insertAttribute =
            [&opts,
            &buildMod,
            &topOfStep] (
            QMap<int, QVector<int>> &buildModAttributes,
            int index, const Where &here)
    {
        int  fileNameIndex = topOfStep.modelIndex;
        QMap<int, QVector<int>>::iterator i = buildModAttributes.find(buildMod.level);
        if (i == buildModAttributes.end()) {
            QVector<int> modAttributes = { 0, 0, 0, Gui::displayPageNum, 0/*step pieces*/, fileNameIndex, topOfStep.lineNumber, opts.stepNum };
            modAttributes[index] = here.lineNumber;
            buildModAttributes.insert(buildMod.level, modAttributes);
        } else {
            i.value()[index] = here.lineNumber;
        }
    };

    auto insertBuildModification =
           [&step,
            &opts,
            &buildModAttributes,
            &buildModKeys,
            &topOfStep] (int buildModLevel)
    {
        int buildModStepIndex = gui->getBuildModStepIndex(topOfStep);
        QString buildModKey   = buildModKeys.value(buildModLevel);
        QVector<int> modAttributes = { 0, 0, 0, 0, 0, -1, 0, 0 };

        QMap<int, QVector<int>>::iterator i = buildModAttributes.find(buildModLevel);
        if (i == buildModAttributes.end()) {
            emit gui->messageSig(LOG_ERROR, tr("Invalid BuildMod Entry for key: %1").arg(buildModKey));
            return;
        }
        modAttributes = i.value();

        modAttributes[BM_DISPLAY_PAGE_NUM] = displayPageNum;
        modAttributes[BM_STEP_PIECES]      = opts.csiParts.size();
        modAttributes[BM_MODEL_NAME_INDEX] = topOfStep.modelIndex;
        modAttributes[BM_MODEL_LINE_NUM]   = topOfStep.lineNumber;
        modAttributes[BM_MODEL_STEP_NUM]   = opts.stepNum;
/*
#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_DEBUG, tr(
                            "Insert DrawPage BuildMod StepIndex: %1, "
                            "Action: Apply(64), "
                            "Attributes: %2 %3 %4 %5 %6 %7 %8 %9, "
                            "ModKey: %10, "
                            "Level: %11")
                        .arg(buildModStepIndex)                      // Attribute Default Initial:
                        .arg(modAttributes.at(BM_BEGIN_LINE_NUM))    // 0         0       this
                        .arg(modAttributes.at(BM_ACTION_LINE_NUM))   // 1         0       this
                        .arg(modAttributes.at(BM_END_LINE_NUM))      // 2         0       this
                        .arg(modAttributes.at(BM_DISPLAY_PAGE_NUM))  // 3         0       this
                        .arg(modAttributes.at(BM_STEP_PIECES))       // 4         0       this
                        .arg(modAttributes.at(BM_MODEL_NAME_INDEX))  // 5        -1       this
                        .arg(modAttributes.at(BM_MODEL_LINE_NUM))    // 6         0       this
                        .arg(modAttributes.at(BM_MODEL_STEP_NUM))    // 7         0       this
                        .arg(buildModKey)
                        .arg(buildModLevel));
#endif
//*/
        gui->insertBuildMod(buildModKey,
                            modAttributes,
                            buildModStepIndex);
        if (step->submodelStack().size())
            gui->setBuildModSubmodelStack(buildModKey,step->submodelStack());
    };

    auto buildModIgnoreOverride = [&multiStep, &buildMod] (bool &buildModIgnore, bool &buildModTypeIgnore)
    {
        if (buildModIgnore) {
            buildModIgnore = (multiStep && buildMod.state != BM_BEGIN);
            // if ignore override, set flag to ignore CSI parts
            buildModTypeIgnore = ! buildModIgnore;
        }
        return ! buildModIgnore;
    };

#ifdef WRITE_PARTS_DEBUG
    auto writeDrawPartsFile = [this,&topOfStep, &opts](const QString &insert = QString())
    {
        if (opts.csiParts.isEmpty())
            return;
        const QString &nameInsert = insert.isEmpty() ? "c_draw_parts" : insert;
        const QString filePath = QDir::currentPath() + "/" + Paths::tmpDir;
        const QString outfileName = QString("%1/%2_%3.ldr")
                .arg(filePath)
                .arg(nameInsert)
                .arg(QString("page_%1_step_%2_model_%3_line_%4")
                     .arg(displayPageNum)        // Page Number
                     .arg(opts.stepNum)          // Step Number
                     .arg(topOfStep.modelIndex)  // ModelIndex
                     .arg(topOfStep.lineNumber));// LineNumber
        QFile file(outfileName);
        if ( ! file.open(QFile::WriteOnly | QFile::Text))
            messageSig(LOG_ERROR,QString("Cannot open draw_parts file %1 for writing: %2")
                       .arg(outfileName) .arg(file.errorString()));
        QTextStream out(&file);
        for (int i = 0; i < opts.csiParts.size(); i++)
            out << opts.csiParts[i] << lpub_endl;
        file.close();
    };
#endif

    // Update buildMod intiialized in setBuildModForNextStep
    auto updateBuildModification = [&step, &opts, &buildModKeys] (int buildModLevel)
    {
        const QString buildModKey = buildModKeys.value(buildModLevel);
        gui->setBuildModStepKey(buildModKey, step->viewerStepKey);
        gui->setBuildModStepPieces(buildModKey, opts.csiParts.size());
        if (step->submodelStack().size())
            gui->setBuildModSubmodelStack(buildModKey,step->submodelStack());
    };

    PartLineAttributes pla(
                opts.csiParts,
                opts.lineTypeIndexes,
                buildModCsiParts,
                buildModLineTypeIndexes,
                buildMod.level,
                buildMod.ignore,
                buildModItems);

    if (!Gui::ContinuousPage())
        drawPageStatus(begin);

  /*
   * do until end of page
   */
    int numLines = lpub->ldrawFile.size(opts.current.modelName);

    //* local ldrawFile used for debugging
#ifdef QT_DEBUG_MODE
    LDrawFile *ldrawFile = &lpub->ldrawFile;
    Q_UNUSED(ldrawFile)
#endif
    //*/

    for ( ; opts.current <= numLines && !Gui::abortProcess() ; opts.current++) {

        // if reading include file, return to current line, do not advance

        if (includeFileRc != EndOfIncludeFileRc && includeFileFound) {
            opts.current.lineNumber--;
        }

        // load initial meta values

      /*
       * For Gui::drawPage(), the BuildMod behaviour performs two functions:
       * Funct 1. Capture the appropriate 'block' of lines to be written to the csiPart list (partIgnore).
       * Funct 2. Write the appropriate part lines to the pliPart list. (pliIgnore)
       *
       * Of course BuildMods in Gui::drawPage() are only processed for Called out submodels,
       * other cases are processed in findPage() - and countPage() accordingly.
       *
       * The buildModLevel flag is enabled for the lines between BUILD_MOD BEGIN and BUILD_MOD END
       * Lines between BUILD_MOD BEGIN and BUILD_MOD END_MOD represent the modified content
       * Lines between BUILD_MOD END_MOD and BUILD_MOD END represent the original content
       *
       * When the buildModLevel flag is true (greater than 0), and:
       * Funct 1 (csiParts):
       * Parse is enabled when 'buildModIgnore' is false.
       * When the build mod action is 'apply', the modification block is parsed. (buildModIgnore is false)
       * When the build mod action is 'remove', the default block is parsed.     (buildModIgnore is false)
       * Remove group, partType and partName is only applied when 'buildModIgnore is false.
       *
       * Funct 2 (pliParts):
       * Process pliParts is enabled when 'buildModPliIgnore' is false.
       * When the build mod action is 'apply', the modification block is parsed but pliParts are
       * not written to the pliList (buildModPliIgnore is true).
       * When the build mod action is 'remove', the defaul block is parsed and the pliParts are
       * written to the pliList (buildModPliIgnore false false)
       *
       * When the build mod meta command is BUILD_MOD END 'buildModIgnore' and 'buildModPliIgnore'
       * are reset to false while buildModLevel is reset to false (0) if the build mod command is not nested
       *
       * BUILD_MOD APPLY or BUILD_MOD REMOVE action meta commands are ignored as they are set in findPage()
       *
       * When the buildModLevel flag is false (0), pli and csi lines are processed normally
       */

//* local optsPageNum used to set breakpoint condition (e.g. optsPageNum > 7)
#ifdef QT_DEBUG_MODE
        int debugDisplayPageNum = Gui::displayPageNum;
        Q_UNUSED(debugDisplayPageNum)
#endif
//*/

        Meta &curMeta = callout ? callout->meta : steps->meta;

        RotStepData saveRotStep;

        QStringList tokens;

        /* If we hit end of file we've got to note end of step */

        if (opts.current >= numLines) {
            line.clear();
            gprc = EndOfFileRc;
            tokens << "0";
        }

        /* not end of file, so get the next LDraw line */

        else {

        /* read the line from the ldrawFile repository */

            line = lpub->ldrawFile.readLine(opts.current.modelName,opts.current.lineNumber);
            split(line,tokens);
        } // If we hit end of file, note end of step or if not, get the next LDraw line

        // STEP - Process part type

        if (tokens.size() == 15 && tokens[0] == "1") {
            // STEP - Create partType

            QString color = tokens[1];
            QString type  = tokens[tokens.size()-1];

            if (color == LDRAW_MAIN_MATERIAL_COLOUR) {
                QStringList addTokens;
                split(addLine,addTokens);
                if (addTokens.size() == 15) {
                    tokens[1] = addTokens[1];
                }
                line = tokens.join(" ");
                color = tokens[1];
            }

            // STEP - Allocate step for type

            if (! buildMod.ignore) {

                /* for multistep build mod case where we are processing a callout
                 and we are overriding the last action, buildModTypeIgnore is set
                 so we do not render overridden types */

                if (! buildModTypeIgnore) {
                    CsiItem::partLine(line,pla,opts.current.lineNumber,OkRc);
                    partsAdded = true;
                }

                /* since we have a part usage, we have a valid STEP */

                if (step == nullptr  && ! noStep) {
                    if (range == nullptr) {
                        range = Gui::newRange(steps,opts.calledOut);
                        steps->append(range);
                    }

                    step = new Step(topOfStep,
                                    range,
                                    opts.stepNum,
                                    curMeta,
                                    opts.calledOut,
                                    multiStep);

                    range->append(step);
                }
            } // Allocate step for type

            /* if part is on excludedPart.lst, set excludedPart */

            excludedPart = ExcludedParts::isExcludedPart(type);

            /* addition of ldraw parts */

            // STEP - Allocate PLI part

            if (curMeta.LPub.pli.show.value()
                    && ! excludedPart
                    && ! pliIgnore
                    && ! partIgnore
                    && ! buildModPliIgnore
                    && ! synthBegin) {
                QString colorType = color+type;

                if (! gui->isSubmodel(type) || curMeta.LPub.pli.includeSubs.value()) {

                    /*  check if alternative part exist and replace */

                    if(PliSubstituteParts::hasSubstitutePart(type)) {

                        QStringList substituteToken;
                        split(line,substituteToken);
                        QString substitutePart = type;

                        if (PliSubstituteParts::getSubstitutePart(substitutePart)) {
                            substituteToken[substituteToken.size()-1] = substitutePart;
                        }

                        line = substituteToken.join(" ");
                    }

                    /* remove part from buffer parts if in step group and buffer load */

                    if (opts.bfxStore2 && bfxLoad) {
                        // bool removed = false;
                        for (int i = 0; i < opts.bfxParts.size(); i++) {
                            if (opts.bfxParts[i] == colorType) {
                                opts.bfxParts.removeAt(i);
                                // removed = true;
                                break;
                            }
                        }

                        // Danny: the following condition should help LPUB to remove automatically from PLI the parts in the buffer,
                        // but does not work if two buffers are used one after another in a multi step page.
                        // Better to make the user use the !LPUB PLI BEGIN IGN / END

                        // if ( ! removed )  {
                        if (! noStep) {
                            opts.pliParts << Pli::partLine(line,opts.current,steps->meta);
                        }
                        // }
                    } else {

                        opts.pliParts << Pli::partLine(line,opts.current,steps->meta);
                    }
                }

                /* bfxStore1 goes true when we've seen BFX STORE the first time
                 in a sequence of steps.  This used to be commented out which
                 means it didn't work in some cases, but we need it in step
                 group cases, so.... bfxStore1 && multiStep (was just bfxStore1) */

                if (bfxStore1 && (multiStep || opts.calledOut)) {
                    opts.bfxParts << colorType;
                }
            } // Allocate PLI part

            // STEP - Process callout

            /* if it is a called out sub-model, then process it.
           * Non called out sub-model are processed in findPage() */

            if (lpub->ldrawFile.isSubmodel(type) && callout && ! noStep && ! buildMod.ignore) {

                CalloutBeginMeta::CalloutMode calloutMode = callout->meta.LPub.callout.begin.value();

                //              logDebug() << "CALLOUT MODE: " << (calloutMode == CalloutBeginMeta::Unassembled ? "Unassembled" :
                //                                                 calloutMode == CalloutBeginMeta::Rotated ? "Rotated" : "Assembled");

                /* we are a callout, so gather all the steps within the callout
                 start with new meta, but no rotation step */

                QString thisType = type;

                // STEP - Process rotated or assembled callout

                if ((opts.assembledCallout = calloutMode != CalloutBeginMeta::Unassembled)) {

                    /* So, we process these callouts in-line, not when we finally hit the STEP or
                     ROTSTEP that ends this processing, but for ASSEMBLED or ROTATED
                     callouts, the ROTSTEP state affects the results, so we have to search
                     forward until we hit STEP or ROTSTEP to know how the submodel might
                     want to be rotated.  Also, for submodel's who's scale is different
                     than their parent's scale, we want to scan ahead and find out the
                     parent's scale and "render" the submodels at the parent's scale */

                    Rc rrc = OkRc;
                    Meta tmpMeta = curMeta;
                    Where walk = opts.current;
                    for (++walk; walk < numLines; ++walk) {
                        QStringList tokens;
                        QString scanLine = lpub->ldrawFile.readLine(walk.modelName,walk.lineNumber);
                        split(scanLine,tokens);
                        if (tokens.size() > 0 && tokens[0] == "0") {
                            rrc = tmpMeta.parse(scanLine,walk,false);
                            if (rrc == StepRc || rrc == RotStepRc) {
                                break;
                            }
                        }
                    }

                    if (calloutMode == CalloutBeginMeta::Rotated) {
                        // When renderers apply CA rotation, set cameraAngles to 0 so only ROTSTEP is sent to renderers.
                        if (! Preferences::applyCALocally)
                            callout->meta.LPub.assem.cameraAngles.setValues(0.0, 0.0);
                    }
                    callout->meta.rotStep = tmpMeta.rotStep;
                    callout->meta.LPub.assem.modelScale = tmpMeta.LPub.assem.modelScale;
                    // The next command applies the rotation due to line, but not due to callout->meta.rotStep
                    thisType = callout->wholeSubmodel(callout->meta,type,line,0);

                } // Process rotated or assembled callout

                // STEP - Set callout in parent step

                if (callout->bottom.modelName != thisType) {

                    Where current2(thisType, gui->getSubmodelIndex(thisType),0);
                    gui->skipHeader(current2);
                    if (calloutMode == CalloutBeginMeta::Assembled) {
                        // In this case, no additional rotation should be applied to the submodel
                        callout->meta.rotStep.clear();
                    }
                    SubmodelStack tos(opts.current.modelName,opts.current.lineNumber,opts.stepNum);
                    const QString levelKey = QString("DrawPage BuildMod Key: %1, ParentModel: %2, LineNumber: %3")
                            .arg(buildMod.key,
                                 opts.current.modelName)
                            .arg(opts.current.lineNumber);
                    int buildModLevel = buildMod.state == BM_BEGIN ? getLevel(levelKey, BM_CURRENT) : opts.buildModLevel;
                    callout->meta.submodelStack << tos;
                    Meta saveMeta = callout->meta;
                    callout->meta.LPub.pli.constrain.resetToDefault();

                    step->append(callout);

                    calloutParts.clear();

                    QStringList csiParts2;
                    QVector<int> lineTypeIndexes2;

                    QHash<QString, QStringList> calloutBfx;
                    QHash<QString, QVector<int>> calloutBfxLineTypeIndexes;

                    DrawPageOptions calloutOpts(
                        current2,           /*where*/
                        csiParts2,          /*CSI parts*/
                        calloutParts,       /*PLI parts*/
                        opts.bfxParts,
                        opts.ldrStepFiles,
                        opts.csiKeys,
                        calloutBfx,
                        opts.pliPartGroups,
                        lineTypeIndexes2,
                        calloutBfxLineTypeIndexes,
                        1                   /*stepNum*/,
                        opts.groupStepNumber,
                        buildModLevel,
                        opts.updateViewer,
                        lpub->ldrawFile.mirrored(tokens),
                        opts.printing,
                        opts.bfxStore2,
                        opts.assembledCallout,
                        true               /*calledOut*/,
                        false              /*displayModel*/,
                        opts.renderModelColour,
                        opts.renderParentModel
                        );

                    returnValue = static_cast<TraverseRc>(Gui::drawPage(callout, line, calloutOpts));

                    callout->meta = saveMeta;

                    bool calloutPliPerStep = callout->meta.LPub.pli.show.value() &&
                            callout->meta.LPub.callout.pli.perStep.value();

                    if (Preferences::buildModEnabled)
                        buildModPliIgnore = calloutPliPerStep || pliIgnore || excludedPart;

                    if (! calloutPliPerStep && ! excludedPart && ! pliIgnore && ! buildModPliIgnore && ! partIgnore && ! synthBegin && calloutMode == CalloutBeginMeta::Unassembled) {
                        opts.pliParts += calloutParts;
                    }

                    if (returnValue != HitNothing) {
                        steps->placement = steps->meta.LPub.assem.placement;
                        if (returnValue == HitBuildModAction || returnValue == HitCsiAnnotation)
                            // return to init drawPage to rerun findPage to regenerate content
                            Gui::pageProcessRunning = PROC_DISPLAY_PAGE;
                        else
                            // return to init findPage
                            Gui::pageProcessRunning = PROC_FIND_PAGE;
                        return static_cast<int>(returnValue);
                    }
                } else {
                    callout->instances++;
                    if (calloutMode == CalloutBeginMeta::Unassembled) {
                        opts.pliParts += calloutParts;
                    }
                } // Set callout in parent step

                /* remind user what file we're working on */

                if (!Gui::ContinuousPage())
                    emit gui->messageSig(LOG_STATUS, tr("Processing %1...").arg(opts.current.modelName));

            } // Process called out submodel

            // STEP - Set display submodel at first submodel step

            if (step && (steps->meta.LPub.subModel.show.value() || (multiStep && steps->meta.LPub.multiStep.subModel.show.value()))) {
                bool topModel     = (lpub->ldrawFile.topLevelFile() == topOfStep.modelName);
                bool showTopModel = (steps->meta.LPub.subModel.showTopModel.value());
                bool calloutOk    = (opts.calledOut ? opts.assembledCallout : true ) &&
                        (opts.calledOut ? steps->meta.LPub.subModel.showSubmodelInCallout.value() : true);
                bool showStepOk   = (steps->meta.LPub.subModel.showStepNum.value() == opts.stepNum || opts.stepNum == 1);
                if (showStepOk && calloutOk && (!topModel || showTopModel)) {
                    if (multiStep && steps->meta.LPub.multiStep.pli.perStep.value() == false) {
                        steps->placeSubModel = true;
                    } else {
                        step->placeSubModel = true;
                    }
                }
            } // Set display submodel at first submodel step
        } // Process part type

        // STEP - Process line, triangle, or polygon type

        else if ((tokens.size() == 8  &&  tokens[0] == "2") ||
                 (tokens.size() == 11 &&  tokens[0] == "3") ||
                 (tokens.size() == 14 && (tokens[0] == "4"  || tokens[0] == "5"))) {

            // STEP - Allocate step for type

             /* we've got a line, triangle or polygon, so add it to the list
             and make sure we know we have a step */

            if (! buildMod.ignore) {

               /* for multistep build mod case where we are processing a callout
               and we are overriding the last action, buildModTypeIgnore is set
               so we do not render overridden types */

                if (! buildModTypeIgnore) {
                    CsiItem::partLine(line,pla,opts.current.lineNumber,OkRc);
                    partsAdded = true;
                }

                if (step == nullptr && ! noStep) {
                    if (range == nullptr) {
                        range = Gui::newRange(steps,opts.calledOut);
                        steps->append(range);
                    }

                    step = new Step(topOfStep,
                                    range,
                                    opts.stepNum,
                                    steps->meta,
                                    opts.calledOut,
                                    multiStep);

                    range->append(step);
                }
            } // Allocate step for type
        } // Process line, triangle, or polygon type

        // STEP - Process meta-command

        else if ((tokens.size() && tokens[0] == "0") || gprc == EndOfFileRc) {

            /* must be a meta-command (or comment) */

            if (global && tokens.contains("!LPUB") && tokens.contains("GLOBAL")) {
                topOfStep = opts.current;
            } else {
                global = false;
            }

            if (gprc == EndOfFileRc) {
                rc = gprc;
            } else {

                // intercept include file flag

                if (includeFileRc != EndOfIncludeFileRc) {
                    if (resetIncludeRc) {
                        rc = IncludeRc;                         // return to IncludeRc to parse another line
                    } else {
                        rc = includeFileRc;                     // execute the Rc returned by include(...)
                        resetIncludeRc = true;                  // reset to run include(...) to parse another line
                    }
                } else {
                    rc = curMeta.parse(line,opts.current,true); // continue
                }
            }

            /* handle specific meta-command */

            switch (rc) {
            /* toss it all out the window, per James' original plan */
            case ClearRc:
                opts.pliParts.clear();
                opts.csiParts.clear();
                opts.lineTypeIndexes.clear();
                //steps->freeSteps();  // had to remove this because it blows steps following clear
                // in step group.
                break;

            case PreferredRendererRc:
            case PreferredRendererAssemRc:
            case PreferredRendererCalloutAssemRc:
            case PreferredRendererGroupAssemRc:
            case PreferredRendererSubModelRc:
            case PreferredRendererPliRc:
            case PreferredRendererBomRc:
                if (!step)
                    break;
                if (rc == PreferredRendererRc) {
                    curMeta.LPub.preferredRenderer.setPreferences();
                    curMeta.LPub.resetCamerasFoV();
                    curMeta.LPub.assem.preferredRenderer = curMeta.LPub.preferredRenderer;
                    step->csiStepMeta.preferredRenderer = curMeta.LPub.preferredRenderer;
                } else if (rc == PreferredRendererAssemRc) {
                    curMeta.LPub.assem.preferredRenderer.setPreferences();
                    curMeta.LPub.assem.resetCameraFoV();
                    curMeta.LPub.preferredRenderer = curMeta.LPub.assem.preferredRenderer;
                    step->csiStepMeta.preferredRenderer = curMeta.LPub.assem.preferredRenderer;
                } else if (rc == PreferredRendererCalloutAssemRc) {
                    curMeta.LPub.callout.csi.preferredRenderer.setPreferences();
                    curMeta.LPub.callout.csi.resetCameraFoV();
                    step->csiStepMeta.preferredRenderer = curMeta.LPub.callout.csi.preferredRenderer;
                } else if (rc == PreferredRendererGroupAssemRc) {
                    curMeta.LPub.multiStep.csi.preferredRenderer.setPreferences();
                    curMeta.LPub.multiStep.csi.resetCameraFoV();
                    step->csiStepMeta.preferredRenderer = curMeta.LPub.multiStep.csi.preferredRenderer;
                } else if (rc == PreferredRendererSubModelRc) {
                    curMeta.LPub.subModel.preferredRenderer.setPreferences();
                    curMeta.LPub.subModel.resetCameraFoV();
                    step->csiStepMeta.preferredRenderer = curMeta.LPub.subModel.preferredRenderer;
                } else if (rc == PreferredRendererPliRc) {
                    curMeta.LPub.pli.preferredRenderer.setPreferences();
                    curMeta.LPub.pli.resetCameraFoV();
                    step->csiStepMeta.preferredRenderer = curMeta.LPub.pli.preferredRenderer;
                } else if (rc == PreferredRendererBomRc) {
                    curMeta.LPub.bom.preferredRenderer.setPreferences();
                    curMeta.LPub.bom.resetCameraFoV();
                    step->csiStepMeta.preferredRenderer = curMeta.LPub.bom.preferredRenderer;
                }
                break;

            case EnableFadeStepsCalloutAssemRc:
            case EnableFadeStepsGroupAssemRc:
            case EnableFadeStepsAssemRc:
                if (!step)
                    break;
                if (!Gui::m_fadeStepsSetup &&
                        (rc == EnableFadeStepsCalloutAssemRc ?
                         !curMeta.LPub.callout.csi.fadeSteps.enable.global &&
                         !curMeta.LPub.callout.csi.fadeSteps.setup.value() :
                         rc == EnableFadeStepsGroupAssemRc ?
                         !curMeta.LPub.multiStep.csi.fadeSteps.enable.global &&
                         !curMeta.LPub.multiStep.csi.fadeSteps.setup.value() :
                         /*rc == EnableFadeStepsAssemRc*/
                         !curMeta.LPub.assem.fadeSteps.enable.global &&
                         !curMeta.LPub.assem.fadeSteps.setup.value())) {
                    emit gui->parseErrorSig(tr("Fade previous steps command IGNORED."
                                               "<br>FADE_STEPS SETUP must be set to TRUE."
                                               "<br>FADE_STEPS SETUP must precede FADE_STEPS ENABLED."
                                               "<br>GLOBAL command must appear in the header of the top model."),opts.current);
                    break;
                }
                if (rc == EnableFadeStepsCalloutAssemRc) {
                    curMeta.LPub.callout.csi.fadeSteps.setPreferences();
                    step->csiStepMeta.fadeSteps = curMeta.LPub.callout.csi.fadeSteps;
                } else if (rc == EnableFadeStepsGroupAssemRc) {
                    curMeta.LPub.multiStep.csi.fadeSteps.setPreferences();
                    step->csiStepMeta.fadeSteps = curMeta.LPub.multiStep.csi.fadeSteps;
                } else if (rc == EnableFadeStepsAssemRc) {
                    curMeta.LPub.assem.fadeSteps.setPreferences();
                    curMeta.LPub.fadeSteps = curMeta.LPub.assem.fadeSteps;
                    step->csiStepMeta.fadeSteps = curMeta.LPub.assem.fadeSteps;
                }
                break;
            case EnableFadeStepsRc:
                if (!step)
                    break;
                if (!curMeta.LPub.fadeSteps.enable.global &&
                        !curMeta.LPub.fadeSteps.setup.value() &&
                        step->csiStepMeta.fadeSteps.lpubFade.value()) {
                    emit gui->parseErrorSig(tr("Fade previous steps command IGNORED."
                                               "<br>FADE_STEPS SETUP must be set to TRUE."
                                               "<br>FADE_STEPS SETUP must precede FADE_STEPS ENABLED."
                                               "<br>GLOBAL command must appear in the header of the top model."),opts.current);
                    break;
                }
                curMeta.LPub.fadeSteps.setPreferences();
                if (step) {
                    curMeta.LPub.assem.fadeSteps = curMeta.LPub.fadeSteps;
                    step->csiStepMeta.fadeSteps = curMeta.LPub.fadeSteps;
                    if (!opts.displayModel) {
                        Gui::suspendFileDisplay = true;
                        gui->insertFinalModelStep();
                        Gui::suspendFileDisplay = false;
                    }
                }
                break;

            case EnableHighlightStepCalloutAssemRc:
            case EnableHighlightStepGroupAssemRc:
            case EnableHighlightStepAssemRc:
                if (!step)
                    break;
                if (!Gui::m_highlightStepSetup &&
                        (rc == EnableHighlightStepCalloutAssemRc ?
                         !curMeta.LPub.callout.csi.highlightStep.enable.global &&
                         !curMeta.LPub.callout.csi.highlightStep.setup.value() :
                         rc == EnableHighlightStepGroupAssemRc ?
                         !curMeta.LPub.multiStep.csi.highlightStep.enable.global &&
                         !curMeta.LPub.multiStep.csi.highlightStep.setup.value() :
                         /*rc == EnableHighlightStepAssemRc*/
                         !curMeta.LPub.assem.highlightStep.enable.global &&
                         !curMeta.LPub.assem.highlightStep.setup.value())) {
                    emit gui->parseErrorSig(tr("Highlight current step command IGNORED."
                                               "<br>HIGHLIGHT_STEP SETUP TRUE not deteced."
                                               "<br>HIGHLIGHT_STEP SETUP must precede HIGHLIGHT_STEP ENABLED."
                                               "<br>GLOBAL command must appear in the header of the top model."),opts.current);
                    break;
                }
                if (rc == EnableHighlightStepCalloutAssemRc) {
                    curMeta.LPub.callout.csi.highlightStep.setPreferences();
                    step->csiStepMeta.highlightStep = curMeta.LPub.callout.csi.highlightStep;
                } else if (rc == EnableHighlightStepGroupAssemRc) {
                    curMeta.LPub.multiStep.csi.highlightStep.setPreferences();
                    step->csiStepMeta.highlightStep = curMeta.LPub.multiStep.csi.highlightStep;
                } else if (rc == EnableHighlightStepAssemRc) {
                    curMeta.LPub.assem.highlightStep.setPreferences();
                    curMeta.LPub.highlightStep = curMeta.LPub.assem.highlightStep;
                    step->csiStepMeta.highlightStep = curMeta.LPub.assem.highlightStep;
                }
                break;
            case EnableHighlightStepRc:
                if (!step)
                    break;
                if (!curMeta.LPub.highlightStep.enable.global &&
                        !curMeta.LPub.highlightStep.setup.value() &&
                        step->csiStepMeta.highlightStep.lpubHighlight.value()) {
                    emit gui->parseErrorSig(tr("Highlight current step command IGNORED."
                                               "<br>HIGHLIGHT_STEP SETUP must be set to TRUE."
                                               "<br>HIGHLIGHT_STEP SETUP must precede HIGHLIGHT_STEP ENABLED."
                                               "<br>GLOBAL command must appear in the header of the top model."),opts.current);
                    break;
                }
                curMeta.LPub.highlightStep.setPreferences();
                if (step) {
                    curMeta.LPub.assem.highlightStep = curMeta.LPub.highlightStep;
                    step->csiStepMeta.highlightStep = curMeta.LPub.highlightStep;
                    if (!opts.displayModel) {
                        Gui::suspendFileDisplay = true;
                        gui->insertFinalModelStep();
                        Gui::suspendFileDisplay = false;
                    }
                }
                break;

            case LPubFadeCalloutAssemRc:
            case LPubFadeGroupAssemRc:
            case LPubFadeAssemRc:
            case LPubFadeRc:
            case LPubHighlightCalloutAssemRc:
            case LPubHighlightGroupAssemRc:
            case LPubHighlightAssemRc:
            case LPubHighlightRc:
                if (!step)
                    break;
                if (rc == LPubFadeCalloutAssemRc) {
                    step->csiStepMeta.fadeSteps.lpubFade = curMeta.LPub.callout.csi.fadeSteps.lpubFade;
                } else if (rc == LPubFadeGroupAssemRc) {
                    step->csiStepMeta.fadeSteps.lpubFade = curMeta.LPub.multiStep.csi.fadeSteps.lpubFade;
                } else if (rc == LPubFadeAssemRc) {
                    curMeta.LPub.fadeSteps.lpubFade = curMeta.LPub.assem.fadeSteps.lpubFade;
                    step->csiStepMeta.fadeSteps.lpubFade = curMeta.LPub.assem.fadeSteps.lpubFade;
                } else if (rc == LPubFadeRc) {
                    curMeta.LPub.assem.fadeSteps.lpubFade = curMeta.LPub.fadeSteps.lpubFade;
                    step->csiStepMeta.fadeSteps.lpubFade = curMeta.LPub.fadeSteps.lpubFade;
                } else if (rc == LPubHighlightCalloutAssemRc) {
                    step->csiStepMeta.highlightStep.lpubHighlight = curMeta.LPub.callout.csi.highlightStep.lpubHighlight;
                } else if (rc == LPubHighlightGroupAssemRc) {
                    step->csiStepMeta.highlightStep.lpubHighlight = curMeta.LPub.multiStep.csi.highlightStep.lpubHighlight;
                } else if (rc == LPubHighlightAssemRc) {
                    curMeta.LPub.highlightStep.lpubHighlight = curMeta.LPub.assem.highlightStep.lpubHighlight;
                    step->csiStepMeta.highlightStep.lpubHighlight = curMeta.LPub.assem.highlightStep.lpubHighlight;
                } else if (rc == LPubHighlightRc) {
                    curMeta.LPub.assem.highlightStep.lpubHighlight = curMeta.LPub.highlightStep.lpubHighlight;
                    step->csiStepMeta.highlightStep.lpubHighlight = curMeta.LPub.highlightStep.lpubHighlight;
                }
                if (Preferences::preferredRenderer != RENDERER_NATIVE) {
                    bool error = false;
                    if ((error |= !step->csiStepMeta.fadeSteps.lpubFade.value()))
                        step->csiStepMeta.fadeSteps.lpubFade.setValue(true);
                    if ((error |= !step->csiStepMeta.highlightStep.lpubHighlight.value()))
                        step->csiStepMeta.highlightStep.lpubHighlight.setValue(true);
                    if (error) {
                        QString const command = line.contains("LPUB_FADE") ? QLatin1String("LPUB_FADE") : QLatin1String("LPUB_HIGHLIGHT");
                        emit gui->parseErrorSig(tr("%1 command IGNORED."
                                                   "<br>%1 command requires preferred render RENDERER_NATIVE."
                                                   "<br>PREFERRED_RENDERER command must precede %1.").arg(command),opts.current);
                    }
                }
                break;

                /* Buffer exchange */
            case BufferStoreRc:
                opts.bfx[curMeta.bfx.value()] = opts.csiParts;
                opts.bfxLineTypeIndexes[curMeta.bfx.value()] = opts.lineTypeIndexes;
                bfxStore1 = true;
                opts.bfxParts.clear();
                break;

            case BufferLoadRc:
                opts.csiParts = opts.bfx[curMeta.bfx.value()];
                opts.lineTypeIndexes = opts.bfxLineTypeIndexes[curMeta.bfx.value()];
                if (!partsAdded) {
                    lpub->ldrawFile.setPrevStepPosition(opts.current.modelName,opts.stepNum,opts.csiParts.size());
                    //qDebug() << "Model:" << current.modelName << ", Step:"  << stepNum << ", bfx Set Fade Position:" << csiParts.size();
                }
                bfxLoad = true;
                break;

            case PartNameRc:
            case PartTypeRc:
            case TexMapRc:
            case MLCadGroupRc:
            case LDCadGroupRc:
            case LeoCadModelRc:
            case LeoCadPieceRc:
            case LeoCadSynthRc:
            case LeoCadGroupBeginRc:
            case LeoCadGroupEndRc:
                CsiItem::partLine(line,pla,opts.current.lineNumber,rc);
                break;

            case LeoCadCameraRc:
            case LeoCadLightRc:
            case LeoCadLightTypeRc:
            case LeoCadLightPOVRayRc:
            case LeoCadLightShadowless:
                if (line.contains(" LOCAL "))
                    line.remove("LOCAL ");
                CsiItem::partLine(line,pla,opts.current.lineNumber,rc);
                if (rc == LeoCadLightTypeRc) {
                    // Light TYPE and NAME written on same line
                    int index = line.size() - line.lastIndexOf("NAME") - 5;
                    const QString name = line.right(index).replace("\"", "");
                    curMeta.LeoCad.light.name.setValue(name);

                    LightData lightData = curMeta.LeoCad.light.value();
                    const QString lightKey = QString("%1 %2").arg(lightData.type, name);
                    lightList.insert(lightKey, lightData);
                }
                else
                    if (rc == LeoCadLightPOVRayRc)
                        curMeta.LeoCad.light.povrayLight = true;
                    else
                    if (rc == LeoCadLightShadowless)
                        curMeta.LeoCad.light.shadowless = true;
                break;

            case IncludeRc:
                includeFileRc = Rc(Gui::include(curMeta,includeHere.lineNumber,includeFileFound)); // includeHere and inserted are include(...) vars
                if (includeFileRc == IncludeFileErrorRc) {
                    includeFileRc = EndOfIncludeFileRc;
                    emit gui->parseErrorSig(tr("INCLUDE file was not resolved."),opts.current,Preferences::IncludeFileErrors);  // file parse error
                } else if (includeFileRc != EndOfIncludeFileRc) {                             // still reading so continue
                    resetIncludeRc = false;                                                   // do not reset, allow includeFileRc to execute
                    continue;
                }
                break;

                /* substitute part/parts with this */

            case PliBeginSub1Rc:
            case PliBeginSub2Rc:
            case PliBeginSub3Rc:
            case PliBeginSub4Rc:
            case PliBeginSub5Rc:
            case PliBeginSub6Rc:
            case PliBeginSub7Rc:
            case PliBeginSub8Rc:
                if (pliIgnore) {
                    emit gui->parseErrorSig(tr("Nested PLI BEGIN/ENDS not allowed"),opts.current, Preferences::ParseErrors, false, true);
                    return static_cast<int>(HitInvalidLDrawLine);
                }

                if (steps->meta.LPub.pli.show.value()
                        && ! excludedPart
                        && ! pliIgnore
                        && ! partIgnore
                        && ! buildModPliIgnore
                        && ! synthBegin) {
                    QString addPart = QString("1 %1  0 0 0  0 0 0 0 0 0 0 0 0 %2")
                            .arg(curMeta.LPub.pli.begin.sub.value().color,
                                 curMeta.LPub.pli.begin.sub.value().part);
                    opts.pliParts << Pli::partLine(addPart,opts.current,curMeta);
                }

                if (gui->isSubmodel(curMeta.LPub.pli.begin.sub.value().part)) {
                    if (curMeta.LPub.setSubstituteAsUnofficialPart.value()) {
                        lpub->ldrawFile.setUnofficialPart(curMeta.LPub.pli.begin.sub.value().part, UNOFFICIAL_PART);
                    } else {
                        QString message, here = tr("(file: %1, line: %2)").arg(opts.current.modelName).arg(opts.current.lineNumber + 1);
                        if (!Preferences::modeGUI && Preferences::lpub3dLoaded) {
                            lpub->ldrawFile.setUnofficialPart(curMeta.LPub.pli.begin.sub.value().part, UNOFFICIAL_PART);
                            message = tr("Substitute part %1 detected as a submodel %2. Subfile set to Unofficial Part")
                                         .arg(curMeta.LPub.pli.begin.sub.value().part, here);
                            emit gui->messageSig(LOG_WARNING, message);
                        } else {
                            message = tr("Substitute part '%1' detected as a submodel %2.<br>"
                                         "Consider adding an !LDRAW_ORG unofficial part header to this subfile.<br>"
                                         "<br>Or click Yes to to set this subfile as an unofficial part now ?")
                                    .arg(curMeta.LPub.pli.begin.sub.value().part, here);
                            auto Rc = QMessageBox::warning(gui,tr("Substitute Part Warning").arg(VER_PRODUCTNAME_STR), message,
                                                           QMessageBox::No|QMessageBox::Yes|QMessageBox::Abort,QMessageBox::Yes);
                            switch (Rc) {
                            case QMessageBox::Yes:
                                lpub->ldrawFile.setUnofficialPart(curMeta.LPub.pli.begin.sub.value().part, UNOFFICIAL_PART);
                                break;
                            case QMessageBox::Abort:
                                Gui::setAbortProcess(true);
                                Gui::displayPageNum = Gui::prevDisplayPageNum;
                                if (Gui::exporting()) {             // exporting
                                    emit gui->setExportingSig(false);
                                } else if (Gui::ContinuousPage()) { // continuous page processing
                                    emit gui->setContinuousPageSig(false);
                                    while (Gui::pageProcessRunning != PROC_NONE) {
                                        QTime waiting = QTime::currentTime().addMSecs(500);
                                        while (QTime::currentTime() < waiting)
                                            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
                                    }
                                }
                                break;
                            default:
                                break;
                            }
                        }
                    }
                }

                if (step == nullptr && ! noStep && ! buildMod.ignore) {
                    if (range == nullptr) {
                        range = Gui::newRange(steps,opts.calledOut);
                        steps->append(range);
                    }
                    step = new Step(topOfStep,
                                    range,
                                    opts.stepNum,
                                    curMeta,
                                    opts.calledOut,
                                    multiStep);

                    range->append(step);
                }
                pliIgnore = true;
                break;

                /* do not put subsequent parts into PLI */
            case PliBeginIgnRc:
                if (pliIgnore) {
                    emit gui->parseErrorSig(tr("Nested PLI BEGIN/ENDS not allowed"),opts.current, Preferences::ParseErrors, false, true);
                    return static_cast<int>(HitInvalidLDrawLine);
                }

                pliIgnore = true;
                break;
            case PliEndRc:
                if ( ! pliIgnore) {
                    emit gui->parseErrorSig(tr("PLI END with no PLI BEGIN"),opts.current, Preferences::ParseErrors, false, true);
                    return static_cast<int>(HitInvalidLDrawLine);
                }

                pliIgnore = false;
                curMeta.LPub.pli.begin.sub.clearAttributes();
                break;

            case AssemAnnotationIconRc:
                if (assemAnnotation) {
                    emit gui->parseErrorSig(tr("Nested ASSEM ANNOTATION ICON not allowed"),opts.current, Preferences::ParseErrors, false, true);
                    return static_cast<int>(HitInvalidLDrawLine);
                } else {
                    if (step && ! Gui::exportingObjects())
                        step->appendCsiAnnotation(opts.current,curMeta.LPub.assem.annotation/*,view*/);
                    assemAnnotation = false;
                }
                break;

                /* discard subsequent parts, and don't create CSI's for them */
            case PartBeginIgnRc:
            case MLCadSkipBeginRc:
                if (partIgnore)
                    emit gui->parseErrorSig(tr("Nested BEGIN/ENDS not allowed"),opts.current);

                partIgnore = true;
                break;

            case PartEndRc:
            case MLCadSkipEndRc:
                if (! partIgnore)
                    emit gui->parseErrorSig(tr("Ignore ending with no ignore begin"),opts.current);

                partIgnore = false;
                break;

            case SynthBeginRc:
                if (synthBegin)
                    emit gui->parseErrorSig(tr("Nested LSynth BEGIN/ENDS not allowed"),opts.current);

                synthBegin = true;
                break;

            case SynthEndRc:
                if ( ! synthBegin)
                    emit gui->parseErrorSig(tr("LSynth ignore ending with no ignore begin"),opts.current);

                synthBegin = false;
                break;

                /* remove a group or all instances of a part type */
            case RemoveGroupRc:
            case RemovePartTypeRc:
            case RemovePartNameRc:
            {
                if (! buildMod.ignore) {
                    QStringList newCSIParts;
                    QVector<int> newLineTypeIndexes;
                    if (rc == RemoveGroupRc) {
                        Gui::remove_group(opts.csiParts,opts.lineTypeIndexes,curMeta.LPub.remove.group.value(),newCSIParts,newLineTypeIndexes,&curMeta);
                    } else if (rc == RemovePartTypeRc) {
                        Gui::remove_parttype(opts.csiParts,opts.lineTypeIndexes,curMeta.LPub.remove.parttype.value(),newCSIParts,newLineTypeIndexes);
                    } else {
                        Gui::remove_partname(opts.csiParts,opts.lineTypeIndexes,curMeta.LPub.remove.partname.value(),newCSIParts,newLineTypeIndexes);
                    }
                    opts.csiParts = newCSIParts;
                    opts.lineTypeIndexes = newLineTypeIndexes;

                    if (step == nullptr && ! noStep) {
                        if (range == nullptr) {
                            range = Gui::newRange(steps,opts.calledOut);
                            steps->append(range);
                        }
                        step = new Step(topOfStep,
                                        range,
                                        opts.stepNum,
                                        curMeta,
                                        opts.calledOut,
                                        multiStep);

                        range->append(step);
                    }
                } // ! buildModIgnore
            }
                break;

            case ReserveSpaceRc:
                /* since we have a part usage, we have a valid step */
                if (opts.calledOut || multiStep) {
                    step = nullptr;
                    Reserve *reserve = new Reserve(topOfStep/*opts.current*/,steps->meta.LPub);
                    if (range == nullptr) {
                        range = Gui::newRange(steps,opts.calledOut);
                        steps->append(range);
                    }
                    range->append(reserve);
                }
                break;

            case InsertFinalModelRc:
            case InsertDisplayModelRc:
            {
                // This is not a step but it's necessary to use the step object to place the model
                // Increment the step number down - so basically use the previous step number for this step.
                // Do this before creating the step so we can use the file name during csi generation
                // to indicate this step file is not an actual step - just a model display
                // The Display Model command syntax is:
                // 0 [ROT]STEP
                // 0 !LPUB INSERT PAGE
                // 0 !LPUB ASSEM CAMERA_ANGLES LOCAL 40.0000 65.0000
                // 0 !LPUB ASSEM MODEL_SCALE LOCAL  2.0000
                // 0 !LPUB INSERT DISPLAY_MODEL
                // 0 [ROT]STEP
                // Note that LOCAL settings must be placed before INSERT PAGE meta command

                Where top = opts.current;
                DisplayType displayType = DT_MODEL_DEFAULT;
                bool proceed = true;
                if (rc == InsertFinalModelRc) {
                    displayType = DT_MODEL_FINAL;
                    proceed = Preferences::enableFadeSteps || Preferences::enableHighlightStep;
                    if (Gui::stepContains(top,partTypeLineRx)) {
                        QString const message = tr("INSERT MODEL meta must be preceded by 0 [ROT]STEP before part (type 1-5)");
                        emit gui->parseErrorSig(message, opts.current, Preferences::InsertErrors, Preferences::ParseErrors, false, true);
                        return static_cast<int>(HitInvalidLDrawLine);
                    }
                } else { /*InsertDisplayModelRc*/
                    QString parseErrorMsg;
                    if (lpub->page.coverPage)
                        parseErrorMsg = tr("Cannot insert a display model to a cover page.");
                    if (multiStep)
                        parseErrorMsg = tr("Cannot insert a display model to a multi-step page.");
                    if (opts.calledOut)
                        parseErrorMsg = tr("Cannot insert a display model to a called out page.");
                    if (!parseErrorMsg.isEmpty())
                        emit gui->parseErrorSig(parseErrorMsg,opts.current);
                    curMeta.LPub.assem.showStepNumber.setValue(false);
                    opts.displayModel = true;
                    if (Gui::stepContains(top,partTypeLineRx)) {
                        displayType = DT_MODEL_DISPLAY; // Is this here to distinguish parts added ? No - it's here to distinguish an inserted submodel
                        opts.csiParts.clear();
                        opts.lineTypeIndexes.clear();
                    }
                    saveRotStep = curMeta.rotStep.value();
                }
                if (proceed) {
                    opts.stepNum--;
                    if (step == nullptr) {
                        if (range == nullptr) {
                            range = Gui::newRange(steps,opts.calledOut);
                            steps->append(range);
                        }
                        step = new Step(topOfStep,
                                        range,
                                        opts.stepNum,
                                        curMeta,
                                        opts.calledOut,
                                        multiStep);
                        range->append(step);
                    }
                    step->displayStep = displayType;
                }
            }
                break;

            case InsertCoverPageRc:
            {
                // Note scan forward and back checks here are partially redundant
                // Similar check (does not start form top of step) performed in parse command.
                Where top = opts.current;
                coverPage = true;
                partsAdded = true;
                lpub->page.coverPage = true;
                QString message = tr("%1 command must be preceded by 0 [ROT]STEP before part (type 1 - 5) at line");
                if (includeFileFound)
                    message = message.arg(QLatin1String("INCLUDE command containing %1"));
                static QRegularExpression rx("^0 !?LPUB INSERT COVER_PAGE (FRONT|BACK)?$");
                QRegularExpressionMatch match = rx.match(line);
                if (match.hasMatch() && match.captured(1) == "BACK") {
                    lpub->page.backCover  = true;
                    lpub->page.frontCover = false;
                    message = message.arg(QLatin1String("INSERT COVER_PAGE BACK"));
                } else if (match.hasMatch() && match.captured(1) == "FRONT") {
                    lpub->page.frontCover = true;
                    lpub->page.backCover  = false;
                    message = message.arg(QLatin1String("INSERT COVER_PAGE FRONT"));
                } else {
                    message = message.arg(QLatin1String("INSERT COVER_PAGE"));
                }
                if (Gui::stepContains(top,partTypeLineRx))
                    emit gui->parseErrorSig(message.append(QString(" %1.").arg(top.lineNumber+1)), opts.current, Preferences::InsertErrors, false, false/*override*/);
            }
                break;

            case InsertPageRc:
            {
                partsAdded = true;

                // nothing to display in 3D Window
                if (! Gui::exporting())
                    emit gui->clearViewerWindowSig();
            }
                break;

            case InsertRc:
            {
                InsertData insertData = curMeta.LPub.insert.value();
                if (insertData.type == InsertData::InsertRotateIcon) // indicate that we have a rotate icon for this step
                    rotateIcon = (opts.calledOut && opts.assembledCallout ? false : true);

                if (insertData.type == InsertData::InsertBom) {
                    // nothing to display in 3D Window
                    //if (! Gui::exporting())
                    //    emit clearViewerWindowSig();
                }

                if (insertData.type == InsertData::InsertText ||
                        insertData.type == InsertData::InsertRichText) {
                    if (insertData.defaultPlacement &&
                            !curMeta.LPub.page.textPlacement.value())
                        curMeta.LPub.insert.initPlacement();
                }

                inserts.append(curMeta.LPub.insert);                  // these are always placed before any parts in step
            }
                break;

            case FadeRc:
            case SilhouetteRc:
            case ColourRc:
                if (step && step->displayStep == DT_MODEL_DISPLAY)
                    CsiItem::partLine(line,pla,opts.current.lineNumber,rc);
                break;

            case SceneItemZValueDirectionRc:
                Gui::setSceneItemZValueDirection(&selectedSceneItems, curMeta, line);
                break;

            case PliPartGroupRc:
                curMeta.LPub.pli.pliPartGroup.setWhere(opts.current);
                opts.pliPartGroups.append(curMeta.LPub.pli.pliPartGroup);
                break;

            case PagePointerRc:
            {
                if (pagePointer) {
                    emit gui->parseErrorSig(tr("Nested page pointers not allowed within the same page."),opts.current, Preferences::ParseErrors, false, true);
                    return static_cast<int>(HitInvalidLDrawLine);
                } else {
                    Positions position    = PP_LEFT;
                    PointerMeta ppm       = curMeta.LPub.page.pointer;
                    PointerAttribMeta pam = curMeta.LPub.page.pointerAttrib;
                    PointerAttribData pad = pam.valueInches();
                    RectPlacement currRp  = curMeta.LPub.page.pointer.value().rectPlacement;

                    if (currRp == TopInside)
                        position = PP_TOP;
                    else
                    if (currRp == BottomInside)
                        position = PP_BOTTOM;
                    else
                    if (currRp == LeftInside)
                        position = PP_LEFT;
                    else
                    if (currRp == RightInside)
                        position = PP_RIGHT;

                    if (!partsAdded) {
                        Where top = opts.current;
                        if (Gui::stepContains(top,partTypeLineRx)) {
                            if (step == nullptr) {
                                if (range == nullptr) {
                                    range = Gui::newRange(steps,opts.calledOut);
                                    steps->append(range);
                                }
                                step = new Step(topOfStep,
                                                range,
                                                opts.stepNum,
                                                curMeta,
                                                opts.calledOut,
                                                multiStep);
                                range->append(step);
                            }
                        }
                    }

                    if (step) {
                        pagePointer = pagePointers.value(position);
                        if (pagePointer) {
                            pad.id     = pagePointer->pointerList.size() + 1;
                            pad.parent = PositionNames[position];
                            pagePointer->parentStep = step;
                            pam.setValueInches(pad);
                            pagePointer->appendPointer(opts.current,ppm,pam);
                            pagePointers.remove(position);
                            pagePointers.insert(position,pagePointer);
                        } else {
                            pagePointer = new PagePointer(&curMeta,view);
                            pagePointer->parentStep = step;
                            pagePointer->setTopOfPagePointer(opts.current);
                            pagePointer->setBottomOfPagePointer(opts.current);
                            if (multiStep) {
                                pagePointer->parentRelativeType = StepGroupType;
                            } else if (opts.calledOut) {
                                pagePointer->parentRelativeType = CalloutType;
                            } else {
                                pagePointer->parentRelativeType = step->relativeType;
                            }
                            PlacementMeta placementMeta;
                            placementMeta.setValue(currRp, PageType);
                            pagePointer->placement = placementMeta;

                            pad.id     = 1;
                            pad.parent = PositionNames[position];
                            pam.setValueInches(pad);
                            pagePointer->appendPointer(opts.current,ppm,pam);
                            pagePointers.insert(position,pagePointer);
                        }
                        pagePointer = nullptr;
                    } else {
                        emit gui->parseErrorSig(tr("You cannot insert a page pointer on this page.<br>"
                                                   "A valid item to point to was not detected."),opts.current);
                    }
                }
            }
                break;

            case PagePointerAttribRc:
            {
                PointerAttribMeta pam = curMeta.LPub.page.pointerAttrib;
                pam.setValueInches(pam.parseAttributes(tokens,opts.current));

                Positions position = PP_LEFT;
                if (pam.value().parent == QLatin1String("BASE_TOP"))
                    position = PP_TOP;
                else
                if (pam.value().parent == QLatin1String("BASE_BOTTOM"))
                    position = PP_BOTTOM;
                else
                if (pam.value().parent == QLatin1String("BASE_LEFT"))
                    position = PP_LEFT;
                else
                if (pam.value().parent == QLatin1String("BASE_RIGHT"))
                    position = PP_RIGHT;

                PagePointer *pp = pagePointers.value(position);
                if (pp) {
                    Pointer *p = nullptr;
                    int i  = pam.value().id - 1;
                    int validIndex = pp->pointerList.size() - 1; /*0-index*/
                    if (i <= validIndex) {
                        p = pp->pointerList[i];
                    } else {
                        emit gui->parseErrorSig(tr("Invalid Page pointer attribute index.<br>"
                                                   "Expected value &#60;= %1, received %2")
                                   .arg(validIndex).arg(i),opts.current);
                        break;
                    }
                    if (p && pam.value().id == p->id) {
                        pam.setOtherDataInches(p->getPointerAttribInches());
                        p->setPointerAttribInches(pam);
                        pp->pointerList.replace(i,p);
                        pagePointers.remove(position);
                        pagePointers.insert(position,pp);
                    }
                } else {
                    emit gui->parseErrorSig(tr("Page Position %1 does not exist.").arg(PositionNames[position]),opts.current);
                }
            }
                break;

            case CalloutBeginRc:
                if (callout) {
                    emit gui->parseErrorSig(tr("Nested CALLOUT not allowed within the same file"),opts.current, Preferences::ParseErrors, false, true);
                    return static_cast<int>(HitInvalidLDrawLine);
                } else if (! buildModIgnoreOverride(buildMod.ignore, buildModTypeIgnore)) {
                    emit gui->parseErrorSig(tr("Failed to process previous BUILD_MOD action for CALLOUT."),opts.current, Preferences::ParseErrors, false, true);
                    return static_cast<int>(HitInvalidLDrawLine);
                } else {
                    callout = new Callout(curMeta,view);
                    callout->setTopOfCallout(opts.current);
                }
                break;

            case CalloutPointerRc:
                if (callout) {
                    callout->appendPointer(opts.current,
                                           curMeta.LPub.callout.pointer,
                                           curMeta.LPub.callout.pointerAttrib);
                }
                break;

            case CalloutPointerAttribRc:
                if (callout) {
                    PointerAttribMeta pam = curMeta.LPub.callout.pointerAttrib;
                    pam.setValueInches(pam.parseAttributes(tokens,opts.current));
                    if (!pam.value().id)
                        break;
                    Pointer          *p = nullptr;
                    int i               = pam.value().id - 1;
                    int validIndex      = callout->pointerList.size() - 1; /*0-index*/
                    if (i <= validIndex) {
                        p = callout->pointerList[i];
                    } else {
                        emit gui->parseErrorSig(tr("Invalid Callout pointer attribute index.<br>"
                                                   "Expected value &#60;= %1, received %2")
                                                   .arg(validIndex).arg(i),opts.current);
                        break;
                    }
                    if (p && pam.value().id == p->id) {
                        pam.setOtherDataInches(p->getPointerAttribInches());
                        p->setPointerAttribInches(pam);
                        callout->pointerList.replace(i,p);
                    }
                }
                break;

            case CalloutDividerRc:
                if (range) {
                    range->sepMeta = curMeta.LPub.callout.sep;
                    dividerType = (line.contains("STEPS") ? StepDivider : RangeDivider);
                    Gui::set_divider_pointers(curMeta,opts.current,range,view,dividerType,opts.stepNum,CalloutDividerRc);
                    if (dividerType != StepDivider) {
                        range = nullptr;
                        step = nullptr;
                    }
                }
                break;

            case CalloutEndRc:
                if ( ! callout) {
                    emit gui->parseErrorSig(tr("CALLOUT END without a CALLOUT BEGIN"),opts.current, Preferences::ParseErrors, false, true);
                    return static_cast<int>(HitInvalidLDrawLine);
                }
                else
                if (! step) {
                    emit gui->parseErrorSig(tr("CALLOUT does not contain a valid STEP"),opts.current, Preferences::ParseErrors, false, true);
                    return static_cast<int>(HitInvalidLDrawLine);
                }
                else
                {
                    callout->parentStep = step;
                    if (multiStep) {
                        callout->parentRelativeType = StepGroupType;
                    } else if (opts.calledOut) {
                        callout->parentRelativeType = CalloutType;
                    } else {
                        callout->parentRelativeType = step->relativeType;
                    }
                    // set csi annotations - callout
                    //if (! Gui::exportingObjects())
                    //    callout->setCsiAnnotationMetas();
                    callout->pli.clear();
                    callout->placement = curMeta.LPub.callout.placement;
                    callout->margin = curMeta.LPub.callout.margin;
                    callout->setBottomOfCallout(opts.current);
                    callout = nullptr;
                }
                break;

            case StepGroupBeginRc:
                if (opts.calledOut) {
                    emit gui->parseErrorSig(tr("MULTI_STEP not allowed inside callout models"),opts.current, Preferences::ParseErrors, false, true);
                    return static_cast<int>(HitInvalidLDrawLine);
                } else if (multiStep) {
                    emit gui->parseErrorSig(tr("Nested MULTI_STEP not allowed"),opts.current, Preferences::ParseErrors, false, true);
                    return static_cast<int>(HitInvalidLDrawLine);
                } else if (! (multiStep = buildModIgnoreOverride(buildMod.ignore, buildModTypeIgnore))) {
                    emit gui->parseErrorSig(tr("Failed to process previous BUILD_MOD action for MULTI_STEP."),opts.current);
                } else {
                    steps->relativeType = StepGroupType;
                    steps->setTopOfSteps(opts.current);
                }
                break;

            case StepGroupDividerRc:
                if (multiStep && range) {
                    range->sepMeta = steps->meta.LPub.multiStep.sep;
                    dividerType = (line.contains("STEPS") ? StepDivider : RangeDivider);
                    Gui::set_divider_pointers(curMeta,opts.current,range,view,dividerType,opts.stepNum,StepGroupDividerRc);
                    if (dividerType != StepDivider) {
                        range = nullptr;
                        step = nullptr;
                    }
                }
                break;

                /* finished off a multiStep */
            case StepGroupEndRc:
                if ((multiStepPage = multiStep && steps->list.size())) {
                    // save the current meta as the meta for step group
                    // PLI for non-pli-per-step
                    if (partsAdded) {
                        emit gui->parseErrorSig(tr("Expected STEP before MULTI_STEP END"), opts.current, Preferences::ParseErrors, false, true);
                        return static_cast<int>(HitInvalidLDrawLine);
                    }

                    multiStep = false;

                   /*
                    * TODO
                    * Consider setting steps->meta to current meta here. This way we pass the latest settings
                    * to formatPage processing - this is particularly helpful if we want to capture step group
                    * metas that are set in steps beyond the first group step by the editor.
                    * Steps can be updated after the first step group step (when the groupStepMeta is set).
                    * Populating the groupStepMeta at the first group step is necessary to capture any
                    * step group specific settings as there is no reasonable way to know when the step group
                    * ends and capturing the curMeta after is too late as it is popped at the end of every step.
                    * steps->meta = curMeta
                    */

                    // get the default number of submodel instances in the model file
                    instances = lpub->ldrawFile.instances(opts.current.modelName, opts.isMirrored);
                    if (!opts.displayModel && countInstances)
                        displayInstanceCount = instances > 1 || steps->meta.LPub.page.countInstanceOverride.value() > 1;
                    // count the instances - use steps->meta (vs. steps->groupStepMeta) to access current submodelStack
                    //
                    // lpub->ldrawFile.instances() always return results for CountAtTop - the historic LPub count scheme.
                    // However, the default countInstances configuration is CountAtModel - configurable in Project globals.
                    // lpub->mi.countInstances(...) uses countInstances and opts.renderModelColour to return instances.
                    if (!opts.displayModel && displayInstanceCount) {
                        // manually override the count instance value using 0 !LPUB SUBMODEL_INSTANCE_COUNT_OVERRIDE
                        if (steps->groupStepMeta.LPub.page.countInstanceOverride.value())
                            instances = steps->groupStepMeta.LPub.page.countInstanceOverride.value();
                        else
                            instances = lpub->mi.countInstances(&steps->meta, opts.current.modelName, opts.renderModelColour, countInstances);
                    }
/*
#ifdef QT_DEBUG_MODE
                    if (steps->meta.LPub.multiStep.pli.perStep.value() !=
                            steps->groupStepMeta.LPub.multiStep.pli.perStep.value())
                        emit gui->messageSig(LOG_TRACE, QString("COMPARE - StepGroup PLI per step: stepsMeta %1")
                                                                .arg(steps->meta.LPub.multiStep.pli.perStep.value()
                                                                ? QString("[On], groupStepMeta [Off]")
                                                                : QString("[Off], groupStepMeta [On]")));
#endif
//*/
                    // Pli per Step settings
                    if (opts.pliParts.size() && /*steps->meta*/steps->groupStepMeta.LPub.multiStep.pli.perStep.value() == false) {
                        PlacementData placementData;
                        // Override default assignments, which is for PliPerStep true
                        // Group step number shown
                        if (/*steps->meta*/steps->groupStepMeta.LPub.multiStep.showGroupStepNumber.value()) {
                            placementData = steps->groupStepMeta.LPub.multiStep.stepNum.placement.value();
                            if ((placementData.relativeTo    == CsiType ||
                                 (placementData.relativeTo    == PartsListType &&
                                  placementData.justification == Top &&
                                  placementData.placement     == Left &&
                                  placementData.preposition   == Outside))) {
                                // Place the grpup step number relative to the page header
                                steps->groupStepMeta.LPub.multiStep.stepNum.placement.setValue(BottomLeftOutside,PageHeaderType);
                            }

                            // update the step number
                            steps->groupStepNumber.placement = steps->groupStepMeta.LPub.multiStep.stepNum.placement;
                            steps->groupStepNumber.margin    = steps->groupStepMeta.LPub.multiStep.stepNum.margin;
                            steps->groupStepNumber.number    = opts.groupStepNumber;
                            steps->groupStepNumber.sizeit();

                            // set PLI placement
                            bool validStepNumber = steps->groupStepNumber.number > 0;
                            if (validStepNumber && placementData.relativeTo == PageHeaderType) {
                                // set PLI relativeTo step number
                                steps->groupStepMeta.LPub.multiStep.pli.placement.setValue(StepNumberType);
                            }
#ifdef QT_DEBUG_MODE
                            emit gui->messageSig(LOG_DEBUG, QString("Add Step group step number for multi-step page %1").arg(opts.current.modelName));
#endif
                            // if PLI and Submodel Preview are relative to StepNumber or PLI relative to CSI (default)
                            placementData = steps->groupStepMeta.LPub.multiStep.pli.placement.value();
                            if (steps->placeSubModel) {
                                if (!validStepNumber && steps->groupStepMeta.LPub.multiStep.subModel.placement.value().relativeTo == StepNumberType) {
                                    // Redirect Submodel Preview relative to Page
                                    steps->groupStepMeta.LPub.multiStep.subModel.placement.setValue(BottomLeftOutside,PageHeaderType);
                                }
                                // Redirect Pli relative to SubModel Preview
                                if (placementData.relativeTo == StepNumberType || placementData.relativeTo == CsiType)  {
                                    steps->groupStepMeta.LPub.multiStep.pli.placement.setValue(BottomLeftOutside,SubModelType);
                                }
                            }
                            // Redirect Pli relative to Step Number if relative to CSI (default)
                            else if(placementData.relativeTo == CsiType) {
                                if (validStepNumber) {
                                    // set PLI relativeTo step number
                                    steps->groupStepMeta.LPub.multiStep.pli.placement.setValue(RightTopOutside,StepNumberType);
                                } else {
                                    // or place the PLI relative to page header
                                    steps->groupStepMeta.LPub.multiStep.pli.placement.setValue(BottomLeftOutside,PageHeaderType);
                                }
                            }
                        }
                        // Group step number not shown
                        else {
                            placementData = steps->groupStepMeta.LPub.multiStep.pli.placement.value();
                            // if Submodel Preview relative to StepNumber
                            if (steps->placeSubModel &&
                                    steps->groupStepMeta.LPub.multiStep.subModel.placement.value().relativeTo == StepNumberType) {
                                // Redirect Submodel Preview relative to Page
                                steps->groupStepMeta.LPub.multiStep.subModel.placement.setValue(BottomLeftOutside,PageHeaderType);
                            }
                            // if Pli relative to StepNumber or CSI
                            if (steps->groupStepMeta.LPub.multiStep.pli.placement.value().relativeTo == StepNumberType ||
                                    steps->groupStepMeta.LPub.multiStep.pli.placement.value().relativeTo == CsiType) {
                                if (steps->placeSubModel) {
                                    // Redirect Pli relative to SubModel Preview
                                    steps->groupStepMeta.LPub.multiStep.pli.placement.setValue(BottomLeftOutside,SubModelType);
                                }
                                else if(placementData.relativeTo == CsiType) {
                                    // Redirect Pli relative to Page if relative to CSI (default)
                                    steps->groupStepMeta.LPub.multiStep.pli.placement.setValue(BottomLeftOutside,PageHeaderType);
                                }
                            }
                        }

                        // PLI
                        steps->pli.bom    = false;
                        steps->pli.margin = steps->groupStepMeta.LPub.multiStep.pli.margin;
                        steps->pli.placement = steps->groupStepMeta.LPub.multiStep.pli.placement;
                        steps->pli.setParts(opts.pliParts,opts.pliPartGroups,steps->groupStepMeta);

                        if (!Gui::ContinuousPage())
                            emit gui->messageSig(LOG_STATUS, tr("Add PLI images for multi-step page %1").arg(opts.current.modelName));

                        if (steps->pli.sizePli(&steps->groupStepMeta, StepGroupType, false) != 0)
                            emit gui->messageSig(LOG_ERROR, tr("Failed to set PLI (per Page) for %1...").arg(topOfStep.modelName));

                        // SubModel Preview
                        if (steps->placeSubModel && steps->groupStepMeta.LPub.multiStep.subModel.show.value()) {
                            steps->groupStepMeta.LPub.subModel.instance.setValue(instances);
                            steps->subModel.margin = steps->groupStepMeta.LPub.subModel.margin;
                            steps->subModel.placement = steps->groupStepMeta.LPub.multiStep.subModel.placement;
                            steps->subModel.setSubModel(opts.current.modelName,steps->groupStepMeta);

                            if (!Gui::ContinuousPage())
                                emit gui->messageSig(LOG_INFO_STATUS, tr("Add Submodel Preview for multi-step page %1").arg(opts.current.modelName));

                            steps->subModel.displayInstanceCount = displayInstanceCount;
                            if (steps->subModel.sizeSubModel(&steps->groupStepMeta,StepGroupType,false) != 0)
                                emit gui->messageSig(LOG_ERROR, tr("Failed to set Submodel Preview for %1...").arg(topOfStep.modelName));
                        }
                    } // pli per step = false
                    else
                    { // pli per step = true
                        steps->groupStepNumber.number     = 0;
                        steps->groupStepNumber.stepNumber = nullptr;

                        // check the pli placement to be sure it's relative to CsiType or, if ok, StepNumberType
                        bool stepNumRelativeToPli = steps->groupStepMeta.LPub.multiStep.stepNum.placement.value().relativeTo == PartsListType;
                        PlacementType pliRelativeTo = steps->groupStepMeta.LPub.multiStep.pli.placement.value().relativeTo;

                        if (pliRelativeTo != CsiType || (stepNumRelativeToPli && pliRelativeTo == StepNumberType)) {
                            PlacementData placementData = steps->groupStepMeta.LPub.multiStep.pli.placement.value();
                            QString message = tr("Invalid PLI placement.<br>"
                                                 "Step group PLI per STEP set to TRUE but PLI placement is %1 %2 %3.<br>"
                                                 "The PLI should be relative to %4.<br>"
                                                 "A valid placement is MULTI_STEP PLI PLACEMENT %5 %6 %7.")
                                    .arg(placementNames[  placementData.placement],
                                         relativeNames [  placementData.relativeTo],
                                         prepositionNames[placementData.preposition],
                                         tr("%1%2").arg(relativeNames [CsiType], stepNumRelativeToPli
                                                                                 ? ""
                                                                                 : tr(" or %1").arg(relativeNames [StepNumberType])),
                                         placementNames[  TopLeft],
                                         relativeNames [  CsiType],
                                         prepositionNames[Outside]);
                            emit gui->parseErrorSig(message, opts.current,Preferences::ParseErrors,false,true/*overide*/);
                        }
                    }

                    opts.pliParts.clear();
                    opts.pliPartGroups.clear();

                    /* this is a page we're supposed to process */

                    // Update steps->meta args with stepGroup args - perhaps update entire steps->meta ?
                    // because steps->meta is popped at the top of the STEP containing MULTI_STEP END
                    steps->placement = steps->groupStepMeta.LPub.multiStep.placement;                  // was steps->meta.LPub.multiStep.placement
                    steps->meta.LPub.page.background = steps->groupStepMeta.LPub.page.background;
                    steps->meta.LPub.multiStep.placement = steps->groupStepMeta.LPub.multiStep.placement;

                    bool endOfSubmodel =
                            /*steps->meta*/steps->groupStepMeta.LPub.contStepNumbers.value() ?
                                /*steps->meta*/steps->groupStepMeta.LPub.contModelStepNum.value() >= lpub->ldrawFile.numSteps(opts.current.modelName) :
                                opts.stepNum - 1 >= lpub->ldrawFile.numSteps(opts.current.modelName);

                    // set csi annotations - multistep
                    if (! Gui::exportingObjects()) {
                        Gui::suspendFileDisplay = true;
                        //steps->setCsiAnnotationMetas();
                        returnValue = static_cast<TraverseRc>(lpub->mi.setCsiAnnotationMetas(steps));
                        Gui::suspendFileDisplay = false;
                        if (Preferences::buildModEnabled && returnValue == HitCsiAnnotation)
                            return static_cast<int>(returnValue);
                    }

                    Page *page = dynamic_cast<Page *>(steps);
                    if (page) {
                        page->inserts              = inserts;
                        page->instances            = instances;
                        page->displayInstanceCount = displayInstanceCount;
                        page->pagePointers         = pagePointers;
                        page->selectedSceneItems   = selectedSceneItems;
                    }

                    if (!Gui::ContinuousPage())
                        emit gui->messageSig(LOG_STATUS, tr("Generate CSI images for multi-step page  %1").arg(opts.current.modelName));

                    if (Render::useLDViewSCall() && opts.ldrStepFiles.size() > 0) {
                        QElapsedTimer timer;
                        timer.start();
                        QString empty("");

                        // renderer parms are added to csiKeys in createCsi call

                        if (static_cast<TraverseRc>(renderer->renderCsi(empty,opts.ldrStepFiles,opts.csiKeys,empty,/*steps->meta*/steps->groupStepMeta)) != HitNothing) {
                            emit gui->messageSig(LOG_ERROR, tr("Render CSI images failed."));
                        }

                        emit gui->messageSig(LOG_INFO,
                                        tr("%1 CSI (Single Call) render took "
                                           "%2 milliseconds to render %3 [Step %4] %5 "
                                           "for %6 step group on page %7.")
                                        .arg(rendererNames[Render::getRenderer()],
                                             Gui::elapsedTime(timer.elapsed(),false))
                                        .arg(opts.ldrStepFiles.size())
                                        .arg(opts.stepNum)
                                        .arg(opts.ldrStepFiles.size() == 1 ? tr("image") : tr("images"),
                                             opts.calledOut ? tr("called out,") : tr("simple,"))
                                        .arg(Gui::stepPageNum));
                    }

                    if (Preferences::modeGUI) {
                        // Load the Visual Editor with the last step of the step group
                        Step *lastStep = step;
                        if (!lastStep) {
                            Range *lastRange = range;
                            if (!lastRange)
                                lastRange = dynamic_cast<Range *>(steps->list[steps->list.size() - 1]);
                            if (lastRange) {
                                int lastStepIndex = lastRange->list.size() - 1;
                                lastStep = dynamic_cast<Step *>(lastRange->list[lastStepIndex]);
                            }
                        }

                        if (lastStep && !lastStep->csiPixmap.isNull()) {
                            emit gui->messageSig(LOG_DEBUG, tr("Step group last step number %1").arg(lastStep->stepNumber.number));
                            lpub->setCurrentStep(lastStep);
                            if (!Gui::exportingObjects()) {
                                gui->showLine(lastStep->topOfStep());
                                lastStep->loadTheViewer();
                            }
                        } else if (!Gui::exportingObjects() && step) {
                            lpub->setCurrentStep(step);
                            gui->showLine(steps->topOfSteps());
                        }
                    }

                    if ((returnValue = static_cast<TraverseRc>(Gui::addGraphicsPageItems(steps,coverPage,endOfSubmodel,opts.printing))) != HitAbortProcess)
                        returnValue = HitEndOfPage;

                    if (!Gui::ContinuousPage())
                        drawPageElapsedTime();

                    if (Gui::abortProcess())
                        returnValue = HitAbortProcess;
                }

                inserts.clear();
                buildModKeys.clear();
                pagePointers.clear();
                selectedSceneItems.clear();
                if (multiStepPage)
                    return static_cast<int>(returnValue);
                break;

                // Update BuildMod action for 'current' step
            case BuildModApplyRc:
            case BuildModRemoveRc:
                if (!Preferences::buildModEnabled)
                    break;
                if (partsAdded)
                    emit gui->parseErrorSig(tr("BUILD_MOD REMOVE/APPLY action command must be placed before step parts"),
                               opts.current,Preferences::BuildModErrors);
                buildModStepIndex = gui->getBuildModStepIndex(topOfStep);
                buildModActionMeta = curMeta.LPub.buildMod;
                buildMod.key = curMeta.LPub.buildMod.key();
                if (gui->buildModContains(buildMod.key)) {
                    if (gui->getBuildModActionPrevIndex(buildMod.key, buildModStepIndex, rc) < buildModStepIndex)
                        emit gui->parseErrorSig(tr("Redundant build modification meta command '%1' - this command can be removed.")
                                   .arg(buildMod.key),opts.current,Preferences::BuildModErrors);
                } else {
                    const QString action = rc == BuildModApplyRc ? tr("Apply") : tr("Remove");
                    emit gui->parseErrorSig(tr("DrawPage BuildMod key '%1' for %2 action was not found.")
                               .arg(buildMod.key, action),
                               opts.current,Preferences::BuildModErrors);
                }
                buildModActionStep = true;
                if (multiStep || opts.calledOut)
                    buildModChange = topOfStep != steps->topOfSteps(); // uses list[0].topOfStep for both multiStep and callout
                if (buildModChange) {
                    buildModActions.insert(buildMod.level, gui->getBuildModAction(buildMod.key, buildModStepIndex));
                    if (buildModActions.value(buildMod.level) != rc) {
/*
#ifdef QT_DEBUG_MODE
                        const QString message = tr("Build Mod Reset Setup - Key: '%1', Current Action: %2, Next Action: %3")
                                .arg(buildMod.key)
                                .arg(buildMod.action == BuildModRemoveRc ? "Remove(65)" : "Apply(64)")
                                .arg(rc == BuildModRemoveRc ? "Remove(65)" : "Apply(64)");
                        emit gui->messageSig(LOG_NOTICE, message);
                        //qDebug() << qPrintable(QString("DEBUG: %1").arg(message));
#endif
//*/
                        // set BuildMod step key for previous (e.g. BEGIN) action
                        const QString buildModStepKey = gui->getViewerStepKey(gui->getBuildModStepIndex(buildMod.key));

                        // set BuildMod action for current step
                        if (lpub->ldrawFile.setViewerStepHasBuildModAction(buildModStepKey, true))
                            gui->setBuildModAction(buildMod.key, buildModStepIndex, rc);
                        else
                            emit gui->parseErrorSig(tr("Could not preserve previous BuildMod %1 action for key '%2'.<br>Step or key was not found.")
                                       .arg(rc == BuildModApplyRc ? tr("Remove") : tr("Apply"), buildMod.key),
                                       opts.current,Preferences::BuildModErrors, true, false, QMessageBox::Warning);
                        gui->setBuildModAction(buildMod.key, buildModStepIndex, rc);
                        // set buildModStepIndex for writeToTmp() and findPage() content
                        gui->setBuildModNextStepIndex(topOfStep);
                        // Check if CsiAnnotation and process them if any
                        if (! Gui::exportingObjects() && (multiStep || opts.calledOut))
                            lpub->mi.setCsiAnnotationMetas(steps);
                        // Rerun to findPage() to regenerate parts and options for buildMod action
                        Gui::revertPageProcess();
                        return static_cast<int>(HitBuildModAction);
                    } // buildMod action != rc
                } // buildModChange
                break;

                // Get BuildMod attributes and set ModIgnore based on 'current' step buildModActions
            case BuildModBeginRc:
                if (!Preferences::buildModEnabled) {
                    buildMod.ignore = true;
                    break;
                }
                buildModMeta = curMeta.LPub.buildMod;
                buildMod.key = curMeta.LPub.buildMod.key();
                buildModExists = gui->buildModContains(buildMod.key);
                buildMod.level = getLevel(buildMod.key, BM_BEGIN);
                // assign buildMod key
                if (! buildModKeys.contains(buildMod.level))
                    buildModKeys.insert(buildMod.level, buildMod.key);
                // step-group and callout build modifications are parsed and configured in the following lines
                // insert new or update existing buildMod
                if (multiStep)
                    buildModInsert = topOfStep != steps->topOfSteps();
                if (opts.calledOut)
                    buildModInsert = !buildModExists;
                if (buildModInsert)
                    insertAttribute(buildModAttributes, BM_BEGIN_LINE_NUM, opts.current);
                // set buildModActions
                if (buildModExists) {
                    if (multiStep || opts.calledOut) // the last action is appended at each step so it should always be current
                        buildModActions.insert(buildMod.level, gui->getBuildModAction(buildMod.key, BM_LAST_ACTION));
                    else // take the action for the current step if exists or last action
                        buildModActions.insert(buildMod.level, gui->getBuildModAction(buildMod.key, gui->getBuildModStepIndex(topOfStep), BM_LAST_ACTION));
                } else if (buildModInsert) {
                    buildModActions.insert(buildMod.level, BuildModApplyRc);
                } else
                    emit gui->parseErrorSig(tr("Build modification '%1' is not registered. BuildMod key was not found.").arg(buildMod.key),
                               opts.current,Preferences::BuildModErrors);
                // set buildMod and buildModPli ignore
                if (buildModActions.value(buildMod.level) == BuildModApplyRc) {
                    buildMod.ignore = false;
                    buildModPliIgnore = true;
                } else if (buildModActions.value(buildMod.level) == BuildModRemoveRc) {
                    buildMod.ignore = true;
                    buildModPliIgnore = pliIgnore || excludedPart ;
                }
                buildMod.state = BM_BEGIN;
                break;

                // Set modActionLineNum and ModIgnore based on 'current' step buildModActions
            case BuildModEndModRc:
                if (!Preferences::buildModEnabled) {
                    buildMod.ignore = getLevel(QString(), BM_END);
                    break;
                }
                if (buildMod.level > 1 && buildMod.key.isEmpty())
                    emit gui->parseErrorSig(tr("Key required for nested build mod meta command"),
                               opts.current,Preferences::BuildModErrors);
                if (buildMod.state != BM_BEGIN)
                    emit gui->parseErrorSig(tr("Required meta BUILD_MOD BEGIN not found"), opts.current, Preferences::BuildModErrors);
                if (buildModInsert)
                    insertAttribute(buildModAttributes, BM_ACTION_LINE_NUM, opts.current);
                // set buildMod and buildModPli ignore
                if (buildModActions.value(buildMod.level) == BuildModApplyRc) {
                    buildMod.ignore = true;
                    buildModPliIgnore = pliIgnore || excludedPart;
                } else if (buildModActions.value(buildMod.level) == BuildModRemoveRc) {
                    buildMod.ignore = false;
                    buildModPliIgnore = true;
                }
                buildModTypeIgnore = false;
                buildMod.state = BM_END_MOD;
                break;

                // Get buildModLevel and reset ModIgnore to default
            case BuildModEndRc:
                if (!Preferences::buildModEnabled)
                    break;
                if (buildMod.state != BM_END_MOD)
                    emit gui->parseErrorSig(tr("Required meta BUILD_MOD END_MOD not found"), opts.current, Preferences::BuildModErrors);
                if (buildModInsert)
                    insertAttribute(buildModAttributes, BM_END_LINE_NUM, opts.current);
                buildMod.level = getLevel(QString(), BM_END);
                if (buildMod.level == opts.buildModLevel) {
                    buildMod.ignore = false;
                    buildModPliIgnore = pliIgnore || excludedPart;
                }
                buildMod.state = BM_END;
                break;

            case NoStepRc:
                noStep = true;
                break;

                /* we've hit some kind of step, or implied step and end of file */
            case EndOfFileRc:
            case RotStepRc:
            case StepRc:
             /*
              * STEP - special case of step group with NOSTEP as last step and rotated or assembled called out submodel
              */
                if (! buildMod.ignore && noStep && opts.calledOut) {
                    if (opts.current.modelName.contains("whole_rotated_") ||
                            opts.current.modelName.contains("whole_assembled_")) {
                        bool nsHasParts    = false;
                        bool nsHasNoStep   = false;
                        bool nsIsStepGroup = false;
                        Meta nsMeta        = curMeta;
                        QStringList nsTokens;
                        split(addLine,nsTokens);
                        // start with the original subfile content
                        QStringList nsContent = lpub->ldrawFile.contents(nsTokens[14]);
                        int nsNumLines = nsContent.size();
                        Where nsWalkBack(nsTokens[14],nsNumLines);
                        for (; nsWalkBack.lineNumber >= 0; nsWalkBack--) {
                            QString nsLine = gui->readLine(nsWalkBack);
                            if (isHeader(nsLine)) {
                                // if we reached the top of the submodel so break
                                break;
                            } else {
                                nsTokens.clear();
                                split(nsLine,nsTokens);
                                bool nsLine_1_5 = nsTokens.size() && nsTokens[0].size() == 1 &&
                                        nsTokens[0] >= "1" && nsTokens[0] <= "5";
                                bool nsLine_0 = nsTokens.size() > 0 && nsTokens[0] == "0";
                                if (nsLine_1_5) {
                                    // we have a valid part so record part added
                                    nsHasParts = true;
                                } else if (nsLine_0) {
                                    Rc nsrc = nsMeta.parse(nsLine,nsWalkBack,false);
                                    if (nsrc == StepRc || nsrc == RotStepRc) {
                                        // are we in a new step which is in a step group ?
                                        if (nsIsStepGroup) {
                                            // confirm previous step does not have a NOSTEP command
                                            // and parts have been added
                                            if (nsHasParts && !nsHasNoStep) {
                                                // we have a STEP where parts were added and no NOSTEP command encountered
                                                // so this step group can be rendered as a rotated or assembled callout
                                                noStep = false;
                                                break;
                                            } else {
                                                // clear the registers for new parts added and NOSTEP check
                                                nsHasParts  = false;
                                                nsHasNoStep = false;
                                            }
                                        } else {
                                            // the last step did not have MULTI_STEP_END so break
                                            break;
                                        }
                                    } else if (nsrc == NoStepRc) {
                                        // NOSTEP encountered so record it and continue to the top of the step group
                                        nsHasNoStep = true;
                                    } else if (nsrc == StepGroupEndRc) {
                                        // we are in a step group so proceed
                                        nsIsStepGroup = true;
                                    } else if (nsrc == StepGroupBeginRc) {
                                        // we have reached the top of the step group so break
                                        break;
                                    }
                                }
                            }
                        }
                    }
                } // STEP - special case of step group with NOSTEP as last step and rotated or assembled called out submodel

                /*
              * STEP - special case of no parts added, but BFX load or BuildMod Action and not NOSTEP
              */
                if (! partsAdded && ! noStep && (bfxLoad || buildModActionStep)) {
                    if (step == nullptr) {
                        if (range == nullptr) {
                            range = Gui::newRange(steps,opts.calledOut);
                            steps->append(range);
                        }
                        step = new Step(topOfStep,
                                        range,
                                        opts.stepNum,
                                        curMeta,
                                        opts.calledOut,
                                        multiStep);
                        range->append(step);
                    }

                    step->setBottomOfStep(opts.current);

                    QString caseType;
                    if (bfxLoad) {
                        caseType.append("bfx load");
                        step->bfxLoadStep = bfxLoad;
                    }
                    if (buildModActionStep) {
                        caseType.append(" build modification");
                        step->buildModActionStep = buildModActionStep;
                        if (buildModActionMeta.action())
                            step->buildModActionMeta = buildModActionMeta;
                    }
                    emit gui->messageSig(LOG_INFO, tr("Processing CSI %1 special case for %2...").arg(caseType, topOfStep.modelName));

                    step->updateViewer = opts.updateViewer;

                    if (buildModActionMeta.action())
                        step->buildModActionMeta = buildModActionMeta;

                    configuredCsiParts = step->configureModelStep(opts.csiParts, topOfStep);

                    returnValue = static_cast<TraverseRc>(step->createCsi(opts.isMirrored ? addLine : "1 color 0 0 0 1 0 0 0 1 0 0 0 1 foo.ldr",
                                                                          configuredCsiParts,
                                                                          opts.lineTypeIndexes,
                                                                          &step->csiPixmap,
                                                                          steps->meta));
                    if (returnValue == HitAbortProcess || Gui::abortProcess()) {
                        return returnValue;
                    }

                    if (Render::useLDViewSCall() && ! step->ldrName.isNull()) {
                        opts.ldrStepFiles << step->ldrName;
                        opts.csiKeys << step->csiKey; // No parts to process
                    }

                    partsAdded = true; // OK, so this is a lie, but it works
                } // STEP - special case of no parts added, but BFX load or BuildMod Action and not NOSTEP

             /*
              *  STEP - case of not NOSTEP and not BuildMod ignore
              */
                if (! noStep && ! buildMod.ignore) {
                  /*
                   * STEP - normal case of parts added
                   */
                    if (partsAdded) {

                        // set step group page meta attributes first step
                        if (!opts.displayModel && firstGroupStep) {
                            steps->groupStepMeta = curMeta;
                            firstGroupStep = false;
                        }

                        if (pliIgnore) {
                            emit gui->parseErrorSig(tr("PLI BEGIN then STEP. Expected PLI END"),opts.current);
                            pliIgnore = false;
                        }
                        if (partIgnore) {
                            emit gui->parseErrorSig(tr("PART BEGIN then STEP. Expected PART END"),opts.current);
                            partIgnore = false;
                        }
                        if (synthBegin) {
                            emit gui->parseErrorSig(tr("SYNTH BEGIN then STEP. Expected SYNTH_END"),opts.current);
                            synthBegin = false;
                        }

                        bool pliPerStep = false;

                        if (!opts.displayModel) {
                            if (multiStep && steps->meta.LPub.multiStep.pli.perStep.value()) {
                                pliPerStep = true;
                            } else if (opts.calledOut && steps->meta.LPub.callout.pli.perStep.value()) {
                                pliPerStep = true;
                            } else if ( ! multiStep && ! opts.calledOut && steps->meta.LPub.stepPli.perStep.value()) {
                                pliPerStep = true;
                            }
                        }

                     /*
                      * STEP - Actual parts added, simple, mulitStep or calledOut (no render graphics)
                      */
                        if (step) {
                            step->setBottomOfStep(opts.current);

                            Page *page = dynamic_cast<Page *>(steps);
                            if (page) {
                                page->inserts              = inserts;
                                page->pagePointers         = pagePointers;
                                page->displayPage          = step->displayStep;
                                page->selectedSceneItems   = selectedSceneItems;
                            }

                            if (!Gui::ContinuousPage())
                                emit gui->messageSig(LOG_INFO_STATUS, tr("Processing CSI for %1...").arg(topOfStep.modelName));

                            if (opts.displayModel)
                                step->showStepNumber = curMeta.LPub.assem.showStepNumber.value();
                            else if (multiStep || opts.calledOut)
                                lpub->ldrawFile.setPrevStepPosition(opts.current.modelName, opts.stepNum, opts.csiParts.size());

                            step->updateViewer = opts.updateViewer;

                            if (buildModMeta.action())
                                step->buildModMeta = buildModMeta;

                            if (buildModActionMeta.action())
                                step->buildModActionMeta = buildModActionMeta;

                            configuredCsiParts = step->configureModelStep(opts.csiParts, topOfStep);

                            returnValue = static_cast<TraverseRc>(step->createCsi(opts.isMirrored ? addLine : QLatin1String("1 color 0 0 0 1 0 0 0 1 0 0 0 1 foo.ldr"),
                                                                                  configuredCsiParts,
                                                                                  opts.lineTypeIndexes,
                                                                                  &step->csiPixmap,
                                                                                  steps->meta));
                            if (returnValue == HitAbortProcess || Gui::abortProcess()) {
                                return returnValue;
                            }

                            step->lightList = lightList;

                            PlacementType relativeType = SingleStepType;

                            // Pli per Step
                            if (!opts.displayModel && pliPerStep) {
                                if (multiStep) {
                                    relativeType = StepGroupType;
                                } else if (opts.calledOut) {
                                    relativeType = CalloutType;
                                } else {
                                    relativeType = SingleStepType;
                                }

                                step->pli.setParts(opts.pliParts,opts.pliPartGroups,steps->meta);
                                opts.pliParts.clear();
                                opts.pliPartGroups.clear();

                                emit gui->messageSig(LOG_INFO, tr("Processing PLI for %1...").arg(topOfStep.modelName));

                                step->pli.sizePli(&steps->meta,relativeType,pliPerStep);
                            } // Pli per Step

                            // Place SubModel at Step 1
                            if (!opts.displayModel && step->placeSubModel) {
                                emit gui->messageSig(LOG_INFO, tr("Set first step submodel display for %1...").arg(topOfStep.modelName));

                                // get the number of submodel instances in the model file
                                instances = lpub->ldrawFile.instances(opts.current.modelName, opts.isMirrored);
                                if (steps->meta.LPub.subModel.showInstanceCount.value())
                                    displayInstanceCount = instances > 1 || steps->meta.LPub.page.countInstanceOverride.value() > 1;
                                // lpub->ldrawFile.instances() always return results for CountAtTop - the historic LPub count scheme.
                                // However, the default countInstances configuration is CountAtModel - configurable in Project globals.
                                // lpub->mi.countInstances(...) uses countInstances and opts.renderModelColour to return instances.
                                if (displayInstanceCount) {
                                    // manually override the count instance value using 0 !LPUB SUBMODEL_INSTANCE_COUNT_OVERRIDE
                                    if (steps->groupStepMeta.LPub.page.countInstanceOverride.value())
                                        instances = steps->groupStepMeta.LPub.page.countInstanceOverride.value();
                                    else
                                        instances = lpub->mi.countInstances(&steps->meta, opts.current.modelName, opts.renderModelColour, countInstances);
                                }

                                steps->meta.LPub.subModel.instance.setValue(instances);

                                step->subModel.setSubModel(opts.current.modelName,steps->meta);

                                step->subModel.displayInstanceCount = displayInstanceCount;

                                if (step->subModel.sizeSubModel(&steps->meta,relativeType,pliPerStep) != 0)
                                    emit gui->messageSig(LOG_ERROR, tr("Failed to set first step submodel display for %1...").arg(topOfStep.modelName));
                            }
                            else
                            {
                                step->subModel.clear();
                            } // Place SubModel

                            switch (dividerType) {
                            // for range divider, we set the dividerType for the last STEP of the previous RANGE.
                            case RangeDivider:
                                if (steps && steps->list.size() > 1) {
                                    int i = steps->list.size()-2;           // previous range index
                                    Range *previousRange = dynamic_cast<Range *>(steps->list[i]);
                                    if (previousRange) {
                                        i = previousRange->list.size()-1;   // last step index in previous range
                                        Step *lastStep = dynamic_cast<Step *>(previousRange->list[i]);
                                        if (lastStep)
                                            lastStep->dividerType = dividerType;
                                    }
                                }
                                break;
                                // for steps divider, we set the dividerType for the previous STEP
                            case StepDivider:
                                if (range && range->list.size() > 1) {
                                    int i = range->list.size()-2;            // previous step index
                                    Step *previousStep = dynamic_cast<Step *>(range->list[i]);
                                    if (previousStep)
                                        previousStep->dividerType = dividerType;
                                }
                                break;
                                // no divider
                            default:
                                step->dividerType = dividerType;
                                break;
                            } // divider type

                            step->placeRotateIcon = rotateIcon;

                            if (returnValue != HitNothing)
                                emit gui->messageSig(LOG_ERROR, tr("Create CSI failed to create file."));

                            // BuildMod create and update - performed after createCsi to enable viewerStepKey
                            if (buildModKeys.size()) {
                                if (buildMod.state != BM_END)
                                    emit gui->parseErrorSig(tr("Required meta BUILD_MOD END not found"), opts.current, Preferences::BuildModErrors);
                                const QList keys = buildModKeys.keys();
                                for (int buildModLevel : keys) {
                                    if (buildModInsert)
                                        insertBuildModification(buildModLevel);
                                    else
                                        updateBuildModification(buildModLevel);
                                }
                                buildModKeys.clear();
                                buildModAttributes.clear();
                            }

                            // Set CSI annotations - single step only
                            if (! Gui::exportingObjects() &&  ! multiStep && ! opts.calledOut) {
                                suspendFileDisplay = true;
                                returnValue = static_cast<TraverseRc>(step->setCsiAnnotationMetas(steps->meta));
                                suspendFileDisplay = false;
                                if (Preferences::buildModEnabled && returnValue == HitCsiAnnotation)
                                    return static_cast<int>(returnValue);
                            }

                            if (Render::useLDViewSCall() && ! step->ldrName.isNull()) {
                                opts.ldrStepFiles << step->ldrName;
                                opts.csiKeys << step->csiKey;
                            }

                        } // STEP - Actual parts added, simple, mulitStep or calledOut (no render graphics)

                        else
                     /*
                      * NO STEP - No step object, e.g. BOM, inserted page (no rendered graphics)
                      */
                        {
                            if (pliPerStep) {
                                opts.pliParts.clear();
                                opts.pliPartGroups.clear();
                            }

                            // Only pages or step can have inserts and pointers... not callouts
                            if ( ! multiStep && ! opts.calledOut) {

                                Page *page = dynamic_cast<Page *>(steps);
                                if (page) {
                                    page->inserts            = inserts;
                                    page->pagePointers       = pagePointers;
                                    page->selectedSceneItems = selectedSceneItems;
                                }
                            }
                        } // NO STEP - No step object, e.g. inserted page (no rendered graphics)

                        /*
                      *  STEP - Simple, not mulitStep, not calledOut (render graphics)
                      */
                        if ( ! multiStep && ! opts.calledOut) {

                            PlacementType relativeType = SingleStepType;

                            steps->placement = steps->meta.LPub.assem.placement;

                            int  numSteps = lpub->ldrawFile.numSteps(opts.current.modelName);

                            bool endOfSubmodel =
                                    numSteps == 0 ||
                                    steps->meta.LPub.contStepNumbers.value() ?
                                        steps->meta.LPub.contModelStepNum.value() >= numSteps :
                                        opts.stepNum >= numSteps;

                            // get the number of submodel instances in the model file
                            instances = lpub->ldrawFile.instances(opts.current.modelName, opts.isMirrored);
                            if (!opts.displayModel && countInstances)
                                displayInstanceCount = instances > 1 || steps->meta.LPub.page.countInstanceOverride.value() > 1;
                            // lpub->ldrawFile.instances() always return results for CountAtTop - the historic LPub count scheme.
                            // However, the default countInstances configuration is CountAtModel - configurable in Project globals.
                            // lpub->mi.countInstances(...) uses countInstances and opts.renderModelColour to return instances.
                            if (!opts.displayModel && displayInstanceCount) {
                                // manually override the count instance value using 0 !LPUB SUBMODEL_INSTANCE_COUNT_OVERRIDE
                                if (steps->meta.LPub.page.countInstanceOverride.value())
                                    instances = steps->meta.LPub.page.countInstanceOverride.value();
                                else
                                    instances = lpub->mi.countInstances(&steps->meta, opts.current.modelName, opts.renderModelColour, countInstances);
                            }

                            // update the page if submodel instances greater than 1
                            if (!opts.displayModel && instances > 1) {
                                Page *page = dynamic_cast<Page *>(steps);
                                if (page) {
                                    page->instances            = instances;
                                    page->displayInstanceCount = displayInstanceCount;
                                    page->inserts              = inserts;
                                    page->pagePointers         = pagePointers;
                                    page->selectedSceneItems   = selectedSceneItems;

                                    if (step) {
                                        step->setBottomOfStep(opts.current);
                                        page->displayPage   = step->displayStep;
                                        step->lightList     = lightList;
                                        step->viewerStepKey = QString("%1;%2;%3%4")
                                                .arg(topOfStep.modelIndex)
                                                .arg(topOfStep.lineNumber)
                                                .arg(opts.stepNum)
                                                .arg(lpub->mi.viewerStepKeySuffix(topOfStep, step));
                                    }

                                    if (! steps->meta.LPub.stepPli.perStep.value()) {

                                        QStringList instancesPliParts;
                                        if (opts.pliParts.size() > 0) {
                                            for (int index = 0; index < opts.pliParts.size(); index++) {
                                                QString pliLine = opts.pliParts[index];
                                                for (int i = 0; i < instances; i++) {
                                                    instancesPliParts << pliLine;
                                                }
                                            }
                                        }

                                        if (step) {
                                            // PLI
                                            step->pli.setParts(instancesPliParts,opts.pliPartGroups,steps->meta);
                                            instancesPliParts.clear();
                                            opts.pliParts.clear();
                                            opts.pliPartGroups.clear();

                                            emit gui->messageSig(LOG_INFO, tr("Add PLI images for single-step page..."));

                                            step->pli.sizePli(&steps->meta,relativeType,pliPerStep);

                                            // SM
                                            if (step->placeSubModel) {
                                                emit gui->messageSig(LOG_INFO, tr("Set first step submodel display for %1...").arg(topOfStep.modelName));

                                                steps->meta.LPub.subModel.instance.setValue(instances);
                                                step->subModel.setSubModel(opts.current.modelName,steps->meta);

                                                step->subModel.displayInstanceCount = displayInstanceCount;

                                                if (step->subModel.sizeSubModel(&steps->meta,relativeType,pliPerStep) != 0)
                                                    emit gui->messageSig(LOG_ERROR, tr("Failed to set first step submodel display for %1...").arg(topOfStep.modelName));
                                            }
                                        }
                                    } // Not PLI per step
                                } // Valid page
                            }  // Submodel instances greater than 1

                            emit gui->messageSig(LOG_INFO, tr("Generate CSI image for single-step page..."));

                            if (Render::useLDViewSCall() && opts.ldrStepFiles.size() > 0) {

                                QElapsedTimer timer;
                                timer.start();
                                QString empty("");

                                // LDView renderer parms are added to csiKeys in createCsi call

                                // render the partially assembled model
                                returnValue = static_cast<TraverseRc>(renderer->renderCsi(empty,opts.ldrStepFiles,opts.csiKeys,empty,steps->meta));
                                if (returnValue != HitNothing)
                                    emit gui->messageSig(LOG_ERROR, tr("Render CSI images failed."));

                                emit gui->messageSig(LOG_INFO,
                                                tr("%1 CSI (Single Call) render took "
                                                   "%2 milliseconds to render %3 [Step %4] %5 for %6 "
                                                   "single step on page %7.")
                                                .arg(rendererNames[Render::getRenderer()],
                                                     Gui::elapsedTime(timer.elapsed(),false))
                                                .arg(opts.ldrStepFiles.size())
                                                .arg(opts.stepNum)
                                                .arg(opts.ldrStepFiles.size() == 1 ? tr("image") : tr("images"),
                                                     opts.calledOut ? tr("called out,") : tr("simple,"))
                                                .arg(Gui::stepPageNum));
                            } // useLDViewSCall()

                            // Load the Visual Editor on Step - callouts and multistep Steps are not loaded
                            if (step) {
                                step->setBottomOfStep(opts.current);
                                if (Preferences::modeGUI) {
                                    if (partsAdded && !coverPage) {
                                        lpub->setCurrentStep(step);
                                        if (!Gui::exportingObjects()) {
                                            gui->showLine(topOfStep);
                                            step->loadTheViewer();
                                        }
                                    }
                                }
                            } // Load the Visual Editor on Step

                            // Load the top model into the visual editor on cover page
                            if (coverPage && Preferences::modeGUI && !Gui::exportingObjects()) {
                                if (curMeta.LPub.coverPageViewEnabled.value()) {
                                    bool frontCover = lpub->page.frontCover;
                                    int stepNum = frontCover ? 0 : opts.stepNum;

                                    if (step == nullptr) {
                                        if (range == nullptr) {
                                            range = Gui::newRange(steps,opts.calledOut);
                                        }
                                        step = new Step(topOfStep,
                                                        range,
                                                        stepNum,
                                                        curMeta,
                                                        false /* calledOut */,
                                                        false /* multiStep */);
                                    }

                                    if (step) {
                                        step->setBottomOfStep(opts.current);
                                        const QString cover = frontCover ? tr("front cover") : tr("back cover");
                                        emit gui->messageSig(LOG_INFO_STATUS, tr("Set cover page model preview..."));
                                        emit gui->messageSig(LOG_INFO, tr("Set cover page model preview at %1 for %2, step number %3...")
                                                                          .arg(cover, topOfStep.modelName).arg(stepNum));
                                        const QString fileName = Preferences::preferredRenderer == RENDERER_NATIVE ? SUBMODEL_IMAGE_BASENAME : SUBMODEL_COVER_PAGE_PREVIEW_BASENAME;
                                        step->displayStep = DT_MODEL_COVER_PAGE_PREVIEW;
                                        steps->meta.LPub.subModel.showStepNum.setValue(stepNum);
                                        QFuture<int> future = QtConcurrent::run([&]() {
                                            step->subModel.viewerSubmodel = true;
                                            step->subModel.setSubModel(topOfStep.modelName,steps->meta);
                                            return step->subModel.sizeSubModel(&steps->meta,relativeType,true);
                                        });
                                        if (future.result()) {
                                            emit gui->messageSig(LOG_ERROR, tr("Failed to set cover page model preview at %1 for %2, stepNum %3 (%4.ldr).")
                                                                               .arg(cover, topOfStep.modelName).arg(stepNum).arg(fileName));
                                        } else {
                                            // set the current step - enable access from other parts of the application - e.g. Renderer
                                            lpub->setCurrentStep(step);
                                            if (lpub->currentStep) {
                                                if (step->subModel.viewerSubmodelKey == lpub->currentStep->viewerStepKey) {
                                                    emit gui->showLineSig(topOfStep, LINE_HIGHLIGHT);
                                                    const QString modelFileName = QDir::toNativeSeparators(QString("%1/%2/%3.ldr").arg(QDir::currentPath(), Paths::tmpDir, fileName));
                                                    emit gui->previewModelSig(modelFileName);
                                                } else {
                                                    QString const currentStepKey = lpub->currentStep->viewerStepKey;
                                                    emit gui->messageSig(LOG_WARNING, tr("The specified submodel step key: '%1' does not match the current step key: '%2'")
                                                                                         .arg(step->subModel.viewerSubmodelKey, currentStepKey));
                                                }
                                            }
                                        }
                                    } // step
                                } // cover page view enabled
                            } // cover page

                            if ((returnValue = static_cast<TraverseRc>(Gui::addGraphicsPageItems(steps,coverPage,endOfSubmodel,opts.printing))) != HitAbortProcess)
                                returnValue = HitEndOfPage;

                            if (opts.displayModel) {
                                lpub->meta.rotStep.setValue(saveRotStep);
                            }

                            Gui::stepPageNum += !coverPage;

                            steps->setBottomOfSteps(opts.current);

                            if (!Gui::ContinuousPage())
                                drawPageElapsedTime();

                            if (Gui::abortProcess()) {
                                return HitAbortProcess;
                            }

                            //return static_cast<int>(returnValue);
                        } // STEP - Simple, not mulitStep, not calledOut (draw graphics)
#ifdef WRITE_PARTS_DEBUG
                        writeDrawPartsFile();
#endif
                        if (!Gui::ContinuousPage())
                            drawPageStatus(end);

                        lightList.clear();

                        dividerType = NoDivider;

                        int local = 1; bool reset = true;

                        // reset local fade previous steps - single step
                        if (curMeta.LPub.fadeSteps.enable.pushed == local)
                            curMeta.LPub.fadeSteps.setPreferences(reset);
                        if (curMeta.LPub.assem.fadeSteps.enable.pushed == local)
                            curMeta.LPub.assem.fadeSteps.setPreferences(reset);

                        // reset local highlight current step - single step
                        if (curMeta.LPub.highlightStep.enable.pushed == local)
                            curMeta.LPub.highlightStep.setPreferences(reset);
                        if (curMeta.LPub.assem.highlightStep.enable.pushed == local)
                            curMeta.LPub.assem.highlightStep.setPreferences(reset);

                        // reset local preferred renderer
                        if (curMeta.LPub.preferredRenderer.pushed == local)
                            curMeta.LPub.preferredRenderer.setPreferences(reset);
                        if (curMeta.LPub.assem.preferredRenderer.pushed == local) {
                            curMeta.LPub.assem.preferredRenderer.setPreferences(reset);
                            curMeta.LPub.assem.resetCameraFoV();
                        } else if (curMeta.LPub.subModel.preferredRenderer.pushed == local) {
                            curMeta.LPub.subModel.preferredRenderer.setPreferences(reset);
                            curMeta.LPub.subModel.resetCameraFoV();
                        } else if (curMeta.LPub.pli.preferredRenderer.pushed == local) {
                            curMeta.LPub.pli.preferredRenderer.setPreferences(reset);
                            curMeta.LPub.pli.resetCameraFoV();
                        } else if (curMeta.LPub.bom.preferredRenderer.pushed == local) {
                            curMeta.LPub.bom.preferredRenderer.setPreferences(reset);
                            curMeta.LPub.bom.resetCameraFoV();
                        }

                        // return for STEP - Simple, not mulitStep, not calledOut (draw graphics)
                        if ( ! multiStep && ! opts.calledOut) {
                            return static_cast<int>(returnValue);
                        }

                        // increment continuous step number
                        if (multiStep && steps->groupStepMeta.LPub.contStepNumbers.value())
                            steps->groupStepMeta.LPub.contModelStepNum.setValue(
                                        steps->groupStepMeta.LPub.contModelStepNum.value() + partsAdded);

                        // reset local fade previous steps
                        if (curMeta.LPub.callout.csi.fadeSteps.enable.pushed == local)
                            curMeta.LPub.callout.csi.fadeSteps.setPreferences(reset);
                        else if (curMeta.LPub.multiStep.csi.fadeSteps.enable.pushed == local)
                            curMeta.LPub.multiStep.csi.fadeSteps.setPreferences(reset);

                        // reset local highlight current step
                        if (curMeta.LPub.callout.csi.highlightStep.enable.pushed == local)
                            curMeta.LPub.callout.csi.highlightStep.setPreferences(reset);
                        else if (curMeta.LPub.multiStep.csi.highlightStep.enable.pushed == local)
                            curMeta.LPub.multiStep.csi.highlightStep.setPreferences(reset);

                        // reset local preferred renderer
                        if (curMeta.LPub.callout.csi.preferredRenderer.pushed == local) {
                            curMeta.LPub.callout.csi.preferredRenderer.setPreferences(reset);
                            curMeta.LPub.callout.csi.resetCameraFoV();
                        } else if (curMeta.LPub.multiStep.csi.preferredRenderer.pushed == local) {
                            curMeta.LPub.multiStep.csi.preferredRenderer.setPreferences(reset);
                            curMeta.LPub.multiStep.csi.resetCameraFoV();
                        }

                        steps->meta.pop();
                        curMeta.LPub.buildMod.clear();
                        opts.stepNum  += partsAdded;
                        opts.bfxStore2 = bfxStore1;
                        topOfStep      = opts.current;  // set next step
                        opts.displayModel = false;
                        partsAdded     = false;
                        coverPage      = false;
                        rotateIcon     = false;
                        bfxStore1      = false;
                        bfxLoad        = false;
                        buildModActionStep = false;
                        step           = nullptr;

                    } // STEP - normal case of parts added
                    else
                        /*
                   *  STEP - case no parts added - e.g. model starting with ROTSTEP END
                   */
                    {
                        topOfStep = opts.current;  // set next step
                    }
                } // STEP - case of not NOSTEP and not BuildMod ignore

                if ( ! multiStep) {
                    inserts.clear();
                    pagePointers.clear();
                    selectedSceneItems.clear();
                }

                steps->setBottomOfSteps(opts.current);
                steps->meta.LPub.buildMod.clear();
                buildModTypeIgnore = false;
                buildModActions.clear();
                buildMod.clear();
                noStep = false;
                break;

            case RangeErrorRc:
            {
                gui->showLine(opts.current);
                QString message;
                if (Preferences::preferredRenderer == RENDERER_NATIVE &&
                        line.indexOf("CAMERA_FOV") != -1)
                    message = tr("Native renderer CAMERA_FOV value is out of range [%1:%2]"
                                 "<br>Meta command: %3"
                                 "<br>Valid values: minimum 1.0, maximum 359.0")
                            .arg(opts.current.modelName)
                            .arg(opts.current.lineNumber)
                            .arg(line);
                else
                    message = tr("Parameter(s) out of range: %1:%2<br>Meta command: %3")
                                 .arg(opts.current.modelName)
                                 .arg(opts.current.lineNumber)
                                 .arg(line);

                returnValue = HitRangeError;

                emit gui->messageSig(LOG_ERROR,message);

            }
            default:
                break;
            } // Handle Specific meta-command

        } // STEP - Process meta-command

        // STEP - Process invalid line

        else if (line != "") {
            const QChar type = line.at(0);

            emit gui->parseErrorSig(tr("Invalid LDraw type %1 line. Expected %2 elements, got \"%3\".")
                       .arg(type).arg(type == '1' ? 15 : type == '2' ? 8 : type == '3' ? 11 : 14).arg(line),opts.current);

            returnValue = HitInvalidLDrawLine;
        } // Process invalid line

        /* unset excludedPart */

        excludedPart = false;

    } // for every line

    // if we get here it's likely and empty page or cover page...
    if (!Gui::ContinuousPage())
        drawPageElapsedTime();

    if (Gui::abortProcess()) {
        Gui::revertPageProcess();
        if (returnValue != HitInvalidLDrawLine && returnValue != HitRangeError)
            returnValue = HitAbortProcess;
    }

    return static_cast<int>(returnValue);
}

int Gui::findPage(
        Meta             meta,
        QString const   &addLine,
        FindPageOptions &opts)
{
    bool isPreDisplayPage = true;
    bool isDisplayPage    = false;
    bool isLocalMeta      = false;

    opts.pageDisplayed    = (opts.pageNum > Gui::displayPageNum) && (opts.printing ? Gui::displayPageNum : true);

    Gui::setPageProcessRunning(PROC_FIND_PAGE);

    if (!Gui::ContinuousPage())
        emit gui->messageSig(LOG_STATUS, tr("Processing find page for %1...").arg(opts.current.modelName));

    gui->skipHeader(opts.current);

    opts.stepNumber = 1 + Gui::sa;

    if (opts.pageNum == 1 + Gui::pa) {
/*
#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_NOTICE, tr("FINDPAGE  - Page 000 topOfPage First Page Start   (ops) - LineNumber %1, ModelName %2")
                                            .arg(opts.current.lineNumber, 3, 10, QChar('0')).arg(opts.current.modelName));
#endif
//*/
        Gui::topOfPages.clear();
        Gui::topOfPages.append(opts.current);
        LDrawFile::_currentLevels.clear();
    }

    Rc rc;
    QStringList bfxParts;
    QStringList saveBfxParts;
    QStringList ldrStepFiles;
    QStringList csiKeys;

    int  saveStepNumber = opts.stepNumber;

    QStringList  csiParts;
    QStringList  saveCsiParts;
    QVector<int> lineTypeIndexes;
    QVector<int> saveLineTypeIndexes;
    Where        saveCurrent = opts.current;
    Where        topOfStep = opts.current;
    Where        stepGroupCurrent;

    Gui::saveStepPageNum = Gui::stepPageNum;

    Meta         saveMeta = meta;

    QHash<QString, QStringList>  bfx;
    QHash<QString, QStringList>  saveBfx;
    QHash<QString, QVector<int>> bfxLineTypeIndexes;
    QHash<QString, QVector<int>> saveBfxLineTypeIndexes;
    QList<PliPartGroupMeta>      emptyPartGroups;

    opts.flags.numLines = lpub->ldrawFile.size(opts.current.modelName);

    opts.flags.countInstances = meta.LPub.countInstance.value();

    RotStepMeta saveRotStep = meta.rotStep;

    bool                    buildModItems = false;
    bool                    buildModInsert = false;

    BuildModFlags           buildMod;
    QMap<int, QString>      buildModKeys;
    QMap<int,int>           buildModActions;
    QStringList             buildModCsiParts;
    QMap<int, QVector<int>> buildModAttributes;
    QVector<int>            buildModLineTypeIndexes;

    auto insertAttribute =
            [&buildMod,
            &topOfStep] (
            QMap<int, QVector<int>> &buildModAttributes,
            int index, const Where &here)
    {
        QMap<int, QVector<int>>::iterator i = buildModAttributes.find(buildMod.level);
        if (i == buildModAttributes.end()) {
            QVector<int> modAttributes = { 0, 0, 0, 1, 0, topOfStep.modelIndex, 0, 0 };
            modAttributes[index] = here.lineNumber;
            buildModAttributes.insert(buildMod.level, modAttributes);
        } else {
            i.value()[index] = here.lineNumber;
        }
    };

    auto insertBuildModification =
            [&opts,
             &buildModAttributes,
             &buildModKeys,
             &topOfStep] (int buildModLevel)
    {
        int buildModStepIndex = gui->getBuildModStepIndex(topOfStep);
        QString buildModKey = buildModKeys.value(buildModLevel);
        QVector<int> modAttributes = { 0, 0, 0, 0, 0, -1, 0, 0 };

        QMap<int, QVector<int>>::iterator i = buildModAttributes.find(buildModLevel);
        if (i == buildModAttributes.end()) {
            emit gui->messageSig(LOG_ERROR, tr("Invalid BuildMod Entry for key: %1").arg(buildModKey));
            return;
        }
        modAttributes = i.value();

        modAttributes[BM_DISPLAY_PAGE_NUM] = Gui::displayPageNum;
        modAttributes[BM_STEP_PIECES]      = opts.flags.partsAdded;
        modAttributes[BM_MODEL_NAME_INDEX] = topOfStep.modelIndex;
        modAttributes[BM_MODEL_LINE_NUM]   = topOfStep.lineNumber;
        modAttributes[BM_MODEL_STEP_NUM]   = opts.stepNumber;
/*
#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_DEBUG, QString(
                                 "Insert FindPage BuildMod StepIndex: %1, "
                                 "Action: Apply(64), "
                                 "Attributes: %2 %3 %4 %5 %6 %7 %8 %9, "
                                 "ModKey: %10, "
                                 "Level: %11")
                             .arg(buildModStepIndex)                      // Attribute Default Initial:
                             .arg(modAttributes.at(BM_BEGIN_LINE_NUM))    // 0         0       this
                             .arg(modAttributes.at(BM_ACTION_LINE_NUM))   // 1         0       this
                             .arg(modAttributes.at(BM_END_LINE_NUM))      // 2         0       this
                             .arg(modAttributes.at(BM_DISPLAY_PAGE_NUM))  // 3         0       this
                             .arg(modAttributes.at(BM_STEP_PIECES))       // 4         0       this
                             .arg(modAttributes.at(BM_MODEL_NAME_INDEX))  // 5        -1       this
                             .arg(modAttributes.at(BM_MODEL_LINE_NUM))    // 6         0       this
                             .arg(modAttributes.at(BM_MODEL_STEP_NUM))    // 7         0       this
                             .arg(buildModKey)
                             .arg(buildModLevel));
#endif
//*/
        gui->insertBuildMod(buildModKey,
                            modAttributes,
                            buildModStepIndex);
    };

    PartLineAttributes pla(
                csiParts,
                lineTypeIndexes,
                buildModCsiParts,
                buildModLineTypeIndexes,
                buildMod.level,
                buildMod.ignore,
                buildModItems);

#ifdef WRITE_PARTS_DEBUG
    auto writeFindPartsFile = [this,&topOfStep, &saveCsiParts, &opts](
            const QString &insert = QString(),
            const QStringList &parts = QStringList())
    {
        const QStringList &partList = parts.isEmpty() ? saveCsiParts : parts;
        if (partList.isEmpty())
            return;
        const QString nameInsert = insert.isEmpty() ? "a_find_parts" : insert;
        const QString filePath = QDir::currentPath() + "/" + Paths::tmpDir;
        const QString outfileName = QString("%1/%2_%3.ldr")
                .arg(filePath)
                .arg(insert)
                .arg(QString("page_%1_step_%2_model_%3_line_%4")
                     .arg(opts.pageNum)          // Page Number
                     .arg(opts.stepNumber)       // Step Number
                     .arg(topOfStep.modelIndex)  // ModelIndex
                     .arg(topOfStep.lineNumber));// LineNumber

        QFile file(outfileName);
        if ( ! file.open(QFile::WriteOnly | QFile::Text))
            messageSig(LOG_ERROR,QString("Cannot open find_parts file %1 for writing: %2")
                       .arg(outfileName) .arg(file.errorString()));
        QTextStream out(&file);
        for (int i = 0; i < partList.size(); i++)
            out << partList[i] << lpub_endl;
        file.close();
    };
#endif

    lpub->ldrawFile.setRendered(
                opts.current.modelName,
                opts.renderModelColour,
                opts.renderParentModel,
                opts.isMirrored,
                opts.stepNumber/*opts.groupStepNumber*/,
                opts.flags.countInstances);

  /*
   * For findPage(), the BuildMod behaviour captures the appropriate 'block' of lines
   * to be written to the csiPart list and writes the build mod action setting at each
   * step where the BUILD_MOD APPLY or BUILD_MOD REMOVE action meta command is encountered.
   *
   * The buildModLevel flag is enabled for the lines between BUILD_MOD BEGIN and BUILD_MOD END
   * Lines between BUILD_MOD BEGIN and BUILD_MOD END_MOD are the modified content
   * Lines between BUILD_MOD END_MOD and BUILD_MOD END are the original content
   *
   * When the buildModLevel flag is true (greater than 0):
   * Funct 1 (csiParts):
   * Parse is enabled when 'buildModIgnore' is false.
   * When the build mod action is 'apply', the modified content block is parsed. (buildModIgnore is false)
   * When the build mod action is 'remove', the original content block is parsed.     (buildModIgnore is false)
   * Remove group, partType and partName are only applied when 'buildModIgnore is false.
   *
   * When the build mod meta command is BUILD_MOD END 'buildModIgnore' and 'buildModPliIgnore'
   * are reset to false and buildModLevel is reset to false (0), if the build mod command is not nested
   * in the same
   *
   * When BUILD_MOD APPLY or BUILD_MOD REMOVE action meta command is encountered,
   * the respective build mod action is set or updated in ldrawFiles buildMod. Build
   * action is persisted as a key,value pair whereby the key is the step number and
   * the value is the build action. These updates are performed up to the display page
   * at each execution of findPage().
   *
   * However additional buildMod parsing are performed in the countPages() call. Particularly
   * processing buildMod commands in partially processed submodels (e.g. when a submodel
   * has multiple pages whereby the buildMod command starts in findPage() and ends in countPages().
   * Additionally, when jumping ahead during navigation, buildMod commands are processed in countPages()
   *
   * When the buildModLevel flag is false pli and csi lines are processed normally
   */

    for ( ;
          opts.current.lineNumber < opts.flags.numLines && ! opts.pageDisplayed && ! Gui::abortProcess();
          opts.current.lineNumber++) {

        // if reading include file, return to current line, do not advance

        if (static_cast<Rc>(opts.flags.includeFileRc) != EndOfIncludeFileRc && opts.flags.includeFileFound) {
            opts.current.lineNumber--;
        }

        // scan through the rest of the model counting pages
        // if we've already hit the display page, then do as little as possible

        QString line = lpub->ldrawFile.readLine(opts.current.modelName,opts.current.lineNumber).trimmed();

        if (line.startsWith("0 GHOST ")) {
            line = line.mid(8).trimmed();
        }

        // Set display vars
        isPreDisplayPage  = opts.pageNum < Gui::displayPageNum;
        isDisplayPage     = opts.pageNum == Gui::displayPageNum;

        QStringList tokens, addTokens;

        switch (line.toLatin1()[0]) {
        case '1':
            split(line,tokens);

            // inherit colour number if material colour
            if (tokens.size() > 2) {
                if (tokens[1] == LDRAW_MAIN_MATERIAL_COLOUR)
                    split(addLine, addTokens);
                if (addTokens.size() == 15)
                    tokens[1] = addTokens[1];
                line = tokens.join(" ");
            }

            // process type 1 line...
            if (! opts.flags.partIgnore) {

                // csiParts << line;

                if (Gui::firstStepPageNum == -1) {
                    Gui::firstStepPageNum = opts.pageNum;
                }
                Gui::lastStepPageNum = opts.pageNum;

                QStringList tokens;

                split(line,tokens);

                if (tokens.size() == 15) {

                    QString type = tokens[tokens.size()-1];
                    QString colorType = tokens[1]+type;

                    /* if it is a sub-model (or assembled/rotated callout), then process it.
                   * Called out sub-models (except those assembled/rotated) are processed in Gui::drawPage() */

                    // if submodel
                    if (lpub->ldrawFile.isSubmodel(type)) {

                        // when the display page is not the end of a submodel
                        bool partiallyRendered = false;
                        bool buildModRendered = false;

                        bool validCallout = opts.flags.callout && meta.LPub.callout.begin.value() != CalloutBeginMeta::Unassembled;

                        bool validSubmodel = (!opts.displayModel && !opts.flags.callout) || (validCallout) || (opts.displayModel && validCallout);

                        opts.renderModelColour = tokens[1];

                        // if not callout or assembled/rotated callout
                        if (validSubmodel) {

                            // check if submodel was rendered
                            bool rendered = lpub->ldrawFile.rendered(type,
                                                                     opts.renderModelColour,
                                                                     opts.current.modelName,
                                                                     lpub->ldrawFile.mirrored(tokens),
                                                                     opts.stepNumber,
                                                                     opts.flags.countInstances);

                            // check if submodel is in current step build modification
                            buildModRendered = Preferences::buildModEnabled && (buildMod.ignore || gui->getBuildModRendered(buildMod.key, colorType));

                            // if the submodel was not rendered, and is not in the buffer exchange call setRendered for the submodel.
                            if (! rendered && ! buildModRendered && (! opts.flags.bfxStore2 || ! bfxParts.contains(colorType))) {

                                if (! buildMod.ignore || ! buildModRendered) {

                                    opts.isMirrored = lpub->ldrawFile.mirrored(tokens);

                                    // add submodel to the model stack - it can't be a callout
                                    SubmodelStack tos(opts.current.modelName,opts.current.lineNumber,opts.stepNumber);
                                    meta.submodelStack << tos;
                                    Where current2(type,gui->getSubmodelIndex(type),0);

                                    // add buildMod settings for this step, needed for submodels in a buildMod.
                                    const QString levelKey = QString("FindPage BuildMod Key: %1, ParentModel: %2, LineNumber: %3")
                                            .arg(buildMod.key,
                                                 opts.current.modelName)
                                            .arg(opts.current.lineNumber);
                                    FindPageFlags flags2;
                                    flags2.buildModStack << buildMod;
                                    flags2.buildModLevel = buildMod.state == BM_BEGIN ? getLevel(levelKey, BM_CURRENT) : opts.flags.buildModLevel;

                                    lpub->ldrawFile.setModelStartPageNumber(current2.modelName,opts.pageNum);

                                    // save rotStep, clear it, and restore it afterwards
                                    // since rotsteps don't affect submodels
                                    RotStepMeta saveRotStep2 = meta.rotStep;
                                    meta.rotStep.clear();

                                    // Save the parent (flags.parentStepGroup/flags.parentCallout)
                                    // stepGroup and callout flags. We use the parent flags to
                                    // suppress partsAdded so a new page (number) is not triggered
                                    // for the callout or stepGroup when we are in a partial render
                                    // state, which is to say when the child submoelStack is greater
                                    // than the parent submoelStack. This means we pass both the child
                                    // (flags.stepGroup/flags.callout) and parent stepGroup and callout
                                    // flags to countPage.
                                    bool saveCallout2 = opts.flags.callout;
                                    bool saveStepGroup2 = opts.flags.stepGroup;

                                    // save Default pageSize information
                                    PageSizeData pageSize2;
                                    if (Gui::exporting()) {
                                        pageSize2       = Gui::pageSizes[DEF_SIZE];
                                        opts.flags.pageSizeUpdate  = false;
#ifdef PAGE_SIZE_DEBUG
                                        logDebug() << "SM: Saving    Default Page size info at PageNumber:" << opts.pageNum
                                                   << "W:"    << pageSize2.sizeW << "H:"    << pageSize2.sizeH
                                                   << "O:"    <<(pageSize2.orientation == Portrait ? "Portrait" : "Landscape")
                                                   << "ID:"   << pageSize2.sizeID
                                                   << "Model:" << opts.current.modelName;
#endif
                                    }

                                    // set the step number and parent model where the submodel will be rendered
                                    FindPageOptions modelOpts(
                                                opts.pageNum,
                                                current2,
                                                opts.pageSize,
                                                flags2,
                                                meta.submodelStack,
                                                opts.pageDisplayed,
                                                opts.displayModel,
                                                opts.updateViewer,
                                                opts.isMirrored,
                                                opts.printing,
                                                opts.stepNumber,
                                                opts.contStepNumber,
                                                opts.groupStepNumber,
                                                opts.renderModelColour,
                                                opts.current.modelName /*renderParentModel*/);

                                    const TraverseRc frc = static_cast<TraverseRc>(findPage(meta, line, modelOpts));
                                    if (frc == HitBuildModAction || frc == HitCsiAnnotation || frc == HitAbortProcess) {
                                        // Set processing state and return to parent findPage
                                        Gui::revertPageProcess();
                                        return static_cast<int>(frc);
                                    }

                                    partiallyRendered = modelOpts.current.lineNumber < modelOpts.flags.numLines;
                                    // when we stop before the end of a child submodel,
                                    // capture the child submodel flags for countPages
                                    if (partiallyRendered) {
                                        opts.pageDisplayed     = modelOpts.pageDisplayed;
                                        opts.pageNum           = modelOpts.pageNum;
                                        opts.current           = modelOpts.current;
                                        opts.pageSize          = modelOpts.pageSize;
                                        opts.flags             = modelOpts.flags;
                                        opts.modelStack        = modelOpts.modelStack;
                                        opts.stepNumber        = modelOpts.stepNumber;
                                        opts.renderModelColour = modelOpts.renderModelColour;
                                        opts.renderParentModel = modelOpts.renderParentModel;
                                        // decrement current lineNumber by 1 line to account for
                                        // lineNumber increment as we iterate to terminate the
                                        // line processing loop at 'pageDisplayed'
                                        if (opts.pageDisplayed)
                                            opts.current--;
                                        // if we are at the last step of the submodel, with no parts added, turn
                                        // on parseBuildMods in countPage in case there is a BUILD_MOD REMOVE command.
                                        bool partsAdded;
                                        Where walk = modelOpts.current;
                                        Rc rc = lpub->mi.scanForward(walk,StepMask,partsAdded);
                                        opts.flags.parseBuildMods = (rc == EndOfFileRc && ! partsAdded);
                                        // if no parts added to the last step,
                                        // set partsAdded to -1 so later increment
                                        // will result in a value of 0.
                                        if (opts.flags.parseBuildMods) {
                                            if (! opts.flags.partsAdded)
                                                opts.flags.partsAdded = -1;
                                            opts.flags.buildModLevel = modelOpts.flags.buildModLevel;
                                        }
                                    } else
                                        meta.submodelStack.pop_back();

                                    // capture the final buildMod flags for this page
                                    if (isDisplayPage) {
                                        opts.flags.buildModStack << buildMod;
                                    }

                                    Gui::saveStepPageNum = Gui::stepPageNum;
                                    meta.rotStep  = saveRotStep2;             // restore old rotstep

                                    opts.flags.parentCallout = saveCallout2;  // restore parent called and stepGroup flags
                                    opts.flags.parentStepGroup = saveStepGroup2;

                                    if (opts.contStepNumber) {                // capture continuous step number from exited submodel
                                        opts.contStepNumber = Gui::saveContStepNum;
                                    }

                                    if (opts.groupStepNumber) {               // capture group step number from exited submodel
                                        opts.groupStepNumber = Gui::saveGroupStepNum;
                                    }

                                    if (isPreDisplayPage) {
                                        saveMeta = meta;
                                    }

                                    if (Gui::exporting()) {
                                        Gui::pageSizes.remove(DEF_SIZE);
                                        Gui::pageSizes.insert(DEF_SIZE,pageSize2); // restore old Default pageSize information
#ifdef PAGE_SIZE_DEBUG
                                        logDebug() << "SM: Restoring Default Page size info at PageNumber:" << opts.pageNum
                                                   << "W:"    << Gui::pageSizes[DEF_SIZE].sizeW << "H:"    << Gui::pageSizes[DEF_SIZE].sizeH
                                                   << "O:"    << (Gui::pageSizes[DEF_SIZE].orientation == Portrait ? "Portrait" : "Landscape")
                                                   << "ID:"   << Gui::pageSizes[DEF_SIZE].sizeID
                                                   << "Model:" << opts.current.modelName;
#endif
                                    } // Exporting

                                } // ! BuildModIgnore

                            } // ! Rendered && (! BfxStore2 || ! BfxParts.contains(colorType))

                        } // ! Callout || (Callout && CalloutMode != CalloutBeginMeta::Unassembled)

                        // add submodel to buildMod rendered list
                        if (Preferences::buildModEnabled &&
                                buildMod.state == BM_BEGIN   &&
                                ! partiallyRendered          &&
                                ! buildModRendered) {
                            gui->setBuildModRendered(buildMod.key, colorType);
                        }

                    } // Contains [IsSubmodel]

                    if (opts.flags.bfxStore1) {
                        bfxParts << colorType;
                    }

                } // Type 1 Line

            } // ! PartIgnore
        case '2':
        case '3':
        case '4':
        case '5':
            // if opts.displayModel, we have a custom display
            if (! opts.displayModel && ! buildMod.ignore) {
                ++opts.flags.partsAdded;
                CsiItem::partLine(line,pla,opts.current.lineNumber,OkRc);
            } // ! BuildModIgnore, for each
            break; // case '1' to '5'

        case '0':

            // intercept include file flag

            if (static_cast<Rc>(opts.flags.includeFileRc) != EndOfIncludeFileRc) {
                if (opts.flags.resetIncludeRc) {
                    rc = IncludeRc;                    // return to IncludeRc to parse another line
                } else {
                    rc = static_cast<Rc>(opts.flags.includeFileRc); // execute the Rc returned by include(...)
                    opts.flags.resetIncludeRc = true;  // reset to run include(...) to parse another line
                }
            } else {
                rc = meta.parse(line,opts.current);    // continue
            }

            switch (rc) {
            case StepGroupBeginRc:
                opts.flags.stepGroup = true;
                stepGroupCurrent = topOfStep;
                if (! opts.pageDisplayed) {
                    if (opts.contStepNumber) {    // save starting step group continuous step number to pass to drawPage for submodel preview
                        if (opts.stepNumber == 1 + Gui::sa) {
                            int showStepNum = opts.contStepNumber == 1 + Gui::sa ? opts.stepNumber : opts.contStepNumber;
                            if (opts.pageNum == 1 + Gui::pa) {
                                meta.LPub.subModel.showStepNum.setValue(showStepNum);
                            } else {
                                saveMeta.LPub.subModel.showStepNum.setValue(showStepNum);
                            }
                        }
                    }

                    // New step group page so increment group step number and persist to global
                    // if show step group number and count group steps are enabled
                    if (!meta.LPub.multiStep.pli.perStep.value()) {
                        if (meta.LPub.multiStep.showGroupStepNumber.value()) {
                            if (meta.LPub.multiStep.countGroupSteps.value()) {
                                Where walk(opts.current.modelName);
                                lpub->mi.scanForwardStepGroup(walk);
                                if (opts.current.lineNumber > walk.lineNumber) {
                                    opts.groupStepNumber += 1 + Gui::sa;
                                    Gui::saveGroupStepNum = opts.groupStepNumber;
                                }
                            } else {
                                opts.groupStepNumber = opts.stepNumber;
                                Gui::saveGroupStepNum = opts.groupStepNumber;
                            }
                        }
                    }
                }

                // Steps within step group modify bfxStore2 as they progress
                // so we must save bfxStore2 and use the saved copy when
                // we call drawPage for a step group.
                opts.flags.stepGroupBfxStore2 = opts.flags.bfxStore2;
                break;

            case StepGroupEndRc:
                if (opts.flags.stepGroup && ! opts.flags.noStep2) {
                    opts.flags.stepGroup = false;
                    if (isPreDisplayPage/*opts.pageNum < Gui::displayPageNum*/) {
                        saveLineTypeIndexes    = lineTypeIndexes;
                        saveCsiParts           = csiParts;
                        saveStepNumber         = opts.stepNumber;
                        saveMeta               = meta;
                        saveBfx                = bfx;
                        saveBfxParts           = bfxParts;
                        saveBfxLineTypeIndexes = bfxLineTypeIndexes;
                        saveRotStep            = meta.rotStep;
                        bfxParts.clear();
#ifdef WRITE_PARTS_DEBUG
                        writeFindPartsFile("a_find_csi_parts", csiParts);
#endif
                    } else if (isDisplayPage/*opts.pageNum == Gui::displayPageNum*/) {
                        // ignored when processing a buildModDisplay
                        Gui::savePrevStepPosition = saveCsiParts.size();
                        Gui::stepPageNum = Gui::saveStepPageNum;
                        if (opts.pageNum == 1 + Gui::pa) {
                            lpub->page.meta = meta;
                        } else {
                            lpub->page.meta = saveMeta;
                        }
                        if (opts.contStepNumber) {  // pass continuous step number to drawPage
                            lpub->page.meta.LPub.contModelStepNum.setValue(saveStepNumber);
                            saveStepNumber = Gui::saveContStepNum;
                        }
                        if (opts.groupStepNumber) { // persist step group step number
                            Gui::saveGroupStepNum = opts.groupStepNumber;
                        }
                        lpub->page.meta.pop();
                        lpub->page.meta.rotStep = saveRotStep;

                        QStringList pliParts;
                        DrawPageOptions pageOptions(
                                    stepGroupCurrent,
                                    saveCsiParts,
                                    pliParts,
                                    saveBfxParts,
                                    ldrStepFiles,
                                    csiKeys,
                                    saveBfx,
                                    emptyPartGroups,
                                    saveLineTypeIndexes,
                                    saveBfxLineTypeIndexes,
                                    saveStepNumber,
                                    opts.groupStepNumber,    // pass group step number to drawPage
                                    opts.flags.buildModLevel,
                                    opts.updateViewer,
                                    opts.isMirrored,
                                    opts.printing,
                                    opts.flags.stepGroupBfxStore2,
                                    false /*assembledCallout*/,
                                    false /*calldeOut*/,
                                    opts.displayModel,
                                    opts.renderModelColour,
                                    opts.renderParentModel);
#ifdef WRITE_PARTS_DEBUG
                        writeFindPartsFile("a_find_save_csi_parts");
#endif
                        const TraverseRc drc = static_cast<TraverseRc>(Gui::drawPage(&lpub->page, addLine, pageOptions));
                        if (drc == HitBuildModAction || drc == HitCsiAnnotation || drc == HitAbortProcess) {
                            // Set processing state and return to init drawPage
                            Gui::revertPageProcess();
                            return static_cast<int>(drc);;
                        }

                        lineTypeIndexes.clear();
                        csiParts.clear();

                        saveCurrent.modelName.clear();
                        saveCurrent.modelIndex = -1;
                        saveCsiParts.clear();
                        saveLineTypeIndexes.clear();
                    } // IsDisplayPage /*opts.pageNum == Gui::displayPageNum*/

                    // ignored when processing buildMod display
                    if (Gui::exporting()) {
                        Gui::pageSizes.remove(opts.pageNum);
                        if (opts.flags.pageSizeUpdate) {
                            opts.flags.pageSizeUpdate = false;
                            Gui::pageSizes.insert(opts.pageNum,opts.pageSize);
#ifdef PAGE_SIZE_DEBUG
                            logTrace() << "SG: Inserting New Page size info     at PageNumber:" << opts.pageNum
                                       << "W:"    << opts.pageSize.sizeW << "H:"    << opts.pageSize.sizeH
                                       << "O:"    <<(opts.pageSize.orientation == Portrait ? "Portrait" : "Landscape")
                                       << "ID:"   << opts.pageSize.sizeID
                                       << "Model:" << opts.current.modelName;
#endif
                        } else {
                            Gui::pageSizes.insert(opts.pageNum,Gui::pageSizes[DEF_SIZE]);
#ifdef PAGE_SIZE_DEBUG
                            logTrace() << "SG: Inserting Default Page size info at PageNumber:" << opts.pageNum
                                       << "W:"    << Gui::pageSizes[DEF_SIZE].sizeW << "H:"    << Gui::pageSizes[DEF_SIZE].sizeH
                                       << "O:"    << (Gui::pageSizes[DEF_SIZE].orientation == Portrait ? "Portrait" : "Landscape")
                                       << "ID:"   << Gui::pageSizes[DEF_SIZE].sizeID
                                       << "Model:" << opts.current.modelName;
#endif
                        }
                    } // Exporting
/*
#ifdef QT_DEBUG_MODE
                    emit gui->messageSig(LOG_NOTICE, tr("FINDPAGE  - Page %1 topOfPage StepGroup End      (tos) - LineNumber %2, ModelName %3")
                                    .arg(opts.pageNum, 3, 10, QChar('0')).arg(topOfStep.lineNumber, 3, 10, QChar('0')).arg(topOfStep.modelName));
#endif
//*/
                    ++opts.pageNum;
                    Gui::topOfPages.append(topOfStep/*opts.current*/);  // TopOfSteps(Page) (Next StepGroup), BottomOfSteps(Page) (Current StepGroup)
                    Gui::saveStepPageNum = ++Gui::stepPageNum;

                    opts.flags.noStep2 = false;

                } // StepGroup && ! NoStep2 (StepGroupEndRc)

                if (opts.current.modelName == gui->topLevelFile())
                    opts.pageDisplayed = opts.pageNum > Gui::displayPageNum;

                opts.flags.noStep2 = false;
                opts.displayModel = false;
                break;

            case BuildModApplyRc:
            case BuildModRemoveRc:
                if (Preferences::buildModEnabled) {
                    // special case where we have BUILD_MOD and NOSTEP commands in the same single STEP
                    if (! opts.flags.parseNoStep && ! opts.pageDisplayed && ! opts.flags.stepGroup && opts.flags.noStep)
                        opts.flags.parseNoStep = meta.LPub.parseNoStep.value();
                    Where current = opts.current;
                    if (lpub->mi.scanForwardNoParts(current, StepMask|StepGroupMask) == StepGroupEndRc)
                        emit gui->parseErrorSig(tr("BUILD_MOD %1 '%2' must be placed after MULTI_STEP END")
                                           .arg(rc == BuildModRemoveRc ? QString("REMOVE") : QString("APPLY"),
                                                meta.LPub.buildMod.key()), opts.current,Preferences::ParseErrors,false,false);
                }
                break;

                // Get BuildMod attributes and set ignore based on 'next' step buildModAction
            case BuildModBeginRc:
                if (!Preferences::buildModEnabled) {
                    buildMod.ignore = true;
                    break;
                }
                if (opts.displayModel) {
                    emit gui->parseErrorSig(tr("Build modifications are not supported in display model Step"),
                                            opts.current,Preferences::BuildModErrors,false,false);
                    buildMod.ignore = true;
                    break;
                }
                buildMod.key    = meta.LPub.buildMod.key();
                buildMod.level  = getLevel(buildMod.key, BM_BEGIN);
                buildMod.action = BuildModApplyRc;
                buildModInsert  = ! gui->buildModContains(buildMod.key);
                if (! opts.pageDisplayed) {
                    if (! buildModInsert)
                        buildModActions.insert(buildMod.level,
                                               gui->getBuildModAction(buildMod.key, gui->getBuildModNextStepIndex(), BM_PREVIOUS_ACTION));
                    else
                        buildModActions.insert(buildMod.level, BuildModApplyRc);
                    if (buildModActions.value(buildMod.level) == BuildModApplyRc)
                        buildMod.ignore = false;
                    else if (buildModActions.value(buildMod.level) == BuildModRemoveRc)
                        buildMod.ignore = true;
                }
                // special case where callout or submodel is in a Build Mod
                buildModInsert &= !isPreDisplayPage;
                if (buildModInsert) {
                    buildModKeys.insert(buildMod.level, buildMod.key);
                    insertAttribute(buildModAttributes, BM_BEGIN_LINE_NUM, opts.current);
                }
                buildMod.state = BM_BEGIN;
                break;

                // Set buildModIgnore based on 'next' step buildModAction
            case BuildModEndModRc:
                if (!Preferences::buildModEnabled) {
                    buildMod.ignore = getLevel(QString(), BM_END);
                    break;
                }
                if (! opts.pageDisplayed) {
                    if (buildModActions.value(buildMod.level) == BuildModApplyRc)
                        buildMod.ignore = true;
                    else if (buildModActions.value(buildMod.level) == BuildModRemoveRc)
                        buildMod.ignore = false;
                }
                if (buildModInsert) {
                    if (buildMod.level > 1 && buildMod.key.isEmpty())
                        emit gui->parseErrorSig(tr("Key required for nested build mod meta command"),
                                   opts.current,Preferences::BuildModErrors);
                    if (buildMod.state != BM_BEGIN)
                        emit gui->parseErrorSig(tr("Required meta BUILD_MOD BEGIN not found"),
                                   opts.current, Preferences::BuildModErrors);
                    insertAttribute(buildModAttributes, BM_ACTION_LINE_NUM, opts.current);
                }
                buildMod.state = BM_END_MOD;
                break;

                // Get buildModLevel and reset buildModIgnore to default
            case BuildModEndRc:
                if (!Preferences::buildModEnabled)
                    break;
                if (! opts.pageDisplayed) {
                    buildMod.level = getLevel(QString(), BM_END);
                    if (buildMod.level == opts.flags.buildModLevel)
                        buildMod.ignore = false;
                }
                if (buildModInsert) {
                    if (buildMod.state != BM_END_MOD)
                        emit gui->parseErrorSig(tr("Required meta BUILD_MOD END_MOD not found"),
                                   opts.current, Preferences::BuildModErrors);
                    insertAttribute(buildModAttributes, BM_END_LINE_NUM, opts.current);
                }
                buildMod.state = BM_END;
                break;

            case RotStepRc:
            case StepRc:
                if (opts.flags.partsAdded && (! opts.flags.noStep || opts.flags.parseNoStep)) {
                    if (! buildMod.ignore) {
                        if (opts.contStepNumber) {   // increment continuous step number until we hit the display page
                            if (isPreDisplayPage/*opts.pageNum < Gui::displayPageNum*/ &&
                                    (opts.stepNumber > FIRST_STEP + Gui::sa || Gui::displayPageNum > FIRST_PAGE + Gui::sa)) { // skip the first step
                                opts.contStepNumber += ! opts.flags.coverPage && ! opts.flags.stepPage;
                            }
                            if (! opts.flags.stepGroup && opts.stepNumber == 1 + Gui::sa) {
                                if (opts.pageNum == 1 + Gui::pa && topOfStep.modelName == gui->topLevelFile()) { // when pageNum is 1 and not multistep, persist contStepNumber to 'meta' only if we are in the main model
                                    meta.LPub.subModel.showStepNum.setValue(opts.contStepNumber);
                                } else {
                                    saveMeta.LPub.subModel.showStepNum.setValue(opts.contStepNumber);
                                }
                            }
                        }

                        opts.stepNumber  += ! opts.flags.coverPage && ! opts.flags.stepPage;
                        Gui::stepPageNum += ! opts.flags.coverPage && ! opts.flags.stepGroup;

                        if (isPreDisplayPage/*opts.pageNum < Gui::displayPageNum*/) {
                            if ( ! opts.flags.stepGroup) {
                                saveLineTypeIndexes    = lineTypeIndexes;
                                saveStepNumber         = opts.stepNumber;
                                saveCsiParts           = csiParts;
                                saveMeta               = meta;
                                saveBfx                = bfx;
                                saveBfxParts           = bfxParts;
                                saveBfxLineTypeIndexes = bfxLineTypeIndexes;
                                Gui::saveStepPageNum   = Gui::stepPageNum;
                                // bfxParts.clear();
                                if (opts.groupStepNumber &&
                                        meta.LPub.multiStep.countGroupSteps.value()) { // count group step number and persist
                                    opts.groupStepNumber += ! opts.flags.coverPage && ! opts.flags.stepPage;
                                    Gui::saveGroupStepNum = opts.groupStepNumber;
                                }
#ifdef WRITE_PARTS_DEBUG
                                writeFindPartsFile("a_find_csi_parts", csiParts);
#endif
                            } // ! StepGroup

                            // insert build Mods when processing step group steps after the first step
                            if (opts.flags.stepGroup) {
                                // insert build modifications
                                if (buildModKeys.size()) {
                                    if (buildMod.state != BM_END)
                                        emit gui->parseErrorSig(tr("Required meta BUILD_MOD END not found"),
                                                   opts.current, Preferences::BuildModErrors);
                                    const QList keys = buildModKeys.keys();
                                    for (int buildModLevel : keys) {
                                        insertBuildModification(buildModLevel);
                                    }
                                    buildModKeys.clear();
                                    buildModAttributes.clear();
                                } // BuildModKeys
                            } // StepGroup

                            if (opts.contStepNumber) { // save continuous step number from current model
                                Gui::saveContStepNum = opts.contStepNumber;
                            }
                            saveCurrent = opts.current;
                            saveRotStep = meta.rotStep;
                        } // isPreDisplayPage/*opts.pageNum < Gui::displayPageNum*/

                        if ( ! opts.flags.stepGroup) {
                            if (isDisplayPage) {
                                lineTypeIndexes.clear();
                                csiParts.clear();
                                Gui::savePrevStepPosition = saveCsiParts.size();
                                Gui::stepPageNum = Gui::saveStepPageNum;
                                if (opts.pageNum == 1 + Gui::pa) {
                                    lpub->page.meta = meta;
                                } else {
                                    lpub->page.meta = saveMeta;
                                }
                                if (opts.contStepNumber) { // pass continuous step number to drawPage
                                    lpub->page.meta.LPub.contModelStepNum.setValue(saveStepNumber);
                                    saveStepNumber    = opts.contStepNumber;
                                }
                                if (opts.groupStepNumber) { // persist group step number
                                    Gui::saveGroupStepNum  = opts.groupStepNumber;
                                }
                                lpub->page.meta.pop();
                                lpub->page.meta.LPub.buildMod.clear();
                                lpub->page.meta.rotStep = saveRotStep;
                                lpub->page.meta.rotStep = meta.rotStep;

                                QStringList pliParts;
                                DrawPageOptions pageOptions(
                                            saveCurrent,
                                            saveCsiParts,
                                            pliParts,
                                            saveBfxParts,
                                            ldrStepFiles,
                                            csiKeys,
                                            saveBfx,
                                            emptyPartGroups,
                                            saveLineTypeIndexes,
                                            saveBfxLineTypeIndexes,
                                            saveStepNumber,
                                            opts.groupStepNumber,
                                            opts.flags.buildModLevel,
                                            opts.updateViewer,
                                            opts.isMirrored,
                                            opts.printing,
                                            opts.flags.bfxStore2,
                                            false,false,
                                            opts.displayModel,
                                            opts.renderModelColour,
                                            opts.renderParentModel);
#ifdef WRITE_PARTS_DEBUG
                                writeFindPartsFile("b_find_save_csi_parts");
#endif
                                const TraverseRc drc = static_cast<TraverseRc>(Gui::drawPage(&lpub->page, addLine, pageOptions));
                                if (drc == HitBuildModAction || drc == HitCsiAnnotation || drc == HitAbortProcess) {
                                    // Set processing state and return to init drawPage
                                    Gui::revertPageProcess();
                                    return static_cast<int>(drc);
                                }

                                saveCurrent.modelName.clear();
                                saveCurrent.modelIndex = -1;
                                saveCsiParts.clear();
                                saveLineTypeIndexes.clear();
                                //buildModActions.clear();

                            } // IsDisplayPage /*opts.pageNum == Gui::displayPageNum*/

                            if (! opts.flags.noStep) {
                                if (Gui::exporting() && ! opts.flags.noStep) {
                                    Gui::pageSizes.remove(opts.pageNum);
                                    if (opts.flags.pageSizeUpdate) {
                                        opts.flags.pageSizeUpdate = false;
                                        Gui::pageSizes.insert(opts.pageNum,opts.pageSize);
#ifdef PAGE_SIZE_DEBUG
                                        logTrace() << "ST: Inserting New Page size info     at PageNumber:" << opts.pageNum
                                                   << "W:"    << opts.pageSize.sizeW << "H:"    << opts.pageSize.sizeH
                                                   << "O:"    <<(opts.pageSize.orientation == Portrait ? "Portrait" : "Landscape")
                                                   << "ID:"   << opts.pageSize.sizeID
                                                   << "Model:" << opts.current.modelName;
#endif
                                    } else {
                                        Gui::pageSizes.insert(opts.pageNum,Gui::pageSizes[DEF_SIZE]);
#ifdef PAGE_SIZE_DEBUG
                                        logTrace() << "ST: Inserting Default Page size info at PageNumber:" << opts.pageNum
                                                   << "W:"    << Gui::pageSizes[DEF_SIZE].sizeW << "H:"    << Gui::pageSizes[DEF_SIZE].sizeH
                                                   << "O:"    << (Gui::pageSizes[DEF_SIZE].orientation == Portrait ? "Portrait" : "Landscape")
                                                   << "ID:"   << Gui::pageSizes[DEF_SIZE].sizeID
                                                   << "Model:" << opts.current.modelName;
#endif
                                    }
                                } // Exporting
/*
#ifdef QT_DEBUG_MODE
                                emit gui->messageSig(LOG_NOTICE, QString("FINDPAGE  - Page %1 topOfPage Step, Not Group    (opt) - LineNumber %2, ModelName %3")
                                                .arg(opts.pageNum, 3, 10, QChar('0')).arg(opts.current.lineNumber, 3, 10, QChar('0')).arg(opts.current.modelName));
#endif
//*/
                                ++opts.pageNum;
                                Gui::topOfPages.append(opts.current); // TopOfStep (Next Step), BottomOfStep (Current Step)
                            } // ! opts.flags.noStep && ! StepGroup (StepRc,RotStepRc)

                            // insert build Mods when processing single step
                            if (/*opts.pageDisplayed*/isPreDisplayPage) {
                                // insert build modifications
                                if (buildModKeys.size()) {
                                    if (buildMod.state != BM_END)
                                        emit gui->parseErrorSig(tr("Required meta BUILD_MOD END not found"),
                                                   opts.current, Preferences::BuildModErrors);

                                    const QList keys = buildModKeys.keys();
                                    for (int buildModLevel : keys)
                                        insertBuildModification(buildModLevel);

                                    buildModKeys.clear();
                                    buildModAttributes.clear();
                                } // BuildModKeys
                            } // PageDisplayed
                        } // ! StepGroup

                        topOfStep = opts.current;  // Set next step
                        opts.flags.partsAdded = 0;
                        opts.flags.coverPage  = false;
                        opts.flags.stepPage   = false;
                        opts.flags.bfxStore2  = opts.flags.bfxStore1;
                        opts.flags.bfxStore1  = false;
                        if ( ! opts.flags.bfxStore2) {
                            bfxParts.clear();
                        } // ! BfxStore2
                        meta.pop();
                        lpub->ldrawFile.clearBuildModRendered();
                    } // ! buildMod.ignore
                } // PartsAdded && ! NoStep
                else if ( ! opts.flags.stepGroup)
                {
                    // Adjust current so that draw page doesn't have to deal with
                    // no PartsAdded, NoStep or BuildModIgnore Steps
                    saveCurrent = opts.current;
                } // ! StepGroup

                if (opts.current.modelName == gui->topLevelFile())
                    opts.pageDisplayed = opts.pageNum > Gui::displayPageNum;

                buildMod.clear();
                meta.LPub.buildMod.clear();
                opts.flags.noStep2 = opts.flags.noStep;
                opts.flags.noStep = false;
                opts.flags.parseNoStep = false;
                opts.displayModel = false;
                break;

            case CalloutBeginRc:
                opts.flags.callout = true;
                break;

            case CalloutEndRc:
                opts.flags.callout = false;
                meta.LPub.callout.placement.clear();
                break;

            case InsertCoverPageRc:
                opts.flags.coverPage  = true;
                opts.flags.partsAdded = true;
                break;

            case InsertPageRc:
                opts.flags.stepPage   = true;
                opts.flags.partsAdded = true;
                Gui::lastStepPageNum  = opts.pageNum;
                break;

            case InsertFinalModelRc:
            case InsertDisplayModelRc:
                Gui::lastStepPageNum = opts.pageNum;
                if (rc == InsertDisplayModelRc)
                    opts.displayModel = true;
                break;

            case PartBeginIgnRc:
                opts.flags.partIgnore = true;
                break;

            case PartEndRc:
                opts.flags.partIgnore = false;
                break;

                // Any of the metas that can change csiParts needs
                // to be processed here

            case ClearRc:
                if (isPreDisplayPage/*opts.pageNum < Gui::displayPageNum*/) {
                    csiParts.clear();
                    saveCsiParts.clear();
                    lineTypeIndexes.clear();
                    saveLineTypeIndexes.clear();
                }
                break;

                /* Buffer exchange */
            case BufferStoreRc:
                if (opts.displayModel) {
                    emit gui->parseErrorSig(tr("BUFEXCHG is not supported in display model Step"),
                                            opts.current,Preferences::BuildModErrors,false,false);
                    break;
                }
                if (isPreDisplayPage/*opts.pageNum < Gui::displayPageNum*/) {
                    bfx[meta.bfx.value()] = csiParts;
                    bfxLineTypeIndexes[meta.bfx.value()] = lineTypeIndexes;
                }
                opts.flags.bfxStore1 = true;
                bfxParts.clear();
                break;

            case BufferLoadRc:
                if (isPreDisplayPage/*opts.pageNum < Gui::displayPageNum*/) {
                    csiParts = bfx[meta.bfx.value()];
                    lineTypeIndexes = bfxLineTypeIndexes[meta.bfx.value()];
                }
                // special case where we have BUFEXCHG load and NOSTEP commands in the same single STEP
                if (! opts.flags.parseNoStep && ! opts.pageDisplayed && ! opts.flags.stepGroup && opts.flags.noStep)
                    opts.flags.parseNoStep = meta.LPub.parseNoStep.value();
                opts.flags.partsAdded = true;
                break;

            case PartNameRc:
            case PartTypeRc:
            case MLCadGroupRc:
            case LDCadGroupRc:
            case LeoCadModelRc:
            case LeoCadPieceRc:
            case LeoCadSynthRc:
            case LeoCadGroupBeginRc:
            case LeoCadGroupEndRc:
                if (isPreDisplayPage/*opts.pageNum < Gui::displayPageNum*/) {
                    CsiItem::partLine(line,pla,opts.current.lineNumber,rc);
                }
                opts.flags.partsAdded = true;
                break;

            case LeoCadCameraRc:
            case LeoCadLightRc:
            case LeoCadLightTypeRc:
            case LeoCadLightPOVRayRc:
            case LeoCadLightShadowless:
                isLocalMeta = line.contains(" LOCAL ");
                if ((isDisplayPage && isLocalMeta) || (isPreDisplayPage/*opts.pageNum < Gui::displayPageNum*/ && !isLocalMeta))
                {
                    if (isLocalMeta)
                        line.remove("LOCAL ");
                    CsiItem::partLine(line,pla,opts.current.lineNumber,rc);
                }
                opts.flags.partsAdded = true;
                break;

                /* remove a group or all instances of a part type */
            case RemoveGroupRc:
            case RemovePartTypeRc:
            case RemovePartNameRc:
                if (isPreDisplayPage/*opts.pageNum < Gui::displayPageNum*/) {
                    if (! buildMod.ignore) {
                        QStringList newCSIParts;
                        QVector<int> newLineTypeIndexes;
                        if (rc == RemoveGroupRc) {
                            Gui::remove_group(csiParts,lineTypeIndexes,meta.LPub.remove.group.value(),newCSIParts,newLineTypeIndexes,&meta);
                        } else if (rc == RemovePartTypeRc) {
                            Gui::remove_parttype(csiParts,lineTypeIndexes,meta.LPub.remove.parttype.value(),newCSIParts,newLineTypeIndexes);
                        } else {
                            Gui::remove_partname(csiParts,lineTypeIndexes,meta.LPub.remove.partname.value(),newCSIParts,newLineTypeIndexes);
                        }
                        csiParts = newCSIParts;
                        lineTypeIndexes = newLineTypeIndexes;
                    } // ! buildModIgnore
                }
                break;

            case IncludeRc:
                opts.flags.includeFileRc = include(meta,opts.flags.includeLineNum,opts.flags.includeFileFound);  // includeHere and inserted are include(...) vars
                if (opts.flags.includeFileRc == static_cast<int>(IncludeFileErrorRc)) {
                    opts.flags.includeFileRc = static_cast<int>(EndOfIncludeFileRc);
                    emit gui->parseErrorSig(tr("INCLUDE file was not resolved."),opts.current,Preferences::IncludeFileErrors); // file parse error
                } else if (static_cast<Rc>(opts.flags.includeFileRc) != EndOfIncludeFileRc) { // still reading so continue
                    opts.flags.resetIncludeRc = false;                                        // do not reset, allow includeFileRc to execute
                    continue;
                }
                break;

            case PageSizeRc:
            {
                if (Gui::exporting()) {
                    opts.flags.pageSizeUpdate  = true;

                    opts.pageSize.sizeW  = meta.LPub.page.size.valueInches(0);
                    opts.pageSize.sizeH  = meta.LPub.page.size.valueInches(1);
                    opts.pageSize.sizeID = meta.LPub.page.size.valueSizeID();

                    if (meta.LPub.page.size.valueOrientation() != InvalidOrientation) {
                        opts.pageSize.orientation = meta.LPub.page.size.valueOrientation();
                        meta.LPub.page.orientation.setValue(opts.pageSize.orientation);
                    }

                    Gui::pageSizes.remove(DEF_SIZE);
                    Gui::pageSizes.insert(DEF_SIZE,opts.pageSize);
#ifdef PAGE_SIZE_DEBUG
                    logTrace() << "1. New Page Size entry for Default  at PageNumber:" << opts.pageNum
                               << "W:"  << opts.pageSize.sizeW << "H:"    << opts.pageSize.sizeH
                               << "O:"  << (opts.pageSize.orientation == Portrait ? "Portrait" : "Landscape")
                               << "ID:" << opts.pageSize.sizeID
                               << "Model:" << opts.current.modelName;
#endif
                }
            }
                break;

            case CountInstanceRc:
                if (! opts.pageDisplayed)
                    opts.flags.countInstances = meta.LPub.countInstance.value();
                break;

            case ContStepNumRc:
                if (isPreDisplayPage/*opts.pageNum < Gui::displayPageNum*/)
                {
                    if (meta.LPub.contStepNumbers.value()) {
                        if (! opts.contStepNumber)
                            opts.contStepNumber += 1 + Gui::sa;
                    } else {
                        opts.contStepNumber = 0;
                    }
                }
                break;

            case StartStepNumberRc:
                if (isPreDisplayPage/*opts.pageNum < Gui::displayPageNum*/)
                {
                    if ((opts.current.modelName == lpub->ldrawFile.topLevelFile() && opts.flags.partsAdded) ||
                         opts.current.modelName != lpub->ldrawFile.topLevelFile())
                        emit gui->parseErrorSig(tr("Start step number must be specified in the top model header."), opts.current);
                    sa = meta.LPub.startStepNumber.value() - 1;
                }
                break;

            case StartPageNumberRc:
                if (isPreDisplayPage/*opts.pageNum < Gui::displayPageNum*/)
                {
                    if ((opts.current.modelName == lpub->ldrawFile.topLevelFile() && opts.flags.partsAdded) ||
                         opts.current.modelName != lpub->ldrawFile.topLevelFile())
                        emit gui->parseErrorSig(tr("Start page number must be specified in the top model header."), opts.current);
                    Gui::pa = meta.LPub.startPageNumber.value() - 1;
                }
                break;

            case BuildModEnableRc:
                if (isPreDisplayPage/*opts.pageNum < Gui::displayPageNum*/)
                {
                    bool enabled = meta.LPub.buildModEnabled.value() && !opts.displayModel;
                    if (Preferences::buildModEnabled != enabled) {
                        Preferences::buildModEnabled  = enabled;
                        gui->enableVisualBuildModification();
                        emit gui->messageSig(LOG_INFO, tr("Build Modifications are %1")
                                                          .arg(enabled ? tr("Enabled") : tr("Disabled")));
                    }
                }
                break;

            case FinalModelEnableRc:
                if (isPreDisplayPage/*opts.pageNum < Gui::displayPageNum*/)
                {
                    bool enabled = meta.LPub.finalModelEnabled.value();
                    if (Preferences::finalModelEnabled != enabled) {
                        Preferences::finalModelEnabled  = enabled;
                        gui->enableVisualBuildModification();
                        emit gui->messageSig(LOG_INFO, tr("Fade/Highlight final model step is %1")
                                                          .arg(enabled ? tr("Enabled") : tr("Disabled")));
                    }
                }
                break;

            case PageOrientationRc:
            {
                if (Gui::exporting()) {
                    opts.flags.pageSizeUpdate = true;

                    if (opts.pageSize.sizeW == 0.0f)
                        opts.pageSize.sizeW    = Gui::pageSizes[DEF_SIZE].sizeW;
                    if (opts.pageSize.sizeH == 0.0f)
                        opts.pageSize.sizeH    = Gui::pageSizes[DEF_SIZE].sizeH;
                    if (opts.pageSize.sizeID.isEmpty())
                        opts.pageSize.sizeID   = Gui::pageSizes[DEF_SIZE].sizeID;
                    opts.pageSize.orientation= meta.LPub.page.orientation.value();

                    Gui::pageSizes.remove(DEF_SIZE);
                    Gui::pageSizes.insert(DEF_SIZE,opts.pageSize);
#ifdef PAGE_SIZE_DEBUG
                    logTrace() << "1. New Orientation entry for Default at PageNumber:" << opts.pageNum
                               << "W:"  << opts.pageSize.sizeW << "H:"    << opts.pageSize.sizeH
                               << "O:"  << (opts.pageSize.orientation == Portrait ? "Portrait" : "Landscape")
                               << "ID:" << opts.pageSize.sizeID
                               << "Model:" << opts.current.modelName;
#endif
                }
            }
                break;

            case NoStepRc:
                opts.flags.noStep = true;
                break;
            default:
                break;
            } // Switch Rc
            break; // case '0'

          default:
            break;
        } // Switch First Character of Line
    } // For Every Line

    csiParts.clear();
    lineTypeIndexes.clear();

    // last step in submodel
    if (! Gui::abortProcess() &&
            opts.flags.partsAdded &&
            ! opts.pageDisplayed &&
            (! opts.flags.noStep || opts.flags.parseNoStep)) {
        isPreDisplayPage = opts.pageNum < Gui::displayPageNum;
        isDisplayPage = opts.pageNum == Gui::displayPageNum;
        // increment continuous step number
        // save continuous step number from current model
        // pass continuous step number to drawPage
        if (opts.contStepNumber) {
            if (isPreDisplayPage/*opts.pageNum < Gui::displayPageNum*/ && ! opts.flags.countInstances &&
                    (opts.stepNumber > FIRST_STEP + sa || Gui::displayPageNum > FIRST_PAGE + sa)) {
                opts.contStepNumber += ! opts.flags.coverPage && ! opts.flags.stepPage;
            }
            if (isDisplayPage/*opts.pageNum == Gui::displayPageNum*/) {
                saveMeta.LPub.contModelStepNum.setValue(saveStepNumber);
                saveStepNumber = opts.contStepNumber;
            }
            Gui::saveContStepNum = opts.contStepNumber;
        }
        if (isPreDisplayPage && opts.groupStepNumber &&
                meta.LPub.multiStep.countGroupSteps.value()) { // count group step number and persist
            opts.contStepNumber += ! opts.flags.coverPage && ! opts.flags.stepPage;
            Gui::saveGroupStepNum = opts.contStepNumber;
        }

        if (isDisplayPage/*opts.pageNum == Gui::displayPageNum*/) {
            if (opts.groupStepNumber)                   // pass group step number to drawPage
                saveStepNumber = Gui::saveGroupStepNum;

            Gui::savePrevStepPosition = saveCsiParts.size();
            lpub->page.meta = saveMeta;
            QStringList pliParts;
            DrawPageOptions pageOptions(
                        saveCurrent,
                        saveCsiParts,
                        pliParts,
                        saveBfxParts,
                        ldrStepFiles,
                        csiKeys,
                        saveBfx,
                        emptyPartGroups,
                        saveLineTypeIndexes,
                        saveBfxLineTypeIndexes,
                        saveStepNumber,
                        opts.groupStepNumber,
                        opts.flags.buildModLevel,
                        opts.updateViewer,
                        opts.isMirrored,
                        opts.printing,
                        opts.flags.bfxStore2,
                        false,false,
                        opts.displayModel,
                        opts.renderModelColour,
                        opts.renderParentModel);
#ifdef WRITE_PARTS_DEBUG
            writeFindPartsFile("b_find_save_csi_parts");
#endif
            const TraverseRc drc = static_cast<TraverseRc>(Gui::drawPage(&lpub->page, addLine, pageOptions));
            if (drc == HitBuildModAction || drc == HitCsiAnnotation || drc == HitAbortProcess) {
                // Set processing state and return to init drawPage
                Gui::revertPageProcess();
                return static_cast<int>(drc);
            }

        } // IsDisplayPage

        if (! opts.flags.noStep) {
            if (Gui::exporting()) {
                Gui::pageSizes.remove(opts.pageNum);
                if (opts.flags.pageSizeUpdate) {
                    Gui::pageSizes.insert(opts.pageNum,opts.pageSize);
#ifdef PAGE_SIZE_DEBUG
                    logTrace() << "PG: Inserting New Page size info     at PageNumber:" << opts.pageNum
                               << "W:"    << opts.pageSize.sizeW << "H:"    << opts.pageSize.sizeH
                               << "O:"    <<(opts.pageSize.orientation == Portrait ? "Portrait" : "Landscape")
                               << "ID:"   << opts.pageSize.sizeID
                               << "Model:" << opts.current.modelName;
#endif
                } else {
                    Gui::pageSizes.insert(opts.pageNum,Gui::pageSizes[DEF_SIZE]);
#ifdef PAGE_SIZE_DEBUG
                    logTrace() << "PG: Inserting Default Page size info at PageNumber:" << opts.pageNum
                               << "W:"    << Gui::pageSizes[DEF_SIZE].sizeW << "H:"    << Gui::pageSizes[DEF_SIZE].sizeH
                               << "O:"    << (Gui::pageSizes[DEF_SIZE].orientation == Portrait ? "Portrait" : "Landscape")
                               << "ID:"   << Gui::pageSizes[DEF_SIZE].sizeID
                               << "Model:" << opts.current.modelName;
#endif
                }
            } // Exporting
/*
#ifdef QT_DEBUG_MODE
            emit gui->messageSig(LOG_NOTICE, QString("FINDPAGE  - Page %1 topOfPage Step, Submodel End (opt) - LineNumber %2, ModelName %3")
                            .arg(opts.pageNum, 3, 10, QChar('0')).arg(opts.current.lineNumber, 3, 10, QChar('0')).arg(opts.current.modelName));
#endif
//*/
            ++opts.pageNum;
            ++Gui::stepPageNum;
            Gui::topOfPages.append(opts.current); // TopOfStep (Next Step), BottomOfStep (Current/Last Step)

            if (opts.current.modelName == gui->topLevelFile()) {
                if (! opts.pageDisplayed)
                    opts.pageDisplayed = opts.pageNum > Gui::displayPageNum;
            }
        } // ! opts.flags.noStep (last step in submodel)

        // Clear parts added so we dont count again in countPage;
        opts.flags.partsAdded = 0;
        opts.flags.parseNoStep = false;
        opts.displayModel = false;

    }  // Last Step in Submodel

    // Set processing state
    Gui::revertPageProcess();
    return Gui::abortProcess() ? static_cast<int>(HitAbortProcess) : static_cast<int>(HitNothing);
}

int Gui::getBOMParts(
          Where   current,
    const QString &addLine)
{
  bool partIgnore   = false;
  bool pliIgnore    = false;
  bool synthBegin   = false;
  bool bfxStore1    = false;
  bool bfxStore2    = false;
  bool bfxLoad      = false;
  bool partsAdded   = false;
  bool excludedPart = false;
  bool displayModel = false;

  bool buildModIgnore = false;

  QStringList bfxParts;

  Meta meta;

  gui->skipHeader(current);

  int numLines = lpub->ldrawFile.size(current.modelName);

  Rc rc;

  for ( ;
        current.lineNumber < numLines;
        current.lineNumber++) {

      // scan through the rest of the model counting pages
      // if we've already hit the display page, then do as little as possible
      QStringList token,addToken;
      QString type;
      QString line = lpub->ldrawFile.readLine(current.modelName,current.lineNumber).trimmed();

      if (line.startsWith("0 GHOST ")) {
          line = line.mid(8).trimmed();
        }

      switch (line.toLatin1()[0]) {
        case '1':

          if (! displayModel && ! partIgnore && ! pliIgnore && ! buildModIgnore && ! synthBegin) {

              split(line,token);

              type = token[token.size()-1];

              /* check if part is in excludedPart.lst*/
              excludedPart = ExcludedParts::isExcludedPart(type);

          } else {
              break;
          }

          if ( ! excludedPart) {

              if (token[1] == LDRAW_MAIN_MATERIAL_COLOUR) {
                  split(addLine,addToken);
                  if (addToken.size() == 15) {
                      token[1] = addToken[1];
                      line = token.join(" ");
                    }
                }

          /*
           * Automatically ignore parts added twice due to buffer exchange
           */
              bool removed = false;
              QString colorPart = QString("%1%2%3").arg(current.lineNumber).arg(token[1], type);

              if (bfxStore2 && bfxLoad) {
                  int i;
                  for (i = 0; i < bfxParts.size(); i++) {
                      if (bfxParts[i] == colorPart) {
                          emit gui->messageSig(LOG_NOTICE, tr("Duplicate PliPart at line [%1] removed [%2].")
                                                              .arg(current.lineNumber).arg(line));
                          bfxParts.removeAt(i);
                          removed = true;
                          break;
                        }
                    }
                }

              if ( ! removed) {

                  if (lpub->ldrawFile.isSubmodel(type)) {

                      if (!lpub->ldrawFile.isDisplayModel(type)) {
                          Where current2(type,0);
                          Gui::getBOMParts(current2,line);
                      }

                    } else {

                      /*  check if alternative part exist and replace */
                      if(PliSubstituteParts::hasSubstitutePart(type)) {

                          QStringList substituteToken;
                          split(line,substituteToken);
                          QString substitutePart = type;

                          if (PliSubstituteParts::getSubstitutePart(substitutePart)) {
                              substituteToken[substituteToken.size()-1] = substitutePart;
                            }
                          line = substituteToken.join(" ");
                        }

                      QString newLine = Pli::partLine(line,current,meta);

                      Gui::bomParts << newLine;
                    }
                }

              if (bfxStore1) {
                  bfxParts << colorPart;
                }

              partsAdded = true;
            }
          break;
        case '0':
          rc = meta.parse(line,current);

          /* substitute part/parts with this */

          switch (rc) {
            case PliBeginSub1Rc:
            case PliBeginSub2Rc:
            case PliBeginSub3Rc:
            case PliBeginSub4Rc:
            case PliBeginSub5Rc:
            case PliBeginSub6Rc:
            case PliBeginSub7Rc:
            case PliBeginSub8Rc:
              if (! displayModel   &&
                  ! pliIgnore      &&
                  ! partIgnore     &&
                  ! buildModIgnore &&
                  ! synthBegin) {
                  QString addPart = QString("1 %1 0 0 0 1 0 0 0 1 0 0 0 1 %2")
                                            .arg(meta.LPub.pli.begin.sub.value().color,
                                                 meta.LPub.pli.begin.sub.value().part);
                  Gui::bomParts << Pli::partLine(addPart,current,meta);
                  pliIgnore = true;
                }
              break;

            case InsertFinalModelRc:
            case InsertDisplayModelRc:
              displayModel = true;
              break;

            case PliBeginIgnRc:
              pliIgnore = true;
              break;

            case PliEndRc:
              pliIgnore = false;
              meta.LPub.pli.begin.sub.clearAttributes();
              break;

            case PartBeginIgnRc:
              partIgnore = true;
              break;

            case PartEndRc:
              partIgnore = false;
              break;

            case SynthBeginRc:
              synthBegin = true;
              break;

            case SynthEndRc:
              synthBegin = false;
              break;

              /* Buffer exchange */
            case BufferStoreRc:
              bfxStore1 = true;
              bfxParts.clear();
              break;

            case BufferLoadRc:
              bfxLoad = true;
              break;

            case BomPartGroupRc:
              meta.LPub.bom.pliPartGroup.setWhere(current);
              meta.LPub.bom.pliPartGroup.setBomPart(true);
              Gui::bomPartGroups.append(meta.LPub.bom.pliPartGroup);
              break;

              // Any of the metas that can change pliParts needs
              // to be processed here

            case ClearRc:
              Gui::bomParts.empty();
              break;

              /*
               * For Gui::getBOMParts(), the BuildMod behaviour only processes
               * top level mods for pliParts, nested mods are ignored.
               * The aim is to process original pli parts and ignore those
               * that are in the build modification block.
               *
               * The buildModLevel flag is enabled (greater than or equal to 1) for the lines between
               * BUILD_MOD BEGIN and BUILD_MOD END
               * Lines between BUILD_MOD BEGIN and BUILD_MOD END_MOD represent
               * the modified content
               * Lines between BUILD_MOD END_MOD and BUILD_MOD END
               * represent the original content
               *
               * When processing a modification block, we end at the build
               * mod action meta command 'BuildModEndModRc'
               * to include the original PLI parts as they are not added
               * in PLI::partLine as they are for CSI parts.
               *
               * BUILD_MOD APPLY or BUILD_MOD REMOVE action meta commands
               * are ignored
               *
               */

            case BuildModBeginRc:
              buildModIgnore = true;
              break;

            case BuildModEndModRc:
              buildModIgnore = getLevel(QString(), BM_END);
              break;

            case PartNameRc:
            case PartTypeRc:
            case MLCadGroupRc:
            case LDCadGroupRc:
            case LeoCadGroupBeginRc:
            case LeoCadGroupEndRc:
              if (! displayModel)
                  Gui::bomParts << Pli::partLine(line,current,meta);
              break;

              /* remove a group or all instances of a part type */
            case RemoveGroupRc:
            case RemovePartTypeRc:
            case RemovePartNameRc:
              if (! displayModel && ! buildModIgnore) {
                  QStringList newBOMParts;
                  QVector<int> dummy;
                  if (rc == RemoveGroupRc) {
                      Gui::remove_group(Gui::bomParts,dummy,meta.LPub.remove.group.value(),newBOMParts,dummy,&meta);
                  } else if (rc == RemovePartTypeRc) {
                      Gui::remove_parttype(Gui::bomParts,dummy,meta.LPub.remove.parttype.value(),newBOMParts,dummy);
                  } else {
                      Gui::remove_partname(Gui::bomParts,dummy,meta.LPub.remove.partname.value(),newBOMParts,dummy);
                  }
                  Gui::bomParts = newBOMParts;
              }
              break;

            case EndOfFileRc:
            case RotStepRc:
            case StepRc:
              if (partsAdded) {
                  bfxStore2 = bfxStore1;
                  bfxStore1 = false;
                  bfxLoad = false;
                  if ( ! bfxStore2) {
                      bfxParts.clear();
                    }
                }
              partsAdded = false;
              displayModel = false;
              break;

            default:
              break;
            } // switch
          break;
        }
    } // for every line
  return 0;
}

int Gui::getBOMOccurrence(Where current) {      // start at top of ldrawFile

  // traverse content to find the number and location of BOM pages
  // key=modelName_LineNumber, value=occurrence
  QHash<QString, int> bom_Occurrence;

  gui->skipHeader(current);

  int numLines        = lpub->ldrawFile.size(current.modelName);
  int occurrenceNum   = 0;
  Gui::boms           = 0;
  Gui::bomOccurrence  = 0;
  Rc rc;
  Meta meta;

  for ( ; current.lineNumber < numLines;
        current.lineNumber++) {

      QString line = lpub->ldrawFile.readLine(current.modelName,current.lineNumber).trimmed();
      switch (line.toLatin1()[0]) {
          case '1':
          {
              QStringList token;
              split(line,token);
              QString type = token[token.size()-1];

              if (lpub->ldrawFile.isSubmodel(type)) {
                  Where current2(type,0);
                  Gui::getBOMOccurrence(current2);
              }
              break;
          }
          case '0':
          {
              rc = meta.parse(line,current);
              switch (rc) {
                  case InsertRc:
                  {
                      InsertData insertData = meta.LPub.insert.value();
                      if (insertData.type == InsertData::InsertBom) {

                          QString uniqueID = QString("%1_%2").arg(current.modelName).arg(current.lineNumber);
                          occurrenceNum++;
                          bom_Occurrence[uniqueID] = occurrenceNum;
                      }
                  }
                      break;
                  default:
                      break;
              } // switch metas
              break;
          }  // switch line type
      }
  } // for every line

  if (occurrenceNum > 1) {
      // now set the bom occurrance based on our current position
      Where here = Gui::topOfPages[Gui::displayPageNum-1];
      for (++here; here.lineNumber < lpub->ldrawFile.size(here.modelName); here++) {
          QString line = gui->readLine(here);
          rc = meta.parse(line,here);
          if (rc == InsertRc) {
              InsertData insertData = meta.LPub.insert.value();
              if (insertData.type == InsertData::InsertBom) {
                  QString bomID   = QString("%1_%2").arg(here.modelName).arg(here.lineNumber);
                  Gui::bomOccurrence = bom_Occurrence[bomID];
                  Gui::boms          = bom_Occurrence.size();
                  break;
              }
          }
      }
  }
  return 0;
}

bool Gui::generateBOMPartsFile(const QString &bomFileName) {
    Where current(lpub->ldrawFile.topLevelFile(),0);
    QFuture<void> future = QtConcurrent::run([current]() {
        Gui::bomParts.clear();
        Gui::bomPartGroups.clear();
        Gui::getBOMParts(current, QString());
    });
    future.waitForFinished();
    if (! Gui::bomParts.size()) {
        emit lpub->messageSig(LOG_ERROR, tr("No BOM parts were detected."));
        return false;
    }

    QStringList tempParts = Gui::bomParts;

    Gui::bomParts.clear();

    for (QString &bomPartsString : tempParts) {
        if (bomPartsString.startsWith("1")) {
            QStringList partComponents = bomPartsString.split(";");
            Gui::bomParts << partComponents.at(0);
            emit lpub->messageSig(LOG_DEBUG, tr("%1 added to export list.").arg(partComponents.at(0)));
        }
    }
    emit gui->messageSig(LOG_INFO, tr("%1 BOM parts processed.").arg(Gui::bomParts.size()));

    // create a BOM parts file
    QFile bomFile(bomFileName);
    if ( ! bomFile.open(QIODevice::WriteOnly)) {
        emit lpub->messageSig(LOG_ERROR, tr("Cannot open BOM parts file for writing: %1, %2.")
                              .arg(bomFileName, bomFile.errorString()));
        return false;
    }

    QTextStream out(&bomFile);
    out << QString("0 Name: %1").arg(QFileInfo(bomFileName).fileName()) << lpub_endl;
    for (QString &bomPart : Gui::bomParts)
        out << bomPart << lpub_endl;
    bomFile.close();
    return true;
}

void Gui::attitudeAdjustment()
{
  bool callout = false;
  int numFiles = lpub->ldrawFile.subFileOrder().size();

  for (int i = 0; i < numFiles; i++) {
      QString fileName = lpub->ldrawFile.subFileOrder()[i];

      if (lpub->ldrawFile.isUnofficialPart(fileName))
          continue;

      int numLines     = lpub->ldrawFile.size(fileName);

      QStringList pending;

      for (Where current(fileName,0);
           current.lineNumber < numLines;
           current.lineNumber++) {

          QString line = lpub->ldrawFile.readLine(current.modelName,current.lineNumber);
          QStringList argv;
          split(line,argv);

          if (argv.size() >= 4 &&
              argv[0] == "0" &&
              (argv[1] == "!LPUB" || argv[1] == "LPUB") &&
              argv[2] == "CALLOUT") {
              if (argv.size() == 4 && argv[3] == "BEGIN") {
                  callout = true;
                  pending.clear();
                } else if (argv[3] == "END") {
                  callout = false;
                  for (int i = 0; i < pending.size(); i++) {
                      lpub->ldrawFile.insertLine(current.modelName,current.lineNumber, pending[i]);
                      ++numLines;
                      ++current;
                    }
                  pending.clear();
                } else if (argv[3] == "ALLOC" ||
                           argv[3] == "BACKGROUND" ||
                           argv[3] == "BORDER" ||
                           argv[3] == "MARGINS" ||
                           argv[3] == "PLACEMENT") {
                  if (callout && argv.size() >= 5 && argv[4] != "GLOBAL") {
                      lpub->ldrawFile.deleteLine(current.modelName,current.lineNumber);
                      pending << line;
                      --numLines;
                      --current;
                    }
                }
            }
        }
    }
}


void Gui::countPages()
{
  if (Gui::maxPages < 1 + Gui::pa || Gui::buildModJumpForward) {
      Gui::setPageProcessRunning(PROC_COUNT_PAGE);
      Meta meta;
      QString empty;
      FindPageFlags fpFlags;
      PageSizeData emptyPageSize;
      QList<SubmodelStack> modelStack;

      current                 =  Where(lpub->ldrawFile.topLevelFile(),0,0);
      Gui::saveDisplayPageNum =  Gui::displayPageNum;
      Gui::displayPageNum     =  1 << 24; // really large number: 16777216
      Gui::firstStepPageNum   = -1;       // for front cover page
      Gui::lastStepPageNum    = -1;       // for back cover page
      Gui::maxPages           =  1 + Gui::pa;
      Gui::stepPageNum        =  1 + Gui::pa;

      skipHeader(current);

      // set model start page - used to enable mpd combo to jump to start page
      lpub->ldrawFile.setModelStartPageNumber(current.modelName,Gui::maxPages);

      QString message = tr("Counting pages...");
      if (Gui::buildModJumpForward) {
          fpFlags.parseBuildMods = true;
          message = tr("BuildMod Next parsing from countPage for jump to page %1...").arg(Gui::saveDisplayPageNum);
      }

      emit gui->messageSig(LOG_TRACE, message);

      FindPageOptions opts(
                  Gui::maxPages,      /*pageNum*/
                  current,
                  emptyPageSize,
                  fpFlags,
                  modelStack,
                  false          /*pageDisplayed*/,
                  false          /*displayModel*/,
                  false          /*updateViewer*/,
                  false          /*mirrored*/,
                  false          /*printing*/,
                  0              /*stepNumber*/,
                  0              /*contStepNumber*/,
                  0              /*groupStepNumber*/,
                  LDRAW_MAIN_MATERIAL_COLOUR /*renderModelColour*/,
                  "model~origin" /*renderParentModel*/);

      LDrawFile::_currentLevels.clear();

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
      QFuture<int> future = QtConcurrent::run(CountPageWorker::countPage, &meta, &lpub->ldrawFile, std::ref(opts), empty);
#else
      QFuture<int> future = QtConcurrent::run(CountPageWorker::countPage, &meta, &lpub->ldrawFile, opts, empty);
#endif
      future.waitForFinished();

      pagesCounted();
   }
}

void Gui::drawPage(DrawPageFlags &dpFlags)
{
    Gui::setPageProcessRunning(PROC_DISPLAY_PAGE);

    QApplication::setOverrideCursor(Qt::WaitCursor);

    if (Preferences::modeGUI && ! Gui::exporting() && ! Gui::ContinuousPage())
        gui->enableNavigationActions(false);

    gui->current      = Where(lpub->ldrawFile.topLevelFile(),0,0);
    Gui::saveMaxPages = Gui::maxPages;
    Gui::maxPages     = 1 + Gui::pa;
    Gui::stepPageNum  = Gui::maxPages;
    lpub->page.relativeType = SingleStepType;

    // set submodels unrendered
    lpub->ldrawFile.unrendered();

    // if not buildMod action
    if (!dpFlags.buildModActionChange && !dpFlags.csiAnnotation) {
        int displayPageIndx  = -1;
        int nextStepIndex    = -1;
        bool firstPage       = true;
        bool adjustTopOfStep = false;
        Where topOfStep      = gui->current;
        lpub->meta           = Meta();
/*
#ifdef QT_DEBUG_MODE
    emit gui->messageSig(LOG_NOTICE, "---------------------------------------------------------------------------");
    emit gui->messageSig(LOG_NOTICE, QString("BEGIN    -  Page %1").arg(Gui::displayPageNum));
#endif
//*/
        // set next step index and test index is display page index - i.e. refresh a page
        if (Preferences::buildModEnabled) {
            displayPageIndx = Gui::exporting() ? Gui::displayPageNum : Gui::displayPageNum - 1;
            firstPage       = ! Gui::topOfPages.size() || Gui::topOfPages.size() < displayPageIndx;

            if (!firstPage) {
                if (Gui::topOfPages.size() > displayPageIndx) {
                    topOfStep     = Gui::topOfPages[displayPageIndx];
                } else {
                    topOfStep     = Gui::topOfPages.takeLast();
                }
            }

            if (!topOfStep.lineNumber)
                gui->skipHeader(topOfStep);

            nextStepIndex = gui->getStepIndex(topOfStep);

            gui->setBuildModNextStepIndex(topOfStep);

            if (!firstPage)
                adjustTopOfStep  = nextStepIndex == gui->getBuildModNextStepIndex();

            lpub->ldrawFile.clearBuildModRendered();

            if (adjustTopOfStep)
                if (! gui->getBuildModStepIndexWhere(nextStepIndex, topOfStep))
                    topOfStep = firstPage ? gui->current : Gui::topOfPages[displayPageIndx];

            QFuture<int> future = QtConcurrent::run([&](){ return gui->setBuildModForNextStep(topOfStep);});

            // if page direction is jump forward, reset vars that may be updated by the count page call
            if ((Gui::buildModJumpForward = Gui::pageDirection == PAGE_JUMP_FORWARD)) {
                const int saveJumpDisplayPageNum = Gui::displayPageNum;
                const QList<Where> saveJumpTopOfPages = Gui::topOfPages;
                const Where saveJumpCurrent = gui->current;
                if (static_cast<TraverseRc>(future.result()) == HitAbortProcess) {
                    QApplication::restoreOverrideCursor();
                    Gui::setAbortProcess(true);
                    return;
                }

                // revert registers to pre jump forward count page settings
                Gui::buildModJumpForward = false;
                Gui::displayPageNum = saveJumpDisplayPageNum;
                gui->current     = saveJumpCurrent;
                Gui::maxPages    = 1 + Gui::pa;
                Gui::stepPageNum = Gui::maxPages;
                Gui::topOfPages  = saveJumpTopOfPages;
                lpub->ldrawFile.unrendered();

            } else {
                if (static_cast<TraverseRc>(future.result()) == HitAbortProcess) {
                    QApplication::restoreOverrideCursor();
                    Gui::setAbortProcess(true);
                    return;
                }
            }
        } // buildModEnabled

        // populate buildMod registers
        gui->setLoadBuildMods(false); // turn off parsing BuildMods in countInstance call until future update
        lpub->ldrawFile.countInstances();

        // set model start page - used to enable mpd combo to jump to start page
        if (firstPage)
            lpub->ldrawFile.setModelStartPageNumber(gui->current.modelName,Gui::maxPages);

    } // not build mod action change
    else if (dpFlags.csiAnnotation) {
        lpub->ldrawFile.countInstances();
    }

    // this call is used primarily by the undo/redo calls when editing BuildMods
    if (!gui->buildModClearStepKey.isEmpty()) {
/*
#ifdef QT_DEBUG_MODE
    emit gui->messageSig(LOG_DEBUG, QString("Reset BuildMod images from step key %1...").arg(buildModClearStepKey));
#endif
//*/

        QStringList keys = gui->buildModClearStepKey.split("_");
        QString key      = keys.first();
        QString option   = keys.last();
        // reset key to avoid endless loop - displayPage called in clear... calls
        gui->buildModClearStepKey.clear();

        // viewer step key or clear submodel (cm) - clear step(s) image and flag submodel stack item(s) as modified
        if (keys.size() == 1 || option == "cm") {
            gui->clearWorkingFiles(gui->getPathsFromViewerStepKey(key));

            // clear page
        } else if (option == "cp") {
            bool multiStepPage = gui->isViewerStepMultiStep(key);
            PlacementType relativeType = multiStepPage ? StepGroupType : SingleStepType;
            gui->clearPageCache(relativeType, &lpub->page, Options::CSI);

            // clear step
        } else if (option == "cs") {
            QString csiPngName = gui->getViewerStepImagePath(key);
            gui->clearStepCSICache(csiPngName);
        }
    }

    gui->writeToTmp();

    if (Gui::abortProcess())
        return;

    Gui::firstStepPageNum     = -1;
    Gui::lastStepPageNum      = -1;
    Gui::savePrevStepPosition =  0;
    Gui::saveGroupStepNum     =  1 + Gui::sa;
    Gui::saveContStepNum      =  1 + Gui::sa;

    // reset buildModActionChange
    dpFlags.buildModActionChange = false;

    QString addLine;
    FindPageFlags fpFlags;
    QList<SubmodelStack> modelStack;

    PageSizeData pageSize;
    if (Gui::exporting()) {
        pageSize.sizeW      = lpub->meta.LPub.page.size.valueInches(0);
        pageSize.sizeH      = lpub->meta.LPub.page.size.valueInches(1);
        pageSize.sizeID     = lpub->meta.LPub.page.size.valueSizeID();
        pageSize.orientation= lpub->meta.LPub.page.orientation.value();
        Gui::pageSizes.insert(     DEF_SIZE,pageSize);
#ifdef PAGE_SIZE_DEBUG
        logTrace() << "0. Inserting INIT page size info at PageNumber:" << Gui::maxPages
                   << "W:"  << pageSize.sizeW << "H:"    << pageSize.sizeH
                   << "O:"  << (pageSize.orientation == Portrait ? "Portrait" : "Landscape")
                   << "ID:" << pageSize.sizeID
                   << "Model:" << gui->current.modelName;
#endif
    }

    FindPageOptions opts(
                Gui::maxPages,/*pageNum*/
                gui->current,
                pageSize,
                fpFlags,
                modelStack,
                dpFlags.updateViewer,
                false,        /*pageDisplayed*/
                false,        /*displayModel*/
                false,        /*mirrored*/
                dpFlags.printing,
                0,            /*stepNumber*/
                0,            /*contStepNumber*/
                0,            /*groupStepNumber*/
                LDRAW_MAIN_MATERIAL_COLOUR,/*renderModelColour*/
                "model~origin"); /*renderParentModel*/

    const TraverseRc frc = static_cast<TraverseRc>(findPage(lpub->meta,addLine,opts));
    if (frc == HitAbortProcess) {
        if (Gui::m_exportMode == GENERATE_BOM) {
            emit gui->clearViewerWindowSig();
            Gui::m_exportMode = Gui::m_saveExportMode;
        }
        Gui::setAbortProcess(true);
        QApplication::restoreOverrideCursor();
        return;

    } else if (Preferences::buildModEnabled && (frc == HitBuildModAction || frc == HitCsiAnnotation)) {

        dpFlags.buildModActionChange = frc == HitBuildModAction;
        dpFlags.csiAnnotation = frc == HitCsiAnnotation;
        Gui::setPageProcessRunning(PROC_DISPLAY_PAGE);
        Gui::clearPage();
        QApplication::restoreOverrideCursor();
        Gui::drawPage(dpFlags);

    } else {

        int modelStackCount = opts.modelStack.size();
        bool parentStepGrpup = opts.flags.parentStepGroup;
        bool parentCallout = opts.flags.parentCallout;

/*
#ifdef QT_DEBUG_MODE
    QString message;
#endif
//*/

        auto countPage = [&] (int modelStackCount)
        {
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
            QFuture<int> future = QtConcurrent::run(CountPageWorker::countPage, &lpub->meta, &lpub->ldrawFile, std::ref(opts), addLine);
#else
            QFuture<int> future = QtConcurrent::run(CountPageWorker::countPage,&lpub->meta,&lpub->ldrawFile,opts,addLine);
#endif
            if (Gui::exporting() || Gui::ContinuousPage() || Gui::countWaitForFinished() || Gui::suspendFileDisplay || modelStackCount) {
/*
#ifdef QT_DEBUG_MODE
          if (modelStackCount) {
            message = QString(" COUNTING  - FutureWatcher WaitForFinsished modelStackCount [%1] WAIT YES").arg(modelStackCount);
            qDebug() << qPrintable(QString("DEBUG: %1").arg(message));
          }
          if (Gui::countWaitForFinished()) {
            message = QString(" COUNTING  - FutureWatcher WaitForFinsished countWaitForFinished WAIT YES");
            qDebug() << qPrintable(QString("DEBUG: %1").arg(message));
          }
          if (suspendFileDisplay) {
            message = QString(" COUNTING  - FutureWatcher WaitForFinsished suspendFileDisplay WAIT YES");
            qDebug() << qPrintable(QString("DEBUG: %1").arg(message));
          }
#endif
//*/
                if (static_cast<TraverseRc>(future.result()) == HitAbortProcess)
                    return static_cast<int>(HitAbortProcess);
                if (!modelStackCount)
                    gui->pagesCounted();
            } else {
/*
#ifdef QT_DEBUG_MODE
        message = QString(" COUNTING  - FutureWatcher SetFuture WAIT NO");
        qDebug() << qPrintable(QString("DEBUG: %1").arg(message));
#endif
//*/
                gui->futureWatcher.setFuture(future);
            }
            return static_cast<int>(HitNothing);
        };
/*
#ifdef QT_DEBUG_MODE
    message = QString(" COUNTING  - Submodel Page (Normal Count) LineNumber %1, ModelName %2, PageNum %3")
                      .arg(opts.current.lineNumber, 3, 10, QChar('0'))
                      .arg(opts.current.modelName)
                      .arg(opts.pageNum, 3, 10, QChar('0'));
    qDebug() << qPrintable(QString("DEBUG: %1").arg(message));
#endif
//*/

        // global meta settings from findPage that went out of scope
        lpub->meta.LPub.countInstance.setValue(opts.flags.countInstances);

        // pass buildMod settings to parent model
        if (Preferences::buildModEnabled && opts.flags.buildModStack.size()) {
            opts.flags.buildMod.setCountPage(opts.flags.buildModStack.last());
/*
#ifdef QT_DEBUG_MODE
      const QString bma [ ] = {"BuildModNoActionRc [0]","BuildModBeginRc [61]","BuildModEndModRc [62]","BuildModEndRc [63]","BuildModApplyRc [64]","BuildModRemoveRc [65]"};
      const QString bms [ ] = {"BM_NONE [-1]","BM_BEGIN [0]","BM_END_MOD [1]","BM_END [2]"};
      message = QString(" COUNTING  - BuildMod flags: "
                        "No. %1, key: '%2', action: %3, level: %4, state %5, ignore: %6")
                        .arg(opts.flags.buildModStack.size(), 2, 10, QChar('0'))
                        .arg(opts.flags.buildMod.key)
                        .arg(opts.flags.buildMod.action ? bma[opts.flags.buildMod.action - BomEndRc] : bma[BuildModNoActionRc])
                        .arg(opts.flags.buildMod.level, 2, 10, QChar('0'))
                        .arg(opts.flags.buildMod.state > BM_NONE ? bms[opts.flags.buildMod.state + 1] : bms[BM_BEGIN])
                        .arg(opts.flags.buildMod.ignore ? "TRUE [1]" : "FALSE [0]");
      qDebug() << qPrintable(QString("DEBUG: %1").arg(message));
#endif
//*/
            // remove buildMod from where we stopped in the parent model
            opts.flags.buildModStack.pop_back();
        }

        if (static_cast<TraverseRc>(countPage(modelStackCount)) == HitAbortProcess) {
            Gui::setAbortProcess(true);
            QApplication::restoreOverrideCursor();
            return;
        }

        // if we start counting from a child submodel, load where findPage stopped in the parent model
        for (int i = 0; i < modelStackCount && !Gui::abortProcess(); i++) {
            // set the step number where the submodel will be rendered
            opts.current = Where(opts.modelStack.last().modelName,
                                 gui->getSubmodelIndex(opts.modelStack.last().modelName),
                                 opts.modelStack.last().lineNumber);

            // set parent model of the submodel being rendered
            opts.renderParentModel = QString(opts.modelStack.last().modelName == gui->topLevelFile()
                                             ? QString() : opts.modelStack.last().modelName);
/*
#ifdef QT_DEBUG_MODE
      message = QString(" COUNTING  - Submodel Page (Model Stack Entry %1%2) LineNumber %3, ModelName %4, PageNum %5")
                        .arg(modelStackCount, 2, 10, QChar('0'))
                        .arg(parentStepGrpup ? ", [StepGroup]" : parentCallout ? ", [Callout]" : "" )
                        .arg(opts.current.lineNumber, 3, 10, QChar('0'))
                        .arg(opts.current.modelName)
                        .arg(opts.pageNum, 3, 10, QChar('0'));
      qDebug() << qPrintable(QString("DEBUG: %1").arg(message));
#endif
//*/

            // pass buildMod settings to parent model
            if (Preferences::buildModEnabled && opts.flags.buildModStack.size()) {
                opts.flags.buildMod.setCountPage(opts.flags.buildModStack.last());
/*
#ifdef QT_DEBUG_MODE
        const QString bma [ ] = {"BuildModNoActionRc [0]","BuildModBeginRc [61]","BuildModEndModRc [62]","BuildModEndRc [63]","BuildModApplyRc [64]","BuildModRemoveRc [65]"};
        const QString bms [ ] = {"BM_NONE [-1]","BM_BEGIN [0]","BM_END_MOD [1]","BM_END [2]"};
        message = QString(" COUNTING  - BuildMod flags: "
                          "No. %1, key: '%2', action: %3, level: %4, state %5, ignore: %6")
                          .arg(opts.flags.buildModStack.size(), 2, 10, QChar('0'))
                          .arg(opts.flags.buildMod.key)
                          .arg(opts.flags.buildMod.action ? bma[opts.flags.buildMod.action - BomEndRc] : bma[BuildModNoActionRc])
                          .arg(opts.flags.buildMod.level, 2, 10, QChar('0'))
                          .arg(opts.flags.buildMod.state > BM_NONE ? bms[opts.flags.buildMod.state + 1] : bms[BM_BEGIN])
                          .arg(opts.flags.buildMod.ignore ? "TRUE [1]" : "FALSE [0]");
        qDebug() << qPrintable(QString("DEBUG: %1").arg(message));
#endif
//*/
                // remove buildMod from where we stopped in the parent model
                opts.flags.buildModStack.pop_back();
            }

            // set flags and increment the parent model lineNumber by 1 if the line is the child submodel
            if (opts.current.lineNumber < lpub->ldrawFile.size(opts.current.modelName)) {
                QString const &line = lpub->ldrawFile.readLine(opts.current.modelName,opts.current.lineNumber).trimmed();
                QStringList token;
                split(line,token);

                if (token.size() == 15) {
                    QString type = token[token.size()-1];

                    if (lpub->ldrawFile.isSubmodel(type)) {
                        // restore parent stepGroup and callout flags.
                        opts.flags.stepGroup = parentStepGrpup;
                        opts.flags.callout = parentCallout;
                        // do not increment partsAdded (enable add new page) if we are in a stepGroup or a callout
                        if (opts.flags.stepGroup || opts.flags.callout)
                            opts.flags.partsAdded = 0;
                        else if (!opts.flags.partsAdded)
                            opts.flags.partsAdded++;
                        // increment to the next line
                        opts.current++;
/*
#ifdef QT_DEBUG_MODE
            bool increment  = !opts.flags.stepGroup && !opts.flags.callout && opts.flags.partsAdded;
            message = QString(" COUNTING  - Submodel Page (Parent Adjust%1) LineNumber %2, ModelName %3, PageNum %4, Line [%5]")
                              .arg(increment ? ", Parts Added" : "")
                              .arg(opts.current.lineNumber, 3, 10, QChar('0'))
                              .arg(opts.current.modelName)
                              .arg(opts.pageNum, 3, 10, QChar('0'))
                              .arg(line);
            qDebug() << qPrintable(QString("DEBUG: %1").arg(message));
#endif
//*/
                    }
                }
            }

            // remove where we stopped in the parent model
            if (opts.modelStack.size())
                opts.modelStack.pop_back();

            // let's go
            if (static_cast<TraverseRc>(countPage(opts.modelStack.size())) == HitAbortProcess) {
                Gui::setAbortProcess(true);
                QApplication::restoreOverrideCursor();
                return;
            }
        } // iterate the model stack

        if (Preferences::modeGUI && ! Gui::exporting() && ! Gui::abortProcess()) {
            bool enable =  Gui::m_exportMode != GENERATE_BOM &&
                    (!lpub->page.coverPage || (lpub->page.coverPage &&
                                              !lpub->page.meta.LPub.coverPageViewEnabled.value()));
            gui->enable3DActions(enable);
        } // modeGUI and not exporting

        QCoreApplication::processEvents();

        QApplication::restoreOverrideCursor();
    }
}

void Gui::finishedCountingPages()
{
    if (static_cast<TraverseRc>(futureWatcher.result()) == HitAbortProcess) {
        Gui::setAbortProcess(true);
        return;
    }
    gui->pagesCounted();
}

void Gui::pagesCounted()
{
    Gui::topOfPages.append(gui->current);

    if (Gui::maxPages > 1)
        Gui::maxPages--;

/*
#ifdef QT_DEBUG_MODE
    emit gui->messageSig(LOG_NOTICE, QString("COUNTED   - Page %1 topOfPage Final Page Finish  (cur) - LineNumber %2, ModelName %3")
                    .arg(Gui::maxPages, 3, 10, QChar('0')).arg(gui->current.lineNumber, 3, 10, QChar('0')).arg(gui->current.modelName));
    if (!Gui::saveDisplayPageNum) {
        emit gui->messageSig(LOG_NOTICE, "---------------------------------------------------------------------------");
        emit gui->messageSig(LOG_NOTICE, QString("RENDERED -  Page %1 of %2").arg(Gui::displayPageNum).arg(Gui::maxPages));
        emit gui->messageSig(LOG_NOTICE, "---------------------------------------------------------------------------");

//        for (int i = 0; i < Gui::topOfPages.size(); i++)
//        {
//            Where top = Gui::topOfPages.at(i);
//            emit gui->messageSig(LOG_NOTICE, QString("COUNTED  -  PageIndex: %1, SubmodelIndex: %2: LineNumber: %3, ModelName: %4")
//                            .arg(i, 3, 10, QChar('0'))               // index
//                            .arg(top.modelIndex, 3, 10, QChar('0'))  // modelIndex
//                            .arg(top.lineNumber, 3, 10, QChar('0'))  // lineNumber
//                            .arg(top.modelName)); // modelName
//        }
    }
#endif
//*/
    if (Preferences::modeGUI && ! Gui::exporting()) {
        QString message;
        if (Gui::saveDisplayPageNum)
            message = QString("%1 of %2") .arg(Gui::saveDisplayPageNum) .arg(Gui::saveMaxPages);
        else
            message = QString("%1 of %2") .arg(Gui::displayPageNum) .arg(Gui::maxPages);

        gui->getAct("setPageLineEditResetAct.1")->setEnabled(false);
        gui->setPageLineEdit->setText(message);
    } // modeGUI and not exporting - countPage and drawPage

    // countPage
    if (Gui::saveDisplayPageNum) {
        if (Gui::displayPageNum > Gui::maxPages)
            Gui::displayPageNum = Gui::maxPages;
        else
            Gui::displayPageNum = Gui::saveDisplayPageNum;

        Gui::saveDisplayPageNum = 0;

        emit gui->messageSig(LOG_STATUS,QString());
    }
    // drawPage
    else
    {
        if (Gui::suspendFileDisplay) {
            if (Preferences::modeGUI && ! Gui::exporting()) {
                QFileInfo fileInfo(Gui::getCurFile());
                emit gui->messageSig(LOG_INFO_STATUS, lpub->ldrawFile._loadAborted ?
                                    tr("Load LDraw file %1 aborted.").arg(fileInfo.fileName()) :
                                    tr("Loaded LDraw file %1 (%2 pages, %3 parts). %4")
                                       .arg(fileInfo.fileName())
                                       .arg(Gui::maxPages)
                                       .arg(lpub->ldrawFile.getPartCount())
                                       .arg(Gui::elapsedTime(fileLoadTimer.elapsed())));
                if (!Gui::maxPages && !lpub->ldrawFile.getPartCount()) {
                    emit gui->messageSig(LOG_ERROR, tr("LDraw file '%1' is invalid - nothing loaded.")
                                                 .arg(fileInfo.absoluteFilePath()));
                    gui->closeModelFile();
                    if (waitingSpinner->isSpinning())
                        waitingSpinner->stop();
                }
            } // modeGUI and not exporting
        } else if (! Gui::ContinuousPage()) {
            emit gui->messageSig(LOG_INFO_STATUS, tr("Page %1 %2. %3.")
                            .arg(Gui::exporting() && Gui::displayPageNum < Gui::maxPages ? Gui::displayPageNum + 1 : Gui::displayPageNum)
                            .arg(Gui::exporting() ? tr("exported") : tr("loaded"),
                                 Gui::elapsedTime(displayPageTimer.elapsed())));
        }

        if (Preferences::modeGUI && ! Gui::exporting() && ! Gui::abortProcess()) {
            gui->enableEditActions();
            if (!Gui::ContinuousPage())
                gui->enableNavigationActions(true);
            if (Gui::m_exportMode == GENERATE_BOM) {
                emit gui->clearViewerWindowSig();
                Gui::m_exportMode = Gui::m_saveExportMode;
            }
            if (waitingSpinner->isSpinning())
                waitingSpinner->stop();
        } // modeGUI and not exporting
    } // drawPage

    if (Gui::suspendFileDisplay) {
        Gui::suspendFileDisplay = false;
        gui->enableActions();
    }

    // reset countPage future wait on last drawPage call from export 'printfile' where exporting() is reset to false
    if (!Gui::exporting() && Gui::countWaitForFinished())
        Gui::setCountWaitForFinished(false);

    Gui::revertPageProcess();

    QApplication::restoreOverrideCursor();
}

int Gui::include(Meta &meta, int &lineNumber, bool &includeFileFound)
{
    Rc rc = OkRc;
    QStringList contents;
    QString filePath = meta.LPub.include.value();
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();

    auto processLine = [&] ()
    {
        Rc prc = InvalidLineRc;
        Where here(fileName,lineNumber);
        QString line = gui->readLine(here);

        if (line.isEmpty())
           return prc;

        switch (line.toLatin1()[0]) {
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
            emit gui->parseErrorSig(tr("Invalid include line [%1].<br>"
                                       "Part lines (type 1 to 5) are ignored in "
                                       "include file.").arg(line),here,Preferences::IncludeFileErrors);
            return prc;
        case '0':
            prc = meta.parse(line,here);
            switch (prc) {
            // Add unsupported include file meta commands here - i.e. commands that involve type 1-5 lines
            case PliBeginSub1Rc:
            case PliBeginSub2Rc:
            case PliBeginSub3Rc:
            case PliBeginSub4Rc:
            case PliBeginSub5Rc:
            case PliBeginSub6Rc:
            case PliBeginSub7Rc:
            case PliBeginSub8Rc:
                emit gui->parseErrorSig(tr("Substitute part meta commands are not supported in include file: [%1].<br>"
                                           "Add this command to the model file or to a submodel.")
                                           .arg(line),here,Preferences::IncludeFileErrors);
                prc = InvalidLineRc;
            default:
                break;
            }
            break;
        }
        return prc;
    };

    auto updateMpdCombo = [&] ()
    {
        QString const includeFile = tr("%1 - Include File").arg(fileName);
        int includeIndex = gui->mpdCombo->findText(includeFile);

        if (includeIndex == -1) {
            int comboIndex = gui->mpdCombo->count() - 1;
            if (lpub->ldrawFile.includeFileList().size() == 1) {
                gui->mpdCombo->addSeparator();
                comboIndex++;
            }
            gui->mpdCombo->addItem(includeFile, fileName);
            comboIndex++;
            int role =
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
            Qt::ForegroundRole;
#else
            Qt::TextColorRole;
#endif
            gui->mpdCombo->setItemData(comboIndex, QBrush(Preferences::darkTheme ? Qt::cyan : Qt::blue), role);
            return true;
        }
        return false;
    };

    includeFileFound = lpub->ldrawFile.isIncludeFile(fileName);

    if (includeFileFound) {
        int numLines = lpub->ldrawFile.size(fileName);
        if (lineNumber < numLines) {
            // update MpdCombo at first line
            if (!lineNumber)
                updateMpdCombo();
            // process one line at a time so break at valid line
            for (;lineNumber < numLines;) {
                rc = processLine();
                lineNumber++;
                if (rc != InvalidLineRc)
                    break;
            }
        } else {
            includeFileFound = false;
            rc = EndOfIncludeFileRc;
        }
    } else {
        if (fileInfo.isReadable()) {
            QFile file(filePath);
            if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
                emit gui->messageSig(LOG_ERROR, tr("Cannot read include file %1<br>%2")
                                                   .arg(filePath, file.errorString()));
                meta.LPub.include.setValue(QString());
                return static_cast<int>(IncludeFileErrorRc);
            }

            emit gui->messageSig(LOG_TRACE, tr("Loading include file '%1'...").arg(filePath));

            QTextStream in(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
           in.setEncoding(lpub->ldrawFile._currFileIsUTF8 ? QStringConverter::Utf8 : QStringConverter::System);
#else
           in.setCodec(lpub->ldrawFile._currFileIsUTF8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForName("System"));
#endif

            /* Read it in to put into subFiles in order of appearance */
            while ( ! in.atEnd()) {
                QString smLine = in.readLine(0);
                if (smLine.isEmpty() || isComment(smLine)) {
                    contents << smLine.trimmed();
                    continue;
                }
                rc = processLine();
                if (rc != InvalidLineRc)
                    contents << smLine.trimmed();
                else
                    contents << QObject::tr("0 // %1 (Invalid include file line)").arg(smLine.trimmed());
            }
            file.close();

            if (contents.size()) {
                gui->disableWatcher();
                QDateTime datetime = fileInfo.lastModified();
                lpub->ldrawFile.insert(fileInfo.fileName(),
                                       contents,
                                       datetime,
                                       UNOFFICIAL_OTHER,
                                       false/*displayModel*/,
                                       true/*generated*/,
                                       true/*includeFile*/,
                                       false/*dataFile*/,
                                       fileInfo.absoluteFilePath(),
                                       fileInfo.completeBaseName());
                gui->enableWatcher();
            }

            updateMpdCombo();

            emit gui->messageSig(LOG_TRACE, tr("Include file '%1' with %2 lines loaded.").arg(fileName).arg(contents.size()));

            rc = EndOfIncludeFileRc;

        } else {
            meta.LPub.include.setValue(QString());

            rc = IncludeFileErrorRc;
        }
    }
    return static_cast<int>(rc);
}

Where Gui::dummy;

Where &Gui::topOfPage()
{
  int pageNum = Gui::displayPageNum - 1;
  if (pageNum < Gui::topOfPages.size()) {
      return Gui::topOfPages[pageNum];
    } else {
      return dummy;
    }
}

Where &Gui::bottomOfPage()
{
  if (Gui::displayPageNum < Gui::topOfPages.size()) {
      return Gui::topOfPages[Gui::displayPageNum];
    } else {
      return dummy;
    }
}

/*
 * For setBuildModForNextStep(), the BuildMod behaviour searches ahead for any BuildMod action meta command in 'next step'.
 *
 * Three operations are performed in this function:
 *
 * 1. BuildMod attributes and content line numbers are captured when BUILD_MOD BEGIN is detected
 * When BUILD_MOD END is detected, the BuildMod item is inserted into the BuildMod list in ldrawFile.
 * The buildModLevel flag uses the getLevel() function to determine the current BuildMod when mods are nested.
 * At this stage, the BuildMod action is set to BuildModApply by default.
 *
 * 2. The BuildMod action for the 'next' step being configured by this instance of writeToTmp() is updated.
 * The action update proceedes as follows: (a.) The index for the 'next' step is captured in buildModStepIndex
 * and used to correctly identify the BuldMod action slot. (b.) The corresponding BuildMod action is determined
 * and subsequently updated when BUILD_MOD APPLY or BUILD_MOD REMOVE meta commands are encountered. These
 * commands must include their respective buildModKey
 *
 * 3. Part lines are written to buildModCsiParts, their corresponding index is added to buildModLineTypeIndexes
 * and buildModItems (bool) is set to the number of mod lines. The buildModCsiParts are added to csiParts and
 * buildModLineTypeIndexes are added to lineTypeIndexes when buildModLevel is BM_BASE_LEVEL [0].
 */

int Gui::setBuildModForNextStep(
        const Where topOfNextStep,
        Where topOfSubmodel)
{
    int  buildModNextStepIndex = getBuildModNextStepIndex(); // set next/'display' step index
    int buildModStepIndex      = BM_FIRST_INDEX;
    int startLine              = BM_BEGIN;
    int partsAdded             = 0;
    bool bottomOfStep          = false;
    bool submodel              = topOfSubmodel.modelIndex > 0 && topOfSubmodel.modelIndex != topOfNextStep.modelIndex;
    QString startModel         = topOfNextStep.modelName;
    Where topOfStep            = topOfNextStep;

    Meta                    meta;

    BuildModFlags           buildMod;
    QMap<int, QString>      buildModKeys;
    QMap<int, QVector<int>> buildModAttributes;

    auto insertAttribute =
            [&buildMod,
             &topOfStep] (
            QMap<int, QVector<int>> &buildModAttributes,
            int index, const Where &here)
    {
        QMap<int, QVector<int>>::iterator i = buildModAttributes.find(buildMod.level);
        if (i == buildModAttributes.end()) {
            QVector<int> modAttributes = { 0, 0, 0, 1, 0, topOfStep.modelIndex, 0, 0 };
            modAttributes[index] = here.lineNumber;
            buildModAttributes.insert(buildMod.level, modAttributes);
        } else {
            i.value()[index] = here.lineNumber;
        }
    };

    auto insertBuildModification =
           [this,
            &partsAdded,
            &buildModAttributes,
            &buildModKeys,
            &topOfStep] (int buildModLevel)
    {
        int buildModStepIndex = getBuildModStepIndex(topOfStep);
        QString buildModKey = buildModKeys.value(buildModLevel);
        QVector<int> modAttributes = { 0, 0, 0, 0, 0, -1, 0, 0 };

        QMap<int, QVector<int>>::iterator i = buildModAttributes.find(buildModLevel);
        if (i == buildModAttributes.end()) {
            emit gui->messageSig(LOG_ERROR, tr("Invalid BuildMod Entry for key: %1").arg(buildModKey));
            return;
        }
        modAttributes = i.value();

        modAttributes[BM_DISPLAY_PAGE_NUM] = Gui::displayPageNum;
        modAttributes[BM_STEP_PIECES]      = partsAdded;
        modAttributes[BM_MODEL_NAME_INDEX] = topOfStep.modelIndex;
        modAttributes[BM_MODEL_LINE_NUM]   = topOfStep.lineNumber;
/*
#ifdef QT_DEBUG_MODE
      emit gui->messageSig(LOG_DEBUG, QString(
                           "Insert NextStep BuildMod StepIndex: %1, "
                           "Action: Apply(64), "
                           "Attributes: %2 %3 %4 %5 %6* %7 %8 %9*, "
                           "ModKey: '%10', "
                           "Level: %11, "
                           "Model: %12")
                           .arg(buildModStepIndex)                      // Attribute Default Initial:
                           .arg(modAttributes.at(BM_BEGIN_LINE_NUM))    // 0         0       this
                           .arg(modAttributes.at(BM_ACTION_LINE_NUM))   // 1         0       this
                           .arg(modAttributes.at(BM_END_LINE_NUM))      // 2         0       this
                           .arg(modAttributes.at(BM_DISPLAY_PAGE_NUM))  // 3         0       this
                           .arg(modAttributes.at(BM_STEP_PIECES))       // 4         0       drawPage
                           .arg(modAttributes.at(BM_MODEL_NAME_INDEX))  // 5        -1       this
                           .arg(modAttributes.at(BM_MODEL_LINE_NUM))    // 6         0       this
                           .arg(modAttributes.at(BM_MODEL_STEP_NUM))    // 7         0       drawPage
                           .arg(buildModKey)
                           .arg(buildModLevel)
                           .arg(topOfStep.modelName));
#endif
//*/
        insertBuildMod(buildModKey,
                       modAttributes,
                       buildModStepIndex);
    };

    // get start index when navigating backwards - check step for first submodel that contan a build modification
    std::function<int(const Where &, int, bool)> getStartIndex;
    getStartIndex = [&](const Where &top, int startIndex, bool submodel) -> int {
        int index = submodel ? startIndex : startIndex + 1;
        Where walk(top);
        if (!walk.lineNumber)
            skipHeader(walk);  // advance past headers
        else
            walk++;            // Advance past STEP meta
        Rc rc;
        QStringList token;
        int numLines = subFileSize(walk.modelName);
        for ( ;walk.lineNumber < numLines && !Gui::abortProcess(); walk.lineNumber++) {
            QString line = gui->readLine(walk);
            switch (line.toLatin1()[0]) {
            case '1':
                split(line,token);
                if (token.size() == 15) {
                    const QString modelName = token[token.size() - 1];
                    if (isSubmodel(modelName)) {
                        Where topOfSubmodel(modelName, getSubmodelIndex(modelName), 0);
                        index = getStartIndex(topOfSubmodel, index, true);
                    }
                }
                break;
            case '0':
                rc = meta.parse(line,walk);
                switch (rc) {
                case BuildModBeginRc:
                case BuildModApplyRc:
                case BuildModRemoveRc:
                    if (submodel)                   // we enountered a  build modification in the step submodel so increment and return the index
                        return index + 1;
                    break;
                case RotStepRc:
                case StepRc:                        // at the end of the current/submodel step so return the index
                    return index;
                default:
                    break;
                }
                break;
            }
        }
        return index;                               // return the index
    };

    // we do this submodel->else block because this call only processes to the end of the specified next step
    if (submodel) {
        if (!topOfSubmodel.lineNumber)
            skipHeader(topOfSubmodel);              // advance past headers

        startLine  = topOfSubmodel.lineNumber;
        startModel = topOfSubmodel.modelName;
        topOfStep  = topOfSubmodel;
/*)
#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_NOTICE, QString("Build Modification Next Step Check - Index: %1, Submodel: '%2'...")
                                                 .arg(buildModNextStepIndex).arg(topOfSubmodel.modelName));
#endif
//*/
    } else {
/* LOCAL LDRAWFILE USED FOR DEBUGGING
#ifdef QT_DEBUG_MODE
        LDrawFile *ldrawFile = &lpub->ldrawFile;
        Q_UNUSED(ldrawFile)
#endif
 //*/
        emit gui->messageSig(LOG_INFO, QString("Build Modification Next Step Check - Index: %1, Model: '%2', Line '%3'...")
                                                      .arg(buildModNextStepIndex).arg(topOfStep.modelName).arg(topOfStep.lineNumber));

        startLine = topOfStep.lineNumber;           // set starting line number

        int startIndex = buildModNextStepIndex;     // when navigating forward, set the delete index to the next step index
        if (Gui::pageDirection > PAGE_JUMP_FORWARD)
            startIndex =
               getStartIndex(topOfStep,startIndex,false); // when navigating backward, set the delete index to the index after the next step index

        deleteBuildMods(startIndex);                // clear all build mods at and after delete index - used after jump ahead and for backward navigation

/*
#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_TRACE, QString("BuildMod Next StartStep - Index: %1, ModelName: %2, LineNumber: %3")
                                                .arg(buildModNextStepIndex).arg(startModel).arg(startLine));
#endif
//*/
        if (Gui::buildModJumpForward)                    // jump forward by more than one step/page
            emit gui->countPagesSig();              // set build mods in countPages call
        else if (Gui::pageDirection != PAGE_FORWARD)     // jump backward by one or more steps/pages
            setBuildModNavBackward();               // set build mod last action for mods up to next step index

        if (Gui::pageDirection != PAGE_FORWARD) {
/*
#ifdef QT_DEBUG_MODE
            emit gui->messageSig(LOG_TRACE, QString("BuildMod Next Jump %1 - Amount: %2 (Steps), StartModel: %3, "
                                                    "StartLine: %4, ModelName: %5, LineNumber: %6")
                                                    .arg(Gui::buildModJumpForward ? tr("Forward") : tr("Backward"))
                                                    .arg(qAbs(buildModNextStepIndex - getBuildModPrevStepIndex()))
                                                    .arg(startModel).arg(startLine)
                                                    .arg(topOfNextStep.modelName)
                                                    .arg(topOfNextStep.lineNumber));
#endif
//*/
            return HitBottomOfStep;
        }
    }

    Where walk(startModel, getSubmodelIndex(startModel), startLine);
    int numLines = subFileSize(walk.modelName);

    // Check if we are at the end of the current submodel
    int hitEndOfSubmodel = 0;
    while (walk.lineNumber == numLines && walk.modelName != topLevelFile()) {
        int nextStepIndex = getStepIndex(walk);
/*
#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_DEBUG, QString("Skip BuildMod Next StepIndex %1, hit end of submodel at ModelName: %2, LineNumber: %3.")
                           .arg(nextStepIndex).arg(walk.modelName).arg(walk.lineNumber));
#endif
//*/
        if (hitEndOfSubmodel > 1)
            setBuildModNextStepIndex(walk);
        if (getBuildModStepIndexWhere(++nextStepIndex, walk)) {
            hitEndOfSubmodel++;
            numLines = subFileSize(walk.modelName);
        }
    }

    if (hitEndOfSubmodel) {
/*
#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_TRACE, QString("BuildMod Next EndStep - Index: %1, ModelName: %2, LineNumber: %3")
                             .arg(buildModNextStepIndex)
                             .arg(walk.modelName)
                             .arg(walk.lineNumber));
#endif
//*/
        return HitEndOfSubmodel;
    }

    if (!submodel)
        LDrawFile::_currentLevels.clear();

    TraverseRc returnValue = HitNothing;
    QString line = readLine(walk);
    Rc rc = meta.parse(line, walk, false);
    if (rc == StepRc || rc == RotStepRc)
        walk++;   // Advance past STEP meta

    // Parse the step lines
    for ( ;
          walk.lineNumber < numLines && !bottomOfStep && !Gui::abortProcess();
          walk.lineNumber++) {

        line = readLine(walk);

        switch (line.toLatin1()[0]) {
        case '0':

            rc =  meta.parse(line,walk,false);

            switch (rc) {

            // Populate BuildMod action and begin, mod_end and end line numbers for 'current' step
            case BuildModApplyRc:
            case BuildModRemoveRc:
                buildModStepIndex = getBuildModStepIndex(topOfStep);
                buildMod.key = meta.LPub.buildMod.key();
                if (buildModContains(buildMod.key)) {
                    buildMod.action = getBuildModAction(buildMod.key, buildModStepIndex);
                } else {
                    const QString action = rc == BuildModApplyRc ? tr("Apply") : tr("Remove");
                    emit gui->parseErrorSig(tr("Next Step BuildMod key '%1' for %2 action was not found.")
                                            .arg(buildMod.key, action),
                                            walk,Preferences::ParseErrors,false,false);
                }
                if ((Rc)buildMod.action != rc) {
                    setBuildModAction(buildMod.key, buildModStepIndex, rc);
                }
                buildMod.state = BM_NONE;
                break;

            // Get BuildMod attributes and set buildModIgnore based on 'next' step buildModAction
            case BuildModBeginRc:
                if (buildMod.state == BM_BEGIN) {
                    QString const message = tr("BUILD_MOD BEGIN '%1' encountered but '%2' was already defined in this STEP.<br><br>"
                                               "Multiple build modifications per STEP are not allowed.")
                                                    .arg(meta.LPub.buildMod.key(), buildMod.key);
                    emit gui->parseErrorSig(message, walk,Preferences::BuildModErrors,false,false);
                }
                buildMod.key   = meta.LPub.buildMod.key();
                buildMod.level = getLevel(buildMod.key, BM_BEGIN);
                buildModKeys.insert(buildMod.level, buildMod.key);
                insertAttribute(buildModAttributes, BM_BEGIN_LINE_NUM, walk);
                buildMod.state = BM_BEGIN;
                break;

            // Set modActionLineNum and buildModIgnore based on 'next' step buildModAction
            case BuildModEndModRc:
                if (buildMod.level > BM_FIRST_LEVEL && buildMod.key.isEmpty())
                    emit gui->parseErrorSig(tr("Key required for nested build mod meta command"),
                                            walk,Preferences::BuildModErrors,false,false);
                if (buildMod.state != BM_BEGIN)
                    emit gui->parseErrorSig(tr("Required meta BUILD_MOD BEGIN for key '%1' not found")
                                            .arg(buildMod.key), walk, Preferences::BuildModErrors,false,false);
                insertAttribute(buildModAttributes, BM_ACTION_LINE_NUM, walk);
                buildMod.state = BM_END_MOD;
                break;

            // Insert buildModAttributes and reset buildMod.level and buildModIgnore to default
            case BuildModEndRc:
                if (buildMod.state != BM_END_MOD)
                    emit gui->parseErrorSig(tr("Required meta BUILD_MOD END_MOD for key '%1' not found")
                                            .arg(buildMod.key), walk, Preferences::BuildModErrors,false,false);
                insertAttribute(buildModAttributes, BM_END_LINE_NUM, walk);
                buildMod.level = getLevel(QString(), BM_END);
                buildMod.state = BM_END;
                break;

            // Search until next occurrence of step/rotstep meta or bottom of step
            // We only need the next STEP build mod even if the next step is in a
            // callout or step group UNLESS, we have callout(s) - i.e. submodel(s)
            // before a build modification in the same STEP in which case we must
            // register the build modification to avoid a 'not registered' error
            // when we get to the drawPage call - basically, we must process any
            // build modifications in the root STEP.
            case RotStepRc:
            case StepRc:
                if (buildModKeys.size()) {
                    if (buildMod.state != BM_END)
                        emit gui->parseErrorSig(tr("Required meta BUILD_MOD END not found"),
                                                walk, Preferences::BuildModErrors,false,false);
                    const QList keys = buildModKeys.keys();
                    for (int buildModLevel : keys)
                        insertBuildModification(buildModLevel);
                }
                bottomOfStep = partsAdded;
                topOfStep = walk;
                buildModKeys.clear();
                buildModAttributes.clear();
                buildMod.clear();
                partsAdded = 0;
                break;
            default:
                break;
            }
            break;

        case '1':
            if (buildMod.state < BM_END_MOD) {
                QStringList token;
                split(line,token);
                if (token.size() == 15) {
                    const QString modelName = token[token.size() - 1];
                    if (isSubmodel(modelName) && !isDisplayModel(modelName)) {
                        Where topOfSubmodel(modelName, getSubmodelIndex(modelName), 0);
                        const TraverseRc nrc = static_cast<TraverseRc>(setBuildModForNextStep(topOfNextStep, topOfSubmodel));
                        if (nrc == HitAbortProcess) {
                            return static_cast<int>(HitAbortProcess);
                        } else {
                            bool buildModFound = false;
                            if (walk.modelIndex == topOfNextStep.modelIndex) {
                                static QRegularExpression buildModBeginRx("^0 !?LPUB BUILD_MOD BEGIN ");
                                if ((buildModFound = Gui::stepContains(walk, buildModBeginRx)))
                                    walk--; // Adjust for line increment
                            }
                            if (!buildModFound) {
                                bottomOfStep = nrc == HitBottomOfStep || nrc == HitEndOfFile;
                            }
                        }
                    }
                }
            }
        case '2':
        case '3':
        case '4':
        case '5':
            partsAdded++;
            break; // case '1' to '5'

        default:
            break;
        } // Process meta-command
    } // For every line

    if (Gui::abortProcess()) {
        returnValue = HitAbortProcess;
    } else {
        // Last step of submodel
        if (buildModKeys.size()) {
            const QList keys = buildModKeys.keys();
            for (int buildModLevel : keys)
                insertBuildModification(buildModLevel);
        }

        if (Gui::abortProcess()) {
            returnValue = HitAbortProcess;
        } else if (bottomOfStep) {
/*
#ifdef QT_DEBUG_MODE
            emit gui->messageSig(LOG_TRACE, QString("BuildMod Next EndStep - Index: %1, ModelName: %2, LineNumber: %3")
                                                    .arg(buildModNextStepIndex)
                                                    .arg(walk.modelName)
                                                    .arg(walk.lineNumber));
#endif
//*/
            returnValue = HitBottomOfStep;
        } else {
            returnValue = HitEndOfFile;
        }
    }

    return static_cast<int>(returnValue);
} // Gui::setBuildModForNextStep()

/*
 * This function applies buffer exchange and LPub3D's remove
 * meta commands before writing them out for the renderers to use.
 * Fade, Highlight and COLOUR meta commands are preserved.
 * This eliminates the need for ghosting parts removed by buffer
 * exchange
 */

QStringList Gui::writeToTmp(const QString &fileName, const QStringList &contents, bool parseContent)
{
  QMutexLocker writeLocker(&writeMutex);

  QFileInfo fileInfo(fileName);
  QString const filePath = QDir::toNativeSeparators(QString("%1/%2/%3").arg(QDir::currentPath(), Paths::tmpDir, fileInfo.fileName()));
  fileInfo.setFile(filePath);
  if(!fileInfo.dir().exists()) {
     fileInfo.dir().mkpath(".");
    }
  QFile file(filePath);
  if ( ! file.open(QFile::WriteOnly|QFile::Text)) {
      emit gui->messageSig(LOG_ERROR, tr("Failed to open %1 for writing:<br>%2")
                                         .arg(filePath, file.errorString()));
      return QStringList();
    } else if (!parseContent) {
      QTextStream out(&file);
      for (int i = 0; i < contents.size(); i++) {
          out << contents[i] << lpub_endl;
        }
      file.close();
    } else {

      LDrawFile::_currentLevels.clear();

      Where topOfStep(fileName, getSubmodelIndex(fileName), 0);

      skipHeader(topOfStep);

      int  buildModBottom     = 0;
      int  buildModLevel      = BM_BASE_LEVEL;
      bool buildModIgnore     = false;
      bool buildModItems      = false;
      bool buildModApplicable = false;
      bool displayModelLine   = false;
      bool isDataFile         = false;

      QString                 buildModKey;
      QMap<int, int>          buildModActions;

      QVector<int> lineTypeIndexes, buildModLineTypeIndexes;
      QStringList  csiParts, buildModCsiParts;
      QHash<QString, QStringList> bfx;

      PartLineAttributes pla(
         csiParts,
         lineTypeIndexes,
         buildModCsiParts,
         buildModLineTypeIndexes,
         buildModLevel,
         buildModIgnore,
         buildModItems);

      Rc    rc;
      Meta  meta;
      for (int i = 0; i < contents.size() && !Gui::abortProcess(); i++) {
          if (displayModelLine)
              continue;

          QString line = contents[i];
          if (line.contains(LDrawFile::_fileRegExp[DAT_RX]))
              isDataFile = true;
          if (isDataFile) {
              csiParts.append(line);
              continue;
          }

          QStringList tokens;
          split(line,tokens);
          if (tokens.size()) {
              if (tokens[0] != "0") {
                  if (! buildModIgnore)
                      CsiItem::partLine(line,pla,i/*relativeTypeIndx*/,OkRc);
              } else {

                  Where here(fileName,i);
                  rc =  meta.parse(line,here,false);

                  switch (rc) {
                  // do not capture display model lines
                  case InsertDisplayModelRc:
                      displayModelLine = true;
                      break;

                  case FadeRc:
                  case SilhouetteRc:
                  case ColourRc:
                      CsiItem::partLine(line,pla,i/*relativeTypeIndx*/,rc);
                      break;

                  // Buffer exchange
                  case BufferStoreRc:
                      bfx[meta.bfx.value()] = csiParts;
                      break;

                  case BufferLoadRc:
                      csiParts = bfx[meta.bfx.value()];
                      break;

                  // Get BuildMod attributes and set buildModIgnore based on 'next' step buildModAction
                  case BuildModBeginRc:
                      if (!Preferences::buildModEnabled) {
                          buildModIgnore = true;
                          break;
                      }
                      buildModBottom    = getBuildModStepLineNumber(getBuildModStepIndex(topOfStep));
                      if ((buildModApplicable = i < buildModBottom)) {
                          buildModKey   = meta.LPub.buildMod.key();
                          buildModLevel = getLevel(buildModKey, BM_BEGIN);
                          // insert 'BuildModRemoveRc' if key not yet created (buildModIgnore = true)
                          if (! buildModContains(buildModKey))
                              buildModActions.insert(buildModLevel, BuildModRemoveRc);
                          else
                              buildModActions.insert(buildModLevel, getBuildModAction(buildModKey, getBuildModNextStepIndex()));
                          if (buildModActions.value(buildModLevel) == BuildModApplyRc)
                              buildModIgnore = false;
                          else if (buildModActions.value(buildModLevel) == BuildModRemoveRc)
                              buildModIgnore = true;
                      }
                      break;

                  // Set modActionLineNum and buildModIgnore based on 'next' step buildModAction
                  case BuildModEndModRc:
                      if (!Preferences::buildModEnabled)
                          break;
                      if (buildModApplicable) {
                          if (buildModLevel > 1 && meta.LPub.buildMod.key().isEmpty())
                              emit gui->parseErrorSig(tr("Key required for nested build mod meta command"),
                                                 here,Preferences::BuildModErrors,false/*option*/,false/*override*/);
                          if (buildModActions.value(buildModLevel) == BuildModApplyRc)
                              buildModIgnore = true;
                          else if (buildModActions.value(buildModLevel) == BuildModRemoveRc)
                              buildModIgnore = false;
                      }
                      break;

                  // Insert buildModAttributes and reset buildModLevel and buildModIgnore to default
                  case BuildModEndRc:
                      if (!Preferences::buildModEnabled)
                          break;
                      if (buildModApplicable) {
                          buildModLevel      = getLevel(QString(), BM_END);
                          if (buildModLevel == BM_BASE_LEVEL) {
                              buildModIgnore     = false;
                              buildModApplicable = false;
                          }
                      }
                      break;

                  case NoStepRc:
                  case RotStepRc:
                  case StepRc:
                      displayModelLine = false;
                      buildModApplicable = false;
                      topOfStep.lineNumber = i;
                      break;

                  case PartNameRc:
                  case PartTypeRc:
                  case TexMapRc:
                  case MLCadGroupRc:
                  case LDCadGroupRc:
                  case LeoCadModelRc:
                  case LeoCadPieceRc:
                  case LeoCadSynthRc:
                  case LeoCadGroupBeginRc:
                  case LeoCadGroupEndRc:
                      CsiItem::partLine(line,pla,i/*relativeTypeIndx*/,rc);
                      break;

                  case LeoCadCameraRc:
                  case LeoCadLightRc:
                  case LeoCadLightTypeRc:
                  case LeoCadLightPOVRayRc:
                  case LeoCadLightShadowless:
                      if (line.contains(" LOCAL "))
                          line.remove("LOCAL ");
                      CsiItem::partLine(line,pla,i/*relativeTypeIndx*/,rc);
                      break;

                      /* remove a group or all instances of a part type */
                  case RemoveGroupRc:
                  case RemovePartTypeRc:
                  case RemovePartNameRc:
                      if (! buildModIgnore) {
                          QStringList newCSIParts;
                          QVector<int> newLineTypeIndexes;
                          if (rc == RemoveGroupRc) {
                              Gui::remove_group(csiParts,lineTypeIndexes,meta.LPub.remove.group.value(),newCSIParts,newLineTypeIndexes,&meta);
                          } else if (rc == RemovePartTypeRc) {
                              Gui::remove_parttype(csiParts,lineTypeIndexes,meta.LPub.remove.parttype.value(),newCSIParts,newLineTypeIndexes);
                          } else {
                              Gui::remove_partname(csiParts,lineTypeIndexes,meta.LPub.remove.partname.value(),newCSIParts,newLineTypeIndexes);
                          }
                          csiParts = newCSIParts;
                          lineTypeIndexes = newLineTypeIndexes;
                      }
                      break;

                  default:
                      break;
                  }
              }
          }
      }

      if (!isDataFile)
          lpub->ldrawFile.setLineTypeRelativeIndexes(topOfStep.modelIndex,lineTypeIndexes);

      QTextStream out(&file);
      for (int i = 0; i < csiParts.size(); i++) {
          out << csiParts[i] << lpub_endl;
        }
      file.close();
      return csiParts;
    }

    return QStringList();
}

void Gui::writeToTmp()
{
  Gui::setPageProcessRunning(PROC_WRITE_TO_TMP);
  QList<QFuture<void>> writeToTmpFutures;
  QElapsedTimer writeToTmpTimer;
  writeToTmpTimer.start();
  bool progressPermInit = true;
  int writtenFiles = 0;
  int subFileCount = lpub->ldrawFile._subFileOrder.size();
  Gui::doFadeStep  = (Preferences::enableFadeSteps || lpub->page.meta.LPub.fadeSteps.setup.value());
  Gui::doHighlightStep = (Preferences::enableHighlightStep || lpub->page.meta.LPub.highlightStep.setup.value()) && !Gui::suppressColourMeta();
  QString const fadeColor = LDrawColor::code(Preferences::validFadeStepsColour);

  auto getExternalFileContent = [] (QString const &fileName)
  {
      QStringList externalFileContents = lpub->ldrawFile.contents(fileName);

      if (externalFileContents.size())
          return externalFileContents;

      QFile file(fileName);
      if (!file.open(QFile::ReadOnly | QFile::Text)) {
          emit gui->messageSig(LOG_ERROR, tr("Cannot read external file %1<br>%2")
                               .arg(fileName, file.errorString()));
          return QStringList();
      }

      QTextStream in(&file);
      while ( ! in.atEnd()) {
          QString sLine = in.readLine(0);
          externalFileContents << sLine.trimmed();
      }
      file.close();

      return externalFileContents;
  };

  for (int i = 0; i < subFileCount && !Gui::abortProcess(); i++) {

      QString const fileName = lpub->ldrawFile._subFileOrder[i].toLower();

      if (lpub->ldrawFile.changedSinceLastWrite(fileName)) {

          if (progressPermInit && !Gui::ContinuousPage()) {
              emit gui->progressPermInitSig();
              emit gui->progressBarPermSetRangeSig(1, subFileCount);
              progressPermInit = false;
          }

          writtenFiles++;

          int numberOfLines = lpub->ldrawFile.size(fileName);

          QString const sourceFilePath = QDir::toNativeSeparators(lpub->ldrawFile.getSubFilePath(fileName));

          bool externalFile = !sourceFilePath.isEmpty();

          bool displayModel = lpub->ldrawFile.isDisplayModel(fileName);

          QString fileType = externalFile ? tr("external ") : QString(" ");
          fileType += lpub->ldrawFile.isUnofficialPart(fileName)
                      ? tr("unofficial %1part ").arg(displayModel ? tr("display ") : QString())
                      : tr("%1submodel ").arg(displayModel ? tr("display ") : QString());

          QString const message = tr("Writing %1'%2' to temp folder...").arg(fileType, fileName);

          QString progressMessage = message;

          if (Gui::suspendFileDisplay) {
              progressMessage = tr("Writing %1%2 of %3 files (%4 lines)...")
                           .arg(fileType,
                                QStringLiteral("%1").arg(i + 1, 3, 10, QLatin1Char('0')),
                                QStringLiteral("%1").arg(subFileCount, 3, 10, QLatin1Char('0')),
                                QStringLiteral("%1").arg(numberOfLines, 5, 10, QLatin1Char('0')));
          } else {
              progressMessage = tr("Writing %1%2 (%3 lines)").arg(fileType).arg(fileName, numberOfLines);
          }

          if (!Gui::ContinuousPage()) {
              emit gui->progressLabelPermSetTextSig(progressMessage);
              emit gui->progressBarPermSetValueSig(i + 1);
              QApplication::processEvents();
          }

          writeToTmpFutures.append(QtConcurrent::run([message, fileName, fileType,
              sourceFilePath, externalFile, fadeColor, displayModel, getExternalFileContent ] () {
              if (!Gui::ContinuousPage())
                  emit gui->messageSig(LOG_INFO, message);

              if (externalFile) {
                  QString const destinationPath = QDir::toNativeSeparators(QDir::currentPath()) + QDir::separator() + Paths::tmpDir + QDir::separator() + fileName;
                  if (QFile::exists(destinationPath)) {
                      QFile::remove(destinationPath);
                  }
                  if(!QFile::copy(sourceFilePath, destinationPath)) {
                      emit gui->messageSig(LOG_ERROR, tr("Could not write %1file '%2' to temp folder...").arg(fileType, fileName));
                  }
              }

              QStringList modelContent;
              if ((Gui::doFadeStep || Gui::doHighlightStep) && externalFile)
                  modelContent = getExternalFileContent(fileName);

              QStringList *futureContent = new QStringList(externalFile ? modelContent : lpub->ldrawFile.contents(fileName));
              gui->writeSmiContent(futureContent, fileName);
              QStringList *cleanContent = new QStringList(gui->writeToTmp(fileName, *futureContent));

              if (!displayModel) {
                  QString const extension = QFileInfo(fileName).suffix().toLower();
                  QString fadeFileNameStr, highlightFileNameStr;

                  if (Gui::doFadeStep) {
                      if (extension.isEmpty())
                        fadeFileNameStr = QString(fileName).append(QString("%1.ldr").arg(FADE_SFX));
                      else
                        fadeFileNameStr = QString(fileName).replace("."+extension, QString("%1.%2").arg(FADE_SFX, extension));
                      emit gui->messageSig(LOG_INFO, tr("Writing %1'%2' to temp folder...").arg(fileType, fadeFileNameStr));
                      QStringList *fadeContent = new QStringList(Gui::configureModelSubFile(*cleanContent, fadeColor, FADE_PART));
                      gui->insertConfiguredSubFile(fadeFileNameStr, *fadeContent);
                      gui->writeToTmp(fadeFileNameStr, *fadeContent, false/*parseContent*/);
                  }

                  if (Gui::doHighlightStep) {
                      if (extension.isEmpty())
                        highlightFileNameStr = QString(fileName).append(QString("%1.ldr").arg(HIGHLIGHT_SFX));
                      else
                        highlightFileNameStr = QString(fileName).replace("."+extension, QString("%1.%2").arg(HIGHLIGHT_SFX, extension));
                      emit gui->messageSig(LOG_INFO, tr("Writing %1'%2' to temp folder...").arg(fileType, highlightFileNameStr));
                      QStringList *highlightContent = new QStringList(Gui::configureModelSubFile(*cleanContent, fadeColor, HIGHLIGHT_PART));
                      gui->insertConfiguredSubFile(highlightFileNameStr, *highlightContent);
                      gui->writeToTmp(highlightFileNameStr, *highlightContent, false/*parseContent*/);
                  }
              }

              delete futureContent;
              delete cleanContent;
          }));
      } // ChangedSinceLastWrite
  } // Parse _subFileOrder

  for (QFuture<void> &future : writeToTmpFutures)
      if (!Gui::abortProcess())
          future.waitForFinished();

  writeToTmpFutures.clear();

  if (Preferences::modeGUI && !Gui::exporting() && !Gui::ContinuousPage() && !Gui::abortProcess()) {
      if (lcGetPreferences().mViewPieceIcons && !Gui::submodelIconsLoaded) {
          // generate submodel icons...
          emit gui->messageSig(LOG_INFO_STATUS, tr("Creating submodel icons..."));
          Pli pli;
          if (pli.createSubModelIcons() == 0)
              gui->SetSubmodelIconsLoaded(Gui::submodelIconsLoaded = true);
          else
              emit gui->messageSig(LOG_ERROR, tr("Could not create submodel icons..."));
      }
  }

  if (!Gui::ContinuousPage()) {
      // complete and close progress
      emit gui->progressPermStatusRemoveSig();

      QString const writeToTmpElapsedTime = Gui::elapsedTime(writeToTmpTimer.elapsed());
      emit gui->messageSig(LOG_INFO_STATUS, tr("%1 %2 written to temp folder. %3")
                                               .arg(writtenFiles ? QString::number(writtenFiles) : tr("No"),
                                                    writtenFiles == 1 ? tr("file") : tr("files"),
                                                    writtenFiles ? writeToTmpElapsedTime : QString()));
  }

  Gui::revertPageProcess();
}

void Gui::writeSmiContent(QStringList *content, const QString &fileName)
{
    if (! Preferences::buildModEnabled)
        return;

    QStringList smiContent = getModelFileContent(content, fileName);

    if (smiContent.size())
        lpub->ldrawFile.setSmiContent(fileName, smiContent);
}

QStringList Gui::getModelFileContent(QStringList *content, const QString &fileName)
{
    QMutexLocker writeLocker(&writeMutex);

    Where top(fileName, 0);

    gui->skipHeader(top);

    QStringList fileContent;

    bool partIgnore       = false;
    bool buildModIgnore   = false;
    bool displayModelLine = false;

    QHash<QString, QStringList> bfx;

    Rc    rc;
    Meta  meta;

    for (int i = top.lineNumber; i < content->size(); i++) {
        if (displayModelLine)
            continue;

        QString line = content->at(i);
        QStringList tokens;
        split(line,tokens);
        if (tokens.size()) {
          if (tokens[0] != "0") {
              if (! buildModIgnore && ! partIgnore)
              fileContent << line;
          } else {

              Where here(fileName,i);
              rc =  meta.parse(line,here,false);

              switch (rc) {
              /* do not capture display model lines */
              case InsertDisplayModelRc:
              displayModelLine = true;
              break;

              case NoStepRc:
              case RotStepRc:
              case StepRc:
              displayModelLine = false;
              break;

              /* buffer exchange */
              case BufferStoreRc:
              bfx[meta.bfx.value()] = fileContent;
              break;

              case BufferLoadRc:
              fileContent = bfx[meta.bfx.value()];
              break;

              /* get BuildMod attributes and set buildModIgnore based on 'next' step buildModAction */
              case BuildModBeginRc:
              buildModIgnore = true;
              break;

              /* set modActionLineNum and buildModIgnore based on 'next' step buildModAction */
              case BuildModEndModRc:
              if (getLevel(QString(), BM_END) == BM_BEGIN)
                      buildModIgnore = false;
              break;

              case PartBeginIgnRc:
              partIgnore = true;
              break;

              case PartEndRc:
              partIgnore = false;
              break;

              case PartNameRc:
              case PartTypeRc:
              case MLCadGroupRc:
              case LDCadGroupRc:
              case LeoCadModelRc:
              case LeoCadPieceRc:
              case LeoCadCameraRc:
              case LeoCadSynthRc:
              case LeoCadGroupBeginRc:
              case LeoCadGroupEndRc:
              fileContent << line;
              break;

              case LeoCadLightRc:
              case LeoCadLightTypeRc:
              case LeoCadLightPOVRayRc:
              case LeoCadLightShadowless:
              if (line.contains(" LOCAL "))
                  line.remove("LOCAL ");
              fileContent << line;
              break;

              /* remove a group or all instances of a part type */
              case RemoveGroupRc:
              case RemovePartTypeRc:
              case RemovePartNameRc:
              if (! buildModIgnore) {
                      QStringList newFileContent;
                      QVector<int> dummy;
                      if (rc == RemoveGroupRc) {
                          Gui::remove_group(fileContent,dummy,meta.LPub.remove.group.value(),newFileContent,dummy,&meta);
                      } else if (rc == RemovePartTypeRc) {
                          Gui::remove_parttype(fileContent,dummy,meta.LPub.remove.parttype.value(),newFileContent,dummy);
                      } else {
                          Gui::remove_partname(fileContent,dummy,meta.LPub.remove.partname.value(),newFileContent,dummy);
                      }
                      fileContent = newFileContent;
              }
              break;

              default:
              break;
              }
          }
        }
    } // for each line

    return fileContent;
}

/*
 * Configure writeToTmp content - make fade or highlight copies of submodel files.
 */
QStringList Gui::configureModelSubFile(const QStringList &contents, const QString &fadeColour, const PartType partType)
{
  QString nameMod, colourPrefix;
  if (partType == FADE_PART) {
    nameMod = FADE_SFX;
    colourPrefix = LPUB3D_COLOUR_FADE_PREFIX;
  } else if (partType == HIGHLIGHT_PART) {
    nameMod = HIGHLIGHT_SFX;
    colourPrefix = LPUB3D_COLOUR_HIGHLIGHT_PREFIX;
  }

  QStringList configuredContents, subfileColourList;
  bool FadeMetaAdded = false;
  bool SilhouetteMetaAdded = false;

  QMutex WriteToTmpMutex;

  if (contents.size() > 0) {

      QStringList argv;

      for (int index = 0; index < contents.size(); index++) {

          WriteToTmpMutex.lock();

          QString contentLine = contents[index];
          split(contentLine, argv);
          if (argv.size() == 15 && argv[0] == "1") {
              // Insert opening fade meta
              if (!FadeMetaAdded && Gui::doFadeStep && partType == FADE_PART) {
                 configuredContents.insert(index,QString("0 !FADE %1").arg(Preferences::fadeStepsOpacity));
                 FadeMetaAdded = true;
              }
              // Insert opening silhouette meta
              if (!SilhouetteMetaAdded && Gui::doHighlightStep && partType == HIGHLIGHT_PART) {
                 configuredContents.insert(index,QString("0 !SILHOUETTE %1 %2")
                                                         .arg(Preferences::highlightStepLineWidth)
                                                         .arg(Preferences::highlightStepColour));
                 SilhouetteMetaAdded = true;
              }
              if (argv[1] != LDRAW_EDGE_MATERIAL_COLOUR &&
                  argv[1] != LDRAW_MAIN_MATERIAL_COLOUR) {
                  QString colourCode;
                  // Insert color code for fade part
                  if (partType == FADE_PART)
                      colourCode = Preferences::fadeStepsUseColour ? fadeColour : argv[1];
                  // Insert color code for silhouette part
                  if (partType == HIGHLIGHT_PART)
                      colourCode = argv[1];
                  // generate fade color entry
                  if (!Gui::colourEntryExist(subfileColourList,argv[1], partType))
                      subfileColourList << Gui::createColourEntry(colourCode, partType);
                  // set color code - fade, highlight or both
                  argv[1] = QString("%1%2").arg(colourPrefix, colourCode);
              }
              // process file naming
              QString fileNameStr = QString(argv[argv.size()-1]).toLower();
              QString extension = QFileInfo(fileNameStr).suffix().toLower();
              // static color parts
              if (LDrawColourParts::isLDrawColourPart(fileNameStr)) {
                  if (extension.isEmpty()) {
                    fileNameStr = fileNameStr.append(QString("%1.ldr").arg(nameMod));
                  } else {
                    fileNameStr = fileNameStr.replace("."+extension, QString("%1.%2").arg(nameMod, extension));
                  }
                }
              // subfiles
              if (lpub->ldrawFile.isSubmodel(fileNameStr)) {
                  if (extension.isEmpty()) {
                    fileNameStr = fileNameStr.append(QString("%1.ldr").arg(nameMod));
                  } else {
                    fileNameStr = fileNameStr.replace("."+extension, QString("%1.%2").arg(nameMod, extension));
                  }
                }
              argv[argv.size()-1] = fileNameStr;
            }

          if (isGhost(contentLine))
              argv.prepend(GHOST_META);

          contentLine = joinLine(argv);

          configuredContents  << contentLine;

          // Insert closing fade and silhouette metas
          if (index+1 == contents.size()) {
              if (FadeMetaAdded) {
                 configuredContents.append(QString("0 !FADE"));
              }
              if (SilhouetteMetaAdded) {
                 configuredContents.append(QString("0 !SILHOUETTE"));
              }
          }

          WriteToTmpMutex.unlock();
      }
  } else {
    return contents;
  }

  // add the color list to the header of the configuredContents
  if (!subfileColourList.isEmpty()) {
      WriteToTmpMutex.lock();

      subfileColourList.removeDuplicates();  // remove dupes
      configuredContents.prepend("0");
      for (int i = 0; i < subfileColourList.size(); ++i)
          configuredContents.prepend(subfileColourList.at(i));
      configuredContents.prepend(QString("0 // %1 step custom colours").arg(VER_PRODUCTNAME_STR));
      configuredContents.prepend("0");

      WriteToTmpMutex.unlock();
  }

  return configuredContents;
}

bool Gui::colourEntryExist(
    const QStringList &colourEntries,
    const QString &code,
    const PartType partType,
    const bool fadeStepsUseColour)
{
    bool _fadeStepsUseColour     = Preferences::fadeStepsUseColour;
    bool fadePartType = partType == FADE_PART;

    if (fadeStepsUseColour != _fadeStepsUseColour)
      _fadeStepsUseColour   = fadeStepsUseColour;

    if (_fadeStepsUseColour && fadePartType && colourEntries.size() > 0)
        return true;

    QString const colourPrefix = fadePartType ? LPUB3D_COLOUR_FADE_PREFIX : LPUB3D_COLOUR_HIGHLIGHT_PREFIX;
    QString const colourCode   = QString("%1%2").arg(colourPrefix, code);

    QStringList colourComponents;
    for (int i = 0; i < colourEntries.size(); ++i) {
        QString colourLine = colourEntries[i];
        split(colourLine,colourComponents);
        if (colourComponents.size() == 11 && colourComponents[4] == colourCode) {
            return true;
        }
    }

    return false;
}

QString Gui::createColourEntry(
    const QString &colourCode,
    const PartType partType,
    const QString &highlightStepColour,
    const QString &fadeStepsColour,
    const bool fadeStepsUseColour,
    const int fadeStepsOpacity)
{
  // Fade Steps Alpha Percent (default = 100%) -  e.g. 50% of Alpha 255 rounded up we get ((255 * 50) + (100 - 1)) / 100

  QString _highlightStepColour = Preferences::highlightStepColour;
  int _fadeStepsOpacity        = Preferences::fadeStepsOpacity;
  bool _fadeStepsUseColour     = Preferences::fadeStepsUseColour;
  bool fadePartType            = partType == FADE_PART;
  const char *colourNamePrefix = fadePartType ? LPUB3D_COLOUR_FADE_NAME_PREFIX : LPUB3D_COLOUR_HIGHLIGHT_NAME_PREFIX;

  if (fadeStepsUseColour      != _fadeStepsUseColour)
      _fadeStepsUseColour      = fadeStepsUseColour;
  if (fadeStepsOpacity        != _fadeStepsOpacity)
      _fadeStepsOpacity        = fadeStepsOpacity;
  if (!highlightStepColour.isEmpty() && highlightStepColour != _highlightStepColour)
      _highlightStepColour     = highlightStepColour;

  int const _fadeAlphaValue        = LPUB3D_OPACITY_TO_ALPHA(_fadeStepsOpacity, LDrawColor::alpha(colourCode));
  int const _alphaValue            = fadePartType ? _fadeAlphaValue : LDrawColor::alpha(colourCode);             // use 100% opacity with highlight color
  QString const _colourPrefix      = fadePartType ? LPUB3D_COLOUR_FADE_PREFIX : LPUB3D_COLOUR_HIGHLIGHT_PREFIX;  // fade prefix 100, highlight prefix 110
  QString const _fadeColour        = _fadeStepsUseColour ? fadeStepsColour.isEmpty() ? LDrawColor::code(Preferences::validFadeStepsColour) : fadeStepsColour : "";
  QString const _colourCode        = _colourPrefix + (fadePartType ? _fadeStepsUseColour ? _fadeColour : colourCode : colourCode);
  QString const _mainColourValue   = LDrawColor::value(colourCode);
  QString const _edgeColourValue   = fadePartType ? LDrawColor::edge(colourCode) + QString("%1").arg(_alphaValue,0,16).toUpper() : _highlightStepColour;
  QString const _colourDescription = colourNamePrefix + LDrawColor::name(colourCode);

  return QString("0 !COLOUR %1 CODE %2 VALUE %3 EDGE %4 ALPHA %5")
                 .arg(_colourDescription,   // description
                      _colourCode,          // original color code
                      _mainColourValue,     // main color value
                      _edgeColourValue)     // edge color value
                 .arg(_alphaValue);         // color alpha value
}

void Gui::setSceneItemZValueDirection(
        QMap<Where, SceneObjectData> *selectedSceneItems,
        Meta &curMeta,
  const QString &line)
{
    auto insertItem = [&selectedSceneItems](Where here, SceneObjectData soData) {
       if (selectedSceneItems->contains(here))
           selectedSceneItems->remove(here);
       selectedSceneItems->insert(here,soData);
       if (Preferences::debugLogging) {
           emit gui->messageSig(LOG_DEBUG, tr("Selected item %1 (%2) added to the current page item list.")
                               .arg(soMap[SceneObject(soData.itemObj)])
                               .arg(soData.itemObj));
       }
   };

   if (line.contains(curMeta.LPub.page.scene.assemAnnotation.preamble))
       insertItem(curMeta.LPub.page.scene.assemAnnotation.here(),
                  curMeta.LPub.page.scene.assemAnnotation.value());
   else if (line.contains(curMeta.LPub.page.scene.assemAnnotationPart.preamble))
       insertItem(curMeta.LPub.page.scene.assemAnnotationPart.here(),
                  curMeta.LPub.page.scene.assemAnnotationPart.value());
   else if (line.contains(curMeta.LPub.page.scene.assem.preamble))
       insertItem(curMeta.LPub.page.scene.assem.here(),
                  curMeta.LPub.page.scene.assem.value());
   else if (line.contains(curMeta.LPub.page.scene.calloutUnderpinning.preamble))
       insertItem(curMeta.LPub.page.scene.calloutUnderpinning.here(),
                  curMeta.LPub.page.scene.calloutUnderpinning.value());
   else if (line.contains(curMeta.LPub.page.scene.calloutBackground.preamble))
       insertItem(curMeta.LPub.page.scene.calloutBackground.here(),
                  curMeta.LPub.page.scene.calloutBackground.value());
   else if (line.contains(curMeta.LPub.page.scene.calloutPointer.preamble))
       insertItem(curMeta.LPub.page.scene.calloutPointer.here(),
                  curMeta.LPub.page.scene.calloutPointer.value());
   else if (line.contains(curMeta.LPub.page.scene.calloutInstance.preamble))
       insertItem(curMeta.LPub.page.scene.calloutInstance.here(),
                  curMeta.LPub.page.scene.calloutInstance.value());
   else if (line.contains(curMeta.LPub.page.scene.dividerBackground.preamble))
       insertItem(curMeta.LPub.page.scene.dividerBackground.here(),
                  curMeta.LPub.page.scene.dividerBackground.value());
   else if (line.contains(curMeta.LPub.page.scene.divider.preamble))
       insertItem(curMeta.LPub.page.scene.divider.here(),
                  curMeta.LPub.page.scene.divider.value());
   else if (line.contains(curMeta.LPub.page.scene.dividerLine.preamble))
       insertItem(curMeta.LPub.page.scene.dividerLine.here(),
                  curMeta.LPub.page.scene.dividerLine.value());
   else if (line.contains(curMeta.LPub.page.scene.dividerPointer.preamble))
       insertItem(curMeta.LPub.page.scene.dividerPointer.here(),
                  curMeta.LPub.page.scene.dividerPointer.value());
   else if (line.contains(curMeta.LPub.page.scene.pointerGrabber.preamble))
       insertItem(curMeta.LPub.page.scene.pointerGrabber.here(),
                  curMeta.LPub.page.scene.pointerGrabber.value());
   else if (line.contains(curMeta.LPub.page.scene.pliGrabber.preamble))
       insertItem(curMeta.LPub.page.scene.pliGrabber.here(),
                  curMeta.LPub.page.scene.pliGrabber.value());
   else if (line.contains(curMeta.LPub.page.scene.submodelGrabber.preamble))
       insertItem(curMeta.LPub.page.scene.submodelGrabber.here(),
                  curMeta.LPub.page.scene.submodelGrabber.value());
   else if (line.contains(curMeta.LPub.page.scene.insertPicture.preamble))
       insertItem(curMeta.LPub.page.scene.insertPicture.here(),
                  curMeta.LPub.page.scene.insertPicture.value());
   else if (line.contains(curMeta.LPub.page.scene.insertText.preamble))
       insertItem(curMeta.LPub.page.scene.insertText.here(),
                  curMeta.LPub.page.scene.insertText.value());
   else if (line.contains(curMeta.LPub.page.scene.pageAttributePixmap.preamble))
       insertItem(curMeta.LPub.page.scene.pageAttributePixmap.here(),
                  curMeta.LPub.page.scene.pageAttributePixmap.value());
   else if (line.contains(curMeta.LPub.page.scene.pageAttributeText.preamble))
       insertItem(curMeta.LPub.page.scene.pageAttributeText.here(),
                  curMeta.LPub.page.scene.pageAttributeText.value());
   else if (line.contains(curMeta.LPub.page.scene.pageNumber.preamble))
       insertItem(curMeta.LPub.page.scene.pageNumber.here(),
                  curMeta.LPub.page.scene.pageNumber.value());
   else if (line.contains(curMeta.LPub.page.scene.pagePointer.preamble))
       insertItem(curMeta.LPub.page.scene.pagePointer.here(),
                  curMeta.LPub.page.scene.pagePointer.value());
   else if (line.contains(curMeta.LPub.page.scene.partsListAnnotation.preamble))
       insertItem(curMeta.LPub.page.scene.pagePointer.here(),
                  curMeta.LPub.page.scene.partsListAnnotation.value());
   else if (line.contains(curMeta.LPub.page.scene.partsListBackground.preamble))
       insertItem(curMeta.LPub.page.scene.partsListBackground.here(),
                  curMeta.LPub.page.scene.partsListBackground.value());
   else if (line.contains(curMeta.LPub.page.scene.partsListInstance.preamble))
       insertItem(curMeta.LPub.page.scene.pageNumber.here(),
                  curMeta.LPub.page.scene.partsListInstance.value());
   else if (line.contains(curMeta.LPub.page.scene.pointerHead.preamble))
       insertItem(curMeta.LPub.page.scene.pageNumber.here(),
                  curMeta.LPub.page.scene.pointerHead.value());
   else if (line.contains(curMeta.LPub.page.scene.pointerFirstSeg.preamble))
       insertItem(curMeta.LPub.page.scene.pageNumber.here(),
                  curMeta.LPub.page.scene.pointerFirstSeg.value());
   else if (line.contains(curMeta.LPub.page.scene.pointerSecondSeg.preamble))
       insertItem(curMeta.LPub.page.scene.pageNumber.here(),
                  curMeta.LPub.page.scene.pointerSecondSeg.value());
   else if (line.contains(curMeta.LPub.page.scene.pointerThirdSeg.preamble))
       insertItem(curMeta.LPub.page.scene.pageNumber.here(),
                  curMeta.LPub.page.scene.pointerThirdSeg.value());
   else if (line.contains(curMeta.LPub.page.scene.rotateIconBackground.preamble))
       insertItem(curMeta.LPub.page.scene.rotateIconBackground.here(),
                  curMeta.LPub.page.scene.rotateIconBackground.value());
   else if (line.contains(curMeta.LPub.page.scene.reserveBackground.preamble))
          insertItem(curMeta.LPub.page.scene.reserveBackground.here(),
                  curMeta.LPub.page.scene.reserveBackground.value());
   else if (line.contains(curMeta.LPub.page.scene.stepNumber.preamble))
       insertItem(curMeta.LPub.page.scene.stepNumber.here(),
                  curMeta.LPub.page.scene.stepNumber.value());
   else if (line.contains(curMeta.LPub.page.scene.subModelBackground.preamble))
       insertItem(curMeta.LPub.page.scene.subModelBackground.here(),
                  curMeta.LPub.page.scene.subModelBackground.value());
   else if (line.contains(curMeta.LPub.page.scene.subModelInstance.preamble))
       insertItem(curMeta.LPub.page.scene.subModelInstance.here(),
                  curMeta.LPub.page.scene.subModelInstance.value());
   else if (line.contains(curMeta.LPub.page.scene.submodelInstanceCount.preamble))
       insertItem(curMeta.LPub.page.scene.submodelInstanceCount.here(),
                  curMeta.LPub.page.scene.submodelInstanceCount.value());
   else if (line.contains(curMeta.LPub.page.scene.partsListPixmap.preamble))
       insertItem(curMeta.LPub.page.scene.partsListPixmap.here(),
                  curMeta.LPub.page.scene.partsListPixmap.value());
   else if (line.contains(curMeta.LPub.page.scene.partsListGroup.preamble))
       insertItem(curMeta.LPub.page.scene.partsListGroup.here(),
                  curMeta.LPub.page.scene.partsListGroup.value());
   else if (line.contains(curMeta.LPub.page.scene.stepBackground.preamble))
       insertItem(curMeta.LPub.page.scene.stepBackground.here(),
                  curMeta.LPub.page.scene.stepBackground.value());
   else if (line.contains(curMeta.LPub.page.scene.multiStepBackground.preamble))
       insertItem(curMeta.LPub.page.scene.stepBackground.here(),
                  curMeta.LPub.page.scene.multiStepBackground.value());
   else /*if (line.contains(curMeta.LPub.page.scene.multiStepsBackground.preamble))*/
       insertItem(curMeta.LPub.page.scene.stepBackground.here(),
                  curMeta.LPub.page.scene.multiStepsBackground.value());
}
