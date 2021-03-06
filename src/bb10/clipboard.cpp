/*
 *  TAO Translator
 *  Copyright (C) 2013-2015  Oleksii Serdiuk <contacts[at]oleksii[dot]name>
 *
 *  $Id: $Format:%h %ai %an$ $
 *
 *  This file is part of TAO Translator.
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

#include "clipboard.h"

#include <QFileSystemWatcher>
#include <bb/system/Clipboard>

Clipboard::Clipboard(QObject *parent)
    : QObject(parent)
    , m_clipboard(new bb::system::Clipboard(this))
    , m_text(QString::fromUtf8(m_clipboard->value("text/plain")))
    , m_clipboardWatcher(new QFileSystemWatcher(this))
{
    connect(m_clipboardWatcher, SIGNAL(fileChanged(QString)), SLOT(onFileChanged(QString)));
    m_clipboardWatcher->addPath("/accounts/1000/clipboard/text.plain");
}

bool Clipboard::clear()
{
    return m_clipboard->clear();
}

bool Clipboard::isEmpty() const
{
    return m_text.isEmpty();
}

QString Clipboard::text() const
{
    return m_text;
}

bool Clipboard::insert(const QString &text)
{
    m_clipboard->clear();
    return m_clipboard->insert("text/plain", text.toUtf8());
}

void Clipboard::onFileChanged(const QString &path)
{
    const QString text = QString::fromUtf8(m_clipboard->value("text/plain"));
    if (m_text == text)
        return;

    m_text = text;
    emit textChanged();
}
