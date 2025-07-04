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

/****************************************************************************
 *
 * This file implements a dialog that allows the user to substitute parts.
 *
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/

#include <QtWidgets>
#include <QFileInfo>
#include <functional>
#include "substitutepartdialog.h"
#include "ui_substitutepartdialog.h"

#include "lc_global.h"
#include "ldrawpartdialog.h"
#include "ldrawcolordialog.h"
#include "lpub_object.h"
#include "pli.h"
#include "color.h"

#include "pieceinf.h"
#include "lc_colors.h"
#include "lc_viewwidget.h"
#include "lc_previewwidget.h"

#include "commonmenus.h"

SubstitutePartDialog::SubstitutePartDialog(
    const QStringList &attributes,
    QWidget           *parent,
    int                action,
    const QStringList &defaultList) :
    QDialog(parent),
    ui(new Ui::SubstitutePartDialog),
    mTypeInfo(nullptr),
    mAction(action),
    mViewWidgetEnabled(false)
{
     ui->setupUi(this);

     setWhatsThis(lpubWT(WT_DIALOG_SUBSTITUTE_PART,windowTitle()));

     ui->currentPartBox->setWhatsThis(lpubWT(WT_CONTROL_SUBSTITUTE_CURRENT_PART,ui->currentPartBox->title()));

     ui->primarySettingsBox->setWhatsThis(lpubWT(WT_CONTROL_SUBSTITUTE_PRIMARY_ARGUMENTS,ui->primarySettingsBox->title()));

     ui->extendedSettingsBox->setWhatsThis(lpubWT(WT_CONTROL_SUBSTITUTE_EXTENDED_ARGUMENTS,ui->extendedSettingsBox->title()));

     QString title = QString("%1 Part")
             .arg(mAction == sSubstitute ?
                   "Substitute" : mAction == sUpdate ?
                       "Change Substitute" : "Remove Substitute");
     setWindowTitle(title);

     // set initial attributes
     mDefaultAttributes = mAction == sSubstitute ? attributes :
                          QStringList() << "type" << "color" << defaultList;
     mInitialAttributes = attributes;

     if (mAction != sRemove) {
         mShowExtAttrsBtn = new QPushButton(tr("More..."));
         ui->buttonBox->addButton(mShowExtAttrsBtn, QDialogButtonBox::ActionRole);
         connect(mShowExtAttrsBtn,SIGNAL(clicked(bool)),
                 this,            SLOT(  showExtendedAttributes(bool)));

         ui->extendedSettingsBox->setVisible(false);
         this->adjustSize();
     }

     initialize();

     connect(ui->typeBtn,SIGNAL(clicked(bool)),
             this,        SLOT(  browseType(bool)));

     connect(ui->substituteEdit,SIGNAL(editingFinished()),
             this,              SLOT(  typeChanged()));

     connect(ui->colorBtn,SIGNAL(clicked(bool)),
             this,        SLOT(  browseColor(bool)));

     connect(ui->ldrawTypeBtn,SIGNAL(clicked(bool)),
             this,            SLOT(  browseType(bool)));

     connect(ui->ldrawEdit,SIGNAL(editingFinished()),
             this,         SLOT(  typeChanged()));

     connect(ui->scaleSpin,SIGNAL(valueChanged(double)),
             this,         SLOT  (valueChanged(double)));

     connect(ui->fovSpin,SIGNAL(valueChanged(double)),
             this,       SLOT  (valueChanged(double)));

     connect(ui->latitudeSpin,SIGNAL(valueChanged(double)),
             this,            SLOT  (valueChanged(double)));

     connect(ui->longitudeSpin,SIGNAL(valueChanged(double)),
             this,             SLOT  (valueChanged(double)));

     connect(ui->targetSpinX,SIGNAL(valueChanged(int)),
             this,           SLOT(valueChanged(int)));

     connect(ui->targetSpinY,SIGNAL(valueChanged(int)),
             this,           SLOT(valueChanged(int)));

     connect(ui->targetSpinZ,SIGNAL(valueChanged(int)),
             this,           SLOT(valueChanged(int)));

     connect(ui->rotateSpinX,SIGNAL(valueChanged(double)),
             this,           SLOT(valueChanged(double)));

     connect(ui->rotateSpinY,SIGNAL(valueChanged(double)),
             this,           SLOT(valueChanged(double)));

     connect(ui->rotateSpinZ,SIGNAL(valueChanged(double)),
             this,           SLOT(valueChanged(double)));

     connect(ui->transformCombo,SIGNAL(currentTextChanged(const QString&)),
             this,                SLOT(   transformChanged(const QString&)));

     connect(ui->nameEdit,   SIGNAL(selectionChanged()),
             this,           SLOT(  typeChanged()));

     connect(ui->substituteEdit,SIGNAL(selectionChanged()),
             this,           SLOT(  typeChanged()));

     connect(ui->ldrawEdit,  SIGNAL(selectionChanged()),
             this,           SLOT(  typeChanged()));

     ui->nameEdit->installEventFilter(this);
     ui->substituteEdit->installEventFilter(this);
     ui->ldrawEdit->installEventFilter(this);

     mResetBtn = new QPushButton(tr("Reset"));
     ui->buttonBox->addButton(mResetBtn, QDialogButtonBox::ActionRole);
     connect(mResetBtn,SIGNAL(clicked(bool)),
             this,     SLOT(  reset(bool)));

     setMinimumSize(40, 40);

     if (mAction == sRemove)
         adjustSize();
}

SubstitutePartDialog::~SubstitutePartDialog()
{
    delete ui;
}

bool SubstitutePartDialog::eventFilter(QObject *target, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        bool valid = false;
        if ((valid = target == ui->nameEdit))
            emit ui->nameEdit->selectionChanged();
        else if ((valid |= target == ui->substituteEdit))
            emit ui->substituteEdit->selectionChanged();
        else if ((valid |= target == ui->ldrawEdit))
            emit ui->ldrawEdit->selectionChanged();
        if (valid)
            return true;
    }
    return QDialog::eventFilter(target, event);
}

void SubstitutePartDialog::initialize()
{
    mAttributes = mInitialAttributes;
    bool show   = mAction != sRemove;
    mLdrawType.clear();
    Which attribute = InitialType;

    if (Preferences::debugLogging)
        emit lpub->messageSig(LOG_DEBUG,QString("Loaded substitution part args for type [%1]: [%2]")
                                   .arg(mAttributes.at(sType), mAttributes.join(" ")));

    // extract valid type and adjust attributes
    QString type = mAttributes.at(sType);
    QString substituteType;
    if (mAttributes.size() - 1 == sOriginalType/*has original type*/) {
        QString originalType = QStringList(mAttributes.last().split(":")).first();
        if (mAction == sUpdate) {
            substituteType = type;
            type = originalType;
        }
        if (QStringList(mAttributes.last().split(":")).last().toInt()/*is Ldraw type*/) {
            mLdrawType = originalType;
        }
        mAttributes.removeLast();
    }

    qreal min = -10000.0, max = 10000.0, step = 0.1, val = 1.0;
    auto dec = [] (const qreal v)
    {
        auto places = [&v] () {
            if (v == 0.0)
                return 2;

            int count = 0;
            qreal num = v;
            num = abs(num);
            num = num - int(num);
            while (abs(num) >= 0.0000001) {
                num = num * 10;
                count = count + 1;
                num = num - int(num);
            }
            return count;
        };

        int a = v - int(v);
        return (a < 1 ? places() : QString::number(a).size() < 3 ? 2 : QString::number(a).size());
    };

    QPalette readOnlyPalette = QApplication::palette();
    if (Preferences::darkTheme)
        readOnlyPalette.setColor(QPalette::Base,QColor(Preferences::themeColors[THEME_DARK_PALETTE_MIDLIGHT]));
    else
        readOnlyPalette.setColor(QPalette::Base,QColor(Preferences::themeColors[THEME_DEFAULT_PALETTE_LIGHT]));
    readOnlyPalette.setColor(QPalette::Text,QColor(LPUB3D_DISABLED_TEXT_COLOUR));
    ui->nameEdit->setReadOnly(true);
    ui->nameEdit->setPalette(readOnlyPalette);
    ui->nameEdit->setText(type);
    ui->titleLbl->setText(Pli::titleDescription(type));
    ui->titleLbl->adjustSize();

    ui->primarySettingsBox->setVisible(show);

    ui->substituteEdit->clear();
    ui->substituteEdit->setClearButtonEnabled(true);
    ui->substitueTitleLbl->clear();
    if (mAction == sUpdate && !substituteType.isEmpty()) {
        ui->substituteEdit->setText(substituteType);
        ui->substitueTitleLbl->setText(Pli::titleDescription(substituteType));
        attribute = SubstituteType;
    }

    ui->ldrawEdit->clear();
    ui->ldrawEdit->setClearButtonEnabled(true);
    ui->ldrawTitleLbl->clear();
    if (!mLdrawType.isEmpty()) {
       ui->ldrawEdit->setText(mLdrawType);
       ui->ldrawTitleLbl->setText(Pli::titleDescription(mLdrawType));
    }

    QColor partColor = lcQColorFromVector4(gColorList[lcGetColorIndex(mAttributes.at(sColorCode).toInt())].Value);
    if(partColor.isValid() ) {
      ui->colorLbl->setAutoFillBackground(true);
      QString styleSheet =
        QString("QLabel { background-color: rgb(%1, %2, %3); }")
            .arg(partColor.red())
            .arg(partColor.green())
            .arg(partColor.blue());
      ui->colorLbl->setStyleSheet(styleSheet);
      ui->colorLbl->setToolTip(QString("%1 (%2) %3")
                               .arg(LDrawColor::name(mAttributes.at(sColorCode)).replace("_"," "),
                                    mAttributes.at(sColorCode),
                                    partColor.name().toUpper()));
    }

    // Extended settings

    showPartPreview(attribute);

    if (show)
        val = mAttributes.at(sModelScale).toDouble();
    ui->scaleSpin->setRange(min,max);
    ui->scaleSpin->setSingleStep(step);
    ui->scaleSpin->setDecimals(dec(val));
    ui->scaleSpin->setValue(val);

    if (Preferences::preferredRenderer == RENDERER_NATIVE) {
        min = double(CAMERA_FOV_NATIVE_MIN_DEFAULT);
        max = double(CAMERA_FOV_NATIVE_MAX_DEFAULT);
    } else {
        min = double(CAMERA_FOV_MIN_DEFAULT);
        max = double(CAMERA_FOV_MAX_DEFAULT);
    }
    step = 1.0;
    if (show)
        val = mAttributes.at(sCameraFoV).toDouble();
    ui->fovSpin->setRange(min,max);
    ui->fovSpin->setSingleStep(step);
    ui->fovSpin->setDecimals(dec(val));
    ui->fovSpin->setValue(val);

    min = -360.0;
    max = 360.0;
    if (show)
        val = mAttributes.at(sCameraAngleXX).toDouble();
    ui->latitudeSpin->setRange(min,max);
    ui->latitudeSpin->setSingleStep(step);
    ui->latitudeSpin->setDecimals(dec(val));
    ui->latitudeSpin->setValue(val);

    if (show)
        val = mAttributes.at(sCameraAngleYY).toDouble();
    ui->longitudeSpin->setRange(min,max);
    ui->longitudeSpin->setSingleStep(step);
    ui->longitudeSpin->setDecimals(dec(val));
    ui->longitudeSpin->setValue(val);

    ui->targetSpinX->setRange(0,10000);
    ui->targetSpinX->setSingleStep(1);
    ui->targetSpinX->setValue(mAttributes.at(sTargetX).toInt());

    ui->targetSpinY->setRange(0,10000);
    ui->targetSpinY->setSingleStep(1);
    ui->targetSpinY->setValue(mAttributes.at(sTargetY).toInt());

    ui->targetSpinZ->setRange(0,10000);
    ui->targetSpinZ->setSingleStep(1);
    ui->targetSpinZ->setValue(mAttributes.at(sTargetZ).toInt());

    if (show)
        val = mAttributes.at(sRotX).toDouble();
    ui->rotateSpinX->setRange(min,max);
    ui->rotateSpinX->setSingleStep(1.0);
    ui->rotateSpinX->setDecimals(dec(val));
    ui->rotateSpinX->setValue(val);

    if (show)
        val = mAttributes.at(sRotY).toDouble();
    ui->rotateSpinY->setRange(min,max);
    ui->rotateSpinY->setSingleStep(1.0);
    ui->rotateSpinY->setDecimals(dec(val));
    ui->rotateSpinY->setValue(val);

    if (show)
        val = mAttributes.at(sRotZ).toDouble();
    ui->rotateSpinZ->setRange(min,max);
    ui->rotateSpinZ->setSingleStep(1.0);
    ui->rotateSpinZ->setDecimals(dec(val));
    ui->rotateSpinZ->setValue(val);

    ui->transformCombo->addItem("ABS");
    ui->transformCombo->addItem("REL");
    ui->transformCombo->addItem("ADD");
    ui->transformCombo->setVisible(show);
    if (show)
        ui->transformCombo->setCurrentIndex(!mAttributes.at(sTransform).isEmpty() ?
                                                ui->transformCombo->findText(mAttributes.at(sTransform)) : 1);

    ui->messageLbl->setVisible(show);

    mModified = !show;
}

