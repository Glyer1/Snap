#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDir>
#include <QQmlContext>
#include "./utils/clipboardutils.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.addImportPath(QDir::currentPath() + "/.qt/qml");

    ClipboardUtils clipboradUtilsManager;
    engine.rootContext()->setContextProperty("clipboradUtilsManager", &clipboradUtilsManager);

    engine.loadFromModule("Snap", "Main");

    return app.exec();
}
