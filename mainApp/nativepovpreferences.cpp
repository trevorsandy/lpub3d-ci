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

#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

#include "ui_nativepovpreferences.h"
#include "nativepovpreferences.h"

#include <TCUserDefaults.h>
#include <LDUserDefaultsKeys.h>
#include "nativepov.h"

#include "lpub_preferences.h"
#include "lc_application.h"
#include "version.h"
#include "name.h"

NativePovPreferencesDialog::NativePovPreferencesDialog(QWidget *parent) :
     QDialog(parent)
{
    ui.setupUi(this);

    ui.povQualityCombo->addItems(                   QString(QUALITY_COMBO_DEFAULT).split(",", QString::SkipEmptyParts));
    ui.povQualityCombo->setCurrentIndex(            TCUserDefaults::longForKey(QUALITY_EXPORT_KEY, QUALITY_EXPORT_DEFAULT));
    ui.povSelectedAspectRatioCombo->addItems(       QString(SELECTED_ASPECT_RATIO_COMBO_DEFAULT).split(",", QString::SkipEmptyParts));
    ui.povSelectedAspectRatioCombo->setCurrentIndex(getSelectedAspectRatio());
    ui.povCustomAspectRatioLnEdit->setEnabled(      TCUserDefaults::longForKey(SELECTED_ASPECT_RATIO_KEY, SELECTED_ASPECT_RATIO_DEFAULT) == 7); // Custom
    ui.povCustomAspectRatioLnEdit->setText(         QString::number(TCUserDefaults::floatForKey(CUSTOM_ASPECT_RATIO_KEY, CUSTOM_ASPECT_RATIO_DEFAULT)));
    ui.povEdgeLineRadiusLnEdit->setText(            QString::number(TCUserDefaults::floatForKey(EDGE_RADIUS_KEY, EDGE_RADIUS_DEFAULT)));
    ui.povSeamWidthLnEdit->setText(                 QString::number(TCUserDefaults::floatForKey(SEAM_WIDTH_KEY, EDGE_RADIUS_DEFAULT)));
    ui.povAmbientLnEdit->setText(                   QString::number(TCUserDefaults::floatForKey(AMBIENT_KEY, AMBIENT_DEFAULT)));
    ui.povDiffuseLnEdit->setText(                   QString::number(TCUserDefaults::floatForKey(DIFFUSE_KEY, DIFFUSE_DEFAULT)));
    ui.povReflLnEdit->setText(                      QString::number(TCUserDefaults::floatForKey(REFLECTION_KEY, REFLECTION_DEFAULT)));
    ui.povPhongLnEdit->setText(                     QString::number(TCUserDefaults::floatForKey(PHONG_KEY, PHONG_DEFAULT)));
    ui.povPhongSizeLnEdit->setText(                 QString::number(TCUserDefaults::floatForKey(PHONG_SIZE_KEY, PHONG_SIZE_DEFAULT)));
    ui.povTransReflLnEdit->setText(                 QString::number(TCUserDefaults::floatForKey(TRANS_REFLECTION_KEY, TRANS_REFLECTION_DEFAULT)));
    ui.povTransFilterLnEdit->setText(               QString::number(TCUserDefaults::floatForKey(TRANS_FILTER_KEY, TRANS_FILTER_DEFAULT)));
    ui.povTransIoRLnEdit->setText(                  QString::number(TCUserDefaults::floatForKey(TRANS_IOR_KEY, TRANS_IOR_DEFAULT)));
    ui.povRubberReflLnEdit->setText(                QString::number(TCUserDefaults::floatForKey(RUBBER_REFLECTION_KEY, RUBBER_REFLECTION_DEFAULT)));
    ui.povRubberPhongLnEdit->setText(               QString::number(TCUserDefaults::floatForKey(RUBBER_PHONG_KEY, RUBBER_PHONG_DEFAULT)));
    ui.povRubberPhongSizeLnEdit->setText(           QString::number(TCUserDefaults::floatForKey(RUBBER_PHONG_SIZE_KEY, RUBBER_PHONG_SIZE_DEFAULT)));
    ui.povChromeReflLnEdit->setText(                QString::number(TCUserDefaults::floatForKey(CHROME_REFLECTION_KEY, CHROME_REFLECTION_DEFAULT)));
    ui.povChromeBrillianceLnEdit->setText(          QString::number(TCUserDefaults::floatForKey(CHROME_BRILLIANCE_KEY, CHROME_BRILLIANCE_DEFAULT)));
    ui.povChromeSpecularLnEdit->setText(            QString::number(TCUserDefaults::floatForKey(CHROME_SPECULAR_KEY, CHROME_SPECULAR_DEFAULT)));
    ui.povChromeRoughnessLnEdit->setText(           QString::number(TCUserDefaults::floatForKey(CHROME_ROUGHNESS_KEY, CHROME_ROUGHNESS_DEFAULT)));
    ui.povFileVersionCombo->addItems(               QString(POV_FILE_VERSION_COMBO_DEFAULT).split(",", QString::SkipEmptyParts));
    ui.povFileVersionCombo->setCurrentIndex(        int(ui.povFileVersionCombo->findText(
                                                    QString::number(TCUserDefaults::floatForKey(FILE_VERSION_KEY, FILE_VERSION_DEFAULT)))));
    ui.povSeamsGrpBox->setChecked(                  TCUserDefaults::boolForKey(SEAMS_KEY, true));
    ui.povReflectionsChkBox->setChecked(            TCUserDefaults::boolForKey(REFLECTIONS_KEY, true));
    ui.povShadowsChkBox->setChecked(                TCUserDefaults::boolForKey(SHADOWS_KEY, true));
    ui.povXmlMapGrpBox->setChecked(                 TCUserDefaults::boolForKey(XML_MAP_KEY, true));
    ui.povInlinePovChkBox->setChecked(              TCUserDefaults::boolForKey(INLINE_POV_KEY, true));
    ui.povSmoothCurvesChkBox->setChecked(           TCUserDefaults::boolForKey(SMOOTH_CURVES_KEY, true));
    ui.povHideStudsChkBox->setChecked(              TCUserDefaults::boolForKey(HIDE_STUDS_KEY, false));
    ui.povUnmirrorStudsChkBox->setEnabled(          TCUserDefaults::longForKey(QUALITY_EXPORT_KEY, QUALITY_EXPORT_DEFAULT) == 3); // Include stud logo
    ui.povUnmirrorStudsChkBox->setChecked(          TCUserDefaults::boolForKey(UNMIRROR_STUDS_KEY, true));
    ui.povFindReplacementsChkBox->setChecked(       TCUserDefaults::boolForKey(FIND_REPLACEMENTS_KEY, false));
    ui.povConditionalEdgeLinesChkBox->setChecked(   TCUserDefaults::boolForKey(CONDITIONAL_EDGE_LINES_KEY, false));
    ui.povPrimitiveSubstitutionChkBox->setChecked(  TCUserDefaults::boolForKey(PRIMITIVE_SUBSTITUTION_KEY, true));
    ui.povEdgeLinesGrpBox->setChecked(              gApplication->mPreferences.mDrawEdgeLines ||
                                                    TCUserDefaults::boolForKey(DRAW_EDGES_KEY, false));

    ui.povXmlMapPathLnEdit->setText(                !QString(TCUserDefaults::pathForKey(XML_MAP_PATH_KEY)).isEmpty() ?
                                                     QString(TCUserDefaults::pathForKey(XML_MAP_PATH_KEY)) :
                                                    !Preferences::lgeoPath.isEmpty() ?
                                                     QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lgeoPath).arg(VER_LGEO_XML_FILE)) : XML_MAP_PATH_DEFAULT);
    ui.povTopIncludeLnEdit->setText(                !QString(TCUserDefaults::stringForKey(TOP_INCLUDE_KEY)).isEmpty() ?
                                                     QString(TCUserDefaults::stringForKey(TOP_INCLUDE_KEY)) : TOP_INCLUDE_DEFAULT);
    ui.povBottomIncludeLnEdit->setText(             !QString(TCUserDefaults::stringForKey(BOTTOM_INCLUDE_KEY)).isEmpty() ?
                                                     QString(TCUserDefaults::stringForKey(BOTTOM_INCLUDE_KEY)) : BOTTOM_INCLUDE_DEFAULT);

    ui.povLightsCombo->addItems(                    Preferences::lights.split(",", QString::SkipEmptyParts));
    ui.povLightsCombo->setCurrentIndex(             ui.povLightsCombo->count() - 1);

    QStringList lightEntryFields = ui.povLightsCombo->currentText().split(" ",QString::SkipEmptyParts); // 1. Latitude 45.0 Longitude 0.0
    ui.povLightNumLnEdit->setText(                 QString("%1").arg(lightEntryFields.at(0)).replace(".",""));
    ui.povLightLatitudeLnEdit->setText(            lightEntryFields.at(2));
    ui.povLightLongitudeLnEdit->setText(           lightEntryFields.at(4));

    QPalette palette;
    palette.setColor(QPalette::Base,Qt::lightGray);
    ui.povLightNumLnEdit->setPalette(palette);
    ui.povLightNumLnEdit->setReadOnly(true);
}