bool SubstitutePartDialog::getSubstitutePart(
   QStringList       &attributes,
   QWidget           *parent,
   int                action,
   const QStringList &defaultList) {

    SubstitutePartDialog *dialog = new SubstitutePartDialog(attributes,parent,action,defaultList);
    bool ok = dialog->exec() == QDialog::Accepted;
    if (ok) {
      attributes = dialog->mAttributes;
    }
    return ok;
}

void SubstitutePartDialog::showExtendedAttributes(bool clicked) {
    Q_UNUSED(clicked);

    if (ui->extendedSettingsBox->isHidden()) {
        ui->extendedSettingsBox->show();
        mShowExtAttrsBtn->setText("Less...");
        this->adjustSize();
    }
    else{
        ui->extendedSettingsBox->hide();
        mShowExtAttrsBtn->setText("More...");
        this->adjustSize();
    }
}

void SubstitutePartDialog::reset(bool value)
{
    Q_UNUSED(value)
    initialize();
}

void SubstitutePartDialog::valueChanged(int value)
{
    if (sender() == ui->targetSpinX) {
            mAttributes[sTargetX] = QString::number(value);
    } else
    if (sender() == ui->targetSpinY) {
            mAttributes[sTargetY] = QString::number(value);
    } else
    if (sender() == ui->targetSpinZ) {
            mAttributes[sTargetZ] = QString::number(value);
    }
    mModified = true;
}
void SubstitutePartDialog::valueChanged(double value)
{
    if (sender() == ui->scaleSpin) {
            mAttributes[sModelScale] = QString::number(value);
    } else
    if (sender() == ui->fovSpin) {
            mAttributes[sCameraFoV] = QString::number(value);
    } else
    if (sender() == ui->latitudeSpin) {
            mAttributes[sCameraAngleXX] = QString::number(value);
    } else
    if (sender() == ui->longitudeSpin) {
            mAttributes[sCameraAngleYY] = QString::number(value);
    } else
    if (sender() == ui->rotateSpinX) {
            mAttributes[sRotX] = QString::number(value);
    } else
    if (sender() == ui->rotateSpinY) {
            mAttributes[sRotY] = QString::number(value);
    } else
    if (sender() == ui->rotateSpinZ) {
            mAttributes[sRotZ] = QString::number(value);
    }
    mModified = true;
}

