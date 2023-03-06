#pragma once

#include <QString>
#include <QDebug>

class Actions: public QObject
{
	Q_OBJECT
public:
	explicit Actions(QObject *parent = nullptr);

signals:
	void newOutput(QString output);
	void paintCanvas(std::vector<double> list);
	void finished();

public slots:
	void run();
	void appendOutput(QString output);
	void worker();
	void afterWorker();
};
