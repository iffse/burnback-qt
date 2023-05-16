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
	void graphErrorIter(std::vector<double> points, double errorMax);
	void updateProgress(uint progress, uint total);
	void finished();
	void readFinished(bool success);

public slots:
	void readMesh(QString filepath);
	void readMeshWorker(QString filepath);
	void afterReadMesh(bool success);
	void run();
	void stop();
	void appendOutput(QString text);
	void worker();
	void afterWorker();
	void exportData(QString filepath, bool pretty);
	std::vector<QString> getBoundaries();
	void updateBoundaries(bool saveToFile, bool pretty);
	void contourDataPreviewGenerate(int width);
	QString getRecession();
	QString getRecession(QString filepath);
	void drawIsocontourLines(uint maxSize, uint numLines);
	void redrawIsocontourLines(uint maxSize, uint numLines);
};