NativePovPreferencesDialog::~NativePovPreferencesDialog()
{}

int NativePovPreferencesDialog::getSelectedAspectRatio()
{
  switch (int(TCUserDefaults::longForKey(SELECTED_ASPECT_RATIO_KEY, SELECTED_ASPECT_RATIO_DEFAULT)))
  {
  case -1:
          return 0;
  case 0:
          return 1;
  case 2:
          return 2;
  case 3:
          return 3;
  case 4:
          return 4;
  case 5:
          return 5;
  case 6:
          return 6;
  case 7:
          return 7;
  default:
          return 8;
  }
}

long NativePovPreferencesDialog::setSelectedAspectRatio()
{
  switch (ui.povSelectedAspectRatioCombo->currentIndex())
  {
  case 0:
          return -1;
  case 1:
          return 0;
  case 2:
          return 2;
  case 3:
          return 3;
  case 4:
          return 4;
  case 5:
          return 5;
  case 6:
          return 6;
  case 7:
          return 7;
  default:
          return 8;
  }
}

void NativePovPreferencesDialog::on_povLightsCombo_currentIndexChanged(int index)
{
  Q_UNUSED(index)

  QStringList lightEntryFields = ui.povLightsCombo->currentText().split(" ",QString::SkipEmptyParts);
  ui.povLightNumLnEdit->setText(                 QString("%1").arg(lightEntryFields.at(0)).replace(".",""));
  ui.povLightLatitudeLnEdit->setText(            lightEntryFields.at(2));
  ui.povLightLongitudeLnEdit->setText(           lightEntryFields.at(4));
}

