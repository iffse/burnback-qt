import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.15

ApplicationWindow {
	id: root
	width: 1280
	height: 720
	visible: true
	title: qsTr("Burnback")

	menuBar: AplicationMenu {}

	contentData: RowLayout {
		anchors.fill: parent
		anchors.margins: 10
		spacing: 10

		Tabs {
			Layout.preferredWidth: 250
			Layout.fillHeight: true
		}
		ColumnLayout {
			Layout.fillWidth: true
			Layout.fillHeight: true
			PlotArea {
				Layout.fillWidth: true
				Layout.fillHeight: true
			}
			Output {
				Layout.fillWidth: true
				height: 200
			}
		}
	}

	footer: Footer {}
}