void SubstitutePartDialog::typeChanged()
{
    Which attribute = InitialType;
    if (sender() == ui->substituteEdit) {
        if (ui->substituteEdit->text().isEmpty()) {
            ui->substitueTitleLbl->setText(QString());
            showPartPreview(attribute);
            return;
        }
        typeChanged(SubstituteType);
    } else if (sender() == ui->ldrawEdit) {
        if (ui->ldrawEdit->text().isEmpty()) {
            ui->ldrawTitleLbl->setText(QString());
            mLdrawType = QString();
             if (!ui->substituteEdit->text().isEmpty())
                 attribute = SubstituteType;
            showPartPreview(attribute);
            return;
        }
        typeChanged(LdrawType);
    } else {
        showPartPreview(attribute);
    }
}

void SubstitutePartDialog::typeChanged(Which attribute)
{
    QString type,newType,currentType;

    if (attribute == SubstituteType) {
        type        = ui->substituteEdit->text();
        currentType = ui->nameEdit->text();
        newType     = QString(QFileInfo(type).suffix().isEmpty() ? type + ".dat" : type);
        if (newType != currentType) {
            ui->substituteEdit->setText(newType);
        } else {
            ui->messageLbl->setText(QString("Part type %1 is the same as current type.").arg(currentType));
            if (Preferences::darkTheme) {
                const QString themeColor = Preferences::themeColors[THEME_DARK_DECORATE_LPUB3D_QUOTED_TEXT];
                ui->messageLbl->setStyleSheet("QLabel { color : " + themeColor + "; }");
            } else {
                ui->messageLbl->setStyleSheet("QLabel { color : blue; }");
            }
            return;
        }
        ui->substitueTitleLbl->setText(Pli::titleDescription(newType));
        if (!ui->substitueTitleLbl->text().isEmpty()) {
            ui->substitueTitleLbl->adjustSize();
            ui->messageLbl->setText(QString());
            mAttributes[sType] = newType;
            mModified = true;
        } else {
            if (!type.isEmpty()) {
                ui->messageLbl->setText(QString("Part type %1 was not found.").arg(type));
                ui->messageLbl->setStyleSheet("QLabel { color : red; }");
            }
        }

    } else if (attribute == LdrawType) {
        type        = ui->ldrawEdit->text();
        currentType = ui->substituteEdit->text();
        newType     = QString(QFileInfo(type).suffix().isEmpty() ? type + ".dat" : type);
        if (newType != currentType) {
            ui->ldrawEdit->setText(newType);
        } else {
            ui->messageLbl->setText(QString("Part type %1 is the same as substitute type.").arg(currentType));
            if (Preferences::darkTheme) {
                const QString themeColor = Preferences::themeColors[THEME_DARK_DECORATE_LPUB3D_QUOTED_TEXT];
                ui->messageLbl->setStyleSheet("QLabel { color : " + themeColor + "; }");
            } else {
                ui->messageLbl->setStyleSheet("QLabel { color : blue; }");
            }
            return;
        }
        ui->ldrawTitleLbl->setText(Pli::titleDescription(newType));
        if (!ui->ldrawTitleLbl->text().isEmpty()) {
            ui->ldrawTitleLbl->adjustSize();
            ui->messageLbl->setText(QString());
            mLdrawType = newType;
            mModified = true;
        } else {
            if (!type.isEmpty()) {
                ui->messageLbl->setText(QString("Part type %1 was not found.").arg(type));
                ui->messageLbl->setStyleSheet("QLabel { color : red; }");
            }
        }
    }

    showPartPreview(attribute);
}