void NativePovPreferencesDialog::on_povAddLightBtn_clicked()
{
  if (ui.povLightLatitudeLnEdit->displayText().isEmpty() || ui.povLightLongitudeLnEdit->displayText().isEmpty())
    return;

  QString lightPosition = QString("Latitude %1 Longitude %2")
      .arg(ui.povLightLatitudeLnEdit->displayText())
      .arg(ui.povLightLongitudeLnEdit->displayText());

  QString lightNumber = QString("%1").arg(ui.povLightNumLnEdit->displayText());

  QString lightEntry = QString("%1. %2").arg(lightNumber).arg(lightPosition);

  int lightIndex = ui.povLightsCombo->findText(lightEntry);

  if (lightIndex != -1)
    return;

  lightNumber = QString("%1").arg(ui.povLightsCombo->count() + 1);

  lightEntry = QString("%1. %2").arg(lightNumber).arg(lightPosition);

  ui.povLightsCombo->addItem(                    lightEntry);
  ui.povLightsCombo->setCurrentIndex(            ui.povLightsCombo->count() - 1);
  ui.povLightNumLnEdit->setText(                 QString("%1").arg(ui.povLightsCombo->count()));
}

void NativePovPreferencesDialog::on_povRemoveLightBtn_clicked()
{
    int lightIndex = ui.povLightNumLnEdit->displayText().toInt() - 1;
    ui.povLightsCombo->removeItem(                 lightIndex);
    ui.povLightsCombo->setCurrentIndex(            ui.povLightsCombo->count() - 1);
    ui.povLightNumLnEdit->setText(                 QString("%1").arg(ui.povLightsCombo->count()));
}

void NativePovPreferencesDialog::on_povTopIncludeBtn_clicked()
{
#ifdef Q_OS_WIN
    QString filter(tr("Include (*.inc);;All Files (*.*)"));
#else
    QString filter(tr("All Files (*.*)"));
#endif

    QString result = QFileDialog::getOpenFileName(this, tr("Select Top Include File"),
                                                  ui.povTopIncludeLnEdit->displayText().isEmpty() ?
                                                  QString("%1/%2/config").arg(Preferences::lpub3d3rdPartyConfigDir, VER_POVRAY_STR) :
                                                  ui.povTopIncludeLnEdit->displayText(),
                                                  filter);

    if (!result.isEmpty()) {
        result = QDir::toNativeSeparators(result);
        ui.povTopIncludeLnEdit->setText(result);
    }
}

