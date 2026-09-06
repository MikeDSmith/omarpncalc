#pragma once

#include <QColor>
#include <QFileSystemWatcher>
#include <QObject>

// Omarchy theme colors, read from the current theme's colors.toml and
// refreshed live when the theme changes.
class Theme : public QObject {
    Q_OBJECT
    Q_PROPERTY(QColor background READ background NOTIFY changed)
    Q_PROPERTY(QColor foreground READ foreground NOTIFY changed)
    Q_PROPERTY(QColor accent READ accent NOTIFY changed)
    Q_PROPERTY(QColor urgent READ urgent NOTIFY changed)

public:
    explicit Theme(QObject *parent = nullptr);

    QColor background() const { return m_background; }
    QColor foreground() const { return m_foreground; }
    QColor accent() const { return m_accent; }
    QColor urgent() const { return m_urgent; }

    // Parse colors.toml text; returns whether anything was picked up.
    bool load(const QString &toml);

signals:
    void changed();

private:
    void reload();
    void watch();

    QColor m_background = QStringLiteral("#101315");
    QColor m_foreground = QStringLiteral("#cacccc");
    QColor m_accent = QStringLiteral("#cacccc");
    QColor m_urgent = QStringLiteral("#a55555");
    QFileSystemWatcher m_watcher;
};
