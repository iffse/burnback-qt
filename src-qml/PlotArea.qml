import QtQuick 2.15
import QtQuick.Controls 2.12

Item {
	GroupBox {
		id: preview
		title: qsTr("Preview")
		anchors.fill: parent
		ScrollView {
			anchors.fill: parent
			clip: true
			contentHeight: canvas.height
			contentWidth: canvas.width

			Canvas {
				id: canvas
				anchors.fill: parent
				height: 2000
				width: 2000

				function paint(list) {
					var ctx = getContext("2d");
					ctx.lineWidth = 0.2;
					for	(var i = 0; i < list.length; i+=4) {
						ctx.moveTo(500 + list[i], 1000 - list[i+1]);
						ctx.lineTo(500 + list[i+2], 1000 - list[i+3]);
					}
					ctx.stroke();
					canvas.requestPaint()
				}

				Connections {
					target: actions
					onPaintCanvas: {
						canvas.paint(list)
					}
				}
			}
		}
	}
}
