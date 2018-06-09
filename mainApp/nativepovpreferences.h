/****************************************************************************
**
** Copyright (C) 2018 Trevor SANDY. All rights reserved.
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

#ifndef NATIVEPOVPREFERENCESDIALOG_H
#define NATIVEPOVPREFERENCESDIALOG_H

#include <QDialog>
#include <TCUserDefaults.h>

#include "ui_nativepovpreferences.h"

namespace Ui{
class NativePovPreferencesDialog;
}

class NativePovPreferencesDialog : public QDialog
{

  Q_OBJECT

public:
  explicit NativePovPreferencesDialog(QWidget *parent = nullptr);
  ~NativePovPreferencesDialog();

  int           getSelectedAspectRatio();
  long          setSelectedAspectRatio();

//  int           quality();
//  int           selectedAspectRatio();
//  float         customAspectRatio();
//  float         seamWidth();
//  float         ambient();
//  float         diffuse();
//  float         refl();
//  float         phong();
//  float         phongSize();
//  float         transRefl();
//  float         transFilter();
//  float         transIoR();
//  float         rubberRefl();
//  float         rubberPhong();
//  float         rubberPhongSize();
//  float         chromeRefl();
//  float         chromeBril();
//  float         chromeSpecular();
//  float         chromeRoughness();
//  float         edgeRadius();
//  int           fileVersion();
//  bool          seams();
//  bool          reflections();
//  bool          shadows();
//  bool          xmlMap();
//  bool          inlinePov();
//  bool          smoothCurves();
//  bool          hideStuds();
//  bool          unmirrorStuds();
//  bool          findReplacements();
//  bool          conditionalEdgeLines();
//  bool          primitiveSubstitution();
//  QString const xmlMapPath();
//  QString const topInclude();
//  QString const bottomInclude();

  QString const lights();

signals:

public slots:
  void accept();
  void cancel();

private slots:
  void on_povTopIncludeBtn_clicked();
  void on_povBottomIncludeBtn_clicked();
  void on_povXmlMapPathBtn_clicked();
  void on_povAddLightBtn_clicked();
  void on_povRemoveLightBtn_clicked();
  void on_povLightsCombo_currentIndexChanged(int index);
  void on_povQualityCombo_currentIndexChanged(int index);
  void on_povSelectedAspectRatioCombo_currentIndexChanged(const QString &currentText);

  void on_povLDrawGeometryGrpResetBtn_clicked();
  void on_povPovGeometryGrpResetBtn_clicked();
  void on_povGeneralGrpResetBtn_clicked();
  void on_povLightingGrpResetBtn_clicked();
  void on_povMaterialGrpResetBtn_clicked();
  void on_povRubberMaterialGrpResetBtn_clicked();
  void on_povTransMaterialGrpResetBtn_clicked();
  void on_povChromeMaterialGrpResetBtn_clicked();

private:
  Ui::NativePovPreferencesDialog ui;
  QWidget       *parent;

  // LDV SETTINGS
  std::string    m_udPrefix;

  friend class PreferencesDialog;
};
#endif // NATIVEPOVPREFERENCESDIALOG_H
