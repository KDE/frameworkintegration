/*  This file is part of the KDE libraries
 *  Copyright 2016 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License or ( at
 *  your option ) version 3 or, at the discretion of KDE e.V. ( which shall
 *  act as a proxy as in section 14 of the GPLv3 ), any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include <QCoreApplication>
#include <QUrl>
#include <QUrlQuery>
#include <QTimer>
#include <QDebug>
#include <QStandardPaths>
#include <QFile>

#include <KLocalizedString>

#include <KNotifications/KNotification>

#include <KNSCore/Engine>
#include <KNSCore/QuestionManager>

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    app.setQuitLockEnabled(false);
    Q_ASSERT(app.arguments().count() == 2);

#ifdef TEST
    QStandardPaths::setTestModeEnabled(true);
#endif

    const QUrl url(app.arguments().last());
    Q_ASSERT(url.isValid());
    Q_ASSERT(url.scheme() == QLatin1String("kns"));

    QString knsname;
    for (const auto &location : KNSCore::Engine::configSearchLocations()) {
        QString candidate = location + QLatin1Char('/') + url.host();
        if (QFile::exists(candidate)) {
            knsname = candidate;
            break;
        }
    }

    if (knsname.isEmpty()) {
        qWarning() << "couldn't find knsrc file for" << url.host();
        return 1;
    }

    const auto pathParts = url.path().split(QLatin1Char('/'), QString::SkipEmptyParts);
    if (pathParts.size() != 2) {
        qWarning() << "wrong format in the url path" << url << pathParts;
        return 1;
    }
    const auto providerid = pathParts.at(0);
    const auto entryid = pathParts.at(1);
    int linkid = 1;
    if (url.hasQuery()) {
        QUrlQuery query(url);
        if (query.hasQueryItem(QLatin1String("linkid"))) {
            bool ok;
            linkid = query.queryItemValue(QLatin1String("linkid")).toInt(&ok);
            if (!ok) {
                qWarning() << "linkid is not an integer" << url << pathParts;
                return 1;
            }
        }
    }


    KNSCore::Engine engine;
    int installedCount = 0;
    QObject::connect(KNSCore::QuestionManager::instance(), &KNSCore::QuestionManager::askQuestion, &engine, [](KNSCore::Question* question){
        auto discardQuestion = [question]() { question->setResponse(KNSCore::Question::InvalidResponse); };
        switch(question->questionType()) {
            case KNSCore::Question::YesNoQuestion: {
                auto f = KNotification::event(KNotification::StandardEvent::Notification, question->title(), question->question());
                f->setActions({i18n("Yes"), i18n("No")});
                QObject::connect(f, &KNotification::action1Activated, question, [question](){ question->setResponse(KNSCore::Question::YesResponse); });
                QObject::connect(f, &KNotification::action2Activated, question, [question](){ question->setResponse(KNSCore::Question::NoResponse); });
                QObject::connect(f, &KNotification::closed, question, discardQuestion);
            }   break;
            case KNSCore::Question::ContinueCancelQuestion: {
                auto f = KNotification::event(KNotification::StandardEvent::Notification, question->title(), question->question());
                f->setActions({i18n("Continue"), i18n("Cancel")});
                QObject::connect(f, &KNotification::action1Activated, question, [question](){ question->setResponse(KNSCore::Question::ContinueResponse); });
                QObject::connect(f, &KNotification::action2Activated, question, [question](){ question->setResponse(KNSCore::Question::CancelResponse); });
                QObject::connect(f, &KNotification::closed, question, discardQuestion);
            }   break;
            case KNSCore::Question::InputTextQuestion:
            case KNSCore::Question::SelectFromListQuestion:
            case KNSCore::Question::PasswordQuestion:
                discardQuestion();
                break;
        }
    });

    QObject::connect(&engine, &KNSCore::Engine::signalProvidersLoaded, &engine, [&engine, entryid](){
        engine.fetchEntryById(entryid);
    });

    QObject::connect(&engine, &KNSCore::Engine::signalError, &engine, [](const QString &error) {
        qWarning() << "kns error:" << error;
        QCoreApplication::exit(1);
    });
    QObject::connect(&engine, &KNSCore::Engine::signalEntryDetailsLoaded, &engine, [providerid, linkid, &engine, &installedCount](const KNSCore::EntryInternal &entry) {
//         qDebug() << "checking..." << entry.status() << entry.providerId();
        if (providerid != QUrl(entry.providerId()).host()) {
            qWarning() << "Wrong provider" << providerid << "instead of" << QUrl(entry.providerId()).host();
            QCoreApplication::exit(1);
        } else if (entry.status() == KNS3::Entry::Downloadable) {
            qDebug() << "installing...";
            installedCount++;
            engine.install(entry, linkid);
        } else if (installedCount == 0) {
            qDebug() << "already installed.";
            QCoreApplication::exit(0);
        }
    });
    QObject::connect(&engine, &KNSCore::Engine::signalEntryChanged, &engine, [&engine, &installedCount](const KNSCore::EntryInternal &entry) {
        if (entry.status() == KNS3::Entry::Installed) {
            installedCount--;
        }
        if (installedCount == 0)
            QCoreApplication::exit(0);
    });

    if (!engine.init(knsname)) {
        qWarning() << "couldn't initialize" << knsname;
        return 1;
    }
    return app.exec();
}
