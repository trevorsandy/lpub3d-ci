
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
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department At sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************************
 *
 * This class implements a dialog that lets the user pick the placement for
 * something relative to other things.
 *
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/

#include "metatypes.h"
#include "placementdialog.h"
#include "meta.h"
#include "version.h"

#include <QString>
#include <QGridLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QFrame>
#include <QComboBox>

#include "commonmenus.h"
#include "QsLog.h"

const QString PlacementDialog::labels[][5] =
{
  {"Top/Left",   "Top:Left",   "Top:Center",   "Top:Right",   "Top/Right" },
  {"Left:Top",   "Top/Left",   "Top",          "Top/Right",   "Right:Top" },
  {"Left:Center","Left",       "Center",       "Right",       "Right:Center" },
  {"Left:Bottom","Bottom/Left","Bottom",       "Bottom/Right","Right:Bottom" },
  {"Bottom/Left","Bottom:Left","Bottom:Center","Bottom:Right","Right/Bottom" },
};

const QList<int> PlacementDialog::relativeToOks[NumRelatives] =
{ //                               {Page , Csi , Pli , Pn , Sn , Callout , Sm , Ph , Pf,  Tf,  At,  Ct,  Et,  Urlt, Ms, Ca}
  /*  0 Page             Page    */{0},
  /*  1 Csi (Assem)      Csi     */{Page},
  /*  2 MultiStep        Ms      */{Page       , Pli},
  /*  3 StepNumber       Sn      */{Page , Csi , Pli                          , Ph , Pf},
  /*  4 Pli              Pli     */{Page , Csi       , Pn , Sn           , Sm , Ph , Pf},
  /*  5 Callout          Callout */{Page , Csi , Pli      , Sn                , Ph , Pf},
  /*  6 PageNum          Pn      */{Page                                      , Ph , Pf},

  /*  7 Title            Tt      */{Page},
  /*  8 ModelName        Mnt     */{Page                                                , Tt},
  /*  9 Author           At      */{Page             , Pn                     , Ph , Pf , Tt ,      Ct , Et , Urlt},
  /* 10 Url              Urlt    */{Page             , Pn                     , Ph , Pf      , At , Ct , Et},
  /* 11 ModelDesc        Mdt     */{Page                                                                           , Pt},
  /* 12 PublishDesc      Pdt     */{Page                                                                           , Mdt},
  /* 13 Copyright        Ct      */{Page             , Pn                     , Ph , Pf      , At ,      Et , Urlt},
  /* 14 Email            Et      */{Page             , Pn                     , Ph , Pf      , At , Ct ,      Urlt},
  /* 15 Disclaimer       Dt      */{Page                                                               , Et},
  /* 16 Parts            Pt      */{Page                                                     , At},
  /* 17 Plug             Plt     */{Page                                                                           , Dt},
  /* 18 SubmodelInsCount Sic     */{Page , Csi , Pli , Pn , Sn                , Ph, Pf       , At , Ct,  Et,  Urlt},
  /* 19 DocumentLogo     Dlt     */{Page                                      , Ph , Pf},
  /* 20 CoverImage       Cit     */{Page},
  /* 21 PlugImage        Pit     */{Page                                           , Pf                            , Plt},
  /* 22 PageHeader       Ph      */{Page},
  /* 23 PageFooter       Pf      */{Page},
  /* 24 Category         Cat     */{Page             , Pn                     , Ph , Pf , Tt ,      Ct , Et , Urlt},
  /* 25 Submodel         Sm      */{Page , Csi , Pli , Pn , Sn                , Ph , Pf},
  /* 26 RotateIcon       Ri      */{Page , Csi , Pli      , Sn},
  /* 27 Csi Part         Cp      */{       Csi},
  /* 28 Step             Stp     */{Page                                                                           , Rng},
  /* 29 Range            Rng     */{Page},
  /* 30 Text             Txt     */{Page                                      , Ph , Pf},
  /* 31 Bom              Bom     */{Page                                      , Ph , Pf},

  /* 32 PagePointer       Pptr   */{Page , Csi                                , Ph , Pf},
  /* 33 SingleStep        Ss     */{Page , Csi},
  /* 34 Reserve           Res    */{Page},
  /* 35 CoverPage         Cvp    */{Page},
  /* 36 CsiAnnotationType Ca     */{                                                                                 Cp},
  /* 37 DividerPointer    Dp     */{                                                                                 Cp}

  /* 38 NumRelatives             */
};

