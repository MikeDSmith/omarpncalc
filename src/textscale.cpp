#include "textscale.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusVariant>
#include <QVariant>

namespace {
const QString kNamespace = QStringLiteral("org.gnome.desktop.interface");
const QString kKey = QStringLiteral("text-scaling-factor");

QVariant unwrap(QVariant value) {
    while (value.canConvert<QDBusVariant>())
        value = value.value<QDBusVariant>().variant();
    return value;
}

// Ignore nonsense and cap to the range GNOME allows.
bool sanitize(const QVariant &value, qreal *factor) {
    bool ok = false;
    const qreal scale = unwrap(value).toDouble(&ok);
    if (!ok || scale <= 0)
        return false;
    *factor = qBound(0.5, scale, 3.0);
    return true;
}
}

TextScale::TextScale(QObject *parent) : QObject(parent) {
    QDBusConnection::sessionBus().connect(
        QString(),
        QStringLiteral("/org/freedesktop/portal/desktop"),
        QStringLiteral("org.freedesktop.portal.Settings"),
        QStringLiteral("SettingChanged"),
        this,
        SLOT(handlePortalSettingChanged(QString,QString,QDBusVariant)));
    request();
}

// Ask the portal once, without blocking startup; a missing portal simply
// leaves the factor at 1.0.
void TextScale::request() {
    const QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected())
        return;

    QDBusMessage call = QDBusMessage::createMethodCall(
        QStringLiteral("org.freedesktop.portal.Desktop"),
        QStringLiteral("/org/freedesktop/portal/desktop"),
        QStringLiteral("org.freedesktop.portal.Settings"),
        QStringLiteral("Read"));
    call << kNamespace << kKey;

    auto *watcher = new QDBusPendingCallWatcher(bus.asyncCall(call), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this](QDBusPendingCallWatcher *finished) {
        const QDBusPendingReply<QDBusVariant> reply(*finished);
        finished->deleteLater();
        qreal factor = 1.0;
        if (reply.isValid() && sanitize(reply.value().variant(), &factor))
            setFactor(factor);
    });
}

void TextScale::handlePortalSettingChanged(const QString &nameSpace, const QString &key,
                                           const QDBusVariant &value) {
    if (nameSpace != kNamespace || key != kKey)
        return;
    qreal factor = 1.0;
    if (sanitize(value.variant(), &factor))
        setFactor(factor);
}

void TextScale::setFactor(qreal factor) {
    if (qFuzzyCompare(m_factor, factor))
        return;
    m_factor = factor;
    emit factorChanged(m_factor);
}
