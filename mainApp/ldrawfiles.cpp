
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
 * This class reads in, manages and writes out LDraw files.  While being
 * edited an MPD file, or a top level LDraw files and any sub-model files
 * are maintained in memory using this class.
 *
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/

#include "version.h"
#include "ldrawfiles.h"
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include <random>

#include <QMessageBox>
#include <QFile>
#include <QHash>
#include <functional>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtConcurrent>
#endif
#include "paths.h"

#include "lpub.h"
#include "ldrawfilesload.h"
#include "QsLog.h"

#include "lc_library.h"
#include "pieceinf.h"
#include "lc_previewwidget.h"

QStringList LDrawFile::_itemsLoaded; 
QList<QRegularExpression> LDrawFile::_fileRegExp; 
QList<QRegularExpression> LDrawHeaderRegExp; 
QList<QRegularExpression> LDrawUnofficialPartRegExp; 
QList<QRegularExpression> LDrawUnofficialSubPartRegExp; 
QList<QRegularExpression> LDrawUnofficialPrimitiveRegExp; 
QList<QRegularExpression> LDrawUnofficialShortcutRegExp;
const QString LDrawUnofficialType[UNOFFICIAL_NUM] =
{
    QLatin1String("Unofficial Submodel"),      // UNOFFICIAL_SUBMODEL
    QLatin1String("Unofficial Part"),          // UNOFFICIAL_PART
    QLatin1String("Unofficial Generated Part"),// UNOFFICIAL_GENERATED_PART
    QLatin1String("Unofficial Subpart"),       // UNOFFICIAL_SUBPART
    QLatin1String("Unofficial Primitive"),     // UNOFFICIAL_PRIMITIVE
    QLatin1String("Unofficial Shortcut"),      // UNOFFICIAL_SHORTUCT
    QLatin1String("Unofficial Data File"),     // UNOFFICIAL_DATA
    QLatin1String("Unofficial Other File")     // UNOFFICIAL_OTHER
};

/********************************************
 *
 * routines for nested levels
 *
 ********************************************/

QList<HiarchLevel*> LDrawFile::_currentLevels;
QList<HiarchLevel*> LDrawFile::_allLevels;

HiarchLevel* addLevel(const QString& key, bool create)
{
    // if level object with specified key exists...
    const QList _levels = LDrawFile::_allLevels;
    for (HiarchLevel* level : _levels)
        // return existing level object
        if (level->key == key)
            return level;

    // else if create object spedified...
    if (create) {
        // create a new level object
        HiarchLevel* level = new HiarchLevel(key);
        // add to 'all' level objects list
        LDrawFile::_allLevels.append(level);
        // return level object
        return level;
    }

    return nullptr;
}

int getLevel(const QString& key, int position)
{
    if (position == BM_BEGIN) {
        // ensure key is specified
        if (key.isEmpty())
            emit gui->messageSig(LOG_ERROR, QString("BUILD_MOD - GetLevel: Invalid request. Key not specified."));

        // add level object to 'all' levels
        HiarchLevel* level = addLevel(key, true);

        // if there are 'currentLevel' objects...
        if (LDrawFile::_currentLevels.size())
            // set last 'current' level object as parent of this level object
            level->level = LDrawFile::_currentLevels[LDrawFile::_currentLevels.size() - 1];
        else
            // set this level object parent to nullptr
            level->level = nullptr;

        // append this level object to 'current' level objects list
        LDrawFile::_currentLevels.append(level);

    } else if (position == BM_END) {
        // if there are 'current' level objects...
        if (LDrawFile::_currentLevels.size()) {
            // return absolute level, remove last level object from 'currentLevels' - reset to parent level or BM_BASE_LEVEL [0]
            LDrawFile::_currentLevels.removeAt(LDrawFile::_currentLevels.size() - 1);
/*
#ifdef QT_DEBUG_MODE
            const QString message = QString("BUILD_MOD - Get absolute level [%1] from 'currentLevels'%2")
                                            .arg(LDrawFile::_currentLevels.size()).arg(!key.isEmpty() ? QString(" - Key: %1").arg(key) : "");

            emit gui->messageSig(LOG_DEBUG, message);
#endif
//*/
        }
    }
//*
#ifdef QT_DEBUG_MODE
    else
    if (position == BM_CURRENT) {
        const QString message = QString("DEBUG: Build Modification - Get absolute level [%1] from 'currentLevels'%2")
                                        .arg(LDrawFile::_currentLevels.size()).arg(!key.isEmpty() ? QString(" - %1").arg(key) : "");
        qDebug() << qPrintable(message);
    }
#endif
//*/

    // return the absolute level from the 'current' list
    return LDrawFile::_currentLevels.size();
}

/********************************************
 *
 ********************************************/

QStringList LDrawFile::_subFileOrder;
QStringList LDrawFile::_subFileOrderNoUnoff;
QStringList LDrawFile::_displayModelList;
QStringList LDrawFile::_includeFileOrder;
QStringList LDrawFile::_buildModList;
QStringList LDrawFile::_loadedItems;
QStringList LDrawFile::_processedSubfiles;
QString LDrawFile::_file           = "";
QString LDrawFile::_description    = PUBLISH_DESCRIPTION_DEFAULT;
QString LDrawFile::_name           = "";
QString LDrawFile::_author         = VER_PRODUCTNAME_STR;
QString LDrawFile::_category       = "";
QString LDrawFile::_modelFile      = "";
int     LDrawFile::_partCount      = 0;
int     LDrawFile::_displayModelPartCount = 0;
int     LDrawFile::_savedLines     = 0;
int     LDrawFile::_uniquePartCount= 0;
int     LDrawFile::_helperPartCount= 0;
int     LDrawFile::_loadIssues     = 0;
qint64  LDrawFile::_elapsed        = 0;
bool    LDrawFile::_lpubFadeHighlight = false;
bool    LDrawFile::_currFileIsUTF8 = false;
bool    LDrawFile::_loadAborted    = false;
bool    LDrawFile::_loadBuildMods  = false;
bool    LDrawFile::_buildModDetected = false;
bool    LDrawFile::_loadUnofficialParts = true;
bool    LDrawFile::_hasUnofficialParts = false;
bool    LDrawFile::_helperPartsNotInArchive = false;
bool    LDrawFile::_lsynthPartsNotInArchive = false;

LDrawSubFile::LDrawSubFile(
  const QStringList &contents,
  QDateTime         &datetime,
  int                unofficialPart,
  bool               displayModel,
  bool               generated,
  bool               includeFile,
  bool               dataFile,
  const QString     &subFilePath,
  const QString     &description)
{
  _contents << contents;
  _subFilePath = subFilePath;
  _description = description;
  _datetime = datetime;
  _modified = false;
  _numSteps = 0;
  _buildMods = 0;
  _instances = 0;
  _mirrorInstances = 0;
  _rendered = false;
  _mirrorRendered = false;
  _changedSinceLastWrite = true;
  _unofficialPart = unofficialPart;
  _displayModel = displayModel;
  _generated = generated;
  _includeFile = includeFile;
  _dataFile = dataFile;
  _startPageNumber = 0;
  _lineTypeIndexes.clear();
  _subFileIndexes.clear();
  _smiContents.clear();
  _prevStepPosition = { 0,0,0 };
}

/* Only used to store fade or highlight content */

ConfiguredSubFile::ConfiguredSubFile(
  const QStringList &contents,
  const QString     &subFilePath)
{
    _contents << contents;
    _subFilePath = subFilePath;
}

/* initialize new Build Mod */
BuildMod::BuildMod(const QVector<int> &modAttributes,
                   int                stepIndex)
{
    _modStepIndex = stepIndex;
    _modAttributes << modAttributes;
    _modActions.insert(stepIndex, BuildModApplyRc);
}

/* initialize new Build Mod Step */
BuildModStep::BuildModStep(const int      buildModStepIndex,
                           const int      buildModAction,
                           const QString &buildModKey)
{
    _buildModStepIndex = buildModStepIndex;
    _buildModAction = buildModAction;
    _buildModKey = buildModKey;
}

/* initialize viewer step*/
ViewerStep::ViewerStep(const QStringList &stepKey,
                       const QStringList &rotatedViewerContents,
                       const QStringList &rotatedContents,
                       const QStringList &unrotatedContents,
                       const QString     &filePath,
                       const QString     &imagePath,
                       const QString     &csiKey,
                       bool               multiStep,
                       bool               calledOut,
                       int                viewType)
{
    _rotatedViewerContents << rotatedViewerContents;
    _rotatedContents       << rotatedContents;
    _unrotatedContents     << unrotatedContents;
    _partCount = 0;
    _filePath  = filePath;
    _imagePath = imagePath;
    _csiKey    = csiKey;
    _modified  = false;
    _multiStep = multiStep;
    _calledOut = calledOut;
    _viewType  = viewType;
    _hasBuildModAction = false;
    int modelIndex = -1, lineNum = 0,stepNum = 0;
    bool ok[3];
    QString key, errors;
    if (stepKey.size() == 3) {
        if (viewType == Options::PLI) {
            // Parts do not have modelIndex or lineNumber
            stepNum = stepKey.at(BM_STEP_NUM_KEY).toInt(&ok[0]);
            if (!ok[0])
                errors.append(QObject::tr(" step number"));
        } else {
            modelIndex = stepKey.at(BM_STEP_MODEL_KEY).toInt(&ok[0]);
            lineNum    = stepKey.at(BM_STEP_LINE_KEY) .toInt(&ok[1]);
            stepNum    = stepKey.at(BM_STEP_NUM_KEY)  .toInt(&ok[2]);
            if (!ok[0])
                errors.append(QObject::tr(" model index"));
            if (!ok[1])
                errors.append(QObject::tr(" line numbr"));
            if (!ok[2])
                errors.append(QObject::tr(" step number"));
        }
        if (!errors.isEmpty())
            key = stepKey.join(";");
    } else {
        key.append(stepKey.join(";"));
        errors.append(QObject::tr("missing attributes"));
    }
    if (errors.isEmpty()) {
        _stepKey = { modelIndex, lineNum, stepNum };
    } else {
        emit gui->messageSig(LOG_ERROR, QObject::tr("Viewer step has invaid key (%1):%2").arg(key, errors));
        _stepKey = { -1, 0, 0 };
    }

}

void LDrawFile::empty()
{
  _subFiles.clear();
  _configuredSubFiles.clear();
  _subFileOrder.clear();
  _subFileOrderNoUnoff.clear();
  _viewerSteps.clear();
  _buildMods.clear();
  _buildModSteps.clear();
  _buildModStepIndexes.clear();
  _buildModRendered.clear();
  _buildModList.clear();
  _includeFileOrder.clear();
  _displayModelList.clear();
  _missingItems.clear();
  _loadedItems.clear();
  _processedSubfiles.clear();
  _name.clear();
  _author.clear();
  _file.clear();
  _modelFile.clear();
  _helperPartsNotInArchive = false;
  _lsynthPartsNotInArchive = false;
  _mpd                   = false;
  _loadAborted           = false;
  _loadBuildMods         = false;
  _buildModDetected      = false;
  _loadUnofficialParts   = true;
  _hasUnofficialParts    = false;
  _lpubFadeHighlight     = false;
  _loadIssues            =  0;
  _elapsed               =  0;
  _partCount             =  0;
  _displayModelPartCount =  0;
  _uniquePartCount       =  0;
  _helperPartCount       =  0;
  _buildModNextStepIndex = -1;
  _buildModPrevStepIndex =  0;
}

void LDrawFile::normalizeHeader(const QString &subfileName, const QString &fileName, int missing)
{
  QString const missingName = QString("Model-%1-%2").arg(randomFour()).arg(subfileName);
  QString const nameLine = QString("0 Name: %1").arg(missingName);
  QString const authorLine = QString("0 Author: %1").arg(Preferences::defaultAuthor);
  QString line, header;
  int typeLineIndx = 0, lineIndx = 0;
  MissingHeaderType headerMissing = MissingHeaderType(missing);
  if (headerMissing) {
    lineIndx = headerMissing == AuthorMissing ? hdrNameLine + 1 : hdrDescLine + 1;
    line = readLine(subfileName, lineIndx);
    bool update = line.contains(_fileRegExp[NAK_RX]) ||
                  line.contains(_fileRegExp[AUK_RX]);
    switch (headerMissing)
    {
      case NameMissing:
        update ? replaceLine(subfileName, lineIndx, nameLine) :
                 insertLine(subfileName, lineIndx, nameLine);
        _name = missingName;
        header = QObject::tr("header 'Name: %1' was added by %2")
                             .arg(missingName, VER_PRODUCTNAME_STR);
        hdrNameNotFound = false;
        hdrNameLine = lineIndx;
        break;
      case AuthorMissing:
        update ? replaceLine(subfileName, lineIndx, authorLine) :
                 insertLine(subfileName, lineIndx, authorLine);
        header = QObject::tr("header 'Author:%1' was added by %2")
                             .arg(Preferences::defaultAuthor, VER_PRODUCTNAME_STR);
        hdrAuthorNotFound = false;
        hdrAuthorLine = lineIndx;
        break;
      case BothMissing:
        update ? replaceLine(subfileName, lineIndx, authorLine) :
                 insertLine(subfileName, lineIndx, authorLine);
        update ? replaceLine(subfileName, lineIndx, nameLine) :
                 insertLine(subfileName, lineIndx, nameLine);
        header = QObject::tr("headers 'Name: %1 and Author: %2' were added by %3")
                             .arg(missingName, Preferences::defaultAuthor, VER_PRODUCTNAME_STR);
        hdrNameNotFound = hdrAuthorNotFound = false;
        hdrNameLine = lineIndx;
        hdrAuthorLine = lineIndx + 1;
        break;
      default:
        break;
    }
    const QString message = QObject::tr("%1 %2 '%3' %4 (file: %3, line: %5).")
                                        .arg(_mpd ? "MPD" : "LDR", fileType(),
                                             subfileName, header).arg(lineIndx + 1);
    const QString statusEntry = QString("%1|%2|%3").arg(BAD_DATA_LOAD_MSG).arg(fileName, message);
    loadStatusEntry(BAD_DATA_LOAD_MSG, statusEntry, fileName, message);
  } else {
    // This block is called by Gui::writeGeneratedColorPartsToTemp() exclusively.
    // The expectation is line index 0 of the generated file will be a type 1-5 part line.
    for (; typeLineIndx < size(subfileName); typeLineIndx++) {
      line = readLine(subfileName, typeLineIndx);
      if (line.contains(_fileRegExp[EOH_RX]) && typeLineIndx == 0) {
        insertLine(subfileName, typeLineIndx, authorLine);
        insertLine(subfileName, typeLineIndx, nameLine);
        break;
      }
    }
  }
}

/* Add a new subFile */

void LDrawFile::insert(const QString &mcFileName,
                      QStringList    &contents,
                      QDateTime      &datetime,
                      int             unofficialPart,
                      bool            displayModel,
                      bool            generated,
                      bool            includeFile,
                      bool            dataFile,
                      const QString  &subFilePath,
                      const QString  &description)
{
  QString    fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    _subFiles.erase(i);
  }
  const QString modelDesc = description.isEmpty() ? QFileInfo(mcFileName).baseName() : description;
  LDrawSubFile subFile(
      contents,
      datetime,
      unofficialPart,
      displayModel,
      generated,
      includeFile,
      dataFile,
      subFilePath,
      modelDesc);
  _subFiles.insert(fileName,subFile);
  if (includeFile) {
    _includeFileOrder << fileName;
  } else {
    _subFileOrder << mcFileName;
    if (displayModel)
      _displayModelList << mcFileName;
    if (unofficialPart == UNOFFICIAL_SUBMODEL)
      _subFileOrderNoUnoff << mcFileName;
    if (unofficialPart > UNOFFICIAL_UNKNOWN)
      _hasUnofficialParts = true;
  }
}

/* Add a new modSubFile - Only used to insert fade or highlight content */

void LDrawFile::insertConfiguredSubFile(const QString &mcFileName,
                                        QStringList    &contents,
                                        const QString  &subFilePath)
{
  QString    fileName = mcFileName.toLower();
  QMap<QString, ConfiguredSubFile>::iterator i = _configuredSubFiles.find(fileName);

  if (i != _configuredSubFiles.end()) {
    _configuredSubFiles.erase(i);
  }
  ConfiguredSubFile subFile(contents,subFilePath);
  _configuredSubFiles.insert(fileName,subFile);
}

/* return the number of lines in the loaded model file */

int LDrawFile::loadedLines()
{
  int lines = 0;
  for (int i = 0; i < _subFileOrder.size(); i++) {
    QString subFileName = _subFileOrder.at(i).toLower();
    QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(subFileName);
    if (f != _subFiles.end() && ! f.value()._generated) {
      lines += f.value()._contents.size();
    }
  }
  return lines;
}

/* return the number of steps in the loaded model file */

int LDrawFile::loadedSteps()
{
  int steps = 0;
  for (int i = 0; i < _subFileOrder.size(); i++) {
    QString subFileName = _subFileOrder.at(i).toLower();
    QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(subFileName);
    if (f != _subFiles.end() && ! f.value()._generated) {
      steps += f.value()._numSteps;
    }
  }
  return steps;
}

/* return the number of lines in the subfile */

int LDrawFile::size(const QString &mcFileName)
{
  QString fileName = mcFileName.toLower();

  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    return i.value()._contents.size();
  }
  return 0;
}

/* Only used to return fade or highlight content size */

int LDrawFile::configuredSubFileSize(const QString &mcFileName)
{
  QString fileName = mcFileName.toLower();

  QMap<QString, ConfiguredSubFile>::iterator i = _configuredSubFiles.find(fileName);

  if (i != _configuredSubFiles.end()) {
    return i.value()._contents.size();
  }
  return 0;
}

int LDrawFile::getSupportPartsNotInArchive()
{
    if (_helperPartsNotInArchive && _lsynthPartsNotInArchive)
        return ExcludedParts::EP_HELPER_AND_LSYNTH;
    else if (_helperPartsNotInArchive)
        return ExcludedParts::EP_HELPER;
    else if (_lsynthPartsNotInArchive)
        return ExcludedParts::EP_LSYNTH;
    else
        return ExcludedParts::EP_STANDARD;
}

bool LDrawFile::isMpd()
{
  return _mpd;
}

QString LDrawFile::description(const QString &mcFileName)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    return i.value()._description;
  }
  return QFileInfo(mcFileName).completeBaseName();
}

int LDrawFile::isUnofficialPart(const QString &name)
{
  QString fileName = name.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    return i.value()._unofficialPart;
  }
  return UNOFFICIAL_UNKNOWN;
}

bool LDrawFile::isIncludeFile(const QString &mcFileName)
{
  return _includeFileOrder.contains(mcFileName,Qt::CaseInsensitive);
}

bool LDrawFile::isDisplayModel(const QString &mcFileName)
{
  return _displayModelList.contains(mcFileName,Qt::CaseInsensitive);
}

/* return the name of the top level file */

QString LDrawFile::topLevelFile()
{
  if (_subFileOrder.size()) {
    return _subFileOrder.at(0).toLower();
  } else {
    return _emptyString;
  }
}

int LDrawFile::fileOrderIndex(const QString &file)
{
  for (int i = 0; i < _subFileOrder.size(); i++) {
    if (_subFileOrder.at(i).toLower() == file.toLower()) {
      return i;
    }
  }
  return -1;
}

/* return the number of steps within the file */

int LDrawFile::numSteps(const QString &mcFileName)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    return i.value()._numSteps;
  }
  return 0;
}

/* return the model start page number value */

int LDrawFile::getModelStartPageNumber(const QString &mcFileName)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    return i.value()._startPageNumber;
  }
  return 0;
}

QDateTime LDrawFile::lastModified(const QString &mcFileName)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    return i.value()._datetime;
  }
  return QDateTime();
}

bool LDrawFile::contains(const QString &file, bool searchAll)
{
  if (searchAll) {
    for (int i = 0; i < _subFileOrder.size(); i++) {
      if (_subFileOrder.at(i).toLower() == file.toLower()) {
        return true;
      }
    }
    for (int i = 0; i < _includeFileOrder.size(); i++) {
      if (_includeFileOrder.at(i).toLower() == file.toLower()) {
        return true;
      }
    }
  } else {
    for (int i = 0; i < _subFileOrderNoUnoff.size(); i++) {
      if (_subFileOrderNoUnoff[i].toLower() == file.toLower()) {
        return true;
      }
    }
  }
  return false;
}

bool LDrawFile::isSubmodel(const QString &file)
{
  QString fileName = file.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
      return i.value()._unofficialPart == UNOFFICIAL_SUBMODEL && !i.value()._generated;
      //return ! i.value()._generated; // added on revision 368 - to generate csiSubModels for 3D render
  }
  return false;
}

bool LDrawFile::isSingleSubfileLine(const QString &line)
{
  QStringList tokens;
  split(line, tokens);
  if (tokens.size() == 15 && tokens[0] == "1")
    return isSubmodel(tokens[tokens.size()-1]);
  return false;
}

bool LDrawFile::modified()
{
  bool modified = false;
  const QList _sfKeys = _subFiles.keys();
  for (const QString &key : _sfKeys)
    modified |= _subFiles[key]._modified;
  return modified;
}

bool LDrawFile::modified(const QString &mcFileName, bool reset)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    const bool modified = i.value()._modified;
    if (reset) {
      i.value()._modified = false;
#ifdef QT_DEBUG_MODE
      if (reset && modified)
          emit gui->messageSig(LOG_DEBUG, QString("Reset Submodel: %1, Modified: [No].").arg(mcFileName));
#endif
    }
    return modified;
  } else {
    return false;
  }
}

bool LDrawFile::modified(const QStringList &parsedStack, bool reset)
{
  bool result = false;
  for (const QString &fileName : parsedStack) {
    LDrawSubFile &subFile = _subFiles[fileName];
    result |= subFile._modified;
    if (reset)
       subFile._modified = false;
  }
  return result;
}

bool LDrawFile::modified(const QVector<int> &parsedIndexes, bool reset)
{
#ifdef QT_DEBUG_MODE
    int count = 0;
    QString modifiedSubmodels;
#endif
    bool result = false;
    for (const int index : parsedIndexes) {
        const QString &fileName = getSubmodelName(index);
        LDrawSubFile &subFile = _subFiles[fileName];
        const bool modified = subFile._modified;
        result |= modified;
#ifdef QT_DEBUG_MODE
        if (modified) {
            modifiedSubmodels.append(QString("%1 (%2), ").arg(fileName).arg(index));
            count++;
        }
#endif
        if (reset)
           subFile._modified = false;
    }

#ifdef QT_DEBUG_MODE
    if (result) {
        modifiedSubmodels = modifiedSubmodels.trimmed();
        modifiedSubmodels.chop(1);
        emit gui->messageSig(LOG_TRACE, QString("LDrawFile Modified %1 %2 %3")
                                                .arg(count).arg(count == 1 ? "Submodel:" : "Submodels:")
                                                .arg(modifiedSubmodels));
    }
#endif
    return result;
}

void LDrawFile::setModified(const QString &mcFileName, bool modified)
{
  const QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    i.value()._modified = modified;
    i.value()._changedSinceLastWrite = modified;
  }
}

QStringList LDrawFile::contents(const QString &mcFileName)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    return i.value()._contents;
  } else {
    return _emptyList;
  }
}

void LDrawFile::setContents(const QString &mcFileName, const QStringList &contents)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    i.value()._modified = true;
    //i.value()._datetime = QDateTime::currentDateTime();
    i.value()._contents = contents;
    i.value()._changedSinceLastWrite = true;
  }
}

void LDrawFile::setSmiContent(const QString &mcFileName, const QStringList &smiContents)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    i.value()._smiContents = smiContents;
  }
}

QStringList LDrawFile::smiContents(const QString &mcFileName)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    return i.value()._smiContents;
  } else {
    return _emptyList;
  }
}

void LDrawFile::setDisplayModel(
    const QString     &mcFileName)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    i.value()._displayModel = true;
    if (!_displayModelList.contains(mcFileName))
       _displayModelList << mcFileName;
  }
}

void LDrawFile::setUnofficialPart(const QString &mcFileName, const int type)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    i.value()._unofficialPart = type;
  }
}

void LDrawFile::setSubFilePath(
                 const QString     &mcFileName,
                 const QString &subFilePath)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    i.value()._subFilePath = subFilePath;
  }
}

QStringList LDrawFile::getSubModels()
{
    QStringList subModelList;
    for (int i = 0; i < _subFileOrder.size(); i++) {
      QString modelName = _subFileOrder.at(i).toLower();
      QMap<QString, LDrawSubFile>::iterator it = _subFiles.find(modelName);
      if (it->_unofficialPart == UNOFFICIAL_SUBMODEL && !it->_generated) {
          subModelList << _subFileOrder.at(i);
      }
    }
    return subModelList;
}

QString LDrawFile::getSubFilePath(const QString &mcFileName)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(fileName);
  if (f != _subFiles.end()) {
    return f.value()._subFilePath;
  }
  return QString();
}

QStringList LDrawFile::getSubFilePaths()
{
  QStringList subFilesPaths;
  for (int i = 0; i < _subFileOrder.size(); i++) {
    QString filePath = getSubFilePath(_subFileOrder.at(i));
    if (!filePath.isEmpty())
        subFilesPaths << filePath;
  }
  if (_includeFileOrder.size()) {
      for (int i = 0; i < _includeFileOrder.size(); i++) {
        QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(_includeFileOrder[i]);
        if (f != _subFiles.end()) {
            if (!f.value()._subFilePath.isEmpty()) {
                subFilesPaths << f.value()._subFilePath;
        }
      }
    }
  }
  return subFilesPaths;
}

void LDrawFile::setModelStartPageNumber(
        const QString &mcFileName,
        const int     &startPageNumber)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end())
    i.value()._startPageNumber = startPageNumber;
}

/* return the last fade position value */

int LDrawFile::getPrevStepPosition(const QString &mcFileName,
                                   const int     &mcLineNumber,
                                   const int     &mcStepNumber)
{
#ifdef QT_DEBUG_MODE
  bool lastPosition = false;
#endif
  int position = 0;
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
      if (mcStepNumber == i.value()._prevStepPosition.at(PS_STEP_NUM)) {
          position = i.value()._prevStepPosition.at(PS_LAST_POS);
#ifdef QT_DEBUG_MODE
          lastPosition = true;
#endif
      } else {
          position = i.value()._prevStepPosition.at(PS_POS);
      }
  }

  if (position && Preferences::buildModEnabled) {
      QVector<int> indexKey = { getSubmodelIndex(mcFileName), mcLineNumber };
      int stepIndex = _buildModStepIndexes.indexOf(indexKey);
      if (stepIndex == BM_INVALID_INDEX)
          emit gui->messageSig(LOG_WARNING, QString("GetPrevStepPosition (StepNumber %3)\n"
                                                    " - Could not find PrevStepPosition index for fileName: %1 and lineNumber: %2")
                                                    .arg(mcFileName).arg(mcLineNumber).arg(mcStepNumber));
#ifdef QT_DEBUG_MODE
      else
          emit gui->messageSig(LOG_DEBUG, QString("GetPrevStepPosition (StepNumber %6)\n"
                                                  " - %1, StepIndex: %2, SubmodelIndex: %3,"
                                                  " LineNumber: %4, ModelName: %5, %7")
                                                  .arg(QString("StepPosition (%1): %2")
                                                  .arg(lastPosition ? "Last" : "Prev")
                                                  .arg(position, 3, 10, QChar('0')))          // position
                                                  .arg(stepIndex, 3, 10, QChar('0'))          // stepIndex
                                                  .arg(indexKey.at(0), 3, 10, QChar('0'))     // modelIndex
                                                  .arg(indexKey.at(1), 3, 10, QChar('0'))     // lineNumber
                                                  .arg(getSubmodelName(indexKey.at(0),false)) // modelName
                                                  .arg(mcStepNumber, 3, 10, QChar('0')));     // stepNumber
#endif
  }
  return position;
}

void LDrawFile::setPrevStepPosition(
        const QString &mcFileName,
        const int     &mcStepNumber,
        const int     &prevStepPosition)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    const int lastStepPosition = i.value()._prevStepPosition.size() ? i.value()._prevStepPosition.at(PS_POS) : PS_POS ;
    if (lastStepPosition != prevStepPosition) {
        QVector<int> stepPositions = { prevStepPosition, lastStepPosition, mcStepNumber };
        i.value()._prevStepPosition = stepPositions;
    }
#ifdef QT_DEBUG_MODE
    emit gui->messageSig(LOG_DEBUG, QString("SetPrevStepPosition (StepNumber %3)\n"
                                            " - PrevStepPosition: %1, LastStepPosition: %2,"
                                            " StepNumber: %3, SubmodelIndex: %4, ModelName: %5, Inserted: %6")
                                            .arg(prevStepPosition, 3, 10, QChar('0'))                  // prevStepPosition
                                            .arg(lastStepPosition , 3, 10, QChar('0'))                 // lastStepPosition
                                            .arg(mcStepNumber)                                         // stepNumber
                                            .arg(getSubmodelIndex(fileName), 3, 10, QChar('0'))        // modelIndex
                                            .arg(fileName)                                             // modelName
                                            .arg(lastStepPosition != prevStepPosition ? "YES" : "NO"));// added
#endif
  }
}

/* set all previous step positions to 0 */

void LDrawFile::clearPrevStepPositions()
{
  const QList _sfKeys = _subFiles.keys();
  for (const QString &key : _sfKeys) {
    _subFiles[key]._prevStepPosition.clear();
  }
}

/* Check the pngFile lastModified date against its submodel file */
bool LDrawFile::older(const QString &fileName,
                      const QDateTime &lastModified) {
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    QDateTime fileDatetime = i.value()._datetime;
    if (fileDatetime > lastModified) {
      return false;
    }
  }
  return true;
}

bool LDrawFile::older(const QStringList &parsedStack,
                      const QDateTime &lastModified)
{
  for (const QString &fileName : parsedStack) {
    QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
    if (i != _subFiles.end()) {
      QDateTime fileDatetime = i.value()._datetime;
      if (fileDatetime > lastModified) {
        return false;
      }
    }
  }
  return true;
}

int LDrawFile::subFileOrderSize() {
    if (_loadUnofficialParts)
        return _subFileOrder.size();
    else
        return _subFileOrderNoUnoff.size();
}

QStringList& LDrawFile::subFileOrder() {
    if (_loadUnofficialParts)
        return _subFileOrder;
    else
        return _subFileOrderNoUnoff;
}

QStringList& LDrawFile::includeFileList() {
    return _includeFileOrder;
}

QStringList& LDrawFile::displayModelList() {
    return _displayModelList;
}

