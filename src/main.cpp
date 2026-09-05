#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlError>
#include <QUrl>

#include "calculator.h"
#include "theme.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("omarpn"));
    // The Wayland app_id, so window rules can match the class "omarpn".
    app.setDesktopFileName(QStringLiteral("omarpn"));
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("omarpn")));

    Calculator calculator(&app);
    Theme theme(&app);

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::warnings, &app,
                     [](const QList<QQmlError> &warnings) {
        for (const QQmlError &warning : warnings)
            qWarning().noquote() << warning.toString();
    });
    engine.rootContext()->setContextProperty(QStringLiteral("calc"), &calculator);
    engine.rootContext()->setContextProperty(QStringLiteral("theme"), &theme);

    engine.load(QUrl(QStringLiteral("qrc:/Main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
