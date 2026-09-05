#include "hyprland.h"

#include <QCoreApplication>
#include <QProcess>
#include <QStandardPaths>

Hyprland::Hyprland(QObject *parent) : QObject(parent) {
    if (!qEnvironmentVariableIsEmpty("HYPRLAND_INSTANCE_SIGNATURE"))
        m_hyprctl = QStandardPaths::findExecutable(QStringLiteral("hyprctl"));
}

void Hyprland::resizeWindow(int width, int height) const {
    if (!available())
        return;
    const QString lua = QStringLiteral(
        "for _, w in ipairs(hl.get_windows({ class = \"%1\" })) do "
        "if w.pid == %2 then "
        "hl.dispatch(hl.dsp.window.resize({ x = %3, y = %4, relative = false, window = w })) "
        "end end")
        .arg(QCoreApplication::applicationName())
        .arg(QCoreApplication::applicationPid())
        .arg(width)
        .arg(height);
    QProcess::startDetached(m_hyprctl, { QStringLiteral("eval"), lua });
}
