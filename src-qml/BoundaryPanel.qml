import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15

ApplicationWindow {
	id: boundaryPanel
	width: 1280
	height: 720
	visible: true
	title: qsTr("Boundary Panel")

	RowLayout {
		anchors.fill: parent
		anchors.margins: 10
		spacing: 10

		GroupBox {
			title: qsTr("Boundary conditions")
			contentWidth: parent.width / 2
			Layout.fillHeight: true

		}
		GroupBox {
			title: qsTr("Preview")
			Layout.fillHeight: true
			Layout.fillWidth: true
			width: 500
		}
	}
}
