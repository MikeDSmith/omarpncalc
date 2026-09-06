#include "hyprland.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QStandardPaths>

Hyprland::Hyprland(QObject *parent) : QObject(parent) {
    if (!qEnvironmentVariableIsEmpty("HYPRLAND_INSTANCE_SIGNATURE"))
        m_hyprctl = QStandardPaths::findExecutable(QStringLiteral("hyprctl"));
}

// Wayland never tells a client the work area -- Qt's Screen.desktopAvailable*
// just repeats the full screen size -- so the space a bar reserves has to come
// from the compositor. `reserved` is [left, top, right, bottom]; confirmed by
// toggling the top bar and watching index 1 move between 26 and 0.
QSize Hyprland::workArea() const {
    if (!available())
        return QSize();

    QProcess hyprctl;
    hyprctl.start(m_hyprctl, { QStringLiteral("monitors"), QStringLiteral("-j") });
    if (!hyprctl.waitForFinished(1000))
        return QSize();

    const QJsonArray monitors = QJsonDocument::fromJson(hyprctl.readAllStandardOutput()).array();
    for (const QJsonValue &value : monitors) {
        const QJsonObject monitor = value.toObject();
        if (!monitor.value(QStringLiteral("focused")).toBool())
            continue;

        const double scale = monitor.value(QStringLiteral("scale")).toDouble();
        if (scale <= 0)
            break;

        const QJsonArray reserved = monitor.value(QStringLiteral("reserved")).toArray();
        const int width = qRound(monitor.value(QStringLiteral("width")).toDouble() / scale)
            - reserved.at(0).toInt() - reserved.at(2).toInt();
        const int height = qRound(monitor.value(QStringLiteral("height")).toDouble() / scale)
            - reserved.at(1).toInt() - reserved.at(3).toInt();
        if (width > 0 && height > 0)
            return QSize(width, height);
        break;
    }
    return QSize();
}
