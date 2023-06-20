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
		value: 0
	}
	Label {
		id: compleatedCount
		text: "0/0"
	}

	CheckBox {
		objectName: "resume"
		text: qsTr("Resume")
		ToolTip.text: qsTr("Mark this checkbox if you want to resume a previous computation")
		ToolTip.visible: hovered
		ToolTip.delay: 500
		hoverEnabled: true
		enabled: false
	}

	Button {
		id: runButton
		enabled: false
		objectName: "runButton"
		text: "Run"
		onClicked: {
			if (runButton.text == "Run") {
				actions.run()
			} else {
				actions.stop()
			}
		}
	}

	Connections {
		target: actions
		function onUpdateProgress(progress, total) {
			progressBar.value = progress/total
			compleatedCount.text = progress + "/" + total
		}
	}
}