//front cover options   Page     | Ph | Pf | Tt | At                  | mnt | pt | mdt | pdt | dlt,
//back  cover options   Page     | Ph | Pf | Tt | At | Ct | Et | Urlt |                      | dlt | dt | plt | pit,

const int PlacementDialog::prepositionOks[NumRelatives] = // indexed by them
{
  /*  0 Page                     */ InsideOk,
  /*  1 Csi                      */ InsideOk|OutsideOk,
  /*  2 MultiStep                */ OutsideOk,
  /*  3 StepNumber               */ OutsideOk,
  /*  4 Pli                      */ OutsideOk,
  /*  5 Callout                  */ OutsideOk,
  /*  6 PageNum                  */ OutsideOk,

  /*  7 Title                    */ OutsideOk,
  /*  8 ModelName                */ OutsideOk,
  /*  9 Author                   */ OutsideOk,
  /* 10 Url                      */ OutsideOk,
  /* 11 ModelDesc                */ OutsideOk,
  /* 12 PublishDesc              */ OutsideOk,
  /* 13 Copyright                */ OutsideOk,
  /* 14 Email                    */ OutsideOk,
  /* 15 Disclaimer               */ OutsideOk,
  /* 16 Parts                    */ OutsideOk,
  /* 17 Plug                     */ OutsideOk,
  /* 18 SubmodelInsCount         */ OutsideOk,
  /* 19 DocumentLogo             */ OutsideOk,
  /* 20 CoverImage               */ OutsideOk,
  /* 21 PlugImage                */ OutsideOk,
  /* 22 PageHeader               */ InsideOk|OutsideOk,
  /* 23 PageFooter               */ InsideOk|OutsideOk,
  /* 24 Category                 */ OutsideOk,
  /* 25 Submodel                 */ OutsideOk,
  /* 26 RotateIcon               */ OutsideOk,
  /* 27 Csi Part                 */ InsideOk|OutsideOk,
  /* 28 Step                     */ InsideOk,
  /* 29 Range                    */ InsideOk,
  /* 30 Text                     */ OutsideOk,
  /* 31 Bom                      */ OutsideOk,

  /* 32 PagePointer              */ OutsideOk,
  /* 33 SingleStep               */ OutsideOk,
  /* 34 Reserve                  */ OutsideOk,
  /* 35 CoverPage                */ InsideOk,
  /* 36 CsiAnnotationType        */ InsideOk|OutsideOk,
  /* 37 DividerPointer           */ InsideOk

  /* 38 NumRelatives             */
};

const QString PlacementDialog::relativeNames[NumRelatives] =
{
  "Page",                       // 0 Page
  "Assem",                      // 1 Csi
  "Step Group",                 // 2 Ms
  "Step Number",                // 3 Sn
  "Parts List",                 // 4 Pli
  "Callout",                    // 5 Callout
  "Page Number",                // 6 Pn

  "Title",                      // 7 Tt
  "Model ID",                   // 8 Mnt
  "Author",                     // 9 At
  "URL",                        //10 Urlt
  "Model Description",          //11 Mdt
  "Publish Description",        //12 Pdt
  "Copyright",                  //13 Ct
  "Email",                      //14 Et
  "Disclaimer",                 //15 Dt
  "Parts",                      //16 Pt
  "Plug",                       //17 Plt
  "Submodel Instance Count",    //18 Sic
  "Logo",                       //19 Dlt
  "Cover Image",                //20 Cit
  "Plug Image",                 //21 Pit
  "Page Header",                //22 Ph
  "Page Footer",                //23 Pf
  "Category",                   //24 Cat
  "Submodel",                   //25 Sm
  "Rotate Icon",                //26 Ri
  "CSI Part",                   //27 Cp
  "Step Rectangle",             //28 Stp
  "Range",                      //29 Rng
  "Text",                       //30 Txt
  "BOM",                        //31 Bom

  "Page Pointer",               //32 Pptr
  "Single Step",                //33 Ss
  "Reserve",                    //34 Res
  "Cover Page",                 //35 Cvp
  "CSI Part Annotation",        //36 Ca
  "Divider Pointer"             //37 Dp

 /*NumRelatives               *///38 NumRelatives
};

