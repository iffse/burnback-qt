#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif

#include <chrono>
#include <cmath>
#include <thread>
#include <QFile>
#include <vector>

#include <src/headers/interface.h>
#include "src/headers/iosystem.h"
#include <src/headers/globals.h>
#include <src/headers/connectivityMatrix.h>
#include <src/headers/operations.h>
#include <src/headers/iterations.h>
#include <src/headers/plotData.h>

#ifdef DEBUG
#include <fenv.h>
#endif

using namespace std;

Actions::Actions(QObject *parent) : QObject(parent) {
	connect(this, &Actions::newOutput, this, &Actions::appendOutput);
	connect(this, &Actions::readFinished, this, &Actions::afterReadMesh);
}

void clearSubstring(QString &str) {
#ifdef _WIN32
	const QString substring = "file:///";
#else
	const QString substring = "file://";
#endif

	if (str.startsWith(substring)) {
		str.remove(0, substring.length());
	}
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
		for (int line = 0; line < maxLines; ++ line) {
			indexRemove = setText.lastIndexOf("\n", indexRemove - 2);
		}
	} else {
		for (int line = 0; line < lines - maxLines; ++ line) {
			indexRemove = setText.indexOf("\n", indexRemove + 2);
		}
	}

	setText.remove(0, indexRemove);
	output->setProperty("text", setText);
}

void Actions::readMeshWorker(QString filepath) {
	numNodes = 0;
	numTriangles = 0;
	numEdges = 0;
	numTriangleEdge = 0;
	meshData.clear();
	connectivityMatrixBoundaryConditions.clear();
	uBoundaryData.clear();
	boundaryDescriptions.clear();

	const QString legacyExtension = ".dat";
	const QString jsonExtension = ".json";

	if (filepath.endsWith(legacyExtension)) {
		QFile file(filepath);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			emit newOutput("Error: Cannot open file");
			emit readFinished(false);
			return;
		}

		QTextStream in(&file);
		try {
			Reader::Legacy::readMesh(in);
			emit readFinished(true);
		} catch (std::invalid_argument &e) {
			emit newOutput("Error: " + QString(e.what()));
			emit readFinished(false);
			file.close(); return;
		} catch (...) {
			emit newOutput("Error: Unknown exception");
			emit readFinished(false);
			file.close(); return;
		}
		file.close();

	} else if (filepath.endsWith(jsonExtension)) {
		try {
			Reader::Json::readMesh(filepath);
		} catch (std::invalid_argument &e) {
			emit newOutput("Error: " + QString(e.what()));
			emit readFinished(false);
			return;
		} catch (...) {
			emit newOutput("Error: Unknown exception when reading mesh");
			emit readFinished(false);
			return;
		}
	} else {
		emit newOutput("Error: Unknown file extension");
		emit readFinished(false);
		return;
	}
	emit readFinished(true);
}

void Actions::readMesh(QString filepath) {
	running = false;
	root->findChild<QObject*>("runButton")->setProperty("enabled", false);
	appendOutput("--> Reading mesh");
	if (filepath.isEmpty()) {
		appendOutput("Error: No file selected");
		return;
	}

	clearSubstring(filepath);

	std::thread thread(&Actions::readMeshWorker, this, filepath);
	thread.detach();
}


void Actions::afterReadMesh(bool sucess) {
	if (!sucess) {
		appendOutput("Error: Failed to read mesh");
		return;
	}

	root->findChild<QObject*>("runButton")->setProperty("enabled", true);
	appendOutput("--> Mesh read sucessfully");
}

void Actions::run() {
	running = true;
	root->findChild<QObject*>("runButton")->setProperty("text", "Stop");
	// clear data
	appendOutput("--> Reading inputs");
	try {
		Reader::readInput();
	} catch (std::invalid_argument &e) {
		appendOutput("Error: " + QString(e.what()));
		root->findChild<QObject*>("runButton")->setProperty("text", "Run");
		return;
	} catch (...) {
		appendOutput("Error: Unknown exception");
		root->findChild<QObject*>("runButton")->setProperty("text", "Run");
		return;
	}

	std::thread thread(&Actions::worker, this);
	thread.detach();
}

void Actions::stop() {
	running = false;
}

