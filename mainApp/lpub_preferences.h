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
#ifndef LPUB_PREFERENCES_H
#define LPUB_PREFERENCES_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMessageBox>

enum UpdateFlag { UpdateExisting, SkipExisting };
#ifdef Q_OS_MACOS
enum LibFlag { LibValid, LibMissing, LibInvalid, LibChkError };
#endif

class Preferences
{
  public:
    enum MsgKey {
        ParseErrors,
        InsertErrors,
        IncludeFileErrors,
        BuildModErrors,
        BuildModEditErrors,
        AnnotationErrors,
        ConfigurationErrors,
        NumKeys
    };

    struct MsgID {
        MsgKey msgKey;
        QString uID;
        MsgID(){}
        MsgID(MsgKey _msgKey, QString _uID)
        {
            msgKey = _msgKey;
            uID    = _uID;
        }
        QString toString() const
        {
           return QString("%1 %2").arg(msgKey).arg(uID);
        }
        QString keyToString() const
        {
            return QString("%1").arg(msgKey);
        }
    };

    struct ThemeSettings {
        QString key;
        QString color;
        QString label;
    };

    Preferences();
    static void lpubPreferences();
    static void ldrawPreferences(bool);
    static void lpub3dLibPreferences(bool);
    static void lpub3dUpdatePreferences();
    static void viewerPreferences();
    static void lgeoPreferences();
    static void rendererPreferences();
    static void preferredRendererPreferences(bool = false);
    static void fadestepPreferences(bool = false);
    static void highlightstepPreferences(bool = false);
    static void pliPreferences();
    static void unitsPreferences();
    static void annotationPreferences();
    static void publishingPreferences();
    static void loggingPreferences();
    static void exportPreferences();
    static void themePreferences();
    static void userInterfacePreferences();
    static void editorPreferences();
    static void keyboardShortcutPreferences();

    static bool extractLDrawLib();
    static void getRequireds();
    static bool getPreferences();
    static bool getShowMessagePreference(MsgKey = ParseErrors);
    static void setLPub3DLoaded();
    static void setLDGLiteIniParams();
    static void setDistribution();
    static void updateViewerInterfaceColors();
    static void updateLDVExportIniFile(UpdateFlag);
    static void updateLDViewIniFile(UpdateFlag);
    static void updateLDViewPOVIniFile(UpdateFlag);
    static void updatePOVRayConfFile(UpdateFlag);
    static void updatePOVRayIniFile(UpdateFlag);
    static void updatePOVRayConfigFiles();
    static bool isBlender28OrLater();

    static void removeBuildModFormatPreference(bool);
    static void removeChildSubmodelFormatPreference(bool);
    static void useSystemEditorPreference(bool);
    static void recountPartsPreference(bool);
    static void setLPub3DAltLibPreferences(const QString &);
    static bool checkLDrawLibrary(const QString &);
    static bool setLDViewExtraSearchDirs(const QString &);
    static bool copyRecursively(const QString &,const QString &);

    static void setSystemTheme();
    static void setSceneGuidesPreference(bool);
    static void setSceneGuidesPositionPreference(int);
    static void setSceneGuidesLinePreference(int);
    static void setSnapToGridPreference(bool);
    static void setHidePageBackgroundPreference(bool);
    static void setShowGuidesCoordinatesPreference(bool);
    static void setShowTrackingCoordinatesPreference(bool);
    static void setGridSizeIndexPreference(int);
    static void setSceneRulerPreference(bool);
    static void setSceneRulerTrackingPreference(int);
    static void setShowSaveOnRedrawPreference(bool);
    static void setShowSaveOnUpdatePreference(bool);
    static void setSuppressFPrintPreference(bool);
    static void setCyclePageDisplay(bool);

    static void setCustomSceneBackgroundColorPreference(bool = true);
    static void setCustomSceneGridColorPreference(bool = true);
    static void setCustomSceneRulerTickColorPreference(bool = true);
    static void setCustomSceneRulerTrackingColorPreference(bool = true);
    static void setCustomSceneGuideColorPreference(bool = true);
    static void setSceneBackgroundColorPreference(QString);
    static void setSceneGridColorPreference(QString);
    static void setSceneRulerTickColorPreference(QString);
    static void setSceneRulerTrackingColorPreference(QString);
    static void setSceneGuideColorPreference(QString);
    static void setBlenderExePathPreference(QString);
    static void setBlenderVersionPreference(QString);
    static void setBlenderLDrawConfigPreference(QString);
    static void setBlenderImportModule(QString);
    static void setBlenderAddonVersionCheck(bool = true);