bool PlacementDialog::getPlacement(
  PlacementType  parentType,
  PlacementType  placedType,
  PlacementData &goods,
  QString        title,
  QWidget       *parent)
{
  PlacementDialog *dialog = new PlacementDialog(parentType,placedType,goods,title,parent);

  bool ok = dialog->exec() == QDialog::Accepted;
  if (ok) {
    goods.offsets[0] = 0;
    goods.offsets[1] = 0;
  }
  return ok;
}

QString PlacementDialog::placementTypeName(int placementType)
{
  if (placementType >= NumRelatives) {
    return relativeNames[0];
  } else {
    return relativeNames[placementType];
  }
}

PlacementDialog::PlacementDialog(
  PlacementType  parentType,
  PlacementType  placedType,
  PlacementData &_goods,
  QString        title,
  QWidget       *parent)
{
  setWindowTitle(tr("%1 %2 Dialogue").arg(QString::fromLatin1(VER_PRODUCTNAME_STR),title));

  setWhatsThis(lpubWT(WT_DIALOG_PLACEMENT,windowTitle()));

  goods = &_goods;

  outsideGrid    = new QGridLayout;
  insideGrid     = new QGridLayout;
  insideFrame    = new QFrame;
  horizontalLine = new QFrame;

  lblRelativeTo = new QLabel("Relative To",parent);
  lblRelativeTo->setToolTip(tr("Select item that %1 will be moved relative to.").arg(title));
  outsideGrid->addWidget(lblRelativeTo,0,0);

  combo = new QComboBox;
  connect(combo,SIGNAL(activated(int)),this,SLOT(relativeToChanged(int)));
  outsideGrid->addWidget(combo,0,1);

  QList<int> oks;

  PageTypeEnc pageType  = goods->pageType;
  bool partsListPerStep = goods->partsListPerStep;

//* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
  logTrace() << "\nPLACEMENT DIALOG "
             << "\n ParentType: " << RelNames[parentType] << " (" << parentType << ")"
             << "\n PlacedType: " << RelNames[placedType] << " (" << placedType << ")"
             << "\n PageType:   " << (pageType == ContentPage
                                          ? "Content Page"
                                          : pageType == FrontCoverPage
                                                ? "Front Cover Page"
                                                : pageType == BackCoverPage
                                                      ? "Back Cover Page"
                                                      : "Default Page")
                                  << " (" << pageType << ")"
                ;
#endif
//*/

  switch (parentType) {
  case StepGroupType:  //parent type
      switch (placedType) {
      case StepType:   //placed type
          oks << Page << Rng;
          break;
      case PageURLType:
          oks << Page << Pn << Ph << Pf;
          break;
      case PageEmailType:
          oks << Page << Pn << Ph << Pf             << Urlt;
          break;
      case PageAuthorType:
          oks << Page << Pn << Ph << Pf       << Et << Urlt;
          break;
      case PageCopyrightType:
          oks << Page << Pn << Ph << Pf << At << Et << Urlt;
          break;
      case PageNumberType:
          if (partsListPerStep) {
              oks << Page << Ph << Pf;
          } else {
              oks << Page << Ph << Pf << Pli;
              if (goods->stepNumberShown && !goods->relativeOf.contains(StepNumberType))
                  oks << Sn;
              if (goods->subModelShown)
                  oks << Sm;
          }
          break;
      case PartsListType:
          if (partsListPerStep) {
              oks << Csi;
              /*<< Stp*/
          } else {
              oks << Page << Ph << Pf << Ms;
          }
          if (goods->stepNumberShown && !goods->relativeOf.contains(StepNumberType))
              oks << Sn;
          if (goods->subModelShown)
              oks << Sm;
          break;
      case StepNumberType:
          if (partsListPerStep) {
              oks << Csi << Pli /*<< Stp*/;
          } else {
              oks << Page << Pli  << Ph << Pf << Ms;
          }
          if (goods->subModelShown)
              oks << Sm;
          break;
      case SubModelType:
          if (partsListPerStep) {
              oks << Csi << Pli /*<< Stp*/;
          } else {
              oks << Page << Pli << Ph << Pf << Ms;
          }
          if (goods->stepNumberShown && !goods->relativeOf.contains(StepNumberType))
              oks << Sn;
          break;
      case CalloutType:
          oks << Page << Csi << Pli /*<< Stp*/;
          if (goods->stepNumberShown && !goods->relativeOf.contains(StepNumberType))
              oks << Sn;
          break;
      case SubmodelInstanceCountType:
          oks << Page << Ph << Pf << Pn;
          break;
      case PagePointerType:
          oks << Page << Csi << Ph << Pf;
          break;
      case RotateIconType:
          oks << Csi << Pli << Sn /*<< Stp*/;
          break;
      case TextType:
          oks << Page << Ph << Pf/* << Stp  << Csi*/;
          break;
      case CsiAnnotationType:
          oks << Cp;
          break;
      default:
          oks << Page << Pn /*<< Stp*/;
          break;
      }
      break;
  case CalloutType:                                  //parent type
      switch (placedType) {
      case PartsListType:
          oks << Csi;
          if (goods->subModelShown)
              oks << Sm;
          break;
      case StepNumberType:
          oks << Csi  << Pli;
          if (goods->subModelShown)
              oks << Sm;
          break;
      case CalloutType:
          oks << Csi  << Pli << Callout;
          if (goods->stepNumberShown && !goods->relativeOf.contains(StepNumberType))
              oks << Sn;
          if (goods->subModelShown)
              oks << Sm;
          break;
      case SubmodelInstanceCountType:
          oks << Page << Ph << Pf << Pn;
          break;
      case PagePointerType:
          oks << Page << Csi << Ph << Pf;
          break;
      case SubModelType:
      case RotateIconType:
          oks << Csi << Pli;
          if (goods->stepNumberShown && !goods->relativeOf.contains(StepNumberType))
              oks << Sn;
          break;
      case CsiAnnotationType:
          oks << Cp;
          break;
      default:
          oks << Csi << Pli;
          if (goods->stepNumberShown)
              oks << Sn;
          break;
      }
      break;
  case StepType:                                     //parent type
      switch (placedType) {
      case PartsListType:
          oks << Page << Csi << Ph << Pf;
          if (goods->stepNumberShown && !goods->relativeOf.contains(StepNumberType))
              oks << Sn;
          if (goods->subModelShown)
              oks << Sm;
          break;
      case StepNumberType:
          oks << Page << Csi << Pli << Ph << Pf;
          if (goods->subModelShown)
              oks << Sm;
          break;
      case CalloutType:
          oks << Page << Csi << Pli;
          if (goods->stepNumberShown && !goods->relativeOf.contains(StepNumberType))
              oks << Sn;
          break;
      case SubmodelInstanceCountType:
          oks << Page << Csi << Pli << Pn;
          if (goods->stepNumberShown && !goods->relativeOf.contains(StepNumberType))
              oks << Sn;
          break;
      case PagePointerType:
          oks << Page << Csi << Ph << Pf;
          break;
      case SubModelType:
          oks << Page << Csi << Pli << Ph << Pf;
          if (goods->stepNumberShown && !goods->relativeOf.contains(StepNumberType))
              oks << Sn;
          break;
      case RotateIconType:
          oks << Csi << Pli;
          if (goods->stepNumberShown && !goods->relativeOf.contains(StepNumberType))
              oks << Sn;
          break;
      case CsiAnnotationType:
          oks << Cp;
          break;
      default:
          oks << Page << Csi << Pli;
          if (goods->stepNumberShown && !goods->relativeOf.contains(StepNumberType))
              oks << Sn;
          break;
      }
      break;
  case SingleStepType:                               //parent type [Single-step page]
      switch (placedType) {
      case PageURLType:
          if (pageType == BackCoverPage) {
              oks << Page                                       << Ct;
          } else
              oks << Page << Pn << Ph << Pf;
          break;
      case PageEmailType:
          if (pageType == BackCoverPage) {
              oks << Page                               << Urlt;
          } else
              oks << Page << Pn << Ph << Pf             << Urlt;
          break;
      case PageAuthorType:
          if (pageType == FrontCoverPage) {
              oks << Page                                        << Tt;
          } else if (pageType == BackCoverPage) {
              oks << Page                                        << Tt;
          } else
              oks << Page << Pn << Ph << Pf       << Et << Urlt;
          break;
      case PageCopyrightType:
          if (pageType == BackCoverPage) {
              oks << Page                   << At;
          } else
              oks << Page << Pn << Ph << Pf << At << Et << Urlt;
          break;
      default:
          oks << relativeToOks[placedType];
          break;
      }
      break;
  default:                               //parent type
      oks << relativeToOks[placedType];
      break;
  }

  int currentIndex = 0;

  for (int i = 0; i < NumRelatives; i++) {
/* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
    qDebug() << "\n POPULATE PLACEMENT COMBO"
             << "\n PlacedType:" << RelNames[placedType]
             << "\n Index:     " << i
             << "\n MatchType: " << RelNames[i];
#endif
//*/
      Q_FOREACH (int ok, oks) {
          if (ok == i) {
/* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
              qDebug() << " Match Ok:  " << ok << "Type:" << RelNames[i];
#endif
//*/
              combo->addItem(relativeNames[i]);
              if (i == goods->relativeTo) {
                  currentIndex = combo->count()-1;
                }
            }
        }
    }
  combo->setCurrentIndex(currentIndex);

  outsideGrid->addWidget(insideFrame,2,1,3,3);
  insideFrame->setLayout(insideGrid);
  insideFrame->setFrameShape(QFrame::StyledPanel);
  insideFrame->setFrameShadow(QFrame::Sunken);
  horizontalLine->setFrameShape(QFrame::HLine);
  horizontalLine->setFrameShadow(QFrame::Sunken);
  outsideGrid->addWidget(horizontalLine,6,0,1,5);

  for (int R = 0; R < 5; R++) {
    for (int C = 0; C < 5; C++) {
      QPushButton *button;
      button = new QPushButton(labels[R][C],parent);
      buttons[R][C] = button;

      if (R == 0 || R == 4) {
          outsideGrid->addWidget(button,R+1,C);
      } else if (C == 0 || C == 4) {
          outsideGrid->addWidget(button,R+1,C);
      } else {
          insideGrid->addWidget(button,R-1+1,C-1);
      }

      switch (R) {
        case 0:
          switch (C) {
            case 0:
              connect(button,SIGNAL(clicked()),this,SLOT(topLeftO()));
            break;
            case 1:
              connect(button,SIGNAL(clicked()),this,SLOT(topleft()));
            break;
            case 2:
              connect(button,SIGNAL(clicked()),this,SLOT(topcenter()));
            break;
            case 3:
              connect(button,SIGNAL(clicked()),this,SLOT(topright()));
            break;
            case 4:
              connect(button,SIGNAL(clicked()),this,SLOT(topRightO()));
            break;
          }
        break;
        case 1:
          switch (C) {
            case 0:
              connect(button,SIGNAL(clicked()),this,SLOT(lefttop()));
            break;
            case 1:
              connect(button,SIGNAL(clicked()),this,SLOT(topLeftI()));
            break;
            case 2:
              connect(button,SIGNAL(clicked()),this,SLOT(top()));
            break;
            case 3:
              connect(button,SIGNAL(clicked()),this,SLOT(topRightI()));
            break;
            case 4:
              connect(button,SIGNAL(clicked()),this,SLOT(righttop()));
            break;
          }
        break;
        case 2:
          switch (C) {
            case 0:
              connect(button,SIGNAL(clicked()),this,SLOT(leftcenter()));
            break;
            case 1:
              connect(button,SIGNAL(clicked()),this,SLOT(left()));
            break;
            case 2:
              connect(button,SIGNAL(clicked()),this,SLOT(center()));
            break;
            case 3:
              connect(button,SIGNAL(clicked()),this,SLOT(right()));
            break;
            case 4:
              connect(button,SIGNAL(clicked()),this,SLOT(rightcenter()));
            break;
          }
        break;
        case 3:
          switch (C) {
            case 0:
              connect(button,SIGNAL(clicked()),this,SLOT(leftbottom()));
            break;
            case 1:
              connect(button,SIGNAL(clicked()),this,SLOT(bottomLeftI()));
            break;
            case 2:
              connect(button,SIGNAL(clicked()),this,SLOT(bottom()));
            break;
            case 3:
              connect(button,SIGNAL(clicked()),this,SLOT(bottomRightI()));
            break;
            case 4:
              connect(button,SIGNAL(clicked()),this,SLOT(rightbottom()));
            break;
          }
        break;
        case 4:
          switch (C) {
            case 0:
              connect(button,SIGNAL(clicked()),this,SLOT(bottomLeftO()));
            break;
            case 1:
              connect(button,SIGNAL(clicked()),this,SLOT(bottomleft()));
            break;
            case 2:
              connect(button,SIGNAL(clicked()),this,SLOT(bottomcenter()));
            break;
            case 3:
              connect(button,SIGNAL(clicked()),this,SLOT(bottomright()));
            break;
            case 4:
              connect(button,SIGNAL(clicked()),this,SLOT(bottomRightO()));
            break;
          }
        break;
      }
    }
  }

  buttonBox = new QDialogButtonBox();               //removed 'this' from constructor
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(reject()));

  outsideGrid->addWidget(buttonBox,7,0,1,5);

  setLayout(outsideGrid);
  
  setEnabled(prepositionOks[goods->relativeTo]);
  highlightPlacement(goods);
  
  setModal(true);
  setMinimumSize(40,20);
}

