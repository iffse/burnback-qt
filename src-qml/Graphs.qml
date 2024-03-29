import QtQuick 2.15
import QtQuick.Controls 2.12
import QtCharts 2.15
import QtQuick.Controls.Material 2.15


Item {
	Row {
		width: parent.width
		height: parent.height
		spacing: 10
		ChartView{
			height: parent.height
			width: (parent.width - parent.spacing) / 2
			antialiasing: true
			theme: Material.theme === Material.Dark ? ChartView.ChartThemeDark : ChartView.ChartThemeLight
			backgroundColor: Material.theme === Material.Dark ? Material.color(Material.Grey, Material.Shade900) : Material.color(Material.Grey, Material.Shade50)

			legend.visible: false
			title: "Burning Area with Time"

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
			}

			Connections {
				target: actions
				function onGraphBurningArea(points, xMax, yMax) {
					series.clear();
					xAxis.max = xMax;
					yAxis.max = yMax;
					for (var i = 0; i < points.length; ++i) {
						var point = points[i];
						series.append(point.x, point.y);
					}
				}
			}
		}

		ChartView{
			height: parent.height
			width: (parent.width - parent.spacing) / 2
			antialiasing: true
			theme: Material.theme === Material.Dark ? ChartView.ChartThemeDark : ChartView.ChartThemeLight
			backgroundColor: Material.theme === Material.Dark ? Material.color(Material.Grey, Material.Shade900) : Material.color(Material.Grey, Material.Shade50)

			legend.visible: false
			title: "Error with Iteration"

			ValueAxis {
				id: errorXAxis
				min: 1
				max: 5
				labelFormat: "%i"
			}

			ValueAxis {
				id: errorYAxis
				min: 0
				max: 1
			}


			LineSeries {
				id: errorSeries
				axisX: errorXAxis
				axisY: errorYAxis
			}

			Connections {
				target: actions
				function onGraphErrorIter(points, errorMax) {
					errorSeries.clear();
					errorYAxis.max = errorMax;
					errorXAxis.max = points.length;

					for (var i = 0; i < points.length; ++i) {
						errorSeries.append(i+1, points[i]);
					}
				}
			}
		}
	}
}
