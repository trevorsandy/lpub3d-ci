/****************************************************************************
**
** Copyright (C) 2015 - 2018 Trevor SANDY. All rights reserved.
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
#include "annotations.h"

#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include "lpub_preferences.h"
#include "name.h"
#include "QsLog.h"

int                         Annotations::returnInt;
QString                     Annotations::returnString;
QList<QString>              Annotations::titleAnnotations;
QHash<QString, QString>     Annotations::freeformAnnotations;
QHash<QString, QStringList> Annotations::annotationStyles;

void Annotations::loadDefaultAnnotationStyles(QByteArray& Buffer){

/*
*   LEGO Standard:
*   ------------------------------------
*   |No |Category      |Annotation Style|
*   |---|-------------------------------|
*   | 1 |round(1)      |axle(1)         |
*   | 2 |square(2)     |beam(2)         |
*   | 3 |square(2)     |cable(3)        |
*   | 4 |square(2)     |connector(4)    |
*   | 5 |square(2)     |hose(5)         |
*   | 6 |square(2)     |panel(6)        |
*   ------------------------------------
*/
    const char LEGODefaultAnnotationStyles[] = {
        "3704.dat=1    1   2    Technic Axle  2\n"
        "32062.dat=1   1   2    Technic Axle  2 Notched\n"
        "4519.dat=1    1   3    Technic Axle  3\n"
        "24316.dat=1   1   3    Technic Axle  3 with Stop\n"
        "6587.dat=1    1   3    Technic Axle  3 with Stud\n"
        "27940.dat=1   1   3    Technic Axle  3L with Middle Perpendicular Pin Hole\n"
        "3705.dat=1    1   4    Technic Axle  4\n"
        "3705c01.dat=1 1   4    Technic Axle  4 Threaded\n"
        "99008.dat=1   1   4    Technic Axle  4 with Middle Cylindrical Stop\n"
        "87083.dat=1   1   4    Technic Axle  4 with Stop\n"
        "32073.dat=1   1   5    Technic Axle  5\n"
        "15462.dat=1   1   5    Technic Axle  5 with Stop\n"
        "32209.dat=1   1   5.5  Technic Axle  5.5 with Stop\n"
        "59426.dat=1   1   5.5  Technic Axle  5.5 with Stop Type 2\n"
        "3706.dat=1    1   6    Technic Axle  6\n"
        "44294.dat=1   1   7    Technic Axle  7\n"
        "3707.dat=1    1   8    Technic Axle  8\n"
        "55013.dat=1   1   8    Technic Axle  8 with Stop\n"
        "60485.dat=1   1   9    Technic Axle  9\n"
        "3737.dat=1    1  10    Technic Axle 10\n"
        "3737c01.dat=1 1  10    Technic Axle 10 Threaded\n"
        "23948.dat=1   1  11    Technic Axle 11\n"
        "3708.dat=1    1  12    Technic Axle 12\n"
        "50451.dat=1   1  16    Technic Axle 16\n"
        "50450.dat=1   1  32    Technic Axle 32\n"

        "32580.dat=1   1   7    Technic Axle Flexible  7\n"
        "32199.dat=1   1  11    Technic Axle Flexible 11\n"
        "32200.dat=1   1  12    Technic Axle Flexible 12\n"
        "32201.dat=1   1  14    Technic Axle Flexible 14\n"
        "32202.dat=1   1  16    Technic Axle Flexible 16\n"
        "32235.dat=1   1  19    Technic Axle Flexible 19\n"

        "18654.dat=2   2   1    Technic Beam  1\n"
        "43857.dat=2   2   2    Technic Beam  2\n"
        "32523.dat=2   2   3    Technic Beam  3\n"
        "32316.dat=2   2   5    Technic Beam  5\n"
        "32524.dat=2   2   7    Technic Beam  7\n"
        "40490.dat=2   2   9    Technic Beam  9\n"
        "32525.dat=2   2  11    Technic Beam 11\n"
        "41239.dat=2   2  13    Technic Beam 13\n"
        "32278.dat=2   2  15    Technic Beam 15\n"

        "11145.dat=2   3  25    Electric Mindstorms EV3 Cable 25 cm\n"
        "11146.dat=2   3  35    Electric Mindstorms EV3 Cable 35 cm\n"
        "11147.dat=2   3  50    Electric Mindstorms EV3 Cable 50 cm\n"

        "55804.dat=2   3  20    Electric Mindstorms NXT Cable 20 cm\n"
        "55805.dat=2   3  35    Electric Mindstorms NXT Cable 35 cm\n"
        "55806.dat=2   3  50    Electric Mindstorms NXT Cable 50 cm\n"

        "32013.dat=2   4   1    Technic Angle Connector #1\n"
        "32034.dat=2   4   2    Technic Angle Connector #2 (180 degree)\n"
        "32016.dat=2   4   3    Technic Angle Connector #3 (157.5 degree)\n"
        "32192.dat=2   4   4    Technic Angle Connector #4 (135 degree)\n"
        "32015.dat=2   4   5    Technic Angle Connector #5 (112.5 degree)\n"
        "32014.dat=2   4   6    Technic Angle Connector #6 (90 degree)\n"

        "76263.dat=2   5   3    Technic Flex-System Hose  3L (60LDU)\n"
        "76250.dat=2   5   4    Technic Flex-System Hose  4L (80LDU)\n"
        "76307.dat=2   5   5    Technic Flex-System Hose  5L (100LDU)\n"
        "76279.dat=2   5   6    Technic Flex-System Hose  6L (120LDU)\n"
        "76289.dat=2   5   7    Technic Flex-System Hose  7L (140LDU)\n"
        "76260.dat=2   5   8    Technic Flex-System Hose  8L (160LDU)\n"
        "76324.dat=2   5   9    Technic Flex-System Hose  9L (180LDU)\n"
        "76348.dat=2   5  10    Technic Flex-System Hose 10L (200LDU)\n"
        "71505.dat=2   5  11    Technic Flex-System Hose 11L (220LDU)\n"
        "71175.dat=2   5  12    Technic Flex-System Hose 12L (240LDU)\n"
        "71551.dat=2   5  13    Technic Flex-System Hose 13L (260LDU)\n"
        "71177.dat=2   5  14    Technic Flex-System Hose 14L (280LDU)\n"
        "71194.dat=2   5  15    Technic Flex-System Hose 15L (300LDU)\n"
        "71192.dat=2   5  16    Technic Flex-System Hose 16L (320LDU)\n"
        "76270.dat=2   5  17    Technic Flex-System Hose 17L (340LDU)\n"
        "71582.dat=2   5  18    Technic Flex-System Hose 18L (360LDU)\n"
        "22463.dat=2   5  19    Technic Flex-System Hose 19L (380LDU)\n"
        "76276.dat=2   5  20    Technic Flex-System Hose 20L (400LDU)\n"
        "70978.dat=2   5  21    Technic Flex-System Hose 21L (420LDU)\n"
        "76252.dat=2   5  22    Technic Flex-System Hose 22L (440LDU)\n"
        "76254.dat=2   5  23    Technic Flex-System Hose 23L (460LDU)\n"
        "76277.dat=2   5  24    Technic Flex-System Hose 24L (480LDU)\n"
        "53475.dat=2   5  26    Technic Flex-System Hose 26L (520LDU)\n"
        "76280.dat=2   5  28    Technic Flex-System Hose 28L (560LDU)\n"
        "76389.dat=2   5  29    Technic Flex-System Hose 29L (580LDU)\n"
        "76282.dat=2   5  30    Technic Flex-System Hose 30L (600LDU)\n"
        "76283.dat=2   5  31    Technic Flex-System Hose 31L (620LDU)\n"
        "57274.dat=2   5  32    Technic Flex-System Hose 32L (640LDU)\n"
        "42688.dat=2   5  33    Technic Flex-System Hose 33L (660LDU)\n"
        "22461.dat=2   5  34    Technic Flex-System Hose 34L (680LDU)\n"
        "46305.dat=2   5  40    Technic Flex-System Hose 40L (800LDU)\n"
        "76281.dat=2   5  45    Technic Flex-System Hose 45L (900LDU)\n"
        "22296.dat=2   5  53    Technic Flex-System Hose 53L (1060LDU)\n"

        "72504.dat=2   5   2    Technic Ribbed Hose  2L\n"
        "72706.dat=2   5   3    Technic Ribbed Hose  3L\n"
        "71952.dat=2   5   4    Technic Ribbed Hose  4L\n"
        "72853.dat=2   5   5    Technic Ribbed Hose  5L\n"
        "71944.dat=2   5   6    Technic Ribbed Hose  6L\n"
        "57719.dat=2   5   7    Technic Ribbed Hose  7L\n"
        "71951.dat=2   5   8    Technic Ribbed Hose  8L\n"
        "71917.dat=2   5   9    Technic Ribbed Hose  9L\n"
        "71949.dat=2   5  10    Technic Ribbed Hose 10L\n"
        "71986.dat=2   5  11    Technic Ribbed Hose 11L\n"
        "71819.dat=2   5  12    Technic Ribbed Hose 12L\n"
        "71923.dat=2   5  14    Technic Ribbed Hose 14L\n"
        "71946.dat=2   5  15    Technic Ribbed Hose 15L\n"
        "71947.dat=2   5  16    Technic Ribbed Hose 16L\n"
        "22900.dat=2   5  17    Technic Ribbed Hose 17L\n"
        "72039.dat=2   5  18    Technic Ribbed Hose 18L\n"
        "43675.dat=2   5  19    Technic Ribbed Hose 19L\n"
        "23397.dat=2   5  24    Technic Ribbed Hose 24L\n"

        "32190.dat=2   6   1    Technic Panel Fairing #1\n"
        "32191.dat=2   6   2    Technic Panel Fairing #2\n"
        "44350.dat=2   6  20    Technic Panel Fairing #20\n"
        "44351.dat=2   6  21    Technic Panel Fairing #21\n"
        "44352.dat=2   6  22    Technic Panel Fairing #22\n"
        "44353.dat=2   6  23    Technic Panel Fairing #23\n"
        "47712.dat=2   6  24    Technic Panel Fairing #24\n"
        "47713.dat=2   6  25    Technic Panel Fairing #25\n"
        "32188.dat=2   6   3    Technic Panel Fairing #3\n"
        "32189.dat=2   6   4    Technic Panel Fairing #4\n"
        "32527.dat=2   6   5    Technic Panel Fairing #5\n"
        "32528.dat=2   6   6    Technic Panel Fairing #6\n"
        "32534.dat=2   6   7    Technic Panel Fairing #7\n"
        "32535.dat=2   6   8    Technic Panel Fairing #8\n"

        "87080.dat=2   6   1    Technic Panel Fairing Smooth #1 (Short)\n"
        "64394.dat=2   6  13    Technic Panel Fairing Smooth #13 (Wide Medium)\n"
        "64680.dat=2   6  14    Technic Panel Fairing Smooth #14 (Wide Medium)\n"
        "64392.dat=2   6  17    Technic Panel Fairing Smooth #17 (Wide Long)\n"
        "64682.dat=2   6  18    Technic Panel Fairing Smooth #18 (Wide Long)\n"
        "87086.dat=2   6   2    Technic Panel Fairing Smooth #2 (Short)\n"
        "11946.dat=2   6  21    Technic Panel Fairing Smooth #21 (Thin Short)\n"
        "11947.dat=2   6  22    Technic Panel Fairing Smooth #22 (Thin Short)\n"
        "64683.dat=2   6   3    Technic Panel Fairing Smooth #3 (Medium)\n"
        "64391.dat=2   6   4    Technic Panel Fairing Smooth #4 (Medium)\n"
        "64681.dat=2   6   5    Technic Panel Fairing Smooth #5 (Long)\n"
        "64393.dat=2   6   6    Technic Panel Fairing Smooth #6 (Long)\n"

        "4109810.dat=1 1   2    _Technic Axle  2 Notched Black\n"
        "4211815.dat=1 1   3    _Technic Axle  3 Light_Bluish_Gray\n"
        "370526.dat=1  1   4    _Technic Axle  4 Black\n"
        "73839.dat=1   1   4    _Technic Axle  4 Threaded Black\n"
        "370626.dat=1  1   6    _Technic Axle  6 Black\n"
        "370726.dat=1  1   8    _Technic Axle  8 Black\n"
        "73485.dat=1   1  10    _Technic Axle 10 Threaded Black\n"
        "370826.dat=1  1  12    _Technic Axle 12 Black\n"
        "4263624.dat=1 1  5.5   _Technic Axle 5.5 With Stop Dark_Bluish_Gray\n"
        "55709.dat=1   1  11    =Technic Axle Flexible 11\n"

        "17141.dat=2   2   3    =Technic Beam  3\n"
        "16615.dat=2   2   7    =Technic Beam  7\n"
        "64289.dat=2   2   9    =Technic Beam  9\n"
        "64290.dat=2   2  11    =Technic Beam 11\n"
        "64871.dat=2   2  15    =Technic Beam 15\n"

        "4211550.dat=2 4   1    _Technic Angle Connector #1 Light_Bluish_Gray\n"
        "4506697.dat=2 4   5    _Technic Angle Connector #5 (112.5 degree) White\n"
        "4107767.dat=2 4   6    _Technic Angle Connector #6 (90 degree) Black\n"

        "22749.dat=2   6   1    =Technic Panel Fairing #1\n"
        "22750.dat=2   6   2    =Technic Panel Fairing #2\n"
        "22972.dat=2   6   3    =Technic Panel Fairing #3\n"
        "22973.dat=2   6   4    =Technic Panel Fairing #4\n"
    };

    const char TENTEDefaultAnnotationStyles[] = {
        "no default annotation styles defined\n"
    };

    const char VEXIQDefaultAnnotationStyles[] = {
        "no default annotation styles defined1\n"
    };

    if (Preferences::validLDrawLibrary == LEGO_LIBRARY)
        Buffer.append(LEGODefaultAnnotationStyles, sizeof(LEGODefaultAnnotationStyles));
    else
    if (Preferences::validLDrawLibrary == TENTE_LIBRARY)
        Buffer.append(TENTEDefaultAnnotationStyles, sizeof(TENTEDefaultAnnotationStyles));
    else
    if (Preferences::validLDrawLibrary == VEXIQ_LIBRARY)
        Buffer.append(VEXIQDefaultAnnotationStyles, sizeof(VEXIQDefaultAnnotationStyles));
}

