import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3

ApplicationWindow {
	id: recessionPanel
	width: 640
	height: 480
	visible: true
	title: qsTr("Recession Panel")

	RowLayout {
		anchors.fill: parent
		anchors.margins: 10
		spacing: 10

		GroupBox {
			Layout.fillWidth: true
			Layout.fillHeight: true
			title: qsTr("Recession velocity per node")

			ScrollView {
				id: scroll
				clip: true
				anchors.fill: parent
				Row {
					spacing: 10
					anchors.fill: parent
					TextArea {
						id: lineNumber
						width: { Math.min(contentWidth, parent.width/2 ) }
						color: "grey"
						readOnly: true

						function updateLineNumbers() {
							var lines = textArea.text.split("\n");
							var lineCount = lines.length;
							var text = "";
							for (var i = 1; i < lineCount; i++)
								text += i + "\n";
							lineNumber.text = text + lineCount;
						}
					}
					TextArea {
						id: textArea
						selectByMouse: true
						width: scroll.width - lineNumber.width - parent.spacing

						onTextChanged: {
							lineNumber.updateLineNumbers();
						}
						Component.onCompleted: {
							lineNumber.updateLineNumbers();
							textArea.text = actions.getRecession();
						}
					}
				}
			}
		}

		GroupBox {
			title: qsTr("Options")
			Layout.fillHeight: true

			Column {
				Button {
					text: qsTr("Cancel")
					onClicked: {
						recessionPanel.close();
					}
				}
				Button {
					text: qsTr("Load")
					onClicked: {
						fileDialogRecession.open();
					}
				}

				FileDialog {
					id: fileDialogRecession
					objectName: "fileDialogRecession"
					selectExisting: true
					selectFolder: false
					folder: ""
					nameFilters: ["text files (*.txt)", "All files (*)"]
					onAccepted: {
						var file = fileDialogRecession.fileUrl;
						if (file != "") {
							textArea.text = actions.getRecession(file);
						}
					}
				}

				Button {
					text: qsTr("Save")
					onClicked: {
						actions.updateRecessions(textArea.text, saveToFile.checked, savePretty.checked);
					}
				}
				CheckBox {
					id: saveToFile
					text: qsTr("Save to file")
				}

				CheckBox {
					id: savePretty
					text: qsTr("Save pretty")
					visible: saveToFile.checked
				}

			}
		}
	}
}
