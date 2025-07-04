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
 * This file implements a few dialogs that contain textually editable
 * versions of number that describe configuration of LDraw files.
 *
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/

#include "pairdialog.h"
#include <QLineEdit>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QDoubleValidator>

#include "version.h"
#include "commonmenus.h"

FloatDialog::FloatDialog(
  QString  title,
  QString  _label0,
  FloatMeta *leaf)
{
  setWindowTitle(title);

  setWhatsThis(lpubWT(WT_DIALOG_FLOAT,windowTitle()));

  QString string = QString("%1") .arg(leaf->value(),
                                      leaf->_fieldWidth,
                                      'f',
                                      leaf->_precision);

  float0 = new QLineEdit(string,this);
  float0->setMaxLength(leaf->_fieldWidth);
  float0->setInputMask(leaf->_inputMask);
  float0->setAlignment(Qt::AlignRight);

  label0 = new QLabel(_label0);

  grid = new QGridLayout;

  grid->addWidget(label0,0,0);
  grid->addWidget(float0,0,1);

  buttonBox = new QDialogButtonBox(this);
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()), SLOT(reject()));

  grid->addWidget(buttonBox,2,0,2,2);

  setLayout(grid);

  setModal(true);
  setMinimumSize(40,20);
}

FloatDialog::~FloatDialog()
{
}

bool FloatDialog::getFloat(
  QString  title,
  QString  heading,
  FloatMeta *leaf,
  float   &try0)
{
  FloatDialog *dialog = new FloatDialog(title,heading,leaf);
  if (dialog->exec() == QDialog::Accepted) {
    bool ok0;
    try0 = dialog->float0->displayText().toFloat(&ok0);
    return ok0;
  } else {
    return false;
  }
}

#include <QVBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>

#include "backgrounddialog.h"
#include "metagui.h"

FloatPairDialog::FloatPairDialog(
  QString  _name,
  QString  _heading1,
  QString  _heading2,
  FloatPairMeta *leaf,
  QWidget *parent)
  : QDialog(parent)
{
  setWindowTitle(_name);

  setWhatsThis(lpubWT(WT_DIALOG_FLOAT_PAIR,windowTitle()));

  QVBoxLayout *layout = new QVBoxLayout(this);
  setLayout(layout);

  QGroupBox *box = new QGroupBox(_name,this);
  layout->addWidget(box);

  meta = leaf;

  floats = new FloatsGui(_heading1,_heading2,leaf,box);

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox(this);

  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  layout->addWidget(buttonBox);

  setModal(true);
  setMinimumSize(40,20);
}

FloatPairDialog::~FloatPairDialog()
{
}

bool FloatPairDialog::getFloatPair(
  QString       _name,
  QString       _heading1,
  QString       _heading2,
  FloatPairMeta *leaf,
  float         values[],
  QWidget       *parent)
{
  FloatPairDialog *dialog = new FloatPairDialog(_name,_heading1,_heading2,leaf,parent);

  bool ok = dialog->exec() == QDialog::Accepted;
  if (ok) {
    values[0] = dialog->meta->value(0);
    values[1] = dialog->meta->value(1);
  }
  return ok;
}

void FloatPairDialog::accept()
{
  if (floats->modified) {
    QDialog::accept();
  } else {
    QDialog::reject();
  }
}

void FloatPairDialog::cancel()
{
  QDialog::reject();
}

