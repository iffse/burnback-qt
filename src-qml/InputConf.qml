import QtQuick 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3
import QtQml 2.0

ScrollView {
	clip: true
	Column {
		spacing: 10
		width: parent.width

		GroupBox {
			title: qsTr("Mesh data")
			width: parent.width

			Column {
				width: parent.width

				Label {
					id: meshLabel
					text: qsTr("Click \"import\" and select a mesh file (*.json)")
					width: parent.width
					wrapMode: Text.Wrap
				}

				Button {
					text: qsTr("Import")
					onClicked: fileDialog.open()
				}
				FileDialog {
					id: fileDialog
					objectName: "fileDialog"
					selectExisting: true
					selectFolder: false
					folder: ""
					nameFilters: ["Mesh files (*.json *.dat)", "All files (*)"]
					onAccepted: {
						meshLabel.text = ("Current selection:\n" + basename(fileUrl.toString()))
						root.inputUrl = fileUrl
					}
					function basename(path) {
						path.slice(path.lastIndexOf("\\") + 1)
						return path.slice(path.lastIndexOf("/") + 1)
					}
				}
			}
		}

		GroupBox {
			width: parent.width
			visible: true
			title: qsTr("Model data")

			Column {
				width: parent.width

				LabelInput {
					text: "Number of areas"
					placeholderText: "Enter a number"
					toolTipText: "Influence the number of segments used to draw the geometry of the solution"
					objName: "areas"
					decimals: true
				}

				LabelInput {
					text: "Initial condition"
					placeholderText: "Enter a number"
					toolTipText: "The initial value used for the model"
					objName: "initialCondition"
					defaultInput: "0"
					decimals: true
				}

				CheckBox {
					objectName: "axisymmetric"
					text: qsTr("Axisymmetric")
					ToolTip.text: qsTr("Mark this checkbox if the model considered is axisymmetric")
					ToolTip.visible: hovered
					ToolTip.delay: 500
					hoverEnabled: true
				}
			}
		}

		GroupBox {
			title: qsTr("Computation data")
				width: parent.width

			Column {
				width: parent.width
				spacing: 5

				LabelInput {
					text: "CFL"
					placeholderText: "Enter a number"
					toolTipText: "Adimensional time. (CFL = c * dt / dx = Speed of sound / Element size)"
					objName: "cfl"
					decimals: true
				}

				LabelInput {
					text: "Minimum iterations"
					placeholderText: "Enter a number"
					toolTipText: "Minimum number of iterations for the computation"
					objName: "minIter"
				}

				LabelInput {
					text: "Maximum iterations"
					placeholderText: "Enter a number"
					toolTipText: "Maximum number of iterations for the computation"
					objName: "maxIter"
				}

				LabelInput {
					text: "Tolerance"
					placeholderText: "Enter a number"
					toolTipText: "Maximum error admitted for the computation"
					objName: "tolerance"
					decimals: true
				}
			}
		}

	}
}
