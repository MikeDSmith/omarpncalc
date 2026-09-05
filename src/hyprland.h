#pragma once

#include <QObject>
#include <QSize>
#include <QString>

// Hyprland ignores a mapped floating window's own resize (the surface just
// gets clipped), so size changes after mapping go through its IPC instead.
class Hyprland : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool available READ available CONSTANT)

public:
    explicit Hyprland(QObject *parent = nullptr);

    bool available() const { return !m_hyprctl.isEmpty(); }

    // Resize this process's window (matched by app id and pid) and re-centre
    // it in the work area.
    Q_INVOKABLE void resizeWindow(int width, int height) const;

    // Usable area of the focused monitor in logical pixels, bars excluded.
    // Empty when the compositor cannot be asked.
    Q_INVOKABLE QSize workArea() const;

private:
    // Run a dispatcher against this process's own window. `w` names it.
    void dispatchForOwnWindow(const QString &dispatcher) const;

    QString m_hyprctl;
};
