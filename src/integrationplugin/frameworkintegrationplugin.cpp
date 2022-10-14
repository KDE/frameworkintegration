/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2012 David Faure <faure+bluesystems@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "frameworkintegrationplugin.h"
#include <KConfigGroup>
#include <KNotification>
#include <KSharedConfig>

#include <QDebug>
#include <qplugin.h>

#if KWIDGETSADDONS_BUILD_DEPRECATED_SINCE(5, 100)
bool KMessageBoxDontAskAgainConfigStorage::shouldBeShownYesNo(const QString &dontShowAgainName, KMessageBox::ButtonCode &result)
#else
bool KMessageBoxDontAskAgainConfigStorage::shouldBeShownTwoActions(const QString &dontShowAgainName, KMessageBox::ButtonCode &result)
#endif
{
    KConfigGroup cg(KMessageBox_againConfig ? KMessageBox_againConfig : KSharedConfig::openConfig().data(), "Notification Messages");
    const QString dontAsk = cg.readEntry(dontShowAgainName, QString()).toLower();
    if (dontAsk == QLatin1String("yes") || dontAsk == QLatin1String("true")) {
        result = KMessageBox::PrimaryAction;
        return false;
    }
    if (dontAsk == QLatin1String("no") || dontAsk == QLatin1String("false")) {
        result = KMessageBox::SecondaryAction;
        return false;
    }
    return true;
}

bool KMessageBoxDontAskAgainConfigStorage::shouldBeShownContinue(const QString &dontShowAgainName)
{
    KConfigGroup cg(KMessageBox_againConfig ? KMessageBox_againConfig : KSharedConfig::openConfig().data(), "Notification Messages");
    return cg.readEntry(dontShowAgainName, true);
}

#if KWIDGETSADDONS_BUILD_DEPRECATED_SINCE(5, 100)
void KMessageBoxDontAskAgainConfigStorage::saveDontShowAgainYesNo(const QString &dontShowAgainName, KMessageBox::ButtonCode result)
#else
void KMessageBoxDontAskAgainConfigStorage::saveDontShowAgainTwoActions(const QString &dontShowAgainName, KMessageBox::ButtonCode result)
#endif
{
    KConfigGroup::WriteConfigFlags flags = KConfig::Persistent;
    if (dontShowAgainName[0] == QLatin1Char(':')) {
        flags |= KConfigGroup::Global;
    }
    KConfigGroup cg(KMessageBox_againConfig ? KMessageBox_againConfig : KSharedConfig::openConfig().data(), "Notification Messages");
    cg.writeEntry(dontShowAgainName, result == KMessageBox::PrimaryAction, flags);
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

void KMessageBoxNotify::sendNotification(QMessageBox::Icon notificationType, const QString &message, QWidget *parent)
{
    QString messageType;
    switch (notificationType) {
    case QMessageBox::Warning:
        messageType = QStringLiteral("messageWarning");
        break;
    case QMessageBox::Critical:
        messageType = QStringLiteral("messageCritical");
        break;
    case QMessageBox::Question:
        messageType = QStringLiteral("messageQuestion");
        break;
    default:
        messageType = QStringLiteral("messageInformation");
        break;
    }

    KNotification::event(messageType, message, QPixmap(), parent, KNotification::DefaultEvent | KNotification::CloseOnTimeout);
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
