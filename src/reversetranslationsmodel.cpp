/*
 *  The Advanced Online Translator.
 *  Copyright (C) 2013  Oleksii Serdiuk <contacts[at]oleksii[dot]name>
 *
 *  $Id: $Format:%h %ai %an$ $
 *
 *  This file is part of The Advanced Online Translator.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "reversetranslationsmodel.h"

ReverseTranslationsModel::ReverseTranslationsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles[TermRole] = "term";
    roles[TranslationsRole] = "translations";
    setRoleNames(roles);
}

int ReverseTranslationsModel::rowCount(const QModelIndex &) const
{
    return m_terms.count();
}

QVariant ReverseTranslationsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || (index.row() < 0) || (index.row() >= m_terms.count()))
        return QVariant();

    switch (role) {
    case TermRole:
        return m_terms.at(index.row()).first;
    case TranslationsRole:
        return m_terms.at(index.row()).second.join(tr(", ", "Separator for joining string lists (don't forget space after comma)."));
    }

    return QVariant();
}

void ReverseTranslationsModel::append(const QString &term, const QStringList &translations)
{
    beginInsertRows(QModelIndex(), m_terms.count(), m_terms.count());
    m_terms.append(qMakePair(term, translations));
    endInsertRows();
}
