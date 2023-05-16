import QtQuick 2.15
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.15

ScrollView {
	id: scroll
	clip: true
	contentHeight: canvas.height
	contentWidth: canvas.width

	Canvas {
		id: canvas
		objectName: "canvas"
		anchors.fill: parent
		height: 1000
		width: 1000

		Connections {
			target: actions
			function onPaintCanvas(list, color) {
				var ctx = canvas.getContext("2d");
				ctx.beginPath();
				ctx.lineWidth = 1;
				for (var i = 0; i < list.length; i+=4) {
					ctx.moveTo(list[i], canvas.height - list[i+1]);
					ctx.lineTo(list[i+2], canvas.height - list[i+3]);
				}

				if (Material.theme == Material.Dark) {
					if (color === "#000000") {
						color = "#ffffff";
					}
					else if (color.indexOf("hsl") == 0) {
						var hsl = color.split(",");
						console.log(hsl[1]);
						if (hsl[1] != " 0%") {
							hsl[2] = " 75%)";
						}
						color = hsl.join(",");
						console.log(color);
					}
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
				scroll.contentWidth = width
				scroll.contentHeight = height
			}
		}
	}

}
