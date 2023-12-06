import QtQuick 2.12
import QtQuick.Layouts 1.1
import QtQml.Models 2.1
import "../../CommonItems" as Common

Item {
    default property alias controls: delegateRow.data
    property int paddingVert: 10;
    property int paddingHoriz: 16;
    property string sectionTitle

    width: ListView.view ? ListView.view.width : implicitWidth
    implicitHeight: delegateRow.implicitHeight + paddingVert * 2

    RowLayout {
        id: delegateRow
        anchors {
            left: parent.left
            leftMargin: paddingHoriz
            right: parent.right
            rightMargin: paddingHoriz
            top: parent.top
            topMargin: paddingVert
        }
    }
}
