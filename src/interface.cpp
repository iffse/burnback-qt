#include <chrono>
#include <thread>
#include <QDebug>
#include <QFile>

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
	try {
		Reader::readInput();
	} catch (std::invalid_argument &e) {
		appendOutput("Error: " + QString(e.what()));
		return;
	} catch (...) {
		appendOutput("Error: Unknown exception");
		return;
	}

	appendOutput("--> Reading mesh");
	auto *fileDialog = root->findChild<QObject*>("fileDialog");
	auto filepath = fileDialog->property("fileUrl").toString();
	if (filepath.isEmpty()) {
		appendOutput("Error: No file selected");
		return;
	}

	const QString substring = "file://";
	if (filepath.startsWith(substring)) {
		filepath.remove(0, substring.length());
	}
	QFile file(filepath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		appendOutput("Error: Cannot open file");
		return;
	}

	QTextStream in(&file);

	try {
		Reader::readMesh(in);
	} catch (std::invalid_argument &e) {
		appendOutput("Error: " + QString(e.what()));
		file.close(); return;
	} catch (...) {
		appendOutput("Error: Unknown exception");
		file.close(); return;
	}

	file.close();
	// std::thread thread(&Actions::worker, this);
	// thread.detach();
}

void Actions::afterWorker() {
}

void Actions::worker() {
	// auto clock = std::chrono::system_clock::now();
	// 	auto now = std::chrono::system_clock::now();
	// 	if (std::chrono::duration_cast<std::chrono::milliseconds>(now - clock).count() > 10) {
	// 	}
	// }
}
