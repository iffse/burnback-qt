#include <QDebug>
#include <chrono>
#include <thread>

#include <src/headers/interface.h>
#include <src/headers/globals.h>

Actions::Actions(QObject *parent) : QObject(parent)
{
	connect(this, &Actions::newOutput, this, &Actions::appendOutput);
}

void Actions::appendOutput(QString text) {
	QObject *output = root->findChild<QObject*>("output");
	auto currentText = output->property("text").toString();
	currentText.append(text);
	output->setProperty("text", currentText);
}


void Actions::worker() {
	for (int i = 0; i < 100; ++i) {
		emit newOutput(QString::number(i) + "\n");
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void Actions::run()
{
	std::thread t1(&Actions::worker, this);
	t1.detach();
}