PlacementDialog::~PlacementDialog()
{  
}

void PlacementDialog::relativeToChanged(int /* unused */)
{
  QString name = combo->currentText();

  for (int i = 0; i < NumRelatives; i++) {
    if (name == relativeNames[i]) {
      setEnabled(prepositionOks[i]);
      goods->relativeTo = PlacementType(i);
      goods->offsets[0] = 0;
      goods->offsets[1] = 0;
      break;
    }
  }
}

void PlacementDialog::setEnabled(int okPrepositions)
{
  for (int R = 0; R < 5; R++) {
    for (int C = 0; C < 5; C++) {
      buttons[R][C]->setEnabled(false);
    }
  }
  for (int R = 0; R < 5; R++) {
    for (int C = 0; C < 5; C++) {
      if (R == 0 || R == 4 || C == 0 || C == 4) {
        if (okPrepositions & OutsideOk) {
            buttons[R][C]->setEnabled(true);
        }
      } else {
        if (okPrepositions & InsideOk) {
            buttons[R][C]->setEnabled(true);
        }
      }
    }
  }
}

int insideDecode[NumPlacements][2] =
{
/*0 TopLeft   */{ 1, 1 },/*1 Top        */{ 1, 2 },/*2 TopRight*/{ 1, 3 },
/*3 Right     */{ 2, 3 },/*4 BottomRight*/{ 3, 3,},/*5 Bottom  */{ 3, 2 },
/*6 BottomLeft*/{ 3, 1 },/*7 Left       */{ 2, 1 },/*8 Center  */{ 2, 2 }
};

