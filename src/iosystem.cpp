#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif

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
	uInit = root->findChild<QObject*>("initialCondition")->property("text").toDouble();
	axisymmetric = root->findChild<QObject*>("axisymmetric")->property("checked").toBool();
	resume = root->findChild<QObject*>("resume")->property("checked").toBool();
	cfl = root->findChild<QObject*>("cfl")->property("text").toDouble();
	cflViscous = root->findChild<QObject*>("viscousCFL")->property("text").toDouble();
	targetIter = root->findChild<QObject*>("targetIter")->property("text").toInt();
	tolerance = root->findChild<QObject*>("tolerance")->property("text").toDouble();

	diffusiveWeight = root->findChild<QObject*>("diffusiveWeight")->property("text").toDouble();
	diffusiveMethod = root->findChild<QObject*>("diffusiveMethod")->property("currentIndex").toInt();
	numIsocontourLines = root->findChild<QObject*>("numIsocontourLines")->property("text").toInt();
	isocontourSize = root->findChild<QObject*>("isocontourSize")->property("text").toInt();
	isocontourColor = root->findChild<QObject*>("isocontourColor")->property("currentIndex").toInt();
}
//}}}

namespace Reader::Legacy {//{{{
void readBoundaryConditions(QTextStream &in) {
	in.readLine();
	uint numBoundaries = in.readLine().simplified().toInt();

	// uBoundaryData.fill(vector<double>(numBoundaries));
	for (uint i = 0; i < numBoundaries; ++i) {
		in.readLine();
		auto list = in.readLine().simplified().split(" ");
		if (list.size() != 2)
			throw std::invalid_argument("Invalid boundary conditions");

		auto boundary = list[0].toInt();
		auto boundaryCode = list[1].toInt();

		switch (boundaryCode) {
			case 6: case 1: { // Source boundary
				uBoundaryData.insert(pair<int, double>(boundary, in.readLine().simplified().toDouble()));
				connectivityMatrixBoundaryConditions.insert(pair<int, int>(boundary, 1));
				break;
			}
			case 7: case 2: { // free boundary
				connectivityMatrixBoundaryConditions.insert(pair<int, int>(boundary, 2));
				uBoundaryData.insert(pair<int, double>(boundary, 0));
				in.readLine();
				break;
			}
			case 4: case 3: { // Symmetry boundary
				auto list = in.readLine().simplified().split(" ");
				if (list.size() != 2)
					throw std::invalid_argument("Invalid symmetry boundary conditions");
				auto angle = atan(list[1].toDouble() / list[0].toDouble());
				uBoundaryData.insert(pair<int, double>(boundary, angle));
				connectivityMatrixBoundaryConditions.insert(pair<int, int>(boundary, 3));
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
	recession = vector<double>(numNodes, 1);


	x = vector<double>(numNodes);
	y = vector<double>(numNodes);
	for (uint i = 0; i < numNodes; ++i) {
		auto list = in.readLine().simplified().split(" ");
		if (list.size() != 2)
			throw std::invalid_argument("Invalid node coordinates");

		x[i] = list[0].toDouble();
		y[i] = list[1].toDouble();
	}

	edgeData = vector<array<int, 4>>(numEdges);
	for (uint i = 0; i < numEdges; ++i) {
		auto list = in.readLine().simplified().split(" ");
		if (list.size() != 4)
			throw std::invalid_argument("Invalid edge connectivity");

		edgeData[i][0] = list[0].toInt();
		edgeData[i][1] = list[1].toInt();
		edgeData[i][2] = list[2].toInt();
		edgeData[i][3] = list[3].toInt();
	}
}

void readMesh(QTextStream &in) {
	const QStringList readTypes = {"BOUNDARY CONDITIONS", "MESH DATA"};
	array<bool, 2> readFlags = {false, false};
	while (!in.atEnd()) {
		auto line = in.readLine().simplified();
		// line.replace(QRegularExpression("[\t ]+"), " ");
		// line = line.trimmed();

		switch (readTypes.indexOf(line)) {

			case 0: // boundary conditions
				readBoundaryConditions(in);
				readFlags[0] = true;
				break;

			case 1: // mesh data
				readMeshData(in);
				readFlags[1] = true;
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
	fstream file(filepath.toStdString());
	json json;

	try {
		json = json::parse(file);
	} catch (...) {
		throw std::invalid_argument("Unable to parse Json file. Invalid JSON file?");
		return;
	}

	try {
		auto &metaData = json["metaData"];
		numNodes = metaData["nodes"];
		numTriangles = metaData["triangles"];
		numEdges = metaData["edges"];
	} catch (...) {
		throw std::invalid_argument("Unable to read mesh data from JSON file. Missing data field?");
		return;
	}

	try {
		auto &mesh = json["mesh"];
		x = vector<double>(numNodes);
		y = vector<double>(numNodes);
		for (uint i = 0; i < numNodes; ++i) {
			x[i] = mesh["nodes"][i][0];
			y[i] = mesh["nodes"][i][1];
		}
	} catch (...) {
		throw std::invalid_argument("Unable to read node coordinates from JSON file. Missing nodes field or wrong format?");
		return;
	}

	try {
		auto &mesh = json["mesh"];
		edgeData = mesh["edges"];
	} catch (...) {
		throw std::invalid_argument("Unable to read edge connectivity from JSON file. Missing edges field or wrong format?");
		return;
	}

	auto &conditions = json["conditions"];
	try {
		uint numBoundaries = conditions["boundary"].size();

		for (uint boundary = 0; boundary < numBoundaries; ++boundary) {
			auto &condition = conditions["boundary"][boundary];
			auto &boundaryTag = condition["tag"];
			auto &boundaryType = condition["type"];
			try {
				auto &boundaryDescription = condition["description"];
				boundaryDescriptions.insert(pair<int, QString>(boundaryTag, QString::fromStdString(boundaryDescription)));
			} catch (...) {
				boundaryDescriptions.insert(pair<int, QString>(boundaryTag, QString::fromStdString("")));
			}
			const QStringList boundaryTypes = {"inlet", "outlet", "symmetry", "condition"};
			switch (boundaryTypes.indexOf(QString::fromStdString(boundaryType))) {
				case 0: // inlet
					uBoundaryData.insert(pair<int, double>(boundaryTag, condition["value"]));
					connectivityMatrixBoundaryConditions.insert(pair<int, int>(boundaryTag, 1));
					break;
				case 1: case 3: // outlet or undefined
					uBoundaryData.insert(pair<int, double>(boundaryTag, 0));
					connectivityMatrixBoundaryConditions.insert(pair<int, int>(boundaryTag, 2));
					break;
				case 2: // symmetry
					uBoundaryData.insert(pair<int, double>(boundaryTag, double(condition["value"]) * M_PI / 180));
					connectivityMatrixBoundaryConditions.insert(pair<int, int>(boundaryTag, 3));
					break;
				default:
					break;
			}
		}
	} catch(...) {
		throw std::invalid_argument("Unable to read boundary conditions from JSON file. Missing boundary field or wrong format?");
		return;
	}
	try {
		auto &recessionCondition = conditions["recession"];
		if (recessionCondition.size() == 0)
			recession = vector<double>(numNodes, 1);
		else
			recession = recessionCondition.get<vector<double>>();
	} catch(...) {
		throw std::invalid_argument("Unable to read recession from JSON file. Missing recession field or wrong format?");
		return;
	}
}
}

namespace Writer::Json {
void writeData(QString &filepath, QString &origin, bool &pretty) {
	fstream originalFile(origin.toStdString());
	json results;
	results["uVertex"] = uVertex;
	results["duVertex"] = duVertex;
	results["fluxes"] = flux;
	results["burningArea"] = burningArea;
	results["timeStep"] = cfl * *min_element(height.begin(), height.end());
	results["timeTotal"] = timeTotal;
	results["error"] = errorIter;

	try {
		json jsonFile = json::parse(originalFile);
		jsonFile["burnbackResults"] = results;

		ofstream file(filepath.toStdString());
		if (pretty)
			file << setw(4) << jsonFile << endl;
		else
		file << jsonFile << endl;
	} catch (...) {
		ofstream file(filepath.toStdString());
		if (pretty)
			file << setw(4) << results << endl;
		else
			file << results << endl;
		throw std::invalid_argument("Unable to parse JSON file. Invalid JSON file?\nA file with only results is created.");
	}

}

void updateBoundaries(QString &filepath, bool &pretty) {
	fstream originalFile(filepath.toStdString());
	json jsonFile;
	try {
		jsonFile = json::parse(originalFile);
	} catch (...) {
		throw std::invalid_argument("Unable to parse JSON file. Invalid JSON file?");
		return;
	}

	json updatedBoundaries;
	for (auto &boundary : uBoundaryData) {
		auto &boundaryTag = boundary.first;
		auto boundaryValue = boundary.second;
		auto boundaryType = "";
		switch (connectivityMatrixBoundaryConditions[boundaryTag]) {
			case 1:
				boundaryType = "inlet";
				break;
			case 2:
				boundaryType = "outlet";
				break;
			case 3:
				boundaryType = "symmetry";
				boundaryValue = boundaryValue * 180 / M_PI;
				break;
		};

		auto &boundaryDescription = boundaryDescriptions[boundaryTag];
		updatedBoundaries.push_back({{"tag", boundaryTag}, {"type", boundaryType}, {"value", boundaryValue}, {"description", boundaryDescription.toStdString()}});
	}
	jsonFile["conditions"]["boundary"] = updatedBoundaries;

	ofstream file(filepath.toStdString());
	if (pretty)
		file << setw(4) << jsonFile << endl;
	else
		file << jsonFile << endl;
}
}
//}}}

