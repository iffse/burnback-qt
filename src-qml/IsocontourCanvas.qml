import QtQuick 2.15
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.15

ScrollView {
	clip: true
	contentHeight: canvas.height
	contentWidth: canvas.width

	Canvas {
		id: canvas
		anchors.fill: parent
		height: 2000
		width: 2000

		Connections {
			target: actions
			function onPaintCanvas(list, color) {
				var ctx = canvas.getContext("2d");
				ctx.beginPath();
				ctx.lineWidth = 1;
				for (var i = 0; i < list.length; i+=4) {
					ctx.moveTo(500 + list[i], 1000 - list[i+1]);
					ctx.lineTo(500 + list[i+2], 1000 - list[i+3]);
				}

				if (Material.theme == Material.Dark && color === "#000000") {
					color = "#ffffff"
				}
				ctx.strokeStyle = color;
				ctx.stroke();
				canvas.requestPaint()
			}

			function onClearCanvas() {
				var ctx = canvas.getContext("2d");
				ctx.clearRect(0, 0, canvas.width, canvas.height);
				canvas.requestPaint()
			}

			function onSetCanvasSize(width, height) {
				canvas.width = width
				canvas.height = height
			}
		}
	}
}
