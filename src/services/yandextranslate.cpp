/*
 *  The Advanced Online Translator
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

#include "yandextranslate.h"
#include "apikeys.h"

#include <qplatformdefs.h>
#include <QFile>
#include <QCoreApplication>
#include <QScriptValueIterator>

#ifdef MEEGO_EDITION_HARMATTAN
#   include <QDir>
#endif

QString YandexTranslate::displayName()
{
    return tr("Yandex.Translate");
}

#include <QDebug>
#include <QStringList>

YandexTranslate::YandexTranslate(QObject *parent) :
    JsonTranslationService(parent)
{
    m_langCodeToName.insert("", tr("Autodetect"));

    QFile f;
#ifdef Q_OS_BLACKBERRY
    f.setFileName(QCoreApplication::applicationDirPath() + QLatin1String("/qml/langs.yandex.json"));
#elif defined(MEEGO_EDITION_HARMATTAN)
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();
    f.setFileName(dir.filePath(QLatin1String("qml/langs.yandex.json")));
#else
    f.setFileName(QLatin1String("qml/langs.yandex.json"));
#endif
    if (f.open(QFile::Text | QFile::ReadOnly)) {
        QScriptValue data = parseJson(f.readAll());
        f.close();
        if (!data.isError()) {
            QScriptValueIterator langs(data.property("langs"));
            while (langs.hasNext()) {
                langs.next();
                const QString code = langs.name();
                const QString name = langs.value().toString();

                m_langCodeToName.insert(code, name);
            }

            QScriptValueIterator dirs(data.property("dirs"));
            QSet<QString> duplicates;
            while (dirs.hasNext()) {
                dirs.next();
                if (dirs.flags() & QScriptValue::SkipInEnumeration)
                    continue;

                const QStringList pair = dirs.value().toString().split("-");

                if (!duplicates.contains(pair.at(0))) {
                    const QString code = pair.at(0);
                    const QString name = m_langCodeToName.value(pair.at(0));
                    m_sourceLanguages << Language(code, name);
                    m_targetLanguages[""] << Language(code, name);
                    duplicates.insert(pair.at(0));
                }
                m_targetLanguages[pair.at(0)] << Language(pair.at(1),
                                                          m_langCodeToName.value(pair.at(1)));
            }
        }
    }

    // Sort the languages alphabetically
    qSort(m_sourceLanguages);
    m_sourceLanguages.prepend(Language("", tr("Autodetect")));
    foreach (QString key, m_targetLanguages.keys()) {
        qSort(m_targetLanguages[key]);
    }

    if (m_sourceLanguages.contains(Language("", "")))
        m_defaultLanguagePair.first = Language("", m_langCodeToName.value(""));
    else
        m_defaultLanguagePair.first = m_sourceLanguages.first();

    if (m_targetLanguages.value(m_defaultLanguagePair.first.info
                                .toString()).contains(Language("en", "")))
        m_defaultLanguagePair.second = Language("en", m_langCodeToName.value("en"));
    else
        m_defaultLanguagePair.second = m_targetLanguages.value(m_defaultLanguagePair.first.info
                                                               .toString()).first();
}

QString YandexTranslate::uid() const
{
    return "YandexTranslate";
}

bool YandexTranslate::targetLanguagesDependOnSourceLanguage() const
{
    return true;
}

bool YandexTranslate::supportsDictionary() const
{
    return false;
}

LanguageList YandexTranslate::sourceLanguages() const
{
    return m_sourceLanguages;
}

LanguageList YandexTranslate::targetLanguages(const Language &sourceLanguage) const
{
    return m_targetLanguages.value(sourceLanguage.info.toString());
}

LanguagePair YandexTranslate::defaultLanguagePair() const
{
    return LanguagePair(Language("", "Autodetect"), Language("en", "English"));
}

QString YandexTranslate::getLanguageName(const QVariant &info) const
{
    return m_langCodeToName.value(info.toString(), tr("Unknown (%1)").arg(info.toString()));
}

bool YandexTranslate::translate(const Language &from, const Language &to, const QString &text)
{
    QString lang;
    if (!from.info.toString().isEmpty())
        lang.append(from.info.toString()).append("-");
    lang.append(to.info.toString());

    QUrl url("https://translate.yandex.net/api/v1.5/tr.json/translate");
    url.addQueryItem("key", YANDEXTRANSLATE_API_KEY);
    url.addQueryItem("lang", lang);
    url.addQueryItem("options", "1");
    url.addQueryItem("text", text);

    m_reply = m_nam.get(QNetworkRequest(url));

    return true;
}

bool YandexTranslate::parseReply(const QByteArray &reply)
{
    QString json;
    json.reserve(reply.size());
    json.append("(").append(reply).append(")");

    QScriptValue data = parseJson(json);
    if (!data.isValid())
        return false;

    if (data.property("code").toInt32() != 200) {
        m_error = data.property("message").toString();
        return false;
    }

    if (data.property("detected").isObject()) {
        const QString detected = data.property("detected").property("lang").toString();
        m_detectedLanguage = Language(detected, getLanguageName(detected));
    }

    m_translation.clear();
    QScriptValueIterator ti(data.property("text"));
    while (ti.hasNext()) {
        ti.next();
        if (ti.flags() & QScriptValue::SkipInEnumeration)
            continue;
        m_translation.append(ti.value().toString());
    }

    return true;
}