import QtQuick 2.5

Rectangle {
    id: mainWindow
    width: 270
    height: 250
    color: colBackground //sky blue (top color of Artificial Horizon image)

    property double hdgVal: 0
    property double pitchVal: 0
    property double rollVal: 0
    property color colText: "white"         // text colour
    property color colCardinal: "yellow"    // cardinal direction text colour
    property color colBackground: "black"   // background widget colour
    property color colNeedle: "red"     // red compass needle
    property color colCompass: "blue" //"#a6c3dd"   // colour of main display

    property int yOff: -10
    property int xOff: 30
    property int maxAngle: 20

    property double tickOffset: height * 0.36    // tick radius percent from centre
    property double textOffset: height * 0.42    // offset from centre
    property bool redrawCanvas: true

    Rectangle {
        id: spiritLevel
        width: parent.height
        height: width
        y: yOff
        x: xOff
        color: "transparent"
        antialiasing: true

        // Outer target circle, just blue inner
        Rectangle {
             width: parent.width / 1.6
             height: width
             color: colCompass
             border.color: "transparent"
             border.width: 1
             radius: width
             anchors.centerIn: parent
        }

        // Inner target circle
        Rectangle {
             width: parent.width / 4.5
             height: width
             color: "transparent"
             border.color: "grey"
             border.width: 1
             radius: width
             anchors.centerIn: parent
        }

        // Target horizontal line
        Rectangle {
            width: parent.width / 1.6
            height: 1
            color: "grey"
            anchors.centerIn: parent
        }

        // Target vertical line
        Rectangle {
            width: 1
            height: parent.width / 1.6
            color: "grey"
            anchors.centerIn: parent
        }

        Image {
                id: imageBubble
                width: parent.height / 6.5
                height: width
                source: "bubble.png"
                anchors.centerIn: parent
                transform: [
                            Translate{y:3.0 * Math.max(Math.min(pitchVal, maxAngle), -maxAngle); x: 3.0 * Math.max(Math.min(rollVal, maxAngle), -maxAngle)}
                ]
        }

    }

    Rectangle {
        id: rotationBox
        width: parent.height
        height: width
        y: yOff
        x: xOff
        color: "transparent"
        rotation: -hdgVal       // apply the heading rotation to this parent rectangle
        antialiasing: true
        // draw a hollow circle that is the compass background
        Rectangle {
            anchors.centerIn: parent
            width: parent.width * 0.9
            height: width
            color:"transparent"
            border.color: colCompass
            border.width: 38
            radius: width
         }

        // Outer target circle, just grey outer inner
        Rectangle {
            width: parent.width / 1.6
            height: width
            color: "transparent"
            border.color: "grey"
            border.width: 1
            radius: width
            anchors.centerIn: parent
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
                    font.pointSize: 12
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
                    font.pointSize: 10
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
                    font.pointSize: 6
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
                    Translate{y:-tickOffset + 5},
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

    Column
    {
        Text {
            text: "h: " + hdgVal.toFixed(1) + "\xB0"
            font.pointSize: 10
            color: colText
        }

        Text {
            text: "p: " + pitchVal.toFixed(1) + "\xB0"
            font.pointSize: 10
            color: colText
        }

        Text {
            text: "r: " + rollVal.toFixed(1) + "\xB0"
            font.pointSize: 10
            color: colText
        }
    }


    // draw horizon overlay and compass needle
    Canvas
    {
        id: overlays
        width: parent.height
        height: width
        y: yOff -1
        x: xOff

        property double clipRadius: (width * 0.25)

        onPaint:
        {
            var ctx = getContext("2d")
            ctx.save()



            // draw top needle triangle
            ctx.lineWidth = 3;
           ctx.strokeStyle = colNeedle
            ctx.fillStyle = colNeedle
            ctx.beginPath()
            ctx.moveTo(overlays.width / 2, overlays.height / 2 - tickOffset)
            ctx.lineTo(overlays.width / 2 - 2, overlays.height / 2 - tickOffset - 5)
            ctx.lineTo(overlays.width / 2 + 2, overlays.height / 2 - tickOffset - 5)
            ctx.closePath()
            ctx.fill()
            ctx.stroke()



            ctx.restore()

            redrawCanvas = false
        }
    }

    // Required: redraw when the background colour changes or forced redraw
    onColNeedleChanged: overlays.requestPaint()
    onColBackgroundChanged: overlays.requestPaint()
    onRedrawCanvasChanged: overlays.requestPaint()
}


