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
#ifndef JSONCOMMANDTRANSLATOR_H
#define JSONCOMMANDTRANSLATOR_H

#include <jsontranslator.h>
#include <QJsonObject>
struct Command;


class JsonCommandTranslator : public JsonTranslator<Command>
{
private:
    Command fromJsonObject(const QJsonObject &object) Q_DECL_OVERRIDE;
    QJsonObject toJsonObject(const Command &command) Q_DECL_OVERRIDE;
};

#endif // JSONCOMMANDTRANSLATOR_H
