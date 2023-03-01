// #include <QRegularExpression>

#include <src/headers/filesystem.h>
#include <src/headers/globals.h>
#include <src/headers/operations.h>
#include <src/headers/interface.h>

using namespace std;

void Reader::readInput() {//{{{

	numberArea = root->findChild<QObject*>("areas")->property("text").toInt();
	if (numberArea < 1)
		throw std::invalid_argument("Number of areas must be greater than 0");

	axisymmetric = root->findChild<QObject*>("axisymmetric")->property("checked").toBool();
	resume = root->findChild<QObject*>("resume")->property("checked").toBool();

	cfl = root->findChild<QObject*>("cfl")->property("text").toDouble();
	if (cfl < 0)
		throw std::invalid_argument("CFL number must be greater than 0");

	minIter = root->findChild<QObject*>("minIter")->property("text").toInt();
	if (minIter < 1)
		throw std::invalid_argument("Minimum number of iterations must be greater than 0");

	maxIter = root->findChild<QObject*>("maxIter")->property("text").toInt();
	if (maxIter < minIter)
		throw std::invalid_argument("Maximum number of iterations must be greater than minimum number of iterations");

	tolerance = root->findChild<QObject*>("tolerance")->property("text").toDouble();
	if (tolerance < 0)
		throw std::invalid_argument("Tolerance must be greater than 0");

	viscosity = root->findChild<QObject*>("viscosity")->property("text").toDouble();
}
//}}}

void readInitialSolution(QTextStream &in) {
	in.readLine();
	uInit.fill(0);
	uInit[0] = in.readLine().simplified().toDouble();
}

void readBoundaryConditions(QTextStream &in) {
	in.readLine();
	uint numBoundariesConditions = in.readLine().simplified().toInt();

	uBoundaryData.fill(vector<double>(numBoundariesConditions));
	if (connectivityMatrixBoundaryConditions.size() == 0) {
		connectivityMatrixBoundaryConditions = vector<int>(numBoundariesConditions);
	} else {
		connectivityMatrixBoundaryConditions = vector<int>(numBoundaries);
	}
	for (int i = 0; i < numBoundaries; ++i) {
		in.readLine();
		auto list = in.readLine().simplified().split(" ");
		if (list.size() != 2)
			throw std::invalid_argument("Invalid boundary conditions");

		auto node = list[0].toInt() - 1; // -1 because the nodes are numbered from 1
		auto boundaryCode = list[1].toInt();
		connectivityMatrixBoundaryConditions[node] = boundaryCode;

		switch (boundaryCode) {
			case 4: { // Symmetry boundary
				auto list = in.readLine().simplified().split(" ");
				if (list.size() != 2)
					throw std::invalid_argument("Invalid symmetry boundary conditions");

				uBoundaryData[0][node] = list[0].toDouble();
				uBoundaryData[1][node] = list[1].toDouble();
				break;
			}
			case 6: { // Source boundary
				uBoundaryData[0][node] = in.readLine().simplified().toDouble();
				break;
			}
			case 7:{ // free boundary
				break;
			}

			default:
				throw std::invalid_argument("Invalid boundary code");
			return;
		}
	}
}

void readMeshData(QTextStream &in) {
	if (numNodes != 0) {
		meshData = vector<int>(65 * numNodes);
	} else {
		meshData = vector<int>(10);
	}

	for (int i = 0; i < 9; ++i) {
		meshData[i] = in.readLine().simplified().toInt();
	}
	numBoundaries = meshData[5];
	numNodes = meshData[1];
	numTriangles = meshData[2];
	numEdges = meshData[3];
	numTriangleEdge = meshData[7];
	meshDataHelper = meshData[8] + 1;

	if (connectivityMatrixBoundaryConditions.size() < numBoundaries) {
		connectivityMatrixBoundaryConditions.resize(numBoundaries);
	}

	if (meshData.size() < 65 * numNodes) {
		meshData.resize(65 * numNodes);
	}

	x = vector<double>(numNodes);
	y = vector<double>(numNodes);
	for (int i = 0; i < numNodes; ++i) {
		auto list = in.readLine().simplified().split(" ");
		if (list.size() != 2)
			throw std::invalid_argument("Invalid node coordinates");

		x[i] = list[0].toDouble();
		y[i] = list[1].toDouble();
	}

	for (int i = 1; i < numEdges; ++i) {
		auto list = in.readLine().simplified().split(" ");
		if (list.size() != 4)
			throw std::invalid_argument("Invalid edge connectivity");

		meshData[numTriangleEdge - 1 + 4 * i] = list[0].toInt();
		meshData[numTriangleEdge - 1 + 4 * i + 1] = list[1].toInt();
		meshData[numTriangleEdge - 1 + 4 * i + 2] = list[2].toInt();
		meshData[numTriangleEdge - 1 + 4 * i + 3] = list[3].toInt();
	}
}

void Reader::readMesh(QTextStream &in) {//{{{
	const QStringList readTypes = {"INITIAL SOLUTION", "BOUNDARY CONDITIONS", "MESH DATA"};
	// 3 = readTypes.size()
	array<bool, 3> readFlags = {false, false, false};
	while (!in.atEnd()) {
		auto line = in.readLine().simplified();
		// line.replace(QRegularExpression("[\t ]+"), " ");
		// line = line.trimmed();

		switch (readTypes.indexOf(line)) {

			case 0: // initial solution
				readInitialSolution(in);
				readFlags[0] = true;
				break;

			case 1: // boundary conditions
				readBoundaryConditions(in);
				readFlags[1] = true;
				break;

			case 2: // mesh data
				readMeshData(in);
				readFlags[2] = true;
				break;

			default:
				break;
		}
	}
	// throw error if any of the flags is false
	for (int i = 0; i < 3; ++i) {
		if (!readFlags[i]) {
			throw std::invalid_argument("Invalid input file. Missing " + readTypes[i].toStdString() + " section");
		}
	}
}

