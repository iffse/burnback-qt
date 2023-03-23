#pragma once

#include <QString>
#include <QObject>
#include <QVariant>

class Actions: public QObject {
	Q_OBJECT
public:
	explicit Actions(QObject *parent = nullptr);

signals:
	void newOutput(QString output);
	void paintCanvas(std::vector<double> list, QString color = "#000000");
	void clearCanvas();
	void setCanvasSize(uint width, uint height);
	void graphBurningArea(QVariant points, double xMax, double yMax);
	void finished();

public slots:
	void run();
	void appendOutput(QString output);
	void worker();
	void afterWorker();
};