void PlacementDialog::highlightPlacement(
  PlacementData *goods)
{
  int r = 0, c = 0;

  if (goods->preposition == Inside) {
    r = insideDecode[goods->placement][0];
    c = insideDecode[goods->placement][1];
  } else {
    switch (goods->placement) {
      case TopLeft:
        r = 0; c = 0;
      break;
      case TopRight:
        r = 0; c = 4;
      break;
      case BottomLeft:
        r = 4; c = 0;
      break;
      case BottomRight:
        r = 4; c = 4;
      break;
      case Top:
      case Bottom:
        if (goods->placement == Top) {
          r = 0;
        } else {
          r = 4;
        }
        switch (goods->justification) {
          case Left:
            c = 1;
          break;
          case Center:
            c = 2;
          break;
          case Right:
            c = 3;
          break;
          default:
          break;
        }
      break;
      case Right:
      case Left:
        if (goods->placement == Left) {
          c = 0;
        } else {
          c = 4;
        }
        switch (goods->justification) {
          case Top:
            r = 1;
          break;
          case Center:
            r = 2;
          break;
          case Bottom:
            r = 3;
          break;
          default:
          break;
        }
      break;
      default:
      break;
    } 
  }
  QFont font;
  for (int R = 0; R < 5; R++) {
    for (int C = 0; C < 5; C++) {
      font = buttons[R][C]->font();
      font.setBold(R == r && C == c);
      buttons[R][C]->setFont(font);
    }
  }   
}