void NativePovPreferencesDialog::on_povBottomIncludeBtn_clicked()
{
#ifdef Q_OS_WIN
    QString filter(tr("Include (*.inc);;All Files (*.*)"));
#else
    QString filter(tr("All Files (*.*)"));
#endif

    QString result = QFileDialog::getOpenFileName(this, tr("Select Bottom Include File"),
                                                  ui.povBottomIncludeLnEdit->displayText().isEmpty() ?
                                                  QString("%1/%2/config").arg(Preferences::lpub3d3rdPartyConfigDir, VER_POVRAY_STR) :
                                                  ui.povBottomIncludeLnEdit->displayText(),
                                                  filter);

    if (!result.isEmpty()) {
        result = QDir::toNativeSeparators(result);
        ui.povBottomIncludeLnEdit->setText(result);
    }
}

void NativePovPreferencesDialog::on_povXmlMapPathBtn_clicked()
{
#ifdef Q_OS_WIN
    QString filter(tr("LGEO (*.xml);;All Files (*.*)"));
#else
    QString filter(tr("All Files (*.*)"));
#endif

    QString result = QFileDialog::getOpenFileName(this, tr("Select LGEO xml File"),
                                                  ui.povXmlMapPathLnEdit->displayText().isEmpty() ?
                                                  Preferences::lgeoPath.isEmpty() ?
                                                  QString("%1/%2/resources/config").arg(Preferences::lpub3d3rdPartyAppDir, VER_LDVIEW_STR) :
                                                  Preferences::lgeoPath :
                                                  ui.povXmlMapPathLnEdit->displayText(),
                                                  filter);

    if (!result.isEmpty()) {
        result = QDir::toNativeSeparators(result);
        ui.povXmlMapPathLnEdit->setText(result);
    }
}

void NativePovPreferencesDialog::on_povSelectedAspectRatioCombo_currentIndexChanged(const QString &currentText)
{
  bool enabled = currentText == "Custom";
  ui.povCustomAspectRatioLnEdit->setEnabled(enabled);
}

void NativePovPreferencesDialog::on_povQualityCombo_currentIndexChanged(int index)
{
    bool enabled = index == 3; // Include stud logo
    ui.povUnmirrorStudsChkBox->setEnabled(enabled);
    if (enabled)
      ui.povUnmirrorStudsChkBox->setChecked(TCUserDefaults::boolForKey(UNMIRROR_STUDS_KEY, true));
}

// RESET BUTTONS
void NativePovPreferencesDialog::on_povGeneralGrpResetBtn_clicked()
{
    ui.povFileVersionCombo->setCurrentIndex(        FILE_VERSION_DEFAULT);
    ui.povQualityCombo->setCurrentIndex(            QUALITY_EXPORT_DEFAULT);
    ui.povSelectedAspectRatioCombo->setCurrentIndex(SELECTED_ASPECT_RATIO_DEFAULT);
    ui.povCustomAspectRatioLnEdit->setText(         QString::number(CUSTOM_ASPECT_RATIO_DEFAULT));
    ui.povReflectionsChkBox->setChecked(            true);
    ui.povShadowsChkBox->setChecked(                true);
    ui.povUnmirrorStudsChkBox->setChecked(          true);
    ui.povTopIncludeLnEdit->setText(                TOP_INCLUDE_DEFAULT);
    ui.povBottomIncludeLnEdit->setText(             BOTTOM_INCLUDE_DEFAULT);
}

void NativePovPreferencesDialog::on_povLDrawGeometryGrpResetBtn_clicked()
{
    ui.povEdgeLineRadiusLnEdit->setText(            QString::number(EDGE_RADIUS_DEFAULT));
    ui.povPrimitiveSubstitutionChkBox->setChecked(  false);
    ui.povConditionalEdgeLinesChkBox->setChecked(   false);
    ui.povSmoothCurvesChkBox->setChecked(           true);
    ui.povHideStudsChkBox->setChecked(              false);
    ui.povSeamsGrpBox->setChecked(                  true);
    ui.povSeamWidthLnEdit->setText(                 QString::number(SEAM_WIDTH_DEFAULT));
}

