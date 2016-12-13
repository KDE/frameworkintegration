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
#include <QTimer>
#include <QDebug>
#include <QStandardPaths>

#include <KNS3/DownloadManager>

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    Q_ASSERT(app.arguments().count() == 2);

    const QUrl url(app.arguments().last());
    Q_ASSERT(url.isValid());
    Q_ASSERT(url.scheme() == QLatin1String("kns"));

    const auto knsname = QStandardPaths::locate(QStandardPaths::GenericConfigLocation, url.host());
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

    KNS3::DownloadManager manager(knsname);
    manager.fetchEntryById(entryid);
    int installedCount = 0;

    QObject::connect(&manager, &KNS3::DownloadManager::searchResult, &manager, [providerid, entryid, &manager, &installedCount](const KNS3::Entry::List &entries) {
        qDebug() << "mup" << entries.count();
        if (entries.isEmpty()) {
            qWarning() << "Couldn't find" << entryid;
            QCoreApplication::exit(1);
        } else foreach(const auto &entry, entries) {
            qDebug() << "checking..." << entry.id() << entry.status() << entry.providerId();
            if (entry.status() == KNS3::Entry::Downloadable && providerid == entry.providerId()) {
                manager.installEntry(entry);
                installedCount++;
            }

            if (installedCount == 0) {
                qDebug() << "nothing to install...";
                QCoreApplication::exit(0);
            }
        }
    });
    QObject::connect(&manager, &KNS3::DownloadManager::entryStatusChanged, &manager, [&manager, &installedCount](const KNS3::Entry &/*entry*/) {
        installedCount--;
        if (installedCount == 0)
            QCoreApplication::exit(0);
    });

    return app.exec();
}