    static void setEditorCyclePagesOnUpdate(bool);
    static void setEditorCyclePagesOnUpdateDialog(bool);

    static void setInitFadeSteps();
    static void setInitHighlightStep();
    static void setInitPreferredRenderer();

    static void resetFadeSteps();
    static void resetHighlightStep();
    static void resetPreferredRenderer();
    static void resetPreferenceFlags();

    static void unsetBuildModifications();

    static void addKeyboardShortcut(const QString &objectName, const QKeySequence &keySequence);
    static bool hasKeyboardShortcut(const QString &objectName);
    static QKeySequence keyboardShortcut(const QString &objectName);

    static bool setMessageLogging(bool useLogLevel = false);
    static void printInfo(const QString &info, bool isError = false);
    static void fprintMessage(const QString &message, bool stdError = false);
    static int showMessage(Preferences::MsgID,
                           const QString &message,
                           const QString &title = "",
                           const QString &type = "",
                           bool option = false,  // true=OkCancel|default=Cancel, false=AbortIgnore|default=Ignore
                           bool override = false,// true=Ok|default=Ok|NoCheckBox, false=AbortIgnore|default=Ignore,
                           int icon = 3);
    #ifdef Q_OS_MACOS
    static LibFlag validRendererLib(const QString &, const QString &);
    #endif
    static void messageBoxAdjustWidth(QMessageBox *box, const QString &title, const QString &text, int minWidth = 0);

    static ThemeSettings defaultThemeColors[];
    static QString themeColors[];
    static QString lpub3dAppName;
    static QString ldrawLibPath;
    static QString altLDConfigPath;
    static QString lpub3dLibFile;
    static QString lgeoPath;
    static QString lpubDataPath;
    static QString lpub3dConfigPath;
    static QString lpubExtrasPath;
    static QString ldgliteExe;
    static QString ldviewExe;
    static QString povrayExe;
    static QString lpub3dPath;
    static QString lpub3dCachePath;
    static QString lpub3dExtrasResourcePath;
    static QString lpub3dDocsResourcePath;
    static QString lpub3d3rdPartyConfigDir;
    static QString lpub3d3rdPartyAppDir;
    static QString lpub3d3rdPartyAppExeDir;
    static QString lpub3dLDVConfigDir;
    static QString highlightStepColour;
    static QString defaultAuthor;
    static QString defaultURL;
    static QString defaultEmail;
    static QString documentLogoFile;
    static QString publishDescription;
    static QString disclaimer;
    static QString copyright;
    static QString plugImage;
    static QString plug;
    static QString ldrawiniFile;
    static QString ldgliteSearchDirs;
    static QString moduleVersion;
    static QString logFilePath;
    static QString loggingLevel;
    static QString availableVersions;
    static QString povrayConf;
    static QString povrayIni;
    static QString ldgliteIni;
    static QString ldviewIni;
    static QString ldviewPOVIni;
    static QString nativeExportIni;
    static QString povrayIniPath;
    static QString povrayIncPath;
    static QString povrayScenePath;
    static QString dataLocation;
    static QString displayTheme;
    static QString systemTheme;
    static QString ldvLights;
    static QString xmlMapPath;
    static QString installPrefix;

    static QString validLDrawLibrary;
    static QString validLDrawDir;
    static QString validLDrawPart;
    static QString validLDrawPartsArchive;
    static QString validLDrawLibraryChange;
    static QString validLDrawCustomArchive;
    static QString validLDrawPartsLibrary;
    static QString validLDrawColorParts;
    static QString validFadeStepsColour;
    static QString validPliControl;
    static QString validTitleAnnotations;
    static QString validFreeFormAnnotations;
    static QString validAnnotationStyles;
    static QString validPliSubstituteParts;
    static QString validExcludedPliParts;
    static QString validStickerPliParts;