void NativePovPreferencesDialog::on_povPovGeometryGrpResetBtn_clicked()
{
    ui.povInlinePovChkBox->setChecked(              true);
    ui.povXmlMapGrpBox->setChecked(                 true);
    ui.povFindReplacementsChkBox->setChecked(       false);
    QFileInfo resourceFile(QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lgeoPath, VER_LGEO_XML_FILE)));
    if (resourceFile.exists()) {
      ui.povXmlMapPathLnEdit->setText(resourceFile.absoluteFilePath());
    } else {
      ui.povXmlMapPathLnEdit->setText(                XML_MAP_PATH_DEFAULT);
    }
}

void NativePovPreferencesDialog::on_povLightingGrpResetBtn_clicked()
{
     ui.povLightsCombo->addItems(                    QString(LIGHTS_COMBO_DEFAULT).split(",", QString::SkipEmptyParts));
     ui.povLightsCombo->setCurrentIndex(             ui.povLightsCombo->count() - 1);
     QStringList lightEntryFields = ui.povLightsCombo->currentText().split(" ",QString::SkipEmptyParts); // 1. Latitude 45.0 Longitude 0.0
     ui.povLightNumLnEdit->setText(                 QString("%1").arg(lightEntryFields.at(0)).replace(".",""));
     ui.povLightLatitudeLnEdit->setText(            lightEntryFields.at(2));
     ui.povLightLongitudeLnEdit->setText(           lightEntryFields.at(4));
     ui.povAmbientLnEdit->setText(                   QString::number(AMBIENT_DEFAULT));
     ui.povDiffuseLnEdit->setText(                   QString::number(DIFFUSE_DEFAULT));
}

void NativePovPreferencesDialog::on_povMaterialGrpResetBtn_clicked()
{
    ui.povReflLnEdit->setText(                      QString::number(REFLECTION_DEFAULT));
    ui.povPhongLnEdit->setText(                     QString::number(PHONG_DEFAULT));
    ui.povPhongSizeLnEdit->setText(                 QString::number(PHONG_SIZE_DEFAULT));
}

void NativePovPreferencesDialog::on_povTransMaterialGrpResetBtn_clicked()
{
    ui.povTransReflLnEdit->setText(                 QString::number(TRANS_REFLECTION_DEFAULT));
    ui.povTransFilterLnEdit->setText(               QString::number(TRANS_FILTER_DEFAULT));
    ui.povTransIoRLnEdit->setText(                  QString::number(TRANS_IOR_DEFAULT));
}

void NativePovPreferencesDialog::on_povRubberMaterialGrpResetBtn_clicked()
{
    ui.povRubberReflLnEdit->setText(                QString::number(RUBBER_REFLECTION_DEFAULT));
    ui.povRubberPhongLnEdit->setText(               QString::number(RUBBER_PHONG_DEFAULT));
    ui.povRubberPhongSizeLnEdit->setText(           QString::number(RUBBER_PHONG_SIZE_DEFAULT));
}

void NativePovPreferencesDialog::on_povChromeMaterialGrpResetBtn_clicked()
{
    ui.povChromeReflLnEdit->setText(                QString::number(CHROME_REFLECTION_DEFAULT));
    ui.povChromeBrillianceLnEdit->setText(          QString::number(CHROME_BRILLIANCE_DEFAULT));
    ui.povChromeSpecularLnEdit->setText(            QString::number(CHROME_SPECULAR_DEFAULT));
    ui.povChromeRoughnessLnEdit->setText(           QString::number(CHROME_ROUGHNESS_DEFAULT));
}

// PASS TO PREFERENCES
QString const NativePovPreferencesDialog::lights()
{
  QStringList povLightsComboList;
  for (int index = 0; index < ui.povLightsCombo->count(); index++)
  {
      if (index == 0)
        povLightsComboList << ui.povLightsCombo->itemText(index);
      else
        povLightsComboList << "," << ui.povLightsCombo->itemText(index);
  }
  return povLightsComboList.join(",");
}


