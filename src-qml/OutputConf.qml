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
					currentIndex: 0
					model: [
						"Triangle Based",
						"Edge Based"
					]
				}

				LabelInput {
					width: parent.width
					text: "Weight"
					placeholderText: "Enter a number"
					toolTipText: "Weight of the diffusive term. The higher the weight, the more diffusive the method is.\n\nDiffusion must be applied, therefore the weight must be greater than 0."
					objName: "diffusiveWeight"
					defaultInput: "1"
					decimals: true
					negative: false
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
					negative: false
				}

				LabelInput {
					id: numIsocontourLines
					text: "Number of lines"
					placeholderText: "Enter a number"
					toolTipText: "Number of isocontour lines to draw. 0 to disable."
					objName: "numIsocontourLines"
					defaultInput: "10"
					decimals: false
					negative: false
				}

				ComboBox {
					id: isocontourColor
					objectName: "isocontourColor"
					width: parent.width
					currentIndex: 0
					model: [
						"5 colors",
						"2 colors",
						"black and white"
					]
				}

				Button {
					text: "Redraw"
					onClicked: {
						actions.redrawIsocontourLines(isocontourSize.input, numIsocontourLines.input, isocontourColor.currentIndex);
					}
				}
			}
		}
	}
}

