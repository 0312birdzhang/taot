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

#include "translationinterface.h"

#include <QScriptEngine>
#include <QScriptValueIterator>
#include <QDebug>

TranslationInterface::TranslationInterface(QObject *parent)
    : QObject(parent), m_service("FooService"), m_busy(false), m_dict(new DictionaryModel()), networkReply(NULL)
{
    settings.beginGroup(m_service);
    m_srcLang = settings.value("SourceLanguage", "auto").toString();
    m_trgtLang = settings.value("TargetLanguage", "en").toString();
    settings.endGroup();
    connect(&nam, SIGNAL(finished(QNetworkReply*)), SLOT(requestFinished(QNetworkReply*)));
}

bool TranslationInterface::busy() const
{
    return m_busy;
}

QString TranslationInterface::sourceLanguage() const
{
    return m_srcLang;
}

QString TranslationInterface::targetLanguage() const
{
    return m_trgtLang;
}

QString TranslationInterface::sourceText() const
{
    return m_srcText;
}

QString TranslationInterface::detectedLanguage() const
{
    return m_detected;
}

QString TranslationInterface::translatedText() const
{
    return m_translation;
}

DictionaryModel *TranslationInterface::dictionary() const
{
    return m_dict;
}

void TranslationInterface::translate()
{
    qDebug() << __PRETTY_FUNCTION__;
    resetTranslation();

    // TODO: Send request to real translation service here.
    QUrl url("http://translate.example.com/");
    url.addQueryItem("from", m_srcLang);
    url.addQueryItem("to", m_trgtLang);
    url.addQueryItem("text", m_srcText);
    networkReply = nam.get(QNetworkRequest(url));
}

void TranslationInterface::setSourceLanguage(const QString &from)
{
    qDebug() << __PRETTY_FUNCTION__;
    if (m_srcLang == from)
        return;
    resetTranslation();
    m_srcLang = from;
    settings.setValue(m_service + "/SourceLanguage", m_srcLang);
    emit fromLanguageChanged(m_srcLang);
}

void TranslationInterface::setTargetLanguage(const QString &to)
{
    qDebug() << __PRETTY_FUNCTION__;
    if (m_trgtLang == to)
        return;
    resetTranslation();
    m_trgtLang = to;
    settings.setValue(m_service + "/TargetLanguage", m_trgtLang);
    emit toLanguageChanged(m_trgtLang);
}

void TranslationInterface::setSourceText(const QString &sourceText)
{
    if (m_srcText == sourceText)
        return;
    resetTranslation();
    m_srcText = sourceText;
    emit sourceTextChanged(m_srcText);
}

void TranslationInterface::requestFinished(QNetworkReply *reply)
{
    qDebug() << __PRETTY_FUNCTION__;
    Q_ASSERT(reply == networkReply);
    networkReply = NULL;
    setBusy(false);

    if (reply->error() == QNetworkReply::OperationCanceledError) {
        // Operation was canceled by us, ignore this error.
        reply->deleteLater();
        return;
    } else if (reply->error() != QNetworkReply::NoError) {
        qDebug() << reply->errorString();
        emit error(reply->errorString());
        reply->deleteLater();
        return;
    }

    QString json;
    json.reserve(reply->size());
    json.append("(").append(QString::fromUtf8(reply->readAll())).append(")");

    QScriptEngine engine;
    if (!engine.canEvaluate(json)) {
        emit error(tr("Couldn't parse response from the server because of an error: \"%1\"")
                   .arg(tr("Can't evaluate JSON data")));
        return;
    }
    QScriptValue data = engine.evaluate(json);
    if (engine.hasUncaughtException()) {
        emit error(tr("Couldn't parse response from the server because of an error: \"%1\"")
                   .arg(engine.uncaughtException().toString()));
        return;
    }

    // TODO: Parse response.
}

void TranslationInterface::resetTranslation()
{
    setTranslatedText(QString());
    setDetectedLanguage(QString());
    m_dict->clear();
    if (!networkReply)
        return;
    if (networkReply->isRunning())
        networkReply->abort();
}

void TranslationInterface::setBusy(bool busy)
{
    if (m_busy == busy)
        return;
    m_busy = busy;
    emit busyChanged(m_busy);
}

void TranslationInterface::setDetectedLanguage(const QString &detectedLanguage)
{
    if (m_detected == detectedLanguage)
        return;
    m_detected = detectedLanguage;
    emit detectedLanguageChanged(m_detected);
}

void TranslationInterface::setTranslatedText(const QString &translatedText)
{
    if (m_translation == translatedText)
        return;
    m_translation = translatedText;
    emit translatedTextChanged(m_translation);
}
