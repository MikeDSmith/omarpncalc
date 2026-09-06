#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlError>
#include <QTimer>
#include <QUrl>

#include "calculator.h"
#include "hyprland.h"
#include "theme.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("omarpncalc"));
    // The Wayland app_id, so window rules can match the class "omarpncalc".
    app.setDesktopFileName(QStringLiteral("omarpncalc"));
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("omarpncalc")));

    Calculator calculator(&app);
    Theme theme(&app);
    Hyprland hyprland(&app);

    // --self-test loads the interface, spins the event loop briefly and exits
    // non-zero if QML complained. Nothing else catches a QML slip: the
    // compiler never sees this file and the engine tests never load it.
    const bool selfTest = app.arguments().contains(QStringLiteral("--self-test"));
    bool complained = false;

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::warnings, &app,
                     [&complained](const QList<QQmlError> &warnings) {
        for (const QQmlError &warning : warnings)
            qWarning().noquote() << warning.toString();
        if (!warnings.isEmpty())
            complained = true;
    });
    engine.rootContext()->setContextProperty(QStringLiteral("calc"), &calculator);
    engine.rootContext()->setContextProperty(QStringLiteral("theme"), &theme);
    engine.rootContext()->setContextProperty(QStringLiteral("hyprland"), &hyprland);

    engine.load(QUrl(QStringLiteral("qrc:/Main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    if (selfTest) {
        QTimer::singleShot(1500, &app, &QGuiApplication::quit);
        app.exec();
        return complained ? 1 : 0;
    }

    return app.exec();
}