Annotations::Annotations()
{
    if (titleAnnotations.size() == 0) {
        QString annotations = Preferences::titleAnnotationsFile;
        QFile file(annotations);
        if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
            QString message = QString("Failed to open Title Annotations file: %1:\n%2")
                                      .arg(annotations)
                                      .arg(file.errorString());
            if (Preferences::modeGUI){
                QMessageBox::warning(nullptr,QMessageBox::tr("LPub3D"),
                                     message);
            } else {
                logError() << message;
            }
            return;
        }
        QTextStream in(&file);

        // Load RegExp from file;
        QRegExp rx("^(\\b.*[^\\s]\\b:)\\s+([\\(|\\^].*)$");
        QRegExp rxin("^#\\sThe\\sRegular\\sExpression\\sused\\sis\\:[\\s](\\^.*)$");
        while ( ! in.atEnd()) {
            QString sLine = in.readLine(0);
            if (sLine.contains(rxin)) {
                rx.setPattern(rxin.cap(1));
                //logDebug() << "TitleAnnotations RegExp Pattern: " << rxin.cap(1);
                break;
            }
        }

        // Load input values
        while ( ! in.atEnd()) {
            QString sLine = in.readLine(0);
            if (sLine.contains(rx)) {
                QString annotation = rx.cap(2);
                titleAnnotations << annotation;
            }
        }
    }

    if (freeformAnnotations.size() == 0) {
        QString annotations = Preferences::freeformAnnotationsFile;
        QFile file(annotations);
        if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
            QString message = QString("Failed to open Freeform Annotations file: %1:\n%2")
                                      .arg(annotations)
                                      .arg(file.errorString());
            if (Preferences::modeGUI){
                QMessageBox::warning(nullptr,QMessageBox::tr("LPub3D"),
                                     message);
            } else {
                logError() << message;
            }
            return;
        }
        QTextStream in(&file);

        // Load RegExp from file;
        QRegExp rx("^(\\b.*[^\\s]\\b)(?:\\s)\\s+(.*)$");
        QRegExp rxin("^#\\sThe\\sRegular\\sExpression\\sused\\sis\\:[\\s](\\^.*)$");
        while ( ! in.atEnd()) {
            QString sLine = in.readLine(0);
            if (sLine.contains(rxin)) {
                rx.setPattern(rxin.cap(1));
                //logDebug() << "FreeFormAnnotations RegExp Pattern: " << rxin.cap(1);
                break;
            }
        }

        // Load input values
        while ( ! in.atEnd()) {
            QString sLine = in.readLine(0);
            if (sLine.contains(rx)) {
                QString parttype = rx.cap(1);
                QString annotation = rx.cap(2);
                freeformAnnotations[parttype.toLower()] = annotation;
            }
        }
    }

    if (annotationStyles.size() == 0) {
        QString styleFile = Preferences::annotationStyleFile;
        QRegExp rx("^(\\b[^=]+\\b)=([1|2])\\s+([1-6])?\\s*([^\\s]+).*$");
        if (!styleFile.isEmpty()) {
            QFile file(styleFile);
            if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
                QString message = QString("Failed to open Annotation style file: %1:\n%2")
                                          .arg(styleFile)
                                          .arg(file.errorString());
                if (Preferences::modeGUI){
                    QMessageBox::warning(nullptr,QMessageBox::tr("LPub3D"),
                                         message);
                } else {
                    logError() << message;
                }
                return;
            }
            QTextStream in(&file);

            // Load RegExp from file;
            QRegExp rxin("^#\\sThe\\sRegular\\sExpression\\sused\\sis\\:[\\s](\\^.*)$");
            while ( ! in.atEnd()) {
                QString sLine = in.readLine(0);
                if (sLine.contains(rxin)) {
                    rx.setPattern(rxin.cap(1));
//                    logDebug() << "AnnotationStyle RegExp Pattern: " << rxin.cap(1);
                    break;
                }
            }

            // Load input values
            while ( ! in.atEnd()) {
                QString sLine = in.readLine(0);
                if (sLine.contains(rx)) {
                    QString parttype = rx.cap(1);
                    QString style = rx.cap(2).isEmpty() ? QString() : rx.cap(2);
                    QString category = rx.cap(3).isEmpty() ? QString() : rx.cap(3);
                    QString annotation = rx.cap(4).isEmpty() ? QString() : rx.cap(4);
                    annotationStyles[parttype.toLower()] << style << category << annotation;
                }
            }
        } else {
            annotationStyles.clear();
            QByteArray Buffer;
            loadDefaultAnnotationStyles(Buffer);
            QTextStream instream(Buffer);
            for (QString sLine = instream.readLine(); !sLine.isNull(); sLine = instream.readLine())
            {
                int Equals = sLine.indexOf('=');
                if (Equals == -1)
                    continue;
                if (sLine.contains(rx)) {
                    QString parttype = rx.cap(1);
                    QString style = rx.cap(2).isEmpty() ? QString() : rx.cap(2);
                    QString category = rx.cap(3).isEmpty() ? QString() : rx.cap(3);
                    QString annotation = rx.cap(4).isEmpty() ? QString() : rx.cap(4);
                    annotationStyles[parttype.toLower()] << style << category << annotation;
                }
            }
        }
    }
}

