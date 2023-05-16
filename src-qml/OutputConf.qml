import QtQuick 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3

ScrollView {
	clip: true
	Column {
		spacing: 10
		width: parent.width

		GroupBox {
			title: qsTr("Diffusive method")
			width: parent.width

			Column {
				width: parent.width

				ComboBox {
					id: diffusiveMethod
					objectName: "diffusiveMethod"
					width: parent.width
					currentIndex: 2
					model: [
						"Abgrall",
						"Tizón",
						"Zhang and Shu"
					]
				}

				LabelInput {
					width: parent.width
					text: "Weight"
					placeholderText: "Enter a number"
					toolTipText: "."
					objName: "diffusiveWeight"
					defaultInput: "1"
					decimals: true
				}

				LabelInput {
					visible: diffusiveMethod.currentIndex == 1
					width: parent.width
					text: "Viscous CFL"
					placeholderText: "Enter a number"
					toolTipText: "Viscous CFL number."
					objName: "viscousCFL"
					defaultInput: "1"
					decimals: true
				}
			}
		}

		GroupBox {
			title: qsTr("Isocontour lines")
			width: parent.width

			Column {
				width: parent.width

				LabelInput {
					id: isocontourSize
					text: "Size to draw (px)"
					placeholderText: "Enter a number"
					toolTipText: "Maximum size of the weight or height to draw the isocontour lines, in pixels."
					objName: "isocontourSize"
					defaultInput: "500"
					decimals: false
				}

				LabelInput {
					id: numIsocontourLines
					text: "Number of lines"
					placeholderText: "Enter a number"
					toolTipText: "Number of isocontour lines to draw. 0 to disable."
					objName: "numIsocontourLines"
					defaultInput: "10"
					decimals: false
				}

				Button {
					text: "Redraw"
					onClicked: {
						actions.redrawIsocontourLines(isocontourSize.input, numIsocontourLines.input);
					}
				}
			}
		}

	}
}

