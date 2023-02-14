import QtQuick 2.15
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.15


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
		Action { text: qsTr("Import / Open") }
		Action { text: qsTr("Save") }
		Action { text: qsTr("Save As") }
		MenuSeparator { }
		Action { text: qsTr("Quit") }
	}
	Menu {
		Material.background: menuBar.Material.background
		title: qsTr("Help")
		Action { text: qsTr("About") }
	}
}
