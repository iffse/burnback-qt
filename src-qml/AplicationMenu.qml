import QtQuick 2.15
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.15
import QtQuick.Dialogs 1.3


MenuBar {
	id: menuBar

	Material.background: {
		if (Material.theme == Material.Light) {
			Material.color(Material.BlueGrey, Material.Shade50)
		}
	}
	Menu {
		Material.background: menuBar.Material.background
		title: qsTr("File")
		Action {
			text: qsTr("Export results")
			onTriggered: {
				exportDialog.exportPretty = false
				exportDialog.open()
			}
		}

		Action {
			text: qsTr("Export results (pretty)")
			onTriggered: {
				exportDialog.exportPretty = true
				exportDialog.open()
			}
		}

		FileDialog {
			property bool exportPretty: false
			id: exportDialog
			title: qsTr("Export results")
			objectName: "exportDialog"
			selectExisting: false
			selectFolder: false
			folder: ""
			nameFilters: ["JSON file (*.json)", "All files (*)"]
			onAccepted: actions.exportData(exportDialog.fileUrl, exportDialog.exportPretty)
		}
		MenuSeparator {}
		Action {
			text: qsTr("Quit")
			onTriggered: Qt.quit()
		}
	}
	Menu {
		Material.background: menuBar.Material.background
		title: qsTr("Conditions")
		Action {
			text: qsTr("Boundary conditions")
			onTriggered: {
				var component = Qt.createComponent("BoundaryPanel.qml");
				var window = component.createObject(root)
				window.show()
			}
		}
		Action {
			text: qsTr("Recession conditions") 
			onTriggered: {
				var component = Qt.createComponent("RecessionPanel.qml");
				var window = component.createObject(root)
				window.show()
			}
		}
	}
	Menu {
		Material.background: menuBar.Material.background
		title: qsTr("Help")
		Action {
			text: qsTr("About")
			onTriggered: aboutDialog.open()
		}
		MessageDialog {
			id: aboutDialog
			title: qsTr("About")
			text: qsTr(
				"Burnback analysis for solid propellant rockets, with Time Marching Method\n\n" +
				"Distributed under AGPL3 license\n" +
				"Source code available at https://codeberg.org/iff/burnback-qt"
			)
			standardButtons: StandardButton.Ok
		}
	}
}
