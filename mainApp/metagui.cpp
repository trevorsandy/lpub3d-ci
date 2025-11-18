
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
 * This file describes a set of classes that implement graphical user
 * interfaces for some of the configuration meta classes described in meta.h
 *
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/

#include "metagui.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QIntValidator>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>

#include <QDialogButtonBox>
#include <QFontDialog>
#include <QColorDialog>
#include <QFileDialog>
#include <QString>
#include <JlCompress.h>

//gradient
#include <QGradient>

#include "meta.h"
#include "metatypes.h"
#include "metaitem.h"
#include "placementdialog.h"
#include "color.h"

#include "lpub.h"
#include "lpub_preferences.h"
#include "resolution.h"
#include "render.h"
#include "commonmenus.h"

#include "lc_colors.h"
#include "lc_colorpicker.h"

#include "gradients.h"
#include "pagesizes.h"

bool MetaGui::notEqual(const double v1, const double v2, int p)
{
    const QString _v1 = QString::number(v1,'f',p);
    const QString _v2 = QString::number(v2,'f',p);
    const bool     r  = _v1 != _v2;
    return r;
}

QString MetaGui::formatMask(
  const float value,
  const int   width,
  const int   precision,
  const int   defaultDecimalPlaces)
{
  QString mask;
  // capture value as a floating point number
  double real  = value;
  // capture the whole number size minus 1 to
  int wholeSize = qMax(QString::number(int(real)).size(), width - (precision + 1));
  // subtract integer to get decimal portion
  double residual = real - int(real);
  // cast decimal portion to string and count characters
  int floatSize = QString::number(residual,'f',precision).size();
  // subtract 2 places to account for '0.' in float
  int decimalSize = floatSize > 1 ? floatSize - 2 : floatSize;
  // capture decimal places
  int decimalPlaces = qMax(decimalSize, defaultDecimalPlaces);
  // construct the mask using 'x' - any non-blank character permitted but not required.
  for (int i = 0; i < wholeSize; i++) i + 1 == wholeSize ? mask.append("xx")/*extra 'x' at end for '-' sign*/ : mask.append("x");
  for (int i = 0; i < decimalPlaces; i++) mask.append("x");

//#ifdef QT_DEBUG_MODE
//  qDebug() << qPrintable(QString("FORMAT MASK - Input: %1, Mask: %2, "
//                                 "Whole String %3, Decimal String: %4, "
//                                 "Decimal Places %5, Field Width %6, "
//                                 "Precision %7, Default Decimal Places %8")
//                                 /*1*/.arg(value)
//                                 /*2*/.arg(mask)
//                                 /*3*/.arg(QString::number(int(real)))
//                                 /*4*/.arg(QString::number(residual,'f',precision))
//                                 /*5*/.arg(decimalPlaces)
//                                 /*6*/.arg(width)
//                                 /*7*/.arg(precision)
//                                 /*8*/.arg(defaultDecimalPlaces));
//#endif

  return mask;
}

/***********************************************************************
 *
 * Checkbox
 *
 **********************************************************************/

CheckBoxGui::CheckBoxGui(
  QString const  &heading,
  BoolMeta       *_meta,
  QGroupBox      *parent)
{
  meta = _meta;
  value = _meta->value();

  QHBoxLayout *layout = new QHBoxLayout(parent);

  if (parent) {
    parent->setLayout(layout);
    setWhatsThis(lpubWT(WT_GUI_CHECK_BOX, parent->title()));
  } else {
    setLayout(layout);
    setWhatsThis(lpubWT(WT_GUI_CHECK_BOX, heading.isEmpty() ? tr("Enable") : heading));
  }

  check = new QCheckBox(heading,parent);
  check->setChecked(value);
  layout->addWidget(check);
  connect(check,SIGNAL(clicked(bool)),
          this, SLOT(  clicked(bool)));
}

void CheckBoxGui::clicked(bool checked)
{
  meta->setValue(checked);
  modified = value != checked;
}

void CheckBoxGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * BoolRadio
 *
 **********************************************************************/

BoolRadioGui::BoolRadioGui(
  const QString &trueHeading,
  const QString &falseHeading,
  BoolMeta       *_meta,
  QGroupBox      *parent)
{
  meta = _meta;
  value = meta->value();

  QVBoxLayout *layout = new QVBoxLayout(parent);

  if (parent) {
      parent->setLayout(layout);
      setWhatsThis(lpubWT(WT_GUI_BOOL_RADIO_BUTTON, parent->title()));
  } else {
      setLayout(layout);
      setWhatsThis(lpubWT(WT_GUI_BOOL_RADIO_BUTTON, tr("Option")));
  }

  trueRadio = new QRadioButton(trueHeading,parent);
  connect(trueRadio,SIGNAL(clicked(bool)),
          this,     SLOT(  trueClicked(bool)));
  layout->addWidget(trueRadio);
  trueRadio->setChecked(value);

  falseRadio = new QRadioButton(falseHeading,parent);
  connect(falseRadio,SIGNAL(clicked(bool)),
          this,      SLOT(  falseClicked(bool)));
  layout->addWidget(falseRadio);
  falseRadio->setChecked(!value);
}

void BoolRadioGui::trueClicked(bool)
{
  meta->setValue(true);
  modified = value != true;
}

void BoolRadioGui::falseClicked(bool)
{
  meta->setValue(false);
  modified = value != false;
}

void BoolRadioGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Units
 *
 **********************************************************************/

UnitsGui::UnitsGui(
  const QString &heading,
  UnitsMeta     *_meta,
  QGroupBox     *parent,
  bool           isMargin)
{
  meta  = _meta;
  data0 = _meta->value(0);
  data1 = _meta->value(1);

  QHBoxLayout *layout = new QHBoxLayout(parent);

  WT_Type wtType = isMargin ? WT_GUI_UNITS_MARGIN : WT_GUI_UNITS;

  if (parent) {
    parent->setLayout(layout);
    parent->setWhatsThis(lpubWT(wtType, parent->title()));
  } else {
    setLayout(layout);
    setWhatsThis(lpubWT(wtType, heading.isEmpty() ? isMargin ? tr("Margins") : tr("Units") : heading));
  }

  bool secondLabel = false;
  QStringList labels;
  if (heading != "") {
    labels = heading.split("|");
    secondLabel = labels.size() > 1;
    label = new QLabel(labels.first(),parent);
    layout->addWidget(label);
  } else {
    label = nullptr;
    label2 = nullptr;
  }

  value0Edit = new QLineEdit(parent);
  QDoubleValidator *value0Validator = new QDoubleValidator(value0Edit);
  value0Validator->setRange(0.0f, 1000.0f);
  value0Validator->setDecimals(meta->_precision);
  value0Validator->setNotation(QDoubleValidator::StandardNotation);
  value0Edit->setValidator(value0Validator);
  value0Edit->setText(QString::number(meta->value(0),'f',meta->_precision));
  reset0Act = value0Edit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  reset0Act->setText(tr("Reset"));
  reset0Act->setEnabled(false);
  connect(value0Edit, SIGNAL( textEdited(const QString&)),
          this,         SLOT(enableReset(const QString&)));
  connect(reset0Act,  SIGNAL(triggered()),
          this,         SLOT(lineEditReset()));
  connect(value0Edit,SIGNAL(  textEdited(const QString&)),
          this,        SLOT(value0Change(const QString&)));
  layout->addWidget(value0Edit);

  if (secondLabel) {
    label2 = new QLabel(labels.last(),parent);
    layout->addWidget(label2);
  }

  value1Edit = new QLineEdit(parent);
  QDoubleValidator *value1Validator = new QDoubleValidator(value1Edit);
  value1Validator->setRange(0.0f, 1000.0f);
  value1Validator->setDecimals(meta->_precision);
  value1Validator->setNotation(QDoubleValidator::StandardNotation);
  value1Edit->setValidator(value1Validator);
  value1Edit->setText(QString::number(meta->value(1),'f',meta->_precision));
  reset1Act = value1Edit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  reset1Act->setText(tr("Reset"));
  reset1Act->setEnabled(false);
  connect(value1Edit, SIGNAL( textEdited(const QString&)),
          this,         SLOT(enableReset(const QString&)));
  connect(reset1Act,  SIGNAL(    triggered()),
          this,         SLOT(lineEditReset()));
  connect(value1Edit, SIGNAL(  textEdited(const QString&)),
          this,         SLOT(value1Change(const QString&)));
  layout->addWidget(value1Edit);

  unitsModified = false;
}

void UnitsGui::enableReset(const QString &displayText)
{
  const double value = displayText.toDouble();

  if (sender() == value0Edit) {
    reset0Act->setEnabled(notEqual(value, data0));
  } else
  if (sender() == value1Edit) {
    reset1Act->setEnabled(notEqual(value, data1));
  }
}

void UnitsGui::lineEditReset()
{
  if (sender() == reset0Act) {
    reset0Act->setEnabled(false);
    if (value0Edit) {
      value0Edit->setText(QString::number(data0,'f',meta->_precision));
      unitsModified = false;
    }
  }
  else
  if (sender() == reset1Act) {
    reset1Act->setEnabled(false);
    if (value1Edit) {
      value1Edit->setText(QString::number(data1,'f',meta->_precision));
      unitsModified = false;
    }
  }
}

void UnitsGui::value0Change(const QString &string)
{
  const float value = string.toFloat();
  meta->setValue(0,value);
  unitsModified = metaModified(notEqual(value, data0));
}

void UnitsGui::value1Change(const QString &string)
{
  const float value = string.toFloat();
  meta->setValue(1,value);
  unitsModified = metaModified(notEqual(value, data1));
}

void UnitsGui::setEnabled(bool enable)
{
  if (label) {
    label->setEnabled(enable);
  }
  if (label2) {
    label2->setEnabled(enable);
  }
  value0Edit->setEnabled(enable);
  value1Edit->setEnabled(enable);
}

void UnitsGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Float Pair
 *
 **********************************************************************/

FloatsGui::FloatsGui(
  const QString &heading0,
  const QString &heading1,
  FloatPairMeta *_meta,
  QGroupBox     *parent,
  int            decPlaces,
  bool           _showPair)
{
  Q_UNUSED(decPlaces)

  meta     = _meta;
  data0    = _meta->value(0);
  data1    = _meta->value(1);

  showPair = _showPair;

  QHBoxLayout *layout = new QHBoxLayout(parent);

  if (parent) {
    parent->setLayout(layout);
    setWhatsThis(lpubWT(WT_GUI_FLOATS, parent->title()));
  } else {
    setLayout(layout);
    bool noHeading = heading0.isEmpty() && heading1.isEmpty();
    const QString title = QString("%1 / %2").arg(heading0, heading1);
    setWhatsThis(lpubWT(WT_GUI_FLOATS, noHeading ? tr("X / Y") : title));
  }

  if (heading0 == "") {
    label0 = nullptr;
  } else {
    label0 = new QLabel(heading0,parent);
    layout->addWidget(label0);
  }

  value0Edit = new QLineEdit(parent);
  QDoubleValidator *value0Validator = new QDoubleValidator(value0Edit);
  value0Validator->setRange(0.0f, 1000.0f);
  value0Validator->setDecimals(_meta->_precision);
  value0Validator->setNotation(QDoubleValidator::StandardNotation);
  value0Edit->setValidator(value0Validator);
  value0Edit->setText(QString::number(_meta->value(0),'f',_meta->_precision));
  reset0Act = value0Edit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  reset0Act->setText(tr("Reset"));
  reset0Act->setEnabled(false);
  connect(value0Edit, SIGNAL( textEdited(const QString&)),
          this,         SLOT(enableReset(const QString&)));
  connect(reset0Act,  SIGNAL(    triggered()),
          this,         SLOT(lineEditReset()));
  connect(value0Edit, SIGNAL(  textEdited(const QString&)),
          this,         SLOT(value0Change(const QString&)));
  layout->addWidget(value0Edit);

  if (showPair) {
    if (heading1 == "") {
      label1 = nullptr;
    } else {
      label1 = new QLabel(heading1,parent);
      layout->addWidget(label1);
    }

    value1Edit = new QLineEdit(parent);
    QDoubleValidator *value1Validator = new QDoubleValidator(value1Edit);
    value1Validator->setRange(0.0f, 1000.0f);
    value1Validator->setDecimals(_meta->_precision);
    value1Validator->setNotation(QDoubleValidator::StandardNotation);
    value1Edit->setValidator(value1Validator);
    value1Edit->setText(QString::number(_meta->value(1),'f',_meta->_precision));
    reset1Act = value1Edit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
    reset1Act->setText(tr("Reset"));
    reset1Act->setEnabled(false);
    connect(value1Edit, SIGNAL( textEdited(const QString&)),
            this,         SLOT(enableReset(const QString&)));
    connect(reset1Act,  SIGNAL(    triggered()),
            this,         SLOT(lineEditReset()));
    connect(value1Edit, SIGNAL(  textEdited(const QString&)),
            this,         SLOT(value1Change(const QString&)));
    layout->addWidget(value1Edit);
  } else {
      QSpacerItem *hSpacer;
      hSpacer = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Fixed);
      layout->addSpacerItem(hSpacer);
  }

  floatsModified = false;
}

void FloatsGui::enableReset(const QString &displayText)
{
  const double value = displayText.toDouble();

  if (sender() == value0Edit)
    reset0Act->setEnabled(notEqual(value, data0));
  else
  if (sender() == value1Edit)
    reset1Act->setEnabled(notEqual(value, data1));
}

void FloatsGui::lineEditReset()
{
  if (sender() == reset0Act) {
    reset0Act->setEnabled(false);
    if (value0Edit) {
      value0Edit->setText(QString::number(data0,'f',meta->_precision));
      floatsModified = false;
    }
  }
  else
  if (sender() == reset1Act) {
    reset1Act->setEnabled(false);
    if (value1Edit) {
      value1Edit->setText(QString::number(data1,'f',meta->_precision));
      floatsModified = false;
    }
  }
}

void FloatsGui::value0Change(const QString &string)
{
  const float value = string.toFloat();
  meta->setValue(0,value);
  if (!showPair)
     meta->setValue(1,value);
  floatsModified = metaModified(notEqual(value, data0));
}

void FloatsGui::value1Change(const QString &string)
{
  const float value = string.toFloat();
  meta->setValue(1,value);
  floatsModified = metaModified(notEqual(value, data1));
}

void FloatsGui::setEnabled(bool enable)
{
  if (label0) {
    label0->setEnabled(enable);
  }
  if (label1) {
    label1->setEnabled(enable);
  }
  value0Edit->setEnabled(enable);
  if (value1Edit) {
    value1Edit->setEnabled(enable);
  }
}

void FloatsGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Integer Spin
 *
 **********************************************************************/

SpinGui::SpinGui(
  const QString &heading,
  IntMeta       *_meta,
  int            min,
  int            max,
  int            step,
  QGroupBox     *parent)
{
  meta = _meta;
  data = _meta->value();

  QHBoxLayout *layout = new QHBoxLayout(parent);

  if (parent) {
    parent->setLayout(layout);
    setWhatsThis(lpubWT(WT_GUI_SPIN, parent->title()));
  } else {
    setLayout(layout);
    setWhatsThis(lpubWT(WT_GUI_SPIN, heading.isEmpty() ? tr("Integer Number") : heading));
  }

  if (heading == "") {
    label = nullptr;
  } else {
    label = new QLabel(heading,parent);
    layout->addWidget(label);
  }

  spin = new QSpinBox(parent);
  spin->setRange(min,max);
  spin->setSingleStep(step);
  spin->setValue(data);
  connect(spin,SIGNAL(valueChanged(int)),
          this,SLOT  (valueChanged(int)));
  layout->addWidget(spin);

  button = new QPushButton(parent);
  button->setIcon(QIcon(":/resources/resetaction.png"));
  button->setIconSize(QSize(16,16));
  button->setStyleSheet("QPushButton { background-color: rgba(255,255,255,0);border: 0px; }");
  button->setToolTip(tr("Reset"));
  button->setEnabled(false);
  connect(spin,   SIGNAL(valueChanged(int)),
          this,     SLOT( enableReset(int)));
  connect(button, SIGNAL(  clicked(bool)),
          this,     SLOT(spinReset(bool)));
  layout->addWidget(button);

  QSpacerItem *hSpacer = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Fixed);
  layout->addSpacerItem(hSpacer);
}

void SpinGui::enableReset(int value)
{
  button->setEnabled(value != data);
}

void SpinGui::spinReset(bool)
{
  button->setEnabled(false);
  if (spin) {
    spin->setValue(data);
    spin->setFocus();
    modified = false;
  }
}

void SpinGui::valueChanged(int value)
{
  meta->setValue(value);
  modified = value != data;
}

void SpinGui::setEnabled(bool enable)
{
  if (label) {
    label->setEnabled(enable);
  }
  spin->setEnabled(enable);
}

void SpinGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Double Spin Box
 *
 **********************************************************************/

DoubleSpinGui::DoubleSpinGui(
  const QString &heading,
  FloatMeta     *_meta,
  float          min,
  float          max,
  float          step,
  QGroupBox     *parent)
{
  meta = _meta;
  data = meta->value();

  QHBoxLayout *layout = new QHBoxLayout(parent);

  if (parent) {
    parent->setLayout(layout);
    setWhatsThis(lpubWT(WT_GUI_DOUBLE_SPIN, parent->title()));
  } else {
    setLayout(layout);
    setWhatsThis(lpubWT(WT_GUI_DOUBLE_SPIN, heading.isEmpty() ? tr("Decimal Number") : heading));
  }

  if (heading == "") {
    label = nullptr;
  } else {
    label = new QLabel(heading,parent);
    layout->addWidget(label);
  }

  const int residual = data - (int)data;
  const int decimalSize = QString::number(residual).size();
  const int decimalPlaces = decimalSize < 3 ? 2 : decimalSize;

  spin = new QDoubleSpinBox(parent);
  spin->setRange(min,max);
  spin->setSingleStep(step);
  spin->setDecimals(decimalPlaces);
  spin->setValue(data);
  connect(spin,SIGNAL(valueChanged(double)),
          this,SLOT  (valueChanged(double)));
  layout->addWidget(spin);

  button = new QPushButton(parent);
  button->setIcon(QIcon(":/resources/resetaction.png"));
  button->setIconSize(QSize(16,16));
  button->setStyleSheet("QPushButton { background-color: rgba(255,255,255,0);border: 0px; }");
  button->setToolTip(tr("Reset"));
  button->setEnabled(false);
  connect(spin,   SIGNAL(valueChanged(double)),
          this,     SLOT( enableReset(double)));
  connect(button, SIGNAL(  clicked(bool)),
          this,     SLOT(spinReset(bool)));
  layout->addWidget(button);

  QSpacerItem *hSpacer = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Fixed);
  layout->addSpacerItem(hSpacer);
}

void DoubleSpinGui::enableReset(double value)
{
  button->setEnabled(notEqual(value, data));
}

void DoubleSpinGui::spinReset(bool)
{
  button->setEnabled(false);
  if (spin) {
    spin->setValue(data);
    spin->setFocus();
    modified = false;
  }
}

void DoubleSpinGui::valueChanged(double value)
{
  meta->setValue(value);
  modified = notEqual(value, data);
}

void DoubleSpinGui::setEnabled(bool enable)
{
  if (label) {
    label->setEnabled(enable);
  }
  spin->setEnabled(enable);
}

void DoubleSpinGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Number
 *
 **********************************************************************/

NumberGui::NumberGui(
  QString    title,
  NumberMeta *_meta,
  QGroupBox  *parent)
{
  meta        = _meta;
  metaValue   = *meta;

  QGridLayout *grid = new QGridLayout(parent);

  if (parent) {
      parent->setLayout(grid);
      parent->setWhatsThis(lpubWT(WT_GUI_NUMBER, parent->title()));
  } else {
      setWhatsThis(lpubWT(WT_GUI_NUMBER, title.isEmpty() ? tr("Number") : title));
      if (!title.isEmpty()) {
          QGridLayout *gridLayout = new QGridLayout(nullptr);
          setLayout(gridLayout);

          gbFormat = new QGroupBox(title,parent);
          parent = gbFormat;
          parent->setLayout(grid);
          gridLayout->addWidget(parent,0,0);
      } else {
          setLayout(grid);
      }
  }

  fontLabel = new QLabel(tr("Font"),parent);
  grid->addWidget(fontLabel,0,0);

  fontExample = new QLabel("1234",parent);
  QFont font;
  font.fromString(metaValue.font.valueFoo());
  fontExample->setFont(font);
  grid->addWidget(fontExample,0,1);

  fontButton = new QPushButton(tr("Change"),parent);
  connect(fontButton,SIGNAL(   clicked(bool)),
          this,        SLOT(browseFont(bool)));
  grid->addWidget(fontButton,0,2);

  colorLabel = new QLabel(tr("Color"),parent);
  grid->addWidget(colorLabel,1,0);

  colorExample = new QLabel(parent);
  colorExample->setFrameStyle(QFrame::Sunken|QFrame::Panel);
  QColor c = QColor(metaValue.color.value());
  QString styleSheet =
      QString("QLabel { background-color: rgb(%1, %2, %3); }").
      arg(c.red()).arg(c.green()).arg(c.blue());
  colorExample->setAutoFillBackground(true);
  colorExample->setStyleSheet(styleSheet);
  colorExample->setToolTip(tr("Hex RGB %1").arg(c.name(QColor::HexRgb).toUpper()));
  grid->addWidget(colorExample,1,1);

  colorButton = new QPushButton(tr("Change"));
  connect(colorButton,SIGNAL(    clicked(bool)),
          this,         SLOT(browseColor(bool)));
  grid->addWidget(colorButton,1,2);

  marginsLabel = new QLabel(tr("Margins"),parent);
  grid->addWidget(marginsLabel,2,0);

  value0Edit = new QLineEdit(parent);
  QDoubleValidator *value0Validator = new QDoubleValidator(value0Edit);
  value0Validator->setRange(0.0f, 100.0f);
  value0Validator->setDecimals(metaValue.margin._precision);
  value0Validator->setNotation(QDoubleValidator::StandardNotation);
  value0Edit->setValidator(value0Validator);
  value0Edit->setText(QString::number(metaValue.margin.value(0),'f',metaValue.margin._precision));
  reset0Act = value0Edit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  reset0Act->setText(tr("Reset"));
  reset0Act->setEnabled(false);
  connect(value0Edit, SIGNAL( textEdited(const QString&)),
          this,         SLOT(enableReset(const QString&)));
  connect(reset0Act,  SIGNAL(    triggered()),
          this,         SLOT(lineEditReset()));
  connect(value0Edit, SIGNAL(   textEdited(const QString&)),
          this,         SLOT(value0Changed(const QString&)));
  grid->addWidget(value0Edit,2,1);

  value1Edit = new QLineEdit(parent);
  QDoubleValidator *value1Validator = new QDoubleValidator(value1Edit);
  value1Validator->setRange(0.0f, 100.0f);
  value1Validator->setDecimals(metaValue.margin._precision);
  value1Validator->setNotation(QDoubleValidator::StandardNotation);
  value1Edit->setValidator(value1Validator);
  value1Edit->setText(QString::number(metaValue.margin.value(1),'f',metaValue.margin._precision));
  reset1Act = value1Edit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  reset1Act->setText(tr("Reset"));
  reset1Act->setEnabled(false);
  connect(value1Edit, SIGNAL( textEdited(const QString&)),
          this,         SLOT(enableReset(const QString&)));
  connect(reset1Act,  SIGNAL(    triggered()),
          this,         SLOT(lineEditReset()));
  connect(value1Edit, SIGNAL(   textEdited(const QString&)),
          this,         SLOT(value1Changed(const QString&)));
  grid->addWidget(value1Edit,2,2);

  fontModified    = false;
  colorModified   = false;
  marginsModified = false;
}

void NumberGui::enableReset(const QString &displayText)
{
  const double value = displayText.toDouble();

  if (sender() == value0Edit)
    reset0Act->setEnabled(notEqual(value, metaValue.margin.value(0)));
  else
  if (sender() == value1Edit)
    reset1Act->setEnabled(notEqual(value, metaValue.margin.value(1)));
}

void NumberGui::lineEditReset()
{
  if (sender() == reset0Act) {
    reset0Act->setEnabled(false);
    if (value0Edit) {
      value0Edit->setText(QString::number(metaValue.margin.value(0),'f',metaValue.margin._precision));
      marginsModified = false;
    }
  }
  else
  if (sender() == reset1Act) {
    reset1Act->setEnabled(false);
    if (value1Edit) {
      value1Edit->setText(QString::number(metaValue.margin.value(1),'f',metaValue.margin._precision));
      marginsModified = false;
    }
  }
}

void NumberGui::browseFont(bool)
{
  QFont font;
  QString fontName = meta->font.valueFoo();
  font.fromString(fontName);
  bool ok;
  font = QFontDialog::getFont(&ok,font);
  fontName = font.toString();

  if (ok) {
    meta->font.setValue(font.toString());
    fontExample->setFont(font);
    fontModified = metaModified(metaValue.font.valueFoo() != meta->font.valueFoo());
  }
}

void NumberGui::browseColor(bool)
{
  QColor qcolor = LDrawColor::color(meta->color.value());
  QColor newColor = QColorDialog::getColor(qcolor,this);
  if (newColor.isValid() && qcolor != newColor) {
      colorExample->setAutoFillBackground(true);
      QString styleSheet =
          QString("QLabel { background-color: rgb(%1, %2, %3); }")
          .arg(newColor.red()).arg(newColor.green()).arg(newColor.blue());
      colorExample->setStyleSheet(styleSheet);
      colorExample->setToolTip(tr("Hex RGB %1").arg(newColor.name(QColor::HexRgb).toUpper()));
      meta->color.setValue(newColor.name());
      colorModified = metaModified(metaValue.color.value() != meta->color.value());
    }
}

void NumberGui::value0Changed(const QString &string)
{
  const float value = string.toFloat();
  meta->margin.setValue(0,value);
  marginsModified = metaModified(notEqual(value, metaValue.margin.value(0)));
}

void NumberGui::value1Changed(const QString &string)
{
  const float value = string.toFloat();
  meta->margin.setValue(1,value);
  marginsModified = metaModified(notEqual(value, metaValue.margin.value(1)));
}

void NumberGui::enableTextFormatGroup(bool checked)
{
    gbFormat->setEnabled(checked);
}

void NumberGui::apply(
  QString &topLevelFile)
{
  if (modified) {
    MetaItem mi;
    if (fontModified)
      mi.setGlobalMeta(topLevelFile,&meta->font);
    if (colorModified)
      mi.setGlobalMeta(topLevelFile,&meta->color);
    if (marginsModified)
      mi.setGlobalMeta(topLevelFile,&meta->margin);
  }
}

/***********************************************************************
 *
 * Constraint
 *
 **********************************************************************/

ConstrainGui::ConstrainGui(
  const QString &heading,
  ConstrainMeta *_meta,
  QGroupBox     *parent)
{
  // these will be updated as changes are incurred
  meta = _meta;
  // store initial values, these will not be updated
  data = _meta->valueUnit();

  QHBoxLayout *layout;

  layout = new QHBoxLayout(parent);

  if (parent) {
    parent->setLayout(layout);
    parent->setWhatsThis(lpubWT(WT_GUI_CONSTRAIN, parent->title()));
  } else {
    setLayout(layout);
    setWhatsThis(lpubWT(WT_GUI_CONSTRAIN, heading.isEmpty() ? tr("Constrain") : heading));
  }

  if (heading != "") {
    headingLabel = new QLabel(heading,parent);
    layout->addWidget(headingLabel);
  } else {
    headingLabel = nullptr;
  }

  QString  string;

  ConstrainData constraint = meta->value();

  string = "";

  switch (constraint.type) {
    case ConstrainData::PliConstrainWidth:
      string = QString("%1") .arg(constraint.constraint.width, 4,'f',2);
    break;
    case ConstrainData::PliConstrainHeight:
      string = QString("%1") .arg(constraint.constraint.height, 4,'f',2);
    break;
    case ConstrainData::PliConstrainColumns:
      string = QString("%1") .arg(constraint.constraint.columns);
    break;
    default:
    break;
  }

  combo = new QComboBox(this);
  combo->addItem("Area");
  combo->addItem("Square");
  combo->addItem("Width");
  combo->addItem("Height");
  combo->addItem("Columns");
  combo->setCurrentIndex(int(constraint.type));
  connect(combo,SIGNAL(currentTextChanged(const QString&)),
          this,   SLOT(         typeChange(const QString&)));
  layout->addWidget(combo);

  /* Constraint */

  valueEdit = new QLineEdit(parent);
  valueValidator = new QDoubleValidator(valueEdit);
  valueValidator->setRange(0.0f, 1000.0f);
  valueValidator->setDecimals(4);
  valueValidator->setNotation(QDoubleValidator::StandardNotation);
  valueEdit->setValidator(valueValidator);
  valueEdit->setText(string);
  //valueEdit->setInputMask("009.99");
  connect(valueEdit,SIGNAL( textEdited(const QString&)),
          this,       SLOT(valueChange(const QString&)));
  layout->addWidget(valueEdit);
  enable();

  typeModified = false;
  constrainModified = false;
}

void ConstrainGui::typeChange(const QString &type)
{
  ConstrainData _data = meta->valueUnit();
  QString string = "";
  if (type == "Area") {
    _data.type = ConstrainData::PliConstrainArea;
  } else if (type == "Square") {
    _data.type = ConstrainData::PliConstrainSquare;
  } else if (type == "Width") {
    _data.type = ConstrainData::PliConstrainWidth;
    string = QString::number(_data.constraint.width,'f',2);
  } else if (type == "Height") {
    _data.type = ConstrainData::PliConstrainHeight;
    string = QString::number(_data.constraint.height,'f',2);
  } else {
    _data.type = ConstrainData::PliConstrainColumns;
    string = QString::number(_data.constraint.columns);
  }
  valueEdit->setText(string);
  meta->setValueUnit(_data);
  typeModified = metaModified(_data.type != data.type);
  enable();
}

void ConstrainGui::valueChange(const QString &string)
{
  ConstrainData _data = meta->valueUnit();
  float newValue = string.toFloat();
  float oldValue = 0.0f;
  int type = combo->currentIndex();
  switch (type) {
  case ConstrainData::PliConstrainWidth:
    _data.constraint.width = newValue;
    oldValue = data.constraint.width;
    break;
  case ConstrainData::PliConstrainHeight:
    _data.constraint.height = newValue;
    oldValue = data.constraint.height;
    break;
  case ConstrainData::PliConstrainColumns:
    _data.constraint.columns = newValue;
    oldValue = data.constraint.columns;
    break;
  default:
    break;
  }
  meta->setValueUnit(_data);
  constrainModified = metaModified(notEqual(newValue, oldValue));
}

void ConstrainGui::setEnabled(bool enable)
{
  if (headingLabel) {
    headingLabel->setEnabled(enable);
  }
  combo->setEnabled(enable);
  valueEdit->setEnabled(enable);
}

void ConstrainGui::enable()
{
  ConstrainData _data = meta->valueUnit();
  switch (_data.type) {
  case ConstrainData::PliConstrainArea:
  case ConstrainData::PliConstrainSquare:
    valueEdit->setValidator(nullptr);
    valueEdit->setText(tr("Auto"));
    valueEdit->setDisabled(true);
    break;
  default:
    valueEdit->setValidator(valueValidator);
    valueEdit->setEnabled(true);
    break;
  }
}

void ConstrainGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * RendererParams
 *
 **********************************************************************/

RendererParamsGui::RendererParamsGui(const QString &heading,
                                     StringMeta    *_envVarsMeta,
                                     StringMeta    *_parmsMeta,
                                     StringMeta    *_parmsPovMeta,
                                     int            renderer,
                                     QGroupBox     *parent)
{
  parmsMeta = _parmsMeta;
  defaultParams = parmsMeta->value();

  useLDViewPovGen = renderer == RENDERER_POVRAY && !Preferences::useNativePovGenerator;
  if (useLDViewPovGen) {
    parmsPovMeta = _parmsPovMeta;
    defaultPovParams = parmsPovMeta->value();
  }

  envVarsMeta = _envVarsMeta;
  defaultEnvVars = _envVarsMeta->value();

  QGridLayout *grid;

  grid = new QGridLayout(parent);

  WT_Type wtType = renderer == RENDERER_BLENDER
    ? WT_GUI_RENDERER_PARAMETERS_BLENDER
    : _parmsPovMeta == nullptr   /* document renderer = empty string, image renderer = nullptr */
        ? WT_GUI_RENDERER_PARAMETERS_POVRAY
        : WT_GUI_RENDERER_PARAMETERS;
  if (parent) {
    parent->setLayout(grid);
    parent->setWhatsThis(lpubWT(wtType, parent->title()));
  } else {
    setLayout(grid);
    setWhatsThis(lpubWT(wtType, heading.isEmpty() ? tr("Renderer Parameters") : heading));
  }

  int row = 0;

  QLabel *label = new QLabel(tr("Parameters"), this);
  grid->addWidget(label,row,0);

  parameterEdit = new QLineEdit(parent);
  parameterEdit->setText(defaultParams);
  parameterEdit->setToolTip(tr("Set additional %1 image generation parameters").arg(rendererNames[renderer]));
  resetParameterEditAct = parameterEdit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  resetParameterEditAct->setText(tr("Reset"));
  resetParameterEditAct->setEnabled(false);
  connect(resetParameterEditAct, SIGNAL(    triggered()),
          this,                    SLOT(lineEditReset()));
  connect(parameterEdit,SIGNAL(textEdited(const QString&)),
          this,           SLOT(editChange(const QString&)));
  grid->addWidget(parameterEdit,row,1,1,3);

  if (useLDViewPovGen) {
      label = new QLabel(tr("POV Gen Parameters"), this);
      grid->addWidget(label,++row,0);

      parameterPovEdit = new QLineEdit(parent);
      parameterPovEdit->setText(defaultPovParams);
      parameterEdit->setToolTip(tr("Set addtional LDView parameters for POV file generation"));
      resetParameterPovEditAct = parameterPovEdit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
      resetParameterPovEditAct->setText(tr("Reset"));
      resetParameterPovEditAct->setEnabled(false);
      connect(resetParameterPovEditAct, SIGNAL(    triggered()),
              this,                       SLOT(lineEditReset()));
      connect(parameterPovEdit,SIGNAL(textEdited(const QString&)),
              this,              SLOT(editChange(const QString&)));
      grid->addWidget(parameterPovEdit,row,1,1,3);
  }

  label = new QLabel(tr("Environment"), this);
  grid->addWidget(label,++row,0);

  enviromentEdit = new QLineEdit(parent);
  enviromentEdit->setText(defaultEnvVars);
  enviromentEdit->setToolTip(tr("Set additional %1 image generation environment variables").arg(rendererNames[renderer]));
  resetEnviromentEditAct = enviromentEdit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  resetEnviromentEditAct->setText(tr("Reset"));
  resetEnviromentEditAct->setEnabled(false);
  connect(resetEnviromentEditAct, SIGNAL(    triggered()),
          this,                     SLOT(lineEditReset()));
  connect(enviromentEdit,SIGNAL(textEdited(const QString&)),
          this,            SLOT(editChange(const QString&)));
  grid->addWidget(enviromentEdit,row,1,1,3);

  parametersModified = false;
  parametersPovModified = false;
  enviromentModified = false;
}

void RendererParamsGui::editChange(const QString &string)
{
  if (sender() == parameterEdit) {
    parmsMeta->setValue(string);
    parametersModified = metaModified(string != defaultParams);
    resetParameterEditAct->setEnabled(parametersModified);
  } else
  if (sender() == parameterPovEdit) {
    parmsPovMeta->setValue(string);
    parametersPovModified = metaModified(string != defaultPovParams);
    resetParameterPovEditAct->setEnabled(parametersPovModified);
  } else
  if (sender() == enviromentEdit) {
    envVarsMeta->setValue(string);
    enviromentModified = metaModified(string != defaultEnvVars);
    resetEnviromentEditAct->setEnabled(enviromentModified);
  }
}

void RendererParamsGui::lineEditReset()
{
  if (sender() == resetParameterEditAct) {
    resetParameterEditAct->setEnabled(false);
    if (parameterEdit) {
      parameterEdit->setText(defaultParams);
      parametersPovModified = false;
    }
  } else
  if (sender() == resetParameterPovEditAct) {
    resetParameterPovEditAct->setEnabled(false);
    if (parameterPovEdit) {
      parameterPovEdit->setText(defaultPovParams);
      parametersModified = false;
    }
  } else
  if (sender() == resetEnviromentEditAct) {
    resetEnviromentEditAct->setEnabled(false);
    if (enviromentEdit) {
      enviromentEdit->setText(defaultEnvVars);
      enviromentModified = false;
    }
  }
}

void RendererParamsGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    if (parametersModified)
        mi.setGlobalMeta(modelName,parmsMeta);
    if (parametersPovModified)
        mi.setGlobalMeta(modelName,parmsPovMeta);
    if (enviromentModified)
        mi.setGlobalMeta(modelName,envVarsMeta);
  }
}

/***********************************************************************
 *
 * Stud Style
 *
 **********************************************************************/

#include "lc_edgecolordialog.h"

StudStyleGui::StudStyleGui(
  AutoEdgeColorMeta     *_autoEdgeMeta,
  StudStyleMeta         *_studStyleMeta,
  HighContrastColorMeta *_highContrastMeta,
  QGroupBox             *parent)
{
  autoEdgeMeta     = _autoEdgeMeta;
  autoEdgeValue    = *autoEdgeMeta;
  studStyleMeta    = _studStyleMeta;
  studStyleValue   = *studStyleMeta;
  highContrastMeta = _highContrastMeta;
  highContrastValue= *highContrastMeta;

  QGridLayout* gridLayout = new QGridLayout(parent);

  if (parent) {
      parent->setLayout(gridLayout);
      parent->setWhatsThis(lpubWT(WT_GUI_STUD_STYLE_AUTOMATE_EDGE_COLOR, parent->title()));
  } else {
      setLayout(gridLayout);
      setWhatsThis(lpubWT(WT_GUI_STUD_STYLE_AUTOMATE_EDGE_COLOR, tr("Stud Style and Automate Edge Color")));
  }

  autoEdgeCheckBox = new QCheckBox(parent);
  autoEdgeCheckBox->setText(tr("Automate edge colors"));
  autoEdgeCheckBox->setChecked(autoEdgeMeta->enable.value());

  autoEdgeButton = new QToolButton(parent);
  autoEdgeButton->setEnabled(autoEdgeCheckBox->isChecked());
  autoEdgeButton->setText(tr("Settings..."));

  QLabel *label = new QLabel(tr("Stud style"), parent);

  studStyleCombo = new QComboBox(parent);
  studStyleCombo->addItem("0 Plain");
  studStyleCombo->addItem("1 Thin Line Logo");
  studStyleCombo->addItem("2 Outline Top Logo");
  studStyleCombo->addItem("3 Sharp Top Logo");
  studStyleCombo->addItem("4 Rounded Top Logo");
  studStyleCombo->addItem("5 Flattened Top Logo");
  studStyleCombo->addItem("6 High Contrast Plain");
  studStyleCombo->addItem("7 High Contrast Thin Line");
  studStyleCombo->setCurrentIndex(int(studStyleMeta->value()));

  studStyleButton = new QToolButton(parent);
  studStyleButton->setEnabled(studStyleCombo->currentIndex() > 5);
  studStyleButton->setText(tr("Settings..."));

  gridLayout->addWidget(label, 0, 0);
  gridLayout->addWidget(studStyleCombo, 0, 1);
  gridLayout->addWidget(studStyleButton, 0, 2);

  gridLayout->addWidget(autoEdgeCheckBox, 1, 0,1, 2);
  gridLayout->addWidget(autoEdgeButton, 1, 2);

  connect(autoEdgeCheckBox,SIGNAL(toggled(bool)),
          this, SLOT  (autoEdgeCheckBoxChanged(bool)));

  connect(studStyleCombo,SIGNAL(currentIndexChanged(int)),
          this, SLOT  (studStyleComboChanged(int)));

  connect(autoEdgeCheckBox,SIGNAL(toggled(bool)),
          this, SLOT  (enableAutoEdgeButton()));
  connect(studStyleCombo,SIGNAL(currentIndexChanged(int)),
          this, SLOT  (enableStudStyleButton(int)));

  connect(autoEdgeButton,SIGNAL(clicked()),
          this, SLOT  (processToolButtonClick()));
  connect(studStyleButton,SIGNAL(clicked()),
          this, SLOT  (processToolButtonClick()));

  lightDarkIndexModified = false;
  studCylinderColorModified = false;
  studCylinderColorEnabledModified = false;
  partEdgeColorModified = false;
  partEdgeColorEnabledModified = false;
  blackEdgeColorModified = false;
  blackEdgeColorEnabledModified = false;
  darkEdgeColorModified = false;
  darkEdgeColorEnabledModified = false;

  studStyleModified = false;
  contrastModified = false;
  saturationModified = false;
  autoEdgeModified = false;
}

void StudStyleGui::enableStudStyleButton(int index)
{
  studStyleButton->setEnabled(index > 5);
}

void StudStyleGui::enableAutoEdgeButton()
{
  autoEdgeButton->setEnabled(autoEdgeCheckBox->isChecked());
}

void StudStyleGui::autoEdgeCheckBoxChanged(bool value)
{
  if (value != autoEdgeValue.enable.value()) {
    if (value && studStyleCombo->currentIndex() > 5) {
      if (QMessageBox::question(nullptr,
        QString("Automate Edge Colors Conflict"),
        QString("High contrast stud style settings are ignored when automate edge colors is enabled.<br>"
                "Do you want to continue ?"),
        QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok) {
        autoEdgeMeta->enable.setValue(value);
        autoEdgeModified = true;
        emit settingsChanged(modified);
      } else {
        autoEdgeCheckBox->setChecked(autoEdgeValue.enable.value());
      }
    }
  }
}

void StudStyleGui::studStyleComboChanged(int value)
{
  if (value != studStyleValue.value()) {
    if (value > 5 && autoEdgeCheckBox->isChecked() && QMessageBox::question(nullptr,
      QString("High Contrast Conflict"),
      QString("High contrast stud style settings are ignored when automate edge colors is enabled.<br>"
              "Would you like to disable automate edge colors ?"),
      QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok) {
      bool disable = false;
      autoEdgeCheckBox->setChecked(disable);
      autoEdgeMeta->enable.setValue(disable);
    }
    studStyleMeta->setValue(value);
    studStyleModified = true;
    emit settingsChanged(modified);
  }
}

void StudStyleGui::processToolButtonClick()
{
  lcAutomateEdgeColorDialog Dialog(this, sender() == studStyleButton);
  if (Dialog.exec() == QDialog::Accepted) {
    if (sender() == studStyleButton) {
      if ((lightDarkIndexModified = Dialog.mPartColorValueLDIndex != highContrastValue.lightDarkIndex.value())) {
        highContrastMeta->lightDarkIndex.setValue(Dialog.mPartColorValueLDIndex);
      }
      if ((studCylinderColorModified = Dialog.mStudCylinderColor != highContrastValue.studCylinderColor.value())) {
        highContrastMeta->studCylinderColor.setValue(Dialog.mStudCylinderColor);
      }
      if ((studCylinderColorEnabledModified = Dialog.mStudCylinderColorEnabled != highContrastValue.studCylinderColorEnabled.value())) {
        highContrastMeta->studCylinderColorEnabled.setValue(Dialog.mStudCylinderColorEnabled);
      }
      if ((partEdgeColorModified = Dialog.mPartEdgeColor != highContrastValue.partEdgeColor.value())) {
        highContrastMeta->partEdgeColor.setValue(Dialog.mPartEdgeColor);
      }
      if ((partEdgeColorEnabledModified = Dialog.mPartEdgeColorEnabled != highContrastValue.partEdgeColorEnabled.value())) {
        highContrastMeta->partEdgeColorEnabled.setValue(Dialog.mPartEdgeColorEnabled);
      }
      if ((blackEdgeColorModified = Dialog.mBlackEdgeColor != highContrastValue.blackEdgeColor.value())) {
        highContrastMeta->blackEdgeColor.setValue(Dialog.mBlackEdgeColor);
      }
      if ((blackEdgeColorEnabledModified = Dialog.mBlackEdgeColorEnabled != highContrastValue.blackEdgeColorEnabled.value())) {
        highContrastMeta->blackEdgeColorEnabled.setValue(Dialog.mBlackEdgeColorEnabled);
      }
      if ((darkEdgeColorModified = Dialog.mDarkEdgeColor != highContrastValue.darkEdgeColor.value())) {
        highContrastMeta->darkEdgeColor.setValue(Dialog.mDarkEdgeColor);
      }
      if ((darkEdgeColorEnabledModified = Dialog.mDarkEdgeColorEnabled != highContrastValue.darkEdgeColorEnabled.value())) {
        highContrastMeta->darkEdgeColorEnabled.setValue(Dialog.mDarkEdgeColorEnabled);
      }
    } else {
      if ((contrastModified = Dialog.mPartEdgeContrast != autoEdgeValue.contrast.value())) {
        autoEdgeMeta->contrast.setValue(Dialog.mPartEdgeContrast);
      }
      if ((saturationModified = Dialog.mPartColorValueLDIndex != autoEdgeValue.saturation.value())) {
        autoEdgeMeta->saturation.setValue(Dialog.mPartColorValueLDIndex);
      }
    }
    modified = (
      autoEdgeModified                 ||
      contrastModified                 ||
      saturationModified               ||
      studStyleModified                ||
      lightDarkIndexModified           ||
      studCylinderColorModified        ||
      studCylinderColorEnabledModified ||
      partEdgeColorModified            ||
      partEdgeColorEnabledModified     ||
      blackEdgeColorModified           ||
      blackEdgeColorEnabledModified    ||
      darkEdgeColorModified            ||
      darkEdgeColorEnabledModified);
    if (modified)
      emit settingsChanged(modified);
  }
}

void StudStyleGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    // Stud style
    if (studStyleModified)
        mi.setGlobalMeta(modelName,studStyleMeta);
    // Auto edge
    if (autoEdgeModified)
        mi.setGlobalMeta(modelName,&autoEdgeMeta->enable);
    if (contrastModified)
        mi.setGlobalMeta(modelName,&autoEdgeMeta->contrast);
    if (saturationModified)
        mi.setGlobalMeta(modelName,&autoEdgeMeta->saturation);
    // High contrast
    if (lightDarkIndexModified)
        mi.setGlobalMeta(modelName,&highContrastMeta->lightDarkIndex);
    if (studCylinderColorModified)
        mi.setGlobalMeta(modelName,&highContrastMeta->studCylinderColor);
    if (studCylinderColorEnabledModified)
        mi.setGlobalMeta(modelName,&highContrastMeta->studCylinderColorEnabled);
    if (partEdgeColorModified)
        mi.setGlobalMeta(modelName,&highContrastMeta->partEdgeColor);
    if (partEdgeColorEnabledModified)
        mi.setGlobalMeta(modelName,&highContrastMeta->partEdgeColorEnabled);
    if (blackEdgeColorModified)
        mi.setGlobalMeta(modelName,&highContrastMeta->blackEdgeColor);
    if (blackEdgeColorEnabledModified)
        mi.setGlobalMeta(modelName,&highContrastMeta->blackEdgeColorEnabled);
    if (darkEdgeColorModified)
        mi.setGlobalMeta(modelName,&highContrastMeta->darkEdgeColor);
    if (darkEdgeColorEnabledModified)
        mi.setGlobalMeta(modelName,&highContrastMeta->darkEdgeColorEnabled);
  }
}

/***********************************************************************
 *
 * PageAttribute
 *
 **********************************************************************/

const QString pageAttributeName[25] =
{
    "Page",                "Assembly",   "Step Group",  "Step Number",
    "Parts List",          "Callout",    "Page Number", "Title",
    "Model ID",            "Author",     "URL",         "Model Description",
    "Publish Description", "Copyright",  "Email",       "Disclaimer",
    "Parts",               "Plug",       "Category",    "Logo",
    "Cover Image",         "Plug Image", "Header",      "Footer",
    "BOM",
};

const int attributeKeysOk[16] =
{
    /*  0 Page           0,*/
    /*  0 Title */       fc | bc,
    /*  1 ModelName */   fc,
    /*  2 Author */      fc | bc | ph | pf,
    /*  3 Url */              bc | ph | pf,
    /*  4 ModelDesc */   fc,
    /*  5 PublishDesc */ fc,
    /*  6 Copyright */        bc | ph | pf,
    /*  7 Email */            bc | ph | pf,
    /*  8 Disclaimer */       bc,
    /*  9 Parts */       fc,
    /* 10 Plug */             bc,
    /* 11 Category */    fc | bc,
    /* 12 DocumentLogo */fc | bc,
    /* 13 CoverImage */  fc,
    /* 14 PlugImage */        bc
};

const QString sectionName[4] =
{
    "Front Cover",      //fc
    "Back Cover",       //bc
    "Page Header ",     //ph
    "Page Footer"       //pf
};

/***********************************************************************
 *
 * PageAttributeText
 *
 **********************************************************************/

PageAttributeTextGui::PageAttributeTextGui(
  PageAttributeTextMeta *_meta,
  QGroupBox  *parent)
{
  QGridLayout   *grid;
  QGridLayout   *gLayout;
  QHBoxLayout   *hLayout;

  meta = _meta;
  metaValue = *meta;

  selection = 0;

  grid = new QGridLayout(parent);

  if (parent) {
      parent->setLayout(grid);
      parent->setWhatsThis(lpubWT(WT_GUI_PAGE_ATTRIBUTE_TEXT, parent->title()));
  } else {
      setWhatsThis(lpubWT(WT_GUI_PAGE_ATTRIBUTE_TEXT, tr("Text Attribute")));
      setLayout(grid);
  }

  int attributeType;
  attributeType = metaValue.type - 7; // adjust PlacementType to match smaller PageAttributeType Enum
  int oks;
  oks = attributeKeysOk[attributeType];

  // Display
  if (parent) {
    parent->setCheckable(true);
    parent->setChecked(metaValue.display.value());
    connect(parent,SIGNAL(toggled(bool)),
            this, SLOT(  toggled(bool)));
  }

  // Section
  sectionLabel = new QLabel(tr("Section"),parent);
  grid->addWidget(sectionLabel,0,0);

  sectionCombo = new QComboBox(parent);
  int currentIndex = 0;

  for (int i = 0; i < 4; i++) {
//  logNotice() << "\n POPULATE PLACEMENT COMBO"
//              << "\n    Index: " << i   << " Bits: " << QString::number(i,2)
//              << "\n      Oks: " << oks << " Bits: " << QString::number(oks,2)
//              << "\n (1 << i): " << (1 << i) << " Bits: " << QString::number((1 << i),2)
//                 ;
    if (oks & (1 << i)) {
//        qDebug() << " MATCH: " << i << " " << oks << " " << (1 << i)
//                     ;
      sectionCombo->addItem(sectionName[i]);
      if (i == selection) {
        currentIndex = sectionCombo->count()-1;
      }
    }
  }
  sectionCombo->setCurrentIndex(currentIndex);

  grid->addWidget(sectionCombo,0,1);
  connect(sectionCombo,SIGNAL(currentIndexChanged(int)),SIGNAL(indexChanged(int)));
  connect(this,SIGNAL(indexChanged(int)),this,SLOT(newIndex(int)));

  // Page Text Placement
  gbPlacement = new QGroupBox(tr("%1 Placement").arg(pageAttributeName[metaValue.type]),parent);
  PlacementData placementData = metaValue.placement.value();
  const QString placementButtonText = tr("Change %1 Placement").arg(pageAttributeName[metaValue.type]);
  setWhatsThis(commonMenus.naturalLanguagePlacementWhatsThis(metaValue.type,placementData,placementButtonText));
  gLayout = new QGridLayout();
  gbPlacement->setLayout(gLayout);
  grid->addWidget(gbPlacement,0,2);

  placementButton = new QPushButton(tr("Change Placement"),parent);
  gLayout->addWidget(placementButton,0,0);
  connect(placementButton,SIGNAL(         clicked(bool)),
          this,             SLOT(placementChanged(bool)));

  // font
  fontLabel = new QLabel(tr("Font"),parent);
  grid->addWidget(fontLabel,1,0);

  fontExample = new QLabel("1234",parent);
  QFont font;
  font.fromString(metaValue.textFont.valueFoo());
  fontExample->setFont(font);
  grid->addWidget(fontExample,1,1);

  fontButton = new QPushButton(tr("Change"),parent);
  connect(fontButton,SIGNAL(   clicked(bool)),
          this,        SLOT(browseFont(bool)));
  grid->addWidget(fontButton,1,2);

  // color
  colorLabel = new QLabel(tr("Color"),parent);
  grid->addWidget(colorLabel,2,0);

  colorExample = new QLabel(parent);
  colorExample->setFrameStyle(QFrame::Sunken|QFrame::Panel);
  QColor c = QColor(metaValue.textColor.value());
  QString styleSheet =
      QString("QLabel { background-color: rgb(%1, %2, %3); }").
      arg(c.red()).arg(c.green()).arg(c.blue());
  colorExample->setAutoFillBackground(true);
  colorExample->setStyleSheet(styleSheet);
  colorExample->setToolTip(tr("Hex RGB %1").arg(c.name(QColor::HexRgb).toUpper()));
  grid->addWidget(colorExample,2,1);

  colorButton = new QPushButton(tr("Change"));
  connect(colorButton,SIGNAL(    clicked(bool)),
          this,         SLOT(browseColor(bool)));
  grid->addWidget(colorButton,2,2);

  // margins
  marginsLabel = new QLabel(tr("Margins"),parent);
  grid->addWidget(marginsLabel,3,0);

  value0Edit = new QLineEdit(parent);
  QDoubleValidator *value0Validator = new QDoubleValidator(value0Edit);
  value0Validator->setRange(0.0f, 100.0f);
  value0Validator->setDecimals(metaValue.margin._precision);
  value0Validator->setNotation(QDoubleValidator::StandardNotation);
  value0Edit->setValidator(value0Validator);
  value0Edit->setText(QString::number(metaValue.margin.value(0),'f',metaValue.margin._precision));
  reset0Act = value0Edit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  reset0Act->setText(tr("Reset"));
  reset0Act->setEnabled(false);
  connect(value0Edit, SIGNAL( textEdited(const QString&)),
          this,         SLOT(enableReset(const QString&)));
  connect(reset0Act,  SIGNAL(    triggered()),
          this,         SLOT(lineEditReset()));
  connect(value0Edit, SIGNAL(   textEdited(const QString&)),
          this,         SLOT(value0Changed(const QString&)));
  grid->addWidget(value0Edit,3,1);

  value1Edit = new QLineEdit(parent);
  QDoubleValidator *value1Validator = new QDoubleValidator(value1Edit);
  value1Validator->setRange(0.0f, 100.0f);
  value1Validator->setDecimals(metaValue.margin._precision);
  value1Validator->setNotation(QDoubleValidator::StandardNotation);
  value1Edit->setValidator(value1Validator);
  value1Edit->setText(QString::number(metaValue.margin.value(1),'f',metaValue.margin._precision));
  reset1Act = value1Edit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  reset1Act->setText(tr("Reset"));
  reset1Act->setEnabled(false);
  connect(value1Edit, SIGNAL( textEdited(const QString&)),
          this,         SLOT(enableReset(const QString&)));
  connect(reset1Act,  SIGNAL(    triggered()),
          this,         SLOT(lineEditReset()));
  connect(value1Edit, SIGNAL(   textEdited(const QString&)),
          this,         SLOT(value1Changed(const QString&)));
  grid->addWidget(value1Edit,3,2);

  //Content Dialogue
  gbContentEdit = new QGroupBox(tr("Content"),parent);
  hLayout = new QHBoxLayout();
  gbContentEdit->setLayout(hLayout);
  grid->addWidget(gbContentEdit,4,0,1,3);

  content = metaValue.content.value();

  if (metaValue.type == PagePlugType       ||
      metaValue.type == PageDisclaimerType ||
      metaValue.type == PageModelDescType  ||
      metaValue.type == PagePublishDescType) {
      contentTextEdit = new QTextEdit(content,parent);

      connect(contentTextEdit,SIGNAL(textChanged()),
            this,             SLOT(textEditChanged()));

      hLayout->addWidget(contentTextEdit);

  } else {
      contentLineEdit = new QLineEdit(content,parent);
      contentLineEdit->setClearButtonEnabled(true);

      connect(contentLineEdit,SIGNAL(    textChanged(const QString&)),
            this,               SLOT(lineEditChanged(const QString&)));

      hLayout->addWidget(contentLineEdit);
  }

  fontModified      = false;
  colorModified     = false;
  marginsModified  = false;
  marginsModified  = false;
  placementModified = false;
  displayModified   = false;
  editModified      = false;
}

void PageAttributeTextGui::browseFont(bool)
{
  QFont font;
  QString fontName = meta->textFont.valueFoo();
  font.fromString(fontName);
  bool ok;
  font = QFontDialog::getFont(&ok,font);
  fontName = font.toString();

  if (ok) {
    meta->textFont.setValue(font.toString());
    fontExample->setFont(font);
    fontModified = metaModified(meta->textFont.valueFoo() != metaValue.textFont.valueFoo());
  }
}

void PageAttributeTextGui::browseColor(bool)
{
  QColor qcolor = LDrawColor::color(meta->textColor.value());
  QColor newColor = QColorDialog::getColor(qcolor,this);
  if (newColor.isValid() && qcolor != newColor) {
      colorExample->setAutoFillBackground(true);
      QString styleSheet =
          QString("QLabel { background-color: rgb(%1, %2, %3); }")
          .arg(newColor.red()).arg(newColor.green()).arg(newColor.blue());
      colorExample->setStyleSheet(styleSheet);
      colorExample->setToolTip(tr("Hex RGB %1").arg(newColor.name(QColor::HexRgb).toUpper()));
      meta->textColor.setValue(newColor.name());
      colorModified = metaModified(meta->textColor.value() != metaValue.textColor.value());
    }
}

void PageAttributeTextGui::enableReset(const QString &displayText)
{
  const double value = displayText.toDouble();

  if (sender() == value0Edit)
    reset0Act->setEnabled(notEqual(value, metaValue.margin.value(0)));
  else
  if (sender() == value1Edit)
    reset1Act->setEnabled(notEqual(value, metaValue.margin.value(1)));
}

void PageAttributeTextGui::lineEditReset()
{
  if (sender() == reset0Act) {
    reset0Act->setEnabled(false);
    if (value0Edit) {
      value0Edit->setText(QString::number(metaValue.margin.value(0),'f',meta->margin._precision));
      marginsModified = false;
    }
  }
  else
  if (sender() == reset1Act) {
    reset1Act->setEnabled(false);
    if (value1Edit) {
      value1Edit->setText(QString::number(metaValue.margin.value(1),'f',meta->margin._precision));
      marginsModified = false;
    }
  }
}

void PageAttributeTextGui::value0Changed(const QString &string)
{
  const float value = string.toFloat();
  meta->margin.setValue(0,value);
  marginsModified = metaModified(notEqual(value, metaValue.margin.value(0)));
}

void PageAttributeTextGui::value1Changed(const QString &string)
{
  const float value = string.toFloat();
  meta->margin.setValue(1,value);
  marginsModified = metaModified(notEqual(value, metaValue.margin.value(1)));
}

void PageAttributeTextGui::textEditChanged()
{
  QStringList  textList = contentTextEdit->toPlainText().split("\n");
  meta->content.setValue(textList.join("\\n"));
  editModified = metaModified(meta->content.value() != metaValue.content.value());
}

void PageAttributeTextGui::lineEditChanged(const QString &value)
{
  QStringList  textList = value.split("\n");
  meta->content.setValue(textList.join("\\n"));
  editModified = metaModified(meta->content.value() != metaValue.content.value());
}

void PageAttributeTextGui::placementChanged(bool)
{
  PlacementData pd = meta->placement.value();
  bool ok = false;
  ok = PlacementDialog
       ::getPlacement(SingleStepType,meta->type,pd,pageAttributeName[meta->type]);
  if (ok) {
    meta->placement.setValue(pd);
    PlacementData pdv = metaValue.placement.value();
    placementModified = (
      pd.placement     != pdv.placement       ||
      pd.justification != pdv.justification   ||
      pd.preposition   != pdv.preposition     ||
      pd.relativeTo    != pdv.relativeTo      ||
      pd.rectPlacement != pdv.rectPlacement   ||
      notEqual(pd.offsets[0], pdv.offsets[0]) ||
      notEqual(pd.offsets[1], pdv.offsets[1]));
    metaModified(placementModified);
  }
}

void PageAttributeTextGui::toggled(bool toggled)
{
  meta->display.setValue(toggled);
  displayModified = metaModified(meta->display.value() != metaValue.display.value());
}

void PageAttributeTextGui::apply(
  QString &topLevelFile)
{
  if (modified) {
    MetaItem mi;
    if (fontModified) {
      mi.setGlobalMeta(topLevelFile,&meta->textFont);
    }
    if (colorModified) {
      mi.setGlobalMeta(topLevelFile,&meta->textColor);
    }
    if (marginsModified) {
      mi.setGlobalMeta(topLevelFile,&meta->margin);
    }
    if (placementModified) {
      mi.setGlobalMeta(topLevelFile,&meta->placement);
    }
    if (displayModified) {
      mi.setGlobalMeta(topLevelFile,&meta->display);
    }
    if (editModified) {
      mi.setGlobalMeta(topLevelFile,&meta->content);
    }
  }
}

/***********************************************************************
 *
 * PageAttributeImage
 *
 **********************************************************************/

 PageAttributeImageGui::PageAttributeImageGui(
  PageAttributeImageMeta *_meta,
  QGroupBox  *parent)
{
  QGridLayout   *grid;
  QGridLayout   *gLayout;
  QHBoxLayout   *hLayout;

  meta  = _meta;
  metaValue = *meta;

  selection = 0;

  grid = new QGridLayout(parent);

  if (parent) {
      parent->setLayout(grid);
      parent->setWhatsThis(lpubWT(WT_GUI_PAGE_ATTRIBUTE_IMAGE, parent->title()));
  } else {
      setWhatsThis(lpubWT(WT_GUI_PAGE_ATTRIBUTE_IMAGE, tr("Image")));
      setLayout(grid);
  }

  int attributeType;
  attributeType = metaValue.type - 7; // adjust PlacementType to match smaller PageAttributeType Enum
  int oks;
  oks = attributeKeysOk[attributeType];

  // Display
  if (parent) {
    parent->setCheckable(true);
    parent->setChecked(metaValue.display.value());
    connect(parent,SIGNAL(toggled(bool)),
            this, SLOT(  toggled(bool)));
  }

  // Section
  sectionLabel = new QLabel(tr("Section"),parent);
  grid->addWidget(sectionLabel,0,0);

  sectionCombo = new QComboBox(parent);
  int currentIndex = 0;

  for (int i = 0; i < 4; i++) {
//  logNotice() << "\n POPULATE PLACEMENT COMBO"
//              << "\n    Index: " << i   << " Bits: " << QString::number(i,2)
//              << "\n      Oks: " << oks << " Bits: " << QString::number(oks,2)
//              << "\n (1 << i): " << (1 << i) << " Bits: " << QString::number((1 << i),2)
//                 ;
    if (oks & (1 << i)) {
//        qDebug() << " MATCH: " << i << " " << oks << " " << (1 << i)
//                     ;
      sectionCombo->addItem(sectionName[i]);
      if (i == selection) {
        currentIndex = sectionCombo->count()-1;
      }
    }
  }
  sectionCombo->setCurrentIndex(currentIndex);
  grid->addWidget(sectionCombo,0,1);
  connect(sectionCombo,SIGNAL(currentIndexChanged(int)),SIGNAL(indexChanged(int)));
  connect(this,SIGNAL(indexChanged(int)),this,SLOT(selectionChanged(int)));

  // PLI Annotation Placement
  gbPlacement = new QGroupBox(tr("%1 Placement").arg(pageAttributeName[metaValue.type]),parent);
  PlacementData placementData = metaValue.placement.value();
  const QString placementButtonText = tr("Change %1 Placement").arg(pageAttributeName[metaValue.type]);
  setWhatsThis(commonMenus.naturalLanguagePlacementWhatsThis(metaValue.type,placementData,placementButtonText));
  gLayout = new QGridLayout();
  gbPlacement->setLayout(gLayout);
  grid->addWidget(gbPlacement,0,2);

  placementButton = new QPushButton(tr("Change Placement"),parent);
  gLayout->addWidget(placementButton,0,0);
  connect(placementButton,SIGNAL(         clicked(bool)),
          this,             SLOT(placementChanged(bool)));

  // margins
  marginsLabel = new QLabel(tr("Margins"),parent);
  grid->addWidget(marginsLabel,1,0);

  value0Edit = new QLineEdit(parent);
  QDoubleValidator *value0Validator = new QDoubleValidator(value0Edit);
  value0Validator->setRange(0.0f, 100.0f);
  value0Validator->setDecimals(metaValue.margin._precision);
  value0Validator->setNotation(QDoubleValidator::StandardNotation);
  value0Edit->setValidator(value0Validator);
  value0Edit->setText(QString::number(metaValue.margin.value(0),'f',metaValue.margin._precision));
  reset0Act = value0Edit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  reset0Act->setText(tr("Reset"));
  reset0Act->setEnabled(false);
  connect(value0Edit, SIGNAL(     textEdited(const QString&)),
          this,         SLOT(enableEditReset(const QString&)));
  connect(reset0Act,  SIGNAL(    triggered()),
          this,         SLOT(lineEditReset()));
  connect(value0Edit, SIGNAL(   textEdited(const QString&)),
          this,         SLOT(value0Changed(const QString&)));
  grid->addWidget(value0Edit,1,1);

  value1Edit = new QLineEdit(parent);
  QDoubleValidator *value1Validator = new QDoubleValidator(value1Edit);
  value1Validator->setRange(0.0f, 100.0f);
  value1Validator->setDecimals(metaValue.margin._precision);
  value1Validator->setNotation(QDoubleValidator::StandardNotation);
  value1Edit->setValidator(value1Validator);
  value1Edit->setText(QString::number(metaValue.margin.value(1),'f',metaValue.margin._precision));
  reset1Act = value1Edit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  reset1Act->setText(tr("Reset"));
  reset1Act->setEnabled(false);
  connect(value1Edit, SIGNAL(     textEdited(const QString&)),
          this,         SLOT(enableEditReset(const QString&)));
  connect(reset1Act,  SIGNAL(    triggered()),
          this,         SLOT(lineEditReset()));
  connect(value1Edit, SIGNAL(   textEdited(const QString&)),
          this,         SLOT(value1Changed(const QString&)));
  grid->addWidget(value1Edit,1,2);

  // Image
  image = metaValue.file.value();
  imageEdit = new QLineEdit(image,parent);
  imageEdit->setClearButtonEnabled(true);
  connect(imageEdit,SIGNAL( textEdited(const QString&)),
          this,       SLOT(imageChange(const QString&)));
  grid->addWidget(imageEdit,2,0,1,2);

  imageButton = new QPushButton(tr("Browse"),parent);
  connect(imageButton,SIGNAL(    clicked(bool)),
          this,         SLOT(browseImage(bool)));
  grid->addWidget(imageButton,2,2,1,1);

  //scale
  float scaleData = metaValue.picScale.value();
  const int residual = scaleData - (int)scaleData;
  const int decimalSize = QString::number(residual).size();
  const int decimalPlaces = decimalSize < 3 ? 2 : decimalSize;

  bool gbChecked = (metaValue.picScale.value() > 1.0 ||
                    metaValue.picScale.value() < 1.0) &&
                   (metaValue.fill.value() == Aspect);
  gbScale = new QGroupBox(tr("Scale"), parent);
  gbScale->setCheckable(true);
  gbScale->setChecked(gbChecked);
  gbScale->setEnabled(metaValue.fill.value() == Aspect);
  hLayout = new QHBoxLayout();
  gbScale->setLayout(hLayout);
  grid->addWidget(gbScale,3,0,1,3);

  scale = new QLabel(tr("Scale %1").arg(pageAttributeName[metaValue.type]),parent);
  hLayout->addWidget(scale);

  spin = new QDoubleSpinBox(parent);
  spin->setRange(metaValue.picScale._min,metaValue.picScale._max);
  spin->setSingleStep(0.1);
  spin->setDecimals(decimalPlaces);
  spin->setValue(scaleData);
  connect(spin,SIGNAL(valueChanged(double)),
          this,  SLOT(valueChanged(double)));
  hLayout->addWidget(spin);

  resetButton = new QPushButton(parent);
  resetButton->setIcon(QIcon(":/resources/resetaction.png"));
  resetButton->setIconSize(QSize(16,16));
  resetButton->setStyleSheet("QPushButton { background-color: rgba(255,255,255,0);border: 0px; }");
  resetButton->setToolTip(tr("Reset"));
  resetButton->setEnabled(false);
  connect(spin,        SIGNAL(   valueChanged(double)),
          this,          SLOT(enableSpinReset(double)));
  connect(resetButton, SIGNAL(  clicked(bool)),
          this,          SLOT(spinReset(bool)));
  hLayout->addWidget(resetButton);

  QSpacerItem *hSpacer = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Fixed);
  hLayout->addSpacerItem(hSpacer);

  connect(gbScale,SIGNAL(clicked(bool)),
          this,   SLOT(gbScaleClicked(bool)));

  // fill
  gbFill = new QGroupBox(tr("Image Fill"), parent);
  hLayout = new QHBoxLayout();
  gbFill->setLayout(hLayout);
  grid->addWidget(gbFill,4,0,1,3);

  aspectRadio = new QRadioButton(tr("Aspect"),gbFill);
  aspectRadio->setChecked(metaValue.fill.value() == Aspect);
  connect(aspectRadio,SIGNAL(clicked(bool)),
          this,        SLOT(  imageFill(bool)));
  hLayout->addWidget(aspectRadio);

  stretchRadio = new QRadioButton(tr("Stretch"),gbFill);
  stretchRadio->setChecked(metaValue.fill.value() == Stretch);
  connect(stretchRadio,SIGNAL(clicked(bool)),
          this,        SLOT(  imageFill(bool)));
  hLayout->addWidget(stretchRadio);
  tileRadio    = new QRadioButton(tr("Tile"),gbFill);
  tileRadio->setChecked(metaValue.fill.value() == Tile);
  connect(tileRadio,SIGNAL(clicked(bool)),
          this,     SLOT(  imageFill(bool)));
  hLayout->addWidget(tileRadio);

  fillModified      = false;
  imageModified     = false;
  marginsModified   = false;
  placementModified = false;
  displayModified   = false;
  scaleModified     = false;
}

void PageAttributeImageGui::enableEditReset(const QString &displayText)
{
  const double value = displayText.toDouble();

  if (sender() == value0Edit)
    reset0Act->setEnabled(notEqual(value, metaValue.margin.value(0)));
  else
  if (sender() == value1Edit)
    reset1Act->setEnabled(notEqual(value, metaValue.margin.value(1)));
}

void PageAttributeImageGui::enableSpinReset(double value)
{
  resetButton->setEnabled(notEqual(value, metaValue.picScale.value()));
}

void PageAttributeImageGui::lineEditReset()
{
  if (sender() == reset0Act) {
    reset0Act->setEnabled(false);
    if (value0Edit) {
      value0Edit->setText(QString::number(metaValue.margin.value(0),'f',metaValue.margin._precision));
      marginsModified = false;
    }
  }
  else
  if (sender() == reset1Act) {
    reset1Act->setEnabled(false);
    if (value1Edit) {
      value1Edit->setText(QString::number(metaValue.margin.value(1),'f',metaValue.margin._precision));
      marginsModified = false;
    }
  }
}

void PageAttributeImageGui::spinReset(bool)
{
  resetButton->setEnabled(false);
  if (spin) {
    spin->setValue(metaValue.picScale.value());
    spin->setFocus();
    scaleModified = false;
  }
}

void PageAttributeImageGui::imageFill(bool checked)
{
  if (sender() == stretchRadio) {
      meta->fill.setValue(Stretch);
      if (checked)
          gbScale->setEnabled(!checked);
  } else if (sender() == tileRadio) {
      meta->fill.setValue(Tile);
      if (checked)
          gbScale->setEnabled(!checked);
  } else { /*aspectRadio*/
      meta->fill.setValue(Aspect);
      if (checked)
          gbScale->setEnabled(checked);
  }
  fillModified = metaModified(meta->fill.value() != metaValue.fill.value());
}

void PageAttributeImageGui::imageChange(const QString &pic)
{
  meta->file.setValue(pic);
  imageModified = metaModified(meta->file.value() != metaValue.file.value());
}

void PageAttributeImageGui::browseImage(bool)
{
  QString image = meta->file.value();
  QString cwd = QDir::currentPath();
  QString filePath = QFileDialog::getOpenFileName(
    gui,
    tr("Choose Image File"),
    image,
    tr("Image Files (*.png *.jpg *.jpeg *.bmp)"));
  if (!filePath.isEmpty()) {
    if (filePath.startsWith(cwd))
      filePath = filePath.replace(cwd,".");
    image = filePath;
    imageEdit->setText(filePath);
    meta->file.setValue(image);
    imageModified = true;
  }
}

void PageAttributeImageGui::gbScaleClicked(bool checked)
{
  if (checked) {
      aspectRadio->setChecked(checked);
      stretchRadio->setChecked(!checked);
      tileRadio->setChecked(!checked);
  }
}

void PageAttributeImageGui::value0Changed(const QString &string)
{
  const float value = string.toFloat();
  meta->margin.setValue(0,value);
  marginsModified = metaModified(value != metaValue.margin.value(0));
}

void PageAttributeImageGui::value1Changed(const QString &string)
{
  const float value = string.toFloat();
  meta->margin.setValue(1,value);
  marginsModified = metaModified(value != metaValue.margin.value(1));
}

void PageAttributeImageGui::valueChanged(double value)
{
  meta->picScale.setValue(value);
  scaleModified = metaModified(notEqual(value, metaValue.picScale.value()));
}

void PageAttributeImageGui::placementChanged(bool)
{
  PlacementData pd = meta->placement.value();
  bool ok;
  ok = PlacementDialog
       ::getPlacement(SingleStepType,meta->type,pd,pageAttributeName[meta->type]);
  if (ok) {
    meta->placement.setValue(pd);
//      logInfo() << "\n PRE PLACEMENT META - "
//                << "\ngetPlacement INPUT DATA - "
//                << " \nPlacement: "                 << placementData.placement
//                << " \nJustification: "             << placementData.justification
//                << " \nPreposition: "               << placementData.preposition
//                << " \nRelativeTo: "                << placementData.relativeTo
//                << " \nRectPlacement: "             << placementData.rectPlacement
//                << " \nOffset[0]: "                 << placementData.offsets[0]
//                << " \nOffset[1]: "                 << placementData.offsets[1]
//                ;
    PlacementData pdv = metaValue.placement.value();
    placementModified = (
      pd.placement     != pdv.placement       ||
      pd.justification != pdv.justification   ||
      pd.preposition   != pdv.preposition     ||
      pd.relativeTo    != pdv.relativeTo      ||
      pd.rectPlacement != pdv.rectPlacement   ||
      notEqual(pd.offsets[0], pdv.offsets[0]) ||
      notEqual(pd.offsets[1], pdv.offsets[1]));
      metaModified(placementModified);
  }
}

void PageAttributeImageGui::toggled(bool toggled)
{
    meta->display.setValue(toggled);
    displayModified = metaModified(meta->display.value() != metaValue.display.value());
}

void PageAttributeImageGui::apply(QString &topLevelFile)
{
  if (modified) {
    MetaItem mi;
    if (imageModified) {
      mi.setGlobalMeta(topLevelFile,&meta->file);
    }
    if (scaleModified) {
      mi.setGlobalMeta(topLevelFile,&meta->picScale);
    }
    if (marginsModified) {
      mi.setGlobalMeta(topLevelFile,&meta->margin);
    }
    if (placementModified) {
        mi.setGlobalMeta(topLevelFile,&meta->placement);
    }
    if (displayModified) {
        mi.setGlobalMeta(topLevelFile,&meta->display);
    }
    if (fillModified) {
        mi.setGlobalMeta(topLevelFile,&meta->fill);
    }
  }
}

/***********************************************************************
 *
 * PageHeaderHeight
 *
 **********************************************************************/

HeaderFooterHeightGui::HeaderFooterHeightGui(
  const QString &heading,
  UnitsMeta     *_meta,
  QGroupBox     *parent)
{
  meta  = _meta;
  data1 = _meta->value(1);

  QHBoxLayout *layout = new QHBoxLayout(parent);

  if (parent) {
    parent->setLayout(layout);
    bool isHeader = parent->title().contains(QString("Header"), Qt::CaseInsensitive);
    WT_Type wtType = isHeader ? WT_GUI_HEADER_HEIGHT : WT_GUI_FOOTER_HEIGHT;
    parent->setWhatsThis(lpubWT(wtType, parent->title()));
  } else {
    setLayout(layout);
    bool isHeader = heading.isEmpty() ? true : heading.contains(QString("Header"), Qt::CaseInsensitive);
    WT_Type wtType = isHeader ? WT_GUI_HEADER_HEIGHT : WT_GUI_FOOTER_HEIGHT;
    setWhatsThis(lpubWT(wtType, heading.isEmpty() ? tr("Header Height") : heading));
  }

  if (heading != "") {
    label = new QLabel(heading,parent);
    layout->addWidget(label);
  } else {
    label = nullptr;
  }

  value0Edit = new QLineEdit(QString::number(_meta->value(0),'f',_meta->_precision),parent);
  value0Edit->setDisabled(true);

  layout->addWidget(value0Edit);

  value1Edit = new QLineEdit(parent);
  QDoubleValidator *value1Validator = new QDoubleValidator(value1Edit);
  value1Validator->setRange(0.0f, 1000.0f);
  value1Validator->setDecimals(_meta->_precision);
  value1Validator->setNotation(QDoubleValidator::StandardNotation);
  value1Edit->setValidator(value1Validator);
  value1Edit->setText(QString::number(data1,'f',_meta->_precision));
  reset1Act = value1Edit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  reset1Act->setText(tr("Reset"));
  reset1Act->setEnabled(false);
  connect(value1Edit, SIGNAL( textEdited(const QString&)),
          this,         SLOT(enableReset(const QString&)));
  connect(reset1Act,  SIGNAL(triggered()),
          this,         SLOT(lineEditReset()));
  connect(value1Edit, SIGNAL(  textEdited(const QString&)),
          this,         SLOT(value1Change(const QString&)));
  layout->addWidget(value1Edit);
}

void HeaderFooterHeightGui::enableReset(const QString &displayText)
{
  const double value = displayText.toDouble();
  reset1Act->setEnabled(notEqual(value, data1));
}

void HeaderFooterHeightGui::lineEditReset()
{
  reset1Act->setEnabled(false);
  if (value1Edit) {
    value1Edit->setText(QString::number(data1,'f',meta->_precision));
    modified = false;
  }
}

void HeaderFooterHeightGui::value1Change(const QString &string)
{
  const float value = string.toFloat();
  meta->setValue(1,value);
  modified = notEqual(value, data1);
}

void HeaderFooterHeightGui::setEnabled(bool enable)
{
  if (label)
    label->setEnabled(enable);
  value0Edit->setEnabled(enable);
}

void HeaderFooterHeightGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Fade Steps
 *
 **********************************************************************/

FadeStepsGui::FadeStepsGui(
        FadeStepsMeta *_meta,
        QGroupBox  *parent)
{
  meta = _meta;
  metaValue = *meta;

  QGridLayout *grid = new QGridLayout(parent);

  if (parent) {
      parent->setLayout(grid);
      parent->setWhatsThis(lpubWT(WT_GUI_FADE_STEPS, parent->title()));
  } else {
      setLayout(grid);
      setWhatsThis(lpubWT(WT_GUI_FADE_STEPS, tr("Fade Steps")));
  }

  // enable fade step row

  fadeCheck = new QCheckBox(tr("Enable Fade Steps"), parent);
  fadeCheck->setChecked(metaValue.enable.value());
  fadeCheck->setToolTip(tr("Turn on global fade previous steps or step parts."));

  connect(fadeCheck,SIGNAL(clicked(bool)),
                this, SLOT(valueChanged(bool)));

  grid->addWidget(fadeCheck,0,0,1,2);

  // LPub fade row
  bool obligatory = Preferences::preferredRenderer != RENDERER_NATIVE;
  QString const toolTip = obligatory
                              ? tr("LPub fade is obligatory when preferred renderer is %1.").arg(rendererNames[RENDERER_NATIVE])
                              : tr("LPub fade is optional when preferred renderer is %1.").arg(rendererNames[RENDERER_NATIVE]);

  lpubFadeCheck = new QCheckBox(tr("Use LPub Fade Steps"), parent);
  lpubFadeCheck->setChecked(metaValue.lpubFade.value() || obligatory);
  lpubFadeCheck->setEnabled(!obligatory);
  lpubFadeCheck->setToolTip(toolTip);

  connect(lpubFadeCheck,SIGNAL(clicked(bool)),
          this, SLOT(valueChanged(bool)));

  grid->addWidget(lpubFadeCheck,1,0,1,2);

  // fade setup row

  setupCheck = new QCheckBox(tr("Setup LPub Fade Steps"), parent);
  setupCheck->setChecked(metaValue.setup.value());
  setupCheck->setToolTip(tr("Setup LPub fade steps. Check to use LPub fade previous steps or step parts locally."));

  connect(setupCheck,SIGNAL(clicked(bool)),
          this, SLOT(valueChanged(bool)));

  grid->addWidget(setupCheck,2,0,1,2);

  // use color row

  useColorCheck = new QCheckBox(tr("Use Fade Color"), parent);
  useColorCheck->setToolTip(tr("Use specified fade color (versus part colour)"));
  useColorCheck->setChecked(metaValue.color.value().useColor);

  connect(useColorCheck,SIGNAL(clicked(bool)),
          this, SLOT(valueChanged(bool)));

  grid->addWidget(useColorCheck,3,0,1,2);

  // color button row

  int trans = Preferences::fadeStepsOpacity;
  int opacity = 100-trans;
  int alpha = LPUB3D_OPACITY_TO_ALPHA(opacity, 255);

  colorExample = new QLabel(parent);
  colorExample->setFixedSize(50,20);
  colorExample->setFrameStyle(QFrame::Sunken|QFrame::Panel);

  colorCombo = new QComboBox(parent);
  colorCombo->addItems(LDrawColor::names());

  QString cn = metaValue.color.value().color;
  QColor c = LDrawColor::color(cn);
  if (!c.isValid()) {
      emit gui->messageSig(LOG_WARNING, tr("Invalid color %1 loading Black")
                                            .arg(metaValue.color.value().color), true);
      cn = QLatin1String("Black");
      c = QColor(Qt::black);
  }
  int colorIndex = colorCombo->findText(cn);
  if (colorIndex == -1) {
      c = LDrawColor::color(cn);
      if (c.isValid()) {
          colorCombo->addItem(cn);
          colorIndex = colorCombo->findText(cn);
      }
  }
  if (c.isValid()) {
      c.setAlpha(alpha);
      QString styleSheet =
          QString("QLabel { background-color: rgba(%1, %2, %3, %4); }")
          .arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha());
      colorExample->setAutoFillBackground(true);
      colorExample->setStyleSheet(styleSheet);
      colorExample->setToolTip(tr("Hex ARGB %1").arg(c.name(QColor::HexArgb).toUpper()));
  }

  grid->addWidget(colorExample,4,0);

  colorCombo->setCurrentIndex(colorIndex);
  colorCombo->setDisabled(true);

  grid->addWidget(colorCombo,4,1);

  connect(colorCombo,SIGNAL(currentTextChanged(const QString&)),
                 this, SLOT(        colorChange(const QString&)));

  // fade opacity row

  QLabel *fadeOpacityLabel = new QLabel(tr("Fade Percent"));
  grid->addWidget(fadeOpacityLabel,5,0);

  fadeOpacitySlider = new QSlider(Qt::Horizontal, parent);
  fadeOpacitySlider->setToolTip(tr("Set the percent of fade, 0 is fully opaque, 100 is fully transparent."));
  fadeOpacitySlider->setRange(0,100);
  fadeOpacitySlider->setTickInterval(5);
  fadeOpacitySlider->setTickPosition(QSlider::TicksAbove);
  fadeOpacitySlider->setValue(metaValue.opacity.value());
  fadeOpacitySlider->setToolTip(tr("Fade Transparency %1%, Opacity %2%, Color Alpha %3/255")
                                    .arg(trans).arg(opacity).arg(alpha));

  connect(fadeOpacitySlider,SIGNAL(valueChanged(int)),
                         this,SLOT(valueChanged(int)));

  grid->addWidget(fadeOpacitySlider,5,1);

  // Color prefix

  colorPrefixLabel = new QLabel(tr("Color Prefix"),parent);
  colorPrefixLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  grid->addWidget(colorPrefixLabel,6,0);

  colorPrefixEdit = new QLineEdit(parent);
  colorPrefixEdit->setToolTip(tr("Set a 3-digit value that will be prepended to the LDConfig color ID."));
  QIntValidator *colorPrefixValidator = new QIntValidator(colorPrefixEdit);
  colorPrefixValidator->setRange(000, 999);
  colorPrefixEdit->setValidator(colorPrefixValidator);
  colorPrefixEdit->setText(QString::number(metaValue.colorPrefix.value().toInt()));
  colorPrefixEdit->setEnabled(metaValue.enable.value());
  resetColorPrefixEditAct = colorPrefixEdit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  resetColorPrefixEditAct->setText(tr("Reset"));
  resetColorPrefixEditAct->setEnabled(false);
  connect(colorPrefixEdit,SIGNAL(     textEdited(const QString&)),
          this,           SLOT(enableEditReset(const QString&)));
  connect(resetColorPrefixEditAct, SIGNAL(triggered()),
          this,           SLOT(  lineEditReset()));
  connect(colorPrefixEdit,SIGNAL(     textEdited(const QString&)),
          this,           SLOT(colorPrefixChange(const QString&)));

  // Color prefix spacer
  QHBoxLayout *hLayout = new QHBoxLayout(nullptr);
  grid->addLayout(hLayout,6,1);
  hLayout->addWidget(colorPrefixEdit);
  QSpacerItem *hColorPrefixSpacer;
  hColorPrefixSpacer = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Fixed);
  hLayout->addSpacerItem(hColorPrefixSpacer);

  emit fadeCheck->clicked(fadeCheck->isChecked());

  setupModified = false;
  lpubFadeModified = false;
  colorModified = false;
  fadeModified = false;
  useColorModified = false;
  opacityModified = false;
  colorPrefixModified = false;
}

void FadeStepsGui::colorChange(const QString &colorName)
{
  QColor fadeColor = LDrawColor::color(colorName);
  if (fadeColor.isValid()) {
    int trans = fadeOpacitySlider->value();
    int opacity = 100 - trans;
    int alpha = LPUB3D_OPACITY_TO_ALPHA(opacity, 255);
    fadeColor.setAlpha(alpha);
    QString styleSheet =
      QString("QLabel { background-color: rgba(%1, %2, %3, %4); }")
              .arg(fadeColor.red())
              .arg(fadeColor.green())
              .arg(fadeColor.blue())
              .arg(fadeColor.alpha());
    colorExample->setAutoFillBackground(true);
    colorExample->setStyleSheet(styleSheet);
    colorExample->setToolTip(tr("Hex ARGB %1").arg(fadeColor.name(QColor::HexArgb).toUpper()));
    fadeOpacitySlider->setToolTip(tr("Fade Transparency %1%, Opacity %2%, Color Alpha %3/255")
                                     .arg(trans).arg(opacity).arg(alpha));
    FadeColorData data = meta->color.value();
    data.color = colorName;
    data.useColor = true;
    meta->color.setValue(data);
    colorModified = metaModified(metaValue.color.value().color != colorName);
  }
}

void FadeStepsGui::valueChanged(bool checked)
{
  if (sender() == fadeCheck) {
    if (checked)
      setupCheck->setChecked(!checked);
    setupCheck->setEnabled(!checked);
    useColorCheck->setEnabled(checked);
    fadeOpacitySlider->setEnabled(checked);
    colorCombo->setEnabled(checked);
    colorPrefixEdit->setEnabled(checked);
    meta->enable.setValue(checked);
    fadeModified = metaModified(checked != metaValue.enable.value());
  } else if (sender() == setupCheck) {
    meta->setup.setValue(checked);
    setupModified = metaModified(checked != metaValue.setup.value());
  } else if (sender() == lpubFadeCheck) {
    meta->lpubFade.setValue(checked);
    lpubFadeModified = metaModified(checked != metaValue.lpubFade.value());
  } else if (sender() == useColorCheck) {
    FadeColorData data = meta->color.value();
    data.useColor = checked;
    meta->color.setValue(data);
    useColorModified = metaModified(checked != metaValue.color.value().useColor);
  }
}

void FadeStepsGui::valueChanged(int value)
{
if (sender() == fadeOpacitySlider) {
    QColor fadeColor = colorExample->palette().window().color();
    if (fadeColor.isValid()) {
      int trans = value;
      int opacity = 100 - trans;
      int alpha = LPUB3D_OPACITY_TO_ALPHA(opacity, 255);
      fadeColor.setAlpha(alpha);
      QString styleSheet =
          QString("QLabel { background-color: rgba(%1, %2, %3, %4); }")
              .arg(fadeColor.red())
              .arg(fadeColor.green())
              .arg(fadeColor.blue())
              .arg(fadeColor.alpha());
      colorExample->setAutoFillBackground(true);
      colorExample->setStyleSheet(styleSheet);
      colorExample->setToolTip(tr("Hex ARGB %1").arg(fadeColor.name(QColor::HexArgb).toUpper()));
      fadeOpacitySlider->setToolTip(tr("Fade Transparency %1%, Opacity %2%, Color Alpha %3/255")
                                        .arg(trans).arg(opacity).arg(alpha));
    }
    opacityModified = metaModified(meta->opacity.value() != value);
    meta->opacity.setValue(value);
  }
}

void FadeStepsGui::colorPrefixChange(const QString &colorPrefix)
{
  meta->colorPrefix.setValue(colorPrefix);
  colorPrefixModified = metaModified(colorPrefix != metaValue.colorPrefix.value());
}

void FadeStepsGui::enableEditReset(const QString &displayText)
{
  if (sender() == colorPrefixEdit)
    resetColorPrefixEditAct->setEnabled(displayText != metaValue.colorPrefix.value());
}

void FadeStepsGui::lineEditReset()
{
  if (sender() == resetColorPrefixEditAct) {
    resetColorPrefixEditAct->setEnabled(false);
    if (colorPrefixEdit) {
      colorPrefixEdit->setText(QString::number(metaValue.colorPrefix.value().toInt()));
      colorPrefixModified = false;
    }
  }
}

void FadeStepsGui::apply(
  QString &topLevelFile)
{
  if (modified) {
    MetaItem mi;
    if (fadeModified)
      mi.setGlobalMeta(topLevelFile,&meta->enable);
    if (setupModified)
      mi.setGlobalMeta(topLevelFile,&meta->setup);
    if (lpubFadeModified)
      mi.setGlobalMeta(topLevelFile,&meta->lpubFade);
    if (colorModified || useColorModified)
      mi.setGlobalMeta(topLevelFile,&meta->color);
    if (opacityModified)
      mi.setGlobalMeta(topLevelFile,&meta->opacity);
    if (colorPrefixModified) {
      Preferences::fadeStepsColourPrefix = meta->colorPrefix.value();
      mi.setGlobalMeta(topLevelFile,&meta->colorPrefix);
    }
  }
}

/***********************************************************************
 *
 * HighlightStep
 *
 **********************************************************************/

HighlightStepGui::HighlightStepGui(
        HighlightStepMeta *_meta,
        QGroupBox  *parent)
{
  meta = _meta;
  metaValue = *meta;

  QGridLayout *grid = new QGridLayout(parent);

  if (parent) {
      parent->setLayout(grid);
      parent->setWhatsThis(lpubWT(WT_GUI_HIGHLIGHT_CURRENT_STEP, parent->title()));
  } else {
      setLayout(grid);
      setWhatsThis(lpubWT(WT_GUI_HIGHLIGHT_CURRENT_STEP, tr("Highlight Step")));
  }

  // enable highlight row

  highlightCheck = new QCheckBox(tr("Enable Highlight Step"), parent);
  highlightCheck->setChecked(metaValue.enable.value());
  highlightCheck->setToolTip(tr("Turn on global highlight current step."));

  connect(highlightCheck,SIGNAL(clicked(bool)),
          this,          SLOT( valueChanged(bool)));

  grid->addWidget(highlightCheck,0,0,1,3);

  // LPub fade row

  bool obligatory = Preferences::preferredRenderer != RENDERER_NATIVE;
  QString const toolTip = obligatory
                              ? tr("LPub highlight is obligatory when preferred renderer is %1.").arg(rendererNames[RENDERER_NATIVE])
                              : tr("LPub highlight is optional when preferred renderer is %1.").arg(rendererNames[RENDERER_NATIVE]);

  lpubHighlightCheck = new QCheckBox(tr("Use LPub Highlight Step"), parent);
  lpubHighlightCheck->setChecked(metaValue.lpubHighlight.value() || obligatory);
  lpubHighlightCheck->setEnabled(!obligatory);
  lpubHighlightCheck->setToolTip(toolTip);

  connect(lpubHighlightCheck,SIGNAL(clicked(bool)),
          this, SLOT(valueChanged(bool)));

  grid->addWidget(lpubHighlightCheck,1,0,1,3);

  // fade setup row

  setupCheck = new QCheckBox(tr("Setup LPub Highlight Step"), parent);
  setupCheck->setChecked(metaValue.setup.value());
  setupCheck->setToolTip(tr("Setup LPub highlight step. Check to use LPub highlight current step or step parts locally."));

  connect(setupCheck,SIGNAL(clicked(bool)),
          this, SLOT(valueChanged(bool)));

  grid->addWidget(setupCheck,2,0,1,3);

  // colour button row

  colorExample = new QLabel(parent);
  colorExample->setFixedSize(50,20);
  colorExample->setFrameStyle(QFrame::Sunken|QFrame::Panel);
  QColor c = QColor(metaValue.color.value());
  if (c.isValid()) {
    QString styleSheet =
      QString("QLabel { background-color: rgba(%1, %2, %3, %4); }")
      .arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha());
    colorExample->setAutoFillBackground(true);
    colorExample->setStyleSheet(styleSheet);
    colorExample->setToolTip(tr("Hex ARGB %1").arg(c.name(QColor::HexArgb).toUpper()));
  }

  grid->addWidget(colorExample,3,0);

  colorButton = new QPushButton(parent);
  colorButton->setText(tr("Highlight Color..."));

  connect(colorButton,SIGNAL(clicked(bool)),
                 this, SLOT(colorChange(bool)));

  grid->addWidget(colorButton,3,1,1,2);

  /* ColorPrefix */

  colorPrefixLabel = new QLabel(tr("Color Prefix"),parent);
  colorPrefixLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  grid->addWidget(colorPrefixLabel,4,0);

  colorPrefixEdit = new QLineEdit(parent);
  colorPrefixEdit->setToolTip(tr("Set a 3-digit value that will be prepended to the LDConfig color ID."));
  QIntValidator *colorPrefixValidator = new QIntValidator(colorPrefixEdit);
  colorPrefixValidator->setRange(000, 999);
  colorPrefixEdit->setValidator(colorPrefixValidator);
  colorPrefixEdit->setText(QString::number(metaValue.colorPrefix.value().toInt()));
  colorPrefixEdit->setEnabled(metaValue.enable.value());
  resetColorPrefixEditAct = colorPrefixEdit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  resetColorPrefixEditAct->setText(tr("Reset"));
  resetColorPrefixEditAct->setEnabled(false);
  connect(colorPrefixEdit,SIGNAL(     textEdited(const QString&)),
          this,           SLOT(enableEditReset(const QString&)));
  connect(resetColorPrefixEditAct, SIGNAL(triggered()),
          this,           SLOT(  lineEditReset()));
  connect(colorPrefixEdit,SIGNAL(     textEdited(const QString&)),
          this,           SLOT(colorPrefixChange(const QString&)));
  grid->addWidget(colorPrefixEdit,4,1,1,2);

  // optional line width row

  if (Preferences::preferredRenderer == RENDERER_LDGLITE) {
    QLabel *lineWidthLabel = new QLabel(tr("Line Width"));
    grid->addWidget(lineWidthLabel,5,0);

    lineWidthSpin = new QSpinBox(parent);
    lineWidthSpin->setRange(0,10);
    lineWidthSpin->setValue(metaValue.lineWidth.value());

    connect(lineWidthSpin,SIGNAL(valueChanged(int)),
            this,         SLOT(  lineWidthChanged(int)));

    grid->addWidget(lineWidthSpin,5,1);

    button = new QPushButton(parent);
    button->setIcon(QIcon(":/resources/resetaction.png"));
    button->setIconSize(QSize(16,16));
    button->setStyleSheet("QPushButton { background-color: rgba(255,255,255,0);border: 0px; }");
    button->setToolTip(tr("Reset"));
    button->setEnabled(false);
    connect(lineWidthSpin, SIGNAL(valueChanged(int)),
            this,            SLOT( enableReset(int)));
    connect(button,        SIGNAL(  clicked(bool)),
            this,            SLOT(spinReset(bool)));
    grid->addWidget(button,5,2);
  }

  if (parent) {
    parent->setLayout(grid);
  } else {
    setLayout(grid);
  }

  emit highlightCheck->clicked(highlightCheck->isChecked());

  setupModified = false;
  lpubHighlightModified = false;
  colorModified = false;
  highlightModified = false;
  lineWidthModified = false;
  colorPrefixModified = false;
}

void HighlightStepGui::enableReset(int value)
{
  button->setEnabled(value != metaValue.lineWidth.value());
}

void HighlightStepGui::spinReset(bool)
{
  button->setEnabled(false);
  if (lineWidthSpin) {
    lineWidthSpin->setValue(metaValue.lineWidth.value());
    lineWidthSpin->setFocus();
    lineWidthModified = false;
  }
}

void HighlightStepGui::colorChange(bool)
{
  QColorDialog::ColorDialogOptions options = QColorDialog::ShowAlphaChannel;
  QColor oldHighlightColour = colorExample->palette().window().color();
  QColor highlightColour = QColorDialog::getColor(oldHighlightColour, this, tr("Highlight Colour"), options);
  if (highlightColour.isValid()) {
    QString styleSheet =
      QString("QLabel { background-color: rgba(%1, %2, %3, %4); }")
              .arg(highlightColour.red())
              .arg(highlightColour.green())
              .arg(highlightColour.blue())
              .arg(highlightColour.alpha());
    colorExample->setAutoFillBackground(true);
    colorExample->setStyleSheet(styleSheet);
    colorExample->setToolTip(tr("Hex ARGB %1").arg(highlightColour.name(QColor::HexArgb).toUpper()));
    meta->color.setValue(highlightColour.name(QColor::HexArgb).toUpper());
    colorModified = metaModified(oldHighlightColour != highlightColour);
  }
}

void HighlightStepGui::valueChanged(bool checked)
{
  if (sender() == highlightCheck) {
    if (checked)
      setupCheck->setChecked(!checked);
    setupCheck->setEnabled(!checked);
    colorButton->setEnabled(checked);
    colorPrefixEdit->setEnabled(checked);
    if (Preferences::preferredRenderer == RENDERER_LDGLITE)
      lineWidthSpin->setEnabled(checked);
    meta->enable.setValue(checked);
    highlightModified = metaModified(checked != metaValue.enable.value());
  } else if (sender() == setupCheck) {
    meta->setup.setValue(checked);
    setupModified = metaModified(checked != metaValue.setup.value());
  } else if (sender() == lpubHighlightCheck) {
    meta->lpubHighlight.setValue(checked);
    lpubHighlightModified = metaModified(checked != metaValue.lpubHighlight.value());
  }
}

void HighlightStepGui::lineWidthChanged(int value)
{
  meta->lineWidth.setValue(value);
  lineWidthModified = metaModified(value != metaValue.lineWidth.value());
}

void HighlightStepGui::colorPrefixChange(const QString &colorPrefix)
{
  meta->colorPrefix.setValue(colorPrefix);
  colorPrefixModified = metaModified(colorPrefix != metaValue.colorPrefix.value());
}

void HighlightStepGui::enableEditReset(const QString &displayText)
{
  if (sender() == colorPrefixEdit)
    resetColorPrefixEditAct->setEnabled(displayText != metaValue.colorPrefix.value());
}

void HighlightStepGui::lineEditReset()
{
  if (sender() == resetColorPrefixEditAct) {
    resetColorPrefixEditAct->setEnabled(false);
    if (colorPrefixEdit) {
      colorPrefixEdit->setText(QString::number(metaValue.colorPrefix.value().toInt()));
      colorPrefixModified = false;
    }
  }
}

void HighlightStepGui::apply(
  QString &topLevelFile)
{
  if (modified) {
    MetaItem mi;
    if (highlightModified)
      mi.setGlobalMeta(topLevelFile,&meta->enable);
    if (setupModified)
      mi.setGlobalMeta(topLevelFile,&meta->setup);
    if (lpubHighlightModified)
      mi.setGlobalMeta(topLevelFile,&meta->lpubHighlight);
    if (colorModified)
      mi.setGlobalMeta(topLevelFile,&meta->color);
    if (lineWidthModified)
      mi.setGlobalMeta(topLevelFile,&meta->lineWidth);
    if (colorPrefixModified) {
      Preferences::highlightStepColourPrefix = meta->colorPrefix.value();
      mi.setGlobalMeta(topLevelFile,&meta->colorPrefix);
    }
  }
}

/***********************************************************************
 *
 * Justify Step
 *
 **********************************************************************/

JustifyStepGui::JustifyStepGui(
        const QString &_label,
        JustifyStepMeta *_meta,
        QGroupBox       *parent)
{

  meta = _meta;
  data = _meta->value();

  QHBoxLayout *layout = new QHBoxLayout(parent);

  if (parent) {
    parent->setLayout(layout);
    parent->setWhatsThis(lpubWT(WT_GUI_STEP_JUSTIFICATION, parent->title()));
  } else {
    setLayout(layout);
    setWhatsThis(lpubWT(WT_GUI_STEP_JUSTIFICATION, _label.isEmpty() ? tr("Step Justification") : _label));
  }

  QLabel    *label;
  label = new QLabel(_label, parent);
  layout->addWidget(label);

  typeCombo = new QComboBox(parent);
  typeCombo->addItem(tr("Center"));
  typeCombo->addItem(tr("Center Horizontal"));
  typeCombo->addItem(tr("Center Vertical"));
  typeCombo->addItem(tr("Left (Default)"));
  typeCombo->setCurrentIndex(int(data.type));
  connect(typeCombo,SIGNAL(currentIndexChanged(int)),
          this,       SLOT(        typeChanged(int)));
  layout->addWidget(typeCombo);

  label = new QLabel(tr("Spacing"),parent);
  layout->addWidget(label);

  const int residual = data.spacing - (int)data.spacing;
  const int decimalSize = QString::number(residual).size();
  const int decimalPlaces = decimalSize < 3 ? 2 : decimalSize;

  spacingSpinBox = new QDoubleSpinBox(parent);
  spacingSpinBox->setRange(0.0,25.0);
  spacingSpinBox->setSingleStep(0.1);
  spacingSpinBox->setDecimals(decimalPlaces);
  spacingSpinBox->setValue(double(data.spacing));
  spacingSpinBox->setToolTip(tr("Set the spaceing, in %1, between items when step "
                                "is center justified").arg(units2name()));
  spacingSpinBox->setEnabled(data.type != JustifyLeft);
  connect(spacingSpinBox,SIGNAL(  valueChanged(double)),
          this,            SLOT(spacingChanged(double)));
  layout->addWidget(spacingSpinBox);

  button = new QPushButton(parent);
  button->setIcon(QIcon(":/resources/resetaction.png"));
  button->setIconSize(QSize(16,16));
  button->setStyleSheet("QPushButton { background-color: rgba(255,255,255,0);border: 0px; }");
  button->setToolTip(tr("Reset"));
  button->setEnabled(false);
  connect(spacingSpinBox,SIGNAL(valueChanged(double)),
          this,            SLOT( enableReset(double)));
  connect(button,        SIGNAL(  clicked(bool)),
          this,            SLOT(spinReset(bool)));
  layout->addWidget(button);

  QSpacerItem *hSpacer = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Fixed);
  layout->addSpacerItem(hSpacer);

  spacingModified = false;
  typeModified = false;
}

void JustifyStepGui::enableReset(double value)
{
  button->setEnabled(notEqual(value, data.spacing));
}

void JustifyStepGui::spinReset(bool)
{
  button->setEnabled(false);
  if (spacingSpinBox) {
    spacingSpinBox->setValue(data.spacing);
    spacingSpinBox->setFocus();
    spacingModified = false;
  }
}

void JustifyStepGui::typeChanged(int value)
{
  spacingSpinBox->setEnabled(value != JustifyLeft);

  JustifyStepData _data = meta->value();
  _data.type = JustifyStepEnc(value);
  meta->setValue(_data);
  typeModified = metaModified(_data.type != data.type);
}

void JustifyStepGui::spacingChanged(double value)
{
  JustifyStepData _data = meta->value();
  _data.spacing = value;
  meta->setValue(_data);
  spacingModified = metaModified(notEqual(value, data.spacing));
}

void JustifyStepGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * RotStep
 *
 **********************************************************************/

RotStepGui::RotStepGui(
        RotStepMeta *_meta,
        QGroupBox  *parent)
{

  meta = _meta;
  data = _meta->value();

  QGridLayout *grid = new QGridLayout();

  if (parent) {
      parent->setLayout(grid);
      parent->setWhatsThis(lpubWT(WT_GUI_STEP_ROTATION, parent->title()));
  } else {
      setLayout(grid);
      setWhatsThis(lpubWT(WT_GUI_STEP_ROTATION, tr("Step Rotation")));
  }

  QLabel    *rotStepLabel;
  rotStepLabel = new QLabel(tr("Rotation"), parent);
  grid->addWidget(rotStepLabel,0,0);

  auto decimalPlaces = [] (double value)
  {
      const int residual = value - (int)value;
      const int decimalSize = QString::number(residual).size();
      return decimalSize < 3 ? 2 : decimalSize;
  };

  qreal value = data.rots[0];
  rotStepSpinX = new QDoubleSpinBox(parent);
  rotStepSpinX->setRange(-360.0,360.0);
  rotStepSpinX->setSingleStep(1.0);
  rotStepSpinX->setDecimals(decimalPlaces(value));
  rotStepSpinX->setValue(value);
  connect(rotStepSpinX,SIGNAL(valueChanged(double)),
          this,        SLOT(rotStepChanged(double)));
  grid->addWidget(rotStepSpinX,0,1);

  button0 = new QPushButton(parent);
  button0->setIcon(QIcon(":/resources/resetaction.png"));
  button0->setIconSize(QSize(16,16));
  button0->setStyleSheet("QPushButton { background-color: rgba(255,255,255,0);border: 0px; }");
  button0->setToolTip(tr("Reset"));
  button0->setEnabled(false);
  connect(rotStepSpinX,SIGNAL(valueChanged(double)),
          this,          SLOT( enableReset( double)));
  connect(button0,     SIGNAL(  clicked(bool)),
          this,          SLOT(spinReset(bool)));
  grid->addWidget(button0,0,2);

  value = data.rots[1];
  rotStepSpinY = new QDoubleSpinBox(parent);
  rotStepSpinY->setRange(-360.0,360.0);
  rotStepSpinY->setSingleStep(1.0);
  rotStepSpinY->setDecimals(decimalPlaces(value));
  rotStepSpinY->setValue(value);
  connect(rotStepSpinY,SIGNAL(valueChanged(double)),
          this,        SLOT(rotStepChanged(double)));
  grid->addWidget(rotStepSpinY,0,3);

  button1 = new QPushButton(parent);
  button1->setIcon(QIcon(":/resources/resetaction.png"));
  button1->setIconSize(QSize(16,16));
  button1->setStyleSheet("QPushButton { background-color: rgba(255,255,255,0);border: 0px; }");
  button1->setToolTip(tr("Reset"));
  button1->setEnabled(false);
  connect(rotStepSpinY, SIGNAL(valueChanged(double)),
          this,           SLOT( enableReset(double)));
  connect(button1,      SIGNAL(  clicked(bool)),
          this,           SLOT(spinReset(bool)));
  grid->addWidget(button1,0,4);

  value = data.rots[2];
  rotStepSpinZ = new QDoubleSpinBox(parent);
  rotStepSpinZ->setRange(-360.0,360.0);
  rotStepSpinZ->setSingleStep(1.0);
  rotStepSpinZ->setDecimals(decimalPlaces(value));
  rotStepSpinZ->setValue(value);
  connect(rotStepSpinZ,SIGNAL( valueChanged(double)),
          this,          SLOT(rotStepChanged(double)));
  grid->addWidget(rotStepSpinZ,0,5);

  button2 = new QPushButton(parent);
  button2->setIcon(QIcon(":/resources/resetaction.png"));
  button2->setIconSize(QSize(16,16));
  button2->setStyleSheet("QPushButton { background-color: rgba(255,255,255,0);border: 0px; }");
  button2->setToolTip(tr("Reset"));
  button2->setEnabled(false);
  connect(rotStepSpinZ,SIGNAL(valueChanged(double)),
          this,          SLOT( enableReset(double)));
  connect(button2,     SIGNAL( clicked(bool)),
          this,          SLOT(spinReset(bool)));
  grid->addWidget(button2,0,6);

  QLabel    *typeLabel;
  typeLabel = new QLabel("Transform", parent);
  grid->addWidget(typeLabel,1,0);

  typeCombo = new QComboBox(parent);
  typeCombo->addItem("ABS");
  typeCombo->addItem("REL");
  typeCombo->addItem("ADD");
  typeCombo->setCurrentIndex(!data.type.isEmpty() ? typeCombo->findText(data.type) : 1);
  connect(typeCombo,SIGNAL(currentTextChanged(const QString&)),
          this,       SLOT(        typeChanged(const QString&)));
  grid->addWidget(typeCombo,1,1);

  spinXModified = false;
  spinYModified = false;
  spinZModified = false;
  typeModified = false;
}

void RotStepGui::reset()
{
  rotStepSpinX->setValue((qreal)data.rots[0]);
  button0->setEnabled(false);
  rotStepSpinY->setValue((qreal)data.rots[1]);
  button1->setEnabled(false);
  rotStepSpinZ->setValue((qreal)data.rots[2]);
  button2->setEnabled(false);
  rotStepSpinX->setFocus();
  typeCombo->setCurrentIndex(!data.type.isEmpty() ? typeCombo->findText(data.type) : 1);

  spinXModified = false;
  spinYModified = false;
  spinZModified = false;
  typeModified = false;
}

void RotStepGui::enableReset(double value)
{
  if (sender() == rotStepSpinX)
      button0->setEnabled(notEqual(value, data.rots[0]));
  else
  if (sender() == rotStepSpinY)
      button1->setEnabled(notEqual(value, data.rots[1]));
  else
  if (sender() == rotStepSpinZ)
      button2->setEnabled(notEqual(value, data.rots[2]));
}

void RotStepGui::spinReset(bool)
{
  if (sender() == button0) {
    button0->setEnabled(false);
    rotStepSpinX->setValue(data.rots[0]);
    rotStepSpinX->setFocus();
    spinXModified = false;
  } else
  if (sender() == button1) {
    button1->setEnabled(false);
    rotStepSpinY->setValue(data.rots[1]);
    rotStepSpinY->setFocus();
    spinYModified = false;
  } else
  if (sender() == button2) {
    button2->setEnabled(false);
    rotStepSpinZ->setValue(data.rots[2]);
    rotStepSpinZ->setFocus();
    spinZModified = false;
  }
}

void RotStepGui::rotStepChanged(double value)
{
  RotStepData _data = meta->value();
  if (sender() == rotStepSpinX) {
    _data.rots[0] = value;
    spinXModified = metaModified(notEqual(value, data.rots[0]));
  } else
  if (sender() == rotStepSpinY) {
    _data.rots[1] = value;
    spinYModified = metaModified(notEqual(value, data.rots[1]));
  } else { /* rotStepSpinZ */
    _data.rots[2] = value;
    spinZModified = metaModified(notEqual(value, data.rots[2]));
  }
  meta->setValue(_data);
  emit settingsChanged(modified);
}

void RotStepGui::typeChanged(const QString &value)
{
  RotStepData _data = meta->value();
  _data.type = value;
  meta->setValue(_data);
  typeModified = metaModified(data.type != value);
  emit settingsChanged(typeModified);
}

void RotStepGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Continuous Step Number
 *
 **********************************************************************/

ContStepNumGui::ContStepNumGui(
  QString const   &heading,
  ContStepNumMeta *_meta,
  QGroupBox       *parent)
{
  meta = _meta;
  value = _meta->value();

  QHBoxLayout *layout = new QHBoxLayout(parent);

  if (parent) {
    parent->setLayout(layout);
    parent->setWhatsThis(lpubWT(WT_GUI_CONTINUOUS_STEP_NUMBERS, parent->title()));
  } else {
    setLayout(layout);
    setWhatsThis(lpubWT(WT_GUI_CONTINUOUS_STEP_NUMBERS, heading.isEmpty() ? tr("Continuous Step Numbers") : heading));
  }

  check = new QCheckBox(heading,parent);
  check->setChecked(value);
  layout->addWidget(check);
  connect(check,SIGNAL(clicked(bool)),
          this, SLOT(  clicked(bool)));
}

void ContStepNumGui::clicked(bool checked)
{
  modified = value != checked;
  meta->setValue(checked);
}

void ContStepNumGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Count Instance
 *
 **********************************************************************/

CountInstanceGui::CountInstanceGui(
  CountInstanceMeta *_meta,
  QGroupBox         *parent)
{
  meta = _meta;
  value = _meta->value();

  QGridLayout *grid = new QGridLayout(parent);

  if (parent) {
    parent->setLayout(grid);
    parent->setWhatsThis(lpubWT(WT_GUI_CONSOLIDATE_SUBMODEL_INSTANCE_COUNT, parent->title()));
  } else {
    setLayout(grid);
    setWhatsThis(lpubWT(WT_GUI_CONSOLIDATE_SUBMODEL_INSTANCE_COUNT, tr("Submodel Instances")));
  }

  countCheck = new QCheckBox(tr("Consolidate Submodel Instances"), parent);
  countCheck->setChecked(value);
  countCheck->setToolTip(tr("Consolidate submodel instance count on first occurrence"));
  connect(countCheck,SIGNAL(clicked(bool)),
          this,        SLOT(valueChanged(bool)));

  grid->addWidget(countCheck,0,0,1,3);

  topRadio    = new QRadioButton(tr("At Top"),parent);
  topRadio->setChecked(value == CountAtTop);
  topRadio->setToolTip(tr("Consolidate instances and display count at last step page of first occurrence in the entire model file."));
  connect(topRadio,SIGNAL(clicked(bool)),
          this,     SLOT(  radioChanged(bool)));

  grid->addWidget(topRadio,1,0);

  modelRadio = new QRadioButton(tr("At Model (default)"),parent);
  modelRadio->setChecked(value > CountFalse && value < CountAtStep);
  modelRadio->setToolTip(tr("Consolidate instances and display count at last step page of first occurrence in the parent model."));
  connect(modelRadio,SIGNAL(clicked(bool)),
          this,      SLOT(  radioChanged(bool)));

  grid->addWidget(modelRadio,1,1);

  stepRadio    = new QRadioButton(tr("At Step"),parent);
  stepRadio->setChecked(value == CountAtStep);
  stepRadio->setToolTip(tr("Consolidate instances and display count at step page of first occurrence in the respective step."));
  connect(stepRadio,SIGNAL(clicked(bool)),
          this,     SLOT(  radioChanged(bool)));

  grid->addWidget(stepRadio,1,2);

  optionModified = false;
  consolidateModified = false;
}

void CountInstanceGui::radioChanged(bool)
{
  if (sender() == topRadio) {
    meta->setValue(CountAtTop);
  } else
  if (sender() == modelRadio) {
    meta->setValue(CountAtModel);
  } else
  if (sender() == stepRadio) {
    meta->setValue(CountAtStep);
  }
  optionModified = metaModified(meta->value() != value);
  emit settingsChanged(optionModified);
}

void CountInstanceGui::valueChanged(bool checked)
{
  int state = meta->value();
  int option = state;
  if (state == CountFalse && checked)
      option = CountTrue;
  meta->setValue(checked ? option : int(CountFalse));
  topRadio->setEnabled(checked);
  modelRadio->setEnabled(checked);
  stepRadio->setEnabled(checked);
  consolidateModified = metaModified(meta->value() != value);
  emit settingsChanged(consolidateModified);
}

void CountInstanceGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Build Modification
 *
 **********************************************************************/

BuildModEnabledGui::BuildModEnabledGui(
  QString const   &heading,
  BuildModEnabledMeta *_meta,
  QGroupBox       *parent)
{
  meta = _meta;
  value = _meta->value();

  QHBoxLayout *layout = new QHBoxLayout(parent);

  if (parent) {
    parent->setLayout(layout);
    parent->setWhatsThis(lpubWT(WT_GUI_BUILD_MODIFICATIONS, parent->title()));
  } else {
    setLayout(layout);
    setWhatsThis(lpubWT(WT_GUI_BUILD_MODIFICATIONS, heading.isEmpty() ? tr("Build Modifications") : heading));
  }

  check = new QCheckBox(heading,parent);
  check->setChecked(value);
  layout->addWidget(check);
  connect(check,SIGNAL(clicked(bool)),
          this, SLOT(  clicked(bool)));
}

void BuildModEnabledGui::clicked(bool checked)
{
  meta->setValue(checked);
  modified = value != checked;
}

void BuildModEnabledGui::applySettings()
{
    // Note: can move this to Project Globals - see HighlightStep Globals colorPrefix
    Preferences::buildModEnabled = meta->value();
    gui->enableVisualBuildModification();
    changeMessage = meta->value() ? tr("Build Modifications are Enabled") :
                                    tr("Build Modifications are Disabled");
    emit gui->messageSig(LOG_INFO, changeMessage);
}

void BuildModEnabledGui::apply(QString &modelName)
{
  if (modified) {
    applySettings();
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Final Model Enabled
 *
 **********************************************************************/

FinalModelEnabledGui::FinalModelEnabledGui(
  QString const   &heading,
  FinalModelEnabledMeta *_meta,
  QGroupBox       *parent)
{
  meta = _meta;
  value = _meta->value();

  QHBoxLayout *layout = new QHBoxLayout(parent);

  if (parent) {
    parent->setLayout(layout);
    parent->setWhatsThis(lpubWT(WT_GUI_FINAL_FADE_HIGHLIGHT_MODEL_STEP, parent->title()));
  } else {
    setLayout(layout);
    setWhatsThis(lpubWT(WT_GUI_FINAL_FADE_HIGHLIGHT_MODEL_STEP, heading.isEmpty() ? tr("Final Model Step") : heading));
  }

  check = new QCheckBox(heading,parent);
  check->setChecked(value);
  layout->addWidget(check);
  connect(check,SIGNAL(clicked(bool)),
          this, SLOT(  clicked(bool)));
}

void FinalModelEnabledGui::clicked(bool checked)
{
  meta->setValue(checked);
  modified = value != checked;
}

void FinalModelEnabledGui::applySettings()
{
    Preferences::finalModelEnabled = meta->value();
    changeMessage = meta->value() ? tr("Fade/Highlight final model step is Enabled") :
                                    tr("Fade/Highlight final model step is Disabled");
    emit gui->messageSig(LOG_INFO, changeMessage);
}

void FinalModelEnabledGui::apply(QString &modelName)
{
  if (modified) {
    applySettings();
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Cover Page Model View Enabled
 *
 **********************************************************************/

CoverPageViewEnabledGui::CoverPageViewEnabledGui(
  QString const   &heading,
  BoolMeta        *_meta,
  QGroupBox       *parent)
{
  meta = _meta;
  value = _meta->value();

  QHBoxLayout *layout = new QHBoxLayout(parent);

  if (parent) {
    parent->setLayout(layout);
    parent->setWhatsThis(lpubWT(WT_GUI_MODEL_VIEW_ON_COVER_PAGE, parent->title()));
  } else {
    setLayout(layout);
    setWhatsThis(lpubWT(WT_GUI_MODEL_VIEW_ON_COVER_PAGE, heading.isEmpty() ? tr("Cover Page Viewer Display") : heading));
  }

  check = new QCheckBox(heading,parent);
  check->setChecked(value);
  layout->addWidget(check);
  connect(check,SIGNAL(clicked(bool)),
          this, SLOT(  clicked(bool)));
}

void CoverPageViewEnabledGui::clicked(bool checked)
{
  meta->setValue(checked);
  modified = value != checked;
}

void CoverPageViewEnabledGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Load Unofficial Parts In Editor Enabled
 *
 **********************************************************************/

LoadUnoffPartsEnabledGui::LoadUnoffPartsEnabledGui(
  QString const     &heading,
  LoadUnoffPartsMeta *_meta,
  QGroupBox          *parent)
{
  meta = _meta;
  value = _meta->enabled.value();

  QHBoxLayout *layout = new QHBoxLayout(parent);

  if (parent) {
    parent->setLayout(layout);
    parent->setWhatsThis(lpubWT(WT_GUI_LOAD_UNOFFICIAL_PART_IN_EDITOR, parent->title()));
  } else {
    setLayout(layout);
    setWhatsThis(lpubWT(WT_GUI_LOAD_UNOFFICIAL_PART_IN_EDITOR, heading.isEmpty() ? tr("Unofficial Parts In Editor") : heading));
  }

  check = new QCheckBox(heading,parent);
  check->setEnabled(_meta->enableSetting.value());
  check->setChecked(value);
  layout->addWidget(check);
  connect(check,SIGNAL(clicked(bool)),
          this, SLOT( clicked(bool)));
}

void LoadUnoffPartsEnabledGui::clicked(bool checked)
{
  meta->enabled.setValue(checked);
  modified = value != checked;
}

void LoadUnoffPartsEnabledGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,&meta->enabled);
  }
}

/***********************************************************************
 *
 * Background
 *
 **********************************************************************/

BackgroundGui::BackgroundGui(
        BackgroundMeta *_meta,
        QGroupBox      *parent,
        bool            pictureSettings)
{
  QComboBox     *combo;
  QGridLayout   *grid;
  QHBoxLayout   *layout;

  meta = _meta;
  data = meta->value();

  QDialog *parentDialog = nullptr;

  grid = new QGridLayout(parent);

  WT_Type wtType = pictureSettings ? WT_GUI_BACKGROUND : WT_GUI_BACKGROUND_NO_IMAGE;

  if (parent) {
      parent->setLayout(grid);
      parent->setWhatsThis(lpubWT(wtType, parent->title()));
      parentDialog = dynamic_cast<QDialog*>(parent->parent());
  } else {
      setLayout(grid);
      setWhatsThis(lpubWT(wtType, tr("Background")));
  }

  rotateIcon = false;
  if (parentDialog) {
      //logDebug() << "Grandparent Window TITLE " + parentDialog->windowTitle();
      rotateIcon = parentDialog->windowTitle().contains("Rotate Icon");
  }

  QString picture, color;
  if (rotateIcon) {
      QDir extrasDir(Preferences::lpubDataPath + QDir::separator() + "extras");
      QFileInfo rotateIconFileInfo(extrasDir.absolutePath() + QDir::separator() + VER_ROTATE_ICON_FILE);
      if (rotateIconFileInfo.exists())
          picture = QString(rotateIconFileInfo.absoluteFilePath());
      else
          picture = QString(":/resources/rotate-icon.png");
  } else
  if (data.type == BackgroundData::BgImage) {
      if (!data.string.isEmpty())
          picture = data.string;
  } else {
      color = data.string;
  }

  combo = new QComboBox(parent);
  combo->addItem("None (transparent)");         // 0
  combo->addItem("Solid Color");                // 1
  combo->addItem("Gradient");                   // 2
  if (pictureSettings) {
     combo->addItem("Picture");                 // 3
     combo->addItem("Submodel Level Color");    // 4
     combo->setCurrentIndex(int(data.type));
  } else {
     combo->addItem("Submodel Level Color");    //3
     combo->setCurrentIndex(data.type == 4 ? 3 : int(data.type));
  }
  connect(combo,SIGNAL(currentTextChanged(const QString&)),
          this,   SLOT(         typeChange(const QString&)));
  grid->addWidget(combo,0,0);

  /* Color and Gradient button */

  colorButton = new QPushButton("Change Color...",parent);
  colorButton->setToolTip(tr("Change color using color picker"));
  connect(colorButton,SIGNAL(    clicked(bool)),
          this,         SLOT(browseColor(bool)));
  grid->addWidget(colorButton,0,1);

  gradientButton = new QPushButton("Change Gradient...",parent);
  gradientButton->setToolTip(tr("Change gradient using gradient dialog"));
  connect(gradientButton,SIGNAL(    clicked(bool)),
          this,            SLOT(setGradient(bool)));
  grid->addWidget(gradientButton,0,1);
  gradientButton->hide();

  /* Color label */

  colorExample = new QLabel(parent);
  colorExample->setFixedWidth(90);
  colorExample->setFrameStyle(QFrame::Sunken|QFrame::Panel);
  QColor c = color.isEmpty() ? QColor(Qt::black) : QColor(color);
  QString styleSheet =
      QString("QLabel { background-color: %1; inset grey;}")
      .arg(color.isEmpty() ? "transparent" :
           QString("rgb(%1, %2, %3)")
           .arg(c.red()).arg(c.green()).arg(c.blue()));
  colorExample->setAutoFillBackground(true);
  colorExample->setStyleSheet(styleSheet);
  colorExample->setToolTip(tr("Hex RGB %1").arg(c.name(QColor::HexRgb).toUpper()));
  grid->addWidget(colorExample,0,2);

  /* Image */

  pictureEdit = new QLineEdit(picture,parent);
  pictureEdit->setClearButtonEnabled(true);
  pictureEdit->setToolTip("Enter image path");
  connect(pictureEdit,SIGNAL( textEdited(const QString&)),
          this,         SLOT(imageChange(const QString&)));
  grid->addWidget(pictureEdit,1,0,1,2);

  pictureButton = new QPushButton("Browse",parent);
  connect(pictureButton,SIGNAL(    clicked(bool)),
          this,           SLOT(browseImage(bool)));
  grid->addWidget(pictureButton,1,2);

  /* Image Fill */

  fill = new QGroupBox("Image Fill",parent);

  layout = new QHBoxLayout();
  fill->setLayout(layout);
  grid->addWidget(fill,2,0,1,3);

  stretchRadio = new QRadioButton("Stretch Image",fill);
  connect(stretchRadio,SIGNAL(clicked(bool)),
          this,          SLOT(stretch(bool)));
  layout->addWidget(stretchRadio);
  tileRadio    = new QRadioButton("Tile Image",fill);
  connect(tileRadio,SIGNAL(clicked(bool)),
          this,       SLOT(   tile(bool)));
  layout->addWidget(tileRadio);

  if (!pictureSettings) {
      pictureEdit->hide();
      pictureButton->hide();
      fill->hide();
      stretchRadio->hide();
      tileRadio->hide();
  }

  typeModified = false;
  colorModified = false;
  gradientModified = false;
  imageModified = false;
  stretchModified = false;
  tileModified = false;

  enable();
}

void BackgroundGui::enable()
{
  BackgroundData background = meta->value();

  stretchRadio->setChecked(background.stretch);
  tileRadio->setChecked( !background.stretch);

  switch (background.type) {
    case BackgroundData::BgImage:
      colorButton->show();
      colorButton->setEnabled(false);
      gradientButton->hide();
      pictureButton->setEnabled(true);
      pictureEdit->setEnabled(true);
      fill->setEnabled(true);
    break;
    case BackgroundData::BgGradient:
      colorButton->hide();
      gradientButton->show();
      gradientButton->setEnabled(true);
      pictureButton->setEnabled(false);
      pictureEdit->setEnabled(false);
      fill->setEnabled(false);
    break;
    case BackgroundData::BgColor:
      colorButton->show();
      colorButton->setEnabled(true);
      gradientButton->hide();
      pictureButton->setEnabled(false);
      pictureEdit->setEnabled(false);
      fill->setEnabled(false);
    break;
    default:
      colorButton->show();
      colorButton->setEnabled(false);
      gradientButton->hide();
      pictureButton->setEnabled(false);
      pictureEdit->setEnabled(false);
      fill->setEnabled(false);
    break;
  }
}

void BackgroundGui::typeChange(const QString &type)
{
  BackgroundData backgroundData = meta->value();
  if (type == "None (transparent)") {
    backgroundData.type = BackgroundData::BgTransparent;
  } else if (type == "Picture") {
    backgroundData.type = BackgroundData::BgImage;
    if (data.type == BackgroundData::BgImage) {
      if (!data.string.isEmpty() && !rotateIcon) {
        pictureEdit->setText(data.string);
        backgroundData.string = data.string;
      }
    }
  } else if (type == "Solid Color") {
    backgroundData.type = BackgroundData::BgColor;
    if (data.type == BackgroundData::BgColor) {
      if (!data.string.isEmpty() && !rotateIcon)
        backgroundData.string = data.string;
    }
  } else if (type == "Gradient") {
    backgroundData.type = BackgroundData::BgGradient;
  } else {
    backgroundData.type = BackgroundData::BgSubmodelColor;
  }
  meta->setValue(backgroundData);
  typeModified = metaModified(backgroundData.type != data.type);
  enable();
}

void BackgroundGui::imageChange(const QString &picture)
{
  BackgroundData backgroundData = meta->value();
  backgroundData.string = picture;
  meta->setValue(backgroundData);
  imageModified = metaModified(backgroundData.string != data.string);
}

void BackgroundGui::setGradient(bool) {

  bool ok = true;

  BackgroundData backgroundData = meta->value();
  QPolygonF pts;
  QGradientStops stops;

  QSize gSize(backgroundData.gsize[0],backgroundData.gsize[1]);

  for (int i=0; i<backgroundData.gpoints.size(); i++)
    pts.append(backgroundData.gpoints.at(i));

  QGradient::CoordinateMode mode = QGradient::LogicalMode;
  switch (backgroundData.gmode) {
    case BackgroundData::LogicalMode:
      mode = QGradient::LogicalMode;
    break;
    case BackgroundData::StretchToDeviceMode:
      mode = QGradient::StretchToDeviceMode;
    break;
    case BackgroundData::ObjectBoundingMode:
      mode = QGradient::ObjectBoundingMode;
    break;
    }

  QGradient::Spread spread = QGradient::RepeatSpread;
  switch (backgroundData.gspread) {
    case BackgroundData::PadSpread:
      spread = QGradient::PadSpread;
    break;
    case BackgroundData::RepeatSpread:
      spread = QGradient::RepeatSpread;
    break;
    case BackgroundData::ReflectSpread:
      spread = QGradient::ReflectSpread;
    break;
    }

  QGradient *g = nullptr;
  switch (backgroundData.gtype) {
    case BackgroundData::LinearGradient:
      g = new QLinearGradient(pts.at(0), pts.at(1));
    break;
    case BackgroundData::RadialGradient:
      {
        QLineF line(pts[0], pts[1]);
        if (line.length() > 132) {
            line.setLength(132);
          }
        g = new QRadialGradient(line.p1(), qMin(gSize.width(), gSize.height()) / 3.0, line.p2());
      }
    break;
    case BackgroundData::ConicalGradient:
      {
        qreal angle = backgroundData.gangle;
        g = new QConicalGradient(pts.at(0), angle);
      }
    break;
    case BackgroundData::NoGradient:
    break;
    }

  for (int i=0; i<backgroundData.gstops.size(); ++i) {
      stops.append(backgroundData.gstops.at(i));
    }

  if (!g)
    g = new QLinearGradient(pts.at(0), pts.at(1));

  g->setStops(stops);
  g->setSpread(spread);
  g->setCoordinateMode(mode);

  GradientDialog *dialog = new GradientDialog(gSize,g);

  ok = dialog->exec() == QDialog::Accepted;
  if (ok) {

      QGradient bgGradient = dialog->getGradient();
      backgroundData.gstops.clear();
      backgroundData.gpoints.clear();

      //type and points
      if (bgGradient.type() == QGradient::LinearGradient) {
          backgroundData.gtype = BackgroundData::LinearGradient;
          QLinearGradient &newbgGradient = (QLinearGradient&)bgGradient;
          backgroundData.gpoints << newbgGradient.start() << newbgGradient.finalStop();
        } else if (bgGradient.type() == QGradient::RadialGradient) {
          backgroundData.gtype = BackgroundData::RadialGradient;
          QRadialGradient &newbgGradient = (QRadialGradient&)bgGradient;
          backgroundData.gpoints << newbgGradient.center() << newbgGradient.focalPoint();
        } else {
          backgroundData.gtype = BackgroundData::ConicalGradient;
          QConicalGradient &newbgGradient = (QConicalGradient&)bgGradient;
          QLineF l(newbgGradient.center(), QPointF(0, 0));
          l.setAngle(newbgGradient.angle());
          l.setLength(120);
          backgroundData.gpoints << newbgGradient.center() << l.p2();
          backgroundData.gangle = newbgGradient.angle();
        }
      //spread
      if (bgGradient.spread() == QGradient::PadSpread) {
          backgroundData.gspread = BackgroundData::PadSpread;
        } else if (bgGradient.spread() == QGradient::RepeatSpread) {
          backgroundData.gspread = BackgroundData::RepeatSpread;
        } else {
          backgroundData.gspread = BackgroundData::ReflectSpread;
        }
      //mode
      if (bgGradient.coordinateMode() == QGradient::LogicalMode) {
          backgroundData.gmode = BackgroundData::LogicalMode;
        } else if (bgGradient.coordinateMode() == QGradient::StretchToDeviceMode) {
          backgroundData.gmode = BackgroundData::StretchToDeviceMode;
        } else {
          backgroundData.gmode = BackgroundData::ObjectBoundingMode;
        }
      //stops
      for (int i=0; i<bgGradient.stops().size(); i++)
        backgroundData.gstops.append(bgGradient.stops().at(i));
    }

  meta->setValue(backgroundData);

  gradientModified = (
    backgroundData.gmode          != data.gmode         ||
    backgroundData.gspread        != data.gspread       ||
    backgroundData.gtype          != data.gtype         ||
    backgroundData.gstops.size()  != data.gstops.size() ||
    backgroundData.gpoints.size() != data.gpoints.size()||
    backgroundData.gsize[0]       != data.gsize[0]      ||
    backgroundData.gsize[1]       != data.gsize[1]      ||
    backgroundData.gangle         != data.gangle);
  metaModified(gradientModified);
}

void BackgroundGui::browseImage(bool)
{
  BackgroundData backgroundData = meta->value();
  QString cwd = QDir::currentPath();
  QString filePath = QFileDialog::getOpenFileName(
    gui,
    tr("Choose Picture File"),
    backgroundData.string,
    tr("Image Files (*.png *.jpg *.jpeg *.bmp)"));
  if (!filePath.isEmpty()) {
    if (filePath.startsWith(cwd))
      filePath = filePath.replace(cwd,".");
    backgroundData.string = filePath;
    pictureEdit->setText(filePath);
    meta->setValue(backgroundData);
    imageModified =
      (data.type == BackgroundData::BgImage || rotateIcon)
      ? backgroundData.string != data.string : true;
  }
}

void BackgroundGui::browseColor(bool)
{
  BackgroundData backgroundData = meta->value();
  QString color;
  if (!(data.type == BackgroundData::BgImage) && !rotateIcon)
      color = data.string;
  QColor qcolor = !color.isEmpty()
      ? LDrawColor::color(color)
      : colorExample->palette().window().color();
  QColor newColor = QColorDialog::getColor(qcolor,this);
  if (newColor.isValid() && qcolor != newColor) {
    color = newColor.name();
    backgroundData.string = newColor.name();
    QString styleSheet =
        QString("QLabel { background-color: rgb(%1, %2, %3); }").
        arg(newColor.red()).arg(newColor.green()).arg(newColor.blue());
    colorExample->setAutoFillBackground(true);
    colorExample->setStyleSheet(styleSheet);
    colorExample->setToolTip(tr("Hex RGB %1").arg(newColor.name(QColor::HexRgb).toUpper()));
    meta->setValue(backgroundData);
    colorModified = metaModified(backgroundData.string != data.string);
  }
}

void BackgroundGui::stretch(bool checked)
{
  BackgroundData backgroundData = meta->value();
  backgroundData.stretch = checked;
  meta->setValue(backgroundData);
  stretchModified = metaModified(backgroundData.stretch != data.stretch);
}
void BackgroundGui::tile(bool checked)
{
  BackgroundData backgroundData = meta->value();
  backgroundData.stretch = !checked;
  meta->setValue(backgroundData);
  tileModified = metaModified(backgroundData.stretch != data.stretch);
}

void BackgroundGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Border
 *
 **********************************************************************/

BorderGui::BorderGui(
  BorderMeta *_meta,
  QGroupBox *parent,
  bool rotateArrow,
  bool corners)
{
  meta = _meta;
  data = _meta->value();

  QGridLayout   *grid;

  grid = new QGridLayout(parent);

  if (parent) {
      parent->setLayout(grid);
      parent->setWhatsThis(lpubWT(WT_GUI_BORDER, parent->title()));
  } else {
      setLayout(grid);
      setWhatsThis(lpubWT(WT_GUI_BORDER, tr("Border")));
  }

  /* Arrows CheckBox */
  QString chkBoxHideArrowsText = data.hideTip ? "Rotate Icon Arrows Hidden" : "Hide Rotate Icon Arrows";
  hideArrowsChk = new QCheckBox(chkBoxHideArrowsText, parent);
  hideArrowsChk->setChecked(data.hideTip);
  hideArrowsChk->setToolTip(tr("Set checked when only icon image is desired."));
  connect(hideArrowsChk,SIGNAL(clicked(bool)),
          this,         SLOT(  checkChange(bool)));
  grid->addWidget(hideArrowsChk,0,0,1,3);

  /* Type Combo */

  typeCombo = new QComboBox(parent);
  typeCombo->addItem(tr("Borderless"));
  typeCombo->addItem(tr("Square Corners"));
  typeCombo->addItem(tr("Round Corners"));
  typeCombo->setCurrentIndex(int(data.type));
  connect(typeCombo,SIGNAL(currentTextChanged(const QString&)),
          this,       SLOT(         typeChange(const QString&)));
  grid->addWidget(typeCombo,0,0);

  /* Radius */

  spinLabel = new QLabel(tr("Radius"),parent);
  spinLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  grid->addWidget(spinLabel,0,1);

  spin = new QSpinBox(parent);
  spin->setRange(0,100);
  spin->setSingleStep(5);
  spin->setValue(int(data.radius));
  connect(spin,SIGNAL(valueChanged(int)),
          this,  SLOT(radiusChange(int)));
  grid->addWidget(spin,0,2);

  resetButton = new QPushButton(parent);
  resetButton->setIcon(QIcon(":/resources/resetaction.png"));
  resetButton->setIconSize(QSize(16,16));
  resetButton->setStyleSheet("QPushButton { background-color: rgba(255,255,255,0);border: 0px; }");
  resetButton->setToolTip(tr("Reset"));
  resetButton->setEnabled(false);
  connect(spin,        SIGNAL(   valueChanged(int)),
          this,          SLOT(enableSpinReset(int)));
  connect(resetButton, SIGNAL(  clicked(bool)),
          this,          SLOT(spinReset(bool)));
  grid->addWidget(resetButton,0,3,1,1,{Qt::AlignVCenter, Qt::AlignLeft});

  /* Line Combo */

  lineCombo = new QComboBox(parent);
  lineCombo->addItem(tr("No Line"));
  lineCombo->addItem(tr("Solid Line"));
  lineCombo->addItem(tr("Dash Line"));
  lineCombo->addItem(tr("Dotted Line"));
  lineCombo->addItem(tr("Dot-Dash Line"));
  lineCombo->addItem(tr("Dot-Dot-Dash Line"));
  lineCombo->setCurrentIndex(data.line);
  connect(lineCombo,SIGNAL(currentTextChanged(const QString&)),
          this,       SLOT(         lineChange(const QString&)));
  grid->addWidget(lineCombo,1,0);


  /* Thickness */

  thicknessLabel = new QLabel(tr("Width"),parent);
  thicknessLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  grid->addWidget(thicknessLabel,1,1);

  thicknessEdit = new QLineEdit(parent);
  QDoubleValidator *thicknessValidator = new QDoubleValidator(thicknessEdit);
  thicknessValidator->setRange(0.0f, 100.0f);
  thicknessValidator->setDecimals(4);
  thicknessValidator->setNotation(QDoubleValidator::StandardNotation);
  thicknessEdit->setValidator(thicknessValidator);
  thicknessEdit->setText(QString::number(data.thickness,'f',4));
  resetThicknessEditAct = thicknessEdit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  resetThicknessEditAct->setText(tr("Reset"));
  resetThicknessEditAct->setEnabled(false);
  connect(thicknessEdit,SIGNAL(     textEdited(const QString&)),
          this,           SLOT(enableEditReset(const QString&)));
  connect(resetThicknessEditAct, SIGNAL(triggered()),
          this,         SLOT(  lineEditReset()));
  connect(thicknessEdit,SIGNAL(     textEdited(const QString&)),
          this,           SLOT(thicknessChange(const QString&)));
  grid->addWidget(thicknessEdit,1,2,1,2);

  /* Color */

  QLabel *label = new QLabel(tr("Color"),parent);
  label->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  grid->addWidget(label,2,0);

  colorExample = new QLabel(parent);
  colorExample->setFrameStyle(QFrame::Sunken|QFrame::Panel);
  QColor c = QColor(data.color);
  QString styleSheet =
      QString("QLabel { background-color: rgb(%1, %2, %3); }").
      arg(c.red()).arg(c.green()).arg(c.blue());
  colorExample->setAutoFillBackground(true);
  colorExample->setStyleSheet(styleSheet);
  colorExample->setToolTip(tr("Hex RGB %1").arg(c.name(QColor::HexRgb).toUpper()));
  grid->addWidget(colorExample,2,1);

  colorButton = new QPushButton(tr("Change"),parent);
  connect(colorButton,SIGNAL(    clicked(bool)),
          this,         SLOT(browseColor(bool)));
  grid->addWidget(colorButton,2,2,1,2);

  /* Margins */

  label = new QLabel(tr("Margins"),parent);
  label->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  grid->addWidget(label,3,0);

  marginXEdit = new QLineEdit(parent);
  QDoubleValidator *marginXValidator = new QDoubleValidator(marginXEdit);
  marginXValidator->setRange(0.0f, 100.0f);
  marginXValidator->setDecimals(4);
  marginXValidator->setNotation(QDoubleValidator::StandardNotation);
  marginXEdit->setValidator(marginXValidator);
  marginXEdit->setText(QString::number(data.margin[0],'f',4));
  resetXEditAct = marginXEdit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  resetXEditAct->setText(tr("Reset"));
  resetXEditAct->setEnabled(false);
  connect(marginXEdit,  SIGNAL(     textEdited(const QString&)),
          this,           SLOT(enableEditReset(const QString&)));
  connect(resetXEditAct,SIGNAL(    triggered()),
          this,           SLOT(lineEditReset()));
  connect(marginXEdit,  SIGNAL(   textEdited(const QString&)),
          this,           SLOT(marginXChange(const QString&)));
  grid->addWidget(marginXEdit,3,1);

  marginYEdit = new QLineEdit(parent);
  QDoubleValidator *marginYValidator = new QDoubleValidator(marginYEdit);
  marginYValidator->setRange(0.0f, 100.0f);
  marginYValidator->setDecimals(4);
  marginYValidator->setNotation(QDoubleValidator::StandardNotation);
  marginYEdit->setValidator(marginYValidator);
  marginYEdit->setText(QString::number(data.margin[1],'f',4));
  resetYEditAct = marginYEdit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  resetYEditAct->setText(tr("Reset"));
  resetYEditAct->setEnabled(false);
  connect(marginYEdit,  SIGNAL(     textEdited(const QString&)),
          this,           SLOT(enableEditReset(const QString&)));
  connect(resetYEditAct,SIGNAL(    triggered()),
          this,           SLOT(lineEditReset()));
  connect(marginYEdit,  SIGNAL(   textEdited(const QString&)),
          this,           SLOT(marginYChange(const QString&)));
  grid->addWidget(marginYEdit,3,2,1,2);

  enable(rotateArrow);

  if (rotateArrow || !corners) {
      typeCombo->hide();
      spinLabel->hide();
      spin->hide();
      resetButton->hide();
      if (!corners)
          hideArrowsChk->hide();
  } else {
      hideArrowsChk->hide();
  }

  typeModified = false;
  hideTipModified = false;
  radiusModified = false;
  thicknessModified = false;
  lineModified = false;
  colorModified = false;
  marginsModified = false;
}

void BorderGui::enableSpinReset(int value)
{
  resetButton->setEnabled(value != (int)data.radius);
}

void BorderGui::enableEditReset(const QString &displayText)
{
  const double value = displayText.toDouble();

  if (sender() == marginXEdit)
    resetXEditAct->setEnabled(notEqual(value, data.margin[0]));
  else
  if (sender() == marginYEdit)
    resetYEditAct->setEnabled(notEqual(value, data.margin[1]));
  else
  if (sender() == thicknessEdit)
    resetThicknessEditAct->setEnabled(notEqual(value, data.thickness));
}

void BorderGui::spinReset(bool)
{
  resetButton->setEnabled(false);
  if (spin) {
    spin->setValue(int(data.radius));
    spin->setFocus();
    radiusModified = false;
  }
}

void BorderGui::lineEditReset()
{
  if (sender() == resetXEditAct) {
    resetXEditAct->setEnabled(false);
    if (marginXEdit) {
      marginXEdit->setText(QString::number(data.margin[0],'f',4));
      marginsModified = false;
    }
  }
  else
  if (sender() == resetYEditAct) {
    resetYEditAct->setEnabled(false);
    if (marginYEdit) {
      marginYEdit->setText(QString::number(data.margin[1],'f',4));
      marginsModified = false;
    }
  }
  else
  if (sender() == resetThicknessEditAct) {
    resetThicknessEditAct->setEnabled(false);
    if (thicknessEdit) {
      thicknessEdit->setText(QString::number(data.thickness,'f',4));
      thicknessModified = false;
    }
  }
}

void BorderGui::enable(bool rotateArrow)
{
  BorderData borderData = meta->value();

  switch (borderData.type) {
    case BorderData::BdrNone:
      lineCombo->setCurrentIndex(0);
      thicknessLabel->setEnabled(false);
      thicknessEdit->setEnabled(false);
      colorButton->setEnabled(false);
      spin->setEnabled(false);
      spinLabel->setEnabled(false);
      resetButton->setEnabled(false);
    break;
    case BorderData::BdrSquare:
      if (rotateArrow && hideArrowsChk->isChecked()) {
          lineCombo->setEnabled(false);
          thicknessLabel->setEnabled(false);
          thicknessEdit->setEnabled(false);
          colorButton->setEnabled(false);
          marginXEdit->setEnabled(false);
          marginYEdit->setEnabled(false);
      } else {
          lineCombo->setEnabled(true);
          thicknessLabel->setEnabled(true);
          thicknessEdit->setEnabled(true);
          colorButton->setEnabled(true);
          marginXEdit->setEnabled(true);
          marginYEdit->setEnabled(true);
      }
      spin->setEnabled(false);
      spinLabel->setEnabled(false);
      resetButton->setEnabled(false);
    break;
    default:
      lineCombo->setEnabled(true);
      thicknessLabel->setEnabled(true);
      thicknessEdit->setEnabled(true);
      colorButton->setEnabled(true);
      spin->setEnabled(true);
      spinLabel->setEnabled(true);
      resetButton->setEnabled(false);
    break;
  }

  switch (borderData.line) {
    case BorderData::BdrLnNone:
      thicknessLabel->setEnabled(false);
      thicknessEdit->setEnabled(false);
      colorButton->setEnabled(false);
      spin->setEnabled(false);
      spinLabel->setEnabled(false);
      resetButton->setEnabled(false);
    break;
    case BorderData::BdrLnSolid:
    case BorderData::BdrLnDash:
    case BorderData::BdrLnDot:
    case BorderData::BdrLnDashDot:
    case BorderData::BdrLnDashDotDot:
    default:
    break;
    }
}

void BorderGui::typeChange(const QString &type)
{
  BorderData borderData = meta->value();

  if (type == "Square Corners") {
    borderData.type = BorderData::BdrSquare;
  } else if (type == "Round Corners") {
    borderData.type = BorderData::BdrRound;
  } else {
    borderData.type = BorderData::BdrNone;
  }

  meta->setValue(borderData);
  typeModified = metaModified(borderData.type != data.type);
  enable();
}

void BorderGui::lineChange(const QString &line)
{
  BorderData borderData = meta->value();

  if (line == "Solid Line") {
    borderData.line = BorderData::BdrLnSolid;
  } else if (line == "Dash Line") {
    borderData.line = BorderData::BdrLnDash;
  } else if (line == "Dotted Line") {
    borderData.line = BorderData::BdrLnDot;
  } else if (line == "Dot-Dash Line") {
    borderData.line = BorderData::BdrLnDashDot;
  } else if (line == "Dot-Dot-Dash Line") {
    borderData.line = BorderData::BdrLnDashDotDot;
  } else {
    borderData.line = BorderData::BdrLnNone;
  }

  meta->setValue(borderData);
  lineModified = metaModified(borderData.line != data.line);
  enable();
}

void BorderGui::checkChange(bool)
{
  BorderData borderData = meta->value();

  borderData.hideTip = hideArrowsChk->isChecked();
  if (hideArrowsChk->isChecked())
    hideArrowsChk->setText("Rotate Icon Arrows Hidden");
  else
    hideArrowsChk->setText("Hide Rotate Icon Arrows");

  meta->setValue(borderData);
  enable(true); // Is Rotate Icon
  hideTipModified = metaModified(borderData.hideTip != data.hideTip);
}

void BorderGui::browseColor(bool)
{
  BorderData borderData = meta->value();

  QColor color = LDrawColor::color(borderData.color);
  QColor newColor = QColorDialog::getColor(color,this);
  if (newColor.isValid() && color != newColor) {
    borderData.color = newColor.name();
    meta->setValue(borderData);
    QString styleSheet =
        QString("QLabel { background-color: rgb(%1, %2, %3); }").
        arg(newColor.red()).arg(newColor.green()).arg(newColor.blue());
    colorExample->setAutoFillBackground(true);
    colorExample->setStyleSheet(styleSheet);
    colorExample->setToolTip(tr("Hex RGB %1").arg(newColor.name(QColor::HexRgb).toUpper()));
    colorModified = metaModified(borderData.color != data.color);
  }
}

void BorderGui::thicknessChange(const QString &thickness)
{
  float value = thickness.toFloat();
  BorderData _border = meta->value();
  _border.thickness = value;
  meta->setValue(_border);
  thicknessModified = metaModified(notEqual(value, data.thickness));
}
void BorderGui::radiusChange(int value)
{
  BorderData _border = meta->value();
  _border.radius = value;
  meta->setValue(_border);
  radiusModified = metaModified(_border.radius != data.radius);
}
void BorderGui::marginXChange(
  const QString &string)
{
  float value = string.toFloat();
  BorderData _border = meta->value();
  _border.margin[0] = value;
  meta->setValue(_border);
  marginsModified = metaModified(notEqual(value, data.margin[0]));
}
void BorderGui::marginYChange(
  const QString &string)
{
  float value = string.toFloat();
  BorderData _border = meta->value();
  _border.margin[1] = value;
  meta->setValue(_border);
  marginsModified = metaModified(notEqual(value, data.margin[1]));
}

void BorderGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Default Placement
 *
 **********************************************************************/

PlacementGui::PlacementGui(
        QString        _title,
        PlacementMeta *_meta,
        PlacementType  _type,
        QGroupBox     *parent)
{
    meta = _meta;
    data = _meta->value();
    title = _title;

    QHBoxLayout *hlayout = new QHBoxLayout(parent);

    const QString placementName = PlacementDialog::placementTypeName(_type);
    const QString placementButtonText = tr("Change %1 Placement").arg(placementName);

    if (parent) {
        parent->setLayout(hlayout);
        parent->setWhatsThis(commonMenus.naturalLanguagePlacementWhatsThis(_type,data,placementButtonText));
    } else {
        setLayout(hlayout);
        setWhatsThis(commonMenus.naturalLanguagePlacementWhatsThis(_type,data,placementButtonText));
    }

    placementLabel = new QLabel(tr("%1 placement").arg(placementName),parent);
    hlayout->addWidget(placementLabel);

    placementButton = new QPushButton(placementButtonText,parent);
    placementButton->setToolTip(tr("Set default %1 placement").arg(placementName));
    hlayout->addWidget(placementButton);
    connect(placementButton,SIGNAL(         clicked(bool)),
            this,             SLOT(placementChanged(bool)));

    modified = false;
}

void PlacementGui::placementChanged(bool)
{
  PlacementData _data = meta->value();
  bool ok = PlacementDialog
       ::getPlacement(SingleStepType,PartsListType,_data,title);
  if (ok) {
      meta->setValue(_data);
      modified = _data.placement     != data.placement      ||
                 _data.justification != data.justification  ||
                 _data.relativeTo    != data.relativeTo     ||
                 _data.preposition   != data.preposition    ||
                 _data.rectPlacement != data.rectPlacement  ||
                 notEqual(_data.offsets[0], data.offsets[0])||
                 notEqual(_data.offsets[1], data.offsets[1]);
  }
}

void PlacementGui::apply(QString &topLevelFile)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(topLevelFile,meta);
  }
}

/***********************************************************************
 *
 * Pointer Attributes
 *
 **********************************************************************/

 PointerAttribGui::PointerAttribGui(
  PointerAttribMeta *_meta,
  QGroupBox         *parent,
     bool          _isCallout)
{
  meta   = _meta;
  data   = meta->value();

  isBorder = _meta->value().attribType == PointerAttribData::Border;
  isLine   = _meta->value().attribType == PointerAttribData::Line;
  isTip    = _meta->value().attribType == PointerAttribData::Tip;

  QGridLayout   *grid;
  QLabel        *label;

  grid = new QGridLayout(parent);

  WT_Type wtType = isBorder ? WT_GUI_POINTER_BORDER : isLine ? WT_GUI_POINTER_LINE : WT_GUI_POINTER_TIP;

  if (parent) {
      parent->setLayout(grid);
      parent->setWhatsThis(lpubWT(wtType, parent->title()));
  } else {
      setWhatsThis(lpubWT(wtType, isBorder ? tr("Border") : isLine ? tr("Line") : tr("Tip")));
      setLayout(grid);
  }

  /*  Attributes */

  int index = 0;
  bool hideTip = false;
  QString title;
  QString size;
  QString color;
  QString units = tr("Units in %1").arg(Preferences::preferCentimeters ? tr("centimetres") : tr("inches"));

  if (!isTip) {
    if (isLine) {
        title = tr("Line");
        index = (int)data.lineData.line - 1;            // - 1  adjusts for removal of 'No-Line'
        size = QString::number(data.lineData.thickness,'f',4);
        color = data.lineData.color;
        hideTip = data.lineData.hideTip;
    } else if (isBorder) {
        title = tr("Border");
        index = (int)data.borderData.line - 1;
        size = QString::number(data.borderData.thickness,'f',4);
        color = data.borderData.color.isEmpty() ? data.lineData.color : data.borderData.color;
    }

    /* Line Combo */

    lineCombo = new QComboBox(parent);
    lineCombo->addItem(tr("Solid Line"));
    lineCombo->addItem(tr("Dash Line"));
    lineCombo->addItem(tr("Dotted Line"));
    lineCombo->addItem(tr("Dot-Dash Line"));
    lineCombo->addItem(tr("Dot-Dot-Dash Line"));
    lineCombo->setCurrentIndex(index); //
    connect(lineCombo,SIGNAL(currentTextChanged(const QString&)),
            this,       SLOT(         lineChange(const QString&)));
    grid->addWidget(lineCombo,0,0);
    grid->setColumnStretch(0,1);
    grid->setColumnStretch(1,1);

    /*  Width */

    label = new QLabel(tr("%1 Width").arg(title),parent);
    label->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    grid->addWidget(label,0,1);

    thicknessEdit = new QLineEdit(parent);
    QDoubleValidator *thicknessValidator = new QDoubleValidator(thicknessEdit);
    thicknessValidator->setRange(0.0f, 100.0f);
    thicknessValidator->setDecimals(4);
    thicknessValidator->setNotation(QDoubleValidator::StandardNotation);
    thicknessEdit->setValidator(thicknessValidator);
    thicknessEdit->setText(size);
    thicknessEdit->setToolTip(units);
    resetThicknessEditAct = thicknessEdit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
    resetThicknessEditAct->setText(tr("Reset"));
    resetThicknessEditAct->setEnabled(false);
    connect(thicknessEdit,SIGNAL(     textEdited(const QString&)),
            this,           SLOT(enableEditReset(const QString&)));
    connect(resetThicknessEditAct, SIGNAL(    triggered()),
            this,                    SLOT(lineEditReset()));
    connect(thicknessEdit,SIGNAL(textEdited(const QString&)),
            this,           SLOT(sizeChange(const QString&)));
    grid->addWidget(thicknessEdit,0,2);

    /*  Color */

    label = new QLabel(tr("%1 Color").arg(title),parent);
    label->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    grid->addWidget(label,1,0);

    colorExample = new QLabel(parent);
    colorExample->setFrameStyle(QFrame::Sunken|QFrame::Panel);
    QColor c = QColor(color);
    QString StyleSheet =
            QString("QLabel { background-color: %1; }")
            .arg(color.isEmpty() ? "transparent" :
                                   QString("rgb(%1, %2, %3)")
                                   .arg(c.red()).arg(c.green()).arg(c.blue()));
    colorExample->setAutoFillBackground(true);
    colorExample->setStyleSheet(StyleSheet);
    colorExample->setToolTip(tr("Hex RGB %1").arg(c.name(QColor::HexRgb).toUpper()));
    grid->addWidget(colorExample,1,1);

    colorButton = new QPushButton(tr("Change"),parent);
    connect(colorButton,SIGNAL(   clicked(bool)),
            this,        SLOT(browseColor(bool)));
    grid->addWidget(colorButton,1,2);

    /* hide arrows [optional] */

    if (isLine && !_isCallout) {
        hideTipBox = new QCheckBox(tr("Hide Pointer Tip"), parent);
        hideTipBox->setChecked(hideTip);
        connect(hideTipBox,SIGNAL(      clicked(bool)),
                this,        SLOT(hideTipChange(bool)));
        grid->addWidget(hideTipBox,2,0,1,3);
    }

  } else {

    title = tr("Tip");

    /*  Width */

    label = new QLabel(tr("%1 Width").arg(title),parent);
    label->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    grid->addWidget(label,0,0);

    widthEdit = new QLineEdit(parent);
    QDoubleValidator *widthValidator = new QDoubleValidator(widthEdit);
    widthValidator->setRange(0.0f, 100.0f);
    widthValidator->setDecimals(4);
    widthValidator->setNotation(QDoubleValidator::StandardNotation);
    widthEdit->setValidator(widthValidator);
    widthEdit->setText(QString::number(data.tipData.tipWidth,'f',4));
    widthEdit->setToolTip(units);
    resetWidthEditAct = widthEdit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
    resetWidthEditAct->setText(tr("Reset"));
    resetWidthEditAct->setEnabled(false);
    connect(widthEdit,        SIGNAL(     textEdited(const QString&)),
            this,               SLOT(enableEditReset(const QString&)));
    connect(resetWidthEditAct,SIGNAL(    triggered()),
            this,               SLOT(lineEditReset()));
    connect(widthEdit,        SIGNAL(textEdited(const QString&)),
            this,               SLOT(sizeChange(const QString&)));
    grid->addWidget(widthEdit,0,1);

    /*  Height */

    label = new QLabel(tr("%1 Height").arg(title),parent);
    label->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    grid->addWidget(label,0,2);

    heightEdit = new QLineEdit(parent);
    QDoubleValidator *heightValidator = new QDoubleValidator(heightEdit);
    heightValidator->setRange(0.0f, 100.0f);
    heightValidator->setDecimals(4);
    heightValidator->setNotation(QDoubleValidator::StandardNotation);
    heightEdit->setValidator(heightValidator);
    heightEdit->setText(QString::number(data.tipData.tipHeight,'f',4));
    heightEdit->setToolTip(units);
    resetHeightEditAct = heightEdit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
    resetHeightEditAct->setText(tr("Reset"));
    resetHeightEditAct->setEnabled(false);
    connect(heightEdit,        SIGNAL(     textEdited(const QString&)),
            this,                SLOT(enableEditReset(const QString&)));
    connect(resetHeightEditAct,SIGNAL(    triggered()),
            this,                SLOT(lineEditReset()));
    connect(heightEdit,        SIGNAL(textEdited(const QString&)),
            this,                SLOT(sizeChange(const QString&)));
    grid->addWidget(heightEdit,0,3);
  }

  tipModified = false;
  lineModified = false;
  borderModified = false;
}

void PointerAttribGui::enableEditReset(const QString &displayText)
{
  const double value = displayText.toDouble();

  if (isTip) {
    if (sender() == widthEdit)
      resetWidthEditAct->setEnabled(notEqual(value, data.tipData.tipWidth));
    else
    if (sender() == heightEdit)
      resetHeightEditAct->setEnabled(notEqual(value, data.tipData.tipHeight));
  } else if (isLine) {
    if (sender() == thicknessEdit)
      resetThicknessEditAct->setEnabled(notEqual(value, data.lineData.thickness));
  } else if (isBorder) {
    if (sender() == thicknessEdit)
      resetThicknessEditAct->setEnabled(notEqual(value, data.borderData.thickness));
  }
}

void PointerAttribGui::lineEditReset()
{
  if (isTip) {
    if (sender() == resetWidthEditAct) {
      resetWidthEditAct->setEnabled(false);
      if (widthEdit) {
        widthEdit->setText(QString::number(data.tipData.tipWidth,'f',4));
      }
    }
    else
    if (sender() == resetHeightEditAct) {
      resetHeightEditAct->setEnabled(false);
      if (heightEdit) {
        heightEdit->setText(QString::number(data.tipData.tipHeight,'f',4));
      }
    }
  } else if (isLine) {
    if (sender() == resetThicknessEditAct) {
      resetThicknessEditAct->setEnabled(false);
      if (thicknessEdit) {
        thicknessEdit->setText(QString::number(data.lineData.thickness,'f',4));
      }
    }
  } else if (isBorder) {
    if (sender() == resetThicknessEditAct) {
      resetThicknessEditAct->setEnabled(false);
      if (thicknessEdit) {
        thicknessEdit->setText(QString::number(data.borderData.thickness,'f',4));
      }
    }
  }
}

void PointerAttribGui::lineChange(const QString &_line)
{
  BorderData::Line padLine = BorderData::BdrLnSolid;
  if (_line == "Solid Line") {
    padLine = BorderData::BdrLnSolid;
  } else if (_line == "Dash Line") {
    padLine = BorderData::BdrLnDash;
  } else if (_line == "Dotted Line") {
    padLine = BorderData::BdrLnDot;
  } else if (_line == "Dot-Dash Line") {
    padLine = BorderData::BdrLnDashDot;
  } else if (_line == "Dot-Dot-Dash Line") {
    padLine = BorderData::BdrLnDashDotDot;
  }
  PointerAttribData _data = meta->value();
  if (isLine) {
    _data.lineData.line = padLine;
    _data.attribType = PointerAttribData::Line;
    lineModified = metaModified(data.lineData.line != padLine);
  } else if (isBorder) {
    _data.borderData.line = padLine;
    _data.attribType = PointerAttribData::Border;
    borderModified = metaModified(data.borderData.line != padLine);
  }
  meta->setValue(_data);
}

void PointerAttribGui::sizeChange(const QString &size)
{
  float value = size.toFloat();
  PointerAttribData _data = meta->value();
  if (isTip) {
    if (sender() == widthEdit) {
      _data.tipData.tipWidth = value;
      tipModified = metaModified(notEqual(value, data.tipData.tipWidth));
    } else if (sender() == heightEdit) {
      _data.tipData.tipHeight = value;
      tipModified = metaModified(notEqual(value, data.tipData.tipHeight));
    }
    _data.attribType = PointerAttribData::Tip;
  } else if (isLine) {
    if (sender() == thicknessEdit) {
      _data.lineData.thickness = value;
      _data.attribType = PointerAttribData::Line;
      lineModified = metaModified(notEqual(value, data.lineData.thickness));
    }
  } else if (isBorder) {
    if (sender() == thicknessEdit) {
      _data.borderData.thickness = value;
      _data.attribType = PointerAttribData::Border;
      borderModified = metaModified(notEqual(value, data.borderData.thickness));
    }
  }
  meta->setValue(_data);
}

void PointerAttribGui::browseColor(bool)
{
  PointerAttribData _data = meta->value();
  QString padColor = isLine ? _data.lineData.color
                            : _data.borderData.color;
  QColor color = LDrawColor::color(padColor);
  QColor newColor = QColorDialog::getColor(color,this);
  if (newColor.isValid() && color != newColor) {
    if (isLine) {
      _data.lineData.color = newColor.name();
      _data.attribType = PointerAttribData::Line;
      lineModified = metaModified(data.lineData.color != newColor.name());
    } else {
      _data.borderData.color = newColor.name();
      _data.attribType = PointerAttribData::Border;
      borderModified = metaModified(data.borderData.color != newColor.name());
    }
    meta->setValue(_data);
    QString styleSheet =
        QString("QLabel { background-color: rgb(%1, %2, %3); }").
        arg(newColor.red()).arg(newColor.green()).arg(newColor.blue());
    colorExample->setAutoFillBackground(true);
    colorExample->setStyleSheet(styleSheet);
    colorExample->setToolTip(tr("Hex RGB %1").arg(newColor.name(QColor::HexRgb).toUpper()));
  }
}

void PointerAttribGui::hideTipChange(bool checked)
{
  PointerAttribData _data = meta->value();
  _data.lineData.hideTip = checked;
  _data.attribType = PointerAttribData::Tip;
  meta->setValue(_data);
  lineModified = metaModified(data.lineData.hideTip != checked);
}

void PointerAttribGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Separator/Divider
 *
 **********************************************************************/

SepGui::SepGui(
  SepMeta   *_meta,
  QGroupBox *parent)
{
  meta = _meta;
  data = meta->value();

  QGridLayout *grid = new QGridLayout(parent);

  if (parent) {
      parent->setLayout(grid);
      parent->setWhatsThis(lpubWT(WT_GUI_SEPARATOR, parent->title()));
  } else {
      setLayout(grid);
      setWhatsThis(lpubWT(WT_GUI_SEPARATOR, tr("Divider")));
  }

  QPushButton *button;
  QComboBox   *typeCombo;

  QLabel *label = new QLabel("Width",parent);
  grid->addWidget(label,0,0);

  thicknessEdit = new QLineEdit(parent);
  QDoubleValidator *widthValidator = new QDoubleValidator(thicknessEdit);
  widthValidator->setRange(0.0f, 100.0f);
  widthValidator->setDecimals(4);
  widthValidator->setNotation(QDoubleValidator::StandardNotation);
  thicknessEdit->setValidator(widthValidator);
  thicknessEdit->setText(QString::number(data.thickness,'f',4));
  resetThicknessEditAct = thicknessEdit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  resetThicknessEditAct->setText(tr("Reset"));
  resetThicknessEditAct->setEnabled(false);
  connect(thicknessEdit,        SIGNAL( textEdited(const QString&)),
          this,                   SLOT(enableReset(const QString&)));
  connect(resetThicknessEditAct,SIGNAL(    triggered()),
          this,                   SLOT(lineEditReset()));
  connect(thicknessEdit,        SIGNAL(     textEdited(const QString&)),
          this,                   SLOT(thicknessChange(const QString&)));
  grid->addWidget(thicknessEdit,0,1);

  label = new QLabel("Length",parent);
  grid->addWidget(label,1,0);

  typeCombo = new QComboBox(parent);
  typeCombo->addItem("Default");
  typeCombo->addItem("Page");
  typeCombo->addItem("Custom");
  typeCombo->setCurrentIndex(int(data.type));
  connect(typeCombo,SIGNAL(currentIndexChanged(int)),
          this,       SLOT(         typeChange(int)));
  grid->addWidget(typeCombo,1,1);

  lengthEdit = new QLineEdit(parent);
  QDoubleValidator *lengthValidator = new QDoubleValidator(lengthEdit);
  lengthValidator->setRange(0.0f, 100.0f);
  lengthValidator->setDecimals(4);
  lengthValidator->setNotation(QDoubleValidator::StandardNotation);
  lengthEdit->setValidator(lengthValidator);
  lengthEdit->setText(QString::number(data.length,'f',4));
  lengthEdit->setEnabled(data.type == SepData::LenCustom);
  resetLengthEditAct = lengthEdit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  resetLengthEditAct->setText(tr("Reset"));
  resetLengthEditAct->setEnabled(false);
  connect(lengthEdit,        SIGNAL( textEdited(const QString&)),
          this,                SLOT(enableReset(const QString&)));
  connect(resetLengthEditAct,SIGNAL(    triggered()),
          this,                SLOT(lineEditReset()));
  connect(lengthEdit,        SIGNAL(  textEdited(const QString&)),
          this,                SLOT(lengthChange(const QString&)));
  grid->addWidget(lengthEdit,1,2);

  label = new QLabel("Color",parent);
  grid->addWidget(label,2,0);

  colorExample = new QLabel(parent);
  colorExample->setFrameStyle(QFrame::Sunken|QFrame::Panel);
  colorExample->setAutoFillBackground(true);
  QColor c = QColor(data.color);
  QString styleSheet =
      QString("QLabel { background-color: rgb(%1, %2, %3); }").
      arg(c.red()).arg(c.green()).arg(c.blue());
  colorExample->setStyleSheet(styleSheet);
  colorExample->setToolTip(tr("Hex RGB %1").arg(c.name(QColor::HexRgb).toUpper()));
  grid->addWidget(colorExample,2,1);

  button = new QPushButton("Change",parent);
  connect(button,SIGNAL(clicked(bool)),
          this,  SLOT(  browseColor(bool)));
  grid->addWidget(button,2,2);

  label = new QLabel("Margins",parent);
  grid->addWidget(label,3,0);

  marginXEdit = new QLineEdit(parent);
  QDoubleValidator *marginXValidator = new QDoubleValidator(marginXEdit);
  marginXValidator->setRange(0.0f, 100.0f);
  marginXValidator->setDecimals(4);
  marginXValidator->setNotation(QDoubleValidator::StandardNotation);
  marginXEdit->setValidator(marginXValidator);
  marginXEdit->setText(QString::number(data.margin[0],'f',4));
  resetXEditAct = marginXEdit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  resetXEditAct->setText(tr("Reset"));
  resetXEditAct->setEnabled(false);
  connect(marginXEdit,  SIGNAL( textEdited(const QString&)),
          this,           SLOT(enableReset(const QString&)));
  connect(resetXEditAct,SIGNAL(    triggered()),
          this,           SLOT(lineEditReset()));
  connect(marginXEdit,  SIGNAL(   textEdited(const QString&)),
          this,           SLOT(marginXChange(const QString&)));
  grid->addWidget(marginXEdit,3,1);

  marginYEdit = new QLineEdit(parent);
  QDoubleValidator *marginYValidator = new QDoubleValidator(marginYEdit);
  marginYValidator->setRange(0.0f, 100.0f);
  marginYValidator->setDecimals(4);
  marginYValidator->setNotation(QDoubleValidator::StandardNotation);
  marginYEdit->setValidator(marginYValidator);
  marginYEdit->setText(QString::number(data.margin[1],'f',4));
  resetYEditAct = marginYEdit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  resetYEditAct->setText(tr("Reset"));
  resetYEditAct->setEnabled(false);
  connect(marginYEdit,  SIGNAL( textEdited(const QString&)),
          this,           SLOT(enableReset(const QString&)));
  connect(resetYEditAct,SIGNAL(triggered()),
          this,           SLOT(lineEditReset()));
  connect(marginYEdit,  SIGNAL(   textEdited(const QString&)),
          this,           SLOT(marginYChange(const QString&)));
  grid->addWidget(marginYEdit,3,2);

  typeModified = false;
  marginsModified = false;
  marginsModified = false;
  lengthModified = false;
  thicknessModified = false;
  colorModified = false;
}

void SepGui::enableReset(const QString &displayText)
{
  const double value = displayText.toDouble();

  if (sender() == marginXEdit)
    resetXEditAct->setEnabled(notEqual(value, data.margin[0]));
  else
  if (sender() == marginYEdit)
    resetYEditAct->setEnabled(notEqual(value, data.margin[1]));
  else
  if (sender() == lengthEdit)
    resetLengthEditAct->setEnabled(notEqual(value, data.length));
  else
  if (sender() == thicknessEdit)
    resetThicknessEditAct->setEnabled(notEqual(value, data.thickness));
}

void SepGui::lineEditReset()
{
  if (sender() == resetXEditAct) {
    resetXEditAct->setEnabled(false);
    if (marginXEdit) {
      marginXEdit->setText(QString::number(data.margin[0],'f',4));
      marginsModified = false;
    }
  }
  else
  if (sender() == resetYEditAct) {
    resetYEditAct->setEnabled(false);
    if (marginYEdit) {
      marginYEdit->setText(QString::number(data.margin[1],'f',4));
      marginsModified = false;
    }
  }
  else
  if (sender() == resetLengthEditAct) {
    resetLengthEditAct->setEnabled(false);
    if (lengthEdit) {
      lengthEdit->setText(QString::number(data.length,'f',4));
      lengthModified = false;
    }
  }
  else
  if (sender() == resetThicknessEditAct) {
    resetThicknessEditAct->setEnabled(false);
    if (thicknessEdit) {
      thicknessEdit->setText(QString::number(data.thickness,'f',4));
      thicknessModified = false;
    }
  }
}

void SepGui::typeChange(
  int type)
{
  SepData _data = meta->value();
  _data.type = SepData::LengthType(type);
  lengthEdit->setEnabled(_data.type == SepData::LenCustom);
  meta->setValue(_data);
  typeModified = metaModified(_data.type != data.type);
}

void SepGui::lengthChange(
  const QString &string)
{
  float value = string.toFloat();
  SepData _data = meta->value();
  _data.length = value;
  meta->setValue(_data);
  lengthModified = metaModified(notEqual(value, data.length));
}

void SepGui::thicknessChange(
  const QString &string)
{
  float value = string.toFloat();
  SepData _data = meta->value();
  _data.thickness = value;
  meta->setValue(_data);
  thicknessModified = metaModified(notEqual(value, data.thickness));
}

void SepGui::browseColor(bool)
{
  SepData _data = meta->value();

  QColor qcolor = LDrawColor::color(_data.color);
  QColor newColor = QColorDialog::getColor(qcolor,this);
  if (newColor.isValid() && qcolor != newColor) {
      colorExample->setAutoFillBackground(true);
      QString styleSheet =
          QString("QLabel { background-color: rgb(%1, %2, %3); }")
          .arg(newColor.red()).arg(newColor.green()).arg(newColor.blue());
      colorExample->setStyleSheet(styleSheet);
      colorExample->setToolTip(tr("Hex RGB %1").arg(newColor.name(QColor::HexRgb).toUpper()));
      _data.color = newColor.name();
      meta->setValue(_data);
      colorModified = metaModified(_data.color != data.color);
    }
}

void SepGui::marginXChange(
  const QString &string)
{
  float value = string.toFloat();
  SepData _data = meta->value();
  _data.margin[0] = value;
  meta->setValue(_data);
  marginsModified = metaModified(notEqual(value, data.margin[0]));
}

void SepGui::marginYChange(
  const QString &string)
{
  float value = string.toFloat();
  SepData _data = meta->value();
  _data.margin[1] = value;
  meta->setValue(_data);
  marginsModified = metaModified(notEqual(value, data.margin[1]));
}

void SepGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Resolution
 *
 **********************************************************************/

ResolutionGui::ResolutionGui(
  ResolutionMeta   *_meta,
  QGroupBox        *parent)
{
  meta = _meta;
  units = _meta->type();
  value = _meta->value();

  QGridLayout *grid = new QGridLayout(parent);

  if (parent) {
    parent->setLayout(grid);
    parent->setWhatsThis(lpubWT(WT_GUI_DOCUMENT_RESOLUTION, parent->title()));
  } else {
    setLayout(grid);
    setWhatsThis(lpubWT(WT_GUI_DOCUMENT_RESOLUTION, tr("Dot Resolution")));
  }

  QLabel    *label;

  label = new QLabel("Units",parent);
  grid->addWidget(label,0,0);

  // default value always inches
  // so convert to centimeters if DPCM
  float resolution = value;
  if (_meta->isDefault() && units == DPCM) {
    resolution = inches2centimeters(value);
  }

  QComboBox *combo;
  combo = new QComboBox(parent);
  combo->addItem("Dots Per Inch");
  combo->addItem("Dots Per Centimetre");
  combo->setCurrentIndex(int(units));
  connect(combo,SIGNAL(currentTextChanged(const QString&)),
          this,   SLOT(        unitsChange(const QString&)));
  grid->addWidget(combo,0,1);

  valueEdit = new QLineEdit(parent);
  QIntValidator *valueValidator = new QIntValidator(valueEdit);
  valueValidator->setRange(0, 100000);
  valueEdit->setValidator(valueValidator);
  valueEdit->setText(QString::number(int(resolution)));
  reset0Act = valueEdit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  reset0Act->setText(tr("Reset"));
  reset0Act->setEnabled(false);
  connect(valueEdit, SIGNAL( textEdited(const QString&)),
          this,        SLOT(enableReset(const QString&)));
  connect(reset0Act,  SIGNAL(    triggered()),
          this,         SLOT(lineEditReset()));

  connect(valueEdit,SIGNAL(textEdited( const QString&)),
          this,     SLOT(  valueChange(const QString&)));
  grid->addWidget(valueEdit,0,2);

  grid->setColumnStretch(0,3);

  if (parent) {
    parent->setLayout(grid);
  } else {
    setLayout(grid);
  }

  unitsModified = false;
  valueModified = false;
}

void ResolutionGui::enableReset(const QString &displayText)
{
  const int value = displayText.toInt();
  reset0Act->setEnabled(value != (int)value);
}

void ResolutionGui::lineEditReset()
{
  if (sender() == reset0Act) {
    reset0Act->setEnabled(false);
    if (valueEdit) {
      valueEdit->setText(QString::number((int)value));
    }
  }
}

void ResolutionGui::unitsChange(const QString &_units)
{
  ResolutionType type;

  if (_units == "Dots Per Centimetre")
    type = DPCM;
  else
    type = DPI;
  // default value always inches
  // so convert to centimeters if DPCM
  float resolution = meta->value();
  if (type == DPCM) {
    resolution = inches2centimeters(resolution)+0.5f;
  }

  meta->setValue(type,resolution);

  valueEdit->setText(QString::number((int)resolution));

  unitsModified = metaModified(type != units);
  if (unitsModified)
    emit unitsChanged(type);
}

void ResolutionGui::valueChange(const QString &string)
{
  float resolution = string.toFloat();
  meta->setValue(resolution);
  valueModified = metaModified(notEqual(resolution, value));
  emit settingsChanged(valueModified);
}

void ResolutionGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Preferred Renderer
 *
 **********************************************************************/

PreferredRendererGui::PreferredRendererGui(
        PreferredRendererMeta *_meta,
        QGroupBox             *parent)
{
  QGridLayout *grid = new QGridLayout(parent);
  QHBoxLayout *hLayout = new QHBoxLayout();

  if (parent) {
      parent->setLayout(grid);
      parent->setWhatsThis(lpubWT(WT_GUI_PREFERRED_RENDERER, parent->title()));
  } else {
      setLayout(grid);
      setWhatsThis(lpubWT(WT_GUI_PREFERRED_RENDERER, tr("Preferred Renderer")));
  }

  meta = _meta;
  data = meta->value();

  combo = new QComboBox(parent);
  combo->addItem(rendererNames[RENDERER_NATIVE]);
  if (Preferences::ldgliteInstalled)
    combo->addItem(rendererNames[RENDERER_LDGLITE]);
  if (Preferences::ldviewInstalled)
    combo->addItem(rendererNames[RENDERER_LDVIEW]);
  if (Preferences::povRayInstalled)
    combo->addItem(rendererNames[RENDERER_POVRAY]);
  combo->setCurrentIndex(int(combo->findText(rendererNames[data.renderer])));

  connect(combo,SIGNAL(currentIndexChanged(int)),
          this,   SLOT(       valueChanged(int)));

  grid->addWidget(combo,0,0,2,1);

  ldvSingleCallBox = new QCheckBox("Use LDView Single Call",parent);
  ldvSingleCallBox->setToolTip("Process a page's part or assembly images in a single renderer call");
  ldvSingleCallBox->setChecked(data.useLDVSingleCall);
  ldvSingleCallBox->setEnabled(data.renderer == RENDERER_LDVIEW);

  connect(ldvSingleCallBox,SIGNAL(clicked(bool)),
          this,              SLOT(checkChanged(bool)));

  grid->addWidget(ldvSingleCallBox,0,1);

  ldvSnapshotListBox = new QCheckBox("Use LDView Snapshot List",parent);
  ldvSnapshotListBox->setToolTip("Capture Single Call ldraw image-generation files in a single list file");
  ldvSnapshotListBox->setChecked(data.useLDVSnapShotList);
  ldvSnapshotListBox->setEnabled(data.renderer == RENDERER_LDVIEW && data.useLDVSingleCall);

  connect(ldvSnapshotListBox,SIGNAL(clicked(bool)),
          this,                SLOT(checkChanged(bool)));

  grid->addWidget(ldvSnapshotListBox,1,1);

  povFileGeneratorGrpBox = new QGroupBox("POV File Generation Renderer",parent);
  povFileGeneratorGrpBox->setEnabled(data.renderer == RENDERER_POVRAY);
  povFileGeneratorGrpBox->setLayout(hLayout);

  grid->addWidget(povFileGeneratorGrpBox,2,0,1,2);

  nativeButton = new QRadioButton("Native",povFileGeneratorGrpBox);
  nativeButton->setChecked(data.useNativeGenerator);

  connect(nativeButton,SIGNAL(clicked(bool)),
          this,        SLOT(buttonChanged(bool)));

  hLayout->addWidget(nativeButton);

  ldvButton = new QRadioButton("LDView",povFileGeneratorGrpBox);
  ldvButton->setChecked(!data.useNativeGenerator);

  connect(ldvButton,SIGNAL(clicked(bool)),
          this,     SLOT(buttonChanged(bool)));

  hLayout->addWidget(ldvButton);

  ldvSingleCallBox->setEnabled(data.renderer == RENDERER_LDVIEW);
  ldvSnapshotListBox->setEnabled(ldvSingleCallBox->isChecked());
  povFileGeneratorGrpBox->setEnabled(data.renderer == RENDERER_POVRAY);

  rendererModified = false;
  snapshotModified = false;
}

void PreferredRendererGui::valueChanged(int)
{
  RendererData rendererData = meta->value();
  if (sender() == combo) {
    const QString pick = combo->currentText();
    ldvSingleCallBox->setEnabled(pick == rendererNames[RENDERER_LDVIEW]);
    povFileGeneratorGrpBox->setEnabled(pick == rendererNames[RENDERER_POVRAY]);
    rendererData.renderer = rendererMap[pick];
    emit rendererChanged(rendererData.renderer);
    rendererModified = metaModified(rendererData.renderer != data.renderer);
  }
  meta->setValue(data);
  emit settingsChanged(modified);
}

void PreferredRendererGui::checkChanged(bool checked)
{
  RendererData rendererData = meta->value();
  if (sender() == ldvSnapshotListBox) {
    snapshotModified = metaModified(rendererData.useLDVSnapShotList != checked);
    rendererData.useLDVSnapShotList = checked;
  } else if (sender() == ldvSingleCallBox) {
    ldvSnapshotListBox->setEnabled(checked);
    rendererData.useLDVSingleCall = checked;
    singleCallModified = metaModified(data.useLDVSingleCall != checked);
  }
  meta->setValue(rendererData);
  emit settingsChanged(modified);
}

void PreferredRendererGui::buttonChanged(bool checked)
{
    RendererData rendererData = meta->value();
    rendererData.useNativeGenerator = checked;
    meta->setValue(rendererData);

    nativeGenModified = metaModified(data.useNativeGenerator != checked);
    emit settingsChanged(nativeGenModified);
}

void PreferredRendererGui::apply(QString &topLevelFile)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(topLevelFile,meta);
  }
}

/***********************************************************************
 *
 * CameraAngles
 *
 **********************************************************************/

CameraAnglesGui::CameraAnglesGui(
    QString const    &heading,
    CameraAnglesMeta *_meta,
    QGroupBox        *parent)
{
  using namespace CameraViews;
  meta = _meta;
  data = meta->value();

  QGridLayout *grid = new QGridLayout(parent);

  if (parent) {
      parent->setLayout(grid);
      parent->setWhatsThis(lpubWT(WT_GUI_CAMERA_ANGLES, parent->title()));
  } else {
      setLayout(grid);
      setWhatsThis(lpubWT(WT_GUI_CAMERA_ANGLES, heading.isEmpty() ? tr("Camera Angles") : heading));
  }

  // latitude
  latitudeLabel = new QLabel(tr("Latitude"),parent);
  grid->addWidget(latitudeLabel,0,0);

  latitudeEdit = new QLineEdit(parent);
  QDoubleValidator *latitudeValidator = new QDoubleValidator(latitudeEdit);
  latitudeValidator->setRange(-360.0f, 360.0f);
  latitudeValidator->setDecimals(meta->_precision);
  latitudeValidator->setNotation(QDoubleValidator::StandardNotation);
  latitudeEdit->setValidator(latitudeValidator);
  //latitudeEdit->setInputMask(MetaGui::formatMask(data.angles[0], meta->_fieldWidth, meta->_precision));
  latitudeEdit->setText(QString::number(data.angles[0],'f',meta->_precision));
  setLatitudeResetAct = latitudeEdit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  setLatitudeResetAct->setText(tr("Reset"));
  setLatitudeResetAct->setEnabled(false);
  connect(latitudeEdit,         SIGNAL( textEdited(const QString&)),
          this,                   SLOT(enableReset(const QString&)));
  connect(setLatitudeResetAct,  SIGNAL(    triggered()),
          this,                   SLOT(lineEditReset()));
  connect(latitudeEdit,         SIGNAL(    textEdited(const QString&)),
          this,                   SLOT(latitudeChange(const QString&)));
  grid->addWidget(latitudeEdit,0,1);

  // longitude
  longitudeLabel = new QLabel(tr("Longitude"),parent);
  grid->addWidget(longitudeLabel,0,2);

  longitudeEdit = new QLineEdit(parent);
  QDoubleValidator *longitudeValidator = new QDoubleValidator(longitudeEdit);
  longitudeValidator->setRange(-360.0f, 360.0f);
  longitudeValidator->setDecimals(meta->_precision);
  longitudeValidator->setNotation(QDoubleValidator::StandardNotation);
  longitudeEdit->setValidator(longitudeValidator);
  //longitudeEdit->setInputMask(MetaGui::formatMask(data.angles[1], meta->_fieldWidth, meta->_precision));
  longitudeEdit->setText(QString::number(data.angles[1],'f',meta->_precision));
  setLongitudeResetAct = longitudeEdit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  setLongitudeResetAct->setText(tr("Reset"));
  setLongitudeResetAct->setEnabled(false);
  connect(longitudeEdit,        SIGNAL( textEdited(const QString&)),
          this,                   SLOT(enableReset(const QString&)));
  connect(setLongitudeResetAct, SIGNAL(    triggered()),
          this,                   SLOT(lineEditReset()));
  connect(longitudeEdit,        SIGNAL(     textEdited(const QString&)),
          this,                   SLOT(longitudeChange(const QString&)));
  grid->addWidget(longitudeEdit,0,3);

  // Camera view
  cameraViewLabel = new QLabel(tr("Viewpoint"),parent);
  grid->addWidget(cameraViewLabel,1,0);

  cameraViewCombo = new QComboBox(parent);
  cameraViewCombo->addItem("Front");
  cameraViewCombo->addItem("Back");
  cameraViewCombo->addItem("Top");
  cameraViewCombo->addItem("Bottom");
  cameraViewCombo->addItem("Left");
  cameraViewCombo->addItem("Right");
  cameraViewCombo->addItem("Home");
  cameraViewCombo->addItem("Latitude/Longitude");
  cameraViewCombo->addItem("Default");
  cameraViewCombo->setCurrentIndex(int(data.cameraView));
  connect(cameraViewCombo,SIGNAL(currentIndexChanged(int)),
          this,             SLOT(   cameraViewChange(int)));
  grid->addWidget(cameraViewCombo,1,1);

  // Home viewpoint angles change
  homeViewpointBox = new QCheckBox(tr("Use Latitude And Longitude Angles"),parent);
  homeViewpointBox->setChecked(data.cameraView == CameraView::Home && data.customViewpoint);
  homeViewpointBox->setToolTip(tr("Set Home viewpoint angles to use specified latitude and longitude."));
  connect(homeViewpointBox,SIGNAL(             clicked(bool)),
          this,              SLOT(homeViewpointChanged(bool)));
  grid->addWidget(homeViewpointBox,1,2,1,2);

  setEnabled(data.cameraView == CameraView::Default);

  latitudeModified = false;
  longitudeModified = false;
  cameraViewModified = false;
  customViewpointModified = false;
}

void CameraAnglesGui::enableReset(const QString &displayText)
{
  const double value = displayText.toDouble();

  if (sender() == latitudeEdit)
    setLatitudeResetAct->setEnabled(notEqual(value, data.angles[0]));
  else
  if (sender() == longitudeEdit)
    setLongitudeResetAct->setEnabled(notEqual(value, data.angles[1]));
}

void CameraAnglesGui::lineEditReset()
{
  if (sender() == setLatitudeResetAct) {
    setLatitudeResetAct->setEnabled(false);
    if (latitudeEdit) {
      latitudeEdit->setText(QString::number(data.angles[0],'f',meta->_precision));
      latitudeModified = false;
    }
  }
  else
  if (sender() == setLongitudeResetAct) {
    setLongitudeResetAct->setEnabled(false);
    if (longitudeEdit) {
      longitudeEdit->setText(QString::number(data.angles[1],'f',meta->_precision));
      longitudeModified = false;
    }
  }
}

void CameraAnglesGui::latitudeChange(const QString &string)
{
  float value = string.toFloat();
  CameraAnglesData cad = meta->value();
  cad.angles[0] = value;
  if (homeViewpointBox->isChecked())  {
    cad.customViewpoint = notEqual(value, 30.0f) || notEqual(cad.angles[1], 45.0f);
    customViewpointModified = metaModified(cad.customViewpoint != data.customViewpoint);
  }
  meta->setValue(cad);
  latitudeModified = metaModified(notEqual(value, data.angles[0]) || notEqual(cad.angles[1], data.angles[1]));
  emit settingsChanged(latitudeModified || customViewpointModified);
}

void CameraAnglesGui::longitudeChange(const QString &string)
{
  float value = string.toFloat();
  CameraAnglesData cad = meta->value();
  cad.angles[1] = string.toFloat();
  if (homeViewpointBox->isChecked()) {
    cad.customViewpoint = notEqual(cad.angles[0], 30.0f) || notEqual(value, 45.0f);
    customViewpointModified = metaModified(cad.customViewpoint != data.customViewpoint);
  }
  meta->setValue(cad);
  longitudeModified = metaModified(notEqual(cad.angles[0], data.angles[0]) || notEqual(value, data.angles[1]));
  emit settingsChanged(longitudeModified || customViewpointModified);
}

void CameraAnglesGui::cameraViewChange(int value)
{
  using namespace CameraViews;
  CameraView cameraView = static_cast<CameraView>(value);
  setEnabled(cameraView > CameraView::Home);

  if (cameraView != meta->cameraView()) {
    double latitude = 0.0f;
    double longitude = 0.0f;
    switch (cameraView)
    {
      case CameraView::Front:
        latitude = 0.0f;
        longitude = 0.0f;
        break;
      case CameraView::Back:
        latitude = 0.0f;
        longitude = 180.0f;
        break;
      case CameraView::Top:
        latitude = 90.0f;
        longitude = 0.0f;
        break;
      case CameraView::Bottom:
        latitude = -90.0f;
        longitude =  0.0f;
        break;
      case CameraView::Left:
        latitude = 0.0f;
        longitude = 90.0f;
        break;
      case CameraView::Right:
        latitude = 0.0f;
        longitude = -90.0f;
        break;
      case CameraView::Home:
        latitude = 30.0f;
        longitude = 45.0f;
        break;
      default:
        latitude = data.angles[0];
        longitude = data.angles[1];
    }

    latitudeEdit->setText(QString::number(latitude,'f',meta->_precision));
    longitudeEdit->setText(QString::number(longitude,'f',meta->_precision));

    if (cameraView != data.cameraView) {
      CameraAnglesData cad = meta->value();
      cad.cameraView = cameraView;
      meta->setValue(cad);
      cameraViewModified = true;
      emit settingsChanged(cameraViewModified);
    }
  }
}

void CameraAnglesGui::homeViewpointChanged(bool enable)
{
  latitudeLabel->setEnabled(enable);
  longitudeLabel->setEnabled(enable);
  latitudeEdit->setEnabled(enable);
  longitudeEdit->setEnabled(enable);
}

void CameraAnglesGui::setEnabled(bool enable)
{
  bool homeViewpoint = cameraViewCombo->currentIndex() == static_cast<int>(CameraViews::CameraView::Home);
  homeViewpointBox->setEnabled(homeViewpoint);
  homeViewpointBox->setChecked(homeViewpoint && meta->customViewpoint());
  latitudeLabel->setEnabled(enable);
  longitudeLabel->setEnabled(enable);
  latitudeEdit->setEnabled(enable);
  longitudeEdit->setEnabled(enable);
}

void CameraAnglesGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * CameraFoV
 *
 **********************************************************************/

CameraFOVGui::CameraFOVGui(
  const QString &heading,
  FloatMeta     *_meta,
  QGroupBox     *parent)
{
  meta = _meta;
  data = _meta->value();

  QHBoxLayout *layout = new QHBoxLayout(parent);

  if (parent) {
    parent->setLayout(layout);
    parent->setWhatsThis(lpubWT(WT_GUI_CAMERA_FIELD_OF_VIEW, parent->title()));
  } else {
    setLayout(layout);
    setWhatsThis(lpubWT(WT_GUI_CAMERA_FIELD_OF_VIEW, heading.isEmpty() ? tr("Camera Field Of View") : heading));
  }

  label = new QLabel(heading.isEmpty() ? tr("Field Of View") : heading,parent);
  layout->addWidget(label);

  const int residual = data - (int)data;
  const int decimalSize = QString::number(residual).size();
  const int decimalPlaces = decimalSize < 3 ? 2 : decimalSize;

  spin = new QDoubleSpinBox(parent);
  spin->setRange(_meta->_min,_meta->_max);
  spin->setSingleStep(0.01f);
  spin->setDecimals(decimalPlaces);
  spin->setValue(data);
  connect(spin,SIGNAL(valueChanged(double)),
          this,SLOT  (valueChanged(double)));
  layout->addWidget(spin);

  button = new QPushButton(parent);
  button->setIcon(QIcon(":/resources/resetaction.png"));
  button->setIconSize(QSize(16,16));
  button->setStyleSheet("QPushButton { background-color: rgba(255,255,255,0);border: 0px; }");
  button->setToolTip(tr("Reset"));
  button->setEnabled(false);
  connect(spin,   SIGNAL(valueChanged(double)),
          this,     SLOT( enableReset(double)));
  connect(button, SIGNAL(  clicked(bool)),
          this,     SLOT(spinReset(bool)));
  layout->addWidget(button);

  QSpacerItem *hSpacer = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Fixed);
  layout->addSpacerItem(hSpacer);
}

void CameraFOVGui::enableReset(double value)
{
  button->setEnabled(notEqual(value, data));
}

void CameraFOVGui::spinReset(bool)
{
  button->setEnabled(false);
  if (spin) {
    spin->setValue(data);
    spin->setFocus();
    modified = false;
  }
}

void CameraFOVGui::valueChanged(double value)
{
  meta->setValue(value);
  modified = notEqual(value, data);
  emit settingsChanged(modified);
}

void CameraFOVGui::setEnabled(bool enable)
{
  label->setEnabled(enable);
  spin->setEnabled(enable);
}

void CameraFOVGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * CameraPlane
 *
 **********************************************************************/

CameraZPlaneGui::CameraZPlaneGui(
  const QString &heading,
  FloatMeta     *_meta,
  bool            zfar,
  QGroupBox     *parent)
{
  meta = _meta;
  data = _meta->value();

  QHBoxLayout *layout = new QHBoxLayout(parent);

  WT_Type wtType = zfar ? WT_GUI_CAMERA_FAR_PLANE : WT_GUI_CAMERA_NEAR_PLANE;

  if (parent) {
    parent->setLayout(layout);
    parent->setWhatsThis(lpubWT(wtType, zfar ? tr("Camera Far Plane") : tr("Camera Near Plane")));
  } else {
    setLayout(layout);
    setWhatsThis(lpubWT(wtType, zfar ? tr("Camera Far Plane") : tr("Camera Near Plane")));
  }

  label = new QLabel(heading.isEmpty() ? zfar ? tr("Z Far Plane") : tr("Z Near Plane") : heading,parent);
  layout->addWidget(label);

  spin = new QDoubleSpinBox(parent);
  spin->setRange(1.0f,90000.0f);
  spin->setSingleStep(1.0f);
  spin->setDecimals(1);
  spin->setValue(data);
  connect(spin,SIGNAL(valueChanged(double)),
          this,  SLOT(valueChanged(double)));
  layout->addWidget(spin);

  button = new QPushButton(parent);
  button->setIcon(QIcon(":/resources/resetaction.png"));
  button->setIconSize(QSize(16,16));
  button->setStyleSheet("QPushButton { background-color: rgba(255,255,255,0);border: 0px; }");
  button->setToolTip(tr("Reset"));
  button->setEnabled(false);
  connect(spin,   SIGNAL(valueChanged(double)),
          this,     SLOT( enableReset(double)));
  connect(button, SIGNAL(  clicked( bool)),
          this,     SLOT(spinReset( bool)));
  layout->addWidget(button);

  QSpacerItem *hSpacer = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Fixed);
  layout->addSpacerItem(hSpacer);

  setEnabled(Preferences::preferredRenderer == RENDERER_NATIVE);
}

void CameraZPlaneGui::enableReset(double value)
{
  button->setEnabled(notEqual(value, data));
}

void CameraZPlaneGui::spinReset(bool)
{
  button->setEnabled(false);
  if (spin) {
    spin->setValue(data);
    spin->setFocus();
    modified = false;
  }
}

void CameraZPlaneGui::valueChanged(double value)
{
  meta->setValue(value);
  modified = notEqual(value, data);
  emit settingsChanged(modified);
}

void CameraZPlaneGui::setEnabled(bool enable)
{
  label->setEnabled(enable);
  spin->setEnabled(enable);
}

void CameraZPlaneGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * CameraDDF
 *
 **********************************************************************/

CameraDDFGui::CameraDDFGui(
  const QString &heading,
  FloatMeta     *_meta,
  QGroupBox     *parent)
{
  meta = _meta;
  data = _meta->value();

  QHBoxLayout *layout = new QHBoxLayout(parent);

  if (parent) {
    parent->setLayout(layout);
    parent->setWhatsThis(lpubWT(WT_GUI_CAMERA_DEFAULT_DISTANCE_FACTOR, parent->title()));
  } else {
    setLayout(layout);
    setWhatsThis(lpubWT(WT_GUI_CAMERA_DEFAULT_DISTANCE_FACTOR, heading.isEmpty() ? tr("Camera Default Distance Factor") : heading));
  }

  label = new QLabel(heading.isEmpty() ? tr("Camera Default Distance Factor") : heading,parent);
  layout->addWidget(label);

  const int residual = data - (int)data;
  const int decimalSize = QString::number(residual).size();
  const int decimalPlaces = decimalSize < 3 ? 2 : decimalSize;

  spin = new QDoubleSpinBox(parent);
  spin->setRange(_meta->_min,_meta->_max);
  spin->setSingleStep(0.01f);
  spin->setDecimals(decimalPlaces);
  spin->setValue(data);
  connect(spin,SIGNAL(valueChanged(double)),
          this,  SLOT(valueChanged(double)));
  layout->addWidget(spin);

  button = new QPushButton(parent);
  button->setIcon(QIcon(":/resources/resetaction.png"));
  button->setIconSize(QSize(16,16));
  button->setStyleSheet("QPushButton { background-color: rgba(255,255,255,0);border: 0px; }");
  button->setToolTip(tr("Reset"));
  button->setEnabled(false);
  connect(spin,   SIGNAL(valueChanged(double)),
          this,     SLOT( enableReset(double)));
  connect(button, SIGNAL(clicked(bool)),
          this,     SLOT(spinReset(bool)));
  layout->addWidget(button);

  QSpacerItem *hSpacer = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Fixed);
  layout->addSpacerItem(hSpacer);
}

void CameraDDFGui::enableReset(double value)
{
  button->setEnabled(notEqual(value, data));
}

void CameraDDFGui::spinReset(bool)
{
  button->setEnabled(false);
  if (spin) {
    spin->setValue(data);
    spin->setFocus();
    modified = false;
  }
}

void CameraDDFGui::valueChanged(double value)
{
  meta->setValue(value);
  modified = notEqual(value, data);
  emit settingsChanged(modified);
}

void CameraDDFGui::setEnabled(bool enable)
{
  label->setEnabled(enable);
  spin->setEnabled(enable);
}

void CameraDDFGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Scale
 *
 **********************************************************************/

ScaleGui::ScaleGui(
  const QString &heading,
  FloatMeta     *_meta,
  QGroupBox     *parent)
{
  meta = _meta;
  data   = _meta->value();

  QHBoxLayout *layout = new QHBoxLayout(parent);

  if (parent) {
    parent->setLayout(layout);
    parent->setWhatsThis(lpubWT(WT_GUI_IMAGE_SCALE, parent->title()));
  } else {
    setLayout(layout);
    setWhatsThis(lpubWT(WT_GUI_IMAGE_SCALE, heading.isEmpty() ? tr("Scale") : heading));
  }

  label = new QLabel(heading.isEmpty() ? tr("Scale") : heading,parent);
  layout->addWidget(label);

  const int residual = data - (int)data;
  const int decimalSize = QString::number(residual).size();
  const int decimalPlaces = decimalSize < 3 ? 2 : decimalSize;

  spin = new QDoubleSpinBox(parent);
  layout->addWidget(spin);
  spin->setRange(_meta->_min,_meta->_max);
  spin->setSingleStep(0.01f);
  spin->setDecimals(decimalPlaces);
  spin->setValue(data);
  connect(spin,SIGNAL(valueChanged(double)),
          this,SLOT  (valueChanged(double)));

  button = new QPushButton(parent);
  button->setIcon(QIcon(":/resources/resetaction.png"));
  button->setIconSize(QSize(16,16));
  button->setStyleSheet("QPushButton { background-color: rgba(255,255,255,0);border: 0px; }");
  button->setToolTip(tr("Reset"));
  button->setEnabled(false);
  connect(spin,   SIGNAL(valueChanged(double)),
          this,     SLOT( enableReset(double)));
  connect(button, SIGNAL(  clicked(bool)),
          this,     SLOT(spinReset(bool)));
  layout->addWidget(button);

  QSpacerItem *hSpacer = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Fixed);
  layout->addSpacerItem(hSpacer);
}

void ScaleGui::enableReset(double value)
{
  button->setEnabled(notEqual(value, data));
}

void ScaleGui::spinReset(bool)
{
  button->setEnabled(false);
  if (spin) {
    spin->setValue(data);
    spin->setFocus();
    modified = false;
  }
}

void ScaleGui::valueChanged(double value)
{
  meta->setValue(value);
  modified = notEqual(value, data);
  emit settingsChanged(modified);
}

void ScaleGui::setEnabled(bool enable)
{
  label->setEnabled(enable);
  spin->setEnabled(enable);
}

void ScaleGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Show Submodel
 *
 **********************************************************************/

ShowSubModelGui::ShowSubModelGui(
    SubModelMeta  *_meta,
    QGroupBox     *parent)
{
  meta = _meta;
  metaValue = *meta;

  QGridLayout *grid = new QGridLayout(parent);

  if (parent) {
    parent->setLayout(grid);
    parent->setWhatsThis(lpubWT(WT_GUI_SUBMODEL_PREVIEW_DISPLAY, parent->title()));
  } else {
    setLayout(grid);
    setWhatsThis(lpubWT(WT_GUI_SUBMODEL_PREVIEW_DISPLAY, tr("Preview Display")));
  }

  showSubmodelsBox = new QCheckBox(tr("Show submodel at first step"),parent);
  showSubmodelsBox->setToolTip(tr("Show Submodel image on first step page"));
  showSubmodelsBox->setChecked(metaValue.show.value());
  connect(showSubmodelsBox,SIGNAL(clicked(bool)),
          this,            SLOT(showSubmodelsChange(bool)));
  grid->addWidget(showSubmodelsBox,0,0,1,2);

  QSettings Settings;
  showSubmodelsDefaultSettings = Settings.contains(QString("%1/%2").arg(SETTINGS,"ShowSubmodels"));
  showSubmodelsDefaultBox = new QCheckBox(tr("Set In Preferences"),parent);
  showSubmodelsDefaultBox->setToolTip(tr("Save show submodel to application settings."));
  showSubmodelsDefaultBox->setChecked(showSubmodelsDefaultSettings);
  grid->addWidget(showSubmodelsDefaultBox,1,0);

  showSubmodelsMetaBox = new QCheckBox(tr("Add LPub Meta Command"),parent);
  showSubmodelsMetaBox->setToolTip(tr("Add show submodel as a global meta command to the LDraw file."));
  showSubmodelsMetaBox->setChecked(!showSubmodelsDefaultSettings);
  grid->addWidget(showSubmodelsMetaBox,1,1);

  QFrame* line = new QFrame();
  line->setFrameShape(QFrame::HLine);
  grid->addWidget(line,2,0,1,2);

  showTopModelBox = new QCheckBox(tr("Show main model at first step"),parent);
  showTopModelBox->setToolTip(tr("Show main model image on first step page"));
  showTopModelBox->setChecked(metaValue.showTopModel.value());
  connect(showTopModelBox,SIGNAL(clicked(bool)),
          this,           SLOT(showTopModelChange(bool)));
  grid->addWidget(showTopModelBox,3,0,1,2);

  showTopModelDefaultSettings = Settings.contains(QString("%1/%2").arg(SETTINGS,"ShowTopModel"));
  showTopModelDefaultBox = new QCheckBox(tr("Set In Preferences"),parent);
  showTopModelDefaultBox->setToolTip(tr("Save show top model to application settings."));
  showTopModelDefaultBox->setChecked(showTopModelDefaultSettings);
  grid->addWidget(showTopModelDefaultBox,4,0);

  showTopModelMetaBox = new QCheckBox(tr("Add LPub Meta Command"),parent);
  showTopModelMetaBox->setToolTip(tr("Add show top model  as a global meta command to the LDraw file."));
  showTopModelMetaBox->setChecked(!showTopModelDefaultSettings);
  grid->addWidget(showTopModelMetaBox,4,1);

  line = new QFrame();
  line->setFrameShape(QFrame::HLine);
  grid->addWidget(line,5,0,1,2);

  showSubmodelInCalloutBox = new QCheckBox(tr("Show submodel in callout"),parent);
  showSubmodelInCalloutBox->setToolTip(tr("Show Submodel image in callout"));
  showSubmodelInCalloutBox->setChecked(metaValue.showSubmodelInCallout.value());
  connect(showSubmodelInCalloutBox,SIGNAL(clicked(bool)),
          this,            SLOT(showSubmodelInCalloutChange(bool)));
  grid->addWidget(showSubmodelInCalloutBox,6,0,1,2);

  showSubmodelInCalloutDefaultSettings = Settings.contains(QString("%1/%2").arg(SETTINGS,"ShowSubmodelInCallout"));
  showSubmodelInCalloutDefaultBox = new QCheckBox(tr("Set In Preferences"),parent);
  showSubmodelInCalloutDefaultBox->setToolTip(tr("Save show submodel in callout to application settings."));
  showSubmodelInCalloutDefaultBox->setChecked(showSubmodelInCalloutDefaultSettings);
  grid->addWidget(showSubmodelInCalloutDefaultBox,7,0);

  showSubmodelInCalloutMetaBox = new QCheckBox(tr("Add LPub Meta Command"),parent);
  showSubmodelInCalloutMetaBox->setToolTip(tr("Add show submodel in callout as a global meta command to the LDraw file."));
  showSubmodelInCalloutMetaBox->setChecked(!showSubmodelInCalloutDefaultSettings);
  grid->addWidget(showSubmodelInCalloutMetaBox,7,1);

  line = new QFrame();
  line->setFrameShape(QFrame::HLine);
  grid->addWidget(line,8,0,1,2);

  showInstanceCountBox = new QCheckBox(tr("Show submodel instance count"),parent);
  showInstanceCountBox->setToolTip(tr("Show Submodel instance count"));
  showInstanceCountBox->setChecked(metaValue.showInstanceCount.value());
  connect(showInstanceCountBox,SIGNAL(clicked(bool)),
          this,                SLOT(showInstanceCountChange(bool)));
  connect(showInstanceCountBox,SIGNAL(clicked(bool)),
          this,                SIGNAL(instanceCountClicked(bool)));
  grid->addWidget(showInstanceCountBox,9,0,1,2);

  showInstanceCountDefaultSettings = Settings.contains(QString("%1/%2").arg(SETTINGS,"ShowInstanceCount"));
  showInstanceCountDefaultBox = new QCheckBox("Set In Preferences",parent);
  showInstanceCountDefaultBox->setToolTip(tr("Save show submodel instance count to application settings."));
  showInstanceCountDefaultBox->setChecked(showInstanceCountDefaultSettings);
  grid->addWidget(showInstanceCountDefaultBox,10,0);

  showInstanceCountMetaBox = new QCheckBox(tr("Add LPub Meta Command"),parent);
  showInstanceCountMetaBox->setToolTip(tr("Add show submodel instance count as a global meta command to the LDraw file."));
  showInstanceCountMetaBox->setChecked(!showInstanceCountDefaultSettings);
  grid->addWidget(showInstanceCountMetaBox,10,1);

  line = new QFrame();
  line->setFrameShape(QFrame::HLine);
  grid->addWidget(line,11,0,1,2);

  const QString placementName = PlacementDialog::placementTypeName(SubModelType);
  QLabel *placementLabel = new QLabel(tr("%1 Placement").arg(placementName),parent);
  grid->addWidget(placementLabel,12,0);

  placementButton = new QPushButton(tr("Change %1 Placement").arg(placementName),parent);
  PlacementData placementData = metaValue.placement.value();
  placementButton->setWhatsThis(commonMenus.naturalLanguagePlacementWhatsThis(SubModelType,placementData,placementButton->text()));
  placementButton->setToolTip(tr("Set %1 default placement").arg(placementName));
  connect(placementButton,SIGNAL(         clicked(bool)),
          this,             SLOT(placementChanged(bool)));
  grid->addWidget(placementButton,12,1);

  showSubmodelsModified         = false;
  showTopModelModified          = false;
  showSubmodelInCalloutModified = false;
  showInstanceCountModified     = false;
  placementModified             = false;

  enableSubmodelControls(metaValue.show.value());
}

void ShowSubModelGui::showSubmodelsChange(bool checked)
{
  meta->show.setValue(checked);
  showSubmodelsModified = metaModified(meta->show.value() != metaValue.show.value());
  enableSubmodelControls(checked);
}

void ShowSubModelGui::showTopModelChange(bool checked)
{
  meta->showTopModel.setValue(checked);
  showTopModelModified = metaModified(meta->showTopModel.value() != metaValue.showTopModel.value());
}

void ShowSubModelGui::showSubmodelInCalloutChange(bool checked)
{
  meta->showSubmodelInCallout.setValue(checked);
  showSubmodelInCalloutModified = metaModified(meta->showSubmodelInCallout.value() != metaValue.showSubmodelInCallout.value());
}

void ShowSubModelGui::showInstanceCountChange(bool checked)
{
  meta->showInstanceCount.setValue(checked);
  showInstanceCountModified = metaModified(meta->showInstanceCount.value() != metaValue.showInstanceCount.value());
}

void ShowSubModelGui::placementChanged(bool)
{
  PlacementData pd = meta->placement.value();
  bool ok = PlacementDialog::getPlacement(SingleStepType,SubModelType,pd,tr("Submodel Placement"));
  if (ok) {
    meta->placement.setValue(pd);
    PlacementData pdv = meta->placement.value();
    placementModified = (
      pd.placement     != pdv.placement       ||
      pd.justification != pdv.justification   ||
      pd.preposition   != pdv.preposition     ||
      pd.relativeTo    != pdv.relativeTo      ||
      pd.rectPlacement != pdv.rectPlacement   ||
      notEqual(pd.offsets[0], pdv.offsets[0]) ||
      notEqual(pd.offsets[1], pdv.offsets[1]));
    metaModified(placementModified);
  }
}

void ShowSubModelGui::enableSubmodelControls(bool checked)
{
  showSubmodelsDefaultBox->setEnabled(checked);
  showSubmodelsMetaBox->setEnabled(checked);

  showTopModelBox->setEnabled(checked);
  showTopModelDefaultBox->setEnabled(checked);
  showTopModelMetaBox->setEnabled(checked);

  showSubmodelInCalloutBox->setEnabled(checked);
  showSubmodelInCalloutDefaultBox->setEnabled(checked);
  showSubmodelInCalloutMetaBox->setEnabled(checked);

  showInstanceCountBox->setEnabled(checked);
  showInstanceCountDefaultBox->setEnabled(checked);
  showInstanceCountMetaBox->setEnabled(checked);

  placementButton->setEnabled(checked);
}

void ShowSubModelGui::applySettings()
{
  QSettings Settings;
  QString changeMessage;
  if (showSubmodelsModified) {
    changeMessage = tr("Show submodels is %1")
        .arg(meta->show.value() ? "ON" : "OFF");
    emit gui->messageSig(LOG_INFO, changeMessage);
    if (showSubmodelsDefaultBox->isChecked()) {
      changeMessage = tr("Show submodels added as application default.");
      emit gui->messageSig(LOG_INFO, changeMessage);
      Preferences::showSubmodels = meta->show.value();
      Settings.setValue(QString("%1/%2").arg(SETTINGS,"ShowSubmodels"),meta->show.value());
    }
    else
    if (showSubmodelsDefaultSettings) {
      Settings.remove(QString("%1/%2").arg(SETTINGS,"ShowSubmodels"));
    }
  }
  if (showTopModelModified) {
    changeMessage = tr("Show top model is %1")
        .arg(meta->showTopModel.value() ? "ON" : "OFF");
    emit gui->messageSig(LOG_INFO, changeMessage);
    if (showTopModelDefaultBox->isChecked()) {
      changeMessage = tr("Show top model added as application default.");
      emit gui->messageSig(LOG_INFO, changeMessage);
      Preferences::showTopModel = meta->showTopModel.value();
      Settings.setValue(QString("%1/%2").arg(SETTINGS,"ShowTopModel"),meta->showTopModel.value());
    }
    else
    if (showTopModelDefaultSettings) {
      Settings.remove(QString("%1/%2").arg(SETTINGS,"ShowTopModel"));
    }
  }
  if (showSubmodelInCalloutModified) {
    changeMessage = tr("Show submodel in callout is %1")
        .arg(meta->showSubmodelInCallout.value() ? "ON" : "OFF");
    emit gui->messageSig(LOG_INFO, changeMessage);
    if (showSubmodelInCalloutDefaultBox->isChecked()) {
      changeMessage = tr("Show submodel in callout added as application default.");
      emit gui->messageSig(LOG_INFO, changeMessage);
      Preferences::showSubmodelInCallout = meta->showSubmodelInCallout.value();
      Settings.setValue(QString("%1/%2").arg(SETTINGS,"ShowSubmodelInCallout"),meta->showSubmodelInCallout.value());
    }
    else
    if (showSubmodelInCalloutDefaultSettings) {
      Settings.remove(QString("%1/%2").arg(SETTINGS,"ShowSubmodelInCallout"));
    }
  }
  if (showInstanceCountModified) {
    changeMessage = tr("Show submodel instance count is %1")
        .arg(meta->showInstanceCount.value() ? "ON" : "OFF");
    emit gui->messageSig(LOG_INFO, changeMessage);
    if (showInstanceCountDefaultBox->isChecked()) {
      changeMessage = tr("Show submodel instance count added as application default.");
      emit gui->messageSig(LOG_INFO, changeMessage);
      Preferences::showInstanceCount = meta->showInstanceCount.value();
      Settings.setValue(QString("%1/%2").arg(SETTINGS,"ShowInstanceCount"),meta->showInstanceCount.value());
    }
    else
    if (showInstanceCountDefaultSettings) {
      Settings.remove(QString("%1/%2").arg(SETTINGS,"ShowInstanceCount"));
    }
  }
}

void ShowSubModelGui::apply(QString &topLevelFile)
{
  if (modified) {
    applySettings();
    MetaItem mi;
    if (showSubmodelsModified) {
      mi.setGlobalMeta(topLevelFile,&meta->show);
    }
    if (showTopModelModified) {
      mi.setGlobalMeta(topLevelFile,&meta->showTopModel);
    }
    if (showSubmodelInCalloutModified) {
      mi.setGlobalMeta(topLevelFile,&meta->showSubmodelInCallout);
    }
    if (showInstanceCountModified) {
      mi.setGlobalMeta(topLevelFile,&meta->showInstanceCount);
    }
    if (placementModified) {
      mi.setGlobalMeta(topLevelFile,&meta->placement);
    }
  }
}

/***********************************************************************
 *
 * PliSort [DEPRECATED]
 *
 **********************************************************************/

PliSortGui::PliSortGui(
  const QString   &heading,
  PliSortMeta     *_meta,
  QGroupBox       *parent,
  bool             bom)
{
  meta = _meta;

  QGridLayout *grid = new QGridLayout(parent);

  if (parent) {
    parent->setLayout(grid);
    parent->setWhatsThis(lpubWT(WT_GUI_PART_SORT, parent->title()));
  } else {
    setLayout(grid);
    setWhatsThis(lpubWT(WT_GUI_PART_SORT, heading.isEmpty() ? tr("Pli Sort") : heading));
  }

  if (heading != "") {
    headingLabel = new QLabel(heading,parent);
    grid->addWidget(headingLabel);
  } else {
    headingLabel = nullptr;
  }

  QLabel      *label;
  label = new QLabel("Sort By",parent);
  grid->addWidget(label,0,0);

  int currentIndex;
  sortOption  = meta->sortOption.value();
  sortOption == SortOptionName[PartSize]     ? currentIndex = PartSize :
  sortOption == SortOptionName[PartColour]   ? currentIndex = PartColour :
  sortOption == SortOptionName[PartCategory] ? currentIndex = PartCategory :
                                         bom ? currentIndex = PartElement :
                                               currentIndex = PartSize;

  combo = new QComboBox(parent);
  combo->addItem(SortOptionName[PartSize]);
  combo->addItem(SortOptionName[PartColour]);
  combo->addItem(SortOptionName[PartCategory]);
  if (bom)
      combo->addItem(SortOptionName[PartElement]);
  combo->setCurrentIndex(currentIndex);
  connect(combo,SIGNAL(currentTextChanged(const QString&)),
          this,   SLOT(       optionChange(const QString&)));
  grid->addWidget(combo,0,1);

  modified = false;
}

void PliSortGui::optionChange(const QString &sortOption)
{
  StringMeta sortBy = meta->sortOption;
  sortBy.setValue(sortOption);
  meta->sortOption.setValue(sortBy.value());
  modified = true;
}

void PliSortGui::apply(QString &topLevelFile)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(topLevelFile,&meta->sortOption);
  }
}

/***********************************************************************
 *
 * PliSortOrder
 *
 **********************************************************************/

PliSortOrderGui::PliSortOrderGui(
  const QString     &heading,
  PliSortOrderMeta *_meta,
  QGroupBox         *parent,
  bool              _bom)
{
  meta = _meta;
  metaValue = *meta;

  bom = _bom;

  QHBoxLayout *hLayout;
  QGridLayout *grid = new QGridLayout(parent);

  if (parent) {
    parent->setLayout(grid);
    parent->setWhatsThis(lpubWT(WT_GUI_PART_SORT_ORDER, parent->title()));
  } else {
    setLayout(grid);
    setWhatsThis(lpubWT(WT_GUI_PART_SORT_ORDER, heading.isEmpty() ? tr("Sort Order and Direction") : heading));
  }

  if (heading != "") {
    headingLabel = new QLabel(heading,parent);
    grid->addWidget(headingLabel);
  } else {
    headingLabel = nullptr;
  }

  int priSort = tokenMap[metaValue.primary.value()];
  int secSort = tokenMap[metaValue.secondary.value()];
  int triSort = tokenMap[metaValue.tertiary.value()];

  // Direction Groups
  gbPrimary = new QGroupBox(tr("Primary"),parent);
  gbPrimary->setEnabled(priSort != NoSort);
  hLayout = new QHBoxLayout();
  gbPrimary->setLayout(hLayout);
  grid->addWidget(gbPrimary,0,0);

  primaryAscendingRadio = new QRadioButton(tr("Asc"),gbPrimary);
  primaryAscendingRadio->setToolTip(tr("Sort Ascending"));
  primaryAscendingRadio->setChecked(tokenMap[metaValue.primaryDirection.value()] == SortAscending);
  hLayout->addWidget(primaryAscendingRadio);
  connect(primaryAscendingRadio,SIGNAL(clicked(bool)),
          this,                 SLOT(  directionChange(bool)));

  primaryDescendingRadio = new QRadioButton(tr("Des"),gbPrimary);
  primaryDescendingRadio->setToolTip(tr("Sort Descending"));
  primaryDescendingRadio->setChecked(tokenMap[metaValue.primaryDirection.value()] == SortDescending);
  hLayout->addWidget(primaryDescendingRadio);
  connect(primaryDescendingRadio,SIGNAL(clicked(bool)),
          this,                  SLOT(  directionChange(bool)));

  gbSecondary = new QGroupBox(tr("Secondary"),parent);
  gbSecondary->setEnabled(secSort != NoSort);
  hLayout = new QHBoxLayout();
  gbSecondary->setLayout(hLayout);
  grid->addWidget(gbSecondary,0,1);

  secondaryAscendingRadio = new QRadioButton(tr("Asc"),gbSecondary);
  secondaryAscendingRadio->setToolTip(tr("Sort Ascending"));
  secondaryAscendingRadio->setChecked(tokenMap[metaValue.secondaryDirection.value()] == SortAscending);
  hLayout->addWidget(secondaryAscendingRadio);
  connect(secondaryAscendingRadio,SIGNAL(clicked(bool)),
          this,                   SLOT(  directionChange(bool)));

  secondaryDescendingRadio = new QRadioButton(tr("Des"),gbSecondary);
  secondaryDescendingRadio->setToolTip(tr("Sort Descending"));
  secondaryDescendingRadio->setChecked(tokenMap[metaValue.secondaryDirection.value()] == SortDescending);
  hLayout->addWidget(secondaryDescendingRadio);
  connect(secondaryDescendingRadio,SIGNAL(clicked(bool)),
          this,                    SLOT(  directionChange(bool)));

  gbTertiary = new QGroupBox(tr("Tertiary"),parent);
  gbTertiary->setEnabled(triSort != NoSort);
  hLayout = new QHBoxLayout();
  gbTertiary->setLayout(hLayout);
  grid->addWidget(gbTertiary,0,2);

  tertiaryAscendingRadio = new QRadioButton(tr("Asc"),gbTertiary);
  tertiaryAscendingRadio->setToolTip(tr("Sort Ascending"));
  tertiaryAscendingRadio->setChecked(tokenMap[metaValue.tertiaryDirection.value()] == SortAscending);
  hLayout->addWidget(tertiaryAscendingRadio);
  connect(tertiaryAscendingRadio,SIGNAL(clicked(bool)),
          this,                  SLOT(  directionChange(bool)));

  tertiaryDescendingRadio = new QRadioButton(tr("Des"),gbTertiary);
  tertiaryDescendingRadio->setToolTip(tr("Sort Descending"));
  tertiaryDescendingRadio->setChecked(tokenMap[metaValue.tertiaryDirection.value()] == SortDescending);
  hLayout->addWidget(tertiaryDescendingRadio);
  connect(tertiaryDescendingRadio,SIGNAL(clicked(bool)),
          this,                   SLOT(  directionChange(bool)));

  primaryCombo   = new QComboBox(parent);
  secondaryCombo = new QComboBox(parent);
  tertiaryCombo  = new QComboBox(parent);

  const QList<QComboBox *> cbl = {primaryCombo, secondaryCombo, tertiaryCombo};
  for (const auto cb : cbl) {
    for (const auto &so : SortOptionName) {
      if (so == SortOptionName[PartElement] && !bom)
        continue;
      cb->addItem(so);
    }
  }

  // adjust indices for missing PartElement option when not bom
  auto OptionIndex = [this](const QString &value) {
    int opt = tokenMap[value];
    return bom ? opt : opt == NoSort ? PartElement : opt;
  };

  primaryCombo->setCurrentIndex(OptionIndex(metaValue.primary.value()));
  connect(primaryCombo,SIGNAL(currentIndexChanged(int)),
          this,          SLOT(        orderChange(int)));
  grid->addWidget(primaryCombo,1,0);

  secondaryCombo->setCurrentIndex(OptionIndex(metaValue.secondary.value()));
  connect(secondaryCombo,SIGNAL(currentIndexChanged(int)),
          this,            SLOT(        orderChange(int)));
  grid->addWidget(secondaryCombo,1,1);

  tertiaryCombo->setCurrentIndex(OptionIndex(metaValue.tertiary.value()));
  connect(tertiaryCombo,SIGNAL(currentIndexChanged(int)),
          this,           SLOT(        orderChange(int)));
  grid->addWidget(tertiaryCombo,1,2);

  if (metaValue.tertiary.value()  == SortOptionName[NoSort]) {
    gbTertiary->setEnabled(false);
  }
  if (metaValue.secondary.value() == SortOptionName[NoSort]) {
    tertiaryCombo->setEnabled(false);
    gbSecondary->setEnabled(false);
    gbTertiary->setEnabled(false);
  }
  if (metaValue.primary.value()   == SortOptionName[NoSort]) {
    secondaryCombo->setEnabled(false);
    tertiaryCombo->setEnabled(false);
    gbPrimary->setEnabled(false);
    gbSecondary->setEnabled(false);
    gbTertiary->setEnabled(false);
  }

  primaryDuplicateOption     = false;
  secondaryDuplicateOption   = false;
  tertiaryDuplicateOption    = false;

  primaryModified            = false;
  secondaryModified          = false;
  tertiaryModified           = false;
  primaryDirectionModified   = false;
  secondaryDirectionModified = false;
  tertiaryDirectionModified  = false;
}

void PliSortOrderGui::duplicateOption(
    QComboBox *combo,
    bool resetOption,
    bool resetText)
{
  if (resetOption) {
    resetOption = false;
    if (primaryDuplicateOption) {
      primaryDuplicateOption = resetOption;
      duplicateOption(primaryCombo,resetOption,true);
    }
    if (secondaryDuplicateOption) {
      secondaryDuplicateOption = resetOption;
      duplicateOption(secondaryCombo,resetOption,true);
    }
    if (tertiaryDuplicateOption) {
      tertiaryDuplicateOption = resetOption;
      duplicateOption(tertiaryCombo,resetOption,true);
    }
    return;
  }

  QFont comboFont = combo->font();
  if (resetText) {
    comboFont.setBold(false);
    combo->setFont(comboFont);
    combo->setPalette(QApplication::palette(combo));
    return;
  }

  QPalette comboPalette = combo->palette();
  QModelIndex index = ((QTreeView *)combo->view())->currentIndex();
  if(!index.parent().isValid()) // parent index
  {
    comboFont.setBold(true);
    comboPalette.setColor(QPalette::Text, Qt::red);
    comboPalette.setColor(QPalette::WindowText, Qt::red);
  }
  else
  {
    comboFont.setBold(false);
    if(combo->parentWidget() != nullptr)
      comboPalette = combo->parentWidget()->palette();
  }
  combo->setFont(comboFont);
  combo->setPalette(comboPalette);
}

void PliSortOrderGui::orderChange(int option)
{
  bool enable;
  // adjust indices for missing PartElement option when not bom
  int NoSortIndex = bom ? NoSort : PartElement;
  int OptionIndex = bom ? option : option == PartElement ? NoSort : option;
  if (sender() == primaryCombo) {
    enable = option != NoSortIndex;
    meta->primary.setValue(SortOptionName[OptionIndex]);
    secondaryCombo->setEnabled(enable);
    tertiaryCombo->setEnabled(enable);
    gbPrimary->setEnabled(enable);
    gbSecondary->setEnabled(enable);
    gbTertiary->setEnabled(enable);
    duplicateOption(tertiaryCombo,true);
    if ((primaryDuplicateOption =
         secondaryCombo->currentIndex() == option ||
         tertiaryCombo->currentIndex() == option) &&
        option != NoSortIndex) {
      duplicateOption(primaryCombo);
    }
    primaryModified = metaModified(meta->primary.value() != metaValue.primary.value());
  }
  if (sender() == secondaryCombo) {
    enable = (option != NoSortIndex &&
        primaryCombo->currentIndex() != NoSortIndex);
    meta->secondary.setValue(SortOptionName[OptionIndex]);
    tertiaryCombo->setEnabled(enable);
    gbSecondary->setEnabled(enable);
    gbTertiary->setEnabled(enable);
    duplicateOption(tertiaryCombo,true);
    if ((secondaryDuplicateOption =
         primaryCombo->currentIndex() == option ||
         tertiaryCombo->currentIndex() == option) &&
        option != NoSortIndex) {
      duplicateOption(secondaryCombo);
    }
    secondaryModified = metaModified(meta->secondary.value() != metaValue.secondary.value());
  }
  if (sender() == tertiaryCombo) {
    enable = (option != NoSortIndex &&
        primaryCombo->currentIndex()   != NoSortIndex &&
        secondaryCombo->currentIndex() != NoSortIndex);
    meta->tertiary.setValue(SortOptionName[OptionIndex]);
    gbTertiary->setEnabled(enable);
    duplicateOption(tertiaryCombo,true);
    if ((tertiaryDuplicateOption =
         (primaryCombo->currentIndex() == option ||
          secondaryCombo->currentIndex() == option) &&
         option != NoSortIndex)) {
      duplicateOption(tertiaryCombo);
    }
    tertiaryModified = metaModified(meta->tertiary.value() != metaValue.tertiary.value());
  }
}

void PliSortOrderGui::directionChange(bool)
{
  if (sender() == primaryAscendingRadio) {
    meta->primaryDirection.setValue(SortDirectionName[SortAscending]);
    primaryDirectionModified = metaModified(meta->primaryDirection.value() != metaValue.primaryDirection.value());
  }
  if (sender() == primaryDescendingRadio) {
    meta->primaryDirection.setValue(SortDirectionName[SortDescending]);
    primaryDirectionModified = metaModified(meta->primaryDirection.value() != metaValue.primaryDirection.value());
  }

  if (sender() == secondaryAscendingRadio) {
    meta->secondaryDirection.setValue(SortDirectionName[SortAscending]);
    secondaryDirectionModified = metaModified(meta->secondaryDirection.value() != metaValue.secondaryDirection.value());
  }
  if (sender() == secondaryDescendingRadio) {
    meta->secondaryDirection.setValue(SortDirectionName[SortDescending]);
    secondaryDirectionModified = metaModified(meta->secondaryDirection.value() != metaValue.secondaryDirection.value());
  }

  if (sender() == tertiaryAscendingRadio) {
    meta->tertiaryDirection.setValue(SortDirectionName[SortAscending]);
    tertiaryDirectionModified = metaModified(meta->tertiaryDirection.value() != metaValue.tertiaryDirection.value());
  }
  if (sender() == tertiaryDescendingRadio) {
    meta->tertiaryDirection.setValue(SortDirectionName[SortDescending]);
    tertiaryDirectionModified = metaModified(meta->tertiaryDirection.value() != metaValue.tertiaryDirection.value());
  }
}

void PliSortOrderGui::apply(QString &topLevelFile)
{
  if (modified) {
    MetaItem mi;
    if (tertiaryDirectionModified) {
      mi.setGlobalMeta(topLevelFile,&meta->tertiaryDirection);
    }
    if (secondaryDirectionModified) {
      mi.setGlobalMeta(topLevelFile,&meta->secondaryDirection);
    }
    if (primaryDirectionModified) {
      mi.setGlobalMeta(topLevelFile,&meta->primaryDirection);
    }
    if (tertiaryModified) {
      mi.setGlobalMeta(topLevelFile,&meta->tertiary);
    }
    if (secondaryModified) {
      mi.setGlobalMeta(topLevelFile,&meta->secondary);
    }
    if (primaryModified) {
      mi.setGlobalMeta(topLevelFile,&meta->primary);
    }
  }
}

/***********************************************************************
 *
 * PliPartElements
 *
 **********************************************************************/

PliPartElementGui::PliPartElementGui(
    const QString      &heading,
    PliPartElementMeta *_meta,
    QGroupBox          *parent)
{
  meta = _meta;
  metaValue = *meta;

  QVBoxLayout *vLayout = new QVBoxLayout(parent);
  QHBoxLayout *hLayout = new QHBoxLayout(nullptr);

  if (parent) {
    parent->setLayout(vLayout);
  } else {
    setLayout(vLayout);
  }
  if (parent) {
    parent->setLayout(vLayout);
    parent->setWhatsThis(lpubWT(WT_GUI_PART_ELEMENTS_BOM, parent->title()));
  } else {
    setLayout(vLayout);
    setWhatsThis(lpubWT(WT_GUI_PART_ELEMENTS_BOM, heading.isEmpty() ? tr("Part Elements") : heading));
  }

  if (heading != "") {
      headingLabel = new QLabel(heading,parent);
      vLayout->addWidget(headingLabel);
  } else {
      headingLabel = nullptr;
  }

  //PLIAnnotation
  gbPliPartElement = new QGroupBox(tr("Display Part Element Annotation"),parent);
  gbPliPartElement->setCheckable(true);
  gbPliPartElement->setChecked(metaValue.display.value());
  gbPliPartElement->setLayout(hLayout);
  vLayout->addWidget(gbPliPartElement);
  connect(gbPliPartElement,SIGNAL(toggled(bool)),
          this,            SIGNAL(toggled(bool)));
  connect(gbPliPartElement,SIGNAL(toggled(bool)),
          this,            SLOT(  gbToggled(bool)));

  bricklinkElementsButton = new QRadioButton(tr("BrickLink"),gbPliPartElement);
  bricklinkElementsButton->setChecked(metaValue.bricklinkElements.value());
  bricklinkElementsButton->setToolTip(tr("Use BrickLink element identification"));
  connect(bricklinkElementsButton,SIGNAL(clicked(bool)),
          this,                   SLOT(  bricklinkElements(bool)));
  hLayout->addWidget(bricklinkElementsButton);

  legoElementsButton = new QRadioButton(tr("LEGO"),gbPliPartElement);
  legoElementsButton->setChecked(metaValue.legoElements.value());
  legoElementsButton->setToolTip(tr("Use LEGO element identification"));
  connect(legoElementsButton,SIGNAL(clicked(bool)),
          this,              SLOT(  legoElements(bool)));
  hLayout->addWidget(legoElementsButton);

  userElementsCheck = new QCheckBox(tr("User Defined"),gbPliPartElement);
  // localElements checked for backwards compatability
  if (metaValue.localElements.value() && !metaValue.userElements.value())
      meta->userElements.setValue(metaValue.localElements.value());
  userElementsCheck->setChecked(metaValue.userElements.value());
  userElementsCheck->setToolTip(tr("Use user-defined part element id file userelements.lst."));
  connect(userElementsCheck,SIGNAL(clicked(bool)),
          this,                  SLOT(  userElements(bool)));
  hLayout->addWidget(userElementsCheck);

  userElementsLDrawKeyCheck = new QCheckBox(tr("LDraw Key"),gbPliPartElement);
  userElementsLDrawKeyCheck->setChecked(metaValue.userElementsLDrawKey.value());
  userElementsLDrawKeyCheck->setEnabled(metaValue.userElements.value());
  userElementsLDrawKeyCheck->setToolTip(tr("Use LDraw Part Type and Color ID for user-defined part elements, Otherwise use Bricklink Item No and Color."));
  connect(userElementsLDrawKeyCheck,SIGNAL(clicked(bool)),
          this,                     SLOT(  userElementsLDrawKey(bool)));
  hLayout->addWidget(userElementsLDrawKeyCheck);

  displayModified              = false;
  bricklinkElementsModified    = false;
  legoElementsModified         = false;
  userElementsModified         = false;
  userElementsLDrawKeyModified = false;
}

void PliPartElementGui::bricklinkElements(bool checked)
{
  meta->bricklinkElements.setValue(checked);
  meta->legoElements.setValue(! checked);
  userElementsCheck->setDisabled(checked);
  bricklinkElementsModified = metaModified(checked != metaValue.bricklinkElements.value());
}

void PliPartElementGui::legoElements(bool checked)
{
  meta->bricklinkElements.setValue(! checked);
  meta->legoElements.setValue( checked);
  userElementsCheck->setEnabled(checked);
  legoElementsModified = metaModified(checked != metaValue.bricklinkElements.value());
}

void PliPartElementGui::userElements(bool checked)
{
  meta->userElements.setValue( checked);
  userElementsLDrawKeyCheck->setEnabled(checked);
  userElementsModified = metaModified(checked != metaValue.bricklinkElements.value());
}

void PliPartElementGui::userElementsLDrawKey(bool checked)
{
  meta->userElementsLDrawKey.setValue(checked);
  userElementsLDrawKeyModified = metaModified(checked != metaValue.bricklinkElements.value());
}

void PliPartElementGui::gbToggled(bool toggled)
{
  meta->display.setValue(toggled);
  if(toggled) {
    bricklinkElementsButton->setChecked(meta->bricklinkElements.value());
    legoElementsButton->setChecked(meta->legoElements.value());
    userElementsCheck->setChecked(meta->userElements.value());
  }
  userElementsCheck->setEnabled(toggled && legoElementsButton->isChecked());
  displayModified = metaModified(toggled != metaValue.display.value());
}

void PliPartElementGui::enablePliPartElementGroup(bool checked)
{
  gbPliPartElement->setEnabled(checked);
}

void PliPartElementGui::apply(QString &topLevelFile)
{
  if (modified) {
    MetaItem mi;
    if (displayModified) {
      mi.setGlobalMeta(topLevelFile,&meta->display);
    }
    if (bricklinkElementsModified) {
      mi.setGlobalMeta(topLevelFile,&meta->bricklinkElements);
    }
    if (legoElementsModified) {
      mi.setGlobalMeta(topLevelFile,&meta->legoElements);
    }
    if (userElementsLDrawKeyModified) {
      mi.setGlobalMeta(topLevelFile,&meta->userElementsLDrawKey);
    }
    if (userElementsModified) {
      // kept localElements for backwards compatability
      if (meta->localElements.value()) {
        QString const message(tr("LPUB command LOCAL_LEGO_ELEMENTS_FILE is deprecated,<br>"
                                 "use USER_ELEMENTS_FILE instead."));
        Where annotationFile(topLevelFile);
        Annotations::annotationMessage(message, annotationFile, false, true);
      }
      if (meta->localElements.value() && !meta->userElements.value()) {
        meta->localElements.setValue(meta->userElements.value());
        mi.setGlobalMeta(topLevelFile,&meta->localElements);
      } else {
        mi.setGlobalMeta(topLevelFile,&meta->userElements);
      }
    }
  }
}

/***********************************************************************
 *
 * PliAnnotation
 *
 **********************************************************************/

PliAnnotationGui::PliAnnotationGui(
    const QString     &heading,
    PliAnnotationMeta *_meta,
    QGroupBox         *parent,
    bool               bom)
{
  meta = _meta;
  metaValue = *meta;

  QGridLayout *grid = new QGridLayout(parent);
  QHBoxLayout *hLayout = new QHBoxLayout(nullptr);

  if (parent) {
    parent->setLayout(grid);
  } else {
    setLayout(grid);
  }

  if (parent) {
    parent->setLayout(grid);
    parent->setWhatsThis(lpubWT(WT_GUI_PART_ANNOTATION_OPTIONS, parent->title()));
  } else {
    setLayout(grid);
    setWhatsThis(lpubWT(WT_GUI_PART_ANNOTATION_OPTIONS, heading.isEmpty() ? tr("Annotation Options") : heading));
  }

  if (heading != "") {
    headingLabel = new QLabel(heading,parent);
    grid->addWidget(headingLabel);
  } else {
    headingLabel = nullptr;
  }

  //PLI Annotation Source
  gbPLIAnnotationSource = new QGroupBox(tr("Display %1 Annotation Source").arg(bom ? tr("Bill Of Materials (BOM)") : tr("Part List (PLI)") ),parent);
  gbPLIAnnotationSource->setWhatsThis(lpubWT(WT_GUI_PART_ANNOTATIONS_SOURCE, gbPLIAnnotationSource->title()));
  gbPLIAnnotationSource->setCheckable(true);
  gbPLIAnnotationSource->setChecked(metaValue.display.value());
  gbPLIAnnotationSource->setLayout(hLayout);
  grid->addWidget(gbPLIAnnotationSource,0,0);
  connect(gbPLIAnnotationSource,SIGNAL(toggled(bool)),
          this,           SIGNAL(toggled(bool)));
  connect(gbPLIAnnotationSource,SIGNAL(toggled(bool)),
          this,           SLOT(  gbToggled(bool)));

  const bool titleAndFreeForm = metaValue.titleAndFreeformAnnotation.value();

  titleAnnotationCheck = new QCheckBox(tr("Title"),gbPLIAnnotationSource);
  titleAnnotationCheck->setChecked(titleAndFreeForm ? true : metaValue.titleAnnotation.value());
  titleAnnotationCheck->setToolTip(tr("Extended background style shape annotations - user configurable"));
  connect(titleAnnotationCheck,SIGNAL(clicked(bool)),
          this,                 SLOT(  titleAnnotation(bool)));
  connect(titleAnnotationCheck,SIGNAL(clicked()),
          this,                 SLOT(  enableAnnotations()));
  connect(titleAnnotationCheck,SIGNAL(clicked()),
          this,                 SLOT(  enableExtendedStyle()));
  hLayout->addWidget(titleAnnotationCheck);

  freeformAnnotationCheck = new QCheckBox(tr("Free Form"),gbPLIAnnotationSource);
  freeformAnnotationCheck->setChecked(titleAndFreeForm ? true : metaValue.freeformAnnotation.value());
  freeformAnnotationCheck->setToolTip(tr("Extended background style shape annotations - user configurable"));
  connect(freeformAnnotationCheck,SIGNAL(clicked(bool)),
          this,                    SLOT(  freeformAnnotation(bool)));
  connect(freeformAnnotationCheck,SIGNAL(clicked()),
          this,                    SLOT(  enableAnnotations()));
  connect(freeformAnnotationCheck,SIGNAL(clicked()),
          this,                    SLOT(  enableExtendedStyle()));
  hLayout->addWidget(freeformAnnotationCheck);

  fixedAnnotationsCheck = new QCheckBox(tr("Fixed"),gbPLIAnnotationSource);
  fixedAnnotationsCheck->setChecked(metaValue.fixedAnnotations.value());
  fixedAnnotationsCheck->setToolTip(tr("Fixed background style shape annotations - axle, beam, cable, connector, hose and panel."));
  connect(fixedAnnotationsCheck,SIGNAL(clicked(bool)),
          this,                 SLOT(  fixedAnnotations(bool)));
  connect(fixedAnnotationsCheck,SIGNAL(clicked()),
          this,                 SLOT(  enableAnnotations()));
  hLayout->addWidget(fixedAnnotationsCheck);

  const bool styleEnabled = metaValue.enableStyle.value();

  WT_Type wtType = bom ? WT_GUI_PART_ANNOTATIONS_TYPE_BOM : WT_GUI_PART_ANNOTATIONS_TYPE_PLI;

  // PLI Annotation Display Options
  gbPLIAnnotationType = new QGroupBox(tr("Display Annotation Type"),parent);
  gbPLIAnnotationType->setWhatsThis(lpubWT(wtType, gbPLIAnnotationType->title()));
  QGridLayout *sgrid = new QGridLayout();
  gbPLIAnnotationType->setLayout(sgrid);
  grid->addWidget(gbPLIAnnotationType,1,0);
  gbPLIAnnotationType->setCheckable(true);
  gbPLIAnnotationType->setChecked(styleEnabled);
  gbPLIAnnotationType->setEnabled(metaValue.display.value());
  connect(gbPLIAnnotationType,SIGNAL(toggled(bool)),
          this,                SLOT(gbStyleToggled(bool)));

  axleStyleCheck = new QCheckBox(tr("Axles"),gbPLIAnnotationType);
  axleStyleCheck->setChecked(metaValue.axleStyle.value());
  axleStyleCheck->setEnabled(styleEnabled && metaValue.fixedAnnotations.value());
  axleStyleCheck->setToolTip(tr("Fixed Axle annotation on circle background"));
  connect(axleStyleCheck,SIGNAL(clicked(bool)),
          this,          SLOT(  axleStyle(bool)));
  sgrid->addWidget(axleStyleCheck,0,0);

  beamStyleCheck = new QCheckBox(tr("Beams"),gbPLIAnnotationType);
  beamStyleCheck->setChecked(metaValue.beamStyle.value());
  beamStyleCheck->setEnabled(styleEnabled && metaValue.fixedAnnotations.value());
  beamStyleCheck->setToolTip(tr("Fixed Beam annotation on square background"));
  connect(beamStyleCheck,SIGNAL(clicked(bool)),
          this,          SLOT(  beamStyle(bool)));
  sgrid->addWidget(beamStyleCheck,0,1);

  cableStyleCheck = new QCheckBox(tr("Cables"),gbPLIAnnotationType);
  cableStyleCheck->setChecked(metaValue.cableStyle.value());
  cableStyleCheck->setEnabled(styleEnabled && metaValue.fixedAnnotations.value());
  cableStyleCheck->setToolTip(tr("Fixed Cable annotation on square background"));
  connect(cableStyleCheck,SIGNAL(clicked(bool)),
          this,           SLOT(  cableStyle(bool)));
  sgrid->addWidget(cableStyleCheck,0,2);

  connectorStyleCheck = new QCheckBox(tr("Connectors"),gbPLIAnnotationType);
  connectorStyleCheck->setChecked(metaValue.connectorStyle.value());
  connectorStyleCheck->setEnabled(styleEnabled && metaValue.fixedAnnotations.value());
  connectorStyleCheck->setToolTip(tr("Fixed Connector annotation on square background"));
  connect(connectorStyleCheck,SIGNAL(clicked(bool)),
          this,               SLOT(  connectorStyle(bool)));
  sgrid->addWidget(connectorStyleCheck,0,3);

  elementStyleCheck = new QCheckBox(tr("Elements"),gbPLIAnnotationType);
  elementStyleCheck->setChecked(metaValue.elementStyle.value());
  elementStyleCheck->setToolTip(tr("Fixed Part Element ID annotation on rectanglular background"));
  elementStyleCheck->setVisible(bom);
  connect(elementStyleCheck,SIGNAL(clicked(bool)),
          this,              SLOT(  elementStyle(bool)));
  sgrid->addWidget(elementStyleCheck,1,0);

  hoseStyleCheck = new QCheckBox(tr("Hoses"),gbPLIAnnotationType);
  hoseStyleCheck->setChecked(metaValue.hoseStyle.value());
  hoseStyleCheck->setEnabled(styleEnabled && metaValue.fixedAnnotations.value());
  hoseStyleCheck->setToolTip(tr("Fixed Hose annotation on square background"));
  connect(hoseStyleCheck,SIGNAL(clicked(bool)),
          this,          SLOT(  hoseStyle(bool)));
  sgrid->addWidget(hoseStyleCheck,1,bom ? 1 : 0);

  panelStyleCheck = new QCheckBox(tr("Panels"),gbPLIAnnotationType);
  panelStyleCheck->setChecked(metaValue.panelStyle.value());
  panelStyleCheck->setEnabled(styleEnabled && metaValue.fixedAnnotations.value());
  panelStyleCheck->setToolTip(tr("Fixed Panel annotation on circle background"));
  connect(panelStyleCheck,SIGNAL(clicked(bool)),
          this,           SLOT(  panelStyle(bool)));
  sgrid->addWidget(panelStyleCheck,1,bom ? 2 : 1);

  extendedStyleCheck = new QCheckBox(tr("Extended"),gbPLIAnnotationType);
  extendedStyleCheck->setChecked(metaValue.extendedStyle.value());
  extendedStyleCheck->setToolTip(tr("Title or Freeform annotation on rectanglular background"));
  connect(extendedStyleCheck,SIGNAL(clicked(bool)),
          this,              SLOT(  extendedStyle(bool)));
  sgrid->addWidget(extendedStyleCheck,1,bom ? 3 : 2);

  titleModified            = false;
  freeformModified         = false;
  titleAndFreeformModified = false;
  fixedAnnotationsModified = false;

  displayModified          = false;
  enableStyleModified      = false;

  axleStyleModified        = false;
  beamStyleModified        = false;
  cableStyleModified       = false;
  elementStyleModified     = false;
  connectorStyleModified   = false;
  extendedStyleModified    = false;
  hoseStyleModified        = false;
  panelStyleModified       = false;

}

void PliAnnotationGui::titleAnnotation(bool checked)
{
  meta->titleAnnotation.setValue(checked);
  if (freeformAnnotationCheck->isChecked() == checked)
      titleAndFreeformAnnotation(checked);
  titleModified = metaModified(checked != meta->titleAnnotation.value());
}

void PliAnnotationGui::freeformAnnotation(bool checked)
{
  meta->freeformAnnotation.setValue( checked);
  if (titleAnnotationCheck->isChecked() == checked)
      titleAndFreeformAnnotation(checked);
  freeformModified = metaModified(checked != meta->freeformAnnotation.value());
}

void PliAnnotationGui::titleAndFreeformAnnotation(bool checked)
{
  if (meta->titleAndFreeformAnnotation.value() == checked)
      return;
  meta->titleAndFreeformAnnotation.setValue( checked);
  titleAndFreeformModified = metaModified(checked != meta->titleAndFreeformAnnotation.value());
}

void PliAnnotationGui::fixedAnnotations(bool checked)
{
  axleStyleCheck->setEnabled(checked);
  beamStyleCheck->setEnabled(checked);
  cableStyleCheck->setEnabled(checked);
  connectorStyleCheck->setEnabled(checked);
  hoseStyleCheck->setEnabled(checked);
  panelStyleCheck->setEnabled(checked);

  meta->fixedAnnotations.setValue( checked);
  fixedAnnotationsModified = metaModified(checked != meta->fixedAnnotations.value());
}

void PliAnnotationGui::enableExtendedStyle()
{
  bool enabled = true;
  if (!titleAnnotationCheck->isChecked() &&
      !freeformAnnotationCheck->isChecked())
  {
    enabled = false;
  }
  extendedStyleCheck->setEnabled(enabled);
}

void PliAnnotationGui::enableAnnotations()
{
  bool enabled = true;
  if (!titleAnnotationCheck->isChecked() &&
      !freeformAnnotationCheck->isChecked() &&
      !fixedAnnotationsCheck->isChecked())
  {
    enabled = false;
  }
  gbPLIAnnotationSource->setChecked(enabled);
  if (meta->display.value() != enabled)
  {
    meta->display.setValue(enabled);
    modified = displayModified = true;
  }
}

/* Never called because fixed controls are disabled
 * when fixedAnnotationsCheck is unchecked */
void PliAnnotationGui::setFixedAnnotations(bool checked)
{
  if (checked)
  {
    if (!fixedAnnotationsCheck->isChecked())
    {
        fixedAnnotationsCheck->setChecked(checked);
        fixedAnnotations(checked);
    }
  }
}

void PliAnnotationGui::axleStyle(bool checked)
{
  setFixedAnnotations(checked);
  meta->axleStyle.setValue(checked);
  axleStyleModified = metaModified(checked != meta->axleStyle.value());
}

void PliAnnotationGui::beamStyle(bool checked)
{
  setFixedAnnotations(checked);
  meta->beamStyle.setValue(checked);
  beamStyleModified = metaModified(checked != meta->beamStyle.value());
}

void PliAnnotationGui::cableStyle(bool checked)
{
  setFixedAnnotations(checked);
  meta->cableStyle.setValue(checked);
  cableStyleModified = metaModified(checked != meta->cableStyle.value());
}

void PliAnnotationGui::connectorStyle(bool checked)
{
  setFixedAnnotations(checked);
  meta->connectorStyle.setValue(checked);
  connectorStyleModified = metaModified(checked != meta->connectorStyle.value());
}

void PliAnnotationGui::elementStyle(bool checked)
{
  meta->elementStyle.setValue(checked);
  elementStyleModified = metaModified(checked != meta->elementStyle.value());
}


void PliAnnotationGui::extendedStyle(bool checked)
{
  meta->extendedStyle.setValue(checked);
  extendedStyleModified = metaModified(checked != meta->elementStyle.value());
}

void PliAnnotationGui::hoseStyle(bool checked)
{
  setFixedAnnotations(checked);
  meta->hoseStyle.setValue(checked);
  hoseStyleModified = metaModified(checked != meta->elementStyle.value());
}

void PliAnnotationGui::panelStyle(bool checked)
{
  setFixedAnnotations(checked);
  meta->panelStyle.setValue(checked);
  panelStyleModified = metaModified(checked != meta->elementStyle.value());
}

void PliAnnotationGui::gbToggled(bool checked)
{
  meta->display.setValue(checked);
  if(checked) {
    bool titleAndFreeForm = meta->titleAndFreeformAnnotation.value();
    titleAnnotationCheck->setChecked(titleAndFreeForm ? true : meta->titleAnnotation.value());
    freeformAnnotationCheck->setChecked(titleAndFreeForm ? true : meta->freeformAnnotation.value());
    fixedAnnotationsCheck->setChecked(meta->fixedAnnotations.value());
  }

  gbPLIAnnotationType->setEnabled(checked);

  displayModified = metaModified(checked != meta->display.value());
}

void PliAnnotationGui::gbStyleToggled(bool checked)
{
  if (meta->enableStyle.value() != checked) {
    meta->enableStyle.setValue(checked);
    enableStyleModified = metaModified(checked != meta->enableStyle.value());
  }
  axleStyleCheck->setEnabled(checked);
  beamStyleCheck->setEnabled(checked);
  cableStyleCheck->setEnabled(checked);
  connectorStyleCheck->setEnabled(checked);
  hoseStyleCheck->setEnabled(checked);
  panelStyleCheck->setEnabled(checked);
}

void PliAnnotationGui::enableElementStyle(bool checked)
{
  elementStyleCheck->setEnabled(checked);
}

void PliAnnotationGui::apply(QString &topLevelFile)
{
  if (modified) {
    MetaItem mi;
    if (displayModified) {
      mi.setGlobalMeta(topLevelFile,&meta->display);
    }
    if (enableStyleModified) {
      mi.setGlobalMeta(topLevelFile, &meta->enableStyle);
    }
    if (titleModified) {
      mi.setGlobalMeta(topLevelFile,&meta->titleAnnotation);
    }
    if (freeformModified) {
      mi.setGlobalMeta(topLevelFile,&meta->freeformAnnotation);
    }
    if (titleAndFreeformModified) {
      mi.setGlobalMeta(topLevelFile,&meta->titleAndFreeformAnnotation);
    }
    if (fixedAnnotationsModified) {
      mi.setGlobalMeta(topLevelFile,&meta->fixedAnnotations);
    }
    if (axleStyleModified) {
      mi.setGlobalMeta(topLevelFile,&meta->axleStyle);
    }
    if (beamStyleModified) {
      mi.setGlobalMeta(topLevelFile,&meta->beamStyle);
    }
    if (cableStyleModified) {
      mi.setGlobalMeta(topLevelFile,&meta->cableStyle);
    }
    if (connectorStyleModified) {
      mi.setGlobalMeta(topLevelFile,&meta->connectorStyle);
    }
    if (elementStyleModified) {
      mi.setGlobalMeta(topLevelFile,&meta->elementStyle);
    }
    if (extendedStyleModified) {
      mi.setGlobalMeta(topLevelFile,&meta->extendedStyle);
    }
    if (hoseStyleModified) {
      mi.setGlobalMeta(topLevelFile,&meta->hoseStyle);
    }
    if (panelStyleModified) {
      mi.setGlobalMeta(topLevelFile,&meta->panelStyle);
    }
  }
}

/***********************************************************************
 *
 * CsiAnnotation
 *
 **********************************************************************/

CsiAnnotationGui::CsiAnnotationGui(
    const QString        &heading,
    CsiAnnotationMeta   *_meta,
    QGroupBox           *parent,
    bool                 fixedAnnotations)
{
  meta = _meta;
  metaValue = *meta;

  QString fixedMessage;
  if (!fixedAnnotations)
      fixedMessage =
              tr("Display Part List (PLI) Annotations 'Fixed Annotations' must be enabled to set this fixed annotaiton display");

  QGridLayout *grid = new QGridLayout(parent);

  if (parent) {
    parent->setLayout(grid);
  } else {
    setLayout(grid);
  }

  if (heading != "") {
    headingLabel = new QLabel(heading,parent);
    grid->addWidget(headingLabel);
  } else {
    headingLabel = nullptr;
  }

  // CSI Annotation Display Options
    gbCSIAnnotationType = new QGroupBox(tr("Display Assembly (CSI) Part Annotation"),parent);
  gbCSIAnnotationType->setWhatsThis(lpubWT(WT_GUI_ASSEM_ANNOTATION_TYPE, gbCSIAnnotationType->title()));
  gbCSIAnnotationType->setCheckable(true);
  gbCSIAnnotationType->setChecked(metaValue.display.value());
  QGridLayout *sgrid = new QGridLayout();
  gbCSIAnnotationType->setLayout(sgrid);
  grid->addWidget(gbCSIAnnotationType);
  connect(gbCSIAnnotationType,SIGNAL(toggled(bool)),
          this,                  SLOT(  gbToggled(bool)));

  axleDisplayCheck = new QCheckBox(tr("Axles"),gbCSIAnnotationType);
  axleDisplayCheck->setChecked(metaValue.axleDisplay.value());
  axleDisplayCheck->setEnabled(fixedAnnotations);
  axleDisplayCheck->setToolTip(fixedAnnotations ? tr("Display Axle annotation") : fixedMessage);
  connect(axleDisplayCheck,SIGNAL(clicked(bool)),
          this,            SLOT(  axleDisplay(bool)));
  sgrid->addWidget(axleDisplayCheck,0,0);

  beamDisplayCheck = new QCheckBox(tr("Beams"),gbCSIAnnotationType);
  beamDisplayCheck->setChecked(metaValue.beamDisplay.value());
  beamDisplayCheck->setEnabled(fixedAnnotations);
  beamDisplayCheck->setToolTip(fixedAnnotations ? tr("Display Beam annotation") : fixedMessage);
  connect(beamDisplayCheck,SIGNAL(clicked(bool)),
          this,            SLOT(  beamDisplay(bool)));
  sgrid->addWidget(beamDisplayCheck,0,1);

  cableDisplayCheck = new QCheckBox(tr("Cables"),gbCSIAnnotationType);
  cableDisplayCheck->setChecked(metaValue.cableDisplay.value());
  cableDisplayCheck->setEnabled(fixedAnnotations);
  cableDisplayCheck->setToolTip(fixedAnnotations ? tr("Display Cable annotation") : fixedMessage);
  connect(cableDisplayCheck,SIGNAL(clicked(bool)),
          this,             SLOT(  cableDisplay(bool)));
  sgrid->addWidget(cableDisplayCheck,0,2);

  connectorDisplayCheck = new QCheckBox(tr("Connectors"),gbCSIAnnotationType);
  connectorDisplayCheck->setChecked(metaValue.connectorDisplay.value());
  connectorDisplayCheck->setEnabled(fixedAnnotations);
  connectorDisplayCheck->setToolTip(fixedAnnotations ? tr("Display Connector annotation") : fixedMessage);
  connect(connectorDisplayCheck,SIGNAL(clicked(bool)),
          this,                 SLOT(  connectorDisplay(bool)));
  sgrid->addWidget(connectorDisplayCheck,0,3);

  hoseDisplayCheck = new QCheckBox(tr("Hoses"),gbCSIAnnotationType);
  hoseDisplayCheck->setChecked(metaValue.hoseDisplay.value());
  hoseDisplayCheck->setEnabled(fixedAnnotations);
  hoseDisplayCheck->setToolTip(fixedAnnotations ? tr("Display Hose annotation") : fixedMessage);
  connect(hoseDisplayCheck,SIGNAL(clicked(bool)),
          this,            SLOT(  hoseDisplay(bool)));
  sgrid->addWidget(hoseDisplayCheck,1,0);

  panelDisplayCheck = new QCheckBox(tr("Panels"),gbCSIAnnotationType);
  panelDisplayCheck->setChecked(metaValue.panelDisplay.value());
  panelDisplayCheck->setEnabled(fixedAnnotations);
  panelDisplayCheck->setToolTip(fixedAnnotations ? tr("Display Panel annotation") : fixedMessage);
  connect(panelDisplayCheck,SIGNAL(clicked(bool)),
          this,             SLOT(  panelDisplay(bool)));
  sgrid->addWidget(panelDisplayCheck,1,1);

  extendedDisplayCheck = new QCheckBox(tr("Extended"),gbCSIAnnotationType);
  extendedDisplayCheck->setChecked(metaValue.extendedDisplay.value());
  extendedDisplayCheck->setToolTip(tr("Display Title or Freeform annotation"));
  connect(extendedDisplayCheck,SIGNAL(clicked(bool)),
          this,                SLOT(  extendedDisplay(bool)));
  sgrid->addWidget(extendedDisplayCheck,1,2);

  // CSI Annotation Placement
  gbPlacement = new QGroupBox(tr("%1 Placement").arg(PlacementDialog::placementTypeName(CsiAnnotationType)),parent);
  PlacementData placementData = metaValue.placement.value();
  const QString placementButtonText = tr("Change %1 Placement").arg(PlacementDialog::placementTypeName(CsiAnnotationType));
  gbPlacement->setWhatsThis(commonMenus.naturalLanguagePlacementWhatsThis(CsiAnnotationType,placementData,placementButtonText));
  QGridLayout *gLayout = new QGridLayout();
  gbPlacement->setLayout(gLayout);
  grid->addWidget(gbPlacement);

  placementButton = new QPushButton(placementButtonText,gbPlacement);
  placementButton->setToolTip(tr("Set annotation placement relative to CSI part"));
  connect(placementButton,SIGNAL(         clicked(bool)),
          this,             SLOT(placementChanged(bool)));
  gLayout->addWidget(placementButton);

  displayModified          = false;
  axleDisplayModified      = false;
  beamDisplayModified      = false;
  cableDisplayModified     = false;
  connectorDisplayModified = false;
  extendedDisplayModified  = false;
  hoseDisplayModified      = false;
  panelDisplayModified     = false;
  placementModified        = false;
}

void CsiAnnotationGui::axleDisplay(bool checked)
{
  meta->axleDisplay.setValue(checked);
  axleDisplayModified = metaModified(checked != metaValue.axleDisplay.value());
}

void CsiAnnotationGui::beamDisplay(bool checked)
{
  meta->beamDisplay.setValue(checked);
  beamDisplayModified = metaModified(checked != metaValue.beamDisplay.value());
}

void CsiAnnotationGui::cableDisplay(bool checked)
{
  meta->cableDisplay.setValue(checked);
  cableDisplayModified = metaModified(checked != metaValue.cableDisplay.value());
}

void CsiAnnotationGui::connectorDisplay(bool checked)
{
  meta->connectorDisplay.setValue(checked);
  connectorDisplayModified = metaModified(checked != metaValue.connectorDisplay.value());
}

void CsiAnnotationGui::extendedDisplay(bool checked)
{
  meta->extendedDisplay.setValue(checked);
  extendedDisplayModified = metaModified(checked != metaValue.extendedDisplay.value());
}

void CsiAnnotationGui::hoseDisplay(bool checked)
{
  meta->hoseDisplay.setValue(checked);
  hoseDisplayModified = metaModified(checked != metaValue.hoseDisplay.value());
}

void CsiAnnotationGui::panelDisplay(bool checked)
{
  meta->panelDisplay.setValue(checked);
  panelDisplayModified = metaModified(checked != metaValue.panelDisplay.value());
}

void CsiAnnotationGui::placementChanged(bool)
{
  PlacementData pd = meta->placement.value();
  bool ok = PlacementDialog::getPlacement(SingleStepType,CsiAnnotationType,pd,tr("Annotation Placement"));
  if (ok) {
    meta->placement.setValue(pd);
    PlacementData pdv = metaValue.placement.value();
    placementModified = (
      pd.placement     != pdv.placement       ||
      pd.justification != pdv.justification   ||
      pd.preposition   != pdv.preposition     ||
      pd.relativeTo    != pdv.relativeTo      ||
      pd.rectPlacement != pdv.rectPlacement   ||
      notEqual(pd.offsets[0], pdv.offsets[0]) ||
      notEqual(pd.offsets[1], pdv.offsets[1]));
    metaModified(placementModified);
  }
}

void CsiAnnotationGui::gbToggled(bool checked)
{
  meta->display.setValue(checked);
  displayModified = metaModified(checked != metaValue.display.value());
  gbPlacement->setEnabled(checked);
}

void CsiAnnotationGui::apply(QString &topLevelFile)
{
  if (modified) {
    MetaItem mi;
    if (displayModified) {
      mi.setGlobalMeta(topLevelFile,&meta->display);
    }
    if (axleDisplayModified) {
      mi.setGlobalMeta(topLevelFile,&meta->axleDisplay);
    }
    if (beamDisplayModified) {
      mi.setGlobalMeta(topLevelFile,&meta->beamDisplay);
    }
    if (cableDisplayModified) {
      mi.setGlobalMeta(topLevelFile,&meta->cableDisplay);
    }
    if (connectorDisplayModified) {
      mi.setGlobalMeta(topLevelFile,&meta->connectorDisplay);
    }
    if (extendedDisplayModified) {
      mi.setGlobalMeta(topLevelFile,&meta->extendedDisplay);
    }
    if (hoseDisplayModified) {
      mi.setGlobalMeta(topLevelFile,&meta->hoseDisplay);
    }
    if (panelDisplayModified) {
      mi.setGlobalMeta(topLevelFile,&meta->panelDisplay);
    }
    if (placementModified) {
      mi.setGlobalMeta(topLevelFile,&meta->placement);
    }
  }
}

/***********************************************************************
 *
 * Page Orientation
 *
 **********************************************************************/

PageOrientationGui::PageOrientationGui(
  QString const            &heading,
  PageOrientationMeta     *_meta,
  QGroupBox               *parent)
{
  meta = _meta;
  metaValue = *meta;

  QGridLayout *grid   = new QGridLayout(parent);

  if (parent) {
    parent->setLayout(grid);
    parent->setWhatsThis(lpubWT(WT_GUI_PAGE_ORIENTATION, parent->title()));
  } else {
    setLayout(grid);
    setWhatsThis(lpubWT(WT_GUI_PAGE_ORIENTATION, heading.isEmpty() ? tr("Page Orientation") : heading));
  }

  if (heading != "") {
    label = new QLabel(heading);
    grid->addWidget(label,0,0);
  } else {
    label = nullptr;
  }

  /* page orientation */
  portraitRadio = new QRadioButton(tr("Portrait"),parent);
  portraitRadio->setChecked(metaValue.value() == Portrait);
  connect(portraitRadio,SIGNAL(clicked(bool)),
          this,        SLOT(  orientationChange(bool)));
  if (heading == "")
    grid->addWidget(portraitRadio,0,0);
  else
    grid->addWidget(portraitRadio,1,0);

  landscapeRadio    = new QRadioButton(tr("Landscape"),parent);
  landscapeRadio->setChecked(metaValue.value() == Landscape);
  connect(landscapeRadio,SIGNAL(clicked(bool)),
          this,     SLOT(  orientationChange(bool)));
  if (heading == "")
    grid->addWidget(landscapeRadio,0,1);
  else
    grid->addWidget(landscapeRadio,1,1);
}

void PageOrientationGui::orientationChange(bool)
{
  QObject *radioButton = sender();
  if (radioButton == portraitRadio)
    meta->setValue(Portrait);
  else
    meta->setValue(Landscape);
  modified = meta->value() != metaValue.value();
}

void PageOrientationGui::apply(QString &topLevelFile)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(topLevelFile,meta);
  }
}

/***********************************************************************
 *
 * Rotate Icon Size
 *
 **********************************************************************/

RotateIconSizeGui::RotateIconSizeGui(
  QString const            &heading,
  UnitsMeta               *_meta,
  QGroupBox               *parent)
{
  meta  = _meta;
  dataW = _meta->value(0);
  dataH = _meta->value(1);

  QGridLayout *grid   = new QGridLayout(parent);

  if (parent) {
    parent->setLayout(grid);
    parent->setWhatsThis(lpubWT(WT_GUI_ROTATE_ICON_SIZE, parent->title()));
  } else {
    setLayout(grid);
    setWhatsThis(lpubWT(WT_GUI_ROTATE_ICON_SIZE, heading.isEmpty() ? tr("Rotate Icon Size") : heading));
  }

  if (heading != "") {
    label = new QLabel(heading);
    grid->addWidget(label,0,0);
  } else {
    label = nullptr;
  }

  QString string  = QString::number(dataW,'f', _meta->_precision);

  valueW = new QLineEdit(parent);
  QDoubleValidator *valueWValidator = new QDoubleValidator(valueW);
  valueWValidator->setRange(0.0f, 1000.0f);
  valueWValidator->setDecimals(_meta->_precision);
  valueWValidator->setNotation(QDoubleValidator::StandardNotation);
  valueW->setValidator(valueWValidator);
  valueW->setText(QString::number(dataW,'f',_meta->_precision));
  resetWAct = valueW->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  resetWAct->setText(tr("Reset"));
  resetWAct->setEnabled(false);
  connect(valueW,    SIGNAL( textEdited(const QString&)),
          this,        SLOT(enableReset(const QString&)));
  connect(resetWAct, SIGNAL(triggered()),
          this,        SLOT(lineEditReset()));
  connect(valueW,    SIGNAL( textChanged(const QString&)),
          this,        SLOT(valueWChange(const QString&)));

  labelW = new QLabel(tr("Width"),parent);
  labelW->setToolTip(tr("Current Width in pixels is %1").arg(_meta->valuePixels(0)));
  if (heading == "") {
    grid->addWidget(labelW,0,0);
    grid->addWidget(valueW,0,1);
  } else {
    grid->addWidget(valueW,1,0);
    grid->addWidget(valueW,1,1);
  }

  string  = QString::number(dataH,'f', _meta->_precision);

  valueH = new QLineEdit(parent);
  QDoubleValidator *valueHValidator = new QDoubleValidator(valueH);
  valueHValidator->setRange(0.0f, 1000.0f);
  valueHValidator->setDecimals(_meta->_precision);
  valueHValidator->setNotation(QDoubleValidator::StandardNotation);
  valueH->setValidator(valueHValidator);
  valueH->setText(QString::number(dataH,'f',_meta->_precision));
  resetHAct = valueH->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  resetHAct->setText(tr("Reset"));
  resetHAct->setEnabled(false);
  connect(valueH,    SIGNAL( textEdited(const QString&)),
          this,        SLOT(enableReset(const QString&)));
  connect(resetHAct, SIGNAL(triggered()),
          this,        SLOT(lineEditReset()));
  connect(valueH,    SIGNAL( textChanged(const QString&)),
          this,        SLOT(valueHChange(const QString&)));

  labelH = new QLabel(tr("Height"),parent);
  labelH->setToolTip(tr("Current Height in pixels is %1").arg(_meta->valuePixels(1)));
  if (heading == "") {
    grid->addWidget(labelH,0,2);
    grid->addWidget(valueH,0,3);
  } else {
    grid->addWidget(labelH,1,2);
    grid->addWidget(valueH,1,3);
  }

  dataModified = false;

  setEnabled(true);
}

void RotateIconSizeGui::enableReset(const QString &displayText)
{
  const double value = displayText.toDouble();

  if (sender() == valueW)
    resetWAct->setEnabled(notEqual(value, dataW));
  else
  if (sender() == valueH)
    resetHAct->setEnabled(notEqual(value, dataH));
}

void RotateIconSizeGui::lineEditReset()
{
  if (sender() == resetWAct) {
    resetWAct->setEnabled(false);
    if (valueW) {
      valueW->setText(QString::number(dataW,'f',meta->_precision));
      dataModified = false;
    }
  }
  else
  if (sender() == resetHAct) {
    resetHAct->setEnabled(false);
    if (valueH) {
      valueH->setText(QString::number(dataH,'f',meta->_precision));
      dataModified = false;
    }
  }
}

void RotateIconSizeGui::valueWChange(const QString &string)
{
  float value = string.toFloat();
  meta->setValue(0,value);
  labelW->setToolTip(tr("Current Height in pixels is %1").arg(meta->valuePixels(1)));
  dataModified = metaModified(notEqual(value, dataH));
}

void RotateIconSizeGui::valueHChange(const QString &string)
{
  float value = string.toFloat();
  meta->setValue(1,value);
  labelH->setToolTip(tr("Current Height in pixels is %1").arg(meta->valuePixels(1)));
  dataModified = metaModified(notEqual(value, dataH));
}

void RotateIconSizeGui::setEnabled(bool enable)
{
  valueW->setEnabled(enable);
  valueH->setEnabled(enable);
}

void RotateIconSizeGui::apply(QString &topLevelFile)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(topLevelFile,meta);
  }
}

/***********************************************************************
 *
 * Page Size NOT USED KO (using SizeAndOrientationGui instead)
 *
 **********************************************************************/

PageSizeGui::PageSizeGui(
  QString const            &heading,
  PageSizeMeta            *_meta,
  QGroupBox               *parent)
{
  meta  = _meta;
  dataW = _meta->value(0);
  dataH = _meta->value(1);

  QGridLayout *grid   = new QGridLayout(parent);

//   logNotice() << " \nPageSizeGui Initialized:" <<
//                 " \nSize 0: " << meta->value(0) <<
//                 " \nSize 1: " << meta->value(1) <<
//                 " \nOrientation: " << ometa->value()
//                 ;

  if (parent) {
    parent->setLayout(grid);
    parent->setWhatsThis(lpubWT(WT_GUI_PAGE_SIZE, parent->title()));
  } else {
    setLayout(grid);
    setWhatsThis(lpubWT(WT_GUI_PAGE_SIZE, heading.isEmpty() ? tr("Page Size") : heading));
  }

  if (heading != "") {
    label = new QLabel(heading);
    grid->addWidget(label,0,0);
  } else {
    label = nullptr;
  }

  /* page size */
  int numPageTypes = PageSizes::numPageTypes();

  sizeCombo = new QComboBox(parent);
  for (int i = 0; i < numPageTypes; i++) {
       sizeCombo->addItem(PageSizes::pageTypeSizeID(i));
  }

  sizeCombo->setCurrentIndex(int(getTypeIndex(dataW,dataH)));
  connect(sizeCombo,SIGNAL(currentTextChanged(const QString&)),
          this,       SLOT(         typeChange(const QString&)));
  if (heading == "")
    grid->addWidget(sizeCombo,0,0);
  else
    grid->addWidget(sizeCombo,1,0);

  valueW = new QLineEdit(parent);
  QDoubleValidator *valueWValidator = new QDoubleValidator(valueW);
  valueWValidator->setRange(0.0f, 1000.0f);
  valueWValidator->setDecimals(meta->_precision);
  valueWValidator->setNotation(QDoubleValidator::StandardNotation);
  valueW->setValidator(valueWValidator);
  valueW->setText(QString::number(dataW,'f',meta->_precision));
  resetWAct = valueW->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  resetWAct->setText(tr("Reset"));
  resetWAct->setEnabled(false);
  connect(valueW,    SIGNAL( textEdited(const QString&)),
          this,        SLOT(enableReset(const QString&)));
  connect(resetWAct, SIGNAL(triggered()),
          this,        SLOT(lineEditReset()));
  connect(valueW,    SIGNAL( textChanged(const QString&)),
          this,        SLOT(valueWChange(const QString&)));

  if (heading == "")
    grid->addWidget(valueW,0,1);
  else
    grid->addWidget(valueW,1,1);

  valueH = new QLineEdit(parent);
  QDoubleValidator *valueHValidator = new QDoubleValidator(valueH);
  valueHValidator->setRange(0.0f, 1000.0f);
  valueHValidator->setDecimals(meta->_precision);
  valueHValidator->setNotation(QDoubleValidator::StandardNotation);
  valueH->setValidator(valueHValidator);
  valueH->setText(QString::number(dataH,'f',meta->_precision));
  resetHAct = valueH->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  resetHAct->setText(tr("Reset"));
  resetHAct->setEnabled(false);
  connect(valueH,    SIGNAL( textEdited(const QString&)),
          this,        SLOT(enableReset(const QString&)));
  connect(resetHAct, SIGNAL(    triggered()),
          this,        SLOT(lineEditReset()));
  connect(valueH,    SIGNAL( textChanged(const QString&)),
          this,        SLOT(valueHChange(const QString&)));

  if (heading == "")
    grid->addWidget(valueH,0,2);
  else
    grid->addWidget(valueH,1,2);

  if (sizeCombo->currentText() == "Custom")
    setEnabled(true);
  else
    setEnabled(false);

//  logDebug() << "Current Page Type: " << sizeCombo->currentText();
}

void PageSizeGui::enableReset(const QString &displayText)
{
  const double value = displayText.toDouble();

  if (sender() == valueW)
    resetWAct->setEnabled(notEqual(value, dataW));
  else
  if (sender() == valueH)
    resetHAct->setEnabled(notEqual(value, dataH));
}

void PageSizeGui::lineEditReset()
{
  if (sender() == resetWAct) {
    resetWAct->setEnabled(false);
    if (valueW) {
      valueW->setText(QString::number(dataW,'f',meta->_precision));
    }
  }
  else
  if (sender() == resetHAct) {
    resetHAct->setEnabled(false);
    if (valueH) {
      valueH->setText(QString::number(dataH,'f',meta->_precision));
    }
  }
}

int PageSizeGui::getTypeIndex(float &widthPg, float &heightPg) {

  bool dpi = lpub->page.meta.LPub.resolution.type() == DPI;
  int  numPageTypes = PageSizes::numPageTypes();
  int index = -1;
  QString pageWidth;
  QString pageHeight;
  QString typeWidth;
  QString typeHeight;
  for (int i = 0; i < numPageTypes; i++) {

    pageWidth  = QString::number( widthPg,  'f', 1 /*meta->_precision*/ );
    pageHeight = QString::number( heightPg, 'f', 1 /*meta->_precision*/ );
    typeWidth  = QString::number((dpi ? PageSizes::pageWidthIn(i) : PageSizes::pageWidthCm(i)),  'f', 1 /*meta->_precision*/ );
    typeHeight = QString::number((dpi ? PageSizes::pageHeightIn(i) : PageSizes::pageHeightCm(i)), 'f', 1 /*meta->_precision*/ );

//    qDebug() << "\n" << PageSizes::pageTypeSizeID(i) << " @ index: " << i
//             << "\nType: (" << typeWidth << "x" << typeHeight << ") "
//             << "\nPage: (" << pageWidth << "x" << pageHeight << ")";

    if ((pageWidth == typeWidth) && (pageHeight == typeHeight)) {
      index = i;
      break;
    }
  }
  if (index == -1)
    index = sizeCombo->findText("Custom");

  return index;
}

void PageSizeGui::typeChange(const QString &pageType) {

  float pageWidth  = meta->value(0);
  float pageHeight = meta->value(1);;
  bool  editLine   = true;

//  qDebug() << "\nPage Type: " << pageType ;

  if (pageType != QStringLiteral("Custom")) {

    bool dpi = lpub->page.meta.LPub.resolution.type() == DPI;
    int   numPageTypes = PageSizes::numPageTypes();

    for (int i = 0; i < numPageTypes; i++) {
      if (pageType == PageSizes::pageTypeSizeID(i)) {
        pageWidth  = dpi ? PageSizes::pageWidthIn(i) : PageSizes::pageWidthCm(i);
        pageHeight = dpi ? PageSizes::pageHeightIn(i) : PageSizes::pageHeightCm(i);
        break;
      }
    }
    editLine = false;
  }

  valueW->setText(QString::number(pageWidth,'f',meta->_precision));

  valueH->setText(QString::number(pageHeight,'f',meta->_precision));

  setEnabled(editLine);
}

void PageSizeGui::valueWChange(const QString &string)
{
  float value = string.toFloat();

  if (lpub->page.meta.LPub.page.orientation.value() == Portrait) {
    meta->setValue(0,value);
    modified = notEqual(value, dataW);
  } else {
    meta->setValue(1,value);
    modified = notEqual(value, dataH);
  }
}

void PageSizeGui::valueHChange(const QString &string)
{
  float value = string.toFloat();
  if (lpub->page.meta.LPub.page.orientation.value() == Portrait) {
    meta->setValue(1,value);
    modified = notEqual(value, dataH);
  } else {
    meta->setValue(0,value);
    modified = notEqual(value, dataW);
  }
//#ifdef QT_DEBUG_MODE
//  qDebug() << "\nChange to " << (lpub->page.meta.LPub.page.orientation.value() == Portrait ? "Portrait" : "Landscape") << " Page"
//           << "\nMeta Value(0) :" << meta->value(0)
//           << "\nMeta Value(1) :" << meta->value(1);
//#endif
}

void PageSizeGui::setEnabled(bool enable)
{
  valueW->setEnabled(enable);
  valueH->setEnabled(enable);
}

void PageSizeGui::apply(QString &topLevelFile)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(topLevelFile,meta);
  }
}

/***********************************************************************
 *
 * Page Size And Orientation
 *
 **********************************************************************/

SizeAndOrientationGui::SizeAndOrientationGui(
  QString const       &heading,
  PageSizeMeta        *_metaS,
  PageOrientationMeta *_metaO,
  QGroupBox           *parent)
{
  metaS          = _metaS;
  dataS          = _metaS->value();

  metaO          = _metaO;
  dataO          = _metaO->value();

  QGridLayout *grid   = new QGridLayout(parent);

  if (parent) {
    parent->setLayout(grid);
    parent->setWhatsThis(lpubWT(WT_GUI_SIZE_AND_ORIENTATION, parent->title()));
  } else {
    setLayout(grid);
    setWhatsThis(lpubWT(WT_GUI_SIZE_AND_ORIENTATION, heading.isEmpty() ? tr("Page Size And Orientation") : heading));
  }

  if (heading != "") {
    label = new QLabel(heading);
    grid->addWidget(label,0,0);
  } else {
    label = nullptr;
  }

//#ifdef QT_DEBUG_MODE
//  logNotice() << " \nSizeAndOrientationGui Initialized:"
//              << " \nSize 0:      " <<  _metaS->value(0)
//              << " \nSize 1:      " <<  _metaS->value(1)
//              << " \nField Width: " <<  _metaS->_fieldWidth
//              << " \nPrecision:   " <<  _metaS->_precision
//              << " \nInput Mask:  " <<  _metaS->_inputMask
//              << " \nSizeID:       " <<  _metaS->valueSizeID()
//              << " \nOrientation: " << (_metaO->value() == Portrait ? "Portrait" : "Landscape")
//                 ;
//#endif

  // page size ID combo
  float sizeIDWidth,sizeIDHeight;
  int   numPageSizeIDs   = PageSizes::numPageTypes();
  bool  dpi              = lpub->page.meta.LPub.resolution.type() == DPI;
  sizeIDCombo            = new QComboBox(parent);
  int sizeIDIndex        = -1;

  for (int i = 0; i < numPageSizeIDs; i++) {

    sizeIDWidth  = dpi ? PageSizes::pageWidthIn(i) : inches2centimeters(PageSizes::pageWidthIn(i));
    sizeIDHeight = dpi ? PageSizes::pageHeightIn(i) : inches2centimeters(PageSizes::pageHeightIn(i));

//    qDebug() << "\n" << pageSizeTypes[i].pageType << " @ index: " << i
//             << "\nSizeID: (" << QString::number(sizeIDWidth, 'f', 3) << "x" <<  QString::number(sizeIDHeight, 'f', 3) << ") "
//             << "\nPage:   (" <<  QString::number(pageWidth, 'f', 3) << "x" <<  QString::number(pageHeight, 'f', 3) << ")";

    QString sizeID = QString("%1 (%2 x %3)")
                           .arg(PageSizes::pageTypeSizeID(i), QString::number(sizeIDWidth, 'f', 1), QString::number(sizeIDHeight, 'f', 1));

    sizeIDCombo->addItem(sizeID);

    if (dataS.sizeID != QStringLiteral("Custom") && PageSizes::pageTypeSizeID(i) == dataS.sizeID) {
        sizeIDIndex = i;
    }
  }

  if (sizeIDIndex == -1) {
    QString customSizeID = QString("%1 (%2 x %3)")
                                 .arg(dataS.sizeID, QString::number(dataS.sizeW,'f',1), QString::number(dataS.sizeH,'f',1));
    int lastItem = sizeIDCombo->count() - 1;
    sizeIDCombo->removeItem(lastItem);
    sizeIDCombo->addItem(customSizeID);
    sizeIDIndex = lastItem;

  }

  sizeIDCombo->setCurrentIndex(sizeIDIndex);
  connect(sizeIDCombo,SIGNAL(currentTextChanged(const QString&)),
          this,       SLOT(         sizeIDChange(const QString&)));

  if (heading == "")
    grid->addWidget(sizeIDCombo,0,0);
  else
    grid->addWidget(sizeIDCombo,1,0);

  // page size width
  valueW = new QLineEdit(parent);
  QDoubleValidator *valueWValidator = new QDoubleValidator(valueW);
  valueWValidator->setRange(0.0f, 1000.0f);
  valueWValidator->setDecimals(_metaS->_precision);
  valueWValidator->setNotation(QDoubleValidator::StandardNotation);
  valueW->setValidator(valueWValidator);
  valueW->setText(QString::number(dataS.sizeW,'f',_metaS->_precision));
  resetWAct = valueW->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  resetWAct->setText(tr("Reset"));
  resetWAct->setEnabled(false);
  connect(valueW,   SIGNAL( textEdited(const QString&)),
          this,       SLOT(enableReset(const QString&)));
  connect(resetWAct,SIGNAL(    triggered()),
          this,       SLOT(lineEditReset()));
  connect(valueW,   SIGNAL( textChanged(const QString&)),
          this,       SLOT(valueWChange(const QString&)));

  if (heading == "")
    grid->addWidget(valueW,0,1);
  else
    grid->addWidget(valueW,1,1);

  // page size height
  valueH = new QLineEdit(parent);
  QDoubleValidator *valueHValidator = new QDoubleValidator(valueH);
  valueHValidator->setRange(0.0f, 1000.0f);
  valueHValidator->setDecimals(_metaS->_precision);
  valueHValidator->setNotation(QDoubleValidator::StandardNotation);
  valueH->setValidator(valueHValidator);
  valueH->setText(QString::number(dataS.sizeH,'f',_metaS->_precision));
  resetHAct = valueH->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
  resetHAct->setText(tr("Reset"));
  resetHAct->setEnabled(false);
  connect(valueH,   SIGNAL( textEdited(const QString&)),
          this,       SLOT(enableReset(const QString&)));
  connect(resetHAct,SIGNAL(    triggered()),
          this,       SLOT(lineEditReset()));
  connect(valueH,   SIGNAL( textChanged(const QString&)),
          this,       SLOT(valueHChange(const QString&)));

  if (heading == "")
    grid->addWidget(valueH,0,2);
  else
    grid->addWidget(valueH,1,2);

  if (sizeIDCombo->currentText() == QStringLiteral("Custom"))
    setEnabled(true);
  else
    setEnabled(false);

  //spacer
  QHBoxLayout *hLayout = new QHBoxLayout(nullptr);
  if (heading == "")
    grid->addLayout(hLayout,1,0);
  else
    grid->addLayout(hLayout,2,0);
  QSpacerItem *hSpacer;
  hSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  hLayout->addSpacerItem(hSpacer);

  // page orientation
  portraitRadio = new QRadioButton(tr("Portrait"),parent);
  portraitRadio->setChecked(dataO == Portrait);
  connect(portraitRadio,SIGNAL(clicked(bool)),
          this,         SLOT(  orientationChange(bool)));

  if (heading == "")
    grid->addWidget(portraitRadio,1,1);
  else
    grid->addWidget(portraitRadio,2,1);

  landscapeRadio    = new QRadioButton(tr("Landscape"),parent);
  landscapeRadio->setChecked(dataO == Landscape);
  connect(landscapeRadio,SIGNAL(clicked(bool)),
          this,          SLOT(  orientationChange(bool)));

  if (heading == "")
    grid->addWidget(landscapeRadio,1,2);
  else
    grid->addWidget(landscapeRadio,2,2);

  sizeModified        = false;
  sizeIDModified      = false;
  orientationModified = false;

//#ifdef QT_DEBUG_MODE
//  logDebug() << qPrintable(QString("0. Current Page SizeID: %1")
//                                   .arg(sizeIDCombo->currentText()));
//#endif
}

void SizeAndOrientationGui::enableReset(const QString &displayText)
{
  const double value = displayText.toDouble();

  if (sender() == valueW)
    resetWAct->setEnabled(notEqual(value, dataS.sizeW));
  else
  if (sender() == valueH)
    resetHAct->setEnabled(notEqual(value, dataS.sizeH));
}

void SizeAndOrientationGui::lineEditReset()
{
  if (sender() == resetWAct) {
    resetWAct->setEnabled(false);
    if (valueW) {
      valueW->setText(QString::number(dataS.sizeW,'f',metaS->_precision));
      sizeModified = false;
    }
  }
  else
  if (sender() == resetHAct) {
    resetHAct->setEnabled(false);
    if (valueH) {
      valueH->setText(QString::number(dataS.sizeH,'f',metaS->_precision));
      sizeModified = false;
    }
  }
}

void SizeAndOrientationGui::sizeIDChange(const QString &sizeIDString)
{
  bool  staticSize = true;
  int   sizeIDIndx = sizeIDString.indexOf(" (");

  PageSizeData _data = metaS->value();
  _data.sizeID = sizeIDString.left(sizeIDIndx);

#ifdef QT_DEBUG_MODE
  logDebug() << qPrintable(QString("2. NewSizeID: %3, OldSizeID")
                                   .arg(_data.sizeID, dataS.sizeID));
#endif

  if ((staticSize = _data.sizeID != QStringLiteral("Custom"))) {
    bool dpi = lpub->page.meta.LPub.resolution.type() == DPI;
    int  numPageSizeIDs = PageSizes::numPageTypes();

    for (int i = 0; i < numPageSizeIDs; i++) {
      if (_data.sizeID == PageSizes::pageTypeSizeID(i)) {
        _data.sizeW = dpi ? PageSizes::pageWidthIn(i) : inches2centimeters(PageSizes::pageWidthIn(i));
        _data.sizeH = dpi ? PageSizes::pageHeightIn(i) : inches2centimeters(PageSizes::pageHeightIn(i));
        break;
      }
    }
  }

  metaS->setValueSizeID(_data.sizeID);

  valueW->setText(QString::number(_data.sizeW,'f',metaS->_precision));
  valueH->setText(QString::number(_data.sizeH,'f',metaS->_precision));

  sizeIDModified = metaModified(_data.sizeID != dataS.sizeID);

  setEnabled(!staticSize);
}

void SizeAndOrientationGui::orientationChange(bool)
{
  OrientationEnc value;

  if (sender() == portraitRadio)
    value = Portrait;
  else
    value = Landscape;

#ifdef QT_DEBUG_MODE
  logDebug() << qPrintable(QString("1. New Orientation: %1, Old Orientation %2")
                                   .arg(value == Portrait ? "Portrait" : "Landscape")
                                   .arg(dataO == Portrait ? "Portrait" : "Landscape"));
#endif

  metaO->setValue(value);

  sizeIDChange(sizeIDCombo->currentText());

  orientationModified = metaModified(value != dataO);
}

void SizeAndOrientationGui::valueWChange(const QString &string)
{
  float value = string.toFloat();
  metaS->setValue(0,value);
  sizeModified = metaModified(notEqual(value, dataS.sizeW));
}

void SizeAndOrientationGui::valueHChange(const QString &string)
{
  float value = string.toFloat();
  metaS->setValue(1,value);
  sizeModified = metaModified(notEqual(value, dataS.sizeH));
}

void SizeAndOrientationGui::setEnabled(bool enable)
{
  valueW->setEnabled(enable);
  valueH->setEnabled(enable);
}

void SizeAndOrientationGui::apply(QString &topLevelFile)
{
  if (modified) {
    MetaItem mi;
    if (sizeIDModified || sizeModified) {
      mi.setGlobalMeta(topLevelFile,metaS);
    }
    if (orientationModified) {
      mi.setGlobalMeta(topLevelFile,metaO);
    }
  }
}

/***********************************************************************
 *
 * SubModelColor
 *
 **********************************************************************/

SubModelColorGui::SubModelColorGui(
  StringListMeta *_meta,
  QGroupBox  *parent)
{
  meta = _meta;
  metaValue = *meta;

  QGridLayout *grid;

  grid = new QGridLayout(parent);

  if (parent) {
      parent->setLayout(grid);
      parent->setWhatsThis(lpubWT(WT_GUI_SUBMODEL_LEVEL_COLORS, parent->title()));
  } else {
      setLayout(grid);
      setWhatsThis(lpubWT(WT_GUI_SUBMODEL_LEVEL_COLORS, tr("Submodel Level Colors")));
  }

  // 01
  subModelColor0Label = new QLabel(tr("Level One"),parent);
  grid->addWidget(subModelColor0Label,0,0);

  subModelColor0Example = new QLabel(parent);
  subModelColor0Example->setFrameStyle(QFrame::Sunken|QFrame::Panel);
  subModelColor0Example->setAutoFillBackground(true);
  QColor c = QColor(metaValue.value(Level1));
  QString styleSheet = QString("QLabel { background-color: rgb(%1, %2, %3); }").
      arg(c.red()).arg(c.green()).arg(c.blue());
  subModelColor0Example->setStyleSheet(styleSheet);
  grid->addWidget(subModelColor0Example,0,1);

  subModelColor0Button = new QPushButton(tr("Change"));
  connect(subModelColor0Button,SIGNAL(             clicked(bool)),
          this,                  SLOT(browseSubModelColor0(bool)));

  grid->addWidget(subModelColor0Button,0,2);

  // 02
  subModelColor1Label = new QLabel(tr("Level Two"),parent);
  grid->addWidget(subModelColor1Label,1,0);

  subModelColor1Example = new QLabel(parent);
  subModelColor1Example->setFrameStyle(QFrame::Sunken|QFrame::Panel);
  subModelColor1Example->setAutoFillBackground(true);
  c = QColor(metaValue.value(Level2));
  styleSheet = QString("QLabel { background-color: rgb(%1, %2, %3); }").
      arg(c.red()).arg(c.green()).arg(c.blue());
  subModelColor1Example->setStyleSheet(styleSheet);
  grid->addWidget(subModelColor1Example,1,1);

  subModelColor1Button = new QPushButton(tr("Change"));
  connect(subModelColor1Button,SIGNAL(             clicked(bool)),
          this,                  SLOT(browseSubModelColor1(bool)));

  grid->addWidget(subModelColor1Button,1,2);

  // 03
  subModelColor2Label = new QLabel(tr("Level Three"),parent);
  grid->addWidget(subModelColor2Label,2,0);

  subModelColor2Example = new QLabel(parent);
  subModelColor2Example->setFrameStyle(QFrame::Sunken|QFrame::Panel);
  subModelColor2Example->setAutoFillBackground(true);
  c = QColor(metaValue.value(Level3));
  styleSheet = QString("QLabel { background-color: rgb(%1, %2, %3); }").
      arg(c.red()).arg(c.green()).arg(c.blue());
  subModelColor2Example->setStyleSheet(styleSheet);
  grid->addWidget(subModelColor2Example,2,1);

  subModelColor2Button = new QPushButton(tr("Change"));
  connect(subModelColor2Button,SIGNAL(             clicked(bool)),
          this,                  SLOT(browseSubModelColor2(bool)));

  grid->addWidget(subModelColor2Button,2,2);

  // 04
  subModelColor3Label = new QLabel(tr("Level Four"),parent);
  grid->addWidget(subModelColor3Label,3,0);

  subModelColor3Example = new QLabel(parent);
  subModelColor3Example->setFrameStyle(QFrame::Sunken|QFrame::Panel);
  subModelColor3Example->setAutoFillBackground(true);
  c = QColor(metaValue.value(Level4));
  styleSheet = QString("QLabel { background-color: rgb(%1, %2, %3); }").
      arg(c.red()).arg(c.green()).arg(c.blue());
  subModelColor3Example->setStyleSheet(styleSheet);
  grid->addWidget(subModelColor3Example,3,1);

  subModelColor3Button = new QPushButton(tr("Change"));
  connect(subModelColor3Button,SIGNAL(             clicked(bool)),
          this,                  SLOT(browseSubModelColor3(bool)));

  grid->addWidget(subModelColor3Button,3,2);

  subModelLevel0ColorModified = false;
  subModelLevel1ColorModified = false;
  subModelLevel2ColorModified = false;
  subModelLevel3ColorModified = false;
}

void SubModelColorGui::browseSubModelColor0(bool)
{
  QColor qcolor = LDrawColor::color(meta->value(Level1));
  QColor newColor = QColorDialog::getColor(qcolor,this);
  if (newColor.isValid() && qcolor != newColor) {
    subModelColor0Example->setAutoFillBackground(true);
    QString styleSheet = QString("QLabel { background-color: rgb(%1, %2, %3); }")
        .arg(newColor.red()).arg(newColor.green()).arg(newColor.blue());
    subModelColor0Example->setStyleSheet(styleSheet);
    meta->setValue(Level1, newColor.name());
    subModelLevel0ColorModified = metaModified(meta->value(Level1) != metaValue.value(Level1));
  }
}

void SubModelColorGui::browseSubModelColor1(bool)
{
  QColor qcolor = LDrawColor::color(meta->value(Level2));
  QColor newColor = QColorDialog::getColor(qcolor,this);
  if (newColor.isValid() && qcolor != newColor) {
    subModelColor1Example->setAutoFillBackground(true);
    QString styleSheet = QString("QLabel { background-color: rgb(%1, %2, %3); }")
        .arg(newColor.red()).arg(newColor.green()).arg(newColor.blue());
    subModelColor1Example->setStyleSheet(styleSheet);
    meta->setValue(Level2, newColor.name());
    subModelLevel1ColorModified = metaModified(meta->value(Level2) != metaValue.value(Level2));
  }
}

void SubModelColorGui::browseSubModelColor2(bool)
{
  QColor qcolor = LDrawColor::color(meta->value(Level3));
  QColor newColor = QColorDialog::getColor(qcolor,this);
  if (newColor.isValid() && qcolor != newColor) {
    subModelColor2Example->setAutoFillBackground(true);
    QString styleSheet = QString("QLabel { background-color: rgb(%1, %2, %3); }")
        .arg(newColor.red()).arg(newColor.green()).arg(newColor.blue());
    subModelColor2Example->setStyleSheet(styleSheet);
    meta->setValue(Level3, newColor.name());
    subModelLevel2ColorModified = metaModified(meta->value(Level3) != metaValue.value(Level3));
  }
}

void SubModelColorGui::browseSubModelColor3(bool)
{
  QColor qcolor = LDrawColor::color(meta->value(Level4));
  QColor newColor = QColorDialog::getColor(qcolor,this);
  if (newColor.isValid() && qcolor != newColor) {
    subModelColor3Example->setAutoFillBackground(true);
    QString styleSheet = QString("QLabel { background-color: rgb(%1, %2, %3); }")
        .arg(newColor.red()).arg(newColor.green()).arg(newColor.blue());
    subModelColor3Example->setStyleSheet(styleSheet);
    meta->setValue(Level4, newColor.name());
    subModelLevel3ColorModified = metaModified(meta->value(Level4) != metaValue.value(Level4));
  }
}

void SubModelColorGui::apply(
  QString &topLevelFile)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(topLevelFile,meta);
  }
}

/***********************************************************************
 *
 * Classes below are not derived from MetaGui and do not write Meta commands
 *
 **********************************************************************/

/***********************************************************************
 *
 * Universal Rotate Step and Target
 *
 **********************************************************************/

void TargetRotateDialogGui::getTargetAndRotateValues(QStringList &keyList) {

  QDialog *dialog = new QDialog();

  dialog->setWindowTitle(tr("Target And Rotation"));

  dialog->setWhatsThis(lpubWT(WT_DIALOG_LOOK_AT_TARGET_AND_STEP_ROTATION,dialog->windowTitle()));

  QFormLayout *form = new QFormLayout(dialog);
  form->addRow(new QLabel(tr("LookAt Target Position")));
  QGroupBox *targetBox = new QGroupBox(tr("Target"));
  targetBox->setWhatsThis(lpubWT(WT_CONTROL_LOOK_AT_TARGET,targetBox->title()));
  form->addWidget(targetBox);
  QGridLayout *subGridLayout = new QGridLayout(targetBox);

  // Target
  targetValues[TR_AXIS_X] = keyList.at(K_TARGETX).toInt();
  targetValues[TR_AXIS_Y] = keyList.at(K_TARGETY).toInt();
  targetValues[TR_AXIS_Z] = keyList.at(K_TARGETZ).toInt();

  QList<QLabel *> targetLabelList;
  QStringList targetLabels = QStringList()
      << QString("X Position:")
      << QString("Y Position:")
      << QString("Z Position:");

  for(int i = 0; i < static_cast<int>(TR_NUM_AXIS); ++i) {
    QLabel *label = new QLabel(targetLabels[i], dialog);
    targetLabelList << label;
    subGridLayout->addWidget(label,i,0);
    QSpinBox *spinBox = new QSpinBox(dialog);
    spinBox->setRange(-10000,10000);
    spinBox->setSingleStep(1);
    spinBox->setValue(targetValues[i]);
    targetSpinBoxList << spinBox;
    connect(spinBox,SIGNAL(     valueChanged(int)),
            this,     SLOT(enableTargetReset(int)));
    subGridLayout->addWidget(spinBox,i,1);
    subGridLayout->setColumnStretch(1,1);

    QPushButton *button = new QPushButton(dialog);
    button->setIcon(QIcon(":/resources/resetaction.png"));
    button->setIconSize(QSize(16,16));
    button->setStyleSheet("QPushButton { background-color: rgba(255,255,255,0);border: 0px; }");
    button->setToolTip(tr("Reset"));
    button->setEnabled(false);
    targetButtonList << button;
    connect(button, SIGNAL(  clicked(bool)),
            this,     SLOT(spinReset(bool)));
    subGridLayout->addWidget(button,i,2);
  }

  form->addRow(new QLabel(tr("Step Rotation")));
  QGroupBox *rotateBox = new QGroupBox(tr("Rotation"));
  rotateBox->setWhatsThis(lpubWT(WT_GUI_STEP_ROTATION,rotateBox->title()));
  form->addWidget(rotateBox);
  subGridLayout = new QGridLayout(rotateBox);

  // Rotstep
  auto decimalPlaces = [] (double value)
  {
    const int residual = value - (int)value;
    const int decimalSize = QString::number(residual).size();
    return decimalSize < 3 ? 2 : decimalSize;
  };

  rotateValues[TR_AXIS_X] = keyList.at(K_ROTSX).toDouble();
  rotateValues[TR_AXIS_Y] = keyList.at(K_ROTSY).toDouble();
  rotateValues[TR_AXIS_Z] = keyList.at(K_ROTSZ).toDouble();

  QList<QLabel *> rotateLabelList;
  QStringList rotateLabels = QStringList()
      << QString("X Angle:   ")
      << QString("Y Angle:   ")
      << QString("Z Angle:   ");

  for(int i = 0; i < static_cast<int>(TR_NUM_AXIS); ++i) {
    QLabel *label = new QLabel(rotateLabels[i], dialog);
    rotateLabelList << label;
    subGridLayout->addWidget(label,i,0);
    QDoubleSpinBox * doubleSpinBox = new QDoubleSpinBox(dialog);
    doubleSpinBox->setRange(-360.0,360.0);
    doubleSpinBox->setSingleStep(1.0);
    doubleSpinBox->setDecimals(decimalPlaces(rotateValues[i]));
    doubleSpinBox->setValue(rotateValues[i]);
    rotateDoubleSpinBoxList << doubleSpinBox;
    connect(doubleSpinBox,SIGNAL(     valueChanged(double)),
            this,           SLOT(enableRotateReset(double)));
    subGridLayout->addWidget(doubleSpinBox,i,1);
    subGridLayout->setColumnStretch(1,1);

    QPushButton *button = new QPushButton(dialog);
    button->setIcon(QIcon(":/resources/resetaction.png"));
    button->setIconSize(QSize(16,16));
    button->setStyleSheet("QPushButton { background-color: rgba(255,255,255,0);border: 0px; }");
    button->setToolTip(tr("Reset"));
    button->setEnabled(false);
    rotateButtonList << button;
    connect(button,       SIGNAL(  clicked(bool)),
            this,           SLOT(spinReset(bool)));
    subGridLayout->addWidget(button,i,2);
  }

  transformValue = keyList.at(K_ROTSTYPE);
  QLabel *label = new QLabel(tr("Transform:"), dialog);
  subGridLayout->addWidget(label,TR_NUM_AXIS,0);
  typeCombo = new QComboBox(dialog);
  typeCombo->addItem("ABS");
  typeCombo->addItem("REL");
  typeCombo->addItem("ADD");
  typeCombo->setCurrentIndex(typeCombo->findText(transformValue));
  subGridLayout->addWidget(typeCombo,TR_NUM_AXIS,1,1,1);

  QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                             Qt::Horizontal, dialog);
  form->addRow(&buttonBox);
  QObject::connect(&buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
  QObject::connect(&buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));
  dialog->setMinimumWidth(250);

  if (dialog->exec() == QDialog::Accepted) {
    keyList.replace(K_TARGETX,QString::number(targetSpinBoxList[TR_AXIS_X]->value()));
    keyList.replace(K_TARGETY,QString::number(targetSpinBoxList[TR_AXIS_Y]->value()));
    keyList.replace(K_TARGETZ,QString::number(targetSpinBoxList[TR_AXIS_Z]->value()));

    keyList.replace(K_ROTSX,QString::number(rotateDoubleSpinBoxList[TR_AXIS_X]->value()));
    keyList.replace(K_ROTSY,QString::number(rotateDoubleSpinBoxList[TR_AXIS_Y]->value()));
    keyList.replace(K_ROTSZ,QString::number(rotateDoubleSpinBoxList[TR_AXIS_Z]->value()));
    keyList.replace(K_ROTSTYPE,typeCombo->currentText());
  } else {
    return;
  }
}

void TargetRotateDialogGui::enableTargetReset(int value)
{
  if (sender() == targetSpinBoxList[TR_AXIS_X])
      targetButtonList[TR_AXIS_X]->setEnabled(value != targetValues[TR_AXIS_X]);
  else
  if (sender() == targetSpinBoxList[TR_AXIS_Y])
    targetButtonList[TR_AXIS_Y]->setEnabled(value != targetValues[TR_AXIS_Y]);
  else
  if (sender() == targetSpinBoxList[TR_AXIS_Z])
    targetButtonList[TR_AXIS_Z]->setEnabled(value != targetValues[TR_AXIS_Z]);
}

void TargetRotateDialogGui::enableRotateReset(double value)
{
  if (sender() == rotateDoubleSpinBoxList[TR_AXIS_X])
    rotateButtonList[TR_AXIS_X]->setEnabled(MetaGui::notEqual(value, rotateValues[TR_AXIS_X]));
  else
  if (sender() == rotateDoubleSpinBoxList[TR_AXIS_Y])
    rotateButtonList[TR_AXIS_Y]->setEnabled(MetaGui::notEqual(value, rotateValues[TR_AXIS_Y]));
  else
  if (sender() == rotateDoubleSpinBoxList[TR_AXIS_Z])
    rotateButtonList[TR_AXIS_Z]->setEnabled(MetaGui::notEqual(value, rotateValues[TR_AXIS_Z]));
}

void TargetRotateDialogGui::spinReset(bool)
{
  if (sender() == targetButtonList[TR_AXIS_X]) {
    targetButtonList[TR_AXIS_X]->setEnabled(false);
    if (targetSpinBoxList[TR_AXIS_X]) {
      targetSpinBoxList[TR_AXIS_X]->setValue(targetValues[TR_AXIS_X]);
      targetSpinBoxList[TR_AXIS_X]->setFocus();
    }
  } else
  if (sender() == targetButtonList[TR_AXIS_Y]) {
    targetButtonList[TR_AXIS_Y]->setEnabled(false);
    if (targetSpinBoxList[TR_AXIS_Y]) {
      targetSpinBoxList[TR_AXIS_Y]->setValue(targetValues[TR_AXIS_Y]);
      targetSpinBoxList[TR_AXIS_Y]->setFocus();
    }
  } else
  if (sender() == targetButtonList[TR_AXIS_Z]) {
    targetButtonList[TR_AXIS_Z]->setEnabled(false);
    if (targetSpinBoxList[TR_AXIS_Z]) {
      targetSpinBoxList[TR_AXIS_Z]->setValue(targetValues[TR_AXIS_Z]);
      targetSpinBoxList[TR_AXIS_Z]->setFocus();
    }
  } else
  if (sender() == rotateButtonList[TR_AXIS_X]) {
    rotateButtonList[TR_AXIS_X]->setEnabled(false);
    if (rotateDoubleSpinBoxList[TR_AXIS_X]) {
      rotateDoubleSpinBoxList[TR_AXIS_X]->setValue(targetValues[TR_AXIS_X]);
      rotateDoubleSpinBoxList[TR_AXIS_X]->setFocus();
    }
  } else
  if (sender() == rotateButtonList[TR_AXIS_Y]) {
    rotateButtonList[TR_AXIS_Y]->setEnabled(false);
    if (rotateDoubleSpinBoxList[TR_AXIS_Y]) {
      rotateDoubleSpinBoxList[TR_AXIS_Y]->setValue(targetValues[TR_AXIS_Y]);
      rotateDoubleSpinBoxList[TR_AXIS_Y]->setFocus();
    }
  } else
  if (sender() == rotateButtonList[TR_AXIS_Z]) {
    rotateButtonList[TR_AXIS_Z]->setEnabled(false);
    if (rotateDoubleSpinBoxList[TR_AXIS_Z]) {
      rotateDoubleSpinBoxList[TR_AXIS_Z]->setValue(targetValues[TR_AXIS_Z]);
      rotateDoubleSpinBoxList[TR_AXIS_Z]->setFocus();
    }
  }
}

/***********************************************************************
 *
 * Open With Program
 *
 **********************************************************************/
OpenWithProgramDialogGui::OpenWithProgramDialogGui() {
    maxPrograms = Preferences::maxOpenWithPrograms;
}

void OpenWithProgramDialogGui::validateProgramEntries() {
  int lastEntry = programEntries.size();
  if (lastEntry < maxPrograms) {
    QSettings Settings;
    QStringList programs = Settings.value(QString("%1/%2").arg(SETTINGS,"OpenWithProgramList")).toStringList();
    for (int i = lastEntry; i < maxPrograms; i++) {
      const QString programEntry = i+1 <= programs.size() ? programs.at(i) : QString("Program %1|").arg(i+1);
      programEntries.append(programEntry);
    }
  } else {
    for (int i = lastEntry; i > maxPrograms; i--) {
      if (programEntries.size() == i) {
        programEntries.removeLast();
      }
      if (programIconList.size() == i) {
        programsLayout->removeWidget(programIconList.last());
        delete programIconList.last();
        programIconList.removeLast();
      }
      if (programNameEditList.size() == i) {
        programsLayout->removeWidget(programNameEditList.last());
        delete programNameEditList.last();
        programNameEditList.removeLast();
      }
      if (programPathEditList.size() == i) {
        programsLayout->removeWidget(programPathEditList.last());
        delete programPathEditList.last();
        programPathEditList.removeLast();
      }
      if (programBrowseButtonList.size() == i) {
        programsLayout->removeWidget(programBrowseButtonList.last());
        delete programBrowseButtonList.last();
        programBrowseButtonList.removeLast();
      }
    }
  }
}

QIcon OpenWithProgramDialogGui::getProgramIcon(const QString &programPath) {
  const QString programName = QString("%1icon.png").arg(QFileInfo(programPath).baseName());
  const QString iconFile = QString("%1/%2").arg(QDir::tempPath(), programName);
  if (!QFileInfo::exists(iconFile)) {
    QPixmap iconPixmap(16,16);
    if (programPath.isEmpty()) {
      iconPixmap.fill(Qt::transparent);
    } else {
      QFileInfo programInfo(programPath);
      QFileSystemModel *fsModel = new QFileSystemModel;
      fsModel->setRootPath(programInfo.path());
      QIcon fileIcon = fsModel->fileIcon(fsModel->index(programInfo.filePath()));
      iconPixmap = fileIcon.pixmap(16,16);
      if (!iconPixmap.save(iconFile))
        emit gui->messageSig(LOG_INFO,QString(tr("Could not save program file icon: %1")).arg(iconFile));
      return fileIcon;
    }
  }
  return QIcon(iconFile);
}

void OpenWithProgramDialogGui::setProgramEntry(int i) {
  const QString programName = programEntries.at(i).split("|").first();
  const QString programPath = QDir::toNativeSeparators(programEntries.at(i).split("|").last());

  QLabel *programLabel = new QLabel("",dialog);
  programLabel->setPixmap(getProgramIcon(programPath).pixmap(16,16));
  if (i < programIconList.size()) {
    programsLayout->removeWidget(programIconList.at(i));
    programIconList.replace(i,programLabel);
  } else {
    programIconList.append(programLabel);
  }
  programsLayout->addWidget(programLabel,i,0);

  QLineEdit *programNameEdit = new QLineEdit(programName, dialog);
  programNameEdit->setClearButtonEnabled(true);
  programNameEdit->setToolTip(tr("Edit program name"));
  if (i < programNameEditList.size()) {
    programsLayout->removeWidget(programNameEditList.at(i));
    programNameEditList.replace(i,programNameEdit);
  } else {
    programNameEditList.append(programNameEdit);
  }
  programsLayout->addWidget(programNameEdit,i,1);

  QLineEdit *programPathEdit = new QLineEdit(programPath, dialog);
  programPathEdit->setClearButtonEnabled(true);
  programPathEdit->setToolTip(tr("Edit program path - program arguments are supported, use quotes with spaced names"));
  if (i < programPathEditList.size()) {
    programsLayout->removeWidget(programPathEditList.at(i));
    programPathEditList.replace(i,programPathEdit);
  } else {
    programPathEditList.append(programPathEdit);
  }
  programsLayout->addWidget(programPathEdit,i,2);

  QPushButton *programBrowseButton = new QPushButton(QString("Browse..."), dialog);
  programBrowseButton->setProperty("programIndex",i);
  if (i < programBrowseButtonList.size()) {
    programsLayout->removeWidget(programBrowseButtonList.at(i));
    programBrowseButtonList.replace(i,programBrowseButton);
  } else {
    programBrowseButtonList.append(programBrowseButton);
  }
  programsLayout->addWidget(programBrowseButton,i,3);
  QObject::connect(programBrowseButton, SIGNAL(clicked(bool)), this, SLOT(browseOpenWithProgram(bool)));
}

void OpenWithProgramDialogGui::setOpenWithProgram()
{
  dialog = new QDialog();

  dialog->setWindowTitle(tr("Open With Programs Setup"));

  QVBoxLayout *mainLayout = new  QVBoxLayout(dialog);

  // max programs box
  QGroupBox *maxProgramsBox = new QGroupBox("", dialog);
  mainLayout->addWidget(maxProgramsBox);
  QHBoxLayout *maxProgramsLayout = new QHBoxLayout(maxProgramsBox);
  maxProgramsBox->setLayout(maxProgramsLayout);

  QLabel *maxProgramsLabel = new QLabel(tr("Program Slots:"), dialog);

  QSpinBox * maxProgramsSpinBox = new QSpinBox(dialog);
  maxProgramsSpinBox->setToolTip(tr("Adjust the number of program slots availabe in the dialogue"));
  maxProgramsSpinBox->setRange(1,100);
  maxProgramsSpinBox->setSingleStep(1);
  maxProgramsSpinBox->setValue(maxPrograms);

  QSpacerItem *horizontalSpacer= new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Fixed);
  maxProgramsLayout->addWidget(maxProgramsLabel);
  maxProgramsLayout->addWidget(maxProgramsSpinBox);
  maxProgramsLayout->addItem(  horizontalSpacer);

  QSettings Settings;
  QString const openWithProgramListKey("OpenWithProgramList");
  QString const maxOpenWithProgramsKey("MaxOpenWithPrograms");

  // open with programs
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,openWithProgramListKey))) {
    for (int i = 0; i < maxPrograms; i++)
      programEntries.append(QString("Program %1|").arg(i + 1));
    Settings.setValue(QString("%1/%2").arg(SETTINGS,openWithProgramListKey), programEntries);
  } else {
    programEntries = Settings.value(QString("%1/%2").arg(SETTINGS,openWithProgramListKey)).toStringList();
    if (programEntries.size() > maxPrograms) {
      maxPrograms = programEntries.size();
      maxProgramsSpinBox->setValue(maxPrograms);
    }
  }

  connect(maxProgramsSpinBox,SIGNAL(valueChanged(int)), this,SLOT (maxProgramsValueChanged(int)));

  // program box
  QGroupBox *programBox = new QGroupBox(tr("Open With Programs"),dialog);
  mainLayout->addWidget(programBox);
  programsLayout = new QGridLayout(programBox);
  programsLayout->setColumnStretch(1,10);
  programsLayout->setColumnStretch(2,30);
  programBox->setLayout(programsLayout);

  // system editor box
  QGroupBox *systemEditorBox = new QGroupBox(tr("System Editor"), dialog);
  mainLayout->addWidget(systemEditorBox);
  systemEditorLayout = new QGridLayout(systemEditorBox);
  systemEditorBox->setLayout(systemEditorLayout);

  validateProgramEntries();

  // open with programs
  for(int i = 0; i < maxPrograms; ++i) {
    setProgramEntry(i);
  }

  // system editor box
  const QString programPath = Preferences::systemEditor;
  QLabel *systemEditorLabel = new QLabel("",dialog);
  systemEditorLabel->setPixmap(getProgramIcon(programPath).pixmap(16,16));
  systemEditorLayout->addWidget(systemEditorLabel,0,0);
  systemEditorEdit = new QLineEdit(programPath, dialog);
  systemEditorEdit->setClearButtonEnabled(true);
#ifdef Q_OS_MACOS
  systemEditorEdit->setToolTip(tr("Select text editor and arguments or leave blank to use 'open -e' - TextEdit"));
#else
  systemEditorEdit->setToolTip(tr("Select text editor and arguments or leave blank to use the operating system designated editor"));
#endif
  systemEditorLayout->addWidget(systemEditorEdit,0,1);
  systemEditorButton = new QPushButton(QString("Browse..."), dialog);
  systemEditorLayout->addWidget(systemEditorButton,0,2);
  connect(systemEditorButton, SIGNAL(clicked(bool)), this, SLOT(browseSystemEditor(bool)));

  // ok cancel button box
  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                                     Qt::Horizontal, dialog);
  mainLayout->addWidget(buttonBox);
  connect(buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));

  dialog->setMinimumWidth(500);

  if (dialog->exec() == QDialog::Accepted) {
    bool updateProgramEntries = false;
    bool updateMaxPrograms    = maxPrograms != Preferences::maxOpenWithPrograms;
    QString programName, programPath;

    for(int i = 0; i < maxPrograms; ++i) {
      programPath = QDir::toNativeSeparators(programPathEditList.at(i)->text());
      programName = programNameEditList.at(i)->text();
      if (programName.isEmpty())
        programName = QFileInfo(programPath).fileName();
      QString updateEntry = QString("%1|%2").arg(programName, programPath);
      if (updateEntry != programEntries.at(i)) {
        programEntries.replace(i,updateEntry);
        updateProgramEntries = true;
      }
    }

    if (updateProgramEntries || updateMaxPrograms) {
      Settings.setValue(QString("%1/%2").arg(SETTINGS,openWithProgramListKey), programEntries);
      if (updateMaxPrograms) {
        Preferences::maxOpenWithPrograms = maxPrograms;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,maxOpenWithProgramsKey), maxPrograms);
      }
      gui->createOpenWithActions(maxPrograms);
      gui->statusBarMsg(tr("Open with programs updated."));
    }
    if (systemEditorEdit->text().toLower() != Preferences::systemEditor.toLower()) {
      QString const systemEditorKey("SystemEditor");
      QFileInfo systemEditorInfo(systemEditorEdit->text());
      if (systemEditorInfo.exists()) {
        Preferences::systemEditor = systemEditorEdit->text();
        Settings.setValue(QString("%1/%2").arg(SETTINGS,systemEditorKey), Preferences::systemEditor);
        emit gui->messageSig(LOG_INFO,QString("System editor set to %1").arg(Preferences::systemEditor));
      } else if (systemEditorEdit->text().isEmpty()) {
        Preferences::systemEditor.clear();
        Settings.remove(QString("%1/%2").arg(SETTINGS,systemEditorKey));
      }
    }
  } else {
    return;
  }
}

void OpenWithProgramDialogGui::adjustWidget()
{
  dialog->adjustSize();
}

void OpenWithProgramDialogGui::maxProgramsValueChanged(int value)
{
  bool addProgram = value > maxPrograms;

  maxPrograms = value;

  validateProgramEntries();

  if (addProgram)
    setProgramEntry(--value);

  QTimer::singleShot(0, this, SLOT(adjustWidget()));
}

void OpenWithProgramDialogGui::browseSystemEditor(bool) {
  QFileDialog dialog(nullptr);
  dialog.setWindowTitle(tr("Locate System Editor"));
  dialog.setFileMode(QFileDialog::ExistingFile);
  if (dialog.exec()) {
    QStringList selectedFiles = dialog.selectedFiles();
    if (selectedFiles.size()) {
      QFileInfo fileInfo(selectedFiles.at(0));
      if (fileInfo.exists()) {
        systemEditorEdit->setText(selectedFiles.at(0));
      }
    }
  }
}

void OpenWithProgramDialogGui::browseOpenWithProgram(bool)
{
  int programIndex = -1;
  for(int i = 0; i < maxPrograms; ++i) {
    if (sender() == programBrowseButtonList.at(i)) {
      programIndex = programBrowseButtonList.at(i)->property("programIndex").toInt();
      break;
    }
  }
  if (programIndex == -1) {
    emit gui->messageSig(LOG_ERROR,tr("Could not locate open with program index"));
    return;
  }
  QString programName = programEntries.at(programIndex).split("|").first();
  QString programPath = QDir::toNativeSeparators(programEntries.at(programIndex).split("|").last());

  QFileDialog dialog(nullptr);
  dialog.setWindowTitle(tr("Locate %1").arg(programName));
  dialog.setFileMode(QFileDialog::ExistingFile);
  if (!programPath.isEmpty())
    dialog.setDirectory(QFileInfo(programPath).absolutePath());
  else
    dialog.setDirectory(QFileInfo(Gui::getCurFile()).absolutePath());
  if (dialog.exec()) {
    QStringList selectedFiles = dialog.selectedFiles();
    if (selectedFiles.size()) {
      QFileInfo  fileInfo(selectedFiles.at(0));
      if (fileInfo.exists()) {
        programPathEditList.at(programIndex)->setText(selectedFiles.at(0));
      }
    }
  }
}

/***********************************************************************
 *
 * Build Modifications
 *
 **********************************************************************/
void BuildModDialogGui::setBuildModActive(QListWidgetItem *item)
{
  activeBuildModItem = item;
}

void BuildModDialogGui::getBuildMod(QStringList & buildModKeys, int action) {

  QString actionLabel;
  switch(action) {
  case BuildModApplyRc:
    actionLabel = tr("Apply");
    break;
  case BuildModRemoveRc:
    actionLabel = tr("Remove");
    break;
  case BM_CHANGE:
    actionLabel = tr("Edit");
    break;
  case BM_DELETE:
    actionLabel = tr("Delete");
    break;
  default:
    return;
  }

  QDialog *dialog = new QDialog();

  dialog->setWindowTitle(tr("Build Modifications"));

  dialog->setWhatsThis(lpubWT(WT_DIALOG_BUILD_MODIFICATIONS,dialog->windowTitle()));

  QFormLayout *form = new QFormLayout(dialog);
  form->addRow(new QLabel(tr("%1 Build Modifications").arg(actionLabel)));
  QGroupBox *buildModBox = new QGroupBox(tr("Select Build Modification To %1").arg(actionLabel.toLower()));
  form->addWidget(buildModBox);

  QHBoxLayout *hLayout = new QHBoxLayout(buildModBox);
  buildModBox->setLayout(hLayout);

  QListWidget *buildModList  = new QListWidget(dialog);
  buildModList->setSelectionMode(QListWidget::SingleSelection);
//  if (actionLabel == "Apply" || actionLabel == "Delete")
//      buildModList->setSelectionMode(QListWidget::ExtendedSelection);

  activeBuildModItem = nullptr;

  if (gui->buildModsCount()) {
    Q_FOREACH (QString buildMod, gui->getBuildModsList()) {
      QListWidgetItem* buildModItem = new QListWidgetItem(buildMod);
      buildModList->addItem(buildModItem);
    }
    buildModList->setCurrentRow(0);

    activeBuildModItem = buildModList->currentItem();

    hLayout->addWidget(buildModList);

    QObject::connect(buildModList, SIGNAL(      itemClicked(QListWidgetItem*)),
                     this,         SLOT(  setBuildModActive(QListWidgetItem*)));
    QObject::connect(buildModList, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
                     this,         SLOT(  setBuildModActive(QListWidgetItem*)));
  } else {
    QString message = tr("No build modifications were detected!");
    QLabel *label = new QLabel(message, dialog);
    hLayout->addWidget(label);
  }

  QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                             Qt::Horizontal, dialog);
  form->addRow(&buttonBox);
  QObject::connect(&buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
  QObject::connect(&buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));
  dialog->setMinimumWidth(250);

  if (dialog->exec() == QDialog::Accepted) {
    if (activeBuildModItem)
      buildModKeys.append(activeBuildModItem->text());
  }
}


/***********************************************************************
 *
 * POVRay renderer
 *
 **********************************************************************/
/*
    K_STEPNUMBER = 0,  // 0  not used
    K_IMAGEWIDTH,      // 1  not used
    K_RESOLUTION,      // 2
    K_RESOLUTIONTYPE,  // 3
    K_MODELSCALE,      // 4
    K_FOV,             // 5  not used
    K_LATITUDE,        // 6
    K_LONGITUDE,       // 7
    K_TARGETX,         // 8
    K_TARGETY,         // 9
    K_TARGETZ,         // 10
    K_ROTSX,           // 11
    K_ROTSY,           // 12
    K_ROTSZ,           // 13
    K_ROTSTYPE         // 14
*/

POVRayRenderDialogGui::PovraySettings POVRayRenderDialogGui::povraySettings[] = {
/* 0  LBL_ALPHA                0   QCheckBox   */ {QObject::tr("Transparent Background   "), QObject::tr("Specify whether to render a background")},
/* 1  LBL_ASPECT               1   QCheckBox   */ {QObject::tr("Maintain Aspect Ratio    "), QObject::tr("Specify whether maintain a one to one image aspect ratio")},
/* 2  LBL_WIDTH                2/0 QLineEdit   */ {QObject::tr("Width                    "), QObject::tr("Specify the image width in pixels")},
/* 3  LBL_HEIGHT               3/1 QLineEdit   */ {QObject::tr("Height                   "), QObject::tr("Specify the image height in pixels")},
/* 4  LBL_LATITUDE             4/2 QLineEdit   */ {QObject::tr("Camera Anlge - Latitude  "), QObject::tr("Set the longitudinal camera angle for the rendered image")},
/* 5  LBL_LONGITUDE            5/3 QLineEdit   */ {QObject::tr("Camera Anlge - Longitude "), QObject::tr("Set the latitudinal camera angle for the rendered image")},
/* 6  LBL_RESOLUTION           6/4 QLineEdit   */ {QObject::tr("Resolution               "), QObject::tr("Specify the resolution in pixels per inch")},
/* 7  LBL_SCALE                7/5 QLineEdit   */ {QObject::tr("Scale                    "), QObject::tr("Specify the output image scale")},

/* 8  LBL_QUALITY              8   QComboBox   */ {QObject::tr("Quality                  "), QObject::tr("Select the POV-Ray render level of quality")},

/* 9  LBL_TARGET_AND_ROTATE    9/0 QToolButton */ {QObject::tr("LookAt Target and Rotstep"), QObject::tr("Specify the target 'Look At' position and/or apply ROTSTEP angles")},
/* 10 LBL_LDV_EXPORT_SETTINGS 10/1 QToolButton */ {QObject::tr("Export Settings          "), QObject::tr("Specify LDView POV-Ray export settings")},
/* 11 LBL_LDV_LDRAW_SETTINGS  11/2 QToolButton */ {QObject::tr("LDraw Preferences        "), QObject::tr("Specify LDView LDraw preferences")}
};

void POVRayRenderDialogGui::getRenderSettings(
    QStringList &csiKeyList,
    int         &width,
    int         &height,
    int         &quality,
    bool        &alpha)
{
  QDialog *dialog = new QDialog(nullptr);

  dialog->setWindowTitle(tr("POV-Ray Render Settings"));

  dialog->setWhatsThis(lpubWT(WT_DIALOG_POVRAY_RENDER_SETTINGS,dialog->windowTitle()));

  QFormLayout *form = new QFormLayout(dialog);
  QGroupBox *settingsBox = new QGroupBox(tr("Select Rendered Image Settings"),dialog);
  form->addWidget(settingsBox);
  QFormLayout *settingsSubform = new QFormLayout(settingsBox);

  mWidth      = width;
  mHeight     = height;
  mQuality    = quality;
  mCsiKeyList = csiKeyList;
  editedCsiKeyList = csiKeyList;

  for(int i = 0; i < numSettings(); ++i) {
    QLabel *label = new QLabel(dialog);
    label->setText(povraySettings[i].label);
    label->setToolTip(povraySettings[i].tooltip);
    settingLabelList << label;

    if (i < LBL_WIDTH) {
      QCheckBox *checkBox = new QCheckBox(dialog);
      checkBox->setChecked(true);
      checkBox->setToolTip(povraySettings[i].tooltip);
      checkBoxList << checkBox;
      settingsSubform->addRow(label,checkBox);
    } else if (i < LBL_QUALITY) {
      QLineEdit *lineEdit = new QLineEdit(dialog);
      lineEdit->setText(
        i == LBL_WIDTH      ? QString::number(mWidth) :
        i == LBL_HEIGHT     ? QString::number(mHeight) :
        i == LBL_LATITUDE   ? csiKeyList.at(K_LATITUDE) :
        i == LBL_LONGITUDE  ? csiKeyList.at(K_LONGITUDE) :
        i == LBL_RESOLUTION ? csiKeyList.at(K_RESOLUTION) :
                              csiKeyList.at(K_MODELSCALE));
      if (i < LBL_LATITUDE) {      // width, height
        connect(lineEdit,SIGNAL(textChanged(const QString&)),
                this,    SLOT  (textChanged(const QString&)));
        lineEdit->setValidator(new QIntValidator(16, RENDER_IMAGE_MAX_SIZE));
      }
      else if (i < LBL_RESOLUTION) // latitued, longitude
        lineEdit->setValidator(new QIntValidator(1, 360));
      else if (i < LBL_SCALE)      // resolution
        lineEdit->setValidator(new QIntValidator(50, INT_MAX));
      else                         // scale
        lineEdit->setValidator(new QDoubleValidator(0.1,1000.0,1));
      lineEdit->setToolTip(povraySettings[i].tooltip);
      lineEditList << lineEdit;
      connect(lineEdit, SIGNAL( textEdited(const QString&)),
              this,     SLOT(  enableReset(const QString&)));
      QAction *resetEditAct = lineEdit->addAction(QIcon(":/resources/resetaction.png"), QLineEdit::TrailingPosition);
      resetEditAct->setText(tr("Reset"));
      resetEditAct->setEnabled(false);
      resetEditActList << resetEditAct;
      connect(resetEditAct, SIGNAL(triggered()),
              this,         SLOT(  editReset()));
      settingsSubform->addRow(label,lineEdit);
    } else if (i < LBL_TARGET_AND_ROTATE) {
      QString items = QString("High|Medium|Low");
      qualityCombo = new QComboBox(dialog);
      qualityCombo->setCurrentIndex(mQuality);
      qualityCombo->addItems(items.split("|"));
      qualityCombo->setToolTip(povraySettings[i].tooltip);
      settingsSubform->addRow(label,qualityCombo);
    } else {
      QToolButton *toolButton = new QToolButton(dialog);
      toolButton->setText(tr("..."));
      if (i == LBL_TARGET_AND_ROTATE)
        connect(toolButton,SIGNAL(clicked()),
                this,SLOT  (setLookAtTargetAndRotate()));
      else if (i == LBL_LDV_EXPORT_SETTINGS)
        connect(toolButton,SIGNAL(clicked()),
                this,SLOT  (setLdvExportSettings()));
      else if (i == LBL_LDV_LDRAW_SETTINGS)
        connect(toolButton,SIGNAL(clicked()),
                this,SLOT  (setLdvLDrawPreferences()));
      toolButton->setToolTip(povraySettings[i].tooltip);
      toolButtonList << toolButton;
      settingsSubform->addRow(label,toolButton);
    }
  }

  if (!settingsSubform->rowCount())
    settingsSubform = nullptr;

  QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                             Qt::Horizontal, dialog);
  form->addRow(&buttonBox);
  QObject::connect(&buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
  QObject::connect(&buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));

  QPushButton *resetButton = new QPushButton(tr("Reset"));
  buttonBox.addButton(resetButton,QDialogButtonBox::ActionRole);
  QObject::connect(resetButton,SIGNAL(clicked()), this,SLOT(resetSettings()));

  if (dialog->exec() == QDialog::Accepted) {
    quality = qualityCombo->currentIndex();
    alpha   = checkBoxList[ALPHA_BOX]->isChecked();
    width   = lineEditList.at(WIDTH_EDIT)->text().toInt();
    height  = lineEditList.at(HEIGHT_EDIT)->text().toInt();
    editedCsiKeyList[K_LATITUDE]   = lineEditList.at(LATITUDE_EDIT)->text();
    editedCsiKeyList[K_LONGITUDE]  = lineEditList.at(LONGITUDE_EDIT)->text();
    editedCsiKeyList[K_RESOLUTION] = lineEditList.at(RESOLUTION_EDIT)->text();
    editedCsiKeyList[K_MODELSCALE] = lineEditList.at(SCALE_EDIT)->text();
    csiKeyList = editedCsiKeyList;
  }
}

void POVRayRenderDialogGui::enableReset(const QString &displayText)
{
  if (sender() == lineEditList[WIDTH_EDIT])
    resetEditActList[WIDTH_EDIT]->setEnabled(displayText != QString::number(mWidth));
  else
  if (sender() == lineEditList[HEIGHT_EDIT])
    resetEditActList[HEIGHT_EDIT]->setEnabled(displayText != QString::number(mHeight));
  else
  if (sender() == lineEditList[LATITUDE_EDIT])
    resetEditActList[LATITUDE_EDIT]->setEnabled(displayText != mCsiKeyList.at(K_LATITUDE));
  else
  if (sender() == lineEditList[LONGITUDE_EDIT])
    resetEditActList[LONGITUDE_EDIT]->setEnabled(displayText != mCsiKeyList.at(K_LONGITUDE));
  else
  if (sender() == lineEditList[RESOLUTION_EDIT])
    resetEditActList[RESOLUTION_EDIT]->setEnabled(displayText !=mCsiKeyList.at(K_RESOLUTION));
  else
  if (sender() == lineEditList[SCALE_EDIT])
    resetEditActList[SCALE_EDIT]->setEnabled(displayText != mCsiKeyList.at(K_MODELSCALE));
}

void POVRayRenderDialogGui::editReset()
{
  if (sender() == resetEditActList[WIDTH_EDIT]) {
    resetEditActList[WIDTH_EDIT]->setEnabled(false);
    if (lineEditList[WIDTH_EDIT]) {
      lineEditList[WIDTH_EDIT]->setText(QString::number(mWidth));
    }
  }
  else
  if (sender() == resetEditActList[HEIGHT_EDIT]) {
    resetEditActList[HEIGHT_EDIT]->setEnabled(false);
    if (lineEditList[HEIGHT_EDIT]) {
      lineEditList[HEIGHT_EDIT]->setText(QString::number(mHeight));
    }
  }
  else
  if (sender() == resetEditActList[LATITUDE_EDIT]) {
    resetEditActList[LATITUDE_EDIT]->setEnabled(false);
    if (lineEditList[LATITUDE_EDIT]) {
      lineEditList[LATITUDE_EDIT]->setText(mCsiKeyList.at(K_LATITUDE));
    }
  }
  else
  if (sender() == resetEditActList[LONGITUDE_EDIT]) {
    resetEditActList[LONGITUDE_EDIT]->setEnabled(false);
    if (lineEditList[LONGITUDE_EDIT]) {
      lineEditList[LONGITUDE_EDIT]->setText(mCsiKeyList.at(K_LONGITUDE));
    }
  }
  else
  if (sender() == resetEditActList[RESOLUTION_EDIT]) {
    resetEditActList[RESOLUTION_EDIT]->setEnabled(false);
    if (lineEditList[RESOLUTION_EDIT]) {
      lineEditList[RESOLUTION_EDIT]->setText(mCsiKeyList.at(K_RESOLUTION));
    }
  }
  else
  if (sender() == resetEditActList[SCALE_EDIT]) {
    resetEditActList[SCALE_EDIT]->setEnabled(false);
    if (lineEditList[SCALE_EDIT]) {
      lineEditList[SCALE_EDIT]->setText(mCsiKeyList.at(K_MODELSCALE));
    }
  }
}

int POVRayRenderDialogGui::numSettings() {
  int size = 0;
  if (!povraySettings[0].label.isEmpty())
    size = sizeof(povraySettings)/sizeof(povraySettings[0]);
  return size;
}

void POVRayRenderDialogGui::setLookAtTargetAndRotate()
{
  TargetRotateDialogGui dialog;
  dialog.getTargetAndRotateValues(editedCsiKeyList);
}

void POVRayRenderDialogGui::setLdvLDrawPreferences()
{
  Render::showLdvLDrawPreferences(POVRayRender);
}

void POVRayRenderDialogGui::setLdvExportSettings()
{
  Render::showLdvExportSettings(POVRayRender);
}

void POVRayRenderDialogGui::resetSettings()
{
  const QVector<QString> resetValues = { QString::number(mWidth),
                                         QString::number(mHeight),
                                         mCsiKeyList.at(K_LATITUDE),
                                         mCsiKeyList.at(K_LONGITUDE),
                                         mCsiKeyList.at(K_RESOLUTION),
                                         mCsiKeyList.at(K_MODELSCALE) };
  for(int i = 0; i < numSettings(); ++i) {
    if (i < LBL_WIDTH) {                        // alpha, aspect
      for(int j = 0; j < checkBoxList.size(); ++j)
        checkBoxList[j]->setChecked(true);
      i += checkBoxList.size();
    } else if (i < LBL_QUALITY) {               //width, height, latitude, longitude, resolution, scale
      for(int j = 0; j < lineEditList.size(); ++j)
        lineEditList[j]->setText(resetValues.at(j));
      i += lineEditList.size();
    } else if (i == LBL_TARGET_AND_ROTATE) {    // quality
      qualityCombo->setCurrentIndex(mQuality);
    } else if (i > LBL_TARGET_AND_ROTATE)
      break;
  }
  editedCsiKeyList = mCsiKeyList;
}

void POVRayRenderDialogGui::textChanged(const QString &value)
{
  /* original height x new width / original width = new height */
  int mNewValue = value.toInt();
  if (checkBoxList.at(ASPECT_BOX)->isChecked()) {
    if (sender() == lineEditList.at(WIDTH_EDIT)) {
      disconnect(lineEditList.at(HEIGHT_EDIT),SIGNAL(textChanged(const QString&)),
                 this,                        SLOT  (textChanged(const QString&)));
      lineEditList.at(HEIGHT_EDIT)->setText(QString::number(qRound(double(mHeight * mNewValue / mWidth))));
      connect(lineEditList.at(HEIGHT_EDIT),SIGNAL(textChanged(const QString&)),
              this,                          SLOT  (textChanged(const QString&)));
    } else
    if (sender() == lineEditList.at(HEIGHT_EDIT)) {
      disconnect(lineEditList.at(WIDTH_EDIT),SIGNAL(textChanged(const QString&)),
                 this,                       SLOT  (textChanged(const QString&)));
      lineEditList.at(WIDTH_EDIT)->setText(QString::number(qRound(double(mNewValue * mWidth / mHeight))));
      connect(lineEditList.at(WIDTH_EDIT),SIGNAL(textChanged(const QString&)),
              this,                       SLOT  (textChanged(const QString&)));
    }
  }
}
