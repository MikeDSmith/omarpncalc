#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlError>
#include <QUrl>

#include "calculator.h"
#include "hyprland.h"
#include "textscale.h"
#include "theme.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("omarpncalc"));
    // The Wayland app_id, so window rules can match the class "omarpncalc".
    app.setDesktopFileName(QStringLiteral("omarpncalc"));
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("omarpncalc")));

    Calculator calculator(&app);
    Theme theme(&app);
    TextScale textScale(&app);
    Hyprland hyprland(&app);

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::warnings, &app,
                     [](const QList<QQmlError> &warnings) {
        for (const QQmlError &warning : warnings)
            qWarning().noquote() << warning.toString();
    });
    engine.rootContext()->setContextProperty(QStringLiteral("calc"), &calculator);
    engine.rootContext()->setContextProperty(QStringLiteral("theme"), &theme);
    engine.rootContext()->setContextProperty(QStringLiteral("textScale"), &textScale);
    engine.rootContext()->setContextProperty(QStringLiteral("hyprland"), &hyprland);

    engine.load(QUrl(QStringLiteral("qrc:/Main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
