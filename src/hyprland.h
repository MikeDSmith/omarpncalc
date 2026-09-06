#pragma once

#include <QObject>
#include <QSize>
#include <QString>

// A small window onto the compositor, for the one thing Wayland will not tell
// a client itself: how much of the screen is actually usable.
class Hyprland : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool available READ available CONSTANT)

public:
    explicit Hyprland(QObject *parent = nullptr);

    bool available() const { return !m_hyprctl.isEmpty(); }

    // Usable area of the focused monitor in logical pixels, bars excluded.
    // Empty when the compositor cannot be asked.
    Q_INVOKABLE QSize workArea() const;

private:
    QString m_hyprctl;
};