QString LDrawFile::getSubmodelName(int submodelIndx, bool lower)
{
    if (submodelIndx > BM_INVALID_INDEX && submodelIndx < _subFileOrder.size()) {
        if (lower)
            return _subFileOrder.at(submodelIndx).toLower();
        else
            return _subFileOrder.at(submodelIndx);
    }
    return QString();
}

int LDrawFile::getSubmodelIndex(const QString &mcFileName)
{
    return _subFileOrder.indexOf(QRegularExpression(mcFileName, QRegularExpression::CaseInsensitiveOption));
}

/* marshall subFile 'child' indexes */

QVector<int> LDrawFile::getSubmodelIndexes(const QString &fileName)
{
    QVector<int> indexes, parsedIndexes;
    const QString mcFileName = fileName.toLower();
    const int mcModelIndex = getSubmodelIndex(mcFileName);

    if (mcFileName == topLevelFile()) {
        QMap<QString, LDrawSubFile>::const_iterator it = _subFiles.constBegin();
        while (it != _subFiles.constEnd()) {
            if (it.value()._unofficialPart == UNOFFICIAL_SUBMODEL && !it.value()._generated)
                parsedIndexes << getSubmodelIndex(it.key());
            ++it;
        }

        // remove top level file index
        parsedIndexes.takeFirst();

        return parsedIndexes;
    }

    auto getIndexes = [this, &parsedIndexes] (const int index, QVector<int> &indexes)
    {
        const QString &mcFileName = getSubmodelName(index);
        QMap<QString, LDrawSubFile>::iterator it = _subFiles.find(mcFileName);
        if (it != _subFiles.end()) {
            if (it.value()._unofficialPart == UNOFFICIAL_SUBMODEL && !it.value()._generated) {
                const QVector<int> _sfIndexes = it.value()._subFileIndexes;
                for(int i : _sfIndexes) {
                    if (!indexes.contains(i) && !parsedIndexes.contains(i))
                        indexes << i;
                }
            }
        }
    };

    getIndexes(mcModelIndex, indexes);

    while (!indexes.isEmpty()) {
        const int modelIndex = indexes.takeFirst();
        if (!parsedIndexes.contains(modelIndex))
            getIndexes(modelIndex, indexes);
        parsedIndexes << modelIndex;
    }

    return parsedIndexes;
}

// The Line Type Index is the position of the type 1 line in the parsed subfile written to temp
// This function returns the position (Relative Type Index) of the type 1 line in the subfile content
int LDrawFile::getLineTypeRelativeIndex(int submodelIndx, int lineTypeIndx) {
    QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(getSubmodelName(submodelIndx));
    if (f != _subFiles.end() && f.value()._lineTypeIndexes.size() > lineTypeIndx) {
        return f.value()._lineTypeIndexes.at(lineTypeIndx);
    }
    return -1;
}

// The Relative Type Index is the position of the type 1 line in the subfile content
// This function inserts the Relative Type Index at the position (Line Type Index)
// of the type 1 line in the parsed subfile written to temp
void LDrawFile::setLineTypeRelativeIndex(int submodelIndx, int relativeTypeIndx) {
    QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(getSubmodelName(submodelIndx));
    if (f != _subFiles.end()) {
        f.value()._lineTypeIndexes.append(relativeTypeIndx);
    }
}

// This function sets the Line Type Indexes vector
void LDrawFile::setLineTypeRelativeIndexes(int submodelIndx, QVector<int> &relativeTypeIndxes) {
    QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(getSubmodelName(submodelIndx));
    if (f != _subFiles.end()) {
        f.value()._lineTypeIndexes = relativeTypeIndxes;
    }
}

// This function returns the submodel Line Type Index
int LDrawFile::getLineTypeIndex(int submodelIndx, int relativeTypeIndx) {
    QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(getSubmodelName(submodelIndx));
    if (f != _subFiles.end() && f.value()._lineTypeIndexes.size()) {
        for (int i = 0; i < f.value()._lineTypeIndexes.size(); ++i)
            if (f.value()._lineTypeIndexes.at(i) == relativeTypeIndx)
                return i;
    }
    return -1;
}

// This function returns a pointer to the submodel Line Type Index vector
QVector<int> *LDrawFile::getLineTypeRelativeIndexes(int submodelIndx) {

    QString fileName = getSubmodelName(submodelIndx);
    QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(fileName);
    if (f != _subFiles.end() && f.value()._lineTypeIndexes.size()) {
        return &f.value()._lineTypeIndexes;
    }
    return nullptr;
}

// This function returns the number of indexes (type 1 parts) specified for the specified submodel
int LDrawFile::getLineTypeRelativeIndexCount(int submodelIndx) {
    QString fileName = getSubmodelName(submodelIndx);
    QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(fileName);
    if (f != _subFiles.end())
        return f.value()._lineTypeIndexes.size();
    return 0;
}

// This function resets the Line Type Indexes vector
void LDrawFile::resetLineTypeRelativeIndex(const QString &mcFileName) {
    QString fileName = mcFileName.toLower();
    QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(fileName);
    if (f != _subFiles.end()) {
        f.value()._lineTypeIndexes.clear();
    }
}

QString LDrawFile::readLine(const QString &mcFileName, int lineNumber)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
      if (lineNumber < i.value()._contents.size())
          return i.value()._contents[lineNumber];
  }
  return QString();
}

void LDrawFile::insertLine(const QString &mcFileName, int lineNumber, const QString &line)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    i.value()._contents.insert(lineNumber,line);
    i.value()._modified = true;
 //   i.value()._datetime = QDateTime::currentDateTime();
    i.value()._changedSinceLastWrite = true;
  }
}

void LDrawFile::replaceLine(const QString &mcFileName, int lineNumber, const QString &line)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    i.value()._contents[lineNumber] = line;
    i.value()._modified = true;
//    i.value()._datetime = QDateTime::currentDateTime();
    i.value()._changedSinceLastWrite = true;
  }
}

void LDrawFile::deleteLine(const QString &mcFileName, int lineNumber)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    i.value()._contents.removeAt(lineNumber);
    i.value()._modified = true;
//    i.value()._datetime = QDateTime::currentDateTime();
    i.value()._changedSinceLastWrite = true;
  }
}

void LDrawFile::changeContents(
                    const QString &mcFileName,
                          int      position,
                          int      charsRemoved,
                    const QString &charsAdded)
{
  QString fileName = mcFileName.toLower();
  if (charsRemoved || charsAdded.size()) {
    QString all = contents(fileName).join("\n");
    all.remove(position,charsRemoved);
    all.insert(position,charsAdded);
    setContents(fileName,all.split("\n"));
  }
}

/*  Only used by SubMeta::parse(...) to read fade or highlight content */

QString LDrawFile::readConfiguredLine(const QString &mcFileName, int lineNumber)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, ConfiguredSubFile>::iterator i = _configuredSubFiles.find(fileName);

  if (i != _configuredSubFiles.end()) {
      if (lineNumber < i.value()._contents.size())
          return i.value()._contents[lineNumber];
  }
  return QString();
}

void LDrawFile::unrendered()
{
  const QList _sfKeys = _subFiles.keys();
  for (const QString &key : _sfKeys) {
    _subFiles[key]._rendered = false;
    _subFiles[key]._mirrorRendered = false;
    _subFiles[key]._renderedKeys.clear();
    _subFiles[key]._mirrorRenderedKeys.clear();
  }
}

void LDrawFile::setRendered(
    const QString &mcFileName,
    const QString &modelColour,
    const QString &renderParentModel,
    bool           mirrored,
    int            renderStepNumber,
    int            countInstance,
    bool           countPage)
{
  CountInstanceEnc howToCount = static_cast<CountInstanceEnc>(countInstance);
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(mcFileName.toLower());
  if (i != _subFiles.end()) {
    QString key;
    if (howToCount > CountTrue && howToCount < CountAtTop) {
      key =
        howToCount == CountAtStep
                  ? QString("%1;%2").arg(renderParentModel).arg(renderStepNumber)
                  : renderParentModel;
    }

    if (!modelColour.isEmpty())
      key.prepend(QString("%1%2").arg(modelColour).arg(key.isEmpty() ? "" : ";"));

    if (countPage)
      key.prepend(QString("%1%2").arg(COUNT_PAGE_PREFIX).arg(key.isEmpty() ? "" : ";"));

    if (mirrored) {
      i.value()._mirrorRendered = true;
      if (!key.isEmpty() && !i.value()._mirrorRenderedKeys.contains(key)) {
        i.value()._mirrorRenderedKeys.append(key);
      }
    } else {
      i.value()._rendered = true;
      if (!key.isEmpty() && !i.value()._renderedKeys.contains(key)) {
        i.value()._renderedKeys.append(key);
      }
    }
/*
#ifdef QT_DEBUG_MODE
    qDebug() << "SET RENDERED:\n"
             << "COUNTPAGE:"         << "[" << qPrintable(countPage ? "Yes" : "No") << "]\n"
             << "KEY:"               << "[" << qPrintable(key.isEmpty() ? "_Empty_" : key) << "]\n"
             << "FileName:"          << "[" << qPrintable(mcFileName) << "]\n"
             << "RenderParentModel:" << "[" << qPrintable(renderParentModel.isEmpty() ? "_Empty_" : renderParentModel)  << "]\n"
             << "ModelColour:"       << "[" << qPrintable(modelColour.isEmpty() ? "_Empty_" : modelColour) << "]\n"
             << "RenderStepNumber:"  << "[" << qPrintable(QString::number(renderStepNumber)) << "]\n"
             << "HowCounted:"        << "[" << qPrintable(howToCount == CountAtStep  ? "CountAtStep"  :
                                                          howToCount == CountAtTop   ? "CountAtTop"   :
                                                          howToCount == CountAtModel ? "CountAtModel" :
                                                          howToCount == CountFalse   ? "CountFalse" :
                                                                        QString::number(howToCount)) << "]\n"
                ;
#endif
//*/
  }
}

bool LDrawFile::rendered(
    const QString &mcFileName,
    const QString &modelColour,
    const QString &renderParentModel,
    bool           mirrored,
    int            renderStepNumber,
    int            countInstance,
    bool           countPage)
{
  CountInstanceEnc howToCount = static_cast<CountInstanceEnc>(countInstance);
  QString key, altKey;
  bool rendered = false, haveKey = false;
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(mcFileName.toLower());
  if (i != _subFiles.end()) {
    if (howToCount > CountTrue && howToCount < CountAtTop) {
      key =
          howToCount == CountAtStep
              ? QString("%1;%2").arg(renderParentModel).arg(renderStepNumber)
              : renderParentModel;
    }

    if (!modelColour.isEmpty())
      key.prepend(QString("%1%2").arg(modelColour).arg(key.isEmpty() ? "" : ";"));

    altKey = key;

    if (countPage)
      key.prepend(QString("%1%2").arg(COUNT_PAGE_PREFIX).arg(key.isEmpty() ? "" : ";"));

    auto getHaveKey = [&] (const QStringList &renderedKeys)
    {
      // check the countPage key COUNT_PAGE_PREFIX if present.
      bool keyFound = key.isEmpty() || (countPage && key == COUNT_PAGE_PREFIX)
                          ? howToCount == CountAtTop
                                ? true
                                : false
                          : renderedKeys.contains(key);
      // if no key found using COUNT_PAGE_PREFIX, attempt to check the findPage key without COUNT_PAGE_PREFIX
      if (!keyFound && countPage) {
          keyFound = altKey.isEmpty()
                         ? howToCount == CountAtTop
                               ? true
                               : false
                         : renderedKeys.contains(altKey);
      }
      return keyFound;
    };

    if (mirrored) {
      haveKey = getHaveKey(i.value()._mirrorRenderedKeys);
      rendered = i.value()._mirrorRendered;
    } else {
      haveKey = getHaveKey(i.value()._renderedKeys);
      rendered = i.value()._rendered;
    }

    rendered &= haveKey;
  }
/*
#ifdef QT_DEBUG_MODE
  qDebug() << "RENDERED:"          << "[" << qPrintable(rendered  ? "YES" : "NO") << "]\n"
           << "COUNTPAGE:"         << "[" << qPrintable(countPage ? "Yes" : "No") << "]\n"
           << "KEY:"               << "[" << qPrintable(key.isEmpty() ? "_Empty_" : key) << "]\n"
           << "ALTKEY:"            << "[" << qPrintable(altKey.isEmpty() ? "_Empty_" : altKey) << "]\n"
           << "HAVEKEY:"           << "[" << qPrintable(haveKey   ? "Yes" : "No") << "]\n"
           << "FileName:"          << "[" << qPrintable(mcFileName) << "]\n"
           << "RenderParentModel:" << "[" << qPrintable(renderParentModel.isEmpty() ? "_Empty_" : renderParentModel)  << "]\n"
           << "ModelColour:"       << "[" << qPrintable(modelColour.isEmpty() ? "Empty" : modelColour) << "]\n"
           << "RenderStepNumber:"  << "[" << qPrintable(QString::number(renderStepNumber)) << "]\n"
           << "HowCounted:"        << "[" << qPrintable(howToCount == CountAtStep  ? "CountAtStep"  :
                                                        howToCount == CountAtTop   ? "CountAtTop"   :
                                                        howToCount == CountAtModel ? "CountAtModel" :
                                                        howToCount == CountFalse   ? "CountFalse" :
                                                        QString::number(howToCount)) << "]\n"
      ;
#endif
//*/
  return rendered;
}

int LDrawFile::instances(const QString &mcFileName, bool mirrored)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  int instances = 0;

  if (i != _subFiles.end()) {
    if (mirrored) {
      instances = i.value()._mirrorInstances;
    } else {
      instances = i.value()._instances;
    }
  }
  return instances;
}

int LDrawFile::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        emit gui->messageSig(LOG_ERROR, QObject::tr("Cannot read LDraw file: [%1]<br>%2.")
                             .arg(fileName, file.errorString()));
        return 1;
    }
    QByteArray qba(file.readAll());
    file.close();

    QElapsedTimer t; t.start();

    // check file encoding
#if QT_VERSION < QT_VERSION_CHECK(6,0,0) || defined(QT_CORE5COMPAT_LIB)
    QTextCodec::ConverterState state;
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QString utfTest = codec->toUnicode(qba.constData(), qba.size(), &state);
    _currFileIsUTF8 = state.invalidChars == 0;
    utfTest = QString();
#else
    auto utf8Decoder = QStringDecoder(QStringDecoder::Utf8);
    auto data = utf8Decoder(qba);
    _currFileIsUTF8 = !utf8Decoder.hasError();
    Q_UNUSED(data)
#endif

    // get rid of what's there before we load up new stuff

    empty();

    // allow files ldr suffix to allow for MPD
    enum TypeEnc {
        UNKNOWN_FILE,
        MPD_FILE,
        LDR_FILE
    };

    TypeEnc type = UNKNOWN_FILE;

    QFileInfo fileInfo(fileName);

    QTextStream in(&qba);
    while ( ! in.atEnd()) {
        QString line = in.readLine(0);
        if (line.contains(_fileRegExp[SOF_RX]) || line.contains(_fileRegExp[DAT_RX])) {
            emit gui->messageSig(LOG_INFO_STATUS, QObject::tr("File '%1' identified as Multi-Part LDraw System (MPD) Document").arg(fileInfo.fileName()));
            type = MPD_FILE;
            break;
        }
        if (line.contains(_fileRegExp[NAM_RX]) || line.contains(_fileRegExp[LDR_RX])) {
            emit gui->messageSig(LOG_INFO_STATUS, QObject::tr("File '%1' identified as LDraw System (LDR) Document").arg(fileInfo.fileName()));
            type = LDR_FILE;
            break;
        }
    }

    if (type == UNKNOWN_FILE) {
        emit gui->messageSig(LOG_ERROR, QObject::tr("File '%1' is not a valid LDraw (LDR) or Multi-Part LDraw (MPD) System Document.").arg(fileInfo.fileName()));
        return 1;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    topLevelModel      = true;
    updatePartsArchive = false;

    _modelFile = fileInfo.fileName();

    QFuture<void> future = QtConcurrent::run([this, fileInfo, type]() {
        if (type == MPD_FILE)
            loadMPDFile(fileInfo.absoluteFilePath());
        else
            loadLDRFile(fileInfo.absoluteFilePath());
        if (updatePartsArchive) {
            QString const textureDir = QString("%1%2%3").arg(Preferences::lpubDataPath).arg(QDir::separator()).arg(Paths::customTextureDir);
            if (!Preferences::ldSearchDirs.contains(textureDir)) {
                Preferences::ldSearchDirs << textureDir;
                emit gui->messageSig(LOG_INFO, QObject::tr("Added custom textures directory: %1").arg(textureDir));
            }
            PartWorker partWorkerLDSearchDirs;
            partWorkerLDSearchDirs.updateLDSearchDirs(true, true);
        }
    } );
    asynchronous(future);

    _processedSubfiles.clear();

    QApplication::restoreOverrideCursor();

    future = QtConcurrent::run( [this](){ addCustomColorParts(topLevelFile()); } );
    asynchronous(future);

    buildModLevel = 0 /*false*/;

    future = QtConcurrent::run( [this](){ countParts(topLevelFile()); } );
    asynchronous(future);

    _loadedItems.sort(Qt::CaseInsensitive);

    _elapsed = t.elapsed();

    const QString loadMessage = QObject::tr("Loaded LDraw %1 model file '%2'. Unique %3 %4. Model Total %5 %6. %7")
                                            .arg(type == MPD_FILE ? "MPD" : "LDR",
                                                 _modelFile,
                                                 _uniquePartCount == 1 ? QObject::tr("Part") : QObject::tr("Parts"))
                                            .arg(_uniquePartCount)
                                            .arg(_partCount == 1 ? QObject::tr("Part") : QObject::tr("Parts"))
                                            .arg(_partCount)
                                            .arg(gui->elapsedTime(_elapsed));

    if (Preferences::modeGUI) {
        int rc = loadStatus();
        if (rc == 2)
            emit gui->messageSig(LOG_INFO_STATUS, loadMessage);
        else if (rc == 1)
            return 1;
    } else {
        emit gui->messageSig(LOG_INFO, loadMessage);
    }

    return 0;
}

int LDrawFile::loadStatus(bool menuAction)
{
    bool showLoadStatus = false;
    if (!menuAction) {
        ShowLoadMsgType loadIssues = static_cast<ShowLoadMsgType>(_loadIssues);
        switch (Preferences::ldrawFilesLoadMsgs)
        {
        case NEVER_SHOW:
            break;
        case ALWAYS_SHOW:
            showLoadStatus = true;
            break;
        case SHOW_ERROR:
            showLoadStatus = loadIssues == SHOW_ERROR;
            break;
        case SHOW_WARNING:
            showLoadStatus = loadIssues == SHOW_WARNING;
            break;
        case SHOW_MESSAGE:
            showLoadStatus = loadIssues > SHOW_MESSAGE;
            break;
        }
    } else if (Preferences::recountParts) {
        emit lpub->messageSig(LOG_STATUS, QObject::tr("Recounting LDraw parts..."));
        QFuture<void> future = QtConcurrent::run([this]() {
            recountParts();
            _loadedItems.sort(Qt::CaseInsensitive);
        });
        asynchronous(future);
    }

    if (showLoadStatus || menuAction) {
        QString const elapsedTime(gui->elapsedTime(_elapsed));

        LoadStatus loadStatus(
            isMpd(),
            loadedLines(),
            loadedSteps(),
            subFileOrderSize(),
            getPartCount(),
            getUniquePartCount(),
            getHelperPartCount(),
            getDisplayModelPartCount(),
            _modelFile,
            elapsedTime,
            _loadedItems);

        QDialog::DialogCode response = LdrawFilesLoad::showLoadStatus(loadStatus, menuAction);

        if (response == QDialog::Rejected) {
            empty();
            _loadAborted = true;
             return 1;
        }
    }
    else
    {
        return 2;
    }

    return 0;
}

bool LDrawFile::loadIncludeFile(const QString &mcFileName)
{
    QFileInfo fileInfo(mcFileName);

    QFile file(mcFileName);
    if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
        const QString message = QObject::tr("Cannot read include file %1.<br>%2").arg(mcFileName, file.errorString());
        const QString statusEntry = QObject::tr("%1|%2|%3.")
                                        .arg(BAD_INCLUDE_LOAD_MSG).arg(fileInfo.fileName(), QString(message).replace("<br>",". "));
        loadStatusEntry(BAD_INCLUDE_LOAD_MSG, statusEntry, fileInfo.fileName(), message);
        return false;
    }

    emit lpub->messageSig(LOG_TRACE, QObject::tr("Loading include file '%1'...").arg(mcFileName));

    QTextStream in(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    in.setEncoding(_currFileIsUTF8 ? QStringConverter::Utf8 : QStringConverter::System);
#else
    in.setCodec(_currFileIsUTF8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForName("System"));
#endif

    static QRegularExpression subRx("^0\\s+!?(?:LPUB)*\\s?(PLI BEGIN SUB|PART BEGIN IGN|PLI END|PART END)[^\n]*");

    auto isValidLine = [&] (const int lineNumber, const QString &smLine) {
        if (smLine.isEmpty())
            return false;

        switch (smLine.toLatin1()[0]) {
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
            {
                const QString message = QObject::tr("Invalid include file '%1' line %2.<br>"
                                                    "Type 1 to 5 lines are ignored in include file.")
                                                    .arg(fileInfo.fileName()).arg(lineNumber);
                const QString statusEntry = QObject::tr("%1|%2|Invalid include file line %3 (type 1 - 5 ignored).")
                                                        .arg(BAD_INCLUDE_LOAD_MSG).arg(fileInfo.fileName()).arg(lineNumber);
                loadStatusEntry(BAD_INCLUDE_LOAD_MSG, statusEntry, fileInfo.fileName(), message);
            }
            return false;
        case '0':
            if (smLine.contains(subRx))
            {
                const QString message = QObject::tr("Invalid include file '%1' line %2.<br>"
                                                    "Substitute meta commands are ignored in include file.")
                                            .arg(fileInfo.fileName()).arg(lineNumber);
                const QString statusEntry = QObject::tr("%1|%2|Invalid include file line %3 (Substitutes ignored).")
                                                .arg(BAD_INCLUDE_LOAD_MSG).arg(fileInfo.fileName()).arg(lineNumber);
                loadStatusEntry(BAD_INCLUDE_LOAD_MSG, statusEntry, fileInfo.fileName(), message);
                return false;
            }
            return true;
        }
        return false;
    };

    int lineNum = 0;
    QStringList contents, tokens;
    while (! in.atEnd()) {
        lineNum++;
        QString smLine = in.readLine(0).trimmed();
        if (smLine.isEmpty() || isComment(smLine)) {
            contents << smLine.trimmed();
        } else
        if (isValidLine(lineNum, smLine)) {
            split(smLine,tokens);
            if (tokens.size() >= 4) {
                processMetaCommand(tokens);
                contents << smLine.trimmed();
            }
        } else {
            contents << QObject::tr("0 // %1 (Invalid include file line)").arg(smLine.trimmed());
        }
    }
    file.close();
    if (contents.size()) {
        QDateTime datetime = fileInfo.lastModified();
        insert(fileInfo.fileName(),
               contents,
               datetime,
               UNOFFICIAL_OTHER,
               false/*displayModel*/,
               true/*generated*/,
               true/*includeFile*/,
               false/*dataFile*/,
               fileInfo.absoluteFilePath(),
               fileInfo.completeBaseName());
    }

    return true;
}

void LDrawFile::processMetaCommand(const QStringList &tokens)
{
    int number;
    bool validNumber;

    if (tokens.size() < 4)
        return;

    QString const enabled = QObject::tr("Enabled");
    QString const disabled = QObject::tr("Disabled");

    // Check if load external parts in command editor is disabled
    if (metaLoadUnoffPartsNotFound) {
        if (tokens.at(2) == QLatin1String("LOAD_UNOFFICIAL_PARTS_IN_EDITOR")) {
            _loadUnofficialParts = tokens.last() == QLatin1String("FALSE") ? false : true ;
            emit gui->messageSig(LOG_INFO, QObject::tr("Load Custom Unofficial Parts In Command Editor is %1")
                                                       .arg(_loadUnofficialParts ? enabled : disabled));
            metaLoadUnoffPartsNotFound = false;
        }
    }

    // Check if BuildMod is enabled
    if (metaBuildModNotFund) {
        if (tokens.at(2) == QLatin1String("BUILD_MOD_ENABLED")) {
            _loadBuildMods  = tokens.last() == QLatin1String("FALSE") ? false : true ;
            Preferences::buildModEnabled = _loadBuildMods;
            emit gui->messageSig(LOG_INFO, QObject::tr("Build Modifications are %1")
                                                       .arg(Preferences::buildModEnabled ? enabled : disabled));
            metaBuildModNotFund = false;
        }
    }

    // Check if FadeSteps is enabled
    if (metaFadeStepsNotFound) {
        if (tokens.at(2) == QLatin1String("FADE_STEPS")) {
            Preferences::enableFadeSteps = tokens.last() == QLatin1String("ENABLED") ? true : false ;
            emit gui->messageSig(LOG_INFO, QObject::tr("Fade Steps are %1")
                                                       .arg(Preferences::enableFadeSteps ? enabled : disabled));
            metaFadeStepsNotFound = false;
        }
    }

    // Check if HighlightStep is enabled
    if (metaHighlightStepNotFound) {
        if (tokens.at(2) == QLatin1String("HIGHLIGHT_STEP")) {
            Preferences::enableHighlightStep = tokens.last() == QLatin1String("ENABLED") ? true : false ;
            emit gui->messageSig(LOG_INFO, QObject::tr("Highlight Step is %1")
                                                       .arg(Preferences::enableHighlightStep ? enabled : disabled));
            metaHighlightStepNotFound = false;
        }
    }

    // Check if insert final model is enabled
    if (metaFinalModelNotFound) {
        if (tokens.at(2) == QLatin1String("FINAL_MODEL_ENABLED")) {
            Preferences::finalModelEnabled = tokens.last() == QLatin1String("FALSE") ? false : true ;
            if (Preferences::enableFadeSteps || Preferences::enableHighlightStep)
                emit gui->messageSig(LOG_INFO, QObject::tr("Display Final Model is %1")
                                                           .arg(Preferences::finalModelEnabled ? enabled : disabled));
            metaFinalModelNotFound = false;
        }
    }

    // Check if Start Page Number is specified
    if (metaStartPageNumNotFound) {
        if (tokens.at(2) == QLatin1String("START_PAGE_NUMBER")) {
            number = tokens.last().toInt(&validNumber);
            Gui::pa  = validNumber ? number - 1 : 0;
            emit gui->messageSig(LOG_INFO, QObject::tr("Start Page Number Set to %1").arg(QString::number(Gui::pa)));
            metaStartPageNumNotFound = false;
        }
    }

    // Check if Start Step Number is specified
    if (metaStartStepNumNotFound) {
        if (tokens.at(2) == QLatin1String("START_STEP_NUMBER")) {
            number = tokens.last().toInt(&validNumber);
            Gui::sa  = validNumber ? number - 1 : 0;
            emit gui->messageSig(LOG_INFO, QObject::tr("Start Step Number Set to %1").arg(QString::number(Gui::sa)));
            metaStartStepNumNotFound = false;
        }
    }
}

QString LDrawFile::fileType(int isUnofficial)
{
    if (isUnofficial)
        return LDrawUnofficialType[unofficialPart];
    else
        return topLevelModel
                ? QObject::tr("model")
                : unofficialPart
                  ? LDrawUnofficialType[unofficialPart]
                  : hdrFILENotFound || !_mpd
                      ? QObject::tr("subfile")
                      : QObject::tr("submodel");
};