//QString const NativePovPreferencesDialog::bottomInclude()
//{
//  return ui.povBottomIncludeLnEdit->displayText();
//}
//QString const NativePovPreferencesDialog::topInclude()
//{
//  return ui.povTopIncludeLnEdit->displayText();
//}
//QString const NativePovPreferencesDialog::xmlMapPath()
//{
//  return ui.povXmlMapPathLnEdit->displayText();
//}
//bool NativePovPreferencesDialog::findReplacements()
//{
//  return ui.povFindReplacementsChkBox->isChecked();
//}
//bool NativePovPreferencesDialog::conditionalEdgeLines()
//{
//  return ui.povConditionalEdgeLinesChkBox->isChecked();
//}
//bool NativePovPreferencesDialog::primitiveSubstitution()
//{
//  return ui.povPrimitiveSubstitutionChkBox->isChecked();
//}
//bool NativePovPreferencesDialog::unmirrorStuds()
//{
//  return ui.povUnmirrorStudsChkBox->isChecked();
//}
//bool NativePovPreferencesDialog::hideStuds()
//{
//  return ui.povHideStudsChkBox->isChecked();
//}
//bool NativePovPreferencesDialog::smoothCurves()
//{
//  return ui.povSmoothCurvesChkBox->isChecked();
//}
//bool NativePovPreferencesDialog::inlinePov()
//{
//  return ui.povInlinePovChkBox->isChecked();
//}
//bool NativePovPreferencesDialog::xmlMap()
//{
//  return ui.povXmlMapGrpBox->isChecked();
//}
//bool NativePovPreferencesDialog::shadows()
//{
//  return ui.povShadowsChkBox->isChecked();
//}
//bool NativePovPreferencesDialog::reflections()
//{
//  return ui.povReflectionsChkBox->isChecked();
//}
//bool NativePovPreferencesDialog::seams()
//{
//  return ui.povSeamsGrpBox->isChecked();
//}
//int NativePovPreferencesDialog::fileVersion()
//{
//  return ui.povFileVersionCombo->currentIndex();
//}
//float NativePovPreferencesDialog::chromeRoughness()
//{
//  return ui.povChromeRoughnessLnEdit->displayText().toFloat();
//}
//float NativePovPreferencesDialog::chromeSpecular()
//{
//  return ui.povChromeSpecularLnEdit->displayText().toFloat();
//}
//float NativePovPreferencesDialog::chromeBril()
//{
//  return ui.povChromeBrillianceLnEdit->displayText().toFloat();
//}
//float NativePovPreferencesDialog::chromeRefl()
//{
//  return ui.povChromeReflLnEdit->displayText().toFloat();
//}
//float NativePovPreferencesDialog::rubberPhongSize()
//{
//  return ui.povRubberPhongSizeLnEdit->displayText().toFloat();
//}
//float NativePovPreferencesDialog::rubberPhong()
//{
//  return ui.povRubberPhongLnEdit->displayText().toFloat();
//}
//float NativePovPreferencesDialog::rubberRefl()
//{
//  return ui.povRubberReflLnEdit->displayText().toFloat();
//}
//float NativePovPreferencesDialog::transIoR()
//{
//  return ui.povTransIoRLnEdit->displayText().toFloat();
//}
//float NativePovPreferencesDialog::transFilter()
//{
//  return ui.povTransFilterLnEdit->displayText().toFloat();
//}
//float NativePovPreferencesDialog::transRefl()
//{
//  return ui.povTransReflLnEdit->displayText().toFloat();
//}
//float NativePovPreferencesDialog::phongSize()
//{
//  return ui.povPhongSizeLnEdit->displayText().toFloat();
//}
//float NativePovPreferencesDialog::phong()
//{
//  return ui.povPhongLnEdit->displayText().toFloat();
//}
//float NativePovPreferencesDialog::refl()
//{
//  return ui.povReflLnEdit->displayText().toFloat();
//}
//float NativePovPreferencesDialog::diffuse()
//{
//  return ui.povDiffuseLnEdit->displayText().toFloat();
//}
//float NativePovPreferencesDialog::ambient()
//{
//  return ui.povAmbientLnEdit->displayText().toFloat();
//}
//float NativePovPreferencesDialog::edgeRadius()
//{
//  return ui.povEdgeLineRadiusLnEdit->displayText().toFloat();
//}
//float NativePovPreferencesDialog::customAspectRatio()
//{
//  return ui.povCustomAspectRatioLnEdit->displayText().toFloat();
//}
//float NativePovPreferencesDialog::seamWidth()
//{
//  return ui.povSeamWidthLnEdit->displayText().toFloat();
//}
//int NativePovPreferencesDialog::selectedAspectRatio()
//{
//  return ui.povSelectedAspectRatioCombo->currentIndex();
//}
//int NativePovPreferencesDialog::quality()
//{
//  return ui.povQualityCombo->currentIndex();
//}
// END PASS TO PREFERENCES

void NativePovPreferencesDialog::accept(){
  QDialog::accept();
}

void NativePovPreferencesDialog::cancel(){
  QDialog::reject();
}
