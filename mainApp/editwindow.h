/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2021 Trevor SANDY. All rights reserved.
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
 * The editwindow is used to display the LDraw file to the user.  The Gui
 * portion of the program (see lpub.h) decides what files and line numbers
 * are displayed.  The edit window has as little responsibility as is
 * possible.  It does work the the syntax highlighter implemented in
 * highlighter.(h,cpp)
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef EDITWINDOW_H
#define EDITWINDOW_H

#include <QDialog>
#include <QMainWindow>
#include <QTextEdit>
#include <QTextCursor>
#include <QFileSystemWatcher>
#include <QFutureWatcher>

#include "lc_global.h"
#include "lc_math.h"

#include "name.h"
#include "historylineedit.h"

class LDrawFile;
class Highlighter;
class HighlighterSimple;
class QFindReplace;
class QFindReplaceCtrls;
class QLineNumberArea;
class QTextEditor;

class QString;
class QAction;
class QMenu;
class QPaintEvent;
class QResizeEvent;
class QSize;
class QCheckBox;

class QLineEdit;
class QPushButton;
class QLabel;
class QComboBox;

class QAbstractItemModel;
class QCompleter;
class QProgressBar;

class WaitingSpinnerWidget;
class LoadModelWorker;

/*class Pli;*/

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class EditWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit EditWindow(QMainWindow *parent = nullptr, bool modelFileEdit = false);
    QTextEditor *textEdit()
    {
        return _textEdit;
    }
    bool modelFileEdit()
    {
        return _modelFileEdit;
    }
    QString &getCurrentFile()
    {
        return fileName;
    }
    bool contentLoading()
    {
        return _contentLoading;
    }

    QToolBar *editToolBar;
    QToolBar *toolsToolBar;

signals:
    void contentsChange(const QString &, int position, int charsRemoved, const QString &charsAdded);
    void refreshModelFileSig();
    void getSubFileListSig();
    void redrawSig();
    void updateSig();
    void enableWatcherSig();
    void disableWatcherSig();
    void updateDisabledSig(bool);
    void editModelFileSig();
    void SelectedPartLinesSig(QVector<TypeLine>&, PartSource = EDITOR_LINE);
    void setStepForLineSig(const TypeLine &);
    void waitingSpinnerStopSig();

public slots:
    void displayFile(LDrawFile *, const QString &fileName, const StepLines& lineScope);
    void displayFile(LDrawFile *, const QString &fileName);
    void setLineScope(const StepLines& lineScope);
    void setSubFiles(const QStringList& subFiles);
    void modelFileChanged(const QString &fileName);
    void showLine(int, int);
    void highlightSelectedLines(QVector<int> &lines, bool clear);
    void triggerPreviewLine();
    void updateDisabled(bool);
    void clearEditorWindow();
    void setTextEditHighlighter();
    void setSelectionHighlighter();
    void pageUpDown(QTextCursor::MoveOperation op, QTextCursor::MoveMode moveMode);
    void setReadOnly(bool enabled);
    void setSubmodel(int index);
    void loadContentBlocks(const QStringList &, bool); /* NOT USED FOR THE MOMENT */
    void loadPagedContent();
    void setPagedContent(const QStringList &);
    void setPlainText(const QString &);
    void setLineCount(int);
    void clearWindow();
    bool maybeSave();
    bool saveFile();

private slots:
    void openWith();
    void contentsChange(int position, int charsRemoved, int charsAdded);
    bool saveFileCopy();
    void redraw();
    void update(bool state);
    void enableSave();
    void highlightCurrentLine();
    void topOfDocument();
    void editLineItem();
    void previewLine();
#ifdef QT_DEBUG_MODE
    void previewViewerFile();
#endif
    void bottomOfDocument();
    void showAllCharacters();
    void mpdComboChanged(int index);
    void showContextMenu(const QPoint &pt);
    void updateSelectedParts();
    void preferences();
    void verticalScrollValueChanged(int action);
    void openFolder();
#ifndef QT_NO_CLIPBOARD
    void updateClipboard();
#endif
    void waitingSpinnerStart();
    void waitingSpinnerStop();
    void contentLoaded();

protected:
    void createActions();
    void createMenus();
    void createToolBars();
    void createOpenWithActions();
    void readSettings();
    void writeSettings();
    void clearEditorHighlightLines();
    void openFolderSelect(const QString& absoluteFilePath);
    void highlightSelectedLines(QVector<int> &lines, bool clear, bool editor);

    QAbstractItemModel *modelFromFile(const QString& fileName);
    void openWithProgramAndArgs(QString &program, QStringList &arguments);
    void updateOpenWithActions();
    void disableActions();
    void enableActions();
    bool setValidPartLine();
    void closeEvent(QCloseEvent*_event);

    enum Decor { SIMPLE, STANDARD };

    WaitingSpinnerWidget *_waitingSpinner;
    QTextEditor       *_textEdit;
    LoadModelWorker   *loadModelWorker;
    QCompleter        *completer;
    Highlighter       *highlighter;
    HighlighterSimple *highlighterSimple;