void Actions::worker() {
	#ifdef DEBUG
	std::feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
	#endif
	if (!resume) {
		emit newOutput("--> Creating connectivity matrix");
		ConnectivityMatrix::NodeTriangles();
		ConnectivityMatrix::TriangleEdge();
		ConnectivityMatrix::NodeEdge();

		emit newOutput("--> Creating vertex-edge matrix");
		ConnectivityMatrix::createVertexEdge();

		emit newOutput("--> Reordering");
		ConnectivityMatrix::reorder();
		regenerateMeshData();

		emit newOutput("--> Recreating connectivity matrix");
		ConnectivityMatrix::NodeTriangles();
		ConnectivityMatrix::TriangleEdge();
		ConnectivityMatrix::NodeEdge();

		emit newOutput("--> Creating boundary matrix");
		setBoundaryConditions();
		emit newOutput("--> Creating angles matrix");
		setAngles();
		emit newOutput("--> Creating metric matrix");
		setMetric();
		emit newOutput("--> Creating boundary matrix");
		setBoundary();

		uVertex = vector<double>(numNodes, uInit);

		duVertex.fill(vector<double>(numNodes));
		duVariable.fill(vector<double>(numTriangles));
		maxDuEdge.fill(vector<double>(numNodes));
		flux.fill(vector<double>(numNodes));
		eps = vector<double>(numNodes);
		currentIter = 0;
		errorIter.clear();
	}
	emit newOutput("--> Starting subiteration loop");
	errorIter.resize(maxIter);

	double error = tolerance + 1;
	QString linesToPrint = "";
	auto clock = std::chrono::system_clock::now();
	timeTotal = 0;

	while (currentIter < minIter || (currentIter < maxIter && error > tolerance)) {
		++currentIter;
		Iterations::subIteration();
		errorIter[currentIter - 1] = getError();
		error = errorIter[currentIter - 1];

		if (linesToPrint != "")
			linesToPrint += "\n";
		linesToPrint += "Iteration: " + QString::number(currentIter) + " Time: " + QString::number(timeTotal) + " Error: " + QString::number(error * 100) + "%";

		auto now = std::chrono::system_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - clock).count() > 10) {
			clock = now;
			emit newOutput(linesToPrint);
			emit updateProgress(currentIter, maxIter);
			linesToPrint = "";
			if (!running) {
				emit newOutput("--> Stopped");
				root->findChild<QObject*>("runButton")->setProperty("text", "Run");
				setBurningArea();
				afterWorker();
				return;
		}
		}
	}

	if (linesToPrint != "") {
		emit newOutput(linesToPrint);
		emit updateProgress(currentIter, maxIter);
	}

	emit newOutput("--> Subiteration ended");

	setBurningArea();
	auto [areaGeometric, areaMDF] = Iterations::mainLoop();

	emit newOutput("--> Main loop ended");
	emit newOutput("Geometry area: " + QString::number(areaGeometric));
	emit newOutput("Burn area using minimum distance function: " + QString::number(areaMDF));
	emit newOutput("Error with respect to minimum distance function with recession speed 1: " + QString::number(100 * abs(areaGeometric - areaMDF) / areaGeometric) + "%");

	afterWorker();
}

void Actions::afterWorker() {
	root->findChild<QObject*>("runButton")->setProperty("text", "Run");
	double &burningWayMax = *max_element(burningWay.begin(), burningWay.end());
	double &burningAreaMax = *max_element(burningArea.begin(), burningArea.end());

	emit graphBurningArea(plotData::burningAreaData(), burningWayMax, burningAreaMax);
	emit graphErrorIter(errorIter, *max_element(errorIter.begin(), errorIter.end()));
	drawIsocontourLines(isocontourSize, numIsocontourLines);
}

void Actions::exportData(QString filepath, bool pretty) {
	auto origin = root->findChild<QObject*>("fileDialog")->property("fileUrl").toString();

	clearSubstring(filepath);
	clearSubstring(origin);

	// add .json if filepath doesn't have it
	if (!filepath.endsWith(".json"))
		filepath += ".json";

	appendOutput("Exporting data to " + filepath);
	try {
		Writer::Json::writeData(filepath, origin, pretty);
	} catch (const std::exception &e) {
		appendOutput("Error while exporting data: " + QString(e.what()));
	} catch (...) {
		appendOutput("Error while exporting data");
	}
}

vector<QString> Actions::getBoundaries() {
	auto numBoundaries = uBoundaryData.size();
	vector<QString> boundaries = vector<QString>(numBoundaries * 4);

	auto index = 0;
	for (const auto &[key, value]: uBoundaryData) {
		boundaries[index] = QString::number(key);
		boundaries[index + 1] = QString::number(connectivityMatrixBoundaryConditions[key] - 1);
		try {
			switch (connectivityMatrixBoundaryConditions[key]){
				case 1:
					boundaries[index + 2] = QString::number(value);
					break;
				case 2:
					boundaries[index + 2] = "";
					break;
				case 3:
					boundaries[index + 2] = QString::number(value * 180 / M_PI);
					break;
			}
		} catch (...) {
			boundaries[index + 2] = "";
		}
		try {
			boundaries[index + 3] = boundaryDescriptions[key];
		} catch (...) {
			boundaries[index + 3] = "";
		}
		index += 4;
	}
	return boundaries;
}

