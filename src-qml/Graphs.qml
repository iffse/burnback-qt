import QtQuick 2.15
import QtQuick.Controls 2.12
import QtCharts 2.15
import QtQuick.Controls.Material 2.15


Item {
	ChartView {
		anchors.fill: parent
		antialiasing: true
		id: chart
		theme: Material.theme === Material.Dark ? ChartView.ChartThemeDark : ChartView.ChartThemeLight
		backgroundColor: Material.theme === Material.Dark ? Material.color(Material.Grey, Material.Shade900) : Material.color(Material.Grey, Material.Shade50)

		// set backgroundcolor to match the theme
		legend.visible: false

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
				xAxis.max = xMax * 1.05;
				yAxis.max = yMax * 1.05;
				for (var i = 0; i < points.length; ++i) {
					var point = points[i];
					series.append(point.x, point.y);
				}
			}
		}
	}
}
