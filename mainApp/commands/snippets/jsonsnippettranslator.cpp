/*
 * Copyright 2013-2014 Christian Loose <christian.loose@hamburg.de>
 * Copyright (C) 2022 - 2025 Trevor SANDY. All rights reserved.
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
#include "jsonsnippettranslator.h"

#include "snippet.h"

namespace {

static const QLatin1String TRIGGER("trigger");
static const QLatin1String SNIPPET("snippet");
static const QLatin1String DESCRIPTION("description");
static const QLatin1String CURSOR("cursor");
static const QLatin1String BUILTIN("builtIn");

}


Snippet JsonSnippetTranslator::fromJsonObject(const QJsonObject &object)
{
    Snippet snippet;

    snippet.trigger = object.value(TRIGGER).toString();
    snippet.snippet = object.value(SNIPPET).toString();
    snippet.description = object.value(DESCRIPTION).toString();
    snippet.cursorPosition = object.value(CURSOR).toDouble();
    snippet.builtIn = object.value(BUILTIN).toInt();

    return snippet;
}

QJsonObject JsonSnippetTranslator::toJsonObject(const Snippet &snippet)
{
    QJsonObject object;

    object.insert(TRIGGER, snippet.trigger);
    object.insert(SNIPPET, snippet.snippet);
    object.insert(DESCRIPTION, snippet.description);
    object.insert(CURSOR, snippet.cursorPosition);
    object.insert(BUILTIN, snippet.builtIn);

    return object;
}
