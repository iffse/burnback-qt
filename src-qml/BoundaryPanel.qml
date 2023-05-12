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

	Component.onCompleted: {
		for (var i = 1; i < 20; i++) {
			var button = buttonDelegate.createObject(grid);
			button.text = i;
			var comboBox = comboBoxDelegate.createObject(grid);
			var value = valueDelegate.createObject(grid);
			var description = descriptionDelegate.createObject(grid);
		}
	}


	contentData: RowLayout {
		anchors.fill: parent
		anchors.margins: 10
		spacing: 10

		GroupBox {
			title: qsTr("Boundary conditions")
			Layout.fillHeight: true
			Layout.fillWidth: true
			ScrollView {
				anchors.fill: parent
				clip: true
				GridLayout {
					id: grid
					columns: 4
					width: parent.parent.width
					// clip: true

					Component {
						id: buttonDelegate
						Button {
							Layout.preferredWidth: 50
						}
					}

					Component {
						id: comboBoxDelegate
						ComboBox {
							model: ["---Condition---", "Inlet", "Outlet", "Symmetry"]
							Layout.preferredWidth: 150
						}
					}

					Component {
						id: valueDelegate
						TextField {
							placeholderText: qsTr("value")
							validator: DoubleValidator {}
							Layout.preferredWidth: 200
							Component.onCompleted: cursorPosition = 0
						}
					}

					Component {
						id: descriptionDelegate
						TextField {
							placeholderText: qsTr("description")
							Layout.fillWidth: true
						}
					}
				}
			}
		}
		GroupBox {
			title: qsTr("Preview")
			Layout.fillHeight: true
			contentWidth: 350
		}
	}

	footer: Row {
		width: parent.width
		layoutDirection: Qt.RightToLeft
		rightPadding: 10
		spacing: 10

		Button {
			text: qsTr("Save")
		}
		CheckBox {
			text: qsTr("Save to file")
		}
	}
}