void LDrawFile::loadMPDFile(const QString &fileName, bool externalFile)
{
    MissingHeaderType headerMissing = NoneMissing;
    auto missingHeaders = [this] ()
    {
        if (hdrNameNotFound && hdrAuthorNotFound)
            return BothMissing;
        else if (hdrNameNotFound)
            return NameMissing;
        else if (hdrAuthorNotFound)
            return AuthorMissing;
        return NoneMissing;
    };

    QFileInfo   fileInfo(fileName);

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        emit gui->messageSig(LOG_ERROR, QObject::tr("Cannot read mpd file %1<br>%2")
                                                    .arg(fileName, file.errorString()));
        return;
    }

    int subfileIndx          = -1;
    int lineIndx             = -1;
    bool alreadyLoaded       = false;
    bool subfileFound        = false;
    bool isDatafile          = false;
    bool partHeaderFinished  = false;
    bool stagedSubfilesFound = externalFile;
    bool modelHeaderFinished = externalFile ? true : false;
    bool sosf = false;
    bool eosf = false;
    _mpd      = true;

    if (topLevelModel) {
        topHeaderFinished          = false;
        topFileNotFound            = true;
        displayModel               = false;
        hdrFILENotFound            = true;
        hdrDescNotFound            = true;
        hdrCategNotFound           = true;
        helperPartsNotFound        = true;
        lsynthPartsNotFound        = true;
        metaLoadUnoffPartsNotFound = true;
        metaFadeStepsNotFound      = true;
        metaHighlightStepNotFound  = true;
        metaBuildModNotFund        = true;
        metaFinalModelNotFound     = true;
        metaStartPageNumNotFound   = true;
        metaStartStepNumNotFound   = true;
        hdrDescLine                = 0;
    }

    hdrNameNotFound   = true;
    hdrAuthorNotFound = true;
    hdrNameKey        = false;
    hdrAuthorKey      = false;
    hdrNameLine       = 0;
    hdrAuthorLine     = 0;
    unofficialPart    = UNOFFICIAL_UNKNOWN;

    QByteArray dataFile;
    QString subfileName, subFile, smLine, datafileName;
    QStringList stagedContents, stagedSubfiles, contents, tokens, searchPaths;
    QRegularExpressionMatch match;

    if (Preferences::searchLDrawSearchDirs)
        searchPaths = Preferences::ldSearchDirs;
    QString ldrawPath = QDir::toNativeSeparators(Preferences::ldrawLibPath);
    if (Preferences::searchOfficialModels)
        if (!searchPaths.contains(ldrawPath + QDir::separator() + "MODELS",Qt::CaseInsensitive))
            searchPaths.append(ldrawPath + QDir::separator() + "MODELS");
    if (Preferences::searchOfficialParts)
        if (!searchPaths.contains(ldrawPath + QDir::separator() + "PARTS",Qt::CaseInsensitive))
            searchPaths.append(ldrawPath + QDir::separator() + "PARTS");
    if (Preferences::searchOfficialPrimitives)
        if (!searchPaths.contains(ldrawPath + QDir::separator() + "P",Qt::CaseInsensitive))
            searchPaths.append(ldrawPath + QDir::separator() + "P");
    if (Preferences::searchUnofficialParts)
        if (!searchPaths.contains(ldrawPath + QDir::separator() + "UNOFFICIAL" + QDir::separator() + "PARTS",Qt::CaseInsensitive))
            searchPaths.append(ldrawPath + QDir::separator() + "UNOFFICIAL" + QDir::separator() + "PARTS");
    if (Preferences::searchUnofficialPrimitives)
        if (!searchPaths.contains(ldrawPath + QDir::separator() + "UNOFFICIAL" + QDir::separator() + "P",Qt::CaseInsensitive))
            searchPaths.append(ldrawPath + QDir::separator() + "UNOFFICIAL" + QDir::separator() + "P");
    if (Preferences::searchUnofficialTextures)
        if (!searchPaths.contains(ldrawPath + QDir::separator() + "UNOFFICIAL" + QDir::separator() + "TEXTURES",Qt::CaseInsensitive))
            searchPaths.append(ldrawPath + QDir::separator() + "UNOFFICIAL" + QDir::separator() + "TEXTURES");

    /* Read it in the first time to put into fileList in order of appearance */

    QTextStream in(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    in.setEncoding(_currFileIsUTF8 ? QStringConverter::Utf8 : QStringConverter::System);
#else
    in.setCodec(_currFileIsUTF8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForName("System"));
#endif

    while ( ! in.atEnd()) {
        QString smLine = in.readLine(0);
        stagedContents << smLine.trimmed();
    }
    file.close();

    int lineCount = stagedContents.size();

    if (topLevelModel)
        emit gui->progressPermInitSig();
    else
        gui->progressBarPermReset();
    emit gui->progressBarPermSetRangeSig(1, lineCount);
    emit gui->progressLabelPermSetTextSig(QObject::tr("Loading MPD Model '%1'...").arg( fileInfo.fileName()));

#ifdef QT_DEBUG_MODE
    emit gui->messageSig(LOG_DEBUG, QString("Stage Contents Size: %1").arg(lineCount));
#endif

    QDateTime datetime = QFileInfo(fileName).lastModified();

    for (lineIndx = 0; lineIndx < lineCount; lineIndx++) {

        smLine = stagedContents.at(lineIndx).trimmed();

        emit gui->progressBarPermSetValueSig(lineIndx);

        if (smLine.isEmpty())
            continue;
        bool sof = smLine.contains(_fileRegExp[SOF_RX]);  //start of submodel file
        bool eof = smLine.contains(_fileRegExp[EOF_RX]);  //end of submodel file

        // load LDCad groups
        if (!ldcadGroupsLoaded) {
            if (smLine.contains(_fileRegExp[LDG_RX])) {
                match = _fileRegExp[LDG_RX].match(smLine);
                insertLDCadGroup(match.captured(3),match.captured(1).toInt());
                insertLDCadGroup(match.captured(2),match.captured(1).toInt());
            } else if (smLine.contains("0 STEP") || smLine.contains("0 ROTSTEP")) {
                ldcadGroupsLoaded = true;
            }
        }

        split(smLine,tokens);

        // indicate submodel in display model step or substitute type
        if (tokens.size() && tokens.at(0) == "0") {
            if (displayModel)
                displayModel = !smLine.contains(_fileRegExp[LDS_RX]); // LDraw Step Boundry
            else
                displayModel = smLine.contains(_fileRegExp[DMS_RX]);  // Display Model Step

            if (isSubstitute(smLine,subFile))
                subfileFound = !subFile.isEmpty();
        }
        // part type 1-5 check
        else if ((subfileFound = tokens.size() == 15)) {
            modelHeaderFinished = partHeaderFinished = true;
            subFile = tokens.at(14);
        }

        // subfile, helper and lsynth part check
        if (subfileFound && ! _processedSubfiles.contains(subFile, Qt::CaseInsensitive)) {
            _processedSubfiles.append(subFile);
            PieceInfo* pieceInfo = lcGetPiecesLibrary()->FindPiece(subFile.toLatin1().constData(), nullptr, false, false);
            if (! pieceInfo && ! LDrawFile::contains(subFile, Qt::CaseInsensitive) && ! stagedSubfiles.contains(subFile, Qt::CaseInsensitive)) {
                if (displayModel)
                    stagedSubfiles.append(QString("%1|displaymodel").arg(subFile));
                else
                    stagedSubfiles.append(subFile);
                stagedSubfilesFound = true;
                // determine if missing helper or lsynth part
                if (helperPartsNotFound || lsynthPartsNotFound) {
                    int is_support_file = ExcludedParts::isExcludedSupportPart(subFile);
                    if (is_support_file == ExcludedParts::EP_HELPER) {
                        helperPartsNotFound = false;
                        _helperPartsNotInArchive = true;
                    } else if (is_support_file == ExcludedParts::EP_LSYNTH) {
                        lsynthPartsNotFound = false;
                        _lsynthPartsNotInArchive = true;
                    }
                }
            }
        }

        if ((sof || !hdrFILENotFound) && !modelHeaderFinished) {
            if (sof) {
                hdrFILENotFound = false;        /* we have an LDraw submodel */
                hdrDescLine = lineIndx + 1;     /* next line should be description */
                if (!externalFile)
                    modelHeaderFinished = false;/* set model header flag */
                // One time populate top level file name
                if (topFileNotFound) {
                    match = _fileRegExp[SOF_RX].match(smLine);
                    if (match.captured(1).isEmpty()) {    /* we have a FILE key without a value */
                        const QString topFile = QString("Model-%1-%2").arg(randomFour()).arg(fileInfo.fileName());
                        smLine = QString("0 FILE %1").arg(topFile);
                        const QString message = QObject::tr("MPD %1 '%2' header 'FILE %3' was added by %4 (file: %5, line: %6).")
                                                            .arg(fileType(), fileInfo.fileName(), topFile, VER_PRODUCTNAME_STR,
                                                                 fileInfo.fileName()).arg(lineIndx + 1);
                        const QString statusEntry = QString("%1|%2|%3").arg(BAD_DATA_LOAD_MSG).arg(fileInfo.fileName(), message);
                        loadStatusEntry(BAD_DATA_LOAD_MSG, statusEntry, fileInfo.fileName(), message);
                    }
                    _file = QFileInfo(match.captured(1)).baseName().trimmed();
                    topFileNotFound = false;
                }
            } else {
                if (hdrDescNotFound && lineIndx == hdrDescLine) {
                    if (smLine.contains(_fileRegExp[DES_RX]) && ! isHeader(smLine)) {
                        _description = _fileRegExp[DES_RX].match(smLine).captured(1);
                        if (topLevelModel)
                            Preferences::publishDescription = _description;
                        hdrDescNotFound = false;
                    } else
                        _description = subfileName.isEmpty() ? _file : subfileName;
                }

                if (hdrNameNotFound && !hdrNameKey) {
                    if (smLine.contains(_fileRegExp[NAM_RX])) {
                        if (topLevelModel)
                            _name = _fileRegExp[NAM_RX].match(smLine).captured(1);
                        hdrNameNotFound = false;
                    } else if (smLine.contains(_fileRegExp[NAK_RX])) {
                        hdrNameKey = true;
                    }
                    if (!hdrNameNotFound || hdrNameKey)
                        hdrNameLine = lineIndx;
                }

                if (hdrAuthorNotFound && !hdrAuthorKey) {
                    if (smLine.contains(_fileRegExp[AUT_RX])) {
                        if (topLevelModel) {
                            _author = _fileRegExp[AUT_RX].match(smLine).captured(1);
                            Preferences::defaultAuthor = _author;
                        }
                        hdrAuthorNotFound = false;
                    } else if (smLine.contains(_fileRegExp[AUK_RX])) {
                        hdrAuthorKey = true;
                    }
                    if (!hdrAuthorNotFound || hdrAuthorKey)
                        hdrAuthorLine = lineIndx;
                }

                // One time populate model category (not used)
                if (hdrCategNotFound) {
                    if (smLine.contains(_fileRegExp[CAT_RX])) {
                        if (topLevelModel)
                            _category = _fileRegExp[CAT_RX].match(smLine).captured(1);
                        hdrCategNotFound = false;
                    }
                }

                if (unofficialPart == UNOFFICIAL_UNKNOWN) {
                    unofficialPart = getUnofficialFileType(smLine);
                    if (unofficialPart) {
                        emit gui->messageSig(LOG_TRACE, QObject::tr("Subfile '%1' spcified as %2.")
                                                                    .arg(subfileName, fileType()));
                    }
                }

                // Check for include file
                if (smLine.contains(_fileRegExp[INC_RX])) {
                    const QString inclFilePath = LPub::getFilePath(_fileRegExp[INC_RX].match(smLine).captured(1));
                    QFileInfo inclFileInfo(inclFilePath);
                    if (inclFileInfo.isReadable()) {
                        if (loadIncludeFile(inclFilePath)) {
                            const QString statusEntry = QObject::tr("%1|%2|MPD Include file %2 (file: %3, line: %4)")
                                              .arg(INCLUDE_FILE_LOAD_MSG).arg(inclFileInfo.fileName(), subfileName).arg(lineIndx + 1);
                            loadStatusEntry(INCLUDE_FILE_LOAD_MSG, statusEntry, inclFileInfo.fileName(), QObject::tr("Subfile [%1] is a MPD Include file"));
                        }
                    } else {
                        const QString message = QObject::tr("MPD Include file '%1' was not found (file: %2, line: %3).")
                                                            .arg(inclFilePath, fileInfo.fileName()).arg(lineIndx + 1);
                        const QString statusEntry = QObject::tr("%1|%2|MPD Include file '%2' was not found (subfile: %3, line: %4).")
                                                                .arg(BAD_INCLUDE_LOAD_MSG).arg(inclFileInfo.fileName(), subfileName).arg(lineIndx + 1);
                        loadStatusEntry(BAD_INCLUDE_LOAD_MSG, statusEntry, inclFileInfo.fileName(), message);
                    }
                }

                // Check meta commands
                if (!isComment(smLine))
                    processMetaCommand(tokens);
            }
        } // modelHeaderFinished

        if ((alreadyLoaded = LDrawFile::contains(subfileName, Qt::CaseInsensitive))) {
            emit gui->messageSig(LOG_TRACE, QObject::tr("MPD %1 '%2' already loaded.").arg(fileType(), subfileName));
            subfileIndx = stagedSubfiles.indexOf(subfileName);
            if (subfileIndx > NOT_FOUND)
                stagedSubfiles.removeAt(subfileIndx);
        }

        // processing inlined parts or base 64 data
        if (!sof && hdrFILENotFound) {
            if (subfileName.isEmpty() && hdrNameNotFound) {
                sosf = smLine.contains(_fileRegExp[NAM_RX]) || (isDatafile = smLine.contains(_fileRegExp[DAT_RX]));
                if (!sosf)
                    contents << smLine;
            } else if (!hdrNameNotFound && smLine.startsWith("0")) {
                if ((eosf = smLine.contains(_fileRegExp[NAM_RX]))) {
                    const QString &lastLine = contents.last();
                    if (lastLine.contains(_fileRegExp[DES_RX]))
                        _description = contents.takeLast();     // for inline files
                } else if ((eosf = lineIndx == lineCount - 1 && smLine == "0"))
                    contents << smLine;                         // for external files
            }

            if (!sosf && !eosf) {
                if (hdrAuthorNotFound) {
                    if (smLine.contains(_fileRegExp[AUT_RX]))
                        hdrAuthorNotFound = false;
                }
                if (! partHeaderFinished && unofficialPart == UNOFFICIAL_UNKNOWN) {
                    unofficialPart = getUnofficialFileType(smLine);
                    if (unofficialPart) {
                        emit gui->messageSig(LOG_TRACE, QObject::tr("Inline file '%1' spcified as %2.")
                                                                    .arg(subfileName, fileType()));
                    }
                }
            }
        } // inlined parts or base 64 data

        // processing base 64 data lines
        if (isDatafile) {
            if (smLine.contains(_fileRegExp[B64_RX])) {
                dataFile.append(_fileRegExp[B64_RX].match(smLine).captured(1).toUtf8());
            } else if (! sosf) {
                eosf = true;
                saveDatafile(datafileName, dataFile);
                dataFile.clear();
            }
        } // base 64 data lines

        /* - if at start of file marker, populate subfileName
         * - if at end of file marker, clear subfileName
         */
        if (sof || eof || sosf || eosf) {
            /* - if at end of file marker
             * - insert items if subfileName and contents are not empty
             * - after insert, clear contents
             */
            if (! subfileName.isEmpty()) {
                if (! alreadyLoaded) {
                    if (contents.isEmpty()) {
                        const QString message = QObject::tr("MPD %1 '%2' is empty and was not loaded (file: %3, line: %4).")
                                                            .arg(fileType(), subfileName, fileInfo.fileName()).arg(lineIndx + 1);
                        const QString statusEntry = QObject::tr("%1|%2|%3").arg(EMPTY_SUBMODEL_LOAD_MSG).arg(subfileName, message);
                        loadStatusEntry(EMPTY_SUBMODEL_LOAD_MSG, statusEntry, subfileName, message);
                    } else {
                        insert(subfileName,
                               contents,
                               datetime,
                               unofficialPart,
                               displayModel,
                               false/*generated*/,
                               false/*includeFile*/,
                               isDatafile/*dataFile*/,
                               externalFile ? fileInfo.absoluteFilePath() : "",
                               _description);
                    }
                    if (contents.size()) {
                        if (!isDatafile && (headerMissing = MissingHeaderType(missingHeaders())))
                            normalizeHeader(subfileName, fileInfo.fileName(), headerMissing);
                        emit gui->messageSig(LOG_NOTICE, QObject::tr("MPD %1 '%2' with %3 lines loaded.")
                                                                     .arg(fileType(), subfileName, QString::number(size(subfileName))));
                    }
                    isDatafile = false;
                    topLevelModel = false;
                    unofficialPart = UNOFFICIAL_UNKNOWN;
                }

                subfileIndx = stagedSubfiles.indexOf(subfileName);
                if (subfileIndx > NOT_FOUND)
                    stagedSubfiles.removeAt(subfileIndx);

                contents.clear();
            }

            /* - if at start of file marker
             * - set subfileName of new file
             * - else if at end of file marker, clear subfileName
             */
            if (sof || sosf) {
                if (sof) {
                    hdrNameLine       = 0;
                    hdrAuthorLine     = 0;
                    hdrDescLine       = 0;
                    hdrNameKey        = false;
                    hdrAuthorKey      = false;
                    hdrNameNotFound   = true;
                    hdrAuthorNotFound = true;
                    hdrFILENotFound   = false; /* we are at the beginning of an LDraw submodel */
                    modelHeaderFinished = false;
                    subfileName = _fileRegExp[SOF_RX].match(smLine).captured(1).trimmed();
                } else/*sosf*/ {
                    hdrNameNotFound = sosf = false;
                    partHeaderFinished = isDatafile ? true : false;
                    subfileName = isDatafile
                                    ?  _fileRegExp[DAT_RX].match(smLine).captured(1).trimmed()
                                    : _fileRegExp[NAM_RX].match(smLine).captured(1);
                    contents << smLine;
                }
                unofficialPart = isDatafile ? UNOFFICIAL_DATA : UNOFFICIAL_UNKNOWN;
                if (! alreadyLoaded)
                    emit gui->messageSig(LOG_INFO_STATUS, QObject::tr("Loading subfile '%1'...").arg(subfileName));
                if (isDatafile) {
                    if (subfileName.isEmpty()) {
                        const QString id = QString("DATA%1").arg(lineIndx + 1);
                        const QString message = QObject::tr("Malformed !DATA command. No file name specified (file: %1, line: %2).<br>Line: %3")
                                                            .arg(fileInfo.fileName()).arg(lineIndx + 1).arg(smLine);
                        const QString statusEntry = QObject::tr("%1|DATA%2|Malformed !DATA command. No file name specified (subfile: %3, line: %4).")
                                                                .arg(BAD_DATA_LOAD_MSG).arg(lineIndx).arg(fileInfo.fileName()).arg(lineIndx + 1);
                        loadStatusEntry(BAD_DATA_LOAD_MSG, statusEntry, id, message);
                    } else {
                        emit gui->messageSig(LOG_TRACE, QObject::tr("MPD subfile '%1' spcified as %2.").arg(subfileName, fileType()));
                    }
                }
            } else if (eof || eosf) {
                /* - at the end of submodel file or inline part
                */
                subfileName.clear();
                hdrNameNotFound   = true; /* reset Name capture*/
                hdrAuthorNotFound = true; /* reset Author capture*/
                hdrDescNotFound   = true; /* reset Description capture */
                hdrNameKey        = false;
                hdrAuthorKey      = false;
                hdrNameLine       = 0;
                hdrAuthorLine     = 0;
                hdrDescLine       = 0;
                if (eof) {
                    hdrFILENotFound = true; /* we are at the end of an LDraw submodel */
                    modelHeaderFinished = false;
                } else/*eosf*/ {
                    /* - if description found, add it to the start of the part's contents
                    */
                    if (!_description.isEmpty())
                        contents << _description;
                    /* - at the Name: of a new inline part so revert by 1 line to capture
                    */
                    lineIndx--;
                    eosf = false;
                }
                /* - clear description for next model or inline part
                */
                _description.clear();
            }
        } else if (! subfileName.isEmpty() && !smLine.isEmpty()) {
            /* - after start of file - subfileName not empty
             * - add line to contents
             */
            contents << smLine;
        }
    } // iterate stagedContents

    // resolve outstanding image file from base 64 data
    if (isDatafile)
        saveDatafile(datafileName, dataFile);

    // at end of file - NOFILE tag not specified
    if ( ! subfileName.isEmpty()) {
        if (LDrawFile::contains(subfileName, Qt::CaseInsensitive)) {
            emit gui->messageSig(LOG_TRACE, QObject::tr("MPD %1 '%2' already loaded").arg(fileType(), subfileName));
        } else {
            if (contents.isEmpty()) {
                const QString message = QObject::tr("MPD %1 '%2' is empty and was not loaded (file: %3, line: %4).")
                                                    .arg(fileType(), subfileName, fileInfo.fileName()).arg(lineIndx + 1);
                const QString statusEntry = QObject::tr("%1|%2|%3").arg(EMPTY_SUBMODEL_LOAD_MSG).arg(subfileName, message);
                loadStatusEntry(EMPTY_SUBMODEL_LOAD_MSG, statusEntry, subfileName, message);
            } else {
                insert(subfileName,
                       contents,
                       datetime,
                       unofficialPart,
                       displayModel,
                       false/*generated*/,
                       false/*includeFile*/,
                       isDatafile/*dataFile*/,
                       externalFile ? fileInfo.absoluteFilePath() : "",
                       _description);
            }
            if (contents.size()) {
                if (!isDatafile && (headerMissing = MissingHeaderType(missingHeaders())))
                    normalizeHeader(subfileName, fileInfo.fileName(), headerMissing);
                emit gui->messageSig(LOG_NOTICE, QObject::tr("MPD %1 '%2' with %3 lines loaded.")
                                                             .arg(fileType(), subfileName, QString::number(size(subfileName))));
            }
        }

        contents.clear();

        subfileIndx = stagedSubfiles.indexOf(subfileName);
        if (subfileIndx > NOT_FOUND)
            stagedSubfiles.removeAt(subfileIndx);
    } // at end of file

    // resolve outstanding subfiles
    if (stagedSubfiles.size()) {
        stagedSubfiles.removeDuplicates();

#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_DEBUG, QString("%1 unresolved staged %2 specified.")
                                                .arg(stagedSubfiles.size()).arg(stagedSubfiles.size() == 1 ? "subfile" : "subfiles"));
#endif
        QString const projectPath = QDir::toNativeSeparators(fileInfo.absolutePath());
        QString subfile, fileDesc;
        for (const QString &stagedFile : stagedSubfiles) {
            displayModel = false;
            if (stagedFile.count("|")) {
                QStringList elements = stagedFile.split("|");
                subfile = elements.first();
                if ((displayModel = elements.last() == QLatin1String("displaymodel")))
                    fileDesc = QObject::tr("(display model) ");
            } else {
                subfile = stagedFile;
            }
#ifdef QT_DEBUG_MODE
            emit gui->messageSig(LOG_DEBUG, QString("Processing staged %1subfile '%2'...")
                                                    .arg(fileDesc, subfile));
#endif
            if (LDrawFile::contains(subfile, Qt::CaseInsensitive)) {
                if (displayModel)
                    setDisplayModel(subfile);
                QString const subfileType = fileType(isUnofficialPart(subfile));
                emit gui->messageSig(LOG_TRACE, QObject::tr("MPD %1%2 '%3' already loaded.")
                                                            .arg(fileDesc, subfileType, subfile));
                continue;
            }
            // subfile path
            if ((subfileFound = QFileInfo(subfile).isFile())) {
                fileInfo = QFileInfo(subfile);
            } else if (Preferences::extendedSubfileSearch) {
                // extended search - current project path
                if (Preferences::searchProjectPath && (subfileFound = QFileInfo(projectPath + QDir::separator() + subfile).isFile())) {
                    fileInfo = QFileInfo(projectPath + QDir::separator() + subfile);
                }
                if (!subfileFound) {
                    // extended search - LDraw subfolder paths and extra search directories
                    for (QString &subFilePath : searchPaths) {
                        if ((subfileFound = QFileInfo(subFilePath + QDir::separator() + subfile).isFile())) {
                            fileInfo = QFileInfo(subFilePath + QDir::separator() + subfile);
                            break;
                        }
                    }
                }
            }

            bool externalSubfile = true;
            if (!subfileFound) {
                QString partFile = subfile.toUpper();
                if (partFile.startsWith("S\\"))
                    partFile.replace("S\\","S/");
                PieceInfo* pieceInfo = lcGetPiecesLibrary()->FindPiece(partFile.toLatin1().constData(), nullptr/*CurrentProject*/, false/*CreatePlaceholder*/, false/*SearchProjectFolder*/);
                if ((subfileFound = pieceInfo) && pieceInfo->IsPartType()) {
                    emit gui->messageSig(LOG_NOTICE, QObject::tr("Staged %1subfile '%2' is a part.").arg(fileDesc, subfile));
                } else
                if ((subfileFound = lcGetPiecesLibrary()->IsPrimitive(partFile.toLatin1().constData()))) {
                    lcLibraryPrimitive* Primitive = lcGetPiecesLibrary()->FindPrimitive(partFile.toLatin1().constData());
                    if (Primitive) {
                        emit gui->messageSig(LOG_NOTICE, QObject::tr("Staged %1subfile '%2' is a %3.")
                                                                     .arg(fileDesc, subfile, Primitive->mSubFile ? QObject::tr("subpart") : QObject::tr("primitive")));
                    }
                }
                externalSubfile = !subfileFound;
            }

            if (!subfileFound) {
                emit gui->messageSig(LOG_WARNING, QObject::tr("Staged %1subfile '%2' not found.").arg(fileDesc, subfile));
            } else if (externalSubfile) {
                emit gui->messageSig(LOG_NOTICE, QObject::tr("External %1subfile '%2' found.").arg(fileDesc, subfile));

                bool savedTopLevelModel  = topLevelModel;
                bool savedUnofficialPart = unofficialPart;
                topLevelModel            = false;

                loadMPDFile(fileInfo.absoluteFilePath(), true/*external*/);

                subfileIndx = stagedSubfiles.indexOf(subfile);
                if (subfileIndx > NOT_FOUND)
                    stagedSubfiles.removeAt(subfileIndx);

                topLevelModel  = savedTopLevelModel;
                unofficialPart = savedUnofficialPart;
            }

            fileDesc.clear();
        }
    } // resolve outstanding subfiles

    // restore last subfileName for final processing
    if (stagedSubfilesFound && subfileName.isEmpty())
        subfileName = QFileInfo(fileName).fileName();
    else
        subfileName = fileInfo.fileName();

#ifdef QT_DEBUG_MODE
    if (!stagedSubfiles.size())
        emit gui->messageSig(LOG_DEBUG, QString("All '%1' staged subfiles processed.").arg(subfileName));
#if QT_VERSION >= QT_VERSION_CHECK(6,5,0)
    QMultiHashIterator<QString, int> i(_ldcadGroups);
#else
    QHashIterator<QString, int> i(_ldcadGroups);
#endif
    while (i.hasNext()) {
        i.next();
        emit gui->messageSig(LOG_TRACE, QString("LDCad Groups: Name[%1], LineID[%2].")
                             .arg(i.key()).arg(i.value()));
    }
#endif

    emit gui->progressBarPermSetValueSig(lineCount);
    if (!stagedSubfiles.size()) {
        emit gui->progressLabelPermSetTextSig(QString());
        emit gui->progressPermStatusRemoveSig();
    }

    emit gui->messageSig(LOG_NOTICE, QObject::tr("MPD file '%1' with %2 lines loaded.")
                                                 .arg(subfileName).arg(lineCount));
}

