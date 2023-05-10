import QtQuick 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15

Item {
	width: parent.width
	RowLayout {
		anchors.fill: parent
		anchors.margins: 10
		spacing: 10

		TextField {
			placeholderText: "id"
			horizontalAlignment: Text.AlignHCenter
			width: 50
			readOnly: true
		}
		TextField {
			placeholderText: "Value"
			width: 200
		}
		TextField {
			placeholderText: "Description"
			Layout.fillWidth: true
		}
	}
}
