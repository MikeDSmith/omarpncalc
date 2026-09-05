#pragma once

#include <QObject>
#include <QString>

// Hyprland ignores a mapped floating window's own resize (the surface just
// gets clipped), so size changes after mapping go through its IPC instead.
class Hyprland : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool available READ available CONSTANT)

public:
    explicit Hyprland(QObject *parent = nullptr);

    bool available() const { return !m_hyprctl.isEmpty(); }

    // Resize this process's window (matched by app id and pid).
    Q_INVOKABLE void resizeWindow(int width, int height) const;

private:
    QString m_hyprctl;
};
