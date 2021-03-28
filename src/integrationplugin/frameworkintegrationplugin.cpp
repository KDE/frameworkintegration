/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2012 David Faure <faure+bluesystems@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "frameworkintegrationplugin.h"
#include <KConfigGroup>
#include <KSharedConfig>

#include <QDebug>
#include <QFileInfo>
#include <QGuiApplication>
#include <qplugin.h>

#include <canberra.h>

bool KMessageBoxDontAskAgainConfigStorage::shouldBeShownYesNo(const QString &dontShowAgainName, KMessageBox::ButtonCode &result)
{
    KConfigGroup cg(KMessageBox_againConfig ? KMessageBox_againConfig : KSharedConfig::openConfig().data(), "Notification Messages");
    const QString dontAsk = cg.readEntry(dontShowAgainName, QString()).toLower();
    if (dontAsk == QLatin1String("yes") || dontAsk == QLatin1String("true")) {
        result = KMessageBox::Yes;
        return false;
    }
    if (dontAsk == QLatin1String("no") || dontAsk == QLatin1String("false")) {
        result = KMessageBox::No;
        return false;
    }
    return true;
}

bool KMessageBoxDontAskAgainConfigStorage::shouldBeShownContinue(const QString &dontShowAgainName)
{
    KConfigGroup cg(KMessageBox_againConfig ? KMessageBox_againConfig : KSharedConfig::openConfig().data(), "Notification Messages");
    return cg.readEntry(dontShowAgainName, true);
}

void KMessageBoxDontAskAgainConfigStorage::saveDontShowAgainYesNo(const QString &dontShowAgainName, KMessageBox::ButtonCode result)
{
    KConfigGroup::WriteConfigFlags flags = KConfig::Persistent;
    if (dontShowAgainName[0] == QLatin1Char(':')) {
        flags |= KConfigGroup::Global;
    }
    KConfigGroup cg(KMessageBox_againConfig ? KMessageBox_againConfig : KSharedConfig::openConfig().data(), "Notification Messages");
    cg.writeEntry(dontShowAgainName, result == KMessageBox::Yes, flags);
    cg.sync();
}

void KMessageBoxDontAskAgainConfigStorage::saveDontShowAgainContinue(const QString &dontShowAgainName)
{
    KConfigGroup::WriteConfigFlags flags = KConfigGroup::Persistent;
    if (dontShowAgainName[0] == QLatin1Char(':')) {
        flags |= KConfigGroup::Global;
    }
    KConfigGroup cg(KMessageBox_againConfig ? KMessageBox_againConfig : KSharedConfig::openConfig().data(), "Notification Messages");
    cg.writeEntry(dontShowAgainName, false, flags);
    cg.sync();
}

void KMessageBoxDontAskAgainConfigStorage::enableAllMessages()
{
    KConfig *config = KMessageBox_againConfig ? KMessageBox_againConfig : KSharedConfig::openConfig().data();
    if (!config->hasGroup("Notification Messages")) {
        return;
    }

    KConfigGroup cg(config, "Notification Messages");

    typedef QMap<QString, QString> configMap;

    const configMap map = cg.entryMap();

    configMap::ConstIterator it;
    for (it = map.begin(); it != map.end(); ++it) {
        cg.deleteEntry(it.key());
    }
}

void KMessageBoxDontAskAgainConfigStorage::enableMessage(const QString &dontShowAgainName)
{
    KConfig *config = KMessageBox_againConfig ? KMessageBox_againConfig : KSharedConfig::openConfig().data();
    if (!config->hasGroup("Notification Messages")) {
        return;
    }

    KConfigGroup cg(config, "Notification Messages");

    cg.deleteEntry(dontShowAgainName);
    config->sync();
}

KMessageBoxNotify::KMessageBoxNotify()
{
    int ret = ca_context_create(&m_context);
    if (ret != CA_SUCCESS) {
        qWarning() << "Failed to initialize canberra context for audio:" << ca_strerror(ret);
        m_context = nullptr;
        return;
    }

    QString desktopFileName = QGuiApplication::desktopFileName();
    // handle apps which set the desktopFileName property with filename suffix,
    // due to unclear API dox (https://bugreports.qt.io/browse/QTBUG-75521)
    if (desktopFileName.endsWith(QLatin1String(".desktop"))) {
        desktopFileName.chop(8);
    }
    ret = ca_context_change_props(m_context,
                                  CA_PROP_APPLICATION_NAME,
                                  qUtf8Printable(qApp->applicationDisplayName()),
                                  CA_PROP_APPLICATION_ID,
                                  qUtf8Printable(desktopFileName),
                                  CA_PROP_APPLICATION_ICON_NAME,
                                  qUtf8Printable(qApp->windowIcon().name()),
                                  nullptr);
    if (ret != CA_SUCCESS) {
        qWarning() << "Failed to set application properties on canberra context for audio notification:" << ca_strerror(ret);
    }
}

KMessageBoxNotify::~KMessageBoxNotify()
{
    if (m_context) {
        ca_context_destroy(m_context);
    }
    m_context = nullptr;
}

void KMessageBoxNotify::sendNotification(QMessageBox::Icon notificationType, const QString &message, QWidget *parent)
{
    Q_UNUSED(message);
    Q_UNUSED(parent);

    QString soundFilename;

    switch (notificationType) {
    case QMessageBox::Warning:
        soundFilename = QStringLiteral("Oxygen-Sys-Warning.ogg");
        break;
    case QMessageBox::Critical:
        soundFilename = QStringLiteral("Oxygen-Sys-App-Error-Critical.ogg");
        break;
    case QMessageBox::Question:
        soundFilename = QStringLiteral("Oxygen-Sys-Question.ogg");
        break;
    default:
        soundFilename = QStringLiteral("Oxygen-Sys-App-Message.ogg");
        break;
    }

    QUrl soundURL;
    const auto dataLocations = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (const QString &dataLocation : dataLocations) {
        soundURL = QUrl::fromUserInput(soundFilename, dataLocation + QStringLiteral("/sounds"), QUrl::AssumeLocalFile);
        if (soundURL.isLocalFile() && QFileInfo::exists(soundURL.toLocalFile())) {
            break;
        } else if (!soundURL.isLocalFile() && soundURL.isValid()) {
            break;
        }
        soundURL.clear();
    }
    if (soundURL.isEmpty()) {
        qWarning() << "Could not find audio file" << soundFilename;
        return;
    }

    int ret = ca_context_play(m_context, 0 /*arbitrary id*/, CA_PROP_MEDIA_FILENAME, QFile::encodeName(soundURL.toLocalFile()).constData(), nullptr);

    if (ret != CA_SUCCESS) {
        qWarning() << "Failed to play sound with canberra:" << ca_strerror(ret);
    }
}

KFrameworkIntegrationPlugin::KFrameworkIntegrationPlugin()
    : QObject()
{
    setProperty(KMESSAGEBOXDONTASKAGAIN_PROPERTY, QVariant::fromValue<KMessageBoxDontAskAgainInterface *>(&m_dontAskAgainConfigStorage));
    setProperty(KMESSAGEBOXNOTIFY_PROPERTY, QVariant::fromValue<KMessageBoxNotifyInterface *>(&m_notify));
}

void KFrameworkIntegrationPlugin::reparseConfiguration()
{
    KSharedConfig::openConfig()->reparseConfiguration();
}
