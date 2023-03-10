import QtQuick 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15

Item{
	StackLayout {
		id: resultBarCotent
		width: parent.width
		height: parent.height - resultBar.height
		currentIndex: resultBar.currentIndex
		// anchors.top: resultBar.bottom
		anchors.topMargin: 10
		IsocontourCanvas {}
		Graphs {}
	}

	TabBar {
		id: resultBar
		width: parent.width
		anchors.top: resultBarCotent.bottom
		TabButton {
			text: qsTr("Isocontour lines")
		}
		TabButton {
			text: qsTr("Graphs")
		}
	}
}
