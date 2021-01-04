/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2016 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QCoreApplication>
#include <QUrl>
#include <QUrlQuery>
#include <QTimer>
#include <QDebug>
#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>

#include <KLocalizedString>

#include <KNotifications/KNotification>

#include <KNSCore/Engine>
#include <KNSCore/QuestionManager>

#include "knshandlerversion.h"

/**
 * Unfortunately there are two knsrc files for the window decorations, but only one is used in the KCM.
 * But both are used by third parties, consequently we can not remove one. To solve this we create a symlink
 * which links the old cache file to the new cache file, which is exposed on the GUI.
 * This way users can again remove window decorations that are installed as a dependency of a global theme.
 * BUG: 414570
 */
void createSymlinkForWindowDecorations()
{
    QFileInfo info(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/knewstuff3/aurorae.knsregistry"));
    // If we have created the symbolic link already we can exit the function here
    if (info.isSymbolicLink()) {
        return;
    }
    // Delete this file, it the KNS entries are not exposed in any GUI
    if (info.exists()) {
        QFile::remove(info.absoluteFilePath());
    }
    QFileInfo newFileInfo(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/knewstuff3/window-decorations.knsregistry"));
    QFile file(newFileInfo.absoluteFilePath());
    // Make sure that the file exists
    if (!newFileInfo.exists()) {
        file.open(QFile::WriteOnly);
        file.close();
    }
    file.link(info.absoluteFilePath());
}

int main(int argc, char** argv)
{
    createSymlinkForWindowDecorations();
    QCoreApplication app(argc, argv);
    app.setApplicationName(QLatin1String("kpackage-knshandler"));
    app.setApplicationVersion(knshandlerversion);
    app.setQuitLockEnabled(false);
    Q_ASSERT(app.arguments().count() == 2);

#ifdef TEST
    QStandardPaths::setTestModeEnabled(true);
#endif

    const QUrl url(app.arguments().last());
    Q_ASSERT(url.isValid());
    Q_ASSERT(url.scheme() == QLatin1String("kns"));

    QString knsname;
    for (const auto &location : KNSCore::Engine::configSearchLocations(true)) {
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

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    const auto pathParts = url.path().split(QLatin1Char('/'), QString::SkipEmptyParts);
#else
    const auto pathParts = url.path().split(QLatin1Char('/'), Qt::SkipEmptyParts);
#endif
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

    QObject::connect(&engine, &KNSCore::Engine::signalErrorCode, &engine, [](const KNSCore::ErrorCode &errorCode, const QString &message, const QVariant &metadata) {
        qWarning() << "kns error:" << errorCode << message << metadata;
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