/*    Pli               *pli; */
    QComboBox         *mpdCombo;
    QFutureWatcher<int> futureWatcher;
    QFileSystemWatcher fileWatcher;
    QElapsedTimer      displayTimer;
    StepLines          stepLines;
    QVector<int>       savedSelection;
    QString            fileName;            // of model file currently being displayed
    int                numOpenWithPrograms;
    int                showLineType;
    int                showLineNumber;
    int                fileOrderIndex;
    QAtomicInt         lineCount;
    bool               isIncludeFile;
    bool               isReadOnly;
    bool               reloaded;
    bool               _modelFileEdit;
    bool               _subFileListPending;
    bool               _contentLoaded;
    bool               _contentLoading;
    QString            _curSubFile;         // currently displayed submodel
    QStringList        _subFileList;
    QStringList        _pageContent;
    int                _pageIndx;
    int                _saveSubfileIndex;

    QScrollBar *verticalScrollBar;
    QList<QAction *> openWithActList;

    QAction  *openWithToolbarAct;
    QAction  *editModelFileAct;   
    QAction  *previewLineAct;
    QAction  *editColorAct;
    QAction  *editPartAct;

    QAction  *mpdComboSeparatorAct;
    QAction  *mpdComboAct;
/*
    QAction  *substitutePartAct;
    QAction  *removeSubstitutePartAct;
*/
#ifdef QT_DEBUG_MODE
    QAction  *previewViewerFileAct;
#endif
    QAction  *topAct;
    QAction  *bottomAct;
    QAction  *cutAct;
    QAction  *copyAct;
    QAction  *pasteAct;
    QAction  *redrawAct;
    QAction  *updateAct;
    QAction  *delAct;
    QAction  *selAllAct;
    QAction  *findAct;
    QAction  *toggleCmmentAct;
    QAction  *showAllCharsAct;
    QAction  *preferencesAct;
    QAction  *openFolderAct;
    QAction  *copyFullPathToClipboardAct;
    QAction  *copyFileNameToClipboardAct;

    QAction  *exitAct;
    QAction  *saveAct;
    QAction  *saveCopyAct;
    QAction  *undoAct;
    QAction  *redoAct;
};

extern class EditWindow *editWindow;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class QTextEditor : public QTextEdit
{
    Q_OBJECT

public:
    explicit QTextEditor(bool modelFileEdit = false, QWidget *parent = nullptr);

    ~QTextEditor()override{}

    void showAllCharacters(bool show);
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    void setIsUTF8(bool isUTF8) { _fileIsUTF8 = isUTF8; }
    bool getIsUTF8() { return _fileIsUTF8; }
    int getFirstVisibleBlockId();
    int lineNumberAreaWidth();
    QFindReplace *popUp;

    void setCompleter(QCompleter *c);
    void setCompleterMinChars(int min_chars);
    void setCompleterMaxSuggestions(int max);
    void setCompleterPrefix(const QString& prefix);
    bool modelFileEdit()
    {
        return detachedEdit;
    }

signals:
    void updateSelectedParts();
    void triggerPreviewLine();

public slots:
    void resizeEvent(QResizeEvent *e) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(QRectF /*rect_f*/);
    void updateLineNumberArea(int /*slider_pos*/);
    void updateLineNumberArea();
    void findDialog();
    void toggleComment();
    void showCharacters(
         QString findString,
         QString replaceString);
    void autocomplete(const QString &completion);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void focusInEvent(QFocusEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    QString currentWord() const;

private:
    QString textUnderCursor() const;
    int wordStart() const;
    QCompleter *completer;
    int         completion_minchars;
    int         completion_max;
    QString     completion_prefix;
    bool       detachedEdit;
    std::atomic<bool> _fileIsUTF8;
    QWidget    *lineNumberArea;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class QLineNumberArea : public QWidget
{
public:
    QLineNumberArea(QTextEditor *editor) : QWidget(editor) {
        textEditor = editor;}

    QSize sizeHint() const {
        return QSize(textEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event){
        textEditor->lineNumberAreaPaintEvent(event);
    }

private:
    QTextEditor *textEditor;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class QFindReplace : public QDialog
{
    Q_OBJECT

public:
    explicit QFindReplace(QTextEditor *textEdit, const QString &selectedText, QWidget *parent = nullptr);

protected slots:
    void popUpClose();

protected:
    QFindReplaceCtrls *find;
    QFindReplaceCtrls *findReplace;
    void readFindReplaceSettings(QFindReplaceCtrls *fr);
    void writeFindReplaceSettings(QFindReplaceCtrls *fr);
private:
    QAbstractItemModel *modelFromFile(const QString& fileName);
    QCompleter *completer;
};

class QFindReplaceCtrls : public QWidget
{
    Q_OBJECT

public:
    explicit QFindReplaceCtrls(QTextEditor *textEdit, QWidget *parent = nullptr);
    QTextEditor *_textEdit;
    HistoryLineEdit *textFind;
    HistoryLineEdit *textReplace;
    QLabel      *labelMessage;

    QPushButton *buttonFind;
    QPushButton *buttonFindNext;
    QPushButton *buttonFindPrevious;
    QPushButton *buttonFindAll;
    QPushButton *buttonFindClear;

    QPushButton *buttonReplace;
    QPushButton *buttonReplaceAndFind;
    QPushButton *buttonReplaceAll;
    QPushButton *buttonReplaceClear;

    QLabel      *label;

    QCheckBox   *checkboxCase;
    QCheckBox   *checkboxWord;
    QCheckBox   *checkboxRegExp;

    QPushButton *buttonCancel;
    bool        _findall;

public slots:
    void find(int direction = 0);

signals:
    void popUpClose();

protected slots:
    void findInText();
    void findInTextNext();
    void findInTextPrevious();
    void findInTextAll();

    void findClear();

    void replaceInText();
    void replaceInTextFind();
    void replaceInTextAll();

    void replaceClear();

    void textFindChanged();
    void textReplaceChanged();
    void validateRegExp(const QString &text);
    void regexpSelected(bool sel);

protected:
    void disableButtons();
    void showError(const QString &error);
    void showMessage(const QString &message);
};

#endif
