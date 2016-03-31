/**
 * KStyle for KDE4
 * Copyright (C) 2004-2005 Maksim Orlovich <maksim@kde.org>
 * Copyright (C) 2005,2006 Sandro Giessl <giessl@kde.org>
 *
 * Based in part on the following software:
 *  KStyle for KDE3
 *      Copyright (C) 2001-2002 Karol Szwed <gallium@kde.org>
 *      Portions  (C) 1998-2000 TrollTech AS
 *  Keramik for KDE3,
 *      Copyright (C) 2002      Malte Starostik   <malte@kde.org>
 *                (C) 2002-2003 Maksim Orlovich  <maksim@kde.org>
 *      Portions  (C) 2001-2002 Karol Szwed     <gallium@kde.org>
 *                (C) 2001-2002 Fredrik Höglund <fredrik@kde.org>
 *                (C) 2000 Daniel M. Duley       <mosfet@kde.org>
 *                (C) 2000 Dirk Mueller         <mueller@kde.org>
 *                (C) 2001 Martijn Klingens    <klingens@kde.org>
 *                (C) 2003 Sandro Giessl      <sandro@giessl.com>
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 * Many thanks to Bradley T. Hughes for the 3 button scrollbar code.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kstyle.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QDialogButtonBox>
#include <QEvent>
#include <QIcon>
#include <QStyleOption>
#include <QPushButton>
#include <QToolBar>

#include <kconfiggroup.h>
#include <kiconloader.h>
#include <kcolorscheme.h>
#include <config-kstyle.h>
#if HAVE_X11
#include <QX11Info>
#include <xcb/xcb.h>
#endif

// ----------------------------------------------------------------------------

static const QStyle::StyleHint SH_KCustomStyleElement = (QStyle::StyleHint)0xff000001;
static const int X_KdeBase = 0xff000000;
static const char s_schemePropertyName[] = "KDE_COLOR_SCHEME_PATH";

/**
 * An event filter intended to be used for all toplevel QWidgets.
 * The filter checks for Show and PaletteChange events and passes the
 * KDE_COLOR_SCHEME_PATH property on the QApplication to the native window.
 * The application property is set by e.g. the KColorSchemeManager and the
 * information is needed by KDE's window manager to use the same color scheme
 * on e.g. the window decoration to provide a consistent look and feel.
 */
class ColorSchemeFilter : public QObject
{
public:
    ColorSchemeFilter(QObject *parent = Q_NULLPTR);
    bool eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE;
private:
    void installColorScheme(QWidget *w);
};

ColorSchemeFilter::ColorSchemeFilter(QObject *parent)
    : QObject(parent)
{
}

bool ColorSchemeFilter::eventFilter(QObject *object, QEvent *event)
{
    QWidget *w = static_cast<QWidget*>(object);
    if ((event->type() == QEvent::Show && qApp->property(s_schemePropertyName).isValid()) ||
            (event->type() == QEvent::PaletteChange)) {
        installColorScheme(w);
    }
    return QObject::eventFilter(object, event);
}

void ColorSchemeFilter::installColorScheme(QWidget *w)
{
    if (!w || !w->isTopLevel()) {
        return;
    }
#if HAVE_X11
    static const bool s_x11 = QX11Info::isPlatformX11();
    if (!s_x11) {
        return;
    }
    static xcb_atom_t atom = XCB_ATOM_NONE;
    xcb_connection_t *c = QX11Info::connection();
    if (atom == XCB_ATOM_NONE) {
        const QByteArray name = QByteArrayLiteral("_KDE_NET_WM_COLOR_SCHEME");
        const xcb_intern_atom_cookie_t cookie = xcb_intern_atom(c, false, name.length(), name.constData());
        QScopedPointer<xcb_intern_atom_reply_t, QScopedPointerPodDeleter> reply(xcb_intern_atom_reply(c, cookie, Q_NULLPTR));
        if (!reply.isNull()) {
            atom = reply->atom;
        } else {
            // no point in continuing, we don't have the atom
            return;
        }
    }
    const QString path = qApp->property(s_schemePropertyName).toString();
    if (path.isEmpty()) {
        xcb_delete_property(c, w->winId(), atom);
    } else {
        xcb_change_property(c, XCB_PROP_MODE_REPLACE, w->winId(), atom, XCB_ATOM_STRING,
                            8, path.size(), qPrintable(path));
    }
#endif
}

