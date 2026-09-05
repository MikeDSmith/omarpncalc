#pragma once

#include <QObject>

class QDBusVariant;

// The desktop's apparent-text-size knob: GNOME's text-scaling-factor, which
// `omarchy display text size` drives (12px == 1.0). Read through the
// xdg-desktop-portal Settings interface so it follows changes live.
class TextScale : public QObject {
    Q_OBJECT
    Q_PROPERTY(qreal factor READ factor NOTIFY factorChanged)

public:
    explicit TextScale(QObject *parent = nullptr);

    qreal factor() const { return m_factor; }

signals:
    void factorChanged(qreal factor);

private slots:
    void handlePortalSettingChanged(const QString &nameSpace, const QString &key,
                                    const QDBusVariant &value);

private:
    void request();
    void setFactor(qreal factor);

    qreal m_factor = 1.0;
};
