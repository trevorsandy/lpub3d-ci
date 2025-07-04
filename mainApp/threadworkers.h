/****************************************************************************
**
** Copyright (C) 2015 - 2025 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public Liceense (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef THREADWORKERS_H
#define THREADWORKERS_H

#include <QFile>
#include <QList>
#include <QMap>
#include <QObject>
#include <QElapsedTimer>
#include <QThread>
#include <QFuture>

#include "ldrawfiles.h"
#include "options.h"
#include "archiveparts.h"
#include "ldsearchdirs.h"
#include "declarations.h"
#include "quazip.h"

class PartWorker;
class ColourPart;

enum partTypeDir{
    LD_PARTS,
    LD_SUB_PARTS,
    LD_PRIMITIVES,
    LD_PRIMITIVES_8,
    LD_PRIMITIVES_48
};

enum headerOption{
    FADESTEP_INTRO_HEADER,
    FADESTEP_FILE_HEADER,
    FADESTEP_COLOUR_PARTS_HEADER,
    FADESTEP_COLOUR_CHILDREN_PARTS_HEADER,
    FADESTEP_FILE_STATUS
};

class ColourPart {
public:
    QStringList _contents;                        // from retrieveContents
    QString     _fileNameStr;                     // from parseParent
    int         _partType;                        // from retrieveContent
    bool        _unOff;

    ColourPart(
            const QStringList   &contents,        // file contents
            const QString       &fileNameStr,     // file name as defined in part content
            const int           &partType,
            const bool          &unOff);

    ~ColourPart()
    {
        _contents.clear();
    }
};

class PartWorker: public QObject
{
   Q_OBJECT

public:
    explicit PartWorker(bool onDemand = false, QObject *parent = nullptr);
    PartWorker(QString archiveFile, QObject *parent = nullptr);
    ~PartWorker()
    {
        _colourParts.clear();
        _ldrawStaticColourParts.clear();
        _partFileContents.clear();

    }

    void insert(
            const QStringList   &contents,
            const QString       &fileNameStr,
            const int           &partType,
            const bool          &unOff);

    bool partAlreadyInList(
        const QString           &fileNameStr);

    void remove(
        const QString           &fileNameStr);

    bool endThreadEventLoopNow();

    void setDoFadeStep(
        bool                     doFadeStep)
        {_doFadeStep           = doFadeStep;}

    void setDoHighlightStep(
        bool                     doHighlightStep)
        {_doHighlightStep      = doHighlightStep;}

    void resetSearchDirSettings()
    {
      _resetSearchDirSettings = true;
      ldsearchDirPreferences();
    }

    void populateUpdateSearchDirs(const QStringList &searchDirs)
    {
      _updateLDSearchDirs = searchDirs;
    }

    QStringList &getUpdateSearchDirs()
    {
      return _updateLDSearchDirs;
    }

    void ldsearchDirPreferences();

    bool loadLDrawSearchDirs();

    void updateLDSearchDirs(bool archive = false, bool custom = false);

    void addCustomDirs();

    void removeCustomDirs();

    QStringList                _partList;

public slots:
     void updateLDSearchDirsParts();

     void processFadeColourParts(bool overwrite, bool setup);      // scan LDraw library files for static colored parts and create fade copy

     void processHighlightColourParts(bool overwrite, bool setup); // scan LDraw library files for static colored parts and create highlight copy

     void processLDSearchDirParts();

     void populateLdgLiteSearchDirs();

     void requestEndThreadNow();

     void processPartsArchive();

signals:
     void progressBarInitSig();

     void progressMessageSig(
            const QString       &text);

     void progressRangeSig(
             const int          &min,
             const int          &max);

     void progressSetValueSig(
             const int          &value);

     void progressResetSig();

     void progressStatusRemoveSig();

     void customColourFinishedSig();

     void partsArchiveFinishedSig();

     void partsArchiveResultSig(int);

     //2 below not used
     void requestFinishSig();
     void ldSearchDirFinishedSig();

private:
    void processCustomColourParts(PartType partType,
                                  bool     overwrite = false,
                                  bool     setup = false);

    bool endThreadNotRequested(){ return ! _endThreadNowRequested; }

    QStringList contents(
        const QString       &fileNameStr);

    int  size(const QString &fileNameStr);

    int size(){return       _partList.size();}

   void empty();

   bool saveCustomFile(
        const QString        &fileName,
        const QStringList    &customPartContent);

   bool createCustomPartFiles(
        const PartType       partType,
        bool                 overwriteCustomParts = false); // convert static color files // replace color code with fade color or highlight edge color

   bool processColourParts(
       const QStringList      &colourPartList,
       const PartType         partType);

   bool processPartsArchive(
       const QStringList     &ldPartsDirs,
       const QString         &comment = QString(),
              bool           overwriteCustomParts = false);

   bool doFadeStep()
      {return                _doFadeStep;}

   bool doHighlightStep()
      {return                _doHighlightStep;}

   bool okToEmitToProgressBar();

   bool                      _endThreadNowRequested;
   QMap<QString, ColourPart> _colourParts;
   QStringList               _emptyList;
   QString                   _emptyString;
   QStringList               _ldrawStaticColourParts;
   QStringList               _partFileContents;
   QStringList               _excludedSearchDirs;
   QStringList               _saveLDSearchDirs;
   QStringList               _updateLDSearchDirs;
   QString                   _customPartDir;
   QString                   _customPrimDir;
   QString                   _lsynthPartsDir;
   QString                   _helperPartsDir;
   QString                   _ldrawModelsDir;
   QString                   _ldrawCustomArchive;
   QString                   _ldSearchDirsKey;
   QString                   _ldrawArchiveFile;
   QElapsedTimer             _timer;
   bool                      _partsArchived;
   bool                      _doFadeStep;
   bool                      _doHighlightStep;
   bool                      _resetSearchDirSettings;
   int                       _customParts;

   LDPartsDirs                ldPartsDirs;     // automatically load LDraw.ini parameters
   ArchiveParts               archiveParts;    // add contents to unofficial zip archive (for Visual Editor)
};


class ColourPartListWorker: public QObject
{
   Q_OBJECT

public:
    explicit ColourPartListWorker(QObject *parent = nullptr);
    ~ColourPartListWorker()
    {
        _colourParts.clear();
        _ldrawStaticColourParts.clear();
        _partFileContents.clear();
    }

    void insert(
            const QStringList   &contents,
            const QString       &fileNameStr,
            const int           &partType,
            const bool          &unOff);

    bool partAlreadyInList(
            const QString       &fileNameStr);

    void remove(
            const QString       &fileNameStr);

    bool endThreadEventLoopNow();

    QStringList                 _partList;

public slots:
     void                       generateCustomColourPartsList();
     void                       requestEndThreadNow();

signals:
     void progressBarInitSig();

     void progressMessageSig(
            const QString       &text);

     void progressRangeSig(
             const int          &min,
             const int          &max);

     void progressSetValueSig(
             const int          &value);

     void progressResetSig();

     void messageSig(
            LogType             logType,
            QString             message);

     void progressStatusRemoveSig();
     void colourPartListFinishedSig();
     void colorPartsListResultSig(int);

     //not used
     void requestFinishSig();

private:
    bool                      _endThreadNowRequested;
    QMap<QString, ColourPart> _colourParts;
    QStringList               _emptyList;
    QString                   _emptyString;
    int                       _cpLines;
    int                       _colWidthFileName;

    QStringList               _ldrawStaticColourParts;
    QStringList               _partFileContents;
    QElapsedTimer             _timer;
    QString                   _filePath;
    QString                   _ldrawCustomArchive;
    LDPartsDirs                ldPartsDirs;                     // automatically load LDraw.ini parameters

    bool endThreadNotRequested(){ return ! _endThreadNowRequested;}
    void processChildren();
    void writeLDrawColourPartFile(bool append = false);

    bool processArchiveParts(const QString &archiveFile);
    void processFileContents(const QString &libFileName,
                             const bool       isUnOffLib);
    void fileSectionHeader(const int &option,
                           const QString &heading = "");
};

class FindPageOptions;
class CountPageWorker : public QObject
{
    Q_OBJECT

public slots:
    static int countPage(
            Meta            *meta,
            LDrawFile       *ldrawFile,
            FindPageOptions &opts,
            QString const   &addLine);
};

class LDrawFile;
class LoadModelWorker : public QObject
{
    Q_OBJECT

public:
    explicit LoadModelWorker(bool detached = false) { _detached = detached; }
    ~LoadModelWorker() { }
public slots:
    static int loadModel(LDrawFile *, const QString &);
private:
    static void statusMessage(const LogType, const QString &);
    static void setPlainText(const QString &);
    static void setPagedContent(const QStringList &);
    static void setSubFiles(const QStringList &);
    static void setLineCount(const int);
    static bool _detached;
};

class ExtractWorker : public QObject
{
    Q_OBJECT
public:
    ExtractWorker(
    const QString &archive,
    const QString &destination);

public slots:
    void doWork();
    void requestEndWorkNow();

signals:
    void finishedSig();
    void resultSig(int);
    void setValueSig(
            const int &value);

protected:
    static bool copyData(QIODevice &inFile, QIODevice &outFile);
    bool extractDir(QuaZip &zip, const QString &dir);
    bool extractFile(QuaZip* zip, QString fileName, QString fileDest);
    bool removeFile(QStringList listFile);
    QString mArchive;
    QString mDestination;
    bool mEndWorkNow;
};

#endif // THREADWORKERS_H
