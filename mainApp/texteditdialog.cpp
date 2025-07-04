/****************************************************************************
**
** Copyright (C) 2019 - 2025 Trevor SANDY. All rights reserved.
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

/****************************************************************************
 *
 * This class implements a window that lets the user edit plain text.
 *
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/

#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QFontDialog>
#include <QColorDialog>
#if QT_VERSION < QT_VERSION_CHECK(6,0,0) || defined(QT_CORE5COMPAT_LIB)
#include <QTextCodec>
#endif
#include <QDebug>

#include "texteditdialog.h"
#include "ui_texteditdialog.h"
#include "messageboxresizable.h"
#include "lpub_object.h"
#include "version.h"
#include "declarations.h"
#include "commonmenus.h"

TextEditDialog *tedit;

bool    TextEditDialog::richText;
bool    TextEditDialog::rendererArgs;
QString TextEditDialog::text;
QFont   TextEditDialog::font;
QColor  TextEditDialog::fontColor;

TextEditDialog::TextEditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TextEditDialog)
{
    ui->setupUi(this);

    setWindowTitle(tr("Edit Text"));

    setWhatsThis(lpubWT(WT_DIALOG_TEXT_EDIT, windowTitle()));

    ui->actionAccept->setObjectName("textEditAcceptAct.3");
    lpub->actions.insert(ui->actionAccept->objectName(), Action(QStringLiteral("File.Accept"), ui->actionAccept));

    ui->actionCancel->setObjectName("textEditCancelAct.3");
    lpub->actions.insert(ui->actionCancel->objectName(), Action(QStringLiteral("File.Cancel"), ui->actionCancel));

    ui->actionNew->setObjectName("textEditNewAct.3");
    lpub->actions.insert(ui->actionNew->objectName(), Action(QStringLiteral("File.New"), ui->actionNew));

    ui->actionCopy->setObjectName("textEditCopyAct.3");
    lpub->actions.insert(ui->actionCopy->objectName(), Action(QStringLiteral("Edit.Copy"), ui->actionCopy));

    ui->actionCut->setObjectName("textEditCutAct.3");
    lpub->actions.insert(ui->actionCut->objectName(), Action(QStringLiteral("Edit.Cut"), ui->actionCut));

    ui->actionPaste->setObjectName("textEditPasteAct.3");
    lpub->actions.insert(ui->actionPaste->objectName(), Action(QStringLiteral("Edit.Paste"), ui->actionPaste));

    ui->actionUndo->setObjectName("textEditUndoAct.3");
    lpub->actions.insert(ui->actionUndo->objectName(), Action(QStringLiteral("Edit.Undo"), ui->actionUndo));

    ui->actionRedo->setObjectName("textEditRedoAct.3");
    lpub->actions.insert(ui->actionRedo->objectName(), Action(QStringLiteral("Edit.Redo"), ui->actionRedo));

    ui->actionFont->setObjectName("textEditFontAct.3");
    lpub->actions.insert(ui->actionFont->objectName(), Action(QStringLiteral("Edit.Font"), ui->actionFont));

    ui->actionFontColor->setObjectName("textEditFontColorAct.3");
    lpub->actions.insert(ui->actionFontColor->objectName(), Action(QStringLiteral("Edit.Font Color"), ui->actionFontColor));

    ui->actionRichText->setObjectName("textEditRichTextAct.3");
    lpub->actions.insert(ui->actionRichText->objectName(), Action(QStringLiteral("Edit.Rich Text"), ui->actionRichText));

    ui->actionBold->setObjectName("textEditBoldAct.3");
    lpub->actions.insert(ui->actionBold->objectName(), Action(QStringLiteral("Edit.Bold"), ui->actionBold));

    ui->actionItalic->setObjectName("textEditItalicAct.3");
    lpub->actions.insert(ui->actionItalic->objectName(), Action(QStringLiteral("Edit.Italic"), ui->actionItalic));

    ui->actionUnderline->setObjectName("textEditUnderlineAct.3");
    lpub->actions.insert(ui->actionUnderline->objectName(), Action(QStringLiteral("Edit.Underline"), ui->actionUnderline));

    ui->actionUndo->setEnabled(false);
    ui->actionRedo->setEnabled(false);

    connect(ui->textEdit,   SIGNAL(undoAvailable(bool)),
            ui->actionUndo, SLOT(  setEnabled(bool)));

    connect(ui->textEdit,   SIGNAL(redoAvailable(bool)),
            ui->actionRedo, SLOT(  setEnabled(bool)));

    tedit = this;

    setModal(true);

    adjustSize();
}

void TextEditDialog::initialize(
        QString &goods,
        const QString &editFont,
        const QString &editFontColor,
        const QString &windowTitle,
        bool _richText,
        bool fontActions)
{
    setWindowTitle(windowTitle);

    rendererArgs = windowTitle.contains("Renderer Arguments");

    text = goods;

    richText = _richText;

    if (rendererArgs)
        ui->statusBar->showMessage("Enter space delimited renderer arguments.");

    QFont textFont("Arial");
    textFont.setStyleHint(QFont::SansSerif);
    if (editFont.isEmpty())
        font = textFont;
    else
        font.fromString(editFont);
#if QT_VERSION >= QT_VERSION_CHECK(6,4,0)
    fontColor = QColor::fromString(editFontColor.isEmpty() ? "Black" : editFontColor);
#else
    fontColor.setNamedColor(editFontColor.isEmpty() ? "Black" : editFontColor);
#endif

    // Suppress font actions - used for adding renderer arguments
    if (!fontActions && !richText)
    {
        ui->actionRichText->setVisible(false);
        ui->actionFontColor->setVisible(false);
        ui->actionFont->setVisible(false);
        ui->actionBold->setVisible(false);
        ui->actionItalic->setVisible(false);
        ui->actionUnderline->setVisible(false);
        ui->textEdit->setPlainText(text);
    }
    else
    // Include rich text actions
    {
        ui->textEdit->setFont(font);
        ui->actionRichText->setChecked(richText);

        connect(ui->textEdit, &QTextEdit::currentCharFormatChanged,
                tedit,        &TextEditDialog::currentCharFormatChanged);

        QByteArray data = text.toUtf8();
        QString content;
#if QT_VERSION < QT_VERSION_CHECK(6,0,0) || defined(QT_CORE5COMPAT_LIB)
        QTextCodec *codec = Qt::codecForHtml(data);
        content = codec->toUnicode(data);
#else
        content = text;
#endif
        if (richText && Qt::mightBeRichText(content)) {
            ui->textEdit->setHtml(content);
        } else {
            if (fontColor.isValid())
                ui->textEdit->setTextColor(fontColor);
            content = QString::fromLocal8Bit(data);
            ui->textEdit->setPlainText(content);
        }

        fontChanged(ui->textEdit->font());
    }
}

TextEditDialog::~TextEditDialog()
{
    tedit = nullptr;
    delete ui;
}

void TextEditDialog::on_actionRichText_triggered()
{
    richText = ui->actionRichText->isChecked();
    setWindowTitle(QString("Edit %1 Text").arg(richText ? "Rich" : "Plain"));
    fontChanged(ui->textEdit->font());
}

void TextEditDialog::currentCharFormatChanged(const QTextCharFormat &format)
{
    if (rendererArgs)
        return;
    fontChanged(format.font());
}

void TextEditDialog::fontChanged(const QFont &f)
{
    ui->actionBold->setChecked(f.bold());
    if (richText) {
        ui->actionItalic->setChecked(f.italic());
        ui->actionUnderline->setChecked(f.underline());
    } else {
        ui->actionItalic->setVisible(false);
        ui->actionUnderline->setVisible(false);
    }

    QString fontProperties = QString("Font: %1, Size: %2")
            .arg(QString("%1%2")
                 .arg(QFontInfo(f).family(),
                      f.bold()
                          ? " Bold"
                          : f.italic()
                              ? " Italic"
                              : f.underline()
                                  ? " Underline"
                                  : ""),
                      QString::number(f.pointSize()));
    ui->statusBar->showMessage(fontProperties);
}

void TextEditDialog::on_actionAccept_triggered()
{
    if (ui->actionRichText->isChecked())
        text = ui->textEdit->toHtml();
    else
        text = ui->textEdit->toPlainText();
    QDialog::accept();
}

void TextEditDialog::on_actionCancel_triggered()
{
    QDialog::reject();
}

void TextEditDialog::on_actionNew_triggered()
{
    ui->textEdit->setText(QString());
}

void TextEditDialog::on_actionCopy_triggered()
{
    ui->textEdit->copy();
}

void TextEditDialog::on_actionCut_triggered()
{
    ui->textEdit->cut();
}

void TextEditDialog::on_actionPaste_triggered()
{
    ui->textEdit->paste();
}

void TextEditDialog::on_actionUndo_triggered()
{
     ui->textEdit->undo();
}

void TextEditDialog::on_actionRedo_triggered()
{
    ui->textEdit->redo();
}

void TextEditDialog::on_actionFont_triggered()
{
    bool fontSelected;
    font = QFontDialog::getFont(&fontSelected, font);
    if (fontSelected)
        ui->textEdit->setFont(font);
}

void TextEditDialog::on_actionFontColor_triggered()
{
    fontColor = ui->textEdit->textColor();
    fontColor = QColorDialog::getColor(fontColor,nullptr);
    if (fontColor.isValid()) {
        ui->textEdit->setTextColor(fontColor);
    }
}

void TextEditDialog::on_actionUnderline_triggered()
{
    ui->textEdit->setFontUnderline(ui->actionUnderline->isChecked());
}

void TextEditDialog::on_actionItalic_triggered()
{
    ui->textEdit->setFontItalic(ui->actionItalic->isChecked());
}

void TextEditDialog::on_actionBold_triggered()
{
    ui->actionBold->isChecked() ? ui->textEdit->setFontWeight(QFont::Bold) :
                                  ui->textEdit->setFontWeight(QFont::Normal);
}

// get text - simple (always plainText), called by MetaItem::setRendererArguments(...)
bool TextEditDialog::getText(
      QString &goods,
      QString  windowTitle)
{
    if (!tedit)
        return false;

    QEventLoop loop;
    QString empty = QString();

    tedit->initialize(goods,empty,empty,windowTitle);

    bool ok =  false;

    connect(tedit, &QDialog::finished, tedit, [&](int r) { ok = r; loop.exit(); });

    tedit->open();

    // Non blocking wait
    loop.exec(QEventLoop::DialogExec);

    if (ok)
        goods = tedit->text;

    disconnect(tedit, &QDialog::finished, nullptr, nullptr);

    return ok;
}

// get text - full (plain and rich text), called by MetaItem::updateText(...)
bool TextEditDialog::getText(
      QString &goods,
      QString &editFont,
      QString &editFontColor,
      bool    &richText,
      QString  windowTitle,
      bool     fontActions)
{
    if (!tedit)
        return false;

    QEventLoop loop;
    QString unformattedGoods = goods;
    if (!goods.isEmpty()) {
        QChar esc('\\');
        QStringList list;
        static QRegularExpression rx("\"");
        QRegularExpressionMatch match;
        QRegularExpressionMatchIterator matchIt;
        const QStringList goodsList = goods.split("\\n");
        for (const QString &item : goodsList) {
            QString string = item.trimmed();
            int pos = 0;
            int adj = 0;
            matchIt = rx.globalMatch(string);
            while(matchIt.hasNext())
            {
                match = matchIt.next();
                pos = match.capturedStart() + adj;
                if (pos < string.size()) {
                    QChar ch = string.at(pos-1);
                    if (ch == esc) {
                        string.remove(pos-1,1);
                        adj--;
                    }
                }
            }
            // if last character is esc \, append space ' ' so not to escape closing string double quote
            if (string.at(string.size()-1) == esc)
                string.append(QChar(' '));
            list << string;
        }
        unformattedGoods = list.join("\n");
    }

    tedit->initialize(unformattedGoods, editFont, editFontColor, windowTitle, richText, fontActions);

    bool ok = false;

    connect(tedit, &QDialog::finished, tedit, [&](int r) { ok = r; loop.exit(); });

    tedit->open();

    // Non blocking wait
    loop.exec(QEventLoop::DialogExec);

    if (ok) {
        goods    = tedit->text;
        richText = tedit->richText;
    }
    if (!richText) {
        editFont      = tedit->font.toString();
        editFontColor = tedit->fontColor.name();
    }

    disconnect(tedit, &QDialog::finished, nullptr, nullptr);

    return ok;
}

bool TextEditDialog::maybeSave()
{
  bool rc = false;

  if (!ui->textEdit->document()->isEmpty()) {

    QMessageBoxResizable box;
    box.setWindowIcon(QIcon());
    box.setIconPixmap (QPixmap(LPUB3D_MESSAGE_ICON));
    box.setTextFormat (Qt::RichText);
    box.setWindowTitle(tr ("%1 Text Editor").arg(VER_PRODUCTNAME_STR));
    box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    QString title = "<b>" + tr ("Document changes detected") + "</b>";
    QString text = tr("There is unsaved content in the current document.<br>"
                      "Do you want to save your changes?");
    box.setText (title);
    box.setInformativeText (text);
    box.setStandardButtons (QMessageBox::No | QMessageBox::Yes);
    box.setDefaultButton   (QMessageBox::Yes);

    if (box.exec() == QMessageBox::Yes)
      rc = true;
  }
  return rc;
}

void TextEditDialog::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        on_actionAccept_triggered();
    else
        on_actionCancel_triggered();
    QDialog::closeEvent(event);
}

