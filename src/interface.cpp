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
	std::thread thread(&Actions::worker, this);
	thread.detach();
}

void Actions::afterWorker() {
	plotData::generateData();
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

	emit newOutput("--> Counting number of boundaries");
	numBoundaries = 0;
	for (uint edge = 0; edge < numEdges; ++edge) {
		int triangle1 = connectivityMatrixTriangleEdge[1 - 1][edge];
		int triangle2 = connectivityMatrixTriangleEdge[2 - 1][edge];
		if (triangle1 * triangle2  < 0)
			++numBoundaries;
	}

	emit newOutput("--> Creating boundary matrix");
	setBoundaryConditions();
	emit newOutput("--> Creating alpha matrix");
	setAlpha();
	emit newOutput("--> Creating metric matrix");
	setMetric();

	emit newOutput("--> Starting subiteration loop");
	uVertex = vector<double>(numNodes);
	for (uint node = 0; node < numNodes; ++node)
		uVertex[node] = uInit[0];

	duVertex.fill(vector<double>(numNodes));
	duVariable.fill(vector<double>(numNodes));
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

	// 	auto now = std::chrono::system_clock::now();
	// 	if (std::chrono::duration_cast<std::chrono::milliseconds>(now - clock).count() > 10) {
	// 	}
	// }
}
