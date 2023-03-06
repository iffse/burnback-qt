#include <QFile>
#include <QTextStream>
#include <vector>

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

vector<double> isocolourData(int numLines) {
	double maxValue = *max_element(uVertex.begin(), uVertex.end());
	double minValue = *min_element(uVertex.begin(), uVertex.end());
	vector<double> data;

	double step = (maxValue - minValue) / numLines;
	for (double value = minValue; value < maxValue; value += step) {

		for (uint triangle = 0; triangle < numTriangles; ++triangle) {
			uint node1 = connectivityMatrixNodeTriangle[0][triangle] - 1;
			uint node2 = connectivityMatrixNodeTriangle[1][triangle] - 1;
			uint node3 = connectivityMatrixNodeTriangle[2][triangle] - 1;

			double value1 = uVertex[node1] - value;
			double value2 = uVertex[node2] - value;
			double value3 = uVertex[node3] - value;

			if (value1 * value2 < 0) {
				double x1 = x[node1];
				double x2 = x[node2];
				double y1 = y[node1];
				double y2 = y[node2];

				double x = lerp(x1, x2, - value2 / (value1 - value2));
				double y = lerp(y1, y2, - value2 / (value1 - value2));

				data.push_back(x);
				data.push_back(y);
			}

			if (value2 * value3 < 0) {
				double x1 = x[node2];
				double x2 = x[node3];
				double y1 = y[node2];
				double y2 = y[node3];

				double x = lerp(x1, x2, - value3 / (value2 - value3));
				double y = lerp(y1, y2, - value3 / (value2 - value3));

				data.push_back(x);
				data.push_back(y);
			}

			if (value3 * value1 < 0) {
				double x1 = x[node3];
				double x2 = x[node1];
				double y1 = y[node3];
				double y2 = y[node1];

				double x = lerp(x1, x2, - value1 / (value3 - value1));
				double y = lerp(y1, y2, - value1 / (value3 - value1));

				data.push_back(x);
				data.push_back(y);

			}
		}

	}
	return data;
}

}
