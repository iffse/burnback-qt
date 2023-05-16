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
						actions.readMesh(fileUrl)
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
					toolTipText: "Number of areas used to graph the burning area\n\nLeave empty to not graph the burning area"
					objName: "areas"
					decimals: false
					negative: false
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

				CheckBox {
					objectName: "resume"
					text: qsTr("Resume")
					ToolTip.text: qsTr("Mark this checkbox if you want to resume a previous computation")
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
					toolTipText: "Non-dimensional time. The smaller the more accurate the solution but the longer the computation time\n\nUse a value between 0 and 0.5 (both non-inclusive) for a stable and monotonous solution\nException: Zhang and Shu's scheme is stable for CFL < 2.5 (empirical value) when the diffusive weight is 1. Resulting in a very fast computation with high precision"
					objName: "cfl"
					decimals: true
				}

				LabelInput {
					text: "Target iterations"
					placeholderText: "Enter a number"
					toolTipText: "Maximum number of iterations to perform"
					objName: "targetIter"
					negative: false
				}

				LabelInput {
					text: "Tolerance"
					placeholderText: "Enter a number"
					toolTipText: "Tolerance for the convergence of the solution.\nThe computation will stop before reaching the target iterations if the error of the solution at the end of the iteration is less than the tolerance\n\nLeave empty to disable this feature"
					objName: "tolerance"
					decimals: true
					negative: false
				}
			}
		}

	}
}
