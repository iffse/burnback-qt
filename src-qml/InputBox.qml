import QtQuick 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15

ScrollView {
	id: scroller
	clip: true
	Column {
		id: column
		spacing: 10
		width: parent.width

		GroupBox {
			id: groupBox
			title: qsTr("Mesh data")
			width: parent.width

			Column {
				id: row2
				width: parent.width

				Label {
					id: label2
					text: qsTr("Click \"import\" and select a mesh file (*.dat)")
					width: parent.width
					wrapMode: Text.Wrap
				}

				Button {
					id: importMesh
					text: qsTr("Import")
				}
			}
		}

		GroupBox {
			width: parent.width
			visible: true
			id: model
			title: qsTr("Model data")

			Column {
				id: column1

				Label {
					id: label
					text: qsTr("Number of segments")
					property string toolTipText: "Number of segments used to draw the geometry of the solution"
					ToolTip.text: toolTipText
					ToolTip.visible: toolTipText ? mouseArea.containsMouse : false
					ToolTip.delay: 500

					MouseArea {
						id: mouseArea
						anchors.fill: parent
						hoverEnabled: true
					}
				}

				TextField {
					id: textField1
					placeholderText: qsTr("Enter a number")
					selectByMouse: true
					validator: DoubleValidator {
						bottom: 0
						top: 999999999
						decimals: 9
					}
				}

				CheckBox {
					id: axisymmetric
					text: qsTr("Axisymmetric")
					ToolTip.text: qsTr("Mark this checkbox if the model considered is axisymmetric")
					ToolTip.visible: hovered
					ToolTip.delay: 500
					hoverEnabled: true
				}

				CheckBox {
					id: resume
					text: qsTr("Resume")
					ToolTip.text: qsTr("Mark this checkbox if the calculation is a continuation of a previous computation")
					ToolTip.visible: hovered
					ToolTip.delay: 500
					hoverEnabled: true
				}
			}
		}

		GroupBox {
			id: calculation
			title: qsTr("Computation data")
			width: parent.width

			Column {
				id: column2

				Label {
					id: label1
					text: qsTr("CFL")
					property string toolTipText: "Adimensional time. (CFL = c * dt / dx = Speed of sound / Element size)"
					ToolTip.text: toolTipText
					ToolTip.visible: toolTipText ? cflMA.containsMouse : false
					ToolTip.delay: 500

					MouseArea {
						id: cflMA
						anchors.fill: parent
						hoverEnabled: true
					}
				}

				TextField {
					id: textField
					placeholderText: qsTr("Enter a number")
					selectByMouse: true
					validator: DoubleValidator {
						bottom: 0
						top: 999999999
						decimals: 9
					}
				}

				Label {
					id: label3
					text: qsTr("Minimum iterations")
					property string toolTipText: "Minimum number of iterations for the computation"
					ToolTip.text: toolTipText
					ToolTip.visible: toolTipText ? minIterMA.containsMouse : false
					ToolTip.delay: 500

					MouseArea {
						id: minIterMA
						anchors.fill: parent
						hoverEnabled: true
					}
				}

				TextField {
					id: textField2
					placeholderText: qsTr("Enter a number")
					selectByMouse: true
					validator: DoubleValidator {
						bottom: 0
						top: 999999999
						decimals: 9
					}
				}

				Label {
					id: label4
					text: qsTr("Maximum iterations")
					property string toolTipText: "Maximum number of iterations for the computation"
					ToolTip.text: toolTipText
					ToolTip.visible: toolTipText ? maxIterMA.containsMouse : false
					ToolTip.delay: 500

					MouseArea {
						id: maxIterMA
						anchors.fill: parent
						hoverEnabled: true
					}
				}

				TextField {
					id: textField3
					placeholderText: qsTr("Enter a number")
					selectByMouse: true
					validator: DoubleValidator {
						bottom: 0
						top: 999999999
						decimals: 9
					}
				}

				Label {
					id: label5
					text: qsTr("Admitted error")
					property string toolTipText: "Maximum error admitted for the computation"
					ToolTip.text: toolTipText
					ToolTip.visible: toolTipText ? maxErrMA.containsMouse : false
					ToolTip.delay: 500
					MouseArea {
						id: maxErrMA
						anchors.fill: parent
						hoverEnabled: true
					}
				}

				TextField {
					id: textField4
					placeholderText: qsTr("Enter a number")
					selectByMouse: true
					validator: DoubleValidator {
						bottom: 0
						top: 999999999
						decimals: 9
					}
				}

				Label {
					id: label6
					text: qsTr("Artificial viscosity coeficient")
					property string toolTipText: "Viscosity added to the solution to obtain smooth solutions across discontinuities"
					ToolTip.text: toolTipText
					ToolTip.visible: toolTipText ? viscMA.containsMouse : false
					ToolTip.delay: 500
					MouseArea {
						id: viscMA
						anchors.fill: parent
						hoverEnabled: true
					}
				}

				TextField {
					id: textField5
					placeholderText: qsTr("Enter a number")
					selectByMouse: true
					validator: DoubleValidator {
						bottom: 0
						top: 999999999
						decimals: 9
					}
				}
			}
		}

	}
}
