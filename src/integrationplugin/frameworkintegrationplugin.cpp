/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2012 David Faure <faure+bluesystems@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "frameworkintegrationplugin.h"
#include <KNotification>

#include <qplugin.h>

void KMessageBoxNotify::sendNotification(QMessageBox::Icon notificationType, const QString &message, QWidget * /*parent*/)
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

    KNotification::event(messageType, message, QPixmap(), KNotification::DefaultEvent | KNotification::CloseOnTimeout);
}

KFrameworkIntegrationPlugin::KFrameworkIntegrationPlugin()
    : QObject()
{
    setProperty(KMESSAGEBOXNOTIFY_PROPERTY, QVariant::fromValue<KMessageBoxNotifyInterface *>(&m_notify));
}

void KFrameworkIntegrationPlugin::reparseConfiguration()
{
}

#include "moc_frameworkintegrationplugin.cpp"
