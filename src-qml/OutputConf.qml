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
					text: "Number of lines"
					placeholderText: "Enter a number"
					toolTipText: "Number of isocontour lines to draw. 0 to disable."
					objName: "numIsocontourLines"
					defaultInput: "10"
					decimals: false
				}

				Button {
					text: "Redraw"
				}
			}
		}

	}
}