void SubstitutePartDialog::colorChanged(const QString &colorName)
{
  QColor newColor(colorName);
  if(newColor.isValid() ) {
      ui->colorLbl->setAutoFillBackground(true);
      QString styleSheet =
          QString("QLabel { background-color: rgb(%1, %2, %3); }")
          .arg(newColor.red())
          .arg(newColor.green())
          .arg(newColor.blue());
      ui->colorLbl->setStyleSheet(styleSheet);
      ui->colorLbl->setToolTip(QString("%1 (%2) %3")
                               .arg(LDrawColor::name(mAttributes.at(sColorCode)).replace("_"," "),
                                    mAttributes.at(sColorCode),
                                    newColor.name().toUpper()));

      showPartPreview(PartColor);

      mModified = true;
    }
}

void SubstitutePartDialog::showPartPreview(Which attribute)
{
    bool ok = false;
    int validCode, colorCode = LDRAW_MATERIAL_COLOUR;
    QString partType;
    switch (attribute)
    {
    case SubstituteType:
        partType = ui->substituteEdit->text().trimmed();
        break;
    case LdrawType:
        partType = ui->ldrawEdit->text().trimmed();
        break;
    default: /*InitialType/PartColor*/
        partType = ui->nameEdit->text().trimmed();
        break;
    }

    validCode = mAttributes.at(sColorCode).toUInt(&ok);
    if (ok)
        colorCode = validCode;

    if (!mViewWidgetEnabled) {
        ui->previewFrame->setFrameStyle(QFrame::StyledPanel);
        ui->previewFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

        ui->previewFrame->setFocusPolicy(Qt::StrongFocus);
        ui->previewFrame->setMouseTracking(true);

        previewLayout = new QGridLayout(ui->previewFrame);
        previewLayout->setContentsMargins(0, 0, 0, 0);
        ui->previewFrame->setLayout(previewLayout);

        Preview    = new lcPreview(true/*substitute preview*/);

        ViewWidget = new lcViewWidget(nullptr/*parent*/, Preview/*owner*/);

        if (Preview && ViewWidget) {
            ViewWidget->setAttribute(Qt::WA_DeleteOnClose, true);
            if (!Preview->SetCurrentPiece(partType, colorCode))
                emit lpub->messageSig(LOG_ERROR, QString("Part preview for %1 failed.").arg(partType));
            previewLayout->addWidget(ViewWidget);
            ViewWidget->SetPreferredSize(ui->previewFrame->size());
            mViewWidgetEnabled        = true;
        } else {
            emit lpub->messageSig(LOG_ERROR, QString("Part preview for %1 failed.").arg(partType));
        }
    } else if (Preview) {
        if (!Preview->SetCurrentPiece(partType, colorCode))
            emit lpub->messageSig(LOG_ERROR, QString("Part preview for %1 failed.").arg(partType));
    }
}

