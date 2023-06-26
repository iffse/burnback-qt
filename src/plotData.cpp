#include "qnamespace.h"
#include <QFile>
#include <QTextStream>
#include <algorithm>
#include <iterator>
#include <QPointF>

#if __cplusplus >= 202002L
#include <cmath>
#endif


#include <src/headers/plotData.h>
#include <src/headers/globals.h>

using namespace Qt;
using namespace std;

namespace plotData {

// lerp is available since c++20
// as time of writing, cpp20 has only partial support by the compilers
// therefore cpp17 is expected to be used
#if __cplusplus < 202002L
double lerp(double x1, double x2, double t) {
	return x1 + t * (x2 - x1);
}
#endif

vector<double> isocolourData(double value, int shiftX, int shiftY, double scale) {
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

			double x = lerp(x1, x2, t) * scale + shiftX;
			double y = lerp(y1, y2, t) * scale + shiftY;

			data.push_back(x);
			data.push_back(y);
		}

		if (value2 * value3 < 0) {
			double x1 = x[node2];
			double x2 = x[node3];
			double y1 = y[node2];
			double y2 = y[node3];

			double t = (value - uVertex2) / (value3 - value2);

			double x = lerp(x1, x2, t) * scale + shiftX;
			double y = lerp(y1, y2, t) * scale + shiftY;

			data.push_back(x);
			data.push_back(y);
		}

		if (value3 * value1 < 0) {
			double x1 = x[node3];
			double x2 = x[node1];
			double y1 = y[node3];
			double y2 = y[node1];

			double t = (value - uVertex3) / (value1 - value3);

			double x = lerp(x1, x2, t) * scale + shiftX;
			double y = lerp(y1, y2, t) * scale + shiftY;

			data.push_back(x);
			data.push_back(y);

		}
	}

	return data;
}

vector<double> contourData(int shiftX, int shiftY, double scale) {

	vector<double> data = vector<double>(4 * numBoundaryEdge);

	for (uint boundary = 0; boundary < numBoundaryEdge; ++boundary) {
		uint node1 = connectivityMatrixNodeBoundary[0][boundary] - 1;
		uint node2 = connectivityMatrixNodeBoundary[1][boundary] - 1;

		double x1 = x[node1];
		double x2 = x[node2];
		double y1 = y[node1];
		double y2 = y[node2];

		data[4 * boundary] = x1 * scale + shiftX;
		data[4 * boundary + 1] = y1 * scale + shiftY;
		data[4 * boundary + 2] = x2 * scale + shiftX;
		data[4 * boundary + 3] = y2 * scale + shiftY;
	}

	return data;
}

// Same as contourData, but returns a dictionary to be handle in the preview directly
void generateContourDataDict(int shiftX, int shiftY, double scale) {
	
	map<uint, vector<double>> data;

	for_each(edgeData.begin(), edgeData.end(), [&data, &scale, &shiftX, &shiftY](auto &edge) {
		int boundary = 0;

		if (edge[2] < 0) {
			boundary = -edge[2];
		} else if (edge[3] < 0) {
			boundary = -edge[3];
		} else {
			return;
		}

		auto node1 = edge[0] - 1;
		auto node2 = edge[1] - 1;
		auto x1 = x[node1] * scale + shiftX;
		auto y1 = y[node1] * scale + shiftY;
		auto x2 = x[node2] * scale + shiftX;
		auto y2 = y[node2] * scale + shiftY;

		auto list = vector<double>{x1, y1, x2, y2};

		if (data.find(boundary) == data.end())
			data[boundary] = list;
		else
			data[boundary].insert(data[boundary].end(), list.begin(), list.end());
	});

	// separate the data in two lists
	auto boundaryTags = vector<uint>{};
	auto boundaryCoordinates = vector<vector<double>>{};

	for (auto &boundary : data) {
		boundaryTags.push_back(boundary.first);
		boundaryCoordinates.push_back(boundary.second);
	}

	auto canvas = root->findChild<QObject*>("previewCanvas");
	canvas->setProperty("boundaryTags", QVariant::fromValue(boundaryTags));
	canvas->setProperty("boundaryCoordinates", QVariant::fromValue(boundaryCoordinates));
	return;
}

QVariant burningAreaData() {

	QVariantList data;

	for(uint area = 0; area < numberArea; ++area) {
		data.append(QPointF(burningTime[area], burningArea[area]));
	}

	return QVariant::fromValue(data);
}
}
