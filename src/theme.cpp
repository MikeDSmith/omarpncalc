#include "theme.h"

#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

namespace {
QString currentDir() {
    return QDir::homePath() + QStringLiteral("/.local/state/omarchy/current");
}
QString colorsPath() {
    return currentDir() + QStringLiteral("/theme/colors.toml");
}
}

Theme::Theme(QObject *parent) : QObject(parent) {
    // A theme switch repoints the current/theme symlink, so watch the
    // directories around the file as well as the file itself, and re-arm
    // after every change since the watched inode may have been replaced.
    const auto changed = [this](const QString &) {
        reload();
        watch();
    };
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, changed);
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, changed);
    reload();
    watch();
}

bool Theme::load(const QString &toml) {
    static const QRegularExpression kv(
        QStringLiteral("^\\s*([A-Za-z0-9_]+)\\s*=\\s*[\"']([^\"']+)[\"']"));
    bool any = false;
    const QStringList lines = toml.split(QLatin1Char('\n'));
    for (const QString &line : lines) {
        const QRegularExpressionMatch m = kv.match(line);
        if (!m.hasMatch())
            continue;
        const QString key = m.captured(1);
        const QColor color(m.captured(2));
        if (!color.isValid())
            continue;
        if (key == QStringLiteral("background")) m_background = color;
        else if (key == QStringLiteral("foreground")) m_foreground = color;
        else if (key == QStringLiteral("accent")) m_accent = color;
        else if (key == QStringLiteral("red")) m_urgent = color;
        else continue;
        any = true;
    }
    return any;
}

void Theme::reload() {
    QFile file(colorsPath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    if (load(QTextStream(&file).readAll()))
        emit changed();
}

void Theme::watch() {
    const QStringList watched = m_watcher.files() + m_watcher.directories();
    if (!watched.isEmpty())
        m_watcher.removePaths(watched);

    const QString themeDir = currentDir() + QStringLiteral("/theme");
    if (QDir(currentDir()).exists())
        m_watcher.addPath(currentDir());
    if (QDir(themeDir).exists())
        m_watcher.addPath(themeDir);
    if (QFile::exists(colorsPath()))
        m_watcher.addPath(colorsPath());
}
