#include <chrono>
#include <thread>
#include <QFile>
#include <vector>

#include <src/headers/interface.h>
#include "src/headers/iosystem.h"
#include <src/headers/globals.h>
#include <src/headers/coeficcientMatrix.h>
#include <src/headers/operations.h>
#include <src/headers/iterations.h>
#include <src/headers/plotData.h>

#ifdef DEBUG
#include <fenv.h>
#endif

using namespace std;

Actions::Actions(QObject *parent) : QObject(parent) {
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

void Actions::run() {
	// clear data
	numNodes = 0;
	numTriangles = 0;
	numEdges = 0;
	numTriangleEdge = 0;
	meshData.clear();
	connectivityMatrixBoundaryConditions.clear();
	uBoundaryData.clear();

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

#ifdef _WIN32
	const QString substring = "file:///";
#else
	const QString substring = "file://";
#endif

	if (filepath.startsWith(substring)) {
		filepath.remove(0, substring.length());
	}

	const QString legacyExtension = ".dat";
	const QString jsonExtension = ".json";

	if (filepath.endsWith(legacyExtension)) {
		QFile file(filepath);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			appendOutput("Error: Cannot open file");
			return;
		}

		QTextStream in(&file);
		try {
			Reader::Legacy::readMesh(in);
		} catch (std::invalid_argument &e) {
			appendOutput("Error: " + QString(e.what()));
			file.close(); return;
		} catch (...) {
			appendOutput("Error: Unknown exception");
			file.close(); return;
		}
		file.close();

	} else if (filepath.endsWith(jsonExtension)) {
		try {
			Reader::Json::readMesh(filepath);
		} catch (std::invalid_argument &e) {
			appendOutput("Error: " + QString(e.what()));
			return;
		} catch (...) {
			appendOutput("Error: Unknown exception when reading mesh");
			return;
		}
		} else {
		appendOutput("Error: Unknown file extension");
		return;
	}

	std::thread thread(&Actions::worker, this);
	thread.detach();
}

void Actions::afterWorker() {
	// plotData::generateData();
	auto numLines = numIsocontourLines;
	++numLines;

	auto &xmin = *min_element(x.begin(), x.end());
	auto &xmax = *max_element(x.begin(), x.end());
	auto &ymin = *min_element(y.begin(), y.end());
	auto &ymax = *max_element(y.begin(), y.end());

	auto scale = isocontourSize/max(xmax - xmin, ymax - ymin);

	auto canvasHeight = uint(1.5 * (ymax - ymin) * scale);
	auto canvasWidth = uint(1.5 * (xmax - xmin) * scale);
    auto shiftX = int(-xmin * scale + (canvasWidth - (xmax - xmin) * scale) / 2);
    auto shiftY = int(-ymin * scale + (canvasHeight - (ymax - ymin) * scale) / 2);

    emit setCanvasSize(canvasWidth, canvasHeight);

	double &burningWayMax = *max_element(burningWay.begin(), burningWay.end());
	double &burningAreaMax = *max_element(burningArea.begin(), burningArea.end());

    emit graphBurningArea(plotData::burningAreaData(), burningWayMax, burningAreaMax);

	double &uVertexMax = *max_element(uVertex.begin(), uVertex.end());
	double &uVertexMin = *min_element(uVertex.begin(), uVertex.end());

    emit clearCanvas();

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
}

void Actions::worker() {
	#ifdef DEBUG
	std::feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
	#endif
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

	emit newOutput("--> Starting subiteration loop");
	uVertex = vector<double>(numNodes, uInit);

	duVertex.fill(vector<double>(numNodes));
	duVariable.fill(vector<double>(numTriangles));
	maxDuEdge.fill(vector<double>(numNodes));
	flux.fill(vector<double>(numNodes));
	eps = vector<double>(numNodes);
	double error = tolerance + 1;

	uint numItereations = 0;
	QString linesToPrint = "";
	auto clock = std::chrono::system_clock::now();
	timeTotal = 0;

	while (numItereations < minIter || (numItereations < maxIter && error > tolerance)) {
		++numItereations;
		Iterations::subIteration();
		error = getError();

		if (linesToPrint != "")
			linesToPrint += "\n";
		linesToPrint += "Iteration: " + QString::number(numItereations) + " Time: " + QString::number(timeTotal) + " Error: " + QString::number(error * 100) + "%";

		auto now = std::chrono::system_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - clock).count() > 10) {
			clock = now;
			emit newOutput(linesToPrint);
			updateProgress(numItereations, maxIter);
			linesToPrint = "";
		}
	}

	if (linesToPrint != "") {
		emit newOutput(linesToPrint);
		updateProgress(numItereations, maxIter);
	}

	emit newOutput("--> Subiteration ended");

	setqbnd();
	setBurningArea();

	auto [areag, areap] = Iterations::mainLoop();

	emit newOutput("--> Main loop ended");
	emit newOutput("Area of the propellant: " + QString::number(areag));
	emit newOutput("Propellant burnt: " + QString::number(areap));
	emit newOutput("Error: " + QString::number(100 * abs(areag - areap) / areag) + "%");

	afterWorker();
}

void Actions::exportData(QString filepath, QString origin) {
#ifdef _WIN32
	const QString substring = "file:///";
#else
	const QString substring = "file://";
#endif

	auto cleanSubstring = [&substring](QString &str) {
		if (str.startsWith(substring)) {
			str.remove(0, substring.length());
		}
	};

	cleanSubstring(filepath);
	cleanSubstring(origin);

	// add .json if filepath doesn't have it
	if (!filepath.endsWith(".json"))
		filepath += ".json";

	appendOutput("Exporting data to " + filepath);
	try {
		Writer::Json::writeData(filepath, origin);
	} catch (const std::exception &e) {
		appendOutput("Error while exporting data: " + QString(e.what()));
	} catch (...) {
		appendOutput("Error while exporting data");
	}
}