DoubleSpinDialog::DoubleSpinDialog(
  QString       _name,
  QString       _heading,
  FloatMeta     *floatMeta,
  float          step,
  DoubleSpinEnc  spinGui,
  QWidget       *parent)
  : QDialog(parent)
{
  setParent(parent);

  setWindowTitle(_name);

  setWhatsThis(lpubWT(WT_DIALOG_DOUBLE_SPIN,windowTitle()));

  QVBoxLayout *layout = new QVBoxLayout;
  setLayout(layout);

  QGroupBox *box = new QGroupBox(_name);
  layout->addWidget(box);

  meta = floatMeta;

  if (spinGui == DoubleSpinFOV) {
      spin = new CameraFOVGui(_heading, meta, box);
  } else if (spinGui == DoubleSpinZPlane) {
      spin = new CameraZPlaneGui(_heading, meta, false/*ZNear*/, box);
  } else if (spinGui == DoubleSpinDDF) {
      spin = new CameraDDFGui(_heading, meta, box);
  } else if (spinGui == DoubleSpinScale) {
      spin = new ScaleGui(_heading, meta, box);
  } else {
      spin = new DoubleSpinGui(
                   _heading,
                   meta,
                   meta->_min,
                   meta->_max,
                   step,
                   box);
  }

  QDialogButtonBox *buttonBox = new QDialogButtonBox;

  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  layout->addWidget(buttonBox);

  setModal(true);
  setMinimumSize(40,20);
}

bool DoubleSpinDialog::getFloat(
  QString        title,
  QString        heading,
  FloatMeta     *floatMeta,
  float          &try0,
  float          step,
  DoubleSpinEnc  spinGui,
  QWidget       *parent)
{
  DoubleSpinDialog *dialog = new DoubleSpinDialog(title,heading,floatMeta,step,spinGui,parent);
  if (dialog->exec() == QDialog::Accepted) {
    try0 = dialog->meta->value();
    return true;
  } else {
    return false;
  }
}

void DoubleSpinDialog::accept()
{
  if (spin->modified) {
    QDialog::accept();
  } else {
    QDialog::reject();
  }
}

void DoubleSpinDialog::cancel()
{
  QDialog::reject();
}

CameraAnglesDialog::CameraAnglesDialog(
  QString           _name,
  CameraAnglesData  &goods,
  QWidget           *parent)
  : QDialog(parent)
{
  setWindowTitle(_name);

  setWhatsThis(lpubWT(WT_DIALOG_CAMERA_ANGLES,windowTitle()));

  meta.setValue(goods);

  QVBoxLayout *layout = new QVBoxLayout(this);
  setLayout(layout);

  QGroupBox *box = new QGroupBox(_name,this);
  layout->addWidget(box);

  cameraAngles = new CameraAnglesGui(_name,&meta,box);

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox(this);

  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  layout->addWidget(buttonBox);

  setModal(true);
  setMinimumSize(40,20);
}

CameraAnglesDialog::~CameraAnglesDialog()
{
}

bool CameraAnglesDialog::getCameraAngles(
  QString          _name,
  CameraAnglesData &goods,
  QWidget          *parent)
{
  CameraAnglesDialog *dialog = new CameraAnglesDialog(_name,goods,parent);

  bool ok = dialog->exec() == QDialog::Accepted;
  if (ok) {
    goods = dialog->meta.value();
  }
  return ok;
}

void CameraAnglesDialog::accept()
{
  if (cameraAngles->modified) {
    QDialog::accept();
  } else {
    QDialog::reject();
  }
}

void CameraAnglesDialog::cancel()
{
  QDialog::reject();
}

CameraFOVDialog::CameraFOVDialog(
  QString    title,
  QString    heading,
  FloatMeta *fovMeta,
  FloatMeta *zNearMeta,
  FloatMeta *zFarMeta,
  QWidget   *parent)
  : QDialog(parent)
{
  setWindowTitle(title);

  setWhatsThis(lpubWT(WT_DIALOG_CAMERA_FOV,windowTitle()));

  QVBoxLayout *layout = new QVBoxLayout;
  setLayout(layout);

  QGroupBox *box = new QGroupBox(title);
  layout->addWidget(box);

  fovmeta = fovMeta;
  znearmeta = zNearMeta;
  zfarmeta = zFarMeta;

  QHBoxLayout *childlayout = new QHBoxLayout();
  box->setLayout(childlayout);

  cameraFOV = new CameraFOVGui(heading, fovmeta);
  childlayout->addWidget(cameraFOV);

  cameraZNear = new CameraZPlaneGui(tr("Camera Z Near"), znearmeta);
  childlayout->addWidget(cameraZNear);

  cameraZFar = new CameraZPlaneGui(tr("Camera Z Far"), zfarmeta,true/*ZFar*/);
  childlayout->addWidget(cameraZFar);

  QDialogButtonBox *buttonBox = new QDialogButtonBox;

  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  layout->addWidget(buttonBox);

  setModal(true);
  setMinimumSize(40,20);
}

