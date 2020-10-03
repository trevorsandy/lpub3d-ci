/****************************************************************************
**
** Copyright (C) 2019 - 2020 Trevor SANDY. All rights reserved.
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
 * This file implements a dialog that allows the user to substitute parts.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef SUBSTITUTEPARTDIALOG_H
#define SUBSTITUTEPARTDIALOG_H

#include <QDialog>

namespace Ui {
class SubstitutePartDialog;
}

class QGridLayout;
class PieceInfo;
class PreviewWidget;
class Project;
class lcModel;
class lcQGLWidget;

class SubstitutePartDialog : public QDialog
{
  Q_OBJECT
  enum Which {
      InitialType,
      SubstituteType,
      LdrawType,
      PartColor
  };
public:
    explicit SubstitutePartDialog(
      const QStringList &attributes,
      QWidget           *parent = nullptr,
      int                action = 1,
      const QStringList &defaultAttrs = QStringList());

    ~SubstitutePartDialog();

    static bool getSubstitutePart(
            QStringList       &attributes,
            QWidget           *parent = nullptr,
            int                action = 1/*Substitute*/,
            const QStringList &defaultList = QStringList());

    bool mModified;
    QString mLdrawType;
    QStringList mAttributes;

public slots:
  void accept();
  void cancel();

private slots:
  void typeChanged();
  void browseType(bool);
  void browseColor(bool);
  void colorChanged(const QString &);
  void valueChanged(double value);
  void valueChanged(int value);
  void transformChanged(QString const &);
  void reset(bool);
  void showExtendedAttributes(bool);

protected:
  void initialize();
  void typeChanged(Which);
  void showPartPreview(Which);

private:
  Ui::SubstitutePartDialog *ui;
  QPushButton              *mResetBtn;
  QPushButton              *mShowExtAttrsBtn;
  PieceInfo                *mTypeInfo;
  QStringList               mDefaultAttributes;
  QStringList               mInitialAttributes;
  int                       mAction;
  bool                      mViewWidgetEnabled;

  QGridLayout              *previewLayout;
  PreviewWidget            *Preview;
  Project                  *PreviewProject;
  lcModel                  *ActiveModel;
  lcQGLWidget              *ViewWidget;
};

#endif // SUBSTITUTEPARTDIALOG_H