void PlacementDialog::topLeftO()
{
  goods->placement = TopLeft;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

void PlacementDialog::topleft()
{
  goods->placement = Top;
  goods->justification = Left;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

void PlacementDialog::topcenter()
{
  goods->placement = Top;
  goods->justification = Center;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

void PlacementDialog::topright()
{
  goods->placement = Top;
  goods->justification = Right;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

void PlacementDialog::topRightO()
{
  goods->placement = TopRight;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

/*************************/

void PlacementDialog::lefttop()
{
  goods->placement = Left;
  goods->justification = Top;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

void PlacementDialog::topLeftI()
{
  goods->placement = TopLeft;
  goods->preposition = Inside;
  highlightPlacement(goods);
}

void PlacementDialog::top()
{
  goods->placement = Top;
  goods->justification = Center;
  goods->preposition = Inside;
  highlightPlacement(goods);
}

void PlacementDialog::topRightI()
{
  goods->placement = TopRight;
  goods->preposition = Inside;
  highlightPlacement(goods);
}

void PlacementDialog::righttop()
{
  goods->placement = Right;
  goods->justification = Top;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

/*************************/

void PlacementDialog::leftcenter()
{
  goods->placement = Left;
  goods->justification = Center;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

void PlacementDialog::left()
{
  goods->placement = Left;
  goods->justification = Center;
  goods->preposition = Inside;
  highlightPlacement(goods);
}

void PlacementDialog::center()
{
  goods->placement = Center;
  goods->justification = Center;
  goods->preposition = Inside;
  highlightPlacement(goods);
}

void PlacementDialog::right()
{
  goods->placement = Right;
  goods->preposition = Inside;
  highlightPlacement(goods);
}

void PlacementDialog::rightcenter()
{
  goods->placement = Right;
  goods->justification = Center;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

/*************************/

void PlacementDialog::leftbottom()
{
  goods->placement = Left;
  goods->justification = Bottom;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

void PlacementDialog::bottomLeftI()
{
  goods->placement = BottomLeft;
  goods->preposition = Inside;
  highlightPlacement(goods);
}

void PlacementDialog::bottom()
{
  goods->placement = Bottom;
  goods->justification = Center;
  goods->preposition = Inside;
  highlightPlacement(goods);
}

void PlacementDialog::bottomRightI()
{
  goods->placement = BottomRight;
  goods->preposition = Inside;
  highlightPlacement(goods);
}

void PlacementDialog::rightbottom()
{
  goods->placement = Right;
  goods->justification = Bottom;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

/*************************/

void PlacementDialog::bottomLeftO()
{
  goods->placement = BottomLeft;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

void PlacementDialog::bottomleft()
{
  goods->placement = Bottom;
  goods->justification = Left;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

void PlacementDialog::bottomcenter()
{
  goods->placement = Bottom;
  goods->justification = Center;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

void PlacementDialog::bottomright()
{
  goods->placement = Bottom;
  goods->justification = Right;
  goods->preposition = Outside;
  highlightPlacement(goods);
}

void PlacementDialog::bottomRightO()
{
  goods->placement = BottomRight;
  goods->preposition = Outside;
  highlightPlacement(goods);
}
