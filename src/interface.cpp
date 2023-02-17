#include <QDebug>
#include <QFile>
#include <chrono>
#include <thread>

#include <src/headers/interface.h>
#include <src/headers/reader.h>
#include <src/headers/globals.h>

Actions::Actions(QObject *parent) : QObject(parent)
{
	connect(this, &Actions::newOutput, this, &Actions::appendOutput);
}

void Actions::appendOutput(QString text) {
	QObject *output = root->findChild<QObject*>("output");
	auto currentText = output->property("text").toString();
	auto setText = currentText + text + "\n";

	// limit the number of lines
	auto maxLines = 100;
	auto lines = setText.count("\n");
	if (lines < maxLines) {
		output->setProperty("text", setText);
		return;
	}

	auto indexRemove = 0;
	if (lines > 2 * maxLines) {
		for (int i = 0; i < maxLines; ++ i) {
			indexRemove = setText.lastIndexOf("\n", indexRemove - 2);
		}
	} else {
		for (int i = 0; i < lines - maxLines; ++ i) {
			indexRemove = setText.indexOf("\n", indexRemove + 2);
		}
	}

	setText.remove(0, indexRemove);
	output->setProperty("text", setText);
}

void Actions::run()
{
	appendOutput("--> Reading inputs");
	if (!Reader::readInput()) {
		emit newOutput("Error when reading inputs");
		return;
	}
	std::thread thread(&Actions::worker, this);
	thread.detach();
}

void Actions::afterWorker() {
}

void Actions::worker() {
	QObject *output = root->findChild<QObject*>("fileDialog");
	auto filePath = output->property("fileUrl").toString();
	if (filePath.isEmpty()) {
		emit newOutput("Error: No file selected");
		return;
	}
	QString substring = "file://";
	if (filePath.startsWith(substring))
		filePath.remove(0, substring.length());

	emit newOutput("--> Reading mesh");
	if (!Reader::readMesh(filePath)) {
		emit newOutput("Error: Could not open mesh file");
		emit finished();
		return;
	}

	// auto clock = std::chrono::system_clock::now();
	// 	auto now = std::chrono::system_clock::now();
	// 	if (std::chrono::duration_cast<std::chrono::milliseconds>(now - clock).count() > 10) {
	// 	}
	// }
}

