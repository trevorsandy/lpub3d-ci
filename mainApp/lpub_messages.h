/****************************************************************************
**
** Copyright (C) 2018 Trevor SANDY. All rights reserved.
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

#ifndef LPUBMESSAGES_H
#define LPUBMESSAGES_H

#include <stdio.h>
#include <QMessageBox>
#include <QStatusBar>

#include <QsLog.h>

#include "lpub_preferences.h"

#include "version.h"
#include "name.h"
#include "lpub.h"

class LPubMessages : public QObject
{
    Q_OBJECT
public:
  LPubMessages();
  ~LPubMessages(){}
public slots:
  void statusMessage(LogType logType, QString message){
      /* logTypes
       * LOG_STATUS:   - same as INFO but writes to log file also
       * LOG_INFO:
       * LOG_TRACE:
       * LOG_DEBUG:
       * LOG_NOTICE:
       * LOG_ERROR:
       * LOG_FATAL:
       * LOG_QWARNING: - visible in Qt debug mode
       * LOG_QDEBUG:   - visible in Qt debug mode
       */
      if (logType == LOG_STATUS ){

          logStatus() << message;

          if (Preferences::modeGUI) {
             statusBarMsg(message);
          } else {
             fprintf(stdout,"%s",QString(message).append("\n").toLatin1().constData());
             fflush(stdout);
          }
      } else {

          logError() << message;

          if (Preferences::modeGUI) {
              QMessageBox::warning(gui,QMessageBox::tr(VER_PRODUCTNAME_STR),QMessageBox::tr(message.toLatin1()));
          } else {
              fprintf(stdout,"%s",QString(message).append("\n").toLatin1().constData());
              fflush(stdout);
          }
      }
  }

  void statusBarMsg(QString msg)
  {
    gui->statusBar()->showMessage(msg);
  }

signals:
  void messageSig(LogType logType, QString message);
};

extern class LPubMessages *alert;

#endif // LPUBMESSAGES_H
