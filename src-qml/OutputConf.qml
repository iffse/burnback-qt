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
					objectName: "diffusiveMethod"
					width: parent.width
					model: ["Abgrall", "Zhang and Shu"]
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
			}
		}

		GroupBox {
			title: qsTr("Isocontour lines")
			width: parent.width

			Column {
				width: parent.width

				LabelInput {
					text: "Size to draw (px)"
					placeholderText: "Enter a number"
					toolTipText: "Maximum size of the weight or height to draw the isocontour lines, in pixels."
					objName: "isocontourSize"
					defaultInput: "1000"
					decimals: false
				}

				LabelInput {
					text: "Number of lines"
					placeholderText: "Enter a number"
					toolTipText: "Number of isocontour lines to draw. 0 to disable."
					objName: "numIsocontourLines"
					defaultInput: "10"
					decimals: false
				}
			}
		}

	}
}