bool Annotations::exportAnnotationStyleFile(){
    QFile file(QString("%1/extras/%2").arg(Preferences::lpubDataPath,Preferences::validFreeFormAnnotations));
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        int counter = 1;
        QTextStream outstream(&file);

        QByteArray Buffer;
        loadDefaultAnnotationStyles(Buffer);
        QTextStream instream(Buffer);
        for (QString sLine = instream.readLine(); !sLine.isNull(); sLine = instream.readLine())
        {
            outstream << sLine << endl;
            counter++;
        }

        file.close();
        QString message = QString("Finished Writing Annotation Style Entries, Processed %1 lines in file [%2]")
                                   .arg(counter)
                                   .arg(file.fileName());
        if (Preferences::modeGUI){
            QMessageBox::information(nullptr,QMessageBox::tr("LPub3D"),
                                 message);
        } else {
            logNotice() << message;
        }
    }
    else
    {
        QString message = QString("Failed to open Annotation style file: %1:\n%2")
                                  .arg(file.fileName())
                                  .arg(file.errorString());
        if (Preferences::modeGUI){
            QMessageBox::warning(nullptr,QMessageBox::tr("LPub3D"),
                                 message);
        } else {
            logError() << message;
        }
       return false;
    }
    return true;
}

const QString &Annotations::freeformAnnotation(QString part)
{
  if (freeformAnnotations.contains(part.toLower())) {
    return freeformAnnotations[part.toLower()];
  }
  return returnString;
}

const int &Annotations::getAnnotationStyle(QString part)
{
  returnInt = 0;
  if (annotationStyles.contains(part.toLower())) {
    bool ok;
    returnInt = annotationStyles[part.toLower()][0].toInt(&ok);
    if (ok)
      return returnInt;
  }
  return returnInt;
}

const int &Annotations::getAnnotationCategory(QString part){
  returnInt = 0;
  if (annotationStyles.contains(part.toLower())) {
    bool ok;
    returnInt = annotationStyles[part.toLower()][1].toInt(&ok);
    if (ok)
      return returnInt;
  }
  return returnInt;
}

const QString &Annotations::getStyleAnnotation(QString part)
{
  if (annotationStyles.contains(part.toLower())) {
    return annotationStyles[part.toLower()][2];
  }
  return returnString;
}
