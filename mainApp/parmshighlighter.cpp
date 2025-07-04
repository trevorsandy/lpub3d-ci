/****************************************************************************
**
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
 * This implements a syntax highlighter class that works with the editwindow
 * to display LDraw files with syntax highlighting.
 *
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/

#include <QtWidgets>
#include "parmshighlighter.h"
#include "version.h"
#include "declarations.h"
#include "lpub_preferences.h"

ParmsHighlighter::ParmsHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    QBrush br01; // Qt Dark green
    QBrush br02; // Qt Dark blue
    QBrush br03; // Custom  blue
    QBrush br04; // Custom  orange/violet

    QBrush br05;
    QBrush br06;
    QBrush br07;

    if (Preferences::displayTheme == THEME_DEFAULT) {
        br01 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_INI_FILE_COMMENT]));
        br02 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_INI_FILE_HEADER]));
        br03 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_INI_FILE_EQUAL]));
        br04 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_INI_FILE_VALUE]));

        br05 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_PARAMETER_FILE_PART_ID]));
        br06 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_PARAMETER_FILE_PART_CONTROL]));
        br07 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_PARAMETER_FILE_PART_DESCRIPTION]));
      }
    else
    if (Preferences::darkTheme)  {
        br01 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_INI_FILE_COMMENT]));
        br02 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_INI_FILE_HEADER]));
        br03 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_INI_FILE_EQUAL]));
        br04 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_INI_FILE_VALUE]));

        br05 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_PARAMETER_FILE_PART_ID]));
        br06 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_PARAMETER_FILE_PART_CONTROL]));
        br07 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_PARAMETER_FILE_PART_DESCRIPTION]));
      }

    /* INI file formats */

    // INI Header
    LPubParmsHdrFormat.setForeground(br02);
    LPubParmsHdrFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("^\\[.*[^\n]\\]$");
    rule.format = LPubParmsHdrFormat;
    highlightingRules.append(rule);

    // Right side value
    LPubParmsValueFormat.setForeground(br04);
    LPubParmsValueFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegularExpression("\\=(.*)");
    rule.format = LPubParmsValueFormat;
    highlightingRules.append(rule);

    // Equal sign
    LPubParmsEqualFormat.setForeground(br03);
    LPubParmsEqualFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("=");
    rule.format = LPubParmsEqualFormat;
    highlightingRules.append(rule);

    // Comment
    LPubParmsCommentFormat.setForeground(br01);
    LPubParmsCommentFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegularExpression("^[#|;][^\n]*");
    rule.format = LPubParmsCommentFormat;
    highlightingRules.append(rule);

    /* List file formats */

    // br05 - Part ID
    LPubVal1Format.setForeground(br05);
    LPubVal1Format.setFontWeight(QFont::Bold);
    lineFormats.append(LPubVal1Format);

    // br06 - Part Control
    LPubVal2Format.setForeground(br06);
    LPubVal2Format.setFontWeight(QFont::Bold);


    // br07 - Part Description
    LPubVal3Format.setForeground(br07);
    LPubVal3Format.setFontWeight(QFont::Normal);

    option = 0;

}

void ParmsHighlighter::highlightBlock(const QString &text)
{
    static QRegularExpression rx;
    QRegularExpressionMatch match;
    QRegularExpressionMatchIterator matchIterator;
    const QVector<HighlightingRule> rules = highlightingRules;
    for (const HighlightingRule &rule : rules) {
        matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    setCurrentBlockState(0);

    if (text.contains(QString::fromLatin1(VER_PLI_SUBSTITUTE_PARTS_FILE),Qt::CaseInsensitive))
        option = 1;
    else if (text.contains(QString::fromLatin1(VER_TITLE_ANNOTATIONS_FILE),Qt::CaseInsensitive))
        option = 2;
    else if (text.contains(QString::fromLatin1(VER_FREEFOM_ANNOTATIONS_FILE),Qt::CaseInsensitive))
        option = 3;
    else if (text.contains(QString::fromLatin1(VER_EXCLUDED_PARTS_FILE),Qt::CaseInsensitive))
        option = 4;
    else if (text.contains(QString::fromLatin1(VER_STICKER_PARTS_FILE),Qt::CaseInsensitive))
        option = 5;
    else if (text.contains(QString::fromLatin1(VER_LPUB3D_COLOR_PARTS),Qt::CaseInsensitive))
        option = 6;
    else if (!option)
        return;

    int index  = 0;

    QStringList tokens;

    switch (option)
    {
    case 1:
    {
        // VER_PLI_SUBSTITUTE_PARTS_FILE
        rx.setPattern("^(\\b.+\\b)\\s+\"(.*)\"\\s+(.*)$");
        match = rx.match(text);
        if (match.hasMatch()) {
            tokens
            << match.captured(1).trimmed()
            << "\""+match.captured(2).trimmed()+"\""
            << match.captured(3).trimmed();
            lineFormats.append(LPubVal2Format);
            lineFormats.append(LPubVal3Format);
        }
    }
        break;
    case 2:
    {
        // VER_TITLE_ANNOTATIONS_FILE
        rx.setPattern("^(\\b.*[^\\s]\\b:)\\s+([\\(|\\^].*)$");
        match = rx.match(text);
        if (match.hasMatch()) {
            tokens
            << match.captured(1).trimmed()
            << match.captured(2).trimmed();
            lineFormats.append(LPubVal3Format);
        }
    }
        break;
    case 3:
    {
        // VER_FREEFOM_ANNOTATIONS_FILE
        rx.setPattern("^(\\b.*[^\\s]\\b)(?:\\s)\\s+(.*)$");
        match = rx.match(text);
        if (match.hasMatch()) {
            tokens
            << match.captured(1).trimmed()
            << match.captured(2).trimmed();
            lineFormats.append(LPubVal3Format);
        }
    }
        break;
    case 4:
    {
        // VER_EXCLUDED_PARTS_FILE
        rx.setPattern("^(\\b.*[^\\s]\\b)(?:\\s)\\s+(.*)$");
        match = rx.match(text);
        if (match.hasMatch()) {
            tokens
            << match.captured(1).trimmed()
            << match.captured(2).trimmed();
            lineFormats.append(LPubVal3Format);
        }
    }
        break;
    case 5:
    {
        // VER_STICKER_PARTS_FILE
        rx.setPattern("^(\\b.*[^\\s]\\b)(?:\\s)\\s+(.*)$");
        match = rx.match(text);
        if (match.hasMatch()) {
            tokens
            << match.captured(1).trimmed()
            << match.captured(2).trimmed();
            lineFormats.append(LPubVal3Format);
        }
    }
        break;
    case 6:
    {
        // VER_LPUB3D_COLOR_PARTS
        rx.setPattern("^(\\b.*[^\\s]\\b)(?:\\s)\\s+(u|o)\\s+(.*)$");
        match = rx.match(text);
        if (match.hasMatch()) {
            tokens
            << match.captured(1).trimmed()
            << match.captured(2).trimmed()
            << match.captured(3).trimmed();
            lineFormats.append(LPubVal2Format);
            lineFormats.append(LPubVal3Format);
        }
    }
        break;
    default:
        return;
    }

    for (int i = 0; i < tokens.size(); i++) {
        if (index >= 0 && index < text.length()) {
            setFormat(index, tokens[i].length(), lineFormats[i]);
            index += tokens[i].length();
            for ( ; index < text.length(); index++) {  // move past blank spaces
                if (text[index] != ' ') {
                    break;
                }
            }
        }
    }
}