    static QString pliSubstitutePartsFile;
    static QString pliControlFile;
    static QString titleAnnotationsFile;
    static QString freeformAnnotationsFile;
    static QString annotationStyleFile;
    static QString excludedPartsFile;
    static QString stickerPartsFile;
    static QString ldrawColourPartsFile;

    static QString blenderVersion;
    static QString blenderAddonVersion;
    static QString blenderLDrawConfigFile;
    static QString blenderPreferencesFile;
    static QString blenderConfigDir;
    static QString blenderImportModule;
    static QString blenderExe;

    static QString blCodesFile;
    static QString userElementsFile;
    static QString blColorsFile;
    static QString ld2blColorsXRefFile;
    static QString ld2blCodesXRefFile;
    static QString ld2rbColorsXRefFile;
    static QString ld2rbCodesXRefFile;
    static QString systemEditor;

    static QString fadeStepsColourKey;
    static QString ldrawSearchDirsKey;
    static QString ldrawLibPathKey;

    static QString sceneBackgroundColor;
    static QString sceneGridColor;
    static QString sceneRulerTickColor;
    static QString sceneRulerTrackingColor;
    static QString sceneGuideColor;
    static QString currentLibrarySave;
    static QString editorFont;
#ifdef Q_OS_MACOS
    static QString homebrewPathPrefix;
    static QString homebrewPathInsert;

    static QStringList missingLibs;
#endif
    static QStringList ldgliteParms;
    static QStringList ldSearchDirs;
    static QStringList messagesNotShown;

    static bool    usingNPP;
    static bool    usingDefaultLibrary;
    static bool    ldrawiniFound;
    static bool    enableDocumentLogo;
    static bool    enableLDViewSingleCall;
    static bool    enableLDViewSnaphsotList;
    static bool    displayAllAttributes;
    static bool    generateCoverPages;
    static bool    printDocumentTOC;
    static bool    doNotShowPageProcessDlg;
    static bool    applyCALocally;
    static bool    preferCentimeters;
    static bool    showAllNotifications;
    static bool    showUpdateNotifications;
    static bool    showSubmodels;
    static bool    showTopModel;
    static bool    showSubmodelInCallout;
    static bool    showInstanceCount;
    static bool    enableDownloader;
    static bool    showDownloadRedirects;
    static bool    portableDistribution;
    static bool    lpub3dLoaded;
    static bool    lgeoStlLib;
    static bool    addHelperSearchDir;
    static bool    addLSynthSearchDir;
    static bool    excludeModelsSearchDir;
    static bool    skipPartsArchive;
    static bool    loadLastOpenedFile;
    static bool    loadLastDisplayedPage;
    static bool    extendedSubfileSearch;
    static bool    searchOfficialModels;
    static bool    searchOfficialParts;
    static bool    searchOfficialPrimitives;
    static bool    searchUnofficialParts;
    static bool    searchUnofficialPrimitives;
    static bool    searchUnofficialTextures;
    static bool    searchProjectPath;
    static bool    searchLDrawSearchDirs;

    static bool    ldgliteInstalled;
    static bool    ldviewInstalled;
    static bool    povRayInstalled;
    static bool    blenderInstalled;
    static bool    blenderAddonVersionCheck;

    static bool    useNativePovGenerator;
    static bool    enableFadeSteps;
    static bool    fadeStepsUseColour;

    static bool    enableHighlightStep;
    static int     highlightStepLineWidth;
    static bool    highlightFirstStep;
    static bool    perspectiveProjection;
    static bool    saveOnRedraw;
    static bool    saveOnUpdate;
    static bool    defaultBlendFile;
    static bool    blenderIs28OrLater;
    static bool    buildModEnabled;
    static bool    finalModelEnabled;
    static bool    cycleEachPage;

    static bool    pdfPageImage;
    static bool    ignoreMixedPageSizesMsg;

    static bool    logging;         // global preference, logging on/off offLevel (grp box)
    static bool    logLevel;        // log level combo (grp box)
    static bool    logLevels;       // individual logging levels (grp box)
    static bool    debugLogging;    // set if debugLevel is enabled
    static bool    loggingEnabled;  // set if logging setup successful

