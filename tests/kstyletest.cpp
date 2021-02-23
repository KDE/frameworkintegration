/*
    KStyle for KF5
    SPDX-FileCopyrightText: 2013 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kstyle.h"

#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

void showDialog()
{
    QScopedPointer<QDialog> dialog(new QDialog);

    auto *layout = new QVBoxLayout(dialog.data());

    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok, dialog.data());

    // Useful to change the text because setting the text triggers setShortcut
    box->button(QDialogButtonBox::Ok)->setText(QLatin1String("Send"));
    QObject::connect(box, &QDialogButtonBox::accepted, dialog.data(), &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, dialog.data(), &QDialog::reject);

    auto usefulWidget = new QTextEdit(dialog.data());
    layout->addWidget(usefulWidget);
    layout->addWidget(box);

    // Make sure we test ctrl+return acceptance with the focus on the button
    usefulWidget->setFocus();

    dialog->resize(200, 200);
    dialog->exec();
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setStyle(new KStyle);

    QWidget w;
    auto *layout = new QVBoxLayout(&w);

    QPushButton *showDialogButton = new QPushButton(QStringLiteral("Dialog"), &w);
    QObject::connect(showDialogButton, &QPushButton::clicked, &showDialog);
    layout->addWidget(showDialogButton);
    w.resize(200, 200);
    w.show();

    return app.exec();
}
