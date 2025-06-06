
/****************************************************************************
**
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

#include <QWidget>
#include <QString>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QDialogButtonBox>

#include "globals.h"
#include "meta.h"
#include "metagui.h"
#include "metaitem.h"


#include "lpub_preferences.h"
#include "step.h"

/**********************************************************************
 *
 * FadeStep
 *
 *********************************************************************/

class GlobalFadeStepPrivate
{
public:
  Meta       meta;
  QString    topLevelFile;
  QList<MetaGui *> children;
  bool  reloadFile;
  bool  keepWork;

  GlobalFadeStepPrivate(QString &_topLevelFile, Meta &_meta)
  : reloadFile(false), keepWork(false)
  {
    topLevelFile = _topLevelFile;
    meta         = _meta;

    MetaItem mi; // examine all the globals and then return

    mi.sortedGlobalWhere(meta,topLevelFile,"ZZZZZZZ");
  }
};

/*
 * fade step
 * color
 */

GlobalFadeStepDialog::GlobalFadeStepDialog(
  QString &topLevelFile,
  Meta &meta)
{
  data = new GlobalFadeStepPrivate(topLevelFile,meta);
  FadeStepsMeta *fadeStepsMeta = &data->meta.LPub.fadeSteps;
  LPubMeta *lpubMeta = &data->meta.LPub;

  setWindowTitle(tr("Fade Steps Globals Setup"));

  setWhatsThis(lpubWT(WT_SETUP_FADE_STEPS,windowTitle()));

  QVBoxLayout *layout = new QVBoxLayout(this);
  GlobalSizeWidget sw(QSize(40,50), QSize(40,40));
  layout->addWidget(&sw);
  setLayout(layout);

  QGroupBox *box = new QGroupBox(tr("Fade Steps"));
  box->setToolTip(tr("Enable fade previous steps"));
  layout->addWidget(box);
  fadeStepsChild = new FadeStepsGui(fadeStepsMeta,box);
  data->children.append(fadeStepsChild);

  box = new QGroupBox("Final Model Step");
  box->setToolTip(tr("Automatically, append an un-faded final step to the top level model file."));
  layout->addWidget(box);
  finalModelEnabledChild = new FinalModelEnabledGui(tr("Enable Final Model Step"),&lpubMeta->finalModelEnabled,box);
  finalModelEnabledChild->getCheckBox()->setObjectName("EnableFinalModel");
  data->children.append(finalModelEnabledChild);

  // enable final model dialog
  connect (fadeStepsChild->getFadeCheckBox(), SIGNAL(clicked(bool)), this, SLOT(enableControls(bool)));
  emit fadeStepsChild->getFadeCheckBox()->clicked(fadeStepsChild->getFadeCheckBox()->isChecked());
  // ...then wire-up the rest of the connections
  connect (fadeStepsChild->getFadeCheckBox(), SIGNAL(clicked(bool)), this, SLOT(reloadDisplayPage(bool)));
  connect (fadeStepsChild->getSetupCheckBox(), SIGNAL(clicked(bool)), this, SLOT(reloadDisplayPage(bool)));
  connect (fadeStepsChild->getLPubFadeCheckBox(), SIGNAL(clicked(bool)), this, SLOT(reloadDisplayPage(bool)));
  connect (finalModelEnabledChild->getCheckBox(), SIGNAL(clicked(bool)), this, SLOT(reloadDisplayPage(bool)));

  QDialogButtonBox *buttonBox = new QDialogButtonBox();
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));
  layout->addWidget(buttonBox);

  setModal(true);
}

void GlobalFadeStepDialog::enableControls(bool b)
{
  finalModelEnabledChild->getCheckBox()->setEnabled(b);
}

void GlobalFadeStepDialog::reloadDisplayPage(bool b)
{
  Q_UNUSED(b)
  if (QObject *obj = sender())
    if (obj->objectName() == QStringLiteral("EnableFinalModel"))
      data->keepWork = true;
  if (!data->reloadFile)
    data->reloadFile = true;
}

void GlobalFadeStepDialog::getFadeStepGlobals(
  QString topLevelFile, Meta &meta)
{
  GlobalFadeStepDialog *dialog = new GlobalFadeStepDialog(topLevelFile, meta);
  dialog->exec();
}

void GlobalFadeStepDialog::accept()
{
  MetaItem mi;

  mi.beginMacro("GlobalFadeStep");

  bool noFileDisplay = false;

  for (MetaGui *child : data->children) {
    child->apply(data->topLevelFile);
    noFileDisplay |= child->modified;
  }

  if (data->reloadFile)
    mi.setSuspendFileDisplayFlag(true);
  else
    mi.setSuspendFileDisplayFlag(!noFileDisplay);

  mi.endMacro();

  if (data->reloadFile)
      mi.clearAndReloadModelFile(false/*closeAndOpen*/, true/*savePrompt*/, data->keepWork);

  QDialog::accept();
}

void GlobalFadeStepDialog::cancel()
{
  QDialog::reject();
}
