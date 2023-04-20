// #include <QRegularExpression>

#include <src/headers/iosystem.h>
#include <src/headers/globals.h>
#include <src/headers/operations.h>
#include <src/headers/interface.h>

#include <nlohmann/json.hpp>
#include <fstream>

using namespace std;
using json = nlohmann::json;

void Reader::readInput() {//{{{

	numberArea = root->findChild<QObject*>("areas")->property("text").toInt();
	if (numberArea < 1)
		throw std::invalid_argument("Number of areas must be greater than 0");

	uInit = root->findChild<QObject*>("initialCondition")->property("text").toDouble();
		if (uInit < 0)
			throw std::invalid_argument("Initial condition must be greater than 0");

	axisymmetric = root->findChild<QObject*>("axisymmetric")->property("checked").toBool();
	resume = root->findChild<QObject*>("resume")->property("checked").toBool();

	cfl = root->findChild<QObject*>("cfl")->property("text").toDouble();
	if (cfl < 0)
		throw std::invalid_argument("CFL number must be greater than 0");

	cflViscous = root->findChild<QObject*>("viscousCFL")->property("text").toDouble();
	if (cflViscous < 0)
		throw std::invalid_argument("Viscous CFL number must be greater than 0");

	minIter = root->findChild<QObject*>("minIter")->property("text").toInt();
	if (minIter < 1)
		throw std::invalid_argument("Minimum number of iterations must be greater than 0");

	maxIter = root->findChild<QObject*>("maxIter")->property("text").toInt();
	if (maxIter < minIter)
		throw std::invalid_argument("Maximum number of iterations must be greater than minimum number of iterations");

	tolerance = root->findChild<QObject*>("tolerance")->property("text").toDouble();
	if (tolerance < 0)
		throw std::invalid_argument("Tolerance must be greater than 0");

	diffusiveWeight = root->findChild<QObject*>("diffusiveWeight")->property("text").toDouble();
	diffusiveMethod = root->findChild<QObject*>("diffusiveMethod")->property("currentIndex").toInt();
	numIsocontourLines = root->findChild<QObject*>("numIsocontourLines")->property("text").toInt();
	isocontourSize = root->findChild<QObject*>("isocontourSize")->property("text").toInt();
}
//}}}