class KStylePrivate
{
public:
    KStylePrivate();

    QHash<QString, int> styleElements;
    int hintCounter, controlCounter, subElementCounter;
    QScopedPointer<ColorSchemeFilter> colorSchemeFilter;
};

KStylePrivate::KStylePrivate()
    : colorSchemeFilter(new ColorSchemeFilter())
{
    controlCounter = subElementCounter = X_KdeBase;
    hintCounter = X_KdeBase + 1; //sic! X_KdeBase is covered by SH_KCustomStyleElement
}

/*
    The functions called by widgets that request custom element support, passed to the effective style.
    Collected in a static inline function due to similarity.
*/

static inline int customStyleElement(QStyle::StyleHint type, const QString &element, QWidget *widget)
{
    if (!widget || widget->style()->metaObject()->indexOfClassInfo("X-KDE-CustomElements") < 0) {
        return 0;
    }

    const QString originalName = widget->objectName();
    widget->setObjectName(element);
    const int id = widget->style()->styleHint(type, 0, widget);
    widget->setObjectName(originalName);
    return id;
}

QStyle::StyleHint KStyle::customStyleHint(const QString &element, const QWidget *widget)
{
    return (StyleHint) customStyleElement(SH_KCustomStyleElement, element, const_cast<QWidget *>(widget));
}

QStyle::ControlElement KStyle::customControlElement(const QString &element, const QWidget *widget)
{
    return (ControlElement) customStyleElement(SH_KCustomStyleElement, element, const_cast<QWidget *>(widget));
}

QStyle::SubElement KStyle::customSubElement(const QString &element, const QWidget *widget)
{
    return (SubElement) customStyleElement(SH_KCustomStyleElement, element, const_cast<QWidget *>(widget));
}

KStyle::KStyle() : d(new KStylePrivate)
{
}

KStyle::~KStyle()
{
    delete d;
}

/*
    Custom Style Element runtime extension:
    We reserve one StyleHint to let the effective style inform widgets whether it supports certain
    string based style elements.
    As this could lead to number conflicts (i.e. an app utilizing one of the hints itself for other
    purposes) there're various safety mechanisms to rule out such interference.

    1) It's most unlikely that a widget in some 3rd party app will accidentally call a general
    QStyle/KStyle styleHint() or draw*() and (unconditionally) expect a valid return, however:
    a. The StyleHint is not directly above Qt's custom base, assuming most 3rd party apps would
    - in case - make use of such
    b. In order to be accepted, the StyleHint query must pass a widget with a perfectly matching
    name, containing the typical element prefix ("CE_", etc.) and being supported by the current style
    c. Instead using Qt's fragile qstyleoption_cast on the QStyleOption provided to the StyleHint
    query, try to dump out a string and hope for the best, we now manipulate the widgets objectName().
    Plain Qt dependent widgets can do that themselves and if a widget uses KStyle's convenience access
    functions, it won't notice this at all

    2) The key problem is that a common KDE widget will run into an apps custom style which will then
    falsely respond to the styleHint() call with an invalid value.
    To prevent this, supporting styles *must* set a Q_CLASSINFO "X-KDE-CustomElements".

    3) If any of the above traps snaps, the returned id is 0 - the QStyle default, indicating
    that this element is not supported by the current style.

    Obviously, this contains the "diminished clean" action to (temporarily) manipulate the
    objectName() of a const QWidget* - but this happens completely inside KStyle or the widget, if
    it does not make use of KStyles static convenience functions.
    My biggest worry here would be, that in a multithreaded environment a thread (usually not being
    owner of the widget) does something crucially relying on the widgets name property...
    This however would also have to happen during the widget construction or stylechanges, when
    the functions in doubt will typically be called.
    So this is imho unlikely causing any trouble, ever.
*/

