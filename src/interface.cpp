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
	root->findChild<QObject*>("resume")->setProperty("enabled", false);
	root->findChild<QObject*>("resume")->setProperty("checked", false);
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

	root->findChild<QObject*>("resume")->setProperty("enabled", true);
	std::thread thread(&Actions::worker, this);
	thread.detach();
}

void Actions::stop() {
	running = false;
}

void Actions::worker() {
	#ifdef DEBUG
	feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
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
		if (anisotropic) {
			emit newOutput("--> Creating anisotropic matrix");
			calculateAnisotropicMatrix();
		}
		emit newOutput("--> Getting maximum recession");
		maxRecession = getMaxRecession();

		uVertex = vector<double>(numNodes, uInit);
		duVertex.fill(vector<double>(numNodes));
		duVariable.fill(vector<double>(numTriangles));
		maxDuEdge.fill(vector<double>(numNodes));
		flux.fill(vector<double>(numNodes));
		currentIter = 0;
		errorIter.clear();
	}
	emit newOutput("--> Starting subiteration loop");
	if (currentIter < targetIter)
		errorIter.resize(targetIter);

	QString linesToPrint = "";
	auto clock = std::chrono::system_clock::now();
	timeTotal = 0;

	for (; currentIter < targetIter; ++currentIter) {
		Iterations::subIteration();
		auto error = getError();
		errorIter[currentIter] = error;

		if (linesToPrint != "")
			linesToPrint += "\n";
		linesToPrint += "Iteration: " + QString::number(currentIter + 1) + " Time: " + QString::number(timeTotal) + " Error: " + QString::number(error * 100) + "%";

		if (error > 1) {
			if (linesToPrint != "") {
				emit newOutput(linesToPrint);
				emit updateProgress(currentIter + 1, targetIter);
			}
			emit newOutput("Error: Divergence detected. Stopping. Try reducing the CFL.");
			emit newOutput("--> Stopped");
			root->findChild<QObject*>("runButton")->setProperty("text", "Run");
			setBurningArea();
			afterWorker();
			return;
		}

		auto now = std::chrono::system_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - clock).count() > 10) {
			clock = now;
			emit newOutput(linesToPrint);
			emit updateProgress(currentIter + 1, targetIter);
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
		emit updateProgress(currentIter, targetIter);
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
	drawIsocontourLines(isocontourSize, numIsocontourLines);

	emit graphErrorIter(
		vector<double>(errorIter.begin(), errorIter.begin() + currentIter),
		*max_element(errorIter.begin(), errorIter.begin() + currentIter) * 1.05
	);

	if (numberArea == 0)
		emit graphBurningArea(QVariantList(), 1, 1);
	else {
		double &burningWayMax = *max_element(burningDepth.begin(), burningDepth.end());
		double &burningAreaMax = *max_element(burningArea.begin(), burningArea.end());
		emit graphBurningArea(plotData::burningAreaData(), burningWayMax, burningAreaMax * 1.05);
	}
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

void Actions::updateRecessions(QString recessions, bool save, bool pretty) {
	try {
		if (recessions == "") {
			recession = vector<double>(numNodes, 1);
			anisotropic = false;
			appendOutput("Recessions updated to 1");
			return;
		}
		auto recessionsList = recessions.split("\n");
		if (recessionsList[0].split(" ").size() == 3) {
			recessionAnisotropic = vector<array<double, 3>>(numNodes, {0, 0, 0});
			anisotropic = true;
			for (uint node = 0; node < numNodes; ++node) {
				auto values = recessionsList[node].split(" ");
				recessionAnisotropic[node][0] = values[0].toDouble();
				recessionAnisotropic[node][1] = values[1].toDouble();
				recessionAnisotropic[node][2] = values[2].toDouble();
			}
			appendOutput("Recessions updated to anisotropic");
			return;
		} else if (recessionsList[0].split(" ").size() == 1) {
			recession = vector<double>(numNodes);
			anisotropic = false;
			for (uint node = 0; node < numNodes; ++node) {
				recession[node] = recessionsList[node].toDouble();
			}
			appendOutput("Recessions updated to isotropic");
			return;
		} else {
			appendOutput("Error while updating recessions: wrong format");
			return;
		}
	} catch (...) {
		appendOutput("Error while updating recessions");
	}

	try {
		if (save){
			auto filepath = root->findChild<QObject*>("fileDialog")->property("fileUrl").toString();
			clearSubstring(filepath);

			try {
				Writer::Json::updateRecessions(filepath, pretty);
				appendOutput("Updated to " + filepath);
			} catch (const std::exception &e) {
				appendOutput("Error while updating boundaries: " + QString(e.what()));
			} catch (...) {
				appendOutput("Error while updating boundaries");
			}
		}
	}catch(...) {
		
	}
}

QString Actions::getRecession() {
	QString output = "";
	if (anisotropic) {
		for (const auto &value: recessionAnisotropic) {
			for (const auto &value2: value)
				output +=  QString::number(value2) + " ";
			output += "\n";
		}
		output.chop(1);
		return output;
	}
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
				switch (isocontourColor) {
					case 0: { // 5 colors
						auto hue = (1 - region) * 240;
						return QString("hsl(") + QString::number(hue) + ", 100%, 50%)";
					}
					case 1: { // 2 colors
						auto hue = 240 + 120 * region;
						return QString("hsl(") + QString::number(hue) + ", 100%, 50%)";
					}
					case 2: { // black and white
						auto lightness = 100 * region;
						return QString("hsl(0, 0%, ") + QString::number(lightness) + "%)";
					}
					default:
						return QString("#000000");
				}
			};

			// paintCanvas(plotData::isocolourData(value), color);
			emit paintCanvas(plotData::isocolourData(value, shiftX, shiftY, scale), pickColor(double(line-1)/(numLines-1)));
			value += step;
		}
		emit newOutput("Isocontour lines drawn with a step of " + QString::number(step) + " from " + QString::number(uVertexMin) + " to " + QString::number(uVertexMax));
	} catch (const std::exception &e) {
		emit newOutput("Error while drawing isocontour lines: " + QString(e.what()));
	} catch (...) {
		emit newOutput("Error while drawing isocontour lines");
	}
	return;
}

void Actions::redrawIsocontourLines(uint maxSize, uint numLines, uint colorIndex) {
	isocontourColor = colorIndex;
	std::thread thread(&Actions::drawIsocontourLines, this, maxSize, numLines);
	thread.detach();
}