void LDrawFile::loadLDRFile(const QString &filePath, const QString &fileName, bool externalFile)
{
    MissingHeaderType headerMissing = NoneMissing;
    auto missingHeaders = [this] ()
    {
        if (hdrNameNotFound && hdrAuthorNotFound)
            return BothMissing;
        else if (hdrNameNotFound)
            return NameMissing;
        else if (hdrAuthorNotFound)
            return AuthorMissing;
        return NoneMissing;
    };

    QString fullName;
    if (fileName.isEmpty())
        fullName = QDir::toNativeSeparators(filePath);
    else
        fullName = QDir::toNativeSeparators(filePath + "/" + fileName);

    QFileInfo   fileInfo(fullName);

    if (_subFiles[fileInfo.fileName()]._contents.isEmpty()) {
        _subFiles.remove(fileInfo.fileName());

        QFile file(fullName);
        if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
            emit gui->messageSig(LOG_ERROR,QObject::tr("Cannot read ldr file %1<br>%2")
                                                       .arg(fullName, file.errorString()));
            return;
        }

        int subfileIndx          = -1;
        int lineIndx             = -1;
        bool modelBegin          = false;
        bool alreadyLoaded       = false;
        bool subfileFound        = false;
        bool partHeaderFinished  = false;
        bool sosf = false;
        bool eosf = false;
        _mpd      = false;

        if (topLevelModel) {
            topFileNotFound            = true;
            topHeaderFinished          = false;
            displayModel               = false;
            hdrDescNotFound            = true;
            hdrCategNotFound           = true;
            helperPartsNotFound        = true;
            lsynthPartsNotFound        = true;
            metaLoadUnoffPartsNotFound = true;
            metaBuildModNotFund        = true;
            metaFadeStepsNotFound      = true;
            metaHighlightStepNotFound  = true;
            metaFinalModelNotFound     = true;
            metaStartPageNumNotFound   = true;
            metaStartStepNumNotFound   = true;
            hdrDescLine                = 0;
        }

        hdrNameNotFound   = true;
        hdrAuthorNotFound = true;
        hdrNameKey        = false;
        hdrAuthorKey      = false;
        hdrNameLine       = 0;
        hdrAuthorLine     = 0;
        unofficialPart = UNOFFICIAL_UNKNOWN;

        QString subfileName, subFile, smLine;
        QStringList stagedContents, stagedSubfiles, contents, tokens, searchPaths;
        QRegularExpressionMatch match;

        if (Preferences::searchLDrawSearchDirs)
            searchPaths = Preferences::ldSearchDirs;
        QString ldrawPath = QDir::toNativeSeparators(Preferences::ldrawLibPath);
        if (Preferences::searchOfficialModels)
            if (!searchPaths.contains(ldrawPath + QDir::separator() + "MODELS",Qt::CaseInsensitive))
                searchPaths.append(ldrawPath + QDir::separator() + "MODELS");
        if (Preferences::searchOfficialParts)
            if (!searchPaths.contains(ldrawPath + QDir::separator() + "PARTS",Qt::CaseInsensitive))
                searchPaths.append(ldrawPath + QDir::separator() + "PARTS");
        if (Preferences::searchOfficialPrimitives)
            if (!searchPaths.contains(ldrawPath + QDir::separator() + "P",Qt::CaseInsensitive))
                searchPaths.append(ldrawPath + QDir::separator() + "P");
        if (Preferences::searchUnofficialParts)
            if (!searchPaths.contains(ldrawPath + QDir::separator() + "UNOFFICIAL" + QDir::separator() + "PARTS",Qt::CaseInsensitive))
                searchPaths.append(ldrawPath + QDir::separator() + "UNOFFICIAL" + QDir::separator() + "PARTS");
        if (Preferences::searchUnofficialPrimitives)
            if (!searchPaths.contains(ldrawPath + QDir::separator() + "UNOFFICIAL" + QDir::separator() + "P",Qt::CaseInsensitive))
                searchPaths.append(ldrawPath + QDir::separator() + "UNOFFICIAL" + QDir::separator() + "P");
        if (Preferences::searchUnofficialTextures)
            if (!searchPaths.contains(ldrawPath + QDir::separator() + "UNOFFICIAL" + QDir::separator() + "TEXTURES",Qt::CaseInsensitive))
                searchPaths.append(ldrawPath + QDir::separator() + "UNOFFICIAL" + QDir::separator() + "TEXTURES");

        /* Read it in the first time to put into fileList in order of appearance */

        QTextStream in(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
        in.setEncoding(_currFileIsUTF8 ? QStringConverter::Utf8 : QStringConverter::System);
#else
        in.setCodec(_currFileIsUTF8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForName("System"));
#endif
        bool checked = false;
        while ( ! in.atEnd()) {
            smLine = in.readLine(0);
            if (!smLine.isEmpty())
                stagedContents << smLine.trimmed();
            if (!checked) {
                checked = (smLine.contains(_fileRegExp[LDR_RX]) || smLine.contains(_fileRegExp[NAM_RX]));
                if (smLine.contains(_fileRegExp[SOF_RX])) {
                    file.close();
                    stagedContents.clear();
                    const QString fileTypeUpper = fileType();
                    const QString scModelType = fileTypeUpper[0].toUpper() + fileType().right(fileType().size() - 1);
                    emit gui->messageSig(LOG_INFO_STATUS, QObject::tr("%1 file %2 identified as Multi-Part LDraw System (MPD) Document").arg(scModelType, fileInfo.fileName()));
                    loadMPDFile(fileInfo.absoluteFilePath());
                    return;
                }
            }
        }
        file.close();

        int lineCount = stagedContents.size();

        if (topLevelModel)
            emit gui->progressPermInitSig();
        else
            gui->progressBarPermReset();
        emit gui->progressBarPermSetRangeSig(1, lineCount);
        emit gui->progressLabelPermSetTextSig(QObject::tr("Loading LDR Model '%1'...").arg(fileInfo.fileName()));

#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_DEBUG, QString("Stage Contents Size: %1").arg(lineCount));
#endif

        QDateTime datetime = fileInfo.lastModified();

        /* read it a second time to find submodels and check for completeness*/

        for (lineIndx = 0; lineIndx < lineCount; lineIndx++) {

            smLine = stagedContents.at(lineIndx).trimmed();

            emit gui->progressBarPermSetValueSig(lineIndx);

            if (smLine.isEmpty())
                continue;

            if (subfileName.isEmpty() && !hdrNameKey && hdrNameNotFound && (topLevelModel || !smLine.isEmpty())) {
                sosf = smLine.contains(_fileRegExp[NAM_RX]);
            } else if (!hdrNameNotFound && smLine.startsWith("0")) {
                if ((eosf = smLine.contains(_fileRegExp[NAM_RX]))) {
                    const QString &lastLine = contents.last();
                    if (lastLine.contains(_fileRegExp[DES_RX])) {
                        _description = _fileRegExp[DES_RX].match(smLine).captured(1); // for inline files
                    }
                } else if ((eosf = lineIndx == lineCount - 1 && smLine == "0"))
                    contents << smLine;                            // for external files
            }

            // load LDCad groups
            if (!ldcadGroupsLoaded) {
                if(smLine.contains(_fileRegExp[LDG_RX])) {
                    match = _fileRegExp[LDG_RX].match(smLine);
                    insertLDCadGroup(match.captured(3),match.captured(1).toInt());
                    insertLDCadGroup(match.captured(2),match.captured(1).toInt());
                } else if (smLine.contains("0 STEP") || smLine.contains("0 ROTSTEP")) {
                    ldcadGroupsLoaded = true;
                }
            }

            // model begin check
            if (!modelBegin && (modelBegin = smLine.contains(_fileRegExp[EOH_RX]))) {
                headerMissing = missingHeaders();
                if (headerMissing == NameMissing || headerMissing == BothMissing) {
                    subfileName = fileInfo.fileName();
                    _file = fileInfo.baseName();
                    _description = _file;
                }
            }

            split(smLine,tokens);

            // indicate submodel in display model step or substitute type
            if (tokens.size() && tokens.at(0) == "0") {
                if (displayModel)
                    displayModel = !smLine.contains(_fileRegExp[LDS_RX]); // LDraw Step Boundry
                else
                    displayModel = smLine.contains(_fileRegExp[DMS_RX]);  // Display Model Step

                if (isSubstitute(smLine,subFile))
                    subfileFound = !subFile.isEmpty();
            }
            // part type 1-5 check
            else if ((subfileFound = tokens.size() == 15)) {
                topHeaderFinished = partHeaderFinished = true;
                subFile = tokens.at(14);
            }

            // subfile, helper and lsynth part check
            if (subfileFound && ! _processedSubfiles.contains(subFile, Qt::CaseInsensitive)) {
                _processedSubfiles.append(subFile);
                PieceInfo* pieceInfo = lcGetPiecesLibrary()->FindPiece(subFile.toLatin1().constData(), nullptr, false, false);
                if (! pieceInfo && ! LDrawFile::contains(subFile, Qt::CaseInsensitive) && ! stagedSubfiles.contains(subFile, Qt::CaseInsensitive)) {
                    if (displayModel)
                        stagedSubfiles.append(QString("%1|displaymodel").arg(subFile));
                    else
                        stagedSubfiles.append(subFile);
                    // determine if missing helper or lsynth part
                    if (helperPartsNotFound || lsynthPartsNotFound) {
                        int is_support_file = ExcludedParts::isExcludedSupportPart(subFile);
                        if (is_support_file == ExcludedParts::EP_HELPER) {
                            helperPartsNotFound = false;
                            _helperPartsNotInArchive = true;
                        } else if (is_support_file == ExcludedParts::EP_LSYNTH) {
                            lsynthPartsNotFound = false;
                            _lsynthPartsNotInArchive = true;
                        }
                    }
                }
            }

            // header check
            if (!topHeaderFinished) {
                // One time populate top level file name
                if (topFileNotFound) {
                    _file = fileInfo.baseName();
                    topFileNotFound = false;
                }

                if (!sosf || !partHeaderFinished) {
                    if (hdrDescNotFound && lineIndx == hdrDescLine) {
                        if (smLine.contains(_fileRegExp[DES_RX]) && ! isHeader(smLine)) {
                            _description = _fileRegExp[DES_RX].match(smLine).captured(1);
                            if (topLevelModel)
                                Preferences::publishDescription = _description;
                            hdrDescNotFound = false;
                        } else
                            _description = !subfileName.isEmpty() ? QFileInfo(subfileName).completeBaseName() : _file;
                    }

                    if (hdrNameNotFound) {
                        if (smLine.contains(_fileRegExp[NAM_RX])) {
                            if (topLevelModel)
                                _name = _fileRegExp[NAM_RX].match(smLine).captured(1);
                            hdrNameNotFound = false;
                        } else if (smLine.contains(_fileRegExp[NAK_RX]))
                            hdrNameKey = true;
                        if (!hdrNameNotFound || hdrNameKey)
                            hdrNameLine = lineIndx;
                    }

                    if (hdrAuthorNotFound) {
                        if (smLine.contains(_fileRegExp[AUT_RX])) {
                            if (topLevelModel) {
                                _author = _fileRegExp[AUT_RX].match(smLine).captured(1);
                                Preferences::defaultAuthor = _author;
                            }
                            hdrAuthorNotFound = false;
                        } else if (smLine.contains(_fileRegExp[AUK_RX]))
                            hdrAuthorKey = true;
                        if (!hdrAuthorNotFound || hdrAuthorKey)
                            hdrAuthorLine = lineIndx;
                    }

                    // One time populate model category (not used)
                    if (hdrCategNotFound) {
                        if (smLine.contains(_fileRegExp[CAT_RX])) {
                            if (topLevelModel)
                                _category = _fileRegExp[CAT_RX].match(smLine).captured(1);
                            hdrCategNotFound = false;
                        }
                    }

                    if (unofficialPart == UNOFFICIAL_UNKNOWN) {
                        unofficialPart = getUnofficialFileType(smLine);
                        if (unofficialPart) {
                            emit gui->messageSig(LOG_TRACE, QObject::tr("Subfile '%1' spcified as %2.")
                                                                        .arg(subfileName, fileType()));
                        }
                    }

                    // Check for include file
                    if (smLine.contains(_fileRegExp[INC_RX])) {
                        const QString inclFilePath = LPub::getFilePath(_fileRegExp[INC_RX].match(smLine).captured(1));
                        QFileInfo inclFileInfo(inclFilePath);
                        if (inclFileInfo.isReadable()) {
                            if (loadIncludeFile(inclFilePath)) {
                                const QString statusEntry = QObject::tr("%1|%2|LDR Include file %2 (file: %3, line: %4)")
                                                                .arg(INCLUDE_FILE_LOAD_MSG).arg(inclFileInfo.fileName(), subfileName).arg(lineIndx + 1);
                                loadStatusEntry(INCLUDE_FILE_LOAD_MSG, statusEntry, inclFileInfo.fileName(), QObject::tr("Subfile [%1] is a LDR Include file"));
                            }
                        } else {
                            const QString message = QObject::tr("LDR Include file '%1' was not found (file: %2, line: %3).")
                                                        .arg(inclFilePath, fileInfo.fileName()).arg(lineIndx + 1);
                            const QString statusEntry = QObject::tr("%1|%2|LDR Include file '%2' was not found (subfile: %3, line: %4).")
                                                            .arg(BAD_INCLUDE_LOAD_MSG).arg(inclFileInfo.fileName(), subfileName).arg(lineIndx + 1);
                            loadStatusEntry(BAD_INCLUDE_LOAD_MSG, statusEntry, inclFileInfo.fileName(), message);
                        }
                    }

                    // Check meta commands
                    if (!isComment(smLine))
                        processMetaCommand(tokens);
                }
            } // topHeaderFinished

            if (subfileName.isEmpty() && hdrNameNotFound && (topLevelModel || !smLine.isEmpty()) && !sosf) {
                if (hdrAuthorNotFound)
                    if (hdrDescNotFound && !hdrNameKey && !hdrAuthorKey)
                        hdrDescLine = lineIndx;
                contents << smLine;
            }

            if ((alreadyLoaded = LDrawFile::contains(subfileName))) {
                emit gui->messageSig(LOG_TRACE, QObject::tr("LDR %1 '%2' already loaded.").arg(fileType(), subfileName));
                subfileIndx = stagedSubfiles.indexOf(subfileName);
                if (subfileIndx > NOT_FOUND)
                    stagedSubfiles.removeAt(subfileIndx);
            }

            // processing inlined parts
            if (!topLevelModel && !hdrNameNotFound) {
                if (hdrAuthorNotFound) {
                    if (smLine.contains(_fileRegExp[AUT_RX]))
                        hdrAuthorNotFound = false;
                }
                if (!partHeaderFinished && unofficialPart == UNOFFICIAL_UNKNOWN) {
                    unofficialPart = getUnofficialFileType(smLine);
                    if (unofficialPart) {
                        emit gui->messageSig(LOG_TRACE, QObject::tr("Inline file '%1' spcified as %2.")
                                                                    .arg(subfileName, fileType()));
                    }
                }
            }

            /* - if at start of file marker, populate subfileName
             * - if at end of file marker, clear subfileName
             */
            if (sosf || eosf) {
                /* - if at end of file marker
                 * - insert items if subfileName not empty
                 * - after insert, clear contents
                 */
                if (! subfileName.isEmpty()) {
                    if (! alreadyLoaded) {
                        if (contents.size()) {
                            insert(subfileName,
                                   contents,
                                   datetime,
                                   unofficialPart,
                                   displayModel,
                                   false/*generated*/,
                                   false/*includeFile*/,
                                   false/*dataFile*/,
                                   externalFile ? fileInfo.absoluteFilePath() : "",
                                   _description);

                            if ((headerMissing = MissingHeaderType(missingHeaders())))
                                normalizeHeader(subfileName, fileInfo.fileName(), headerMissing);
                            emit gui->messageSig(LOG_NOTICE, QObject::tr("MPD %1 '%2' with %3 lines loaded.")
                                                                         .arg(fileType(), subfileName, QString::number(size(subfileName))));
                        }
                        topLevelModel = false;
                        unofficialPart = UNOFFICIAL_UNKNOWN;
                    }
                    subfileIndx = stagedSubfiles.indexOf(subfileName);
                    if (subfileIndx > NOT_FOUND)
                        stagedSubfiles.removeAt(subfileIndx);

                    contents.clear();
                }

                /* - if at start of file marker
                 * - set subfileName of new file
                 * - else if at end of file marker, clear subfileName
                 */
                if (sosf) {
                    unofficialPart  = UNOFFICIAL_UNKNOWN;
                    hdrNameNotFound = sosf = false;
                    partHeaderFinished = subfileFound ? true : false;
                    subfileName = _fileRegExp[NAM_RX].match(smLine).captured(1);
                    contents << smLine;
                    if (! alreadyLoaded)
                        emit gui->messageSig(LOG_INFO_STATUS, QObject::tr("Loading '%1'...").arg(subfileName));
                } else if (eosf) {
                    /* - at the end of inline part
                    */
                    subfileName.clear();
                    hdrNameNotFound   = true; /* reset Name capture*/
                    hdrAuthorNotFound = true; /* reset Author capture*/
                    hdrDescNotFound   = true; /* reset Description capture*/
                    hdrNameKey        = false;
                    hdrAuthorKey      = false;
                    hdrDescLine       = 0;
                    hdrNameLine       = 0;
                    hdrAuthorLine     = 0;
                    /* - if description found, add it to the start of the part's contents
                    */
                    if (!_description.isEmpty())
                        contents << _description;
                    /* - clear description for next part
                    */
                    _description.clear();
                    /* - at the Name: of a new inline part so revert by 1 line to capture
                    */
                    lineIndx--;
                    eosf = false;
                }
            } else if (!subfileName.isEmpty() && !smLine.isEmpty()) {
                /* - after start of file - subfileName not empty
                 * - add line to contents
                 */
                contents << smLine;
            }
        } // iterate stagedContents

        // at end of file
        if ( ! subfileName.isEmpty() && ! contents.isEmpty()) {
            if (LDrawFile::contains(subfileName)) {
                emit gui->messageSig(LOG_TRACE, QObject::tr("LDR %1 '%2' already loaded.").arg(fileType(), subfileName));
            } else {
                if (contents.isEmpty()) {
                    const QString message = QObject::tr("LDR %1 '%2' is empty and was not loaded (file: %3, line: %4).")
                                                        .arg(fileType(), subfileName, fileInfo.fileName()).arg(lineIndx + 1);
                    const QString statusEntry = QObject::tr("%1|%2|%3").arg(EMPTY_SUBMODEL_LOAD_MSG).arg(subfileName, message);
                    loadStatusEntry(EMPTY_SUBMODEL_LOAD_MSG, statusEntry, subfileName, message);
                } else {
                    insert(subfileName,
                           contents,
                           datetime,
                           unofficialPart,
                           displayModel,
                           false/*generated*/,
                           false/*includeFile*/,
                           false/*dataFile*/,
                           externalFile ? fileInfo.absoluteFilePath() : "",
                           _description);
                    if ((headerMissing = MissingHeaderType(missingHeaders())))
                        normalizeHeader(subfileName, fileInfo.fileName(), headerMissing);
                    emit gui->messageSig(LOG_NOTICE, QObject::tr("LDR  %1 '%2' with %3 lines loaded.")
                                                                 .arg(fileType(), subfileName, QString::number(size(subfileName))));
                }
            }

            contents.clear();

            subfileIndx = stagedSubfiles.indexOf(subfileName);
            if (subfileIndx > NOT_FOUND)
                stagedSubfiles.removeAt(subfileIndx);

            displayModel = false;
        } // at end of file

        // resolve outstanding subfiles
        if (stagedSubfiles.size()) {
            stagedSubfiles.removeDuplicates();

#ifdef QT_DEBUG_MODE
            emit gui->messageSig(LOG_DEBUG, QString("%1 unresolved staged %2 specified.")
                                                    .arg(stagedSubfiles.size()).arg(stagedSubfiles.size() == 1 ? "subfile" : "subfiles"));
#endif
            QString const projectPath = QDir::toNativeSeparators(fileInfo.absolutePath());
            QString subfile, fileDesc;
            for (const QString &stagedFile : stagedSubfiles) {
                displayModel = false;
                if (stagedFile.count("|")) {
                    QStringList elements = stagedFile.split("|");
                    subfile = elements.first();
                    if ((displayModel = elements.last().toLower() == QLatin1String("displaymodel")))
                        fileDesc = QObject::tr("(display model) ");
                } else {
                    subfile = stagedFile;
                }
#ifdef QT_DEBUG_MODE
                emit gui->messageSig(LOG_DEBUG, QString("Processing staged %1subfile '%2'...")
                                                        .arg(fileDesc, subfile));
#endif
                if (LDrawFile::contains(subfile)) {
                    if (displayModel)
                        setDisplayModel(subfile);
                    QString const subfileType = fileType(isUnofficialPart(subfile));
                    emit gui->messageSig(LOG_TRACE, QObject::tr("LDR %1%2 '%3' already loaded.").arg(fileDesc, subfileType, subfile));
                    continue;
                }
                // subfile path
                if ((subfileFound = QFileInfo(subfile).isFile())) {
                    fileInfo = QFileInfo(subfile);
                } else if (Preferences::extendedSubfileSearch) {
                    // extended search - current project path
                    if (Preferences::searchProjectPath && (subfileFound = QFileInfo(projectPath + QDir::separator() + subfile).isFile())) {
                        fileInfo = QFileInfo(projectPath + QDir::separator() + subfile);
                    }
                    if (!subfileFound) {
                        // extended search - LDraw subfolder paths and extra search directories
                        for (QString &subFilePath : searchPaths) {
                            if ((subfileFound = QFileInfo(subFilePath + QDir::separator() + subfile).isFile())) {
                                fileInfo = QFileInfo(subFilePath + QDir::separator() + subfile);
                                break;
                            }
                        }
                    }
                }

                bool externalSubfile = true;
                if (!subfileFound) {
                    QString partFile = subfile.toUpper();
                    if (partFile.startsWith("S\\"))
                        partFile.replace("S\\","S/");
                    PieceInfo* pieceInfo = lcGetPiecesLibrary()->FindPiece(partFile.toLatin1().constData(), nullptr/*CurrentProject*/, false/*CreatePlaceholder*/, false/*SearchProjectFolder*/);
                    if ((subfileFound = pieceInfo) && pieceInfo->IsPartType()) {
                        emit gui->messageSig(LOG_NOTICE, QObject::tr("Staged %1subfile '%2' is a part.").arg(fileDesc, subfile));
                    } else
                    if ((subfileFound = lcGetPiecesLibrary()->IsPrimitive(partFile.toLatin1().constData()))) {
                        lcLibraryPrimitive* Primitive = lcGetPiecesLibrary()->FindPrimitive(partFile.toLatin1().constData());
                        if (Primitive) {
                            emit gui->messageSig(LOG_NOTICE, QObject::tr("Staged %1subfile '%2' is a %3.")
                                                                         .arg(fileDesc, subfile, Primitive->mSubFile ? QObject::tr("subpart") : QObject::tr("primitive")));
                        }
                    }
                    externalSubfile = !subfileFound;
                    Q_UNUSED(externalSubfile)
                }

                if (!subfileFound) {
                    emit gui->messageSig(LOG_WARNING, QObject::tr("Staged %1subfile '%2' not found.").arg(fileDesc, subfile));
                } else {
                    bool savedTopLevelModel  = topLevelModel;
                    bool savedUnofficialPart = unofficialPart;
                    topLevelModel            = false;

                    emit gui->messageSig(LOG_NOTICE, QObject::tr("External %1subfile '%2' found.").arg(fileDesc, subfile));

                    loadLDRFile(fileInfo.absoluteFilePath(), QString(), true/*external*/);

                    subfileIndx = stagedSubfiles.indexOf(subfile);
                    if (subfileIndx > NOT_FOUND)
                        stagedSubfiles.removeAt(subfileIndx);

                    topLevelModel  = savedTopLevelModel;
                    unofficialPart = savedUnofficialPart;
                }

                fileDesc.clear();
            }
        } // resolve outstanding subfiles

        subfileName = QFileInfo(fullName).fileName();

#ifdef QT_DEBUG_MODE
        if (!stagedSubfiles.size())
            emit gui->messageSig(LOG_DEBUG, QString("All '%1' staged subfiles processed.").arg(subfileName));

#if QT_VERSION >= QT_VERSION_CHECK(6,5,0)
        QMultiHashIterator<QString, int> i(_ldcadGroups);
#else
        QHashIterator<QString, int> i(_ldcadGroups);
#endif
        while (i.hasNext()) {
            i.next();
            emit gui->messageSig(LOG_TRACE, QString("LDCad Groups: Name[%1], LineID[%2].")
                                 .arg(i.key()).arg(i.value()));
        }
#endif

        emit gui->progressBarPermSetValueSig(lineCount);
        if (!stagedSubfiles.size()) {
            emit gui->progressLabelPermSetTextSig(QString());
            emit gui->progressPermStatusRemoveSig();
        }

        emit gui->messageSig(LOG_NOTICE, QObject::tr("LDR file '%1' with %2 lines loaded.")
                                                     .arg(subfileName).arg(lineCount));
    }
}

bool LDrawFile::saveFile(const QString &fileName)
{
    _savedLines = 0;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool rc;
    if (isIncludeFile(fileName)) {
      rc = saveIncludeFile(fileName);
    } else {
      rc = saveModelFile(fileName);
    }
    QApplication::restoreOverrideCursor();
    return rc;
}

bool LDrawFile::mirrored(
  const QStringList &tokens)
{
  if (tokens.size() != 15) {
    return false;
  }
  /* 5  6  7
     8  9 10
    11 12 13 */

  float a = tokens[5].toFloat();
  float b = tokens[6].toFloat();
  float c = tokens[7].toFloat();
  float d = tokens[8].toFloat();
  float e = tokens[9].toFloat();
  float f = tokens[10].toFloat();
  float g = tokens[11].toFloat();
  float h = tokens[12].toFloat();
  float i = tokens[13].toFloat();

  float a1 = a*(e*i - f*h);
  float a2 = b*(d*i - f*g);
  float a3 = c*(d*h - e*g);

  float det = (a1 - a2 + a3);

  return det < 0;
  //return a*(e*i - f*h) - b*(d*i - f*g) + c*(d*h - e*g) < 0;
}

void LDrawFile::addCustomColorParts(const QString &mcFileName,bool autoAdd)
{
  if (!Preferences::enableFadeSteps && !Preferences::enableHighlightStep)
      return;
 
  emit gui->messageSig(LOG_INFO_STATUS, QObject::tr("Loading Custom Colour Parts..."));
  
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(fileName);
  if (f != _subFiles.end()) {
    // get content size
    int j = f->_contents.size();
    // process submodel content...
    for (int i = 0; i < j; i++) {
      QStringList tokens;
      QString line = f->_contents[i];
      split(line,tokens);
      // we interrogate substitue files
      if (tokens.size() >= 6 &&
          tokens[0] == "0" &&
         (tokens[1] == "!LPUB" || tokens[1] == "LPUB") &&
          tokens[2] == "PLI" &&
          tokens[3] == "BEGIN" &&
          tokens[4] == "SUB") {
        // do we have a color file ?
        if (LDrawColourParts::isLDrawColourPart(tokens[5])) {
          // parse the part lines for custom sub parts
          for (++i; i < j; i++) {
            split(f->_contents[i],tokens);
            if (tokens.size() == 15 && tokens[0] == "1") {
              if (contains(tokens[14])) {
                // we have custom part so let's add it to the custom part list
                LDrawColourParts::addLDrawColorPart(tokens[14]);
                // now lets check if the custom part has any sub parts
                addCustomColorParts(tokens[14],true/*autoadd*/);
              }
            } else if (tokens.size() == 4 &&
                       tokens[0] == "0" &&
                      (tokens[1] == "!LPUB" || tokens[1] == "LPUB") &&
                      (tokens[2] == "PLI" || tokens[2] == "PART") &&
                       tokens[3] == "END") {
              break;
            }
          }
        }
      } else if (autoAdd) {
        // parse the the add part lines for custom sub parts
        for (; i < j; i++) {
          split(f->_contents[i],tokens);
          if (tokens.size() == 15 && tokens[0] == "1") {
            if (contains(tokens[14])) {
              // do we have a color part ?
              if (tokens[1] != LDRAW_MAIN_MATERIAL_COLOUR &&
                  tokens[1] != LDRAW_EDGE_MATERIAL_COLOUR)
                LDrawColourParts::addLDrawColorPart(tokens[14]);
              // now lets check if the part has any sub parts
              addCustomColorParts(tokens[14],true/*autoadd*/);
            }
          } else if (f->_contents[i] == "0 //Segments") {
            // we have reached the custpm part segments so break
            break;
          }
        }
      } else if (tokens.size() == 15 && tokens[0] == "1") {
        bool containsSubFile = contains(tokens[14]);
        if (containsSubFile) {
          addCustomColorParts(tokens[14],false/*autoadd*/);
        }
      }
    }
  }
}

void LDrawFile::countInstances(
  const QString &mcFileName,
        bool     firstStep,
        bool     isMirrored,
        bool     callout)
{
  QMutexLocker ldrawLocker(&ldrawMutex);

  //logTrace() << QString("countInstances, File: %1, Mirrored: %2, Callout: %3").arg(mcFileName,(isMirrored?"Yes":"No"),(callout?"Yes":"No"));

  /*
   * For countInstances, the BuildMod behaviour creates a sequential
   * list (Vector<int>) of all the steps in the loaded model file.
   * Step indices are appended to the _buildModStepIndexes register.
   * Each step index contains the step's parent model and the line number
   * of the STEP meta command indicating the top of the 'next' step.
   * The buildModLevel flag uses getLevel() function to determine the current
   * BuildMod when mods are nested.
   */
  Where top(mcFileName, getSubmodelIndex(mcFileName), 0);

  gui->skipHeader(top);

  Where topOfStep(top); // set after skipHeader

  setBuildModsCount(topOfStep.modelName, BM_INIT);

  bool partsAdded        = false;
  bool noStep            = false;
  bool isInsertStep      = false;
  bool stepIgnore        = false;
  bool buildModIgnore    = false;
  int  buildModState     = BM_NONE;
  int  buildModStepIndex = BM_NONE;
  int  buildModPartCount = 0;
  int  buildModAction    = BuildModNoActionRc;

  QMap<int, QString>      buildModKeys;
  QMap<int, QVector<int>> buildModAttributes;

  auto loadBuildMods = [this] ()
  {
      int stepIndexes = _buildModStepIndexes.size();

      bool result =
              _loadBuildMods && (Gui::suspendFileDisplay ||
              (stepIndexes >= _buildModPrevStepIndex &&
               stepIndexes <= _buildModNextStepIndex));

      return result;
  };

  auto insertAttribute = [this, &top] (QMap<int, QVector<int>> &buildModAttributes, int indx)
  {
      QMap<int, QVector<int>>::iterator i = buildModAttributes.find(buildModLevel);
      if (i == buildModAttributes.end()) {
          QVector<int> modAttributes = { 0, 0, 0, 1, 0, top.modelIndex, 0, 0 };
          modAttributes[indx] = top.lineNumber;
          buildModAttributes.insert(buildModLevel, modAttributes);
      } else {
          i.value()[indx] = top.lineNumber;
      }
  };

  auto insertBuildModification = [this, &buildModPartCount, &buildModAttributes, &buildModKeys, &topOfStep] (int level)
  {
      int buildModStepIndex = _buildModStepIndexes.indexOf({topOfStep.modelIndex, topOfStep.lineNumber});

      QString buildModKey = buildModKeys.value(level);
      QVector<int> modAttributes = { 0, 0, 0, 0, 0, -1, 0, 0 };

      QMap<int, QVector<int>>::iterator i = buildModAttributes.find(level);
      if (i == buildModAttributes.end()) {
          emit gui->messageSig(LOG_ERROR, QString("COUNTINSTANCE - WARNING - Invalid BuildMod Entry for key: %1").arg(buildModKey));
          return;
      }
      modAttributes = i.value();

      modAttributes[BM_DISPLAY_PAGE_NUM] = Gui::displayPageNum;
      modAttributes[BM_STEP_PIECES]      = buildModPartCount;
      modAttributes[BM_MODEL_NAME_INDEX] = topOfStep.modelIndex;
      modAttributes[BM_MODEL_LINE_NUM]   = topOfStep.lineNumber;
//*
#ifdef QT_DEBUG_MODE
      if (Gui::suspendFileDisplay) {
          emit gui->messageSig(LOG_DEBUG, QString(
                               "Insert CountInst BuildMod StepIndex: %1, "
                               "Attributes: %2 %3 %4 %5 %6 %7 %8, "
                               "ModKey: '%9', "
                               "Level: %10, "
                               "Model: %11")
                               .arg(buildModStepIndex)                      // Attribute Default Initial:
                               .arg(modAttributes.at(BM_BEGIN_LINE_NUM))    // 0         0       this
                               .arg(modAttributes.at(BM_ACTION_LINE_NUM))   // 1         0       this
                               .arg(modAttributes.at(BM_END_LINE_NUM))      // 2         0       this
                               .arg(modAttributes.at(BM_DISPLAY_PAGE_NUM))  // 3         0       this
                               .arg(modAttributes.at(BM_STEP_PIECES))       // 4         0       this
                               .arg(modAttributes.at(BM_MODEL_NAME_INDEX))  // 5        -1       this
                               .arg(modAttributes.at(BM_MODEL_LINE_NUM))    // 6         0       this
                               .arg(buildModKey)
                               .arg(level)
                               .arg(topOfStep.modelName));
      }
#endif
//*/
      insertBuildMod(buildModKey,
                     modAttributes,
                     buildModStepIndex);
  };

  if (firstStep) {
    if (top.modelName.toLower() == topLevelFile().toLower()) {
      buildModLevel = BM_BASE_LEVEL;
      _currentLevels.clear();
      _buildModStepIndexes.clear();
    }

    if (! _buildModStepIndexes.contains({ top.modelIndex, top.lineNumber }))
      _buildModStepIndexes.append({ top.modelIndex, top.lineNumber });
  }

  QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(top.modelName.toLower());
  if (f != _subFiles.end()) {
    if (f->_beenCounted) {
      if (isMirrored) {
        ++f->_mirrorInstances;
      } else {
        ++f->_instances;
      }
      // children
      for (int i = 0; i < f->_subFileIndexes.size(); i++) {
        QMap<QString, LDrawSubFile>::iterator s = _subFiles.find(getSubmodelName(f->_subFileIndexes.at(i)));
        if (s != _subFiles.end()) {
          if (isMirrored) {
            ++s->_mirrorInstances;
          } else {
            ++s->_instances;
          }
        }
      }
      return;
    }

    // get content size and reset numSteps
    int j = f->_contents.size();
    f->_numSteps = 0;

    // process submodel content...
    for (; top.lineNumber < j; top.lineNumber++) {
      QStringList tokens;
      QString line = f->_contents[top.lineNumber];
      split(line,tokens);

      //lpub3d ignore part - so set ignore step
      if (tokens.size() > 1 && tokens[0] == "0") {
        if (!displayModel)
          displayModel = line.contains(_fileRegExp[DMS_RX]);
        if (tokens[1] == "!LPUB" || tokens[1] == "LPUB") {
          // pli part meta commands
          if (tokens.size() == 5) {
            if ((tokens[2] == "PART"  || tokens[2] == "PLI") &&
              tokens[3] == "BEGIN"  &&
              tokens[4] == "IGN") {
              stepIgnore = true;
            }
            // build modification - commands
            else if (!displayModel && tokens[2] == "BUILD_MOD") {
              if (tokens[3] == "BEGIN") {
                if (! Preferences::buildModEnabled) {
                    buildModIgnore = true;
                    continue;
                }
                buildModLevel = getLevel(tokens[4], BM_BEGIN);
                buildModIgnore = false;
                buildModAction = BuildModApplyRc;
                setBuildModsCount(topOfStep.modelName);
                if (loadBuildMods()) {
                  buildModKeys.insert(buildModLevel, tokens[4]);
                  insertAttribute(buildModAttributes, BM_BEGIN_LINE_NUM);
                }
                buildModState = BM_BEGIN;
              } else if (tokens[3] == "ADD" || tokens[3] == "REMOVE") {
                if (loadBuildMods()) {
                  int newAction = tokens[3] == "ADD" ? BuildModApplyRc : BuildModRemoveRc;
                  if (buildModContains(tokens[4])) {
                    buildModStepIndex = getBuildModStepIndex(topOfStep.modelIndex, topOfStep.lineNumber);
                    if (buildModStepIndex > BM_INVALID_INDEX) {
                      buildModAction = getBuildModAction(tokens[4], buildModStepIndex);
                      if (buildModAction && (buildModAction != newAction)) {
                        setBuildModAction(tokens[4], buildModStepIndex, newAction);
                      }
                    }
                  } else {
                    emit gui->messageSig(LOG_TRACE, QString("COUNTINSTANCE - WARNING - BuildMod for key '%1' not found.").arg(tokens[4]));
                  }
                }
              }
            } // build modification commands
          // we have a meta command...
          } else if (tokens.size() == 4) {
            // lpub3d part - so set include step
            if ((tokens[2] == "PART" || tokens[2] == "PLI") &&
                 tokens[3] == "END") {
              stepIgnore = false;
            // called out
            /* Sorry, but models that are callouts are not counted as instances */
            } else if (tokens[2] == "CALLOUT" &&
                       tokens[3] == "BEGIN") {
              callout    = true;
              partsAdded = true;
              //process callout content
              for (++top.lineNumber; top.lineNumber < j; top.lineNumber++) {
                split(f->_contents[top.lineNumber],tokens);
                if (tokens.size() == 15 && tokens[0] == "1") {
                  // exclude unofficial inline files
                  if (contains(tokens[14],false/*searchAll*/) && ! stepIgnore && ! buildModIgnore) {
                    // add contains 'child' index to parent list
                    if (isSubmodel(tokens[14])) {
                      const int subFileIndex = getSubmodelIndex(tokens[14]);
                      if (top.modelIndex && !f->_subFileIndexes.contains(subFileIndex))
                        f->_subFileIndexes.append(subFileIndex);
                    }
                    countInstances(tokens[14], true/*firstStep*/, mirrored(tokens), callout);
                  }
                } else if (tokens.size() == 4 && tokens[0] == "0" &&
                          (tokens[1] == "!LPUB" || tokens[1] == "LPUB") &&
                           tokens[2] == "CALLOUT" &&
                           tokens[3] == "END") {
                  callout = false;
                  break;
                }
              }
            // build modification - END_MOD and END commands
            } else if (tokens[2] == "BUILD_MOD") {
              if (tokens[3] == "END_MOD") {
                if (! Preferences::buildModEnabled)
                    continue;
                if (buildModAction == BuildModApplyRc)
                    buildModIgnore = true;
                if (loadBuildMods())
                  insertAttribute(buildModAttributes, BM_ACTION_LINE_NUM);
                buildModState = BM_END_MOD;
              } else if (tokens[3] == "END") {
                if (! Preferences::buildModEnabled)
                    continue;
                if (loadBuildMods() && buildModState == BM_END_MOD)
                  insertAttribute(buildModAttributes, BM_END_LINE_NUM);
                buildModLevel  = getLevel(QString(), BM_END);
                if (buildModLevel == BM_BASE_LEVEL || buildModLevel == _currentLevels.size())
                  buildModIgnore = false;
                buildModState  = BM_END;
              }
              // page insert, e.g. dispaly model, bom
            } else if (tokens[2] == "INSERT" &&
                    (tokens[3] == "PAGE" || tokens[3] == "COVER_PAGE")) {
              isInsertStep = partsAdded = true;
            } else if (tokens[1] == "BUFEXCHG") {}
          } // lines with 4 elements
        // no step
        } else if (tokens.size() == 3 &&
                  (tokens[2] == "NOSTEP" || tokens[2] == "NOFILE")) {
          noStep = true;
          displayModel = false;
        // LDraw step or rotstep - so check if parts added
        } else if (tokens.size() >= 2 &&
                  (tokens[1] == "STEP" || tokens[1] == "ROTSTEP")) {
          displayModel = false;
          if (! noStep) {
            if (partsAdded) {
              // parts added - increment step
              int incr = (((isMirrored && f->_mirrorInstances == 0) ||
                           (! isMirrored && f->_instances == 0)) && ! isInsertStep);
              f->_numSteps += incr;
            }
            // set step index for occurrences of STEP or ROTSTEP not ignored by BuildMod
            if (! buildModIgnore) {
              _buildModStepIndexes.append({ top.modelIndex, top.lineNumber });
              // build modification inserts
              const QList bmKeys = buildModKeys.keys();
              if (loadBuildMods() && bmKeys.size()) {
                for (int level : bmKeys)
                  insertBuildModification(level);
              }
            }
            buildModLevel     = BM_BASE_LEVEL;
            buildModState     = BM_NONE;
            buildModAction    = BuildModNoActionRc;
            buildModIgnore    = false;
            if (loadBuildMods()) {
              buildModPartCount = 0;
              buildModKeys.clear();
              buildModAttributes.clear();
            }
          } // not nostep
          // set top of next step
          topOfStep = top;
          // reset partsAdded, noStep and emptyLines
          isInsertStep = partsAdded = false;
          noStep = false;
        } // step or rotstep
      // check if subfile and process
      } else if (tokens.size() == 15 && tokens[0] >= "1" && tokens[0] <= "5") {
        if (! displayModel && ! stepIgnore && ! buildModIgnore) {
          buildModPartCount++;
          // exclude unofficial inline files
          if (contains(tokens[14],false/*searchAll*/)) {
            // add contains 'child' index to parent list
            if (isSubmodel(tokens[14])) {
              const int subFileIndex = getSubmodelIndex(tokens[14]);
              if (top.modelIndex && !f->_subFileIndexes.contains(subFileIndex))
                f->_subFileIndexes.append(subFileIndex);
            }
            countInstances(tokens[14], true /*firstStep*/, mirrored(tokens), callout);
          }
        }
        partsAdded = true;
      } // part line
    } // for each line

    // increment steps and add BuildMod step index if parts added in the last step of the sub/model and not ignored by BuildMod
    if (partsAdded && ! noStep) {
      int incr = (((isMirrored && f->_mirrorInstances == 0) ||
                   (! isMirrored && f->_instances == 0)) && ! isInsertStep);
      f->_numSteps += incr;
      if (! buildModIgnore) {
        _buildModStepIndexes.append({ top.modelIndex, top.lineNumber });
        // insert buildMod entries at end of content
        const QList bmKeys = buildModKeys.keys();
        if (loadBuildMods() && bmKeys.size()) {
          for (int level : bmKeys)
            insertBuildModification(level);
        }
      }
    } // partsAdded && ! noStep

    if ( ! callout) {
      if (isMirrored) {
        ++f->_mirrorInstances;
      } else {
        ++f->_instances;
      }
    } // callout

    displayModel = false;
  } // subfile end

  f->_beenCounted = true;
}