void Actions::updateBoundaries(bool saveToFile, bool pretty) {
	for (const auto &[key, _]: uBoundaryData) {
		auto type = root->findChild<QObject*>("boundaryComboBox" + QString::number(key))->property("currentIndex").toInt() + 1;
		auto value = root->findChild<QObject*>("boundaryValue" + QString::number(key))->property("text").toDouble();
		if (type == 3)
			value *= M_PI / 180;
		auto description = root->findChild<QObject*>("boundaryDescription" + QString::number(key))->property("text").toString();

		boundaryDescriptions[key] = description;
		uBoundaryData[key] = value;
		connectivityMatrixBoundaryConditions[key] = type;
	}

	appendOutput("Boundaries updated");

	if (saveToFile){
		auto filepath = root->findChild<QObject*>("fileDialog")->property("fileUrl").toString();
		clearSubstring(filepath);

		try {
			Writer::Json::updateBoundaries(filepath, pretty);
			appendOutput("Updated to " + filepath);
		} catch (const std::exception &e) {
			appendOutput("Error while updating boundaries: " + QString(e.what()));
		} catch (...) {
			appendOutput("Error while updating boundaries");
		}
	}
}

void Actions::contourDataPreviewGenerate(int width) {
	QObject *canvas = root->findChild<QObject*>("previewCanvas");

	auto &xmin = *min_element(x.begin(), x.end());
	auto &xmax = *max_element(x.begin(), x.end());
	auto &ymin = *min_element(y.begin(), y.end());
	auto &ymax = *max_element(y.begin(), y.end());
	width -= 5;
	auto scale = width/(xmax - xmin);
	auto height = int((ymax - ymin) * scale);
	height -= 5;
	canvas->setProperty("height", height + 10);

	auto shiftX = int(width/2.0 - scale * ((xmax - xmin)/2 + xmin)) + 2;
	auto shiftY = int(height/2.0 - scale * ((ymax - ymin)/2 + ymin)) + 3;

	plotData::generateContourDataDict(shiftX, shiftY, scale);
	return;
}

QString Actions::getRecession() {
	QString output = "";
	for (const auto &value: recession) {
		output +=  QString::number(value) + "\n";
	}
	output.chop(1);
	return output;
}
QString Actions::getRecession(QString filepath) {
	QString output = "";
	clearSubstring(filepath);
	QFile file(filepath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		appendOutput("Error while opening file " + filepath);
		return "";
	}
	QTextStream in(&file);
	while (!in.atEnd()) {
		output += in.readLine() + "\n";
	}
	file.close();
	output.chop(1);
	return output;
}

void Actions::drawIsocontourLines(uint maxSize, uint numLines) {
	if (x.empty() || y.empty() || uVertex.empty()) {
		emit newOutput("No data to draw");
		return;
	}

	try {
		++numLines;
		auto &xmin = *min_element(x.begin(), x.end());
		auto &xmax = *max_element(x.begin(), x.end());
		auto &ymin = *min_element(y.begin(), y.end());
		auto &ymax = *max_element(y.begin(), y.end());

		auto scale = maxSize/max(xmax - xmin, ymax - ymin);

		emit clearCanvas();
		auto canvasHeight = uint(1.5 * (ymax - ymin) * scale);
		auto canvasWidth = uint(1.5 * (xmax - xmin) * scale);
		auto shiftX = int(canvasWidth / 2.0 - scale * ((xmax - xmin) / 2 + xmin));
		auto shiftY = int(canvasHeight / 2.0 - scale * ((ymax - ymin) / 2 + ymin));

		emit setCanvasSize(canvasWidth, canvasHeight);

		double &uVertexMax = *max_element(uVertex.begin(), uVertex.end());
		double &uVertexMin = *min_element(uVertex.begin(), uVertex.end());

		emit paintCanvas(plotData::contourData(shiftX, shiftY, scale), "#000000");

		auto step = (uVertexMax - uVertexMin)/numLines;

		auto value = step;
		for (uint line = 1; line < numLines; ++line) {

			auto pickColor = [](double region) {
				if (region < 0.25) {
					return QString("rgb(%1, %2, %3)").arg(255).arg(255*region*4).arg(0);
				} else if (region < 0.5) {
					return QString("rgb(%1, %2, %3)").arg(255*(1-(region-0.25)*4)).arg(255).arg(0);
				} else if (region < 0.75) {
					return QString("rgb(%1, %2, %3)").arg(0).arg(255).arg(255*(region-0.5)*4);
				} else {
					return QString("rgb(%1, %2, %3)").arg(0).arg(255*(1-(region-0.75)*4)).arg(255);
				}
			};

			// paintCanvas(plotData::isocolourData(value), color);
			emit paintCanvas(plotData::isocolourData(value, shiftX, shiftY, scale), pickColor(double(line-1)/(numLines-1)));
			value += step;

		}
	} catch (const std::exception &e) {
		emit newOutput("Error while drawing isocontour lines: " + QString(e.what()));
	} catch (...) {
		emit newOutput("Error while drawing isocontour lines");
	}
	return;
}

void Actions::redrawIsocontourLines(uint maxSize, uint numLines) {
	std::thread thread(&Actions::drawIsocontourLines, this, maxSize, numLines);
	thread.detach();
}