    static bool    includeLogLevel;
    static bool    includeLineNumber;
    static bool    includeFunction;
    static bool    includeFileName;
    static bool    includeTimestamp;

    static bool    debugLevel;
    static bool    traceLevel;
    static bool    noticeLevel;
    static bool    infoLevel;
    static bool    statusLevel;
    static bool    warningLevel;
    static bool    errorLevel;
    static bool    fatalLevel;

    static bool    includeAllLogAttributes;
    static bool    allLogLevels;

    static bool    povrayDisplay;
    static bool    povrayAutoCrop;
    static bool    isAppImagePayload;
    static bool    modeGUI;
    static bool    useSystemTheme;
    static bool    darkTheme;

    static bool    sceneRuler;
    static int     sceneRulerTracking;
    static bool    sceneGuides;
    static bool    snapToGrid;
    static bool    hidePageBackground;
    static bool    showGuidesCoordinates;
    static bool    showTrackingCoordinates;
    static bool    lineParseErrors;
    static bool    showInsertErrors;
    static bool    showBuildModErrors;
    static bool    showBuildModEditErrors;
    static bool    showIncludeFileErrors;
    static bool    showAnnotationErrors;
    static bool    showConfigurationErrors;
    static bool    showSaveOnRedraw;
    static bool    showSaveOnUpdate;
    static bool    suppressFPrint;
    static bool    archivePartsOnLaunch;
    static bool    inlineNativeContent;
    static bool    autoUpdateChangeLog;
    static bool    displayPageProcessingErrors;

    static bool    customSceneBackgroundColor;
    static bool    customSceneGridColor;
    static bool    customSceneRulerTickColor;
    static bool    customSceneRulerTrackingColor;
    static bool    customSceneGuideColor;
    static bool    useSystemEditor;
    static bool    recountParts;
    static bool    removeBuildModFormat;
    static bool    removeChildSubmodelFormat;
    static bool    editorBufferedPaging;
    static bool    editorHighlightLines;
    static bool    editorLoadSelectionStep;
    static bool    editorPreviewOnDoubleClick;
    static bool    editorCyclePagesOnUpdate;
    static bool    editorCyclePagesOnUpdateDialog;
    static bool    editorTabLock;

#ifdef Q_OS_MACOS
    static bool    missingRendererLibs;
#endif
    static int     preferredRenderer;
    static int     nativeImageCameraFoVAdjust;
    static int     fadeStepsOpacity;
    static int     checkUpdateFrequency;
    static int     pageWidth;
    static int     pageHeight;
    static int     gridSizeIndex;
    static int     pageDisplayPause;
    static int     rendererTimeout;
    static int     sceneGuidesLine;
    static int     sceneGuidesPosition;
    static int     povrayRenderQuality;
    static int     ldrawFilesLoadMsgs;
    static int     maxOpenWithPrograms;
    static int     editorLinesPerPage;
    static int     editorDecoration;
    static int     fileLoadWaitTime;
    static int     editorFontSize;
    static int     msgBoxMinimumWidth;

    static bool    initEnableFadeSteps;
    static bool    initFadeStepsUseColour;
    static int     initFadeStepsOpacity;
    static QString initValidFadeStepsColour;

    static bool    initEnableHighlightStep;
    static int     initHighlightStepLineWidth;
    static QString initHighlightStepColour;

    static int     initPreferredRenderer;

    static bool    loadTheme;
    static bool    setSceneTheme;
    static bool    reloadPage;
    static bool    reloadFile;
    static bool    resetCustomCache;
    static bool    restartApplication;
    static bool    libraryChangeRestart;

    static int     assemblyCameraLatitude;
    static int     assemblyCameraLongitude;
    static int     partCameraLatitude;
    static int     partCameraLongitude;
    static int     submodelCameraLatitude;
    static int     submodelCameraLongitude;

    static QMap<QString, QKeySequence> keyboardShortcuts;

    virtual ~Preferences() {}
};

extern QHash<QString, int> rendererMap;
extern const QString rendererNames [];
extern const QString msgKeyTypes [][2];
extern Preferences preferences;

#endif
