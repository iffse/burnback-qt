import QtQuick 2.15
import QtQuick.Controls 2.12
import QtCharts 2.15

Item {
	ChartView {
		anchors.fill: parent
		antialiasing: true
		id: chart

		ValueAxis {
			id: xAxis
			min: 0 
			max: 1
		}

		ValueAxis {
			id: yAxis
			min: 0
			max: 1
		}


		LineSeries {
			id: series
			axisX: xAxis
			axisY: yAxis

			XYPoint { x: 0; y: 0 }

		}

		Connections {
			target: actions
			function onGraphBurningArea(points, xMax, yMax) {
				series.clear();
				xAxis.max = xMax;
				yAxis.max = yMax;
				for (var i = 0; i < points.length; ++i) {
					var point = points[i];
					console.log("x: " + point.x + " y: " + point.y);
					series.append(point.x, point.y);
				}
			}
		}
	}
}
