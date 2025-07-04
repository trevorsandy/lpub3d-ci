/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** Copyright (C) 2022 - 2025 Trevor SANDY. All rights reserved.
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef FILTERLINEEDIT_H
#define FILTERLINEEDIT_H

#include <QLineEdit>
#include "lpub_qtcompat.h"

QT_BEGIN_NAMESPACE
class QAction;
class QActionGroup;
class QCompleter;
class CommandCollection;
class SnippetCollection;
QT_END_NAMESPACE

Q_DECLARE_METATYPE(RegExp)

class FilterLineEdit : public QLineEdit
{
    Q_OBJECT
    Q_PROPERTY(Qt::CaseSensitivity caseSensitivity READ caseSensitivity WRITE setCaseSensitivity)
    Q_PROPERTY(RegExp pattern READ pattern WRITE setPattern)
public:
    explicit FilterLineEdit(CommandCollection *collection, QWidget *parent = nullptr);
    explicit FilterLineEdit(SnippetCollection *collection, QWidget *parent = nullptr);

    Qt::CaseSensitivity caseSensitivity() const;
    void setCaseSensitivity(Qt::CaseSensitivity);

    RegExp pattern() const;
    void setPattern(RegExp);

signals:
    void filterChanged();

private slots:
    void completerChanged();
    void filterTriggered();
    void filterTextChanged(const QString&);

private:
    void initialize();
    CommandCollection *m_commandsCollection;
    SnippetCollection *m_snippetsCollection;
    QAction *m_caseSensitivityAction;
    QAction *m_useCompleterAction;
    QActionGroup *m_patternGroup;
};

#endif // FILTERLINEEDIT_H