namespace Reader::Legacy {//{{{
void readBoundaryConditions(QTextStream &in) {
	in.readLine();
	uint numBoundaries = in.readLine().simplified().toInt();

	uBoundaryData.fill(vector<double>(numBoundaries));
	connectivityMatrixBoundaryConditions = vector<int>(numBoundaries);

	for (uint i = 0; i < numBoundaries; ++i) {
		in.readLine();
		auto list = in.readLine().simplified().split(" ");
		if (list.size() != 2)
			throw std::invalid_argument("Invalid boundary conditions");

		auto boundary = list[0].toInt() - 1; // -1 because the nodes are numbered from 1
		auto boundaryCode = list[1].toInt();
		connectivityMatrixBoundaryConditions[boundary] = boundaryCode;

		switch (boundaryCode) {
			case 4: case 3: { // Symmetry boundary
				auto list = in.readLine().simplified().split(" ");
				if (list.size() != 2)
					throw std::invalid_argument("Invalid symmetry boundary conditions");

				uBoundaryData[0][boundary] = list[0].toDouble();
				uBoundaryData[1][boundary] = list[1].toDouble();
				break;
			}
			case 6: case 1: { // Source boundary
				uBoundaryData[0][boundary] = in.readLine().simplified().toDouble();
				break;
			}
			case 7: case 2: { // free boundary
				in.readLine();
				break;
			}

			default:
				throw std::invalid_argument("Invalid boundary code");
			return;
		}
	}
}

void readMeshData(QTextStream &in) {
	if (meshData.size() < 10)
		meshData = vector<int>(10);

	for (int i = 0; i < 9; ++i)
		meshData[i] = in.readLine().simplified().toInt();

	numNodes = meshData[1];
	numTriangles = meshData[2];
	numEdges = meshData[3];

	x = vector<double>(numNodes);
	y = vector<double>(numNodes);
	for (uint i = 0; i < numNodes; ++i) {
		auto list = in.readLine().simplified().split(" ");
		if (list.size() != 2)
			throw std::invalid_argument("Invalid node coordinates");

		x[i] = list[0].toDouble();
		y[i] = list[1].toDouble();
	}

	edgeData.fill(vector<int>(numEdges));
	for (uint i = 0; i < numEdges; ++i) {
		auto list = in.readLine().simplified().split(" ");
		if (list.size() != 4)
			throw std::invalid_argument("Invalid edge connectivity");

		edgeData[0][i] = list[0].toInt();
		edgeData[1][i] = list[1].toInt();
		edgeData[2][i] = list[2].toInt();
		edgeData[3][i] = list[3].toInt();
	}
}

void readMesh(QTextStream &in) {
	const QStringList readTypes = {"BOUNDARY CONDITIONS", "MESH DATA"};
	// 3 = readTypes.size()
	array<bool, 3> readFlags = {false, false};
	while (!in.atEnd()) {
		auto line = in.readLine().simplified();
		// line.replace(QRegularExpression("[\t ]+"), " ");
		// line = line.trimmed();

		switch (readTypes.indexOf(line)) {

			case 0: // boundary conditions
				readBoundaryConditions(in);
				readFlags[1] = true;
				break;

			case 1: // mesh data
				readMeshData(in);
				readFlags[2] = true;
				break;

			default:
				break;
		}
	}
	// throw error if any of the flags is false
	for (uint i = 0; i < readFlags.size(); ++i) {
		if (!readFlags[i]) {
			throw std::invalid_argument("Invalid input file. Missing " + readTypes[i].toStdString() + " section");
		}
	}
}
}
//}}}

namespace Reader::Json {//{{{
void readMesh(QString &filepath) {
	// remove later
	fstream file(filepath.toStdString());
	json json = json::parse(file);

	auto &metaData = json["metaData"];
	numNodes = metaData["nodes"];
	numTriangles = metaData["triangles"];
	numEdges = metaData["edges"];

	auto &mesh = json["mesh"];
	x = vector<double>(numNodes);
	y = vector<double>(numNodes);
	for (uint i = 0; i < numNodes; ++i) {
		x[i] = mesh["nodes"][i][0][0];
		y[i] = mesh["nodes"][i][0][1];
	}

	edgeData.fill(vector<int>(numEdges));
	for (uint i = 0; i < numEdges; ++i) {
		edgeData[0][i] = mesh["edges"][i][0][0];
		edgeData[1][i] = mesh["edges"][i][0][1];
		edgeData[2][i] = mesh["edges"][i][1];
		edgeData[3][i] = mesh["edges"][i][2];
	}

	
	auto &conditions = json["conditions"];
	uint numBoundaries = conditions["boundary"].size();
	uBoundaryData.fill(vector<double>(numBoundaries));
	connectivityMatrixBoundaryConditions = vector<int>(numBoundaries);

	for (uint boundary = 0; boundary < numBoundaries; ++boundary) {
		auto &condition = conditions["boundary"][boundary];
		auto &boundaryCode = condition[0];
		auto &boundaryType = condition[1];

		connectivityMatrixBoundaryConditions[boundary] = boundaryCode;

		const QStringList boundaryTypes = {"inlet", "outlet", "symmetry"};
		switch (boundaryTypes.indexOf(QString::fromStdString(boundaryType))) {

			case 0: // inlet
				uBoundaryData[0][boundary] = condition[2];
				break;
			case 1: // outlet
				break;
			case 2: // symmetry
				uBoundaryData[0][boundary] = condition[2][0];
				uBoundaryData[1][boundary] = condition[2][1];
				break;
			default:
				break;
		}
	}
}
}
//}}}