/*
    The functions called by the real style implementation to add support for a certain element.
    Checks for well-formed string (containing the element prefix) and returns 0 otherwise.
    Checks whether the element is already supported or inserts it otherwise; Returns the proper id
    NOTICE: We could check for "X-KDE-CustomElements", but this would bloat style start up times
    (if they e.g. register 100 elements or so)
*/

static inline int newStyleElement(const QString &element, const char *check, int &counter, QHash<QString, int> *elements)
{
    if (!element.contains(check)) {
        return 0;
    }
    int id = elements->value(element, 0);
    if (!id) {
        ++counter;
        id = counter;
        elements->insert(element, id);
    }
    return id;
}

QStyle::StyleHint KStyle::newStyleHint(const QString &element)
{
    return (StyleHint)newStyleElement(element, "SH_", d->hintCounter, &d->styleElements);
}

QStyle::ControlElement KStyle::newControlElement(const QString &element)
{
    return (ControlElement)newStyleElement(element, "CE_", d->controlCounter, &d->styleElements);
}

KStyle::SubElement KStyle::newSubElement(const QString &element)
{
    return (SubElement)newStyleElement(element, "SE_", d->subElementCounter, &d->styleElements);
}

void KStyle::polish(QWidget *w)
{
    // Enable hover effects in all itemviews
    if (QAbstractItemView *itemView = qobject_cast<QAbstractItemView *>(w)) {
        itemView->viewport()->setAttribute(Qt::WA_Hover);
    }

    if (QDialogButtonBox *box = qobject_cast<QDialogButtonBox *>(w)) {
        QPushButton *button = box->button(QDialogButtonBox::Ok);
        if (button && button->shortcut().isEmpty()) {
            button->setShortcut(Qt::CTRL | Qt::Key_Return);
        }
    }

    // install the event filter to pass the color scheme app property to all toplevel native windows
    if (w->isTopLevel()) {
        w->installEventFilter(d->colorSchemeFilter.data());
    }

    QCommonStyle::polish(w);
}

QPalette KStyle::standardPalette() const
{
    return KColorScheme::createApplicationPalette(KSharedConfig::openConfig());
}

