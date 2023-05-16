import QtQuick 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15

Column {
	spacing: 0
	property string text: ""
	property string defaultInput: ""
	property string input: inputField.text
	property string objName: ""
	property string placeholderText: ""
	property string toolTipText: ""
	property bool decimals: false
	property bool negative: true
	width: parent.width

	Label {
		text: parent.text
		ToolTip.text: parent.toolTipText
		ToolTip.visible: parent.toolTipText ? mouseArea.containsMouse : false
		ToolTip.delay: 500

		MouseArea {
			id: mouseArea
			anchors.fill: parent
			hoverEnabled: true
		}
	}

	TextField {
		id: inputField
		property var intValidator: IntValidator {
			bottom: parent.negative ? -Infinity : 0
		}
		property var doubleValidator: DoubleValidator {
			bottom: parent.negative ? -Infinity : 0
		}
		placeholderText: parent.placeholderText
		selectByMouse: true
		objectName: parent.objName
		width: parent.width
		text: parent.defaultInput
		validator: parent.decimals ? doubleValidator : intValidator
	}
}
