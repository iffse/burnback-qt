import QtQuick 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15

Item{
	TabBar {
		id: bar
		width: parent.width
		TabButton {
			text: qsTr("Inputs")
		}
		TabButton {
			text: qsTr("Outputs")
		}
	}

	StackLayout {
		id: barCotent
		width: parent.width
		height: parent.height - bar.height
		currentIndex: bar.currentIndex
		anchors.top: bar.bottom
		anchors.topMargin: 10
		Item {
			id: inputTab
			width: parent.width
			height: parent.height
			InputConf {
				width: parent.width
				height: parent.height - 10
			}
		}
		Item {
			id: outputTab
			width: parent.width
			height: parent.height
			OutputConf {
				width: parent.width
				height: parent.height - 10
			}
		}
	}
}
