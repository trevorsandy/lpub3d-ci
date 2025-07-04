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

#include "plisubstituteparts.h"

#include <QMessageBox>
#include <QIcon>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include "declarations.h"
#include "version.h"
#include "lpub_preferences.h"
#include "lpub_qtcompat.h"
#include "QsLog.h"

bool                    PliSubstituteParts::result;
QString                 PliSubstituteParts::empty;
QMap<QString, QString>  PliSubstituteParts::substituteParts;

PliSubstituteParts::PliSubstituteParts()
{
    if (substituteParts.size() == 0) {
        QString substitutePartsFile = Preferences::pliSubstitutePartsFile;
        static QRegularExpression rxin("^#[\\w\\s]+\\:[\\s](\\^.*)$");
        static QRegularExpression rx("^(\\b.+\\b)\\s+\"(.*)\"\\s+(.*)$");
        QRegularExpressionMatch match;
        if (!substitutePartsFile.isEmpty()) {
            QFile file(substitutePartsFile);
            if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
                QString message(QObject::tr("Failed to open %1.<br>"
                                "Regenerate by renaming the existing file and select<br>"
                                "Substitute Parts List from<br>Configuration,<br>"
                                "Edit Parameter Files menu.<br>%2")
                                .arg(substitutePartsFile, file.errorString()));
                if (Preferences::modeGUI) { 
                    QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR " - Substitute Parts"),message); 
                } else { 
                    logWarning() << qPrintable(message.replace("<br>"," ")); 
                }
                return;
            }
            QTextStream in(&file);

            // Load RegExp from file;
            while ( ! in.atEnd()) {
                QString sLine = in.readLine(0);
                match = rxin.match(sLine);
                if (match.hasMatch()) {
                    rx.setPattern(match.captured(1));
                    //logDebug() << "SubstituteParts RegExp Pattern: " << rxin.cap(1);
                    break;
                }
            }

            // Load input values
            in.seek(0);
            while ( ! in.atEnd()) {
                QString sLine = in.readLine(0);
                match = rx.match(sLine);
                if (match.hasMatch()) {
                    QString modeledPartID = match.captured(1);
                    QString substitutePartID = match.captured(2);
                    substituteParts.insert(modeledPartID.toLower().trimmed(),substitutePartID.toLower().trimmed());
                    //logDebug() << "** ModeledPartID Loaded: " << modeledPartID.toLower() << " SubstitutePartID: " << substitutePartID.toLower(); //TEST
                }
            }
        }
    }
}

const bool &PliSubstituteParts::hasSubstitutePart(QString part)
{
    if (substituteParts.contains(part.toLower().trimmed())) {
        result = true;
        return result;
    } else {
        result = false;
        return result;
    }
}

const bool &PliSubstituteParts::getSubstitutePart(QString &part) {
    if (substituteParts.contains(part.toLower().toLower().trimmed())) {
        part = substituteParts.value(part.toLower());
#ifdef QT_DEBUG_MODE
        logError() <<  QString("Substitute Part: ").arg(part);
#endif
        result = true;
        return result;
    } else {
        part = empty;
        result = false;
        return result;
    }
}