void LDrawFile::countInstances()
{
  for (int i = 0; i < _subFileOrder.size(); i++) {
    QString fileName = _subFileOrder.at(i).toLower();
    QMap<QString, LDrawSubFile>::iterator it = _subFiles.find(fileName);
    it->_instances = 0;
    it->_mirrorInstances = 0;
    it->_beenCounted = false;
  }

#ifdef QT_DEBUG_MODE
  QElapsedTimer timer;
  timer.start();
#endif

  /*
   * For countInstances, the BuildMod behaviour creates a sequential
   * list (Vector<QString>) of all the steps in the loaded model file.
   * The buildMod flag uses a multilevel (_currentLevels) framework to
   * determine the current BuildMod when mods are nested.
   */
  QFuture<void> future = QtConcurrent::run([this]() {
      countInstances(topLevelFile(),
                     true/*firstStep*/,
                     false/*isMirrored*/,
                     false/*callout*/);
  });
  asynchronous(future);

  _buildModStepIndexes.append({ 0/*SubmodelIndex*/, size(topLevelFile()) });

//*
#ifdef QT_DEBUG_MODE
  if (Gui::suspendFileDisplay) {
    emit gui->messageSig(LOG_DEBUG, QString("COUNT INSTANCES Step Indexes"));
    emit gui->messageSig(LOG_DEBUG, "----------------------------");
    for (int i = 0; i < _buildModStepIndexes.size(); i++)
    {
      const QVector<int> &key = _buildModStepIndexes.at(i);
      emit gui->messageSig(LOG_DEBUG, QString("StepIndex: %1, SubmodelIndex: %2: LineNumber: %3, ModelName: %4")
                           .arg(i, 3, 10, QChar('0'))               // index
                           .arg(key.at(0), 3, 10, QChar('0'))       // modelIndex
                           .arg(key.at(1), 3, 10, QChar('0'))       // lineNumber
                           .arg(getSubmodelName(key.at(0),false))); // modelName
    }
    emit gui->messageSig(LOG_DEBUG, QString("Count steps and submodel instances - %1")
                         .arg(Gui::elapsedTime(timer.elapsed())));
    emit gui->messageSig(LOG_DEBUG, "----------------------------");
  }
#endif
//*/
}

void LDrawFile::loadStatusEntry(const int messageType,
                                const QString &statusEntry,
                                const QString &type,
                                const QString &statusMessage,
                                bool uniqueCount)
{
  LoadMsgType msgType = static_cast<LoadMsgType>(messageType);

  bool alreadyLoaded = false, emitStatusMessage = true;

  if (_itemsLoaded.contains(type)) {
    emitStatusMessage = false;
    if (msgType < MPD_SUBMODEL_LOAD_MSG)
      alreadyLoaded = true;
  } else {
    _itemsLoaded.append(type);
  }

  if (!alreadyLoaded) {
    LogType logType = LOG_NOTICE;
    int showMessage = 0;

    if (msgType == MISSING_PART_LOAD_MSG   ||
        msgType == EMPTY_SUBMODEL_LOAD_MSG ||
        msgType == BAD_INCLUDE_LOAD_MSG    ||
        msgType == BAD_DATA_LOAD_MSG) {
      logType = LOG_WARNING;
      if (msgType == MISSING_PART_LOAD_MSG) {
        _loadIssues = static_cast<int>(SHOW_ERROR);
      } else if (_loadIssues < static_cast<int>(SHOW_ERROR)) {
        _loadIssues = static_cast<int>(SHOW_WARNING);
      }
    }

    if (uniqueCount)
      _uniquePartCount++;
    else
      _loadedItems.append(statusEntry);
    QString message;
    if (statusMessage.endsWith(" validated."))
      message = statusMessage.arg(_uniquePartCount).arg(type);
    else if (msgType != EMPTY_SUBMODEL_LOAD_MSG &&
             msgType != BAD_INCLUDE_LOAD_MSG &&
             msgType != BAD_DATA_LOAD_MSG)
      message = statusMessage.arg(type);
    else
      message = statusMessage;

    if (emitStatusMessage)
        emit gui->messageSig(logType, message, showMessage);
  }
  if (uniqueCount)
    _loadedItems.append(statusEntry);
}

void LDrawFile::countParts(const QString &fileName, bool recount) {

    _itemsLoaded.clear();
    displayModel       = false;
    bool lpubFade      = false;
    bool lpubHighlight = false;
    bool checkTexmap   = false;
    bool progressPermInit = true;

    Where top(fileName, getSubmodelIndex(fileName), 0);

    int topModelIndx  = top.modelIndex;

    QString const title = QObject::tr("%1 parts for %2...")
                                      .arg(recount
                                           ? QObject::tr("Recounting")
                                           : QObject::tr("Counting"), top.modelName);

    static QRegularExpression texmapRx("^0\\s+!?TEXMAP\\s+(?:START|NEXT)\\s+(\\b\\w+\\b)");

    LDrawUnofficialFileType subFileType;
    std::function<void(Where&)> countModelParts;
    countModelParts = [&] (Where& top)
    {
        QStringList content = contents(top.modelName);

        // get content size
        int lines = content.size();

        if (content.size()) {
            if (progressPermInit) {
                emit gui->progressPermInitSig();
                emit gui->progressBarPermSetRangeSig(1, size(top.modelName));
                emit gui->progressLabelPermSetTextSig(title);
                progressPermInit = false;
            }

            // skip the header
            gui->skipHeader(top);

            QString description = QFileInfo(top.modelName).baseName();
            LoadMsgType msgType = _mpd ? MPD_SUBMODEL_LOAD_MSG : LDR_SUBFILE_LOAD_MSG;
            QString statusEntry = QObject::tr("%1|%2|Submodel: %3 with %4 lines (file: %5, line: %6)")
                                              .arg(msgType).arg(top.modelName, description).arg(size(top.modelName)).arg(top.modelName).arg(top.lineNumber);
            loadStatusEntry(msgType, statusEntry, top.modelName, QObject::tr("Model [%1] is a SUBMODEL"));

            // initialize valid line
            bool lineIncluded  = true;

            // process submodel content...
            for (; top.lineNumber < lines; top.lineNumber++) {

                QString type;
                QStringList tokens;
                QString line = content.at(top.lineNumber);

                bool countThisLine = true;
                bool helperPart    = true/*allow helper part*/;

                if (top.modelIndex == topModelIndx)
                    emit gui->progressBarPermSetValueSig(top.lineNumber);

                // adjust ghost lines
                if (line.startsWith("0 GHOST "))
                    line = line.mid(8).trimmed();

                split(line,tokens);

                // meta command parse
                if (tokens.size() > 1 && tokens[0] == "0") {
                    if (displayModel) {
                        if (!(displayModel = !line.contains(_fileRegExp[LDS_RX]))) // LDraw Step Boundry
                            continue;
                    }
                    if (tokens[1] == "!LPUB" || tokens[1] == "LPUB") {
                        if (tokens.size() >= 4) {
                            if (!displayModel) {
                                if ((displayModel =  line.contains(_fileRegExp[DMS_RX]))) // Display Model
                                    continue;
                            }
                            if (!_lpubFadeHighlight) {
                                if ((_lpubFadeHighlight = line.contains(_fileRegExp[LFH_RX]))) { // LPub Fade or LPub Highlight
                                    if (_fileRegExp[LFH_RX].match(line).captured(1) == "LPUB_FADE")
                                        lpubFade = true;
                                    else
                                        lpubHighlight = true;
                                }
                                continue;
                            }
                            if (_lpubFadeHighlight) {
                                if (line.contains(_fileRegExp[FHE_RX])) { // Fade or Highlight Enabled (or Setup)
                                    emit gui->enableLPubFadeOrHighlightSig(lpubFade,lpubHighlight,true/*wait for finish*/);
                                    continue;
                                }
                            }
                            // build modification - starts at BEGIN command and ends at END_MOD action
                            if (tokens[2] == "BUILD_MOD") {
                                if (tokens[3] == "BEGIN") {
                                    buildModLevel = getLevel(tokens[4], BM_BEGIN);
                                    _buildModDetected = true;
                                } else if (tokens[3] == "END_MOD") {
                                    buildModLevel = getLevel(QString(), BM_END);
                                }
                                lineIncluded = ! buildModLevel;
                            }
                        }
                        // ignore parts begin
                        if (tokens.size() == 5 && tokens[0] == "0" &&
                           (tokens[2] == "PART" || tokens[2] == "PLI") &&
                            tokens[3] == "BEGIN"  &&
                            tokens[4] == "IGN") {
                            lineIncluded = false;
                        } else
                        // ignore part end
                        if (tokens.size() == 4 &&
                           (tokens[2] == "PART" || tokens[2] == "PLI") &&
                           tokens[3] == "END") {
                            lineIncluded = true;
                        }
                    } else
                    if (checkTexmap) {
                        if (line.contains(texmapRx)) {
                            QStringList argv = line.split(" ");
                            int t = 13; // PLANAR
                            QString const method = texmapRx.match(line).captured(1).toUpper();
                            if (method == "CYLINDRICAL")
                                t += 1;
                            else if (method == "SPHERICAL")
                                t += 2;
                            for (int i = t; i < argv.size(); i++)
                                type += (argv[i]+" ");
                            type = type.trimmed();
                        }
                    }
                } // meta command lines

                if ((countThisLine = tokens.size() == 15 && tokens[0] == "1")) {
                    type = tokens[14];
                } else if (checkTexmap && !type.isEmpty()) {
                    countThisLine = true;
                } else if (isSubstitute(line, type)) {
                    countThisLine = !type.isEmpty();
                }

                if (countThisLine && lineIncluded && !ExcludedParts::isExcludedPart(type, helperPart)) {

                    if (contains(type)) {
                        subFileType = LDrawUnofficialFileType(isUnofficialPart(type));
                        if (subFileType == UNOFFICIAL_SUBMODEL) {
                            Where top(type, getSubmodelIndex(type), 0);
                            countModelParts(top);
                        } else {
                            QString partFile = type.toUpper();
                            if (partFile.startsWith("S\\"))
                                partFile.replace("S\\","S/");
                            PieceInfo* pieceInfo = lcGetPiecesLibrary()->FindPiece(partFile.toLatin1().constData(), nullptr/*CurrentProject*/, false/*CreatePlaceholder*/, false/*SearchProjectFolder*/);
                            if (pieceInfo && pieceInfo->IsPartType()) {
                                description = pieceInfo->m_strDescription;
                            } else
                            if (lcGetPiecesLibrary()->IsPrimitive(partFile.toLatin1().constData())) {
                                lcLibraryPrimitive* Primitive = lcGetPiecesLibrary()->FindPrimitive(partFile.toLatin1().constData());
                                description = Primitive->mName;
                            }
                            QString statusDesc;
                            bool inMissingItems = false;
                            switch(subFileType) {
                            case UNOFFICIAL_PART:
                            case UNOFFICIAL_SHORTCUT:
                            case UNOFFICIAL_GENERATED_PART:
                                if (helperPart || displayModel) {
                                    if (helperPart)
                                        _helperPartCount++;
                                    _displayModelPartCount++;
                                } else {
                                    _partCount++;
                                }
                                inMissingItems = isMissingItem(type);
                                statusDesc  = QObject::tr("Unofficial Inline%1 - %2 (file: %3, line: %4)")
                                                          .arg(subFileType == UNOFFICIAL_GENERATED_PART
                                                          ? QObject::tr(" LDCad Generated Part")
                                                          : helperPart
                                                                ? QObject::tr(" Helper Part")
                                                                : " Part",
                                                            description, top.modelName).arg(top.lineNumber);
                                statusEntry = QObject::tr("%1|%2|%3")
                                                          .arg(subFileType == UNOFFICIAL_GENERATED_PART
                                                          ? INLINE_GENERATED_PART_LOAD_MSG
                                                          : helperPart
                                                                 ? HELPER_PART_LOAD_MSG
                                                                 : INLINE_PART_LOAD_MSG)
                                                          .arg(type, statusDesc);
                                loadStatusEntry(INLINE_PART_LOAD_MSG, statusEntry, type, subFileType == UNOFFICIAL_GENERATED_PART
                                                                                             ? QObject::tr("Part %1 is an LDCad Generated INLINE PART.")
                                                                                             : helperPart
                                                                                                   ? QObject::tr("Part %1 is a Helper INLINE PART.")
                                                                                                   : QObject::tr("Part %1 is an INLINE PART."));
                                statusEntry = QObject::tr("%1|%2|%3").arg(VALID_LOAD_MSG).arg(type, statusDesc);
                                loadStatusEntry(VALID_LOAD_MSG, statusEntry, type, QObject::tr("Part %1 [Inline %2] validated."),true/*unique count*/);
                                if (subFileType == UNOFFICIAL_PART) {
                                    checkTexmap = true;
                                    Where top(type, getSubmodelIndex(type), 0);
                                    countModelParts(top);
                                }
                                break;
                            case UNOFFICIAL_SUBPART:
                                inMissingItems = isMissingItem(type);
                                statusEntry = QObject::tr("%1|%2|Unofficial Inline Subpart - %3 (file: %4, line: %5)")
                                                          .arg(INLINE_SUBPART_LOAD_MSG).arg(type, description, top.modelName).arg(top.lineNumber);
                                loadStatusEntry(INLINE_SUBPART_LOAD_MSG, statusEntry, type, QObject::tr("Part [%1] is an INLINE SUBPART"));
                                break;
                            /* Add these primitives into the load status dialogue because they are loaded in the LDrawFile.subfiles */
                            case UNOFFICIAL_PRIMITIVE:
                                inMissingItems = isMissingItem(type);
                                statusEntry = QObject::tr("%1|%2|Unofficial Inline Primitive - %3 (file: %4, line: %5)")
                                                          .arg(INLINE_PRIMITIVE_LOAD_MSG).arg(type, description, top.modelName).arg(top.lineNumber);
                                loadStatusEntry(INLINE_PRIMITIVE_LOAD_MSG, statusEntry, type, QObject::tr("Part [%1] is an INLINE PRIMITIVE"));
                                break;
                            case UNOFFICIAL_DATA:
                                description = QString("Base 64 data file");
                                inMissingItems = isMissingItem(type);
                                statusEntry = QObject::tr("%1|%2|Unofficial Inline Data - %3 (file: %4, line: %5)")
                                                          .arg(INLINE_DATA_LOAD_MSG).arg(type, description, top.modelName).arg(top.lineNumber);
                                loadStatusEntry(INLINE_DATA_LOAD_MSG, statusEntry, type, QObject::tr("Part [%1] is an INLINE DATA"));
                                break;
                            default:
                                break;
                            }
                            if (inMissingItems)
                                removeMissingItem(type);
                        }
                    } else {
                        QString partFile = type.toUpper();
                        if (partFile.startsWith("S\\"))
                            partFile.replace("S\\","S/");
                        PieceInfo* pieceInfo = lcGetPiecesLibrary()->FindPiece(partFile.toLatin1().constData(), nullptr/*CurrentProject*/, false/*CreatePlaceholder*/, false/*SearchProjectFolder*/);
                        if (pieceInfo && pieceInfo->IsPartType()) {
                            if (helperPart || displayModel) {
                                if (helperPart) {
                                    statusEntry = QObject::tr("%1|%2|%3 (file: %4, line: %5)")
                                                      .arg(HELPER_PART_LOAD_MSG)
                                                      .arg(type, pieceInfo->m_strDescription)
                                                      .arg(top.modelName).arg(top.lineNumber);
                                    loadStatusEntry(HELPER_PART_LOAD_MSG, statusEntry, type, QObject::tr("Part %1 is a Helper PART."));
                                    _helperPartCount++;
                                }
                                _displayModelPartCount++;
                            } else {
                                _partCount++;
                            }
                            statusEntry = QObject::tr("%1|%2|%3 (file: %4, line: %5)")
                                                      .arg(VALID_LOAD_MSG).arg(type, pieceInfo->m_strDescription, top.modelName).arg(top.lineNumber);
                            loadStatusEntry(VALID_LOAD_MSG, statusEntry, type, QObject::tr("Part %1 [%2] validated."),true/*unique count*/);
                        } else
                        if (lcGetPiecesLibrary()->IsPrimitive(partFile.toLatin1().constData())) {
                            continue;
                            /* Do not add these into the load status dialogue because they are not loaded in the LDrawFile.subfiles
                            lcLibraryPrimitive* Primitive = lcGetPiecesLibrary()->FindPrimitive(partFile.toLatin1().constData());
                            if (Primitive) {
                                const QString description = Primitive->mName;
                                if (Primitive->mSubFile) {
                                    statusEntry = QObject::tr("%1|%2|%3 (file: %4, line: %5)")
                                                              .arg(SUBPART_LOAD_MSG).arg(type).arg(description).arg(top.modelName).arg(top.lineNumber);
                                    loadStatusEntry(SUBPART_LOAD_MSG, statusEntry, type, QObject::tr("Part [%1] is a SUBPART"));
                                } else {
                                    statusEntry = QObject::tr("%1|%2|%3 (file: %4, line: %5)")
                                                              .arg(PRIMITIVE_LOAD_MSG).arg(type).arg(description).arg(top.modelName).arg(top.lineNumber);
                                    loadStatusEntry(PRIMITIVE_LOAD_MSG, statusEntry, type, QObject::tr("Part [%1] is a PRIMITIVE"));
                                }
                            }*/
                        } else {
                            const QString message = QObject::tr("Part [%1] was not found!");
                            statusEntry = QObject::tr("%1|%2|Part not found! [%3] (file: %4, line: %5)")
                                                      .arg(MISSING_PART_LOAD_MSG).arg(type, line, top.modelName).arg(top.lineNumber);
                            loadStatusEntry(MISSING_PART_LOAD_MSG, statusEntry, type, message);
                            if (!isMissingItem(type))
                                insertMissingItem(QStringList() << type << statusEntry);
                        }
                    }  // check archive
                } // countThisLine && lineIncluded && partIncluded
            } // process submodel content
        } // content size
    };

    countModelParts(top);

    emit gui->progressBarPermSetValueSig(size(top.modelName));
    emit gui->progressLabelPermSetTextSig(QString());
    emit gui->progressPermStatusRemoveSig();
}

void LDrawFile::recountParts()
{
    if (!_loadedItems.size())
        return;

    QElapsedTimer t; t.start();

    int loadedItems = _loadedItems.size();
    for (int i = loadedItems - 1; i >= 0; i--) {
        QString const &item = _loadedItems.at(i);
        LoadMsgType mt =
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
        static_cast<LoadMsgType>(QStringView{item}.left(item.indexOf('|')).toInt());
#else
        static_cast<LoadMsgType>(item.leftRef(item.indexOf('|')).toInt());
#endif
        if (mt <= MISSING_PART_LOAD_MSG)
            _loadedItems.removeAt(i);
    }

    int save_loadIssues = _loadIssues;
    _loadIssues = 0;
    _partCount  = 0;
    _uniquePartCount = 0;
    _helperPartCount = 0;
    _displayModelPartCount = 0;

    countParts(topLevelFile(), true/*recount*/);

    if (_loadIssues < static_cast<int>(SHOW_ERROR))
        _loadIssues = save_loadIssues;

    const QString recountMessage = QObject::tr("Recount LDraw parts for %1 model file '%2'. Unique %3 %4. Total %5 %6. %7")
                                    .arg(isMpd() ? "MPD" : "LDR",
                                         _modelFile,
                                         _uniquePartCount == 1 ? QObject::tr("Part") : QObject::tr("Parts"))
                                    .arg(_uniquePartCount)
                                    .arg(_partCount == 1 ? QObject::tr("Part") : QObject::tr("Parts"))
                                    .arg(_partCount)
                                    .arg(Gui::elapsedTime(t.elapsed()));
    emit gui->messageSig(LOG_INFO_STATUS, recountMessage);
}