QIcon KStyle::standardIcon(StandardPixmap standardIcon, const QStyleOption *option,
                           const QWidget *widget) const
{
    switch (standardIcon) {
    case QStyle::SP_DesktopIcon:
        return QIcon::fromTheme(QStringLiteral("user-desktop"));
    case QStyle::SP_TrashIcon:
        return QIcon::fromTheme(QStringLiteral("user-trash"));
    case QStyle::SP_ComputerIcon:
        return QIcon::fromTheme(QStringLiteral("computer"));
    case QStyle::SP_DriveFDIcon:
        return QIcon::fromTheme(QStringLiteral("media-floppy"));
    case QStyle::SP_DriveHDIcon:
        return QIcon::fromTheme(QStringLiteral("drive-harddisk"));
    case QStyle::SP_DriveCDIcon:
    case QStyle::SP_DriveDVDIcon:
        return QIcon::fromTheme(QStringLiteral("drive-optical"));
    case QStyle::SP_DriveNetIcon:
        return QIcon::fromTheme(QStringLiteral("folder-remote"));
    case QStyle::SP_DirHomeIcon:
        return QIcon::fromTheme(QStringLiteral("user-home"));
    case QStyle::SP_DirOpenIcon:
        return QIcon::fromTheme(QStringLiteral("document-open-folder"));
    case QStyle::SP_DirClosedIcon:
        return QIcon::fromTheme(QStringLiteral("folder"));
    case QStyle::SP_DirIcon:
        return QIcon::fromTheme(QStringLiteral("folder"));
    case QStyle::SP_DirLinkIcon:
        return QIcon::fromTheme(QStringLiteral("folder")); //TODO: generate (!?) folder with link emblem
    case QStyle::SP_FileIcon:
        return QIcon::fromTheme(QStringLiteral("text-plain")); //TODO: look for a better icon
    case QStyle::SP_FileLinkIcon:
        return QIcon::fromTheme(QStringLiteral("text-plain")); //TODO: generate (!?) file with link emblem
    case QStyle::SP_FileDialogStart:
        return QIcon::fromTheme(QStringLiteral("media-playback-start")); //TODO: find correct icon
    case QStyle::SP_FileDialogEnd:
        return QIcon::fromTheme(QStringLiteral("media-playback-stop")); //TODO: find correct icon
    case QStyle::SP_FileDialogToParent:
        return QIcon::fromTheme(QStringLiteral("go-up"));
    case QStyle::SP_FileDialogNewFolder:
        return QIcon::fromTheme(QStringLiteral("folder-new"));
    case QStyle::SP_FileDialogDetailedView:
        return QIcon::fromTheme(QStringLiteral("view-list-details"));
    case QStyle::SP_FileDialogInfoView:
        return QIcon::fromTheme(QStringLiteral("document-properties"));
    case QStyle::SP_FileDialogContentsView:
        return QIcon::fromTheme(QStringLiteral("view-list-icons"));
    case QStyle::SP_FileDialogListView:
        return QIcon::fromTheme(QStringLiteral("view-list-text"));
    case QStyle::SP_FileDialogBack:
        return QIcon::fromTheme(QStringLiteral("go-previous"));
    case QStyle::SP_MessageBoxInformation:
        return QIcon::fromTheme(QStringLiteral("dialog-information"));
    case QStyle::SP_MessageBoxWarning:
        return QIcon::fromTheme(QStringLiteral("dialog-warning"));
    case QStyle::SP_MessageBoxCritical:
        return QIcon::fromTheme(QStringLiteral("dialog-error"));
    case QStyle::SP_MessageBoxQuestion:
        return QIcon::fromTheme(QStringLiteral("dialog-information"));
    case QStyle::SP_DialogOkButton:
        return QIcon::fromTheme(QStringLiteral("dialog-ok"));
    case QStyle::SP_DialogCancelButton:
        return QIcon::fromTheme(QStringLiteral("dialog-cancel"));
    case QStyle::SP_DialogHelpButton:
        return QIcon::fromTheme(QStringLiteral("help-contents"));
    case QStyle::SP_DialogOpenButton:
        return QIcon::fromTheme(QStringLiteral("document-open"));
    case QStyle::SP_DialogSaveButton:
        return QIcon::fromTheme(QStringLiteral("document-save"));
    case QStyle::SP_DialogCloseButton:
        return QIcon::fromTheme(QStringLiteral("dialog-close"));
    case QStyle::SP_DialogApplyButton:
        return QIcon::fromTheme(QStringLiteral("dialog-ok-apply"));
    case QStyle::SP_DialogResetButton:
        return QIcon::fromTheme(QStringLiteral("document-revert"));
    case QStyle::SP_DialogDiscardButton:
        return QIcon::fromTheme(QStringLiteral("dialog-cancel"));
    case QStyle::SP_DialogYesButton:
        return QIcon::fromTheme(QStringLiteral("dialog-ok-apply"));
    case QStyle::SP_DialogNoButton:
        return QIcon::fromTheme(QStringLiteral("dialog-cancel"));
    case QStyle::SP_ArrowUp:
        return QIcon::fromTheme(QStringLiteral("go-up"));
    case QStyle::SP_ArrowDown:
        return QIcon::fromTheme(QStringLiteral("go-down"));
    case QStyle::SP_ArrowLeft:
        return QIcon::fromTheme(QStringLiteral("go-previous-view"));
    case QStyle::SP_ArrowRight:
        return QIcon::fromTheme(QStringLiteral("go-next-view"));
    case QStyle::SP_ArrowBack:
        return QIcon::fromTheme(QStringLiteral("go-previous"));
    case QStyle::SP_ArrowForward:
        return QIcon::fromTheme(QStringLiteral("go-next"));
    case QStyle::SP_BrowserReload:
        return QIcon::fromTheme(QStringLiteral("view-refresh"));
    case QStyle::SP_BrowserStop:
        return QIcon::fromTheme(QStringLiteral("process-stop"));
    case QStyle::SP_MediaPlay:
        return QIcon::fromTheme(QStringLiteral("media-playback-start"));
    case QStyle::SP_MediaStop:
        return QIcon::fromTheme(QStringLiteral("media-playback-stop"));
    case QStyle::SP_MediaPause:
        return QIcon::fromTheme(QStringLiteral("media-playback-pause"));
    case QStyle::SP_MediaSkipForward:
        return QIcon::fromTheme(QStringLiteral("media-skip-forward"));
    case QStyle::SP_MediaSkipBackward:
        return QIcon::fromTheme(QStringLiteral("media-skip-backward"));
    case QStyle::SP_MediaSeekForward:
        return QIcon::fromTheme(QStringLiteral("media-seek-forward"));
    case QStyle::SP_MediaSeekBackward:
        return QIcon::fromTheme(QStringLiteral("media-seek-backward"));
    case QStyle::SP_MediaVolume:
        return QIcon::fromTheme(QStringLiteral("audio-volume-medium"));
    case QStyle::SP_MediaVolumeMuted:
        return QIcon::fromTheme(QStringLiteral("audio-volume-muted"));

    default:
        break;
    }

    return QCommonStyle::standardIcon(standardIcon, option, widget);
}