bool CameraFOVDialog::getCameraFOV(
  QString    title,
  QString    heading,
  FloatMeta *fovMeta,
  FloatMeta *zNearMeta,
  FloatMeta *zFarMeta,
  float     &fovData,
  float     &zNearData,
  float     &zFarData,
  QWidget   *parent)
{
  CameraFOVDialog *dialog = new CameraFOVDialog(title,heading,fovMeta,zNearMeta,zFarMeta,parent);
  if (dialog->exec() == QDialog::Accepted) {
    fovData = dialog->fovmeta->value();
    zNearData = dialog->znearmeta->value();
    zFarData = dialog->zfarmeta->value();
    return true;
  } else {
    return false;
  }
}

void CameraFOVDialog::accept()
{
  if (cameraFOV->modified || cameraZNear->modified || cameraZFar->modified) {
    QDialog::accept();
  } else {
    QDialog::reject();
  }
}

void CameraFOVDialog::cancel()
{
  QDialog::reject();
}

LocalDialog::LocalDialog(
  QString  title,
  QString  question,
  QWidget *parent)
{
  QLabel *label = new QLabel(question,parent);

  QVBoxLayout *grid = new QVBoxLayout(parent);

  setWindowTitle(title);

  setWhatsThis(lpubWT(WT_DIALOG_LOCAL,windowTitle()));

  grid->addWidget(label);

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox(parent);
  buttonBox->addButton(QDialogButtonBox::Yes);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::No);
  connect(buttonBox,SIGNAL(rejected()), SLOT(reject()));

  grid->addWidget(buttonBox);

  setLayout(grid);

  setModal(true);
  setMinimumSize(40,20);
}

LocalDialog::~LocalDialog()
{
}

bool LocalDialog::getLocal(
  QString  title,
  QString  question,
  QWidget *parent)
{
  LocalDialog *dialog = new LocalDialog(title,question,parent);
  if (dialog->exec() == QDialog::Accepted) {
    return true;
  } else {
    return false;
  }
}

