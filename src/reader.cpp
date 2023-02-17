#include <src/headers/reader.h>
#include <src/headers/globals.h>
#include <src/headers/interface.h>

#include <QDebug>
#include <QFile>
#include <QRegularExpression>
#include <tuple>

bool Reader::readInput() {
	Actions actions;
	actions.appendOutput("--> Reading inputs");

	numberArea = root->findChild<QObject*>("areas")->property("text").toInt();
	if (numberArea < 1) {
		actions.appendOutput("Error: Number of segments must be greater than 0");
		return false;
	}
	axisymmetric = root->findChild<QObject*>("axisymmetric")->property("checked").toBool();
	resume = root->findChild<QObject*>("resume")->property("checked").toBool();

	cfl = root->findChild<QObject*>("cfl")->property("text").toDouble();
	if (cfl < 0) {
		actions.appendOutput("Error: CFL number must be greater than 0");
		return false;
	}
	minIter = root->findChild<QObject*>("minIter")->property("text").toInt();
	if (minIter < 1) {
		actions.appendOutput("Error: Minimum number of iterations must be greater than 0");
		return false;
	}
	maxIter = root->findChild<QObject*>("maxIter")->property("text").toInt();
	if (maxIter < 1) {
		actions.appendOutput("Error: Maximum number of iterations must be greater than 0");
		return false;
	}
	tolerance = root->findChild<QObject*>("tolerance")->property("text").toDouble();
	if (tolerance < 0) {
		actions.appendOutput("Error: Tolerance must be greater than 0");
		return false;
	}
	viscosity = root->findChild<QObject*>("viscosity")->property("text").toDouble();

	return true;
}

bool Reader::readMesh(QString filepath) {
	QFile file(filepath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return false;
	}
	QTextStream in(&file);

	QStringList readTypes = {"INITIAL SOLUTION", "BOUNDARY CONDITIONS", "MESH DATA"};
	while (!in.atEnd()) {
		QString line = in.readLine().simplified();
		// line.replace(QRegularExpression("[\t ]+"), " ");
		// line = line.trimmed();

		switch (readTypes.indexOf(line)) {

			case 0: // initial solution
				in.readLine();
				uInit[1] = in.readLine().toDouble();
				break;

			case 1: // boundary conditions
				in.readLine();
				numBoundaries = in.readLine().toInt();
				for (int i = 0; i < numBoundaries; ++i) {
					in.readLine();
					auto list = in.readLine().split(" ");
					int node = list[0].toInt();
					int borderCode = list[1].toInt();

					switch (borderCode) {
						case 4: { // Symmetry boundary
							auto list = in.readLine().split(" ");
							ubData[1][node] = list[0].toDouble();
							ubData[2][node] = list[1].toDouble();
							break;
						}
						case 6: { // Source boundary
							ubData[1][node] = in.readLine().toDouble();
							break;
						}
						case 7:{ // free boundary
							break;
						}
							
						default:
							return false;
					}
				}
				break;

			case 2: // mesh data
				
				break;

			default:
				return false;
		}
	}
	file.close();
	return true;
}
