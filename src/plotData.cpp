#include <QFile>
#include <QTextStream>
#include "QPointF"

#if __cplusplus >= 202002L
#include <cmath>
#endif


#include <src/headers/plotData.h>
#include <src/headers/globals.h>

using namespace Qt;
using namespace std;

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

// lerp is available since c++20
// as time of writing, cpp20 has only partial support by the compilers
// therefore cpp17 is expected to be used
#if __cplusplus < 202002L
double lerp(double x1, double x2, double t) {
	return x1 + t * (x2 - x1);
}
#endif

vector<double> isocolourData(double value) {
	vector<double> data;

	for (uint triangle = 0; triangle < numTriangles; ++triangle) {
		uint node1 = connectivityMatrixNodeTriangle[0][triangle] - 1;
		uint node2 = connectivityMatrixNodeTriangle[1][triangle] - 1;
		uint node3 = connectivityMatrixNodeTriangle[2][triangle] - 1;

		double &uVertex1 = uVertex[node1];
		double &uVertex2 = uVertex[node2];
		double &uVertex3 = uVertex[node3];
		double value1 = uVertex1 - value;
		double value2 = uVertex2 - value;
		double value3 = uVertex3 - value;

		if (value1 * value2 < 0) {
			double x1 = x[node1];
			double x2 = x[node2];
			double y1 = y[node1];
			double y2 = y[node2];

			double t = (value - uVertex1) / (value2 - value1);

			double x = lerp(x1, x2, t) * 10;
			double y = lerp(y1, y2, t) * 10;

			data.push_back(x);
			data.push_back(y);
		}

		if (value2 * value3 < 0) {
			double x1 = x[node2];
			double x2 = x[node3];
			double y1 = y[node2];
			double y2 = y[node3];

			double t = (value - uVertex2) / (value3 - value2);

			double x = lerp(x1, x2, t) * 10;
			double y = lerp(y1, y2, t) * 10;

			data.push_back(x);
			data.push_back(y);
		}

		if (value3 * value1 < 0) {
			double x1 = x[node3];
			double x2 = x[node1];
			double y1 = y[node3];
			double y2 = y[node1];

			double t = (value - uVertex3) / (value1 - value3);

			double x = lerp(x1, x2, t) * 10;
			double y = lerp(y1, y2, t) * 10;

			data.push_back(x);
			data.push_back(y);

		}
	}

	return data;
}

vector<double> contourData() {

	vector<double> data = vector<double>(4 * numBoundaryEdge);

	for (uint boundary = 0; boundary < numBoundaryEdge; ++boundary) {
		uint node1 = connectivityMatrixNodeBoundary[0][boundary] - 1;
		uint node2 = connectivityMatrixNodeBoundary[1][boundary] - 1;

		double x1 = x[node1];
		double x2 = x[node2];
		double y1 = y[node1];
		double y2 = y[node2];

		data[4 * boundary] = x1 * 10;
		data[4 * boundary + 1] = y1 * 10;
		data[4 * boundary + 2] = x2 * 10;
		data[4 * boundary + 3] = y2 * 10;
	}

	return data;

}

QVariant burningAreaData() {

	QVariantList data;

	for(uint area = 0; area < numberArea; ++area) {
		data.append(QPointF(burningWay[area], burningArea[area]));
	}

	return QVariant::fromValue(data);
}

}