void SubstitutePartDialog::browseType(bool clicked)
{
  Q_UNUSED(clicked);
  if (sender() == ui->typeBtn) {
      mTypeInfo = LDrawPartDialog::getLDrawPart(QString("%1;%2")
                                                        .arg(ui->nameEdit->text(),
                                                             mAttributes.at(sColorCode)));
      if (mTypeInfo) {
          ui->substituteEdit->setText(mTypeInfo->mFileName);
          typeChanged(SubstituteType);
      }
  } else
  if (sender() == ui->ldrawTypeBtn) {
      mTypeInfo = LDrawPartDialog::getLDrawPart(QString("%1;%2")
                                                        .arg(ui->substituteEdit->text(),
                                                             mAttributes.at(sColorCode)));
      if (mTypeInfo) {
          ui->ldrawEdit->setText(mTypeInfo->mFileName);
          typeChanged(LdrawType);
      }
  }

}

void SubstitutePartDialog::browseColor(bool clicked)
{
  Q_UNUSED(clicked);
  QColor colorName = lcQColorFromVector4(gColorList[lcGetColorIndex(mAttributes.at(sColorCode).toInt())].Value).name();
  int newColorIndex = ~0U;
  QColor newColor = LDrawColorDialog::getLDrawColor(mAttributes.at(sColorCode).toInt(),newColorIndex,this);
  QString newColorName = newColor.name();
  if (newColor.isValid() && colorName != newColorName) {
      mAttributes[sColorCode] = QString::number(newColorIndex);
      colorChanged(newColor.name());
  }
}

