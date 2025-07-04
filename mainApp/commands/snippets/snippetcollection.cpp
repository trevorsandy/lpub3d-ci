/*
 * Copyright 2013 Christian Loose <christian.loose@hamburg.de>
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
#include "snippetcollection.h"

#include <QSharedPointer>


SnippetCollection::SnippetCollection(QObject *parent) :
    QObject(parent)
{
}

int SnippetCollection::count() const
{
    return snippets.count();
}

int SnippetCollection::insert(const Snippet &snippet)
{
    QMap<QString, Snippet>::iterator it = snippets.insert(snippet.trigger, snippet);
    emit collectionChanged(SnippetCollection::ItemAdded, snippet);
    return std::distance(snippets.begin(), it);
}

void SnippetCollection::update(const Snippet &snippet)
{
    snippets.insert(snippet.trigger, snippet);
    emit collectionChanged(SnippetCollection::ItemChanged, snippet);
}

void SnippetCollection::remove(const Snippet &snippet)
{
    snippets.remove(snippet.trigger);
    emit collectionChanged(SnippetCollection::ItemDeleted, snippet);
}

const QString SnippetCollection::name() const
{
    return QStringLiteral("snippets");
}

bool SnippetCollection::contains(const QString &trigger) const
{
    return snippets.contains(trigger);
}

const Snippet SnippetCollection::snippet(const QString &trigger) const
{
    return snippets.value(trigger);
}

const Snippet &SnippetCollection::at(int offset) const
{
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    return std::next(snippets.constBegin(), offset).value();
#else
    return (snippets.begin() + offset).value();
#endif
}

QStringList SnippetCollection::triggerList() const
{
   QStringList triggerList;
   const QList values = snippets.values();
   foreach (const Snippet &snippet, values) {
       triggerList << snippet.trigger;
   }
   return triggerList;
}

QSharedPointer<SnippetCollection> SnippetCollection::userDefinedSnippets() const
{
    QSharedPointer<SnippetCollection> userDefinedSnippets = QSharedPointer<SnippetCollection>::create();
    const QList values = snippets.values();
    foreach (Snippet snippet, values) {
        if (snippet.builtIn != Snippet::True) {
            userDefinedSnippets->insert(snippet);
        }
    }

    return userDefinedSnippets;
}
