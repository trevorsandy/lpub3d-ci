/****************************************************************************
**
** Copyright (C) 2015 - 2019 Trevor SANDY. All rights reserved.
** Copyright (C) 2015 - 2019 Trevor SANDY. All rights reserved.
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
 * This implements a syntax highlighter class that works with the editwindow
 * to display LDraw files with syntax highlighting.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/


#ifndef PARMSHIGHLIGHTER_H
#define PARMSHIGHLIGHTER_H

#include <QTextCharFormat>
#include <QSyntaxHighlighter>
#include <QHash>

class QTextDocument;

class ParmsHighlighter : public QSyntaxHighlighter
{
        Q_OBJECT

public:
    ParmsHighlighter(QTextDocument *parent = 0);

protected:
    void highlightBlock(const QString &text);

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };

    QVector<HighlightingRule> highlightingRules;

    QList<QTextCharFormat> lineFormats;

    QTextCharFormat LPubParmsCommentFormat;
    QTextCharFormat LPubParmsHdrFormat;
    QTextCharFormat LPubParmsEqualFormat;
    QTextCharFormat LPubParmsValueFormat;

    QTextCharFormat LPubVal1Format;   // br05 Part ID
    QTextCharFormat LPubVal2Format;   // br06 Part Control
    QTextCharFormat LPubVal3Format;   // br07 Part Description

    int option;

};

#endif // PARMSHIGHLIGHTER_H