bool LDrawFile::saveModelFile(const QString &fileName)
{
    QString writeFileName = QDir::toNativeSeparators(fileName);
    QFile file(writeFileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        emit gui->messageSig(LOG_ERROR,QString("Cannot write %1 file %2:<br>%3.")
                             .arg(_mpd ? "MPD" : "LDR",
                                  writeFileName,
                                  file.errorString()));
        return false;
    }

    emit gui->messageSig(LOG_INFO,QString("Saving %1 file %2.")
                         .arg(_mpd ? "MPD" : "LDR", writeFileName));

    Gui::suspendFileDisplay = true;
    gui->deleteFinalModelStep(true);

    bool newLineIinserted = false;

    QTextStream out(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    out.setEncoding(_currFileIsUTF8 ? QStringConverter::Utf8 : QStringConverter::System);
#else
    out.setCodec(_currFileIsUTF8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForName("System"));
#endif

    for (int i = 0; i < _subFileOrder.size(); i++) {

        bool addFILEMeta = false;
        bool afterFILEMeta = false;
        bool omitNOFIlEMeta = false;
        bool isModelHeader = true;
        bool isLDCadContent = false;

        QString subFileName = _subFileOrder.at(i);
        QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(subFileName.toLower());

        if (f != _subFiles.end() && ! f.value()._generated) {

            addFILEMeta    = _mpd && !f.value()._includeFile && !f.value()._dataFile;

            if (!f.value()._subFilePath.isEmpty()) {

                file.close();
                writeFileName = QDir::toNativeSeparators(f.value()._subFilePath);
                file.setFileName(writeFileName);

                if (!file.open(QFile::WriteOnly | QFile::Text)) {
                    emit gui->messageSig(LOG_ERROR,QString("Cannot write %1 subfile %2:<br>%3.")
                                        .arg(_mpd ? "MPD" : "LDR",
                                             writeFileName,
                                             file.errorString()));
                    gui->insertFinalModelStep();
                    Gui::suspendFileDisplay = false;
                    return false;
                }

                emit gui->messageSig(LOG_INFO,QString("Saving %1 subfile %2.")
                                     .arg(_mpd ? "MPD" : "LDR", writeFileName));

                QTextStream out(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
                out.setEncoding(_currFileIsUTF8 ? QStringConverter::Utf8 : QStringConverter::System);
#else
                out.setCodec(_currFileIsUTF8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForName("System"));
#endif
            }

            if (addFILEMeta) {
                out << "0 FILE " << subFileName << lpub_endl;
                afterFILEMeta = true;
                _savedLines++;
            }

            static QRegularExpression newLineBeforeRx("^\\s*0\\s+\\/\\/\\s*(Segments| Segment |Start cap |End cap |Fixed color segments)[^\n]*", QRegularExpression::CaseInsensitiveOption);
            static QRegularExpression newLineAfterRx("^\\s*0\\s+\\/\\/\\s*( The path is approx | License: )[^\n]*", QRegularExpression::CaseInsensitiveOption);
            for (int j = 0; j < f.value()._contents.size(); j++) {

                _savedLines++;

                bool insertNewLineBefore = false;
                bool insertNewLineAfter = false;

                QString line = f.value()._contents[j];

                afterFILEMeta &= line.trimmed() == "0";

                if (isModelHeader && line[0] == '0') {
                    if (!isLDCadContent)
                        isLDCadContent = line.contains(_fileRegExp[LDC_RX]);
                } else if (line[0] != '0') {
                    isModelHeader = false;
                    if (isLDCadContent) {
                        QStringList tokens;
                        split(line,tokens);
                        if (tokens.size() == 15 && contains(tokens.at(14)) && _subFileOrder.size() > i)
                            omitNOFIlEMeta = tokens.at(14).toLower() == _subFileOrder.at(i+1).toLower();
                    }
                }

                if (!newLineIinserted) {
                    if (isLDCadContent) {
                        insertNewLineBefore = line.contains(newLineBeforeRx) && (j ? !f.value()._contents.at(j-1).startsWith("0 //Segments",Qt::CaseSensitive) : true);
                        if (!insertNewLineBefore) {
                            insertNewLineAfter  = line.startsWith("0 BFC ");
                            insertNewLineAfter |= line.contains(newLineAfterRx);
                        }
                        if (insertNewLineBefore || insertNewLineAfter)
                            _savedLines++;
                    } else if (line.startsWith("0 !LICENSE ") && f.value()._contents.size() > j) {
                        insertNewLineAfter |= f.value()._contents.at(j+1).startsWith("0 !LDCAD GENERATED ",Qt::CaseSensitive);
                        _savedLines++;
                    }
/*
                    // this condition block adds a space after the folowing headers
                    else
                    if (f.value()._unofficialPart > UNOFFICIAL_SUBMODEL &&
                       (line.startsWith("0 !LICENSE ")                  ||
                        line.startsWith("0 BFC ")                       ||
                        line.startsWith("0 !HISTORY "))                 &&
                       (f.value()._contents.size() > (j+1)              &&
                        f.value()._contents[j+1] != "0"                 &&
                        f.value()._contents[j+1] != ""))
                    {
                        insertNewLineAfter = true;
                    }
//*/
                }

                if (insertNewLineBefore)
                {
                    out << lpub_endl;
                    out << line << lpub_endl;
                }
                else if (insertNewLineAfter)
                {
                    out << line << lpub_endl;
                    out << lpub_endl;
                }
                else if (!afterFILEMeta)
                    out << line << lpub_endl;

                newLineIinserted = insertNewLineBefore || insertNewLineAfter;

                afterFILEMeta = false;
            }

            if (addFILEMeta) {
                if (!omitNOFIlEMeta) {
                    out << "0 NOFILE" << lpub_endl;
                    _savedLines++;
                    out << "0" << lpub_endl;
                    _savedLines++;
                }
                else
                {
                    out << lpub_endl;
                    _savedLines++;
                }
            }
        }
    }

    file.close();

    gui->insertFinalModelStep();
    Gui::suspendFileDisplay = false;

    return true;
}

bool LDrawFile::saveIncludeFile(const QString &fileName) {
    QString includeFileName = fileName.toLower();

    QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(includeFileName);
    if (f != _subFiles.end() && f.value()._includeFile) {

      if (f.value()._modified) {
        QFile file;
        QString writeFileName;

        if (!f.value()._subFilePath.isEmpty()) {
            writeFileName = f.value()._subFilePath;
            file.setFileName(writeFileName);
        }

        if (!file.open(QFile::WriteOnly | QFile::Text)) {
          emit gui->messageSig(LOG_ERROR,QString("Cannot write include file %1:<br>%2.")
                               .arg(writeFileName, file.errorString()));
          return false;
        }

        QTextStream out(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
        out.setEncoding(_currFileIsUTF8 ? QStringConverter::Utf8 : QStringConverter::System);
#else
        out.setCodec(_currFileIsUTF8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForName("System"));
#endif

        for (int j = 0; j < f.value()._contents.size(); j++) {
          out << f.value()._contents[j] << lpub_endl;
          _savedLines++;
        }

        file.close();
      }
    }
    return true;
}

bool LDrawFile::saveDatafile(const QString &fileName, const QByteArray &dataFile)
{
  if (dataFile.isEmpty()) {
    emit gui->messageSig(LOG_WARNING, QObject::tr("Base 64 image data for %1 is empty.").arg(fileName));
    return false;
  }

  QString const imageDirPath = QString("%1%2%3").arg(Preferences::lpubDataPath).arg(QDir::separator()).arg(Paths::customTextureDir);
  QString const imageFilePath = QString("%1%2%3").arg(imageDirPath).arg(QDir::separator()).arg(fileName);

  QFileInfo fileInfo(imageFilePath);
  QString const datafileName = fileInfo.baseName().toUpper();

  bool imageFileExists = fileInfo.exists();
  bool imageFileArchived = lcGetPiecesLibrary()->FindTexture(QString(datafileName).toLatin1().constData(), nullptr, false);

  if (imageFileExists && imageFileArchived) {
    emit gui->messageSig(LOG_INFO, QObject::tr("Image file '%1' exists.").arg(fileName));
    return true;
  }

  QDir textureDir(imageDirPath);
  if (!textureDir.exists())
    textureDir.mkpath(".");

  QPixmap image;
  if (!image.loadFromData(QByteArray::fromBase64(dataFile))) {
    emit gui->messageSig(LOG_WARNING, QObject::tr("Cannot load base 64 image data for %1").arg(fileName));
    return false;
  }

  if (!imageFileExists) {
    QFile file(imageFilePath);
    if (file.open(QIODevice::WriteOnly)) {
      if ((imageFileExists = image.save(&file, "PNG"))) {
        imageFileArchived = false;
        emit gui->messageSig(LOG_INFO, QObject::tr("Saved image file %1.").arg(file.fileName()));
      } else
        emit gui->messageSig(LOG_WARNING, QObject::tr("Cannot save image file %1").arg(file.fileName()));
    } else
      emit gui->messageSig(LOG_WARNING, QObject::tr("Cannot open for write image file %1<br>%2").arg(file.fileName(), file.errorString()));
  }

  if (imageFileExists && !imageFileArchived) {
    updatePartsArchive = true;
    return true;
  }

  return false;
}

void LDrawFile::insertMissingItem(const QStringList &item)
{
    QString const type = item.at(0).toLower();
    QMap<QString, MissingItem>::iterator i = _missingItems.find(type);
    if (i != _missingItems.end())
      _missingItems.erase(i);
    MissingItem missingItem(type, item.at(1));
    _missingItems.insert(type, missingItem);
}

bool LDrawFile::isMissingItem(const QString &fileName)
{
    QString const type = fileName.toLower();
    QMap<QString, MissingItem>::iterator i = _missingItems.find(type);
    if (i != _missingItems.end())
      return true;
    return false;
}

void LDrawFile::removeMissingItem(const QString &fileName)
{
    QString const type = fileName.toLower();
    QMap<QString, MissingItem>::iterator i = _missingItems.find(type);
    if (i != _missingItems.end())
        _missingItems.erase(i);
}

bool LDrawFile::changedSinceLastWrite(const QString &fileName)
{
  QString mcFileName = fileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(mcFileName);
  if (i != _subFiles.end()) {
    bool value = i.value()._changedSinceLastWrite;
    i.value()._changedSinceLastWrite = false;
    return value;
  }
  return false;
}

void LDrawFile::tempCacheCleared()
{
  const QList _sfKeys =_subFiles.keys();
  for (const QString &key : _sfKeys) {
    _subFiles[key]._changedSinceLastWrite = true;
    _subFiles[key]._modified = true;
  }
}

void LDrawFile::insertLDCadGroup(const QString &name, int lid)
{
  QMultiHash<QString, int>::const_iterator i = _ldcadGroups.constBegin();
  while (i != _ldcadGroups.constEnd()) {
    if (i.key() == name && i.value() == lid)
      return;
    ++i;
  }
  _ldcadGroups.insert(name,lid);
}

bool LDrawFile::ldcadGroupMatch(const QString &name, const QStringList &lids)
{
  const QList<int> values = _ldcadGroups.values(name);
  for (const QString &lid : lids) {
    if (values.contains(lid.toInt()))
      return true;
  }
  return false;
}

/* Build Modification Routines */

void LDrawFile::insertBuildMod(const QString      &buildModKey,
                               const QVector<int> &modAttributes,
                               int                 stepIndex)
{
  QString modKey  = buildModKey.toLower();
  QVector<int>  modSubmodelStack;
  QMap<int,int> modActions;
  QVector<int>  newAttributes;
  QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
  if (i != _buildMods.end()) {
    // Preserve actions
    modActions = i.value()._modActions;

    // Preserve submodelStack
    modSubmodelStack = i.value()._modSubmodelStack;

    // Remove action for specified stepIndex
    QMap<int,int>::iterator a = modActions.find(stepIndex);
    if (a != modActions.end())
        modActions.erase(a);

    // Update attributes
    newAttributes = { 0, 0, 0, 0, 0, -1, 0, 0 };
    for (int a = 0; a < modAttributes.size(); a++) {
      if (a < i.value()._modAttributes.size()) {
        if (modAttributes.at(a) != i.value()._modAttributes.at(a) && modAttributes.at(a) != newAttributes.at(a))
          newAttributes[a] = modAttributes.at(a);
        else
          newAttributes[a] = i.value()._modAttributes.at(a);
      } else
        newAttributes[a] = i.value()._modAttributes.at(a);
    }

    // Remove BuildMod if exist
    _buildMods.erase(i);

  } else
    newAttributes = modAttributes;

  // Initialize new BuildMod
  BuildMod buildMod(newAttributes, stepIndex);

  // Restore preserved actions
  if (modActions.size()) {
    QMap<int,int>::const_iterator a = modActions.constBegin();
    while (a != modActions.constEnd()) {
      buildMod._modActions.insert(a.key(), a.value());
      ++a;
    }
  }

  // Insert new BuildMod
  _buildMods.insert(modKey, buildMod);

  // Insert BuildModStep - must come after _buildMods.insert()
  insertBuildModStep(modKey, stepIndex, BuildModApplyRc);

  // Set submodelStack items modified if exists
  const QVector<int> msmStack = modSubmodelStack;
  if (msmStack.size()) {
      buildMod._modSubmodelStack = modSubmodelStack;
      for (int modelIndex : msmStack)
          setModified(getSubmodelName(modelIndex),true);
  }

  // Set subfile modified
  QString modFileName = getBuildModStepKeyModelName(modKey);
  setModified(modFileName, true);

  // Update BuildMod list
  if (!_buildModList.contains(buildModKey))
      _buildModList.append(buildModKey);
}

/* This call captures steps that have buildMod command(s) */

void LDrawFile::insertBuildModStep(const QString &buildModKey,
                                   const int      stepIndex,
                                   const int      modAction)
{
    bool modBegin = false;
    QString modKey = buildModKey.toLower();
    BuildModStep newModStep(stepIndex, modAction ? modAction : getBuildModAction(modKey, stepIndex), modKey);
    QMultiMap<int, BuildModStep>::iterator i = _buildModSteps.find(stepIndex);
    while (i != _buildModSteps.end() && i.key() == stepIndex) {
        if (i.value()._buildModKey == modKey) {
            modBegin = getBuildModStepIndex(i.value()._buildModKey) == stepIndex;
            if (modBegin)
                setViewerStepHasBuildModAction(getViewerStepKey(i.key()), false);
            _buildModSteps.erase(i);
            break;
        }
        ++i;
    }

    // we update submodels in the calls that use this call:
    //   ::insertBuildMod and ::setBuildModAction

    _buildModSteps.insert(stepIndex, newModStep);

#ifdef QT_DEBUG_MODE
    int action = modBegin ? static_cast<int>(BuildModBeginRc) : modAction ? modAction : newModStep._buildModAction;
    const QString message =
            QString("Insert BuildMod Step ModStepIndex: %1, Action: %2, ModKey: '%3'")
                    .arg(stepIndex).arg(action == BuildModApplyRc ? "Apply(64)" : action == BuildModRemoveRc ? "Remove(65)" : action == BuildModBeginRc ? "Begin(61)" : "None(0)").arg(buildModKey);
    //qDebug() << qPrintable("DEBUG: " + message + "\n\n");
    emit gui->messageSig(LOG_DEBUG, message);
#endif
}

int LDrawFile::getBuildModStepAction(const QString &modelName,
                                     const     int &lineNumber)
{
    int modAction = BuildModNoActionRc;
    int modelIndex = getSubmodelIndex(modelName);
    int modStepIndex = getBuildModStepIndex(modelIndex, lineNumber);
    int modBeginStepIndex = BuildModInvalidIndexRc;

    QMultiMap<int, BuildModStep>::const_iterator i = _buildModSteps.constFind(modStepIndex);
    if (i != _buildModSteps.constEnd()) {
        modAction = i.value()._buildModAction;
        modBeginStepIndex = getBuildModStepIndex(i.value()._buildModKey);
        if(modBeginStepIndex == modStepIndex)
            modAction = BuildModBeginRc;
#ifdef QT_DEBUG_MODE
        const QString message =
                QString("Get BuildModStepAction "
                        "StepIndex: %1, "
                        "BeginIndex: %2, "
                        "StepModCount: %3, "
                        "ModAction: %4%5, "
                        "Top ModelIndex: %6, "
                        "Top LineNumber: %7, "
                        "ModKey: '%8'")
                        .arg(modStepIndex)
                        .arg(modBeginStepIndex)
                        .arg(_buildModSteps.values(modStepIndex).size())
                        .arg(modAction == BuildModBeginRc ? "Begin(61)" : modAction == BuildModApplyRc ? "Apply(64)" : modAction == BuildModRemoveRc ? "Remove(65)" : "None(0)")
                        .arg(modAction == BuildModBeginRc ? QString(", LineNumber: %1").arg(getBuildModBeginLineNumber(i.value()._buildModKey)) : "")
                        .arg(modelIndex).arg(lineNumber).arg(i.value()._buildModKey);
        //qDebug() << qPrintable("DEBUG: " + message + "\n\n");
        emit gui->messageSig(LOG_DEBUG, message);
#endif
    }

    return modAction;
}

int LDrawFile::getBuildModStepAction(const int stepIndex)
{
    QVector<int> topOfStep = _buildModStepIndexes.at(stepIndex);
    QString modelName      = getSubmodelName(topOfStep.at(BM_MODEL_NAME));
    int lineNumber         = topOfStep.at(BM_LINE_NUMBER);

    if (!modelName.isEmpty() && lineNumber > BM_BEGIN_LINE_NUM)
        return getBuildModStepAction(modelName, lineNumber);

    return static_cast<int>(BuildModNoActionRc);
}

/* This call will capture a step's build mod actions (if exists) and check if the captured action is BuildModBeginRc */

QList<QVector<int> > LDrawFile::getBuildModStepActions(
        const QString &modelName,
        const     int &lineNumber)
{
    QList<QVector<int> > modActions;
    int modelIndex = getSubmodelIndex(modelName);
    int modStepIndex = getBuildModStepIndex(modelIndex, lineNumber);
#ifdef QT_DEBUG_MODE
    int stepModCount = 1;
#endif
    QMultiMap<int, BuildModStep>::const_iterator i = _buildModSteps.constFind(modStepIndex);
    while (i != _buildModSteps.constEnd() && i.key() == modStepIndex) {
        QVector<int> modAction = { BM_INVALID_INDEX, BM_INIT, BM_INIT };
        static QRegularExpression buildModKeyRx(i.value()._buildModKey, QRegularExpression::CaseInsensitiveOption);
        int modBeginStepIndex = getBuildModStepIndex(i.value()._buildModKey);
        modAction[BM_ACTION_KEY_INDEX] = _buildModList.indexOf(buildModKeyRx);
        if (modBeginStepIndex == modStepIndex) {        // step index has BuildModBeginRc
            modAction[BM_ACTION_LINE_NUM] = getBuildModBeginLineNumber(i.value()._buildModKey);
            modAction[BM_ACTION_CODE] = static_cast<int>(BuildModBeginRc);
        } else {
            modAction[BM_ACTION_LINE_NUM] = lineNumber; // action topOfStep.lineNumber
            modAction[BM_ACTION_CODE] = i.value()._buildModAction; // apply or remove
        }
#ifdef QT_DEBUG_MODE
        const QString message =
                QString("Get BuildModStepActions "
                        "StepIndex: %1, "
                        "BeginIndex: %2, "
                        "StepModCount: %3, "
                        "ModAction: %4%5, "
                        "Top ModelIndex: %6, "
                        "Top LineNumber: %7, "
                        "ModKey(Index): '%8(%9)'")
                        .arg(modStepIndex)
                        .arg(modBeginStepIndex)
                        .arg(stepModCount)
                        .arg(modAction[BM_ACTION_CODE] == BuildModBeginRc ? "Begin(61)" : modAction[BM_ACTION_CODE] == BuildModApplyRc ? "Apply(64)" : modAction[BM_ACTION_CODE] == BuildModRemoveRc ? "Remove(65)" : "None(0)")
                        .arg(modAction[BM_ACTION_CODE] == BuildModBeginRc ? QString(", LineNumber: %1").arg(modAction[BM_ACTION_LINE_NUM]) : "")
                        .arg(modelIndex)
                        .arg(lineNumber)
                        .arg(i.value()._buildModKey)
                        .arg(modAction[BM_ACTION_KEY_INDEX]);
        //qDebug() << qPrintable("DEBUG: " + message + "\n\n");
        emit gui->messageSig(LOG_DEBUG, message);
        stepModCount++;
#endif
        modActions.append(modAction);
        i++;
    }

    return modActions;
}

void LDrawFile::clearBuildModStep(const QString &buildModKey,const int stepIndex)
{
    bool modBegin = false;
    QString modKey = buildModKey.toLower();
    QMultiMap<int, BuildModStep>::iterator i = _buildModSteps.find(stepIndex);
    while (i != _buildModSteps.end() && i.key() == stepIndex) {
        if (i.value()._buildModStepIndex == stepIndex &&
            i.value()._buildModKey == modKey) {
            modBegin = getBuildModStepIndex(i.value()._buildModKey) == stepIndex;
            if (modBegin)
                setViewerStepHasBuildModAction(getViewerStepKey(i.key()), false);
#ifdef QT_DEBUG_MODE
            int action = modBegin ? static_cast<int>(BuildModBeginRc) : i.value()._buildModAction;
            const QString message =
                    QString("Remove BuildModStep ModStepIndex: %1, Action: %2, ModKey: '%3'")
                            .arg(stepIndex)
                            .arg(action == BuildModApplyRc ? "Apply(64)" : action == BuildModRemoveRc ? "Remove(65)" : action == BuildModBeginRc ? "Begin(61)" : "None(0)")
                            .arg(buildModKey);
            //qDebug() << qPrintable("DEBUG: " + message + "\n\n");
            emit gui->messageSig(LOG_DEBUG, message);
#endif
            _buildModSteps.erase(i);
            break;
        }
        ++i;
    }
}

void LDrawFile::clearBuildModSteps(const QString &buildModKey)
{
#ifdef QT_DEBUG_MODE
    emit gui->messageSig(LOG_TRACE, QString("Remove BuildModStep actions for ModKey %1...") .arg(buildModKey));
#endif
    bool modBegin = false;
    QString modKey = buildModKey.toLower();
    QMultiMap<int, BuildModStep> buildModSteps;
    QMultiMap<int, BuildModStep>::iterator i = _buildModSteps.begin();
    while (i != _buildModSteps.end()) {
        if(i.value()._buildModKey != modKey ) {
            buildModSteps.insert(i.key(),i.value());
        }
        else
        {
            modBegin = getBuildModStepIndex(i.value()._buildModKey) == i.value()._buildModStepIndex;
            if (modBegin)
                setViewerStepHasBuildModAction(getViewerStepKey(i.key()), false);
#ifdef QT_DEBUG_MODE
            int action = modBegin ? static_cast<int>(BuildModBeginRc) : i.value()._buildModAction;
            const QString message =
                    QString("Removed Step Index: %1, Action: %2")
                            .arg(i.value()._buildModStepIndex)
                            .arg(action == BuildModApplyRc ? "Apply(64)" : action == BuildModRemoveRc ? "Remove(65)" : action == BuildModBeginRc ? "Begin(61)" : "None(0)");
            //qDebug() << qPrintable("DEBUG: " + message + "\n\n");
            emit gui->messageSig(LOG_TRACE, message);
#endif
        }
        i++;
    }

    if (buildModSteps.size()) {
        _buildModSteps.clear();
        _buildModSteps = buildModSteps;
    }
}

bool LDrawFile::deleteBuildMod(const QString &buildModKey)
{
    QString modKey = buildModKey.toLower();
    QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
    if (i != _buildMods.end()) {
        _buildMods.erase(i);
        if (_buildModList.contains(buildModKey, Qt::CaseInsensitive)) {
            static QRegularExpression buildModKeyRx(buildModKey, QRegularExpression::CaseInsensitiveOption);
            const int buildModListIndex = _buildModList.indexOf(buildModKeyRx);
            if (buildModListIndex > -1)
                _buildModList.removeAt(buildModListIndex);
        }

        // Set subfile modified
        QString modFileName = getBuildModStepKeyModelName(modKey);
        QMap<QString, LDrawSubFile>::iterator s = _subFiles.find(modFileName);
        if (s != _subFiles.end()) {
          if (s.value()._buildMods > 0)
            s.value()._buildMods -= 1;
          s.value()._modified = true;
          s.value()._changedSinceLastWrite = true;
        }

        // Clear build mod steps
        clearBuildModSteps(modKey);

#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_DEBUG, QString("Removed BuildMod ModKey: '%1'").arg(buildModKey));
#endif
        return true;
    }
    return false;
}

void LDrawFile::deleteBuildMods(const int stepIndex)
{
    for(QString &key : getBuildModsList()) {
        const int index = getBuildModStepIndex(key);
        if (index >= stepIndex) {
            deleteBuildMod(key);
        }
    }
}

void LDrawFile::deleteBuildMods()
{
    for (QString &key : getBuildModsList()) {
        deleteBuildMod(key);
    }
}

void LDrawFile::setBuildModStepKey(const QString &buildModKey, const QString &modStepKey)
{
    QString  modKey = buildModKey.toLower();
    QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
    if (i != _buildMods.end()) {
        QStringList stepKeys = modStepKey.split(";");
        i.value()._modAttributes[BM_MODEL_NAME_INDEX] = stepKeys.at(BM_STEP_MODEL_KEY).toInt();
        i.value()._modAttributes[BM_MODEL_LINE_NUM]   = stepKeys.at(BM_STEP_LINE_KEY).toInt();
        i.value()._modAttributes[BM_MODEL_STEP_NUM]   = stepKeys.at(BM_STEP_NUM_KEY).toInt();
#ifdef QT_DEBUG_MODE
        int action = BuildModNoActionRc;
        QMap<int, int>::iterator ai = i.value()._modActions.find(i.value()._modStepIndex);
        if (ai != i.value()._modActions.end())
            action = ai.value();
        emit gui->messageSig(LOG_DEBUG, QString("Update BuildMod StepKey: %1;%2;%3, StepIndex: %4, Action: %5, ModKey: '%6'")
                             .arg(i.value()._modAttributes.at(BM_MODEL_NAME_INDEX))
                             .arg(i.value()._modAttributes.at(BM_MODEL_LINE_NUM))
                             .arg(i.value()._modAttributes.at(BM_MODEL_STEP_NUM))
                             .arg(i.value()._modStepIndex)
                             .arg(action ? action == BuildModApplyRc ? "Apply(64)" : "Remove(65)" : "None(0)")
                             .arg(buildModKey));
#endif
    }
}

int LDrawFile::getBuildModBeginLineNumber(const QString &buildModKey)
{
  QString modKey = buildModKey.toLower();
  QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
  if (i != _buildMods.end() && i.value()._modAttributes.size() > BM_BEGIN_LINE_NUM) {
    return i.value()._modAttributes.at(BM_BEGIN_LINE_NUM);
  }

  return 0;
}

int LDrawFile::getBuildModActionLineNumber(const QString &buildModKey)
{
  QString modKey = buildModKey.toLower();
  QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
  if (i != _buildMods.end() && i.value()._modAttributes.size() > BM_ACTION_LINE_NUM) {
    return i.value()._modAttributes.at(BM_ACTION_LINE_NUM);
  }

  return 0;
}

int LDrawFile::getBuildModEndLineNumber(const QString &buildModKey)
{
  QString modKey = buildModKey.toLower();
  QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
  if (i != _buildMods.end() && i.value()._modAttributes.size() > BM_END_LINE_NUM) {
    return i.value()._modAttributes.at(BM_END_LINE_NUM);
  }

  return 0;
}

int LDrawFile::getBuildModDisplayPageNumber(const QString &buildModKey)
{
  QString modKey = buildModKey.toLower();
  QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
  if (i != _buildMods.end() && i.value()._modAttributes.size() > BM_DISPLAY_PAGE_NUM) {
    return i.value()._modAttributes.at(BM_DISPLAY_PAGE_NUM);
  }

  return 0;
}

int LDrawFile::setBuildModDisplayPageNumber(const QString &buildModKey, int displayPageNum)
{
    QString  modKey = buildModKey.toLower();
    QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
    if (i != _buildMods.end()) {
        i.value()._modAttributes[BM_DISPLAY_PAGE_NUM] = displayPageNum;

#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_DEBUG, QString("Set BuildMod DisplayPageNumber: %1, ModKey: '%2'")
                                                .arg(i.value()._modAttributes.at(BM_DISPLAY_PAGE_NUM))
                                                .arg(buildModKey));
#endif

        return i.value()._modAttributes.at(BM_DISPLAY_PAGE_NUM);
    }

    return 0;
}

int LDrawFile::getBuildModStepPieces(const QString &buildModKey)
{
  QString modKey = buildModKey.toLower();
  QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
  if (i != _buildMods.end() && i.value()._modAttributes.size() > BM_STEP_PIECES) {
    return i.value()._modAttributes.at(BM_STEP_PIECES);
  }

  return 0;
}

int LDrawFile::setBuildModStepPieces(const QString &buildModKey, int pieces)
{
    QString  modKey = buildModKey.toLower();
    QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
    if (i != _buildMods.end()) {
        i.value()._modAttributes[BM_STEP_PIECES] = pieces;
#ifdef QT_DEBUG_MODE
        int action = BuildModNoActionRc;
        QMap<int, int>::iterator ai = i.value()._modActions.find(i.value()._modStepIndex);
        if (ai != i.value()._modActions.end())
            action = ai.value();
        emit gui->messageSig(LOG_DEBUG, QString("Update BuildMod StepPieces: %1, StepIndex: %2, Action: %3, ModKey: '%4'")
                                                .arg(i.value()._modAttributes.at(BM_STEP_PIECES))
                                                .arg(i.value()._modStepIndex)
                                                .arg(action ? action == BuildModApplyRc ? "Apply(64)" : "Remove(65)" : "None(0)")
                                                .arg(buildModKey));
#endif
        return i.value()._modAttributes.at(BM_STEP_PIECES);
    }

    return 0;
}

int LDrawFile::setBuildModRendered(const QString &buildModKey, const QString &renderedModel)
{
#ifdef QT_DEBUG_MODE
    bool entryAdded = true;
#endif
    QString  modKey = buildModKey.toLower();
    QString  subModel = renderedModel.toLower();
    QMap<QString, QStringList>::iterator i = _buildModRendered.find(modKey);
    if (i == _buildModRendered.end()) {
        _buildModRendered.insert(modKey, QStringList() << subModel);
    } else if (! i.value().contains(subModel)) {
        i.value().append(subModel);
    }
#ifdef QT_DEBUG_MODE
    else {
        entryAdded = false;
    }
    if (entryAdded)
        emit gui->messageSig(LOG_DEBUG, QString("Insert BuildMod RenderedModel: %1, ModKey: '%2'").arg(renderedModel, buildModKey));
#endif
    return 0;
}

bool LDrawFile::getBuildModRendered(const QString &buildModKey, const QString &renderedModel, bool countPage)
{
    QString const modKey = QString("%1%2").arg(countPage ? COUNT_PAGE_PREFIX";" : "").arg(buildModKey);
    QMap<QString, QStringList>::iterator i = _buildModRendered.find(modKey.toLower());
    if (i != _buildModRendered.end()) {
        if (i.value().contains(renderedModel.toLower())) {
#ifdef QT_DEBUG_MODE
            emit gui->messageSig(LOG_DEBUG, QString("BuildMod RenderedModel: %1, ModKey: '%2'")
                                                    .arg(renderedModel, modKey));
#endif
            return true;
        }
    }
    return false;
}

void LDrawFile::clearBuildModRendered(const QString &buildModKey, const QString &renderedModel)
{
    QString modKey = buildModKey.toLower();
    QString subModel = renderedModel.toLower();
    QMap<QString, QStringList>::iterator i = _buildModRendered.find(modKey);
    if (! i.value().contains(subModel)) {
        int cleared = i.value().removeAll(subModel);
#ifdef QT_DEBUG_MODE
        if (cleared)
            emit gui->messageSig(LOG_DEBUG, QString("Clear BuildMod RenderedModel: %1, Count: %2, ModKey: '%3'")
                                 .arg(renderedModel).arg(cleared).arg(buildModKey));
#else
        Q_UNUSED(cleared)
#endif
    }
}

void LDrawFile::clearBuildModRendered(bool countPage)
{
    const QList _bmKeys = _buildModRendered.keys();
    for (const QString &key : _bmKeys) {
        if (countPage) {
           const QList bmRendered = _buildModRendered[key];
           for (const QString &modelFile : bmRendered) {
               if (modelFile.startsWith(COUNT_PAGE_PREFIX)) {
                   _buildModRendered[key].removeAll(modelFile);
               }
           }
        } else {
            _buildModRendered[key].clear();
        }
    }
}

void LDrawFile::setBuildModSubmodelStack(const QString &buildModKey, const QStringList &submodelStack)
{
    QString  modKey = buildModKey.toLower();
    QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
    if (i != _buildMods.end()) {
        for (const QString &modelFile : submodelStack) {
            int submodelIndex = getSubmodelIndex(modelFile);
            if (!i.value()._modSubmodelStack.contains(submodelIndex)) {
                i.value()._modSubmodelStack << submodelIndex;
                setModified(modelFile.toLower(), true);
            }
        }
#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_DEBUG, QString("Update BuildMod ParentSubmodels: %1, StepIndex: %2, ModKey: '%4'")
                                                .arg(i.value()._modSubmodelStack.size())
                                                .arg(i.value()._modStepIndex)
                                                .arg(buildModKey));
#endif
    }
}

int LDrawFile::getBuildModActionPrevIndex(const QString &buildModKey, const int stepIndex, const int action)
{
    int actionStepIndex;
    if (getBuildModAction(buildModKey, stepIndex - 1, BM_LAST_ACTION, actionStepIndex) == action)
        return actionStepIndex;
    return stepIndex;
}

int LDrawFile::getBuildModAction(const QString &buildModKey, const int stepIndex)
{
    int unusedIndex;
    return getBuildModAction(buildModKey, stepIndex, BM_LAST_ACTION, unusedIndex);
}

int LDrawFile::getBuildModAction(const QString &buildModKey, const int stepIndex, const int defaultIndex)
{
    int unusedIndex = BM_INVALID_INDEX;
    return getBuildModAction(buildModKey, stepIndex, defaultIndex, unusedIndex);
}

int LDrawFile::getBuildModAction(const QString &buildModKey, const int stepIndex, const int defaultIndex, int &actionStepIndex)
{
  QString insert = QString();
  QString modKey = buildModKey.toLower();
  int action = BuildModNoActionRc;
  actionStepIndex = stepIndex;
  QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
  if (i != _buildMods.end()) {
      // return BuildModRemoveRc (65) for 'Future' requests where the requested stepIndex is before the buildMod stepIndex
      if (stepIndex < i.value()._modActions.firstKey() && stepIndex > BM_INVALID_INDEX) {
          actionStepIndex = i.value()._modActions.firstKey();
          action = BuildModRemoveRc;
          insert = " Future";
      } else if (stepIndex == BM_LAST_ACTION) {
          action = i.value()._modActions.last();
          actionStepIndex = i.value()._modActions.lastKey();
          insert = " Last";
      } else {
          QMap<int, int>::iterator a = i.value()._modActions.find(stepIndex);
          if (a != i.value()._modActions.end() && defaultIndex == BM_LAST_ACTION) {
              action = i.value()._modActions.value(stepIndex);
              insert = " Current";
          } else if (i.value()._modActions.size()) {
              int keyIndex = stepIndex;
              if (defaultIndex == BM_LAST_ACTION) {
                  action = i.value()._modActions.last();
                  keyIndex = i.value()._modActions.lastKey();
                  insert = " Last";
              } else if (defaultIndex == BM_PREVIOUS_ACTION) {
                  // iterate backward to get the last action index before the specified step index (account for action index gap)
                  for (; keyIndex >= BM_FIRST_INDEX; keyIndex--) {
                      if (i.value()._modActions.value(keyIndex, BM_INVALID_INDEX) > BM_INVALID_INDEX) {
                          action = i.value()._modActions.value(keyIndex);
                          insert = " Previous";
                          break;
                      }
                  }
              } else if (defaultIndex == BM_NEXT_ACTION) {
                  // iterate forward to get the next action index after the specified step index
                  for (; keyIndex < i.value()._modActions.size(); keyIndex++) {
                      if (i.value()._modActions.value(keyIndex, BM_INVALID_INDEX) > BM_INVALID_INDEX) {
                          action = i.value()._modActions.value(keyIndex);
                          insert = " Next";
                          break;
                      }
                  }
              }
              actionStepIndex = keyIndex;
          }
      }
  }

  // BuildMod update 18/07/2022 - this may not be correct as the actual
  // stepIndex request is usually before the buildMod stepIndex - hence
  // the need to 'set' the action. The 'Default' behaviour should be to
  // return BuildModRemoveRc (65) as the BuildMod is in the future.
  // The 'Future' scenario is addressed above but I'm leaving this code
  // in to see if there are any other valid use-cases for it.
  if (!action) {
     action = setBuildModAction(buildModKey, stepIndex, BuildModApplyRc);
     if (insert == " Future")
         insert = " Default (Future)";
     insert = " Default";
  }

  if (!action)
      emit gui->messageSig(LOG_ERROR, QString("Get BuildMod%1 Action: (INVALID), StepIndex: %2, ActionStepIndex: %3, ModKey: '%4'")
                                              .arg(insert)
                                              .arg(stepIndex)
                                              .arg(actionStepIndex)
                                              .arg(buildModKey));
#ifdef QT_DEBUG_MODE
  else
      emit gui->messageSig(LOG_TRACE, QString("Get BuildMod%1 Action: %2, %3ActionStepIndex: %4, ModKey: '%5'")
                                              .arg(insert)
                                              .arg(action ? action == BuildModApplyRc ? "Apply(64)" : "Remove(65)" : "None(0)")
                                              .arg(stepIndex >= 0 ? QString("StepIndex: %1, ").arg(stepIndex) : "")
                                              .arg(actionStepIndex)
                                              .arg(buildModKey));
#endif

  return action;
}

int LDrawFile::setBuildModAction(
        const QString  &buildModKey,
        const int       stepIndex,
        const int       modAction)
{
    QString  modKey = buildModKey.toLower();
    QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);

    int action = modAction;

    if (i != _buildMods.end()) {
        QMap<int, int>::iterator ai = i.value()._modActions.find(stepIndex);
        if (ai != i.value()._modActions.end())
            i.value()._modActions.remove(stepIndex);
        i.value()._modActions.insert(stepIndex, modAction);

        insertBuildModStep(modKey, stepIndex);

        QString modFileName = getBuildModStepKeyModelName(modKey);
        setModified(modFileName, true);
        const QVector<int> _smStack = i.value()._modSubmodelStack;
        for (const int modelIndex : _smStack)
            setModified(getSubmodelName(modelIndex), true);

        action = i.value()._modActions.value(stepIndex);

#ifdef QT_DEBUG_MODE
        bool change = modified(modFileName);
        emit gui->messageSig(LOG_DEBUG, QString("Set BuildMod Action: %1, ModKey: '%2', StepIndex: %3, Changed: %4, ModelFile: %5")
                                                .arg(action ? action == BuildModApplyRc ? "Apply(64)" : "Remove(65)" : "None(0)")
                                                .arg(modKey)
                                                .arg(stepIndex)
                                                .arg(change ? "True" : "False")
                                                .arg(modFileName));
#endif
    }

    return action;
}

