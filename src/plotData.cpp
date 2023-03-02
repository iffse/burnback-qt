#include <QFile>
#include <QTextStream>


#include <src/headers/plotData.h>
#include <src/headers/globals.h>

using namespace Qt;

namespace plotData {

void generateData() {
	QString filename = "plotData.txt";
	QFile file(filename);

	if (!(file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)))
		throw std::runtime_error("Could not open file");

	QTextStream out(&file);

	out << numTriangles << endl;
	for (uint triangle = 0; triangle < numTriangles; ++triangle) {
		// out << QString::number(3) + " ";
		for (int i = 0; i < 3; ++i)
			out << QString::number(connectivityMatrixNodeTriangle[i][triangle]) + " ";
		out << endl;
	}

	out << numNodes << endl;
	// for (int i = 0; i < 6; ++i)
	// 	out << QString::number(1) + " ";
	// out << endl;

	for (uint node = 0; node < numNodes; ++node) {
		out << x[node] << " " << y[node] << " "
			<< uVertex[node] << " " << duVertex[0][node] << " " << duVertex[1][node]
		;out << endl;
	}

	out << QString::number(numBoundaryEdge) << endl;

	for (uint boundary = 0; boundary < numBoundaryEdge; ++boundary) {
		out << connectivityMatrixNodeBoundary[0][boundary] << " "
			<< connectivityMatrixNodeBoundary[1][boundary]
			<< endl;
	}

}

}
