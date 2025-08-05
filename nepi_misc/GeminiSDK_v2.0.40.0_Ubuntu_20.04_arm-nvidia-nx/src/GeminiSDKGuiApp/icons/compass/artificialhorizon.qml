import QtQuick 2.5

Rectangle {
    id: mainWindow
    width: 270
    height: 230
    color: "#a6c3dd" //sky blue (top color of Artificial Horizon image)

    property double hdgVal: 0
    property double pitchVal: 0
    property double rollVal: 0
    property color colText: "white"         // text colour
    property color colCardinal: "yellow"    // cardinal direction text colour
    property color colBackground: "black"   // background widget colour
    property color colNeedle: "#22b14c"     // green compass needle

    property double tickOffset: width * 0.40    // tick radius percent from centre
    property double textOffset: width * 0.31    // offset from centre
    property bool redrawCanvas: true

    Image {
        id: imageHorizon
        width: parent.width
        height: width
        y: -10
        source: "ArtificialHorizon.png"
        transform: [
            Translate{y:pitchVal*1.45}, //e.g. *1.45 to scale to image
            Rotation{origin.x:width/2; origin.y:height/2; angle:-rollVal}
        ]
    }

    Rectangle {
        id: rotationBox
        width: parent.width
        height: width
        y: -10
        color: "transparent"
        rotation: -hdgVal       // apply the heading rotation to this parent rectangle
        antialiasing: true

        // draw a circle that is the compass background
        Rectangle {
            anchors.centerIn: parent
            width: parent.width * 1.5
            height: width
            color: "transparent"
            border.color: colBackground
            border.width: parent.width * 0.5
            radius: width
        }

        // cardinal directions
        Repeater {
            model: ["N", "E", "S", "W"]
            width: parent.width
            height: parent.height

            Rectangle{
                anchors.centerIn: parent
                anchors.alignWhenCentered: false
                width: 1
                height: 1
                color: "transparent"
                transform: [
                    Translate{y:-textOffset},
                    Rotation{angle:index * 90}
                ]

                Text {
                    text: modelData
                    font.pointSize: 14
                    color: colCardinal
                    anchors.centerIn: parent
                }
            }
        }

        // angle text
        Repeater {
            model: [30, 60, 120, 150, 210, 240, 300, 330]
            width: parent.width
            height: parent.height

            Rectangle{
                anchors.centerIn: parent
                anchors.alignWhenCentered: false
                width: 1
                height: 1
                color: "transparent"
                transform: [
                    Translate{y:-textOffset},
                    Rotation{angle:modelData}
                ]

                Text {
                    text: modelData
                    font.pointSize: 12
                    font.bold: true
                    color: colText
                    anchors.centerIn: parent
                }
            }
        }

        // large compass ticks
        Repeater {
            model: 36
            width: parent.width
            height: parent.height

            // create a rectangle for text positioning
            Rectangle {
                anchors.centerIn: parent
                anchors.alignWhenCentered: false
                width: 1
                height: 1
                color: "transparent"
                transform: [
                    Translate{y:-tickOffset},
                    Rotation{angle:index * 10}
                ]

                // write a vertical line for 10º ticks
                Text {
                    text: "|"
                    font.pointSize: 14
                    font.bold: true
                    color: colText
                    anchors.centerIn: parent
                }
            }
        }

        // small compass ticks
        Repeater {
            model: 36
            width: parent.width
            height: parent.height

            // create a rectangle for text positioning
            Rectangle {
                anchors.centerIn: parent
                anchors.alignWhenCentered: false
                width: 1
                height: 1
                color: "transparent"
                transform: [
                    Translate{y:-tickOffset + 9},
                    Rotation{angle:index * 10 + 5}
                ]

                // write a block for 5º ticks
                Text {
                    text: "■"
                    font.pointSize: 4
                    color: colText
                    anchors.centerIn: parent
                }
            }
        }
    }

    // draw horizon overlay and compass needle
    Canvas
    {
        id: overlays
        width: parent.width
        height: width
        y: -10

        property double clipRadius: (width * 0.25)

        onPaint:
        {
            var ctx = getContext("2d")
            ctx.save()

            // draw centre horizon overlay
            ctx.lineWidth = 3;
            ctx.strokeStyle = "green"
            ctx.beginPath()
            ctx.moveTo(overlays.width / 2 - 50, overlays.height / 2 + (ctx.lineWidth / 2))
            ctx.lineTo(overlays.width / 2 - 20, overlays.height / 2 + (ctx.lineWidth / 2))
            ctx.lineTo(overlays.width / 2 - 20, overlays.height / 2 + (ctx.lineWidth / 2) + 5)

            ctx.moveTo(overlays.width / 2 + 50, overlays.height / 2 + (ctx.lineWidth / 2))
            ctx.lineTo(overlays.width / 2 + 20, overlays.height / 2 + (ctx.lineWidth / 2))
            ctx.lineTo(overlays.width / 2 + 20, overlays.height / 2 + (ctx.lineWidth / 2) + 5)
            ctx.stroke()

            // draw top needle triangle
            ctx.lineWidth = 3;
            ctx.strokeStyle = colNeedle
            ctx.fillStyle = colNeedle
            ctx.beginPath()
            ctx.moveTo(overlays.width / 2, overlays.height / 2 - tickOffset)
            ctx.lineTo(overlays.width / 2 - 5, overlays.height / 2 - tickOffset - 10)
            ctx.lineTo(overlays.width / 2 + 5, overlays.height / 2 - tickOffset - 10)
            ctx.closePath()
            ctx.fill()
            ctx.stroke()

            // draw bottom clipping region
            ctx.fillStyle = colBackground
            ctx.beginPath()
            ctx.moveTo(0, overlays.height)
            ctx.lineTo(0, overlays.height / 2)
            ctx.arc(overlays.width / 2, overlays.height / 2, clipRadius, Math.PI, Math.PI * 0.75, true)
            ctx.arc(overlays.width / 2, overlays.height / 2, clipRadius, Math.PI * 0.25, 0, true)
            ctx.lineTo(overlays.width, overlays.height / 2)
            ctx.lineTo(overlays.width, overlays.height)
            ctx.closePath()
            ctx.fill()

            ctx.restore()

            redrawCanvas = false
        }
    }

    // Required: redraw when the background colour changes or forced redraw
    onColNeedleChanged: overlays.requestPaint()
    onColBackgroundChanged: overlays.requestPaint()
    onRedrawCanvasChanged: overlays.requestPaint()
}


