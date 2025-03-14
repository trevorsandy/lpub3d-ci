/****************************************************************************
**
** Copyright (C) 2021 - 2025 Trevor SANDY. All rights reserved.
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
 * This file implements a dialog that allows the user to select the
 * preferred renderer and its associated settings.
 *
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/

#include <QVBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>

#include "preferredrendererdialog.h"
#include "metagui.h"
#include "commonmenus.h"

PreferredRendererDialog::PreferredRendererDialog(
  RendererData &goods,
  QString      _name,
  QWidget      *parent)
  : QDialog(parent)
{
  setWindowTitle(_name);

  setWhatsThis(lpubWT(WT_DIALOG_PREFERRED_RENDERER,windowTitle()));

  meta.setValue(goods);

  QVBoxLayout *layout = new QVBoxLayout(this);
  setLayout(layout);

  QGroupBox *box = new QGroupBox("Renderer",this);
  layout->addWidget(box);
  preferredRenderer = new PreferredRendererGui(&meta,box);

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox(this);

  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  layout->addWidget(buttonBox);

  setModal(true);
  setMinimumSize(300,20);
}

PreferredRendererDialog::~PreferredRendererDialog()
{
}

bool PreferredRendererDialog::getPreferredRenderer(
  PreferredRendererMeta &meta,
  QString                name,
  QWidget               *parent)
{
  RendererData goods = meta.value();
  PreferredRendererDialog *dialog = new PreferredRendererDialog(goods,name,parent);

  bool ok = dialog->exec() == QDialog::Accepted;
  if (ok) {
    if (dialog->meta.value().renderer != goods.renderer)
      commonMenus.setWhatsThis();
    meta.setValue(dialog->meta.value());
  }
  return ok;
}

void PreferredRendererDialog::accept()
{
  if (preferredRenderer->modified) {
    QDialog::accept();
  } else {
    QDialog::reject();
  }
}

void PreferredRendererDialog::cancel()
{
  QDialog::reject();
}
