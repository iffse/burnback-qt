import QtQuick 2.15
import QtQuick.Controls 2.12

Item {
		GroupBox {
			id: output
			title: qsTr("Output")
			anchors.fill: parent
			TextArea {
				id: textArea
				text: ""
				placeholderText: "Outputs would be printed here."
				anchors.fill: parent

				selectByMouse: true
				readOnly: true
			}
		}
}
