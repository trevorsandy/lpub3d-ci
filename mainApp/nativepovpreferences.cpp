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
#include "lpub_preferences.h"
#include "lc_application.h"
#include "version.h"
#include "name.h"

NativePovPreferencesDialog::NativePovPreferencesDialog(QWidget *parent) :
     QDialog(parent)
{
    ui.setupUi(this);

    ui.povQualityCombo->addItems(                   QString(QUALITY_COMBO_DEFAULT).split(",", QString::SkipEmptyParts));
    ui.povQualityCombo->setCurrentIndex(            Preferences::quality);
    ui.povSelectedAspectRatioCombo->addItems(       QString(SELECTED_ASPECT_RATIO_COMBO_DEFAULT).split(",", QString::SkipEmptyParts));
    ui.povSelectedAspectRatioCombo->setCurrentIndex(Preferences::selectedAspectRatio);
    ui.povCustomAspectRatioLnEdit->setEnabled(      Preferences::selectedAspectRatio == 8); // Custom
    ui.povCustomAspectRatioLnEdit->setText(         QString::number(Preferences::customAspectRatio));
    ui.povEdgeLineRadiusLnEdit->setText(            QString::number(Preferences::edgeRadius));
    ui.povSeamWidthLnEdit->setText(                 QString::number(Preferences::seamWidth));
    ui.povAmbientLnEdit->setText(                   QString::number(Preferences::ambient));
    ui.povDiffuseLnEdit->setText(                   QString::number(Preferences::diffuse));
    ui.povReflLnEdit->setText(                      QString::number(Preferences::refl));
    ui.povPhongLnEdit->setText(                     QString::number(Preferences::phong));
    ui.povPhongSizeLnEdit->setText(                 QString::number(Preferences::phongSize));
    ui.povTransReflLnEdit->setText(                 QString::number(Preferences::transRefl));
    ui.povTransFilterLnEdit->setText(               QString::number(Preferences::transFilter));
    ui.povTransIoRLnEdit->setText(                  QString::number(Preferences::transIoR));
    ui.povRubberReflLnEdit->setText(                QString::number(Preferences::rubberRefl));
    ui.povRubberPhongLnEdit->setText(               QString::number(Preferences::rubberPhong));
    ui.povRubberPhongSizeLnEdit->setText(           QString::number(Preferences::rubberPhongSize));
    ui.povChromeReflLnEdit->setText(                QString::number(Preferences::chromeRefl));
    ui.povChromeBrillianceLnEdit->setText(          QString::number(Preferences::chromeBril));
    ui.povChromeSpecularLnEdit->setText(            QString::number(Preferences::chromeSpecular));
    ui.povChromeRoughnessLnEdit->setText(           QString::number(Preferences::chromeRoughness));
    ui.povFileVersionCombo->addItems(               QString(POV_FILE_VERSION_COMBO_DEFAULT).split(",", QString::SkipEmptyParts));
    ui.povFileVersionCombo->setCurrentIndex(        Preferences::fileVersion);

    ui.povSeamsGrpBox->setChecked(                  Preferences::seams);
    ui.povReflectionsChkBox->setChecked(            Preferences::reflections);
    ui.povShadowsChkBox->setChecked(                Preferences::shadows);
    ui.povXmlMapGrpBox->setChecked(                 Preferences::xmlMap);
    ui.povInlinePovChkBox->setChecked(              Preferences::inlinePov);
    ui.povSmoothCurvesChkBox->setChecked(           Preferences::smoothCurves);
    ui.povHideStudsChkBox->setChecked(              Preferences::hideStuds);
    ui.povUnmirrorStudsChkBox->setEnabled(          Preferences::quality == 3); // Include stud logo
    ui.povUnmirrorStudsChkBox->setChecked(          Preferences::unmirrorStuds);
    ui.povFindReplacementsChkBox->setChecked(       Preferences::findReplacements);
    ui.povConditionalEdgeLinesChkBox->setChecked(   Preferences::conditionalEdgeLines);
    ui.povPrimitiveSubstitutionChkBox->setChecked(  Preferences::primitiveSubstitution);
    ui.povEdgeLinesGrpBox->setChecked(              gApplication->mPreferences.mDrawEdgeLines);

    ui.povXmlMapPathLnEdit->setText(                Preferences::xmlMapPath);
    ui.povTopIncludeLnEdit->setText(                Preferences::topInclude);
    ui.povBottomIncludeLnEdit->setText(             Preferences::bottomInclude);

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
      ui.povUnmirrorStudsChkBox->setChecked(Preferences::unmirrorStuds);
}

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
QString const NativePovPreferencesDialog::bottomInclude()
{
  return ui.povBottomIncludeLnEdit->displayText();
}
QString const NativePovPreferencesDialog::topInclude()
{
  return ui.povTopIncludeLnEdit->displayText();
}
QString const NativePovPreferencesDialog::xmlMapPath()
{
  return ui.povXmlMapPathLnEdit->displayText();
}
bool NativePovPreferencesDialog::findReplacements()
{
  return ui.povFindReplacementsChkBox->isChecked();
}
bool NativePovPreferencesDialog::conditionalEdgeLines()
{
  return ui.povConditionalEdgeLinesChkBox->isChecked();
}
bool NativePovPreferencesDialog::primitiveSubstitution()
{
  return ui.povPrimitiveSubstitutionChkBox->isChecked();
}
bool NativePovPreferencesDialog::unmirrorStuds()
{
  return ui.povUnmirrorStudsChkBox->isChecked();
}
bool NativePovPreferencesDialog::hideStuds()
{
  return ui.povHideStudsChkBox->isChecked();
}
bool NativePovPreferencesDialog::smoothCurves()
{
  return ui.povSmoothCurvesChkBox->isChecked();
}
bool NativePovPreferencesDialog::inlinePov()
{
  return ui.povInlinePovChkBox->isChecked();
}
bool NativePovPreferencesDialog::xmlMap()
{
  return ui.povXmlMapGrpBox->isChecked();
}
bool NativePovPreferencesDialog::shadows()
{
  return ui.povShadowsChkBox->isChecked();
}
bool NativePovPreferencesDialog::reflections()
{
  return ui.povReflectionsChkBox->isChecked();
}
bool NativePovPreferencesDialog::seams()
{
  return ui.povSeamsGrpBox->isChecked();
}
int NativePovPreferencesDialog::fileVersion()
{
  return ui.povFileVersionCombo->currentIndex();
}
float NativePovPreferencesDialog::chromeRoughness()
{
  return ui.povChromeRoughnessLnEdit->displayText().toFloat();
}
float NativePovPreferencesDialog::chromeSpecular()
{
  return ui.povChromeSpecularLnEdit->displayText().toFloat();
}
float NativePovPreferencesDialog::chromeBril()
{
  return ui.povChromeBrillianceLnEdit->displayText().toFloat();
}
float NativePovPreferencesDialog::chromeRefl()
{
  return ui.povChromeReflLnEdit->displayText().toFloat();
}
float NativePovPreferencesDialog::rubberPhongSize()
{
  return ui.povRubberPhongSizeLnEdit->displayText().toFloat();
}
float NativePovPreferencesDialog::rubberPhong()
{
  return ui.povRubberPhongLnEdit->displayText().toFloat();
}
float NativePovPreferencesDialog::rubberRefl()
{
  return ui.povRubberReflLnEdit->displayText().toFloat();
}
float NativePovPreferencesDialog::transIoR()
{
  return ui.povTransIoRLnEdit->displayText().toFloat();
}
float NativePovPreferencesDialog::transFilter()
{
  return ui.povTransFilterLnEdit->displayText().toFloat();
}
float NativePovPreferencesDialog::transRefl()
{
  return ui.povTransReflLnEdit->displayText().toFloat();
}
float NativePovPreferencesDialog::phongSize()
{
  return ui.povPhongSizeLnEdit->displayText().toFloat();
}
float NativePovPreferencesDialog::phong()
{
  return ui.povPhongLnEdit->displayText().toFloat();
}
float NativePovPreferencesDialog::refl()
{
  return ui.povReflLnEdit->displayText().toFloat();
}
float NativePovPreferencesDialog::diffuse()
{
  return ui.povDiffuseLnEdit->displayText().toFloat();
}
float NativePovPreferencesDialog::ambient()
{
  return ui.povAmbientLnEdit->displayText().toFloat();
}
float NativePovPreferencesDialog::edgeRadius()
{
  return ui.povEdgeLineRadiusLnEdit->displayText().toFloat();
}
float NativePovPreferencesDialog::customAspectRatio()
{
  return ui.povCustomAspectRatioLnEdit->displayText().toFloat();
}
float NativePovPreferencesDialog::seamWidth()
{
  return ui.povSeamWidthLnEdit->displayText().toFloat();
}
int NativePovPreferencesDialog::selectedAspectRatio()
{
  return ui.povSelectedAspectRatioCombo->currentIndex();
}
int NativePovPreferencesDialog::quality()
{
  return ui.povQualityCombo->currentIndex();
}

// reset buttons
void NativePovPreferencesDialog::on_povGeneralGrpResetBtn_clicked()
{
    ui.povFileVersionCombo->setCurrentIndex(        FILE_VERSION_DEFAULT);
    ui.povQualityCombo->setCurrentIndex(            QUALITY_DEFAULT);
    ui.povSelectedAspectRatioCombo->setCurrentIndex(SELECTED_ASPECT_RATIO_DEFAULT);
    ui.povCustomAspectRatioLnEdit->setText(         QString::number(CUSTOM_ASPECT_RATIO_DEFAULT));
    ui.povReflectionsChkBox->setChecked(            true);
    ui.povShadowsChkBox->setChecked(                true);
    ui.povUnmirrorStudsChkBox->setChecked(          true);
    ui.povTopIncludeLnEdit->setText(                TOP_INCLUDE_DEFAULT);
    ui.povBottomIncludeLnEdit->setText(             BOTTM_INCLUDE_DEFAULT);
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

void NativePovPreferencesDialog::accept(){
  QDialog::accept();
}

void NativePovPreferencesDialog::cancel(){
  QDialog::reject();
}