CycleDialog::CycleDialog(
        QString  title,
        QString  question,
        bool isEditor,
        QWidget *parent) :
        cycleResult(CycleNone),
        rememberEditorCycleChoice(false)
{
  QLabel *label = new QLabel(question,parent);

  QVBoxLayout *grid = new QVBoxLayout(parent);

  setWindowTitle(title);

  setWhatsThis(lpubWT(WT_DIALOG_CYCLE,windowTitle()));

  grid->addWidget(label);

  if (isEditor) {
      QCheckBox *cb = new QCheckBox(tr("Remember Choice and do not show this again."));
      grid->addWidget(cb);
#if QT_VERSION >= QT_VERSION_CHECK(6,9,0)
      QObject::connect(cb, &QCheckBox::checkStateChanged, this, [&](int state) {
#else
      QObject::connect(cb, &QCheckBox::stateChanged, [&](int state) {
#endif
          if (static_cast<Qt::CheckState>(state) == Qt::CheckState::Checked) {
             Preferences::setEditorCyclePagesOnUpdateDialog(false);
             rememberEditorCycleChoice = true;
          }
      });
  }

  buttonBox = new QDialogButtonBox(parent);
  buttonBox->addButton(QDialogButtonBox::Yes);
  buttonBox->addButton(QDialogButtonBox::No);
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox, SIGNAL(clicked(QAbstractButton*)),
          this,      SLOT(buttonClicked(QAbstractButton*)));

  grid->addWidget(buttonBox);

  setLayout(grid);

  setModal(true);
  setMinimumSize(40,20);
}

CycleDialog::~CycleDialog()
{
}

CyclePageDlgType CycleDialog::getCycle(
        QString  title,
        QString  question,
        bool isEditor,
        QWidget *parent)
{
  CycleDialog *dialog = new CycleDialog(title,question,isEditor,parent);
  dialog->exec();
  return dialog->cycleResult;
}

void CycleDialog::buttonClicked( QAbstractButton *button )
{
  switch (buttonBox->standardButton(button))
  {
    case QDialogButtonBox::Yes:
      cycleResult = CycleYes;
      if (rememberEditorCycleChoice)
        Preferences::setEditorCyclePagesOnUpdate(true);
      return QDialog::accept();
    case QDialogButtonBox::No:
      cycleResult = CycleNo;
      if (rememberEditorCycleChoice)
        Preferences::setEditorCyclePagesOnUpdate(false);
      break;
    case QDialogButtonBox::Cancel:
      cycleResult = CycleCancel;
      break;
    default:
      qDebug() << Q_FUNC_INFO << qPrintable("Cycle pages button not handled");
      break;
  }
  QDialog::reject();
}

OptionDialog::OptionDialog(
  QString  titles,
  QString  options,
  QWidget *parent)
{
  setParent(parent);

  QStringList titleList   = titles.split("|");
  QStringList optionsList = options.split("|");

  QVBoxLayout *vLayout = new QVBoxLayout(this);

  setWindowTitle(titleList.first());

  setWhatsThis(lpubWT(WT_DIALOG_APPEND_PAGE_OPTION,windowTitle()));

  int numOptions = optionsList.size();
  bool multiChoice = numOptions > 1;

  if (multiChoice) {
    QString groupTitle = titleList.size() > 1 ? titleList.last() :
                                             titles.replace(QString("%1 - ")
                                                           .arg(VER_PRODUCTNAME_STR),QString());
    QGroupBox *groupBox = new QGroupBox(tr("%1 Options").arg(groupTitle), this);
    QVBoxLayout *boxLayout = new QVBoxLayout(groupBox);
    groupBox->setLayout(boxLayout);
    for (int i = 0; i < numOptions; ++i) {
      QRadioButton  *optionsRadio = new QRadioButton(optionsList.at(i),groupBox);
      optionsRadio->setChecked(!i);
      boxLayout->addWidget(optionsRadio);
      buttonList.append(optionsRadio);
    }
    vLayout->addWidget(groupBox);
  } else {
    QLabel *label = new QLabel(options,this);
    vLayout->addWidget(label);
  }

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox(this);
  buttonBox->addButton(multiChoice ? QDialogButtonBox::Ok : QDialogButtonBox::Yes);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(multiChoice ? QDialogButtonBox::Cancel : QDialogButtonBox::No);
  connect(buttonBox,SIGNAL(rejected()), SLOT(reject()));

  vLayout->addWidget(buttonBox);

  setLayout(vLayout);

  setModal(true);
  setMinimumSize(40,20);
}

OptionDialog::~OptionDialog()
{
}

int OptionDialog::getOption(
  QString  titles,
  QString  options,
  QWidget *parent)
{
  OptionDialog *dialog = new OptionDialog(titles,options,parent);
  if (dialog->exec() == QDialog::Accepted) {
    if (dialog->buttonList.size()) {
      for (int i = 0; i < dialog->buttonList.size(); i++) {
        if (dialog->buttonList.at(i)->isChecked())
          return i + 1;
      }
      return 0;
    } else {
       return 0;
    }
  } else {
    return 0;
  }
}