QMap<int, int>LDrawFile::getBuildModActions(const QString &buildModKey)
{
    QMap<int, int> empty;
    QString  modKey = buildModKey.toLower();
    QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
    if (i != _buildMods.end()) {
        return i.value()._modActions;
    }

    return empty;
}

void LDrawFile::clearBuildModAction(const QString &buildModKey,const int stepIndex)
{
    QString  modKey = buildModKey.toLower();
    QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
    if (i != _buildMods.end()) {
#ifdef QT_DEBUG_MODE
        bool change = false;
        int action = BuildModNoActionRc;
#endif
        QString modFileName = getBuildModStepKeyModelName(modKey);
        QMap<int, int>::iterator a = i.value()._modActions.find(stepIndex);
        if (a != i.value()._modActions.end()) {
#ifdef QT_DEBUG_MODE
            action = i.value()._modActions.value(stepIndex);
#endif
            i.value()._modActions.remove(stepIndex);

            clearBuildModStep(modKey, stepIndex);

            QMap<QString, LDrawSubFile>::iterator s = _subFiles.find(modFileName);
            if (s != _subFiles.end()) {
              s.value()._modified = true;
              s.value()._changedSinceLastWrite = true;
#ifdef QT_DEBUG_MODE
              change = true;
#endif
            }
#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_TRACE, QString("Remove BuildMod Action: %1, StepIndex: %2, Changed: %3, ModelFile: %4")
                                                .arg(action ? action == BuildModApplyRc ? "Apply(64)" : "Remove(65)" : "None(0)")
                                                .arg(stepIndex)
                                                .arg(change ? "True" : "False")
                                                .arg(modFileName));
#endif
        }
    }
}

/* This call is used exclusively to navigate backwards when buildMod is enabled */

void LDrawFile::setBuildModNavBackward()
{
#ifdef QT_DEBUG_MODE
    QElapsedTimer timer;
    timer.start();
    int count = 0;
    QString keys;
#endif
    const QList _bmKeys = _buildMods.keys();
    for (const QString &modKey : _bmKeys) {
        const QList _maKeys = _buildMods[modKey]._modActions.keys();
        for (int stepIndex : _maKeys) {
            if (stepIndex > _buildModNextStepIndex && stepIndex <= _buildModPrevStepIndex) {
                int action = _buildMods[modKey]._modActions.last();
#ifdef QT_DEBUG_MODE
                int actionStepIndex = _buildMods[modKey]._modActions.lastKey();
                keys.append(QString("'%1' StepIndex: [%2], ").arg(modKey).arg(stepIndex));
                emit gui->messageSig(LOG_TRACE, QString("Get BuildMod Last Action: %1, StepIndex: %2, ActionStepIndex: %3, ModKey: '%4'")
                                                        .arg(action == BuildModApplyRc ? "Apply(64)" : "Remove(65)")
                                                        .arg(stepIndex)
                                                        .arg(actionStepIndex)
                                                        .arg(modKey));
                count++;
#endif
                if (action == BuildModRemoveRc)
                    setBuildModAction(modKey, stepIndex, BuildModApplyRc);
                else
                    setBuildModAction(modKey, stepIndex, BuildModRemoveRc);
            }
        }
    }
#ifdef QT_DEBUG_MODE
    keys.chop(1);
    emit gui->messageSig(LOG_TRACE, QString("BuildMod Jump Backward Updated %1 %2 %3 %4")
                                            .arg(count).arg(count == 1 ? "Key:" : "Keys:", keys,
                                                 Gui::elapsedTime(timer.elapsed())));
#endif
}

/* Returns index for BEGIN, APPLY and REMOVE BuildMod commands, requires valid TopOfStep */

int LDrawFile::getBuildModStepIndex(const int _modelIndex, const int _lineNumber, bool indexCheck)
{
    LogType logType = LOG_DEBUG;
    QString insert = QString("Get BuildMod");

    int modelIndex = _modelIndex;
    int lineNumber = _lineNumber;
    int stepIndex = BM_INVALID_INDEX;

    // If we are processing the first step, _buildModStepIndexes is not yet initialized.
    if (!_buildModStepIndexes.size() && !modelIndex) {
#ifdef QT_DEBUG_MODE
        insert = QString("Get BuildMod (FIRST STEP)");
#endif
        stepIndex = BM_FIRST_INDEX;

    } else if (modelIndex > BM_INVALID_INDEX) {
        QVector<int> indexKey = { modelIndex, lineNumber };
        stepIndex = _buildModStepIndexes.indexOf(indexKey);
        if (stepIndex == BM_INVALID_INDEX) {
            if (indexCheck) {
                insert = QString("Index Check BuildMod (INVALID)");
            } else {
                logType = LOG_ERROR;
                insert = QString("Get BuildMod (INVALID)");
            }
        }
    }
    else {
        logType = LOG_ERROR;
        insert = QString("Get BuildMod (INVALID)");
    }

    if (logType == LOG_ERROR)
        emit gui->messageSig(logType, QString("%1 StepIndex: %2, ModelIndex: %3, LineNumber %4, ModelName: %5")
                                              .arg(insert)
                                              .arg(stepIndex)
                                              .arg(modelIndex)
                                              .arg(lineNumber)
                                              .arg(getSubmodelName(modelIndex,false)));
#ifdef QT_DEBUG_MODE
    else
        emit gui->messageSig(logType, QString("%1 StepIndex: %2, ModelIndex: %3, LineNumber %4, ModelName: %5")
                                              .arg(insert)
                                              .arg(stepIndex)
                                              .arg(modelIndex)
                                              .arg(lineNumber)
                                              .arg(getSubmodelName(modelIndex,false)));
#endif

    return stepIndex;
}

/*  This call uses the global step index (versus the viewer StepKey) */

int LDrawFile::getBuildModStepLineNumber(int stepIndex, bool bottom)
{
    LogType logType = LOG_DEBUG;
    QString message;

    int lineNumber = 0;
    if (stepIndex  > BM_INVALID_INDEX && stepIndex < _buildModStepIndexes.size()) {
        if (bottom) {
            const int bottomStepIndex = stepIndex + 1;
            const int topModelIndex = _buildModStepIndexes.at(stepIndex).at(BM_STEP_MODEL_KEY);
            const int bottomModelIndex = _buildModStepIndexes.at(bottomStepIndex).at(BM_STEP_MODEL_KEY);
            if (bottomModelIndex != topModelIndex) // bottom of step so return number of lines
                lineNumber = size(getSubmodelName(topModelIndex));
            else
                lineNumber = _buildModStepIndexes.at(bottomStepIndex).at(BM_LINE_NUMBER);
        } else /*if (top)*/ {
            lineNumber = _buildModStepIndexes.at(stepIndex).at(BM_LINE_NUMBER);
        }
#ifdef QT_DEBUG_MODE
        message = QString("Get BuildMod %1 LineNumber: %2, StepIndex: %3, ModelName: %4")
                          .arg(bottom ? "BottomOfStep," : "TopOfStep,")
                          .arg(lineNumber).arg(stepIndex)
                          .arg(getSubmodelName(_buildModStepIndexes.at(bottom ? stepIndex + 1 : stepIndex).at(BM_MODEL_NAME),false));
#endif
    } else {
        logType = LOG_ERROR;
        message = QString("Get BuildMod (INVALID) %1 LineNumber: %2, StepIndex: %3")
                          .arg(bottom ? "BottomOfStep," : "TopOfStep,")
                          .arg(lineNumber).arg(stepIndex);
    }

    if (logType == LOG_ERROR)
        emit gui->messageSig(logType, message);
#ifdef QT_DEBUG_MODE
    else
        emit gui->messageSig(logType, message);
#endif

    return lineNumber;
}

/* Returns index for BEGIN BuildMod command, requires BuildMod Key*/

int LDrawFile::getBuildModStepIndex(const QString &buildModKey)
{
    QString modKey = buildModKey.toLower();
    QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
    if (i != _buildMods.end()) {
      return i.value()._modStepIndex;
    }

    return -1;
}

bool LDrawFile::getBuildModStepIndexWhere(const int stepIndex, QString &modelName,int &modelIndex, int &lineNumber)
{
    LogType logType = LOG_DEBUG;
    QString insert  = QString("Get BuildMod Where");

    bool validIndex = false;
    if (stepIndex > BM_INVALID_INDEX && stepIndex < _buildModStepIndexes.size()) {
        modelIndex = _buildModStepIndexes.at(stepIndex).at(BM_MODEL_NAME);
        lineNumber = _buildModStepIndexes.at(stepIndex).at(BM_LINE_NUMBER);
        modelName  = getSubmodelName(modelIndex,false);
        validIndex = ! modelName.isEmpty() && lineNumber > 0;
    } else {
        logType = LOG_ERROR;
        insert = QString("Get BuildMod (INVALID) Where");
    }

    if (logType == LOG_ERROR)
        emit gui->messageSig(logType, QString("%1 StepIndex: %2, ModelIndex: %3, LineNumber %4, ModelName: %5")
                                              .arg(insert)
                                              .arg(stepIndex)
                                              .arg(modelIndex)
                                              .arg(lineNumber)
                                              .arg(modelName));
#ifdef QT_DEBUG_MODE
    else
        emit gui->messageSig(logType, QString("%1 StepIndex: %2, ModelIndex: %3, LineNumber %4, ModelName: %5")
                                              .arg(insert)
                                              .arg(stepIndex)
                                              .arg(modelIndex)
                                              .arg(lineNumber)
                                              .arg(modelName));
#endif

  return validIndex;
}

/* Returns the first step index that contains a build modification declaration */
int LDrawFile::getBuildModFirstStepIndex()
{
    if (_buildMods.size())
        return _buildMods.first()._modStepIndex;
    return BM_INVALID_INDEX;
}

int LDrawFile::getBuildModPrevStepIndex()
{
    return _buildModPrevStepIndex;
}

int LDrawFile::getBuildModNextStepIndex()
{
    int lineNumber  = 0;
    LogType logType = LOG_DEBUG;
    QString message;
#ifdef QT_DEBUG_MODE
    bool validIndex = false;
    bool firstIndex = false;
#endif

    int stepIndex   = BM_FIRST_INDEX;

    if (!_buildModStepIndexes.size()) {

        skipHeader(getSubmodelName(0), lineNumber);

#ifdef QT_DEBUG_MODE
        firstIndex = _buildModNextStepIndex == stepIndex;
        validIndex = true;
#endif
    } else if (_buildModNextStepIndex > BM_INVALID_INDEX && _buildModStepIndexes.size() > _buildModNextStepIndex) {

        stepIndex  = _buildModNextStepIndex;
#ifdef QT_DEBUG_MODE
        validIndex = stepIndex != BM_INVALID_INDEX;
#endif
    } else {
#ifdef QT_DEBUG_MODE
        stepIndex = BM_INVALID_INDEX;
        validIndex = false;
#endif
        logType = LOG_ERROR;
        message = QString("Get BuildMod (INVALID) StepIndex: %1")
                          .arg(_buildModNextStepIndex);
    }

    if (logType == LOG_ERROR) {
        emit gui->messageSig(logType, message);
    }
#ifdef QT_DEBUG_MODE
    else
    {
        if (validIndex) {
            QVector<int> topOfStep = { 0,0 };

            if (!firstIndex) {
                topOfStep     = _buildModStepIndexes.at(_buildModNextStepIndex);
                lineNumber    = topOfStep.at(BM_LINE_NUMBER);
            }

            QString modelName = getSubmodelName(topOfStep.at(BM_MODEL_NAME),false);
            QString insert    = firstIndex ? "First" : "Next";
            validIndex        = !modelName.isEmpty() && lineNumber > BM_BEGIN_LINE_NUM;

            if (!validIndex) {
                insert.prepend("(INVALID) ");
                if (modelName.isEmpty())
                    modelName = "undefined";
                if (lineNumber == -1)
                    lineNumber = BM_BEGIN_LINE_NUM;
            }
            message = QString("Get BuildMod %1 "
                              "StepIndex: %2, "
                              "PrevStepIndex: %3, "
                              "ModelName: %4, "
                              "LineNumber: %5, "
                              "Result: %6")
                              .arg(insert)
                              .arg(_buildModNextStepIndex)
                              .arg(_buildModPrevStepIndex)
                              .arg(modelName)
                              .arg(lineNumber)
                              .arg(validIndex ? "OK" : "KO");
        }

        emit gui->messageSig(logType, message);
    }
#endif

    return stepIndex;

}

bool LDrawFile::setBuildModNextStepIndex(const QString &modelName, const int &lineNumber)
{
    int  newStepIndex = BM_INVALID_INDEX;

    if (!_buildModStepIndexes.size() && modelName == topLevelFile())
        newStepIndex = BM_FIRST_INDEX;
    else
        newStepIndex = getStepIndex(modelName,lineNumber);

    bool validIndex  = newStepIndex > BM_INVALID_INDEX;
    bool firstIndex  = _buildModNextStepIndex == BM_INVALID_INDEX;
    bool nextIndex   = validIndex && !firstIndex;

    _buildModPrevStepIndex = nextIndex ? _buildModNextStepIndex : BM_FIRST_INDEX;
    _buildModNextStepIndex = newStepIndex;

#ifdef QT_DEBUG_MODE
    emit gui->messageSig(LOG_TRACE, QString("Set BuildMod %1 "
                                            "StepIndex: %2, "
                                            "PrevStepIndex: %3, "
                                            "ModelName: %4, "
                                            "LineNumber: %5, "
                                            "Result: %6")
                                            .arg(firstIndex ? "First" : "Next")
                                            .arg(_buildModNextStepIndex)
                                            .arg(_buildModPrevStepIndex)
                                            .arg(modelName)
                                            .arg(lineNumber)
                                            .arg(validIndex ? "OK" : "KO"));
#endif

    return validIndex;
}

int LDrawFile::getStepIndex(const QString &modelName, const int &lineNumber)
{
    int modelIndex = getSubmodelIndex(modelName);
    int stepIndex  = BM_INVALID_INDEX;

    if (!_buildModStepIndexes.size() && !modelIndex)
        stepIndex = BM_FIRST_INDEX;
    else {
        QVector<int> topOfStep = { modelIndex, lineNumber };
        stepIndex = _buildModStepIndexes.indexOf(topOfStep);
        if (stepIndex == BM_INVALID_INDEX) {
            int topLineNumber = lineNumber;
            stepIndex = getTopOfStep(modelIndex, topLineNumber);
        }
    }

    return stepIndex;
}

int LDrawFile::getTopOfStep(const int &modelIndex, const int &lineNumber)
{
    if (modelIndex == BM_INVALID_INDEX) {
        emit gui->messageSig(LOG_ERROR, QObject::tr("Invalid submodel specified - undefined (%1).")
                                                    .arg(modelIndex));
        return BM_INVALID_INDEX;
    }

    int stepIndex = _buildModStepIndexes.indexOf({ modelIndex, lineNumber });

    if (stepIndex == BM_INVALID_INDEX) {
        stepIndex = _buildModStepIndexes.size() - 1;
        do {
            QVector<int> topOfStep = _buildModStepIndexes.at(stepIndex);
            if (topOfStep.at(BM_STEP_MODEL_KEY) == modelIndex) {
                if (topOfStep.at(BM_STEP_LINE_KEY) <= lineNumber) {
                    break;
                }
            }
        } while (--stepIndex >= 0);
    }

    if (stepIndex == BM_INVALID_INDEX)
        emit gui->messageSig(LOG_ERROR, QObject::tr("Could not find Step index for submodel %1 (%2) - lineNumber %3.")
                                                    .arg(getSubmodelName(modelIndex,false)).arg(modelIndex).arg(lineNumber));

    return stepIndex;
}

QString LDrawFile::getViewerStepKey(const int stepIndex)
{
    QString stepKey;
    if (stepIndex > BM_INVALID_INDEX) {
        QVector<int> topOfStep = _buildModStepIndexes.at(stepIndex);
        const int lineNumber = topOfStep.at(BM_STEP_LINE_KEY);
        const int modelIndex = topOfStep.at(BM_STEP_MODEL_KEY);
        stepKey = QString("%1;%2;0").arg(modelIndex).arg(lineNumber);

        QMap<QString, ViewerStep>::const_iterator i = _viewerSteps.constBegin();
        while (i != _viewerSteps.constEnd()) {
            bool validType = i->_viewType == Options::CSI || i->_viewType == Options::SMI;
            if (validType && i->_stepKey.modelIndex == modelIndex && i->_stepKey.lineNum == lineNumber) {
                if (i->_stepKey.stepNum) {
                    stepKey.chop(1);
                    stepKey.append(QString::number(i->_stepKey.stepNum));
                }
                if (!i->_keySuffix.isEmpty())
                    stepKey.append(i->_keySuffix);
                break;
            }
            ++i;
        }
    }
    return stepKey;
}

QString LDrawFile::getViewerStepKeyWhere(const int modelIndex, const int lineNumber)
{
    int stepIndex = getTopOfStep(modelIndex, lineNumber);

    return getViewerStepKey(stepIndex);
}

QString LDrawFile::getViewerStepKeyFromRange(const int modelIndex, const int lineNumber, const int topModelIndex, const int topLineNumber, const int bottomModelIndex, const int bottomLineNumber)
{
    int stepIndex = getTopOfStep(modelIndex, lineNumber);
    if (stepIndex != BM_INVALID_INDEX)
        return getViewerStepKey(stepIndex);

    int topStepIndex = getTopOfStep(topModelIndex, topLineNumber);
    bool ok = (topStepIndex != BM_INVALID_INDEX);
    if (!ok)
        emit gui->messageSig(LOG_WARNING, QObject::tr("Invalid top of step range specified - %1 (%2), line %3.")
                                                      .arg(getSubmodelName(modelIndex,false)).arg(topModelIndex).arg(topLineNumber));

    int bottomStepIndex = getTopOfStep(bottomModelIndex, bottomLineNumber);
    ok &= (bottomStepIndex != BM_INVALID_INDEX);
    if (!ok)
        emit gui->messageSig(LOG_WARNING, QObject::tr("Invalid bottom of step range specified - %1 (%2), line %3.")
                                                      .arg(getSubmodelName(bottomModelIndex,false)).arg(bottomModelIndex).arg(bottomLineNumber));

    if (ok && topModelIndex == modelIndex && topLineNumber <= lineNumber && bottomModelIndex == modelIndex)
        return getViewerStepKey(topStepIndex);
    else
        emit gui->messageSig(LOG_ERROR, QObject::tr("Step at %1 (%2), line %3 is outside specified top %4 (%5), line %6 and bottom %7 (%8), line %9 range.")
                                                    .arg(getSubmodelName(modelIndex,false)).arg(modelIndex).arg(lineNumber)
                                                    .arg(getSubmodelName(topModelIndex,false)).arg(topModelIndex).arg(topLineNumber)
                                                    .arg(getSubmodelName(bottomModelIndex,false)).arg(bottomModelIndex).arg(bottomLineNumber));

    return QString();
}

/* This function returns the buildModKey for the specified topOfStep */

QString LDrawFile::getBuildModKey(const QString &modelName, const int &lineNumber)
{
    int stepIndex = getStepIndex(modelName, lineNumber);
    QMultiMap<int, BuildModStep>::iterator i = _buildModSteps.find(stepIndex);
    while (i != _buildModSteps.end()) {
        if (i.key() == stepIndex && i.value()._buildModStepIndex == stepIndex)
            return i.value()._buildModKey;
        ++i;
    }
    return QString();
}

/* This function returns the equivalent of the ViewerStepKey */

QString LDrawFile::getBuildModStepKey(const QString &buildModKey)
{
  QString modKey = buildModKey.toLower();
  QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
  if (i != _buildMods.end()) {
    QString stepKey = QString("%1;%2;%3")
                              .arg(i.value()._modAttributes.at(BM_MODEL_NAME_INDEX))
                              .arg(i.value()._modAttributes.at(BM_MODEL_LINE_NUM))
                              .arg(i.value()._modAttributes.at(BM_MODEL_STEP_NUM));
#ifdef QT_DEBUG_MODE
    emit gui->messageSig(LOG_DEBUG, QString("Get BuildMod StepKey: %1").arg(stepKey));
#endif
    return stepKey;
  }

  return QString();
}

QString LDrawFile::getBuildModStepKeyModelName(const QString &buildModKey)
{
  QString modKey = buildModKey.toLower();
  QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
  if (i != _buildMods.end() && i.value()._modAttributes.size() > BM_MODEL_NAME_INDEX) {
    int index = i.value()._modAttributes.at(BM_MODEL_NAME_INDEX);
    return getSubmodelName(index);
  }

  return QString();
}

int LDrawFile::getBuildModStepKeyLineNum(const QString &buildModKey)
{
  QString modKey = buildModKey.toLower();
  QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
  if (i != _buildMods.end() && i.value()._modAttributes.size() > BM_MODEL_LINE_NUM) {
    return i.value()._modAttributes.at(BM_MODEL_LINE_NUM);
  }

  return 0;
}

int LDrawFile::getBuildModStepKeyStepNum(const QString &buildModKey)
{
  QString modKey = buildModKey.toLower();
  QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
  if (i != _buildMods.end() && i.value()._modAttributes.size() > BM_MODEL_STEP_NUM) {
    return i.value()._modAttributes.at(BM_MODEL_STEP_NUM);
  }

  return 0;
}

int LDrawFile::getBuildModStepKeyModelIndex(const QString &buildModKey)
{
  QString modKey = buildModKey.toLower();
  QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
  if (i != _buildMods.end() && i.value()._modAttributes.size() > BM_MODEL_NAME_INDEX) {
    int index = i.value()._modAttributes.at(BM_MODEL_NAME_INDEX);
    return index;
  }

  return 0;
}

/* return paths - using buildMod key - to the end of the submodel and set parent submodels and viewer steps to modified */

QStringList LDrawFile::getPathsFromBuildModKeys(const QStringList &buildModKeys)
{
#ifdef QT_DEBUG_MODE
    int subModelStackCount = 0;
#endif
  QStringList imageFilePaths;
  for (const QString &buildModKey : buildModKeys) {
    const QString modKey = buildModKey.toLower();
    QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
    if (i != _buildMods.end() && i.value()._modAttributes.size() > BM_MODEL_STEP_NUM) {
      ViewerStep::StepKey viewerStepKey = { i.value()._modAttributes.at(BM_MODEL_NAME_INDEX),
                                            i.value()._modAttributes.at(BM_MODEL_LINE_NUM),
                                            i.value()._modAttributes.at(BM_MODEL_STEP_NUM) };

      setModified(getSubmodelName(viewerStepKey.modelIndex), true);
      if (i.value()._modSubmodelStack.size()) {
        const QVector<int> _smStack = i.value()._modSubmodelStack;
        for (int index : _smStack)
          setModified(getSubmodelName(index), true);
      }
#ifdef QT_DEBUG_MODE
      subModelStackCount = i.value()._modSubmodelStack.size() + 1;
#endif

      QMap<QString, ViewerStep>::iterator si = _viewerSteps.begin();
      while (si != _viewerSteps.end()) {
        if (viewerStepKey.modelIndex == si->_stepKey.modelIndex && si->_viewType == Options::CSI) {
          if (viewerStepKey.stepNum <= si->_stepKey.stepNum) {
            if (modified(getSubmodelName(viewerStepKey.modelIndex))) {
              si->_modified = true;
              if (QFileInfo::exists(si->_imagePath)) {
                imageFilePaths.append(si->_imagePath);
              }
            }
          }
        }
        ++si;
      }
    }
  }

  if (imageFilePaths.size() > 1)
    imageFilePaths.removeDuplicates();

#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_DEBUG, QString("Get BuildMod Modified Subfiles: %1, Image File Paths %2, %3 %4")
                                                .arg(subModelStackCount)
                                                .arg(imageFilePaths.size())
                                                .arg(buildModKeys.size() == 1 ? "ModelKey:" : "ModelKeys:")
                                                .arg(buildModKeys.join(" ")));
#endif

  return imageFilePaths;
}

/* return the number of build mods within the submodel file */

int LDrawFile::getBuildModsCount(const QString &mcFileName)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    return i.value()._buildMods;
  }
  return 0;
}

void LDrawFile::setBuildModsCount(const QString &mcFileName, const int value)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    if (value == BM_NONE)
      i.value()._buildMods += 1;
    else
      i.value()._buildMods = value;
  }
}

bool LDrawFile::getBuildModExists(const QString &mcFileName, const QString &buildModKey)
{
    QString fileName = mcFileName.toLower();
    static QRegularExpression buildModBeginRx("^0 !?LPUB BUILD_MOD BEGIN ");
    for(QString &line : lpub->ldrawFile.contents(fileName)) {
        if (line[0] == '1')
            continue;
        if (line.contains(buildModBeginRx))
            if (line.contains(buildModKey, Qt::CaseInsensitive))
                return true;
    }
    return false;
}

bool LDrawFile::buildModContains(const QString &buildModKey)
{
  QString modKey = buildModKey;
  return _buildModList.contains(modKey, Qt::CaseInsensitive);
}

QStringList LDrawFile::getBuildModsList()
{
  return _buildModList;
}

int LDrawFile::buildModsCount()
{
  return _buildMods.size();
}

void LDrawFile::clearBuildModSteps()
{
    _buildModSteps.clear();
}

/* Visual Editor routines */

void LDrawFile::insertViewerStep(const QString     &stepKey,
                                 const QStringList &rotatedViewerContents,
                                 const QStringList &rotatedContents,
                                 const QStringList &unrotatedContents,
                                 const QString     &filePath,
                                 const QString     &imagePath,
                                 const QString     &csiKey,
                                 bool               multiStep,
                                 bool               calledOut,
                                 int                viewType)
{
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
  if (i != _viewerSteps.end()) {
    _viewerSteps.erase(i);
  }

  QString mStepKey = stepKey;

  // treat parts with '_' in the name - encode
  QString type;
  if (viewType == Options::PLI) {
    type = mStepKey.split(";").first();
    if (type.count("_"))
      mStepKey.replace(type, QString(type).replace("_", "@@"));
  }

  QStringList keyList = mStepKey.split("_"); // get _keySuffix for display submodel

  // treat parts with '_' in the name - decode
  if (!type.isEmpty()) {
    mStepKey.replace("@@","_");
    keyList[0] = type;
  }

  const QStringList keys = keyList.size() > 1 ? keyList.first().split(";") : mStepKey.split(";");

  ViewerStep viewerStep(keys,rotatedViewerContents,rotatedContents,unrotatedContents,filePath,imagePath,csiKey,multiStep,calledOut,viewType);

  viewerStep._keySuffix = keyList.size() > 1 ? QString("_%1").arg(keyList.last()) : QString();
  viewerStep._partCount = rotatedContents.size();

  _viewerSteps.insert(mStepKey,viewerStep);

#ifdef QT_DEBUG_MODE
  const QString debugMessage =
          QString("Insert %1 ViewerStep Key: '%2' [%3 %4 StepNumber: %5], Type: [%6]")
                  .arg(viewType == Options::PLI ? "PLI" : viewType == Options::CSI ? "CSI" : "SMI",
                       mStepKey,
                       viewType == Options::PLI ? QString("PartName: %1,").arg(keys.at(BM_STEP_MODEL_KEY)) :
                                                  QString("ModelIndex: %1 (%2),").arg(keys.at(BM_STEP_MODEL_KEY), getSubmodelName(keys.at(BM_STEP_MODEL_KEY).toInt(),false)),
                       viewType == Options::PLI ? QString("Colour: %1,").arg(keys.at(BM_STEP_LINE_KEY)) :
                                                  QString("LineNumber: %1,").arg(keys.at(BM_STEP_LINE_KEY)),
                       keys.at(BM_STEP_NUM_KEY),
                       calledOut ? "called out" : multiStep ? "step group" : viewType == Options::PLI ? "part" : "single step");
  emit gui->messageSig(LOG_DEBUG, debugMessage);
#endif
}

