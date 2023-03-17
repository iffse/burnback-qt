#include <chrono>
#include <thread>
#include <QFile>
#include <vector>

#include <src/headers/interface.h>
#include <src/headers/filesystem.h>
#include <src/headers/globals.h>
#include <src/headers/coeficcientMatrix.h>
#include <src/headers/operations.h>
#include <src/headers/iterations.h>
#include <src/headers/plotData.h>

#include <fenv.h>

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

#ifdef _WIN32
	const QString substring = "file:///";
#else
	const QString substring = "file://";
#endif

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
	std::thread thread(&Actions::worker, this);
	thread.detach();
}

void Actions::afterWorker() {
	// plotData::generateData();
	auto numLines = numIsocontourLines;
	++numLines;

	double &burningWayMax = *max_element(burningWay.begin(), burningWay.end());
	double &burningAreaMax = *max_element(burningArea.begin(), burningArea.end());

	graphBurningArea(plotData::burningAreaData(), burningWayMax, burningAreaMax);

	double &uVertexMax = *max_element(uVertex.begin(), uVertex.end());
	double &uVertexMin = *min_element(uVertex.begin(), uVertex.end());

	clearCanvas();

	paintCanvas(plotData::contourData(), "#000000");

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
		paintCanvas(plotData::isocolourData(value), pickColor(double(line-1)/(numLines-1)));
		value += step;

	}
}

void Actions::worker() {
	feenableexcept(FE_INVALID | FE_OVERFLOW);


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
	emit newOutput("--> Creating alpha matrix");
	setAlpha();
	emit newOutput("--> Creating metric matrix");
	setMetric();

	emit newOutput("--> Starting subiteration loop");
	uVertex = vector<double>(numNodes, uInit[0]);

	duVertex.fill(vector<double>(numNodes));
	duVariable.fill(vector<double>(numTriangles));
	flux.fill(vector<double>(numNodes));
	eps = vector<double>(numNodes);
	dt = vector<double>(numNodes);
	double error = tolerance + 1;

	uint numItereations = 0;
	QString linesToPrint = "";
	auto clock = std::chrono::system_clock::now();

	while (numItereations < minIter || (numItereations < maxIter && error > tolerance)) {
		Iterations::subIteration();
		error = getError();

		if (linesToPrint != "")
			linesToPrint += "\n";
		linesToPrint += "Iteration: " + QString::number(numItereations + 1) + " Time: " + QString::number(timeTotal) + " Error: " + QString::number(error * 100) + "%";

		auto now = std::chrono::system_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - clock).count() > 10) {
			clock = now;
			emit newOutput(linesToPrint);
			linesToPrint = "";
		}
		++numItereations;
	}

	if (linesToPrint != "") {
		emit newOutput(linesToPrint);
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
