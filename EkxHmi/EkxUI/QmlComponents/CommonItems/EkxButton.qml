import QtQuick 2.0

MouseArea
{
    id: idRoot;

    enum ButtonType
    {
        NONE,
        OK,
        PREVIOUS,
        NEXT,
        CANCEL,
        CLOSE,
        APPLY,
        DELETE,
        ADD,
        EDIT,
        BACKSP,
        PLUS
    }

    property int buttonType: EkxButton.ButtonType.OK;
    property bool visualDebug: false;

    // LongPress not implemented yet.
    property bool enableLongpress: false;
    property color longPressFillColor: "green";
    signal longPressed;
    signal simplePressed;

    // look-ups:
    property var messages: (new Map([
                [EkxButton.ButtonType.OK,       "Ok"],
                [EkxButton.ButtonType.PREVIOUS, "Previous"],
                [EkxButton.ButtonType.NEXT,     "Next"],
                [EkxButton.ButtonType.CLOSE,    "Close"],
                [EkxButton.ButtonType.DELETE,   "Delete"],
                [EkxButton.ButtonType.BACKSP,   "Backspace"],
                [EkxButton.ButtonType.PLUS,     "Plus"],
                [EkxButton.ButtonType.EDIT,     "Edit"]
        ]));
    property var icons: (new Map([
                [EkxButton.ButtonType.OK,       "qrc:/Icons/ICON-Strokes_confirm-72px_white.png"],
                [EkxButton.ButtonType.PREVIOUS, "qrc:/Icons/ICON-Strokes_Back-72px_white.png"],
                [EkxButton.ButtonType.NEXT,     "qrc:/Icons/ICON-Strokes_Back-72px_white.png"], // mirrored "PREVIOUS"
                [EkxButton.ButtonType.CLOSE,    "qrc:/Icons/ICON-Strokes_Close-72px_white.png"],
                [EkxButton.ButtonType.DELETE,   "qrc:/Icons/ICON_Trashcan-72px_white.png"],
                [EkxButton.ButtonType.BACKSP,   "qrc:/Icons/ICON_Delete-72px_white.png"],
                [EkxButton.ButtonType.PLUS,     "qrc:/Icons/ICON-Strokes_Add-40px_white.png"],
                [EkxButton.ButtonType.EDIT,     "qrc:/Icons/ICON_Edit-72px_white.png"]
        ]));
    property var cornerLut: (new Map([
                [EkxButton.ButtonType.OK,       Qt.BottomRightCorner],
                [EkxButton.ButtonType.PREVIOUS, Qt.BottomLeftCorner],
                [EkxButton.ButtonType.NEXT,     Qt.BottomRightCorner],
                [EkxButton.ButtonType.CLOSE,    Qt.TopLeftCorner],
        ]));

    width: 72;
    height: 72;
    anchors.left:   ( cornerLut.get(buttonType)===Qt.TopLeftCorner    || cornerLut.get(buttonType)===Qt.BottomLeftCorner )? parent.left   : undefined;
    anchors.right:  ( cornerLut.get(buttonType)===Qt.TopRightCorner   || cornerLut.get(buttonType)===Qt.BottomRightCorner)? parent.right  : undefined;
    anchors.top:    ( cornerLut.get(buttonType)===Qt.TopLeftCorner    || cornerLut.get(buttonType)===Qt.TopRightCorner   )? parent.top    : undefined;
    anchors.bottom: ( cornerLut.get(buttonType)===Qt.BottomLeftCorner || cornerLut.get(buttonType)===Qt.BottomRightCorner)? parent.bottom : undefined;

    Image
    {
        anchors.fill: parent;
        source: idRoot.icons.get(idRoot.buttonType) || "";
        mirror: (idRoot.buttonType===EkxButton.ButtonType.NEXT)?true:false;
    }

    DebugFrame {visible: idRoot.visualDebug;}
}
