import QtQuick 2.15
import QtQuick.Controls 2.12

Item {
	GroupBox {
		id: results
		title: qsTr("Results")
		anchors.fill: parent
		ResultTab {
			anchors.fill: parent
		}
	}
}
