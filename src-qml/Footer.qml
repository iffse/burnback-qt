import QtQuick 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3

RowLayout {
	anchors.left: parent.left
	anchors.right: parent.right
	anchors.leftMargin: 10
	anchors.rightMargin: 10
	spacing: 10
	ProgressBar {
		id: progressBar
		Layout.fillWidth: true
		value: 0.5
	}
	Label {
		id: compleatedCount
		text: "0/0"
	}
	Button {
		id: runButton
		text: "Run"
		onClicked: actions.run()
	}
}