int KStyle::styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    switch (hint) {
    case SH_DialogButtonBox_ButtonsHaveIcons: {
        // was KGlobalSettings::showIconsOnPushButtons() :
        KConfigGroup g(KSharedConfig::openConfig(), "KDE");
        return g.readEntry("ShowIconsOnPushButtons", true);
    }

    case SH_ItemView_ArrowKeysNavigateIntoChildren:
        return true;

    case SH_Widget_Animate: {
        KConfigGroup g(KSharedConfig::openConfig(), "KDE-Global GUI Settings");
        return g.readEntry("GraphicEffectsLevel", true);
    }

    #if QT_VERSION >= QT_VERSION_CHECK(5,5,0)
    case QStyle::SH_Menu_SubMenuSloppyCloseTimeout: return 300;
    #endif

    case SH_ToolButtonStyle: {
        KConfigGroup g(KSharedConfig::openConfig(), "Toolbar style");

        bool useOthertoolbars = false;
        const QWidget *parent = widget ? widget->parentWidget() : nullptr;

        //If the widget parent is a QToolBar and the magic property is set
        if (parent && qobject_cast< const QToolBar * >(parent)) {
            if (parent->property("otherToolbar").isValid()) {
                useOthertoolbars = true;
            }
        }

        QString buttonStyle;
        if (useOthertoolbars) {
            buttonStyle = g.readEntry("ToolButtonStyleOtherToolbars", "NoText").toLower();
        } else {
            buttonStyle = g.readEntry("ToolButtonStyle", "TextBesideIcon").toLower();
        }

        return buttonStyle == QLatin1String("textbesideicon") ? Qt::ToolButtonTextBesideIcon
               : buttonStyle == QLatin1String("icontextright") ? Qt::ToolButtonTextBesideIcon
               : buttonStyle == QLatin1String("textundericon") ? Qt::ToolButtonTextUnderIcon
               : buttonStyle == QLatin1String("icontextbottom") ? Qt::ToolButtonTextUnderIcon
               : buttonStyle == QLatin1String("textonly") ? Qt::ToolButtonTextOnly
               : Qt::ToolButtonIconOnly;
    }

    case SH_KCustomStyleElement:
        if (!widget) {
            return 0;
        }

        return d->styleElements.value(widget->objectName(), 0);

    default:
        break;
    };

    return QCommonStyle::styleHint(hint, option, widget, returnData);
}

int KStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric) {
    case PM_SmallIconSize:
    case PM_ButtonIconSize:
        return KIconLoader::global()->currentSize(KIconLoader::Small);

    case PM_ToolBarIconSize:
        return KIconLoader::global()->currentSize(KIconLoader::Toolbar);

    case PM_LargeIconSize:
        return KIconLoader::global()->currentSize(KIconLoader::Dialog);

    case PM_MessageBoxIconSize:
        // TODO return KIconLoader::global()->currentSize(KIconLoader::MessageBox);
        return KIconLoader::SizeHuge;
    default:
        break;
    }

    return QCommonStyle::pixelMetric(metric, option, widget);
}