/* Viewer Step Exist */

void LDrawFile::updateViewerStep(const QString &stepKey, const QStringList &contents, bool rotated)
{
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);

  if (i != _viewerSteps.end()) {
    if (rotated)
      i.value()._rotatedViewerContents = contents;
    else
      i.value()._unrotatedContents = contents;
    i.value()._partCount = 0;
    for (const QString &line : contents)
      if (line[0] == '1')
        i.value()._partCount++;
    i.value()._modified = true;
  }
}

/* return viewer step Content line */

QString LDrawFile::getViewerStepContentLine(const QString &stepKey, const int lineTypeIndex, bool rotated, bool relative)
{
  if (lineTypeIndex == BM_INVALID_INDEX)
      return QString();

  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
  if (i != _viewerSteps.end()) {
    if (rotated) {
      if (i.value()._rotatedContents.size()) {
        if (relative) {
          if (i.value()._rotatedContents.size() > lineTypeIndex)
            return i.value()._rotatedContents.at(lineTypeIndex);
        } else {
          for (int j = 0; j < i.value()._rotatedContents.size(); ++j)
            if (j == lineTypeIndex)
              return i.value()._rotatedContents.at(j);
        }
      }
    } else {
      if (i.value()._unrotatedContents.size()) {
        if (relative) {
          if (i.value()._unrotatedContents.size() > lineTypeIndex)
            return i.value()._unrotatedContents.at(lineTypeIndex);
        } else {
          for (int j = 0; j < i.value()._unrotatedContents.size(); ++j)
            if (j == lineTypeIndex)
              return i.value()._unrotatedContents.at(j);
        }
      }
    }
  }
  return QString();
}

/* return viewer step Contents. */

QStringList LDrawFile::getViewerStepContents(const QString &stepKey)
{
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
  if (i != _viewerSteps.end()) {
    return i.value()._rotatedContents;
  }
  return _emptyList;
}

/* return viewer step rotatedContents - Called by viewer project*/

QStringList LDrawFile::getViewerStepRotatedContents(const QString &stepKey)
{
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
  if (i != _viewerSteps.end()) {
    return i.value()._rotatedViewerContents;
  }
  return _emptyList;
}

/* return viewer step unrotatedContents */

QStringList LDrawFile::getViewerStepUnrotatedContents(const QString &stepKey)
{
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
  if (i != _viewerSteps.end()) {
    return i.value()._unrotatedContents;
  }
  return _emptyList;
}

/* return viewer step file path */

QString LDrawFile::getViewerStepFilePath(const QString &stepKey)
{
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
  if (i != _viewerSteps.end()) {
    return i.value()._filePath;
  }
  return _emptyString;
}

/* return paths - using viewer step key - to the end of the submodel and set parent submodels and viewer steps to modified */

QStringList LDrawFile::getPathsFromViewerStepKey(const QString &stepKey)
{
  QStringList list = stepKey.split(";");
  if (list.size() == BM_SUBMODEL_STACK) {
    const QStringList submodelStack = list.takeFirst().split(":");
    if (submodelStack.size())
      for (const QString &index : submodelStack)
        setModified(getSubmodelName(index.toInt()),true);
  }

  ViewerStep::StepKey viewerStepKey = { list.at(BM_STEP_MODEL_KEY).toInt(),
                                        list.at(BM_STEP_LINE_KEY).toInt(),
                                        list.at(BM_STEP_NUM_KEY).toInt() };

  setModified(getSubmodelName(viewerStepKey.modelIndex), true);

  list.clear();
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.begin();
  while (i != _viewerSteps.end()) {
    if (viewerStepKey.modelIndex == i->_stepKey.modelIndex && i->_viewType == Options::CSI) {
      if (viewerStepKey.stepNum <= i->_stepKey.stepNum) {
        if (modified(getSubmodelName(viewerStepKey.modelIndex))) {
          i->_modified = true;
          if (QFileInfo::exists(i->_imagePath)) {
            list.append(i->_imagePath);
          }
        }
      }
    }
    ++i;
  }

  if (list.size() > 1)
    list.removeDuplicates();

  return list;
}

/* return viewer step image path */

QString LDrawFile::getViewerStepImagePath(const QString &stepKey)
{
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
  if (i != _viewerSteps.end()) {
    return i.value()._imagePath;
  }
  return _emptyString;
}

/* return viewer step CSI key */

QString LDrawFile::getViewerConfigKey(const QString &stepKey)
{
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
  if (i != _viewerSteps.end()) {
    return i.value()._csiKey;
  }
  return _emptyString;
}

/* return viewer step part count */

int LDrawFile::getViewerStepPartCount(const QString &stepKey)
{
    QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
    if (i != _viewerSteps.end()) {
      return i.value()._partCount;
    }
    return 0;
}

/* Viewer Step Exist */

bool LDrawFile::viewerStepContentExist(const QString &stepKey)
{
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);

  if (i != _viewerSteps.end()) {
    return true;
  }
  return false;
}

/* Delete Viewer Step */

bool LDrawFile::deleteViewerStep(const QString &stepKey)
{
    QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
    if (i != _viewerSteps.end()) {
        _viewerSteps.erase(i);
        return true;
    }
    return false;
}

/* return viewer step is multiStep */

bool LDrawFile::isViewerStepMultiStep(const QString &stepKey)
{
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
  if (i != _viewerSteps.end()) {
    return i.value()._multiStep;
  } else {
    return false;
  }
}

/* return viewer step is calledOut */

bool LDrawFile::isViewerStepCalledOut(const QString &stepKey)
{
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
  if (i != _viewerSteps.end()) {
    return i.value()._calledOut;
  } else {
    return false;
  }
}

/* return viewer step is currently performing a build modification action  */

bool LDrawFile::getViewerStepHasBuildModAction(const QString &stepKey)
{
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
  if (i != _viewerSteps.end()) {
    return i.value()._hasBuildModAction;
  } else {
    return false;
  }
}

bool LDrawFile::setViewerStepHasBuildModAction(const QString &stepKey, bool value)
{
  bool rc = false;
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
  if ((rc = i != _viewerSteps.end())) {
    i.value()._hasBuildModAction = value;
#ifdef QT_DEBUG_MODE
    int viewType = i.value()._viewType;
    const QString debugMessage = QString("Set %1 ViewerStep Key: '%2', ModelIndex: %3 (%4), LineNumber: %5, StepNumber: %6, HasBuildModAction: %7.")
                                         .arg(viewType == Options::PLI ? "PLI" : viewType == Options::CSI ? "CSI" : "SMI",
                                              stepKey)
                                         .arg(i.value()._stepKey.modelIndex)
                                         .arg(getSubmodelName(i.value()._stepKey.modelIndex,false))
                                         .arg(i.value()._stepKey.lineNum)
                                         .arg(i.value()._stepKey.stepNum)
                                         .arg(value ? "Yes" : "No");
    emit gui->messageSig(LOG_DEBUG, debugMessage);
    //qDebug() << qPrintable(QString("DEBUG: %1").arg(debugMessage));
#endif
  }
#ifdef QT_DEBUG_MODE
  else if (!stepKey.isEmpty()) {
    const QStringList Keys = stepKey.split(";");
    const QString noticeMessage = QString("Cannot set ViewerStep BuildMod Action for Key: '%5', ModelIndex: %1 (%2), LineNumber: %3, StepNumber: %4. Viewer Step/Key does not exist.")
                                          .arg(Keys.at(BM_STEP_MODEL_KEY),
                                               getSubmodelName(Keys.at(BM_STEP_MODEL_KEY).toInt(),false),
                                               Keys.at(BM_STEP_LINE_KEY),
                                               Keys.at(BM_STEP_NUM_KEY),
                                               stepKey);
    //emit gui->messageSig(LOG_NOTICE, noticeMessage);
    qDebug() << qPrintable(QString("DEBUG: %1").arg(noticeMessage));
  }
#endif
  return rc;
}

bool LDrawFile::viewerStepContentChanged(const QString &stepKey, const QStringList &unrotatedContents)
{
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
  if (i != _viewerSteps.end()) {
    QByteArray newByteArray, oldByteArray;
    QDataStream newContent(&newByteArray, QIODevice::WriteOnly);
    newContent << unrotatedContents;

    QDataStream oldContent(&oldByteArray, QIODevice::WriteOnly);
    oldContent << i.value()._unrotatedContents;

    return newByteArray != oldByteArray;
  }
  return false;
}

bool LDrawFile::viewerStepModified(const QString &stepKey, bool reset)
{
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
  if (i != _viewerSteps.end()) {
    bool modified = i.value()._modified;
    if (reset)
      i.value()._modified = false;
#ifdef QT_DEBUG_MODE
    const QStringList keys = stepKey.split(";");
    int viewType = i.value()._viewType;
    const QString debugMessage =
            QString("%1%2 ViewerStep Key: '%3' [%4 %5 StepNumber: %6] %7")
                    .arg(reset && modified ? "Reset " : "",
                         viewType == Options::PLI ? "PLI" : viewType == Options::CSI ? "CSI" : "SMI",
                         stepKey,
                         viewType == Options::PLI ? QString("PartName: %1,").arg(keys.at(BM_STEP_MODEL_KEY)) :
                                                    QString("ModelIndex: %1 (%2),").arg(keys.at(BM_STEP_MODEL_KEY), getSubmodelName(keys.at(BM_STEP_MODEL_KEY).toInt(),false)),
                         viewType == Options::PLI ? QString("Colour: %1,").arg(keys.at(BM_STEP_LINE_KEY)) :
                                                    QString("LineNumber: %1,").arg(keys.at(BM_STEP_LINE_KEY)),
                         keys.at(BM_STEP_NUM_KEY),
                         i.value()._modified ? ", Modified: [Yes]." :", Modified: [No].");
    emit gui->messageSig(LOG_DEBUG, debugMessage);
#endif
    return modified;
  } else {
    return false;
  }
}

void LDrawFile::setViewerStepModified(const QString &stepKey)
{
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
  if (i != _viewerSteps.end()) {
    i.value()._modified = true;
#ifdef QT_DEBUG_MODE
    int viewType = i.value()._viewType;
    const QString debugMessage =
            QString("Set %1 ViewerStep Key: '%2', ModelIndex: %3 (%4), LineNumber: %5, StepNumber: %6, Modified: [Yes].")
                    .arg(viewType == Options::PLI ? "PLI" : viewType == Options::CSI ? "CSI" : "SMI",
                         stepKey)
                    .arg(i.value()._stepKey.modelIndex)
                    .arg(getSubmodelName(i.value()._stepKey.modelIndex,false))
                    .arg(i.value()._stepKey.lineNum)
                    .arg(i.value()._stepKey.stepNum);
    emit gui->messageSig(LOG_DEBUG, debugMessage);
#endif
  }
#ifdef QT_DEBUG_MODE
  else {
    const QStringList Keys = stepKey.split(";");
    const QString warnMessage =
            QString("Cannot modify, ViewerStep  Key: '%5', ModelIndex: %1 (%2), LineNumber: %3, StepNumber: %4. Key does not exist.")
                    .arg(Keys.at(BM_STEP_MODEL_KEY),
                         getSubmodelName(Keys.at(BM_STEP_MODEL_KEY).toInt(),false),
                         Keys.at(BM_STEP_LINE_KEY),
                         Keys.at(BM_STEP_NUM_KEY),
                         stepKey);
    emit gui->messageSig(LOG_WARNING, warnMessage);
    //qDebug() << qPrintable(QString("WARNING: %1").arg(warnMessage));
  }
#endif
}

/* Clear ViewerSteps */

void LDrawFile::clearViewerSteps()
{
  _viewerSteps.clear();
}

void LDrawFile::skipHeader(const QString &modelName, int &lineNumber)
{
    int numLines = size(modelName);
    for ( ; lineNumber < numLines; lineNumber++) {
        QString line = readLine(modelName,lineNumber);
        if (line.isEmpty())
            continue;
        int p;
        for (p = 0; p < line.size(); ++p) {
            if (line[p] != ' ') {
                break;
            }
        }
        if (line[p] >= '1' && line[p] <= '5') {
            if (lineNumber > 0) {
                --lineNumber;
            }
            break;
        } else if ( ! isHeader(line)) {
            if (lineNumber != 0) {
                --lineNumber;
                break;
            }
        }
    }
}

// -- -- Utility Functions -- -- //

int split(const QString &line, QStringList &argv)
{
  QString     chopped = line;
  int         p = 0;
  int         length = chopped.length();

  // line length check
  if (p == length) {
      return 0;
    }
  // eol check
  while (chopped[p] == ' ') {
      if (++p == length) {
          return -1;
        }
    }

  argv.clear();

  // if line starts with 1 (part line)
  if (chopped[p] == '1') {

      // line length check
      argv << "1";
      p += 2;
      if (p >= length) {
          return -1;
        }
      // eol check
      while (chopped[p] == ' ') {
          if (++p >= length) {
              return -1;
            }
        }

      // color x y z a b c d e f g h i //

      // populate argv with part line tokens
      for (int i = 0; i < 13; i++) {
          QString token;

          while (chopped[p] != ' ') {
              token += chopped[p];
              if (++p >= length) {
                  return -1;
                }
            }
          argv << token;
          while (chopped[p] == ' ') {
              if (++p >= length) {
                  return -1;
                }
            }
        }

      argv << chopped.mid(p);

      if (argv.size() > 1 && argv[1] == "WRITE") {
          argv.removeAt(1);
        }

    } else if (chopped[p] >= '2' && chopped[p] <= '5') {
      chopped = chopped.mid(p);
      argv << chopped.split(" ", SkipEmptyParts);
    } else if (chopped[p] == '0') {

      /* Parse the input line into argv[] */

      int soq = validSoQ(chopped,chopped.indexOf("\""));
      if (soq == -1) {
          argv << chopped.split(" ", SkipEmptyParts);
        } else {
          // quotes found
          while (chopped.size()) {
              soq = validSoQ(chopped,chopped.indexOf("\""));
              if (soq == -1) {
                  argv << chopped.split(" ", SkipEmptyParts);
                  chopped.clear();
                } else {
                  QString left = chopped.left(soq);
                  left = left.trimmed();
                  argv << left.split(" ", SkipEmptyParts);
                  chopped = chopped.mid(soq+1);
                  soq = validSoQ(chopped,chopped.indexOf("\""));
                  if (soq == -1) {
                      argv << left;
                      return -1;
                    }
                  argv << chopped.left(soq);
                  chopped = chopped.mid(soq+1);
                  if (chopped == "\"") {
                    }
                }
            }
        }

      if (argv.size() > 1 && argv[0] == "0" && argv[1] == "GHOST") {
          argv.removeFirst();
          argv.removeFirst();
        }
    }

  return 0;
}

// check for escaped quotes
int validSoQ(const QString &line, int soq) {

  int nextq;
//  logTrace() << "\n  A. START VALIDATE SoQ"
//             << "\n SoQ (at Index):   " << soq
//             << "\n Line Content:     " << line;
  if(soq > 0 && line.at(soq-1) == '\\') {
      nextq = validSoQ(line,line.indexOf("\"",soq+1));
      soq = nextq;
    }
//  logTrace() << "\n  D. END VALIDATE SoQ"
//             << "\n SoQ (at Index):   " << soq;
  return soq;
}

LDrawFile::LDrawFile()
#if QT_VERSION < QT_VERSION_CHECK(5,14,0)
  : ldrawMutex(QMutex::Recursive)
#endif
{
  {
    _fileRegExp
        << QRegularExpression("^0\\s+FILE\\s?(.*)$", QRegularExpression::CaseInsensitiveOption)       // SOF_RX - Start of File
        << QRegularExpression("^0\\s+!?DATA\\s+(.*)$", QRegularExpression::CaseInsensitiveOption)     // DAT_RX - Imbedded Image Data
        << QRegularExpression("^0\\s+!:\\s+(.*)$", QRegularExpression::CaseInsensitiveOption)         // B64_RX - Base 64 Image Data Line
        << QRegularExpression("^0\\s+NOFILE\\s*$", QRegularExpression::CaseInsensitiveOption)         // EOF_RX - End of File
        << QRegularExpression("^1\\s+.*$", QRegularExpression::CaseInsensitiveOption)                 // LDR_RX - LDraw File
        << QRegularExpression("^0\\s+(.*)$", QRegularExpression::CaseInsensitiveOption)               // DES_RX - Model Description
        << QRegularExpression("^0\\s+NAME:\\s+(.*)$", QRegularExpression::CaseInsensitiveOption)      // NAM_RX - Name Header
        << QRegularExpression("^0\\s+NAME:\\s*$", QRegularExpression::CaseInsensitiveOption)          // NAK_RX - Name Header Key
        << QRegularExpression("^0\\s+AUTHOR:\\s+(.*)$", QRegularExpression::CaseInsensitiveOption)    // AUT_RX - Author Header
        << QRegularExpression("^0\\s+AUTHOR:\\s*$", QRegularExpression::CaseInsensitiveOption)        // AUK_RX - Author Header Key
        << QRegularExpression("^0\\s+!?CATEGORY\\s+(.*)$", QRegularExpression::CaseInsensitiveOption) // CAT_RX - Category Header
        << QRegularExpression("^0\\s+!?LPUB\\s+INCLUDE\\s+[\"']?([^\"']*)[\"']?$", QRegularExpression::CaseInsensitiveOption)                          // INC_RX - Include File
        << QRegularExpression("^0\\s+!?LDCAD\\s+GROUP_DEF.*\\s+\\[LID=(\\d+)\\]\\s+\\[GID=([\\d\\w]+)\\]\\s+\\[name=(.[^\\]]+)\\].*$", QRegularExpression::CaseInsensitiveOption) // LDG_RX - LDCad Group
        << QRegularExpression("^0\\s+!?LDCAD\\s+(CONTENT|PATH_POINT|PATH_SKIN|GENERATED)[^\n]*")                                                           // LDC_RX - LDCad Generated Content
        << QRegularExpression("^[0-5]\\s+!?(?:LPUB)*\\s?(?:STEP|ROTSTEP|MULTI_STEP BEGIN|CALLOUT BEGIN|BUILD_MOD BEGIN|ROTATION|\\d)[^\n]*")               // EOH_RX - End of Header
        << QRegularExpression("^0\\s+!?(?:LPUB)*\\s?(INSERT DISPLAY_MODEL)[^\n]*")                                              // DMS_RX - Display Model Step
        << QRegularExpression("^0\\s+!?(?:LPUB)*\\s?(STEP|ROTSTEP|NOSTEP|NOFILE)[^\n]*")                                        // LDS_RX - LDraw Step boundry
        << QRegularExpression("(?:FADE_STEPS|HIGHLIGHT_STEP)\\s+(SETUP|ENABLED)\\s*(GLOBAL|LOCAL)?\\s*TRUE[^\n]*")              // FHE_RX - Fade or Highlight Enabled (or Setup)
        << QRegularExpression("(?:FADE_STEPS|HIGHLIGHT_STEP)\\s+(LPUB_FADE|LPUB_HIGHLIGHT)\\s*(GLOBAL|LOCAL)?\\s*TRUE[^\n]*")   // LFH_RX - LPub Fade or LPub Highlight
        ;
  }

  {
    LDrawHeaderRegExp
        << QRegularExpression("^0\\s+FILE\\s+(.+)$", QRegularExpression::CaseInsensitiveOption)
        << QRegularExpression("^0\\s+AUTHOR:?[^\n]*", QRegularExpression::CaseInsensitiveOption)
        << QRegularExpression("^0\\s+BFC[^\n]*", QRegularExpression::CaseInsensitiveOption)
        << QRegularExpression("^0\\s+!?CATEGORY[^\n]*", QRegularExpression::CaseInsensitiveOption)
        << QRegularExpression("^0\\s+CLEAR[^\n]*", QRegularExpression::CaseInsensitiveOption)
        << QRegularExpression("^0\\s+!?COLOUR[^\n]*", QRegularExpression::CaseInsensitiveOption)
        << QRegularExpression("^0\\s+!?CMDLINE[^\n]*", QRegularExpression::CaseInsensitiveOption)
        << QRegularExpression("^0\\s+!?HELP[^\n]*", QRegularExpression::CaseInsensitiveOption)
        << QRegularExpression("^0\\s+!?HISTORY[^\n]*", QRegularExpression::CaseInsensitiveOption)
        << QRegularExpression("^0\\s+!?KEYWORDS[^\n]*", QRegularExpression::CaseInsensitiveOption)
        << QRegularExpression("^0\\s+!?LDRAW_ORG[^\n]*", QRegularExpression::CaseInsensitiveOption)
        << QRegularExpression("^0\\s+!?LICENSE[^\n]*", QRegularExpression::CaseInsensitiveOption)
        << QRegularExpression("^0\\s+NAME:?[^\n]*", QRegularExpression::CaseInsensitiveOption)
        << QRegularExpression("^0\\s+OFFICIAL[^\n]*", QRegularExpression::CaseInsensitiveOption)
        << QRegularExpression("^0\\s+ORIGINAL\\s+LDRAW[^\n]*", QRegularExpression::CaseInsensitiveOption)
        << QRegularExpression("^0\\s+PAUSE[^\n]*", QRegularExpression::CaseInsensitiveOption)
        << QRegularExpression("^0\\s+PRINT[^\n]*", QRegularExpression::CaseInsensitiveOption)
        << QRegularExpression("^0\\s+ROTATION[^\n]*", QRegularExpression::CaseInsensitiveOption)
        << QRegularExpression("^0\\s+SAVE[^\n]*", QRegularExpression::CaseInsensitiveOption)
        << QRegularExpression("^0\\s+UNOFFICIAL[^\n]*", QRegularExpression::CaseInsensitiveOption)
        << QRegularExpression("^0\\s+UN-OFFICIAL[^\n]*", QRegularExpression::CaseInsensitiveOption)
        << QRegularExpression("^0\\s+WRITE[^\n]*", QRegularExpression::CaseInsensitiveOption)
        << QRegularExpression("^0\\s+~MOVED\\s+TO[^\n]*", QRegularExpression::CaseInsensitiveOption)
        << QRegularExpression("^0\\s+NOFILE\\s*$", QRegularExpression::CaseInsensitiveOption)
           ;
  }

  {
      LDrawUnofficialPartRegExp
              << QRegularExpression("^0\\s+!?UNOFFICIAL\\s+PART[^\n]*", QRegularExpression::CaseInsensitiveOption)
              << QRegularExpression("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_Part)[^\n]*", QRegularExpression::CaseInsensitiveOption)
              << QRegularExpression("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_Part Alias)[^\n]*", QRegularExpression::CaseInsensitiveOption)
              << QRegularExpression("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_Part Physical_Colour)[^\n]*", QRegularExpression::CaseInsensitiveOption)
              << QRegularExpression("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_Part Physical Colour)[^\n]*", QRegularExpression::CaseInsensitiveOption)
              << QRegularExpression("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial Part)[^\n]*", QRegularExpression::CaseInsensitiveOption)
              << QRegularExpression("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial Part Alias)[^\n]*", QRegularExpression::CaseInsensitiveOption)
              << QRegularExpression("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial Part Physical_Colour)[^\n]*", QRegularExpression::CaseInsensitiveOption)
              << QRegularExpression("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial Part Physical Colour)[^\n]*", QRegularExpression::CaseInsensitiveOption)
              ;
  }

  {
      LDrawUnofficialSubPartRegExp
              << QRegularExpression("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_Subpart)[^\n]*", QRegularExpression::CaseInsensitiveOption)
              << QRegularExpression("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial Subpart)[^\n]*", QRegularExpression::CaseInsensitiveOption)
                 ;
  }

  {
      LDrawUnofficialPrimitiveRegExp
              << QRegularExpression("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_Primitive)[^\n]*", QRegularExpression::CaseInsensitiveOption)
              << QRegularExpression("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_8_Primitive)[^\n]*", QRegularExpression::CaseInsensitiveOption)
              << QRegularExpression("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_48_Primitive)[^\n]*", QRegularExpression::CaseInsensitiveOption)
              << QRegularExpression("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial Primitive)[^\n]*", QRegularExpression::CaseInsensitiveOption)
              << QRegularExpression("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial 8_Primitive)[^\n]*", QRegularExpression::CaseInsensitiveOption)
              << QRegularExpression("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial 48_Primitive)[^\n]*", QRegularExpression::CaseInsensitiveOption)
              ;
  }

  {
      LDrawUnofficialShortcutRegExp
              << QRegularExpression("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_Shortcut)[^\n]*", QRegularExpression::CaseInsensitiveOption)
              << QRegularExpression("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_Shortcut Alias)[^\n]*", QRegularExpression::CaseInsensitiveOption)
              << QRegularExpression("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_Shortcut Physical_Colour)[^\n]*", QRegularExpression::CaseInsensitiveOption)
              << QRegularExpression("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_Shortcut Physical Colour)[^\n]*", QRegularExpression::CaseInsensitiveOption)
              << QRegularExpression("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial Shortcut)[^\n]*", QRegularExpression::CaseInsensitiveOption)
              << QRegularExpression("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial Shortcut Alias)[^\n]*", QRegularExpression::CaseInsensitiveOption)
              << QRegularExpression("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial Shortcut Physical_Colour)[^\n]*", QRegularExpression::CaseInsensitiveOption)
              << QRegularExpression("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial Shortcut Physical Colour)[^\n]*", QRegularExpression::CaseInsensitiveOption)
              ;
  }
}

int randomFour()
{
  int result=0;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(1, 6);
  for (int n=0; n<4; ++n)
  {
    result=result*10+dis(gen);
  }
  return result;
}

bool isHeader(const QString &line)
{
  for (int i = 0; i < LDrawHeaderRegExp.size(); i++) {
    if (line.contains(LDrawHeaderRegExp[i])) {
      return true;
    }
  }
  return false;
}

bool isComment(const QString &line) {
  static QRegularExpression commentLine("^\\s*0\\s+\\/\\/\\s*.*$");
  if (line.contains(commentLine))
    return true;
  return false;
}

QString joinLine(const QStringList &argv)
{
  QString line;
  for (int i = 0; i < argv.size(); i++) {
     QString item = argv.at(i);
     if (item.contains(" "))
       item = "\""+item+"\"";
     line += (i+1 == argv.size() ? item : item+" ");
  }
  return line;
}

/*
 * Assume extensions are up to 4 chars in length so part.lfx_01
 * is not considered as having an extension, but part.dat
 * is considered as having extensions
 */
bool isSubstitute(const QString &line, QString &lineOut)
{
  static QRegularExpression substitutePartRx("\\sBEGIN\\sSUB\\s(.*(?:\\.dat|\\.ldr)|[^.]{5})", QRegularExpression::CaseInsensitiveOption);
  QRegularExpressionMatch match = substitutePartRx.match(line);
  if (match.hasMatch()) {
    lineOut = match.captured(1);
    return true;
  }
  return false;
}

bool isSubstitute(const QString &line)
{
  QString dummy;
  return isSubstitute(line, dummy);
}

bool isGhost(const QString &line)
{
  static QRegularExpression ghostMeta("^\\s*0\\s+GHOST\\s+.*$");
  QRegularExpressionMatch match = ghostMeta.match(line);
  if (match.hasMatch())
      return true;
  return false;
}

void asynchronous(const QFuture<void> &future)
{
  QEventLoop wait;
  QFutureWatcher<void> fw;
  QObject::connect   ( &fw, &QFutureWatcher<void>::finished, &wait, &QEventLoop::quit );
  fw.setFuture(future);
  wait.exec();
  QObject::disconnect( &fw, &QFutureWatcher<void>::finished, &wait, &QEventLoop::quit );
}

int asynchronous(const QFuture<int> &future)
{
  QEventLoop wait;
  QFutureWatcher<int> fw;
  QObject::connect   ( &fw, &QFutureWatcher<int>::finished, &wait, &QEventLoop::quit );
  fw.setFuture(future);
  wait.exec();
  QObject::disconnect( &fw, &QFutureWatcher<int>::finished, &wait, &QEventLoop::quit );
  return fw.result();
}

QStringList asynchronous(const QFuture<QStringList> &future)
{
  QEventLoop wait;
  QFutureWatcher<QStringList> fw;
  QObject::connect   ( &fw, &QFutureWatcher<QStringList>::finished, &wait, &QEventLoop::quit );
  fw.setFuture(future);
  wait.exec();
  QObject::disconnect( &fw, &QFutureWatcher<QStringList>::finished, &wait, &QEventLoop::quit );
  return fw.result();
}

int getUnofficialFileType(QString &line)
{
  static QRegularExpression unnoffTypeRx("^0\\s+!?(LDCAD GENERATED)[^\n]*");
  QRegularExpressionMatch match = unnoffTypeRx.match(line);
  if (match.hasMatch()) {
    return UNOFFICIAL_GENERATED_PART;
  }
  int size = LDrawUnofficialPartRegExp.size();
  for (int i = 0; i < size; i++) {
    if (line.contains(LDrawUnofficialPartRegExp[i])) {
      return UNOFFICIAL_PART;
    }
  }
  size = LDrawUnofficialSubPartRegExp.size();
  for (int i = 0; i < size; i++) {
    if (line.contains(LDrawUnofficialSubPartRegExp[i])) {
      return UNOFFICIAL_SUBPART;
    }
  }
  size = LDrawUnofficialPrimitiveRegExp.size();
  for (int i = 0; i < size; i++) {
    if (line.contains(LDrawUnofficialPrimitiveRegExp[i])) {
      return UNOFFICIAL_PRIMITIVE;
    }
  }
  size = LDrawUnofficialShortcutRegExp.size();
  for (int i = 0; i < size; i++) {
    if (line.contains(LDrawUnofficialShortcutRegExp[i])) {
      return UNOFFICIAL_SHORTCUT;
    }
  }
  if (line.contains(LDrawFile::_fileRegExp[DAT_RX]))
    return UNOFFICIAL_DATA;

  return UNOFFICIAL_UNKNOWN;
}
