#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QFontDatabase>

#include <src/headers/interface.h>
#include <src/headers/globals.h>

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
	QGuiApplication app(argc, argv);

	QQmlApplicationEngine engine;
	const QUrl url(QStringLiteral("qrc:/main.qml"));
	QObject::connect(
			&engine, &QQmlApplicationEngine::objectCreated,
			&app, [url](QObject *obj, const QUrl &objUrl) {
				if (!obj && url == objUrl)
					QCoreApplication::exit(-1);
			}, Qt::QueuedConnection);
	engine.load(url);

	// Get the root object in order to access children properties
	root = engine.rootObjects().at(0);

	// Use monospaced font for output
	const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
	QObject *output = root->findChild<QObject*>("output");
	output->setProperty("font", fixedFont);

	// Set the context property to access the class from QML
	QQmlContext *rootContext = engine.rootContext();

	Actions actions;
	rootContext->setContextProperty("actions", &actions);

	return app.exec();
}