bool PliSubstituteParts::exportSubstitutePartsHeader() {
    QFile file(QString("%1/extras/%2").arg(Preferences::lpubDataPath,Preferences::validPliSubstituteParts));

    if (!overwriteFile(file.fileName()))
        return true;

    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        int counter = 1;
        QTextStream outstream(&file);
        outstream << "# File: " << VER_PLI_SUBSTITUTE_PARTS_FILE << lpub_endl;
        outstream << "#" << lpub_endl;
        outstream << "# This space-delimited list captures substitute part ID and its substitute part absolute path." << lpub_endl;
        outstream << "# This file is an alternative to the embedded file substitution used when defining a PLI/BOM." << lpub_endl;
        outstream << "# Parts on this list should have an accompanying substitute part." << lpub_endl;
        outstream << "# Substitue parts must be defined using their absolute file path." << lpub_endl;
        outstream << "# When building the PLI/BOM files on this list are replaced with their substitute." << lpub_endl;
        outstream << "#" << lpub_endl;
        outstream << "# The file path must be quoted even if there are no spaces in the path" << lpub_endl;
        outstream << "#" << lpub_endl;
        outstream << "# This file can be edited from LPub3D from:" << lpub_endl;
        outstream << "#    Configuration=>Edit Parameter Files=>Edit PLI/BOM Substitue Parts List" << lpub_endl;
        outstream << "#" << lpub_endl;
        outstream << "# LPub3D will attempt to load the regular expression below first, if the" << lpub_endl;
        outstream << "# load fails, LPub3D will load the hard-coded (default) regular expression." << lpub_endl;
        outstream << "# If you wish to modify the file import, you can edit this regular expression." << lpub_endl;
        outstream << "# It would be wise to backup the default entry before performing and update - copy" << lpub_endl;
        outstream << "# and paste to a new line with starting phrase other than 'The Regular Expression...'" << lpub_endl;
        outstream << "#" << lpub_endl;
        outstream << "# The Regular Expression used to load this file is: ^(\\b.+\\b)\\s+\"(.*)\"\\s+(.*)$" << lpub_endl;
        outstream << "#" << lpub_endl;
        outstream << "#" << lpub_endl;
        outstream << "# 1. Part ID:          LDraw Part Name                               (Required)" << lpub_endl;
        outstream << "# 2. Part Path:        Substitute Part Absolute File Path            (Required)" << lpub_endl;
        outstream << "# 3. Part Description: LDraw Part Description - for reference only   (Optional)" << lpub_endl;
        outstream << "#" << lpub_endl;
        outstream << "# When adding a Part Description, be sure to replace double quotes \" with '." << lpub_endl;
        outstream << "#" << lpub_endl;
        outstream << "# ----------------------Do not delete above this line----------------------------------" << lpub_endl;
        outstream << "#" << lpub_endl;
        outstream << "# Official Parts" << lpub_endl;
        outstream << "" << lpub_endl;
        outstream << "" << lpub_endl;
        outstream << "# Unofficial Parts" << lpub_endl;
        outstream << "" << lpub_endl;
        outstream << "" << lpub_endl;
        outstream << "# Custom Parts" << lpub_endl;

        QByteArray Buffer;
        QTextStream instream(Buffer);
        for (QString sLine = instream.readLine(); !sLine.isNull(); sLine = instream.readLine())
        {
            outstream << sLine << lpub_endl;
            counter++;
        }

        file.close();
        QString message = QString("Finished Writing Substitute Part Entries, Processed %1 lines in file [%2]")
                                   .arg(counter)
                                   .arg(file.fileName());
        if (Preferences::modeGUI) {
            QMessageBox::information(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR " - Substitute Parts"),message);
        } else {
            logNotice() << message;
        }
    }
    else
    {
        QString message = QString("Failed to open PLI substitute parts file: %1:<br>%2")
                                  .arg(file.fileName(),file.errorString());
        if (Preferences::modeGUI) {
            QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR " - Substitute Parts"),message);
        } else {
            logError() << message.replace("<br>"," ");
        }
       return false;
    }
    return true;
}

bool PliSubstituteParts::overwriteFile(const QString &file)
{
    QFileInfo fileInfo(file);

    if (!fileInfo.exists())
        return true;

    QMessageBox box;
    box.setWindowIcon(QIcon());
    box.setIconPixmap (QPixmap(LPUB3D_MESSAGE_ICON));
    box.setTextFormat (Qt::RichText);
    box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    QString title = "<b>" + QMessageBox::tr ("Export %1").arg(fileInfo.fileName()) + "</b>";
    QString text = QMessageBox::tr("\"%1\"<br>This file already exists.<br>Replace existing file?").arg(fileInfo.fileName());
    box.setText (title);
    box.setInformativeText (text);
    box.setStandardButtons (QMessageBox::Cancel | QMessageBox::Yes);
    box.setDefaultButton   (QMessageBox::Yes);

    return (box.exec() == QMessageBox::Yes);
}
