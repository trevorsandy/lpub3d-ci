/*
 * Copyright 2013-2014 Christian Loose <christian.loose@hamburg.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "snippetcompleter.h"

#include <QApplication>
#include <QAbstractItemView>
#include <QCompleter>
#include <QClipboard>
#include <QScrollBar>

#include "snippet.h"
#include "snippetcollection.h"
#include "snippetlistmodel.h"


SnippetCompleter::SnippetCompleter(SnippetCollection *collection, QWidget *parentWidget) :
    QObject(parentWidget),
    snippetCollection(collection),
    completer(new QCompleter(this))
{
    completer->setWidget(parentWidget);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseSensitive);

    connect(completer, SIGNAL(activated(const QString&)),
            this, SLOT(insertSnippet(const QString&)));

    SnippetListModel *model = new SnippetListModel(completer);
    connect(collection, SIGNAL(collectionChanged(SnippetCollection::CollectionChangedType,const Snippet&)),
            model, SLOT(snippetCollectionChanged(SnippetCollection::CollectionChangedType,const Snippet&)));

    completer->setModel(model);

    loadModel();
}

void SnippetCompleter::performCompletion(const QString &textUnderCursor, const QStringList &words, const QRect &popupRect)
{
    const QString completionPrefix = textUnderCursor;

    // TODO: find more elegant solution
    qobject_cast<SnippetListModel*>(completer->model())->setWords(words);

    if (completionPrefix != completer->completionPrefix()) {
        completer->setCompletionPrefix(completionPrefix);
        completer->popup()->setCurrentIndex(completer->completionModel()->index(0, 0));
    }

    if (completer->completionCount() == 1) {
        insertSnippet(completer->currentCompletion());
    } else {
        QRect rect = popupRect;
        rect.setWidth(completer->popup()->sizeHintForColumn(0) +
                completer->popup()->verticalScrollBar()->sizeHint().width());
        completer->complete(rect);
    }
}

void SnippetCompleter::loadModel() const
{
    if (snippetCollection)
        for (int i = 0; i < snippetCollection->count(); ++i) {
            Snippet snippet = snippetCollection->at(i);
            emit snippetCollection->collectionChanged(SnippetCollection::ItemAdded, snippet);
        }
}

bool SnippetCompleter::isPopupVisible() const
{
    return completer->popup()->isVisible();
}

void SnippetCompleter::hidePopup()
{
    completer->popup()->hide();
}

void SnippetCompleter::insertSnippet(const QString &trigger)
{
    if (!snippetCollection || !snippetCollection->contains(trigger)) {
        // insert word directly
        emit snippetSelected(completer->completionPrefix(), trigger, trigger.length());
        return;
    }

    const Snippet snippet = snippetCollection->snippet(trigger);

    QString snippetContent(snippet.snippet);
    replaceClipboardVariable(snippetContent);

    emit snippetSelected(completer->completionPrefix(), snippetContent, snippet.cursorPosition);
}

void SnippetCompleter::replaceClipboardVariable(QString &snippetContent)
{
    if (snippetContent.contains("%clipboard")) {
        QClipboard *clipboard = QApplication::clipboard();
        snippetContent.replace("%clipboard", clipboard->text());
    }
}