void SubstitutePartDialog::transformChanged(QString const &value)
{
       mModified = true;
       mAttributes[sTransform] = value;
}

void SubstitutePartDialog::accept()
{
    auto setAttributes = [this] ()
    {
        enum Levels{ Level1,Level2,Level3,Level4,Level5,Level6,Level7,Level8,NumLevels };
        bool removeLevel[NumLevels];
        removeLevel[Level1] = removeLevel[Level2] = removeLevel[Level3] =
        removeLevel[Level4] = removeLevel[Level5] = removeLevel[Level6] = false;
        QString label1,label2;

        // Rmovals must be in reverse order

        if (mAction != sRemove) {

            if (mAction == sSubstitute) {
                label1 = "substitute";
                label2 = "Substitution";
            } else if (mAction == sUpdate) {
                label1 = "update";
                label2 = "Substitute update";
            }

            // Level 8 Substitution - Target and Rotation
            QString removeLevel8Sub = QString("%1%2%3%4%5%6%7")
                    .arg(mInitialAttributes.at(sTargetX),
                         mInitialAttributes.at(sTargetY),
                         mInitialAttributes.at(sTargetZ),
                         mInitialAttributes.at(sRotX),
                         mInitialAttributes.at(sRotY),
                         mInitialAttributes.at(sRotZ),
                         mInitialAttributes.at(sTransform));
            QString removeLevel8DefSub = QString("%1%2%3%4%5%6%7")
                    .arg(mDefaultAttributes.at(sTargetX),
                         mDefaultAttributes.at(sTargetY),
                         mDefaultAttributes.at(sTargetZ),
                         mDefaultAttributes.at(sRotX),
                         mDefaultAttributes.at(sRotY),
                         mDefaultAttributes.at(sRotZ),
                         mDefaultAttributes.at(sTransform));
            QString level8Sub = QString("%1%2%3%4%5%6%7")
                    .arg(mAttributes.at(sTargetX),
                         mAttributes.at(sTargetY),
                         mAttributes.at(sTargetZ),
                         mAttributes.at(sRotX),
                         mAttributes.at(sRotY),
                         mAttributes.at(sRotZ),
                         mAttributes.at(sTransform));

            if ((removeLevel[Level8] = level8Sub == removeLevel8Sub &&
                (mAction == sUpdate ? level8Sub == removeLevel8DefSub : true))) {
                 mAttributes.removeAt(sTransform);
                 mAttributes.removeAt(sRotZ);
                 mAttributes.removeAt(sRotY);
                 mAttributes.removeAt(sRotX);
                 mAttributes.removeAt(sTargetZ);
                 mAttributes.removeAt(sTargetY);
                 mAttributes.removeAt(sTargetX);
            }

            if (!removeLevel[Level8]) {
                // Level 7 Substitution - Rotation
                QString removeLevel7Sub = QString("%1%2%3%4")
                        .arg(mInitialAttributes.at(sRotX),
                             mInitialAttributes.at(sRotY),
                             mInitialAttributes.at(sRotZ),
                             mInitialAttributes.at(sTransform));
                QString removeLevel7DefSub = QString("%1%2%3%4")
                        .arg(mDefaultAttributes.at(sRotX),
                             mDefaultAttributes.at(sRotY),
                             mDefaultAttributes.at(sRotZ),
                             mDefaultAttributes.at(sTransform));
                QString level7Sub = QString("%1%2%3%4")
                        .arg(mAttributes.at(sRotX),
                             mAttributes.at(sRotY),
                             mAttributes.at(sRotZ),
                             mAttributes.at(sTransform));

                if ((removeLevel[Level7] = level7Sub == removeLevel7Sub &&
                   (mAction == sUpdate ? level7Sub == removeLevel7DefSub : true))) {
                    mAttributes.removeAt(sTransform);
                    mAttributes.removeAt(sRotZ);
                    mAttributes.removeAt(sRotY);
                    mAttributes.removeAt(sRotX);
                }

                // Level 6 Substitution - Target
                QString removeLevel6Sub = QString("%1%2%3%4")
                        .arg(mInitialAttributes.at(sTargetX),
                             mInitialAttributes.at(sTargetY),
                             mInitialAttributes.at(sTargetZ));
                QString removeLevel6DefSub = QString("%1%2%3%4")
                        .arg(mDefaultAttributes.at(sTargetX),
                             mDefaultAttributes.at(sTargetY),
                             mDefaultAttributes.at(sTargetZ));
                QString level6Sub = QString("%1%2%3")
                        .arg(mAttributes.at(sTargetX),
                             mAttributes.at(sTargetY),
                             mAttributes.at(sTargetZ));

                if (removeLevel[Level7] &&
                   (removeLevel[Level6] = level6Sub == removeLevel6Sub &&
                   (mAction == sUpdate ? level6Sub == removeLevel6DefSub : true))) {
                    mAttributes.removeAt(sTargetZ);
                    mAttributes.removeAt(sTargetY);
                    mAttributes.removeAt(sTargetX);
                }
            }

            // Level 5 Substitution - Camera Angles
            QString removeLevel5Sub = QString("%1%2")
                    .arg(mInitialAttributes.at(sCameraAngleXX),
                         mInitialAttributes.at(sCameraAngleYY));
            QString removeLevel5DefSub = QString("%1%2")
                    .arg(mDefaultAttributes.at(sCameraAngleXX),
                         mDefaultAttributes.at(sCameraAngleYY));
            QString level5Sub = QString("%1%2")
                    .arg(mAttributes.at(sCameraAngleXX),
                         mAttributes.at(sCameraAngleYY));

            if ((removeLevel[Level8] || removeLevel[Level6]) &&
               (removeLevel[Level5] = level5Sub == removeLevel5Sub &&
               (mAction == sUpdate ? level5Sub == removeLevel5DefSub : true))) {
                mAttributes.removeAt(sCameraAngleYY);
                mAttributes.removeAt(sCameraAngleXX);
            }

            // Level 4 Substitution - Camera FOV
            if (removeLevel[Level5] &&
               (removeLevel[Level4] =
                mInitialAttributes.at(sCameraFoV) == mAttributes.at(sCameraFoV) &&
               (mAction == sUpdate ? mDefaultAttributes.at(sCameraFoV) == mAttributes.at(sCameraFoV) : true))) {
                mAttributes.removeAt(sCameraFoV);
            }

            // Level 3 Substitution - Model Scale / Camera Distance Factor
            if (removeLevel[Level4] &&
               (removeLevel[Level3] =
                mInitialAttributes.at(sModelScale) == mAttributes.at(sModelScale) &&
               (mAction == sUpdate ? mDefaultAttributes.at(sModelScale) == mAttributes.at(sModelScale) : true))) {
                mAttributes.removeAt(sModelScale);
            }

            // Level 2 Substitution - Colour [always populate colour]
            removeLevel[Level2] =
                            mInitialAttributes.at(sColorCode) == mAttributes.at(sColorCode) &&
                           (mAction == sUpdate ? mDefaultAttributes.at(sColorCode) == mAttributes.at(sColorCode) : true);

            // Level 1 Substitution - Part Type
            if (removeLevel[Level2] &&
               (removeLevel[Level1] =
                 mInitialAttributes.at(sType) == mAttributes.at(sType) &&
               (mAction == sUpdate ? mDefaultAttributes.at(sColorCode) == mAttributes.at(sColorCode) : true))) {
                emit lpub->messageSig(LOG_TRACE,QString("Nothing to %1 for part type [%2]: [%3]")
                                           .arg(label1, mAttributes.at(sType), mAttributes.join(" ")));
                mAttributes.clear();
                mModified = false;
                return;
            }

            // Finally attached Ldraw Type if exist and other attribues also exist
            if (mAttributes.size() && !mLdrawType.isEmpty()) {
                mAttributes.append(QString("LDRAW_TYPE %1").arg(mLdrawType));
            }

        } else {
            label1 = "remove";
            label2 = "Substitute removal";
        }

        emit lpub->messageSig(LOG_TRACE,QString("%1 for part type %2: [%3], Input: [%4]")
                                                .arg(label2, mAttributes.at(sType), mAttributes.join(" "), mInitialAttributes.join(" ")));
    };

    if (mModified) {
      if (Preferences::debugLogging)
          emit lpub->messageSig(LOG_DEBUG,QString("Set substitution for part type %1: [%2], Input: [%3]")
                                                  .arg(mAttributes.at(sType), mAttributes.join(" "), mInitialAttributes.join(" ")));
      setAttributes();
    }

    if (mModified) {
      QDialog::accept();
    } else {
      QDialog::reject();
    }
}

void SubstitutePartDialog::cancel()
{
    QDialog::reject();
}
