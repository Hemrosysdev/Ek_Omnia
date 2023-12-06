import QtQuick 2.12
import "../../CommonItems"

Rectangle
{
    width: 480
    height: 360

    color: "black"

    property int grammageBig;
    property int grammageComma;
    property int grammageIcon;

    property int libraryIndex;
    property int grammageIndex;

    signal savedGrammageChanges();

    function setGrammageStartValues( currentLibraryRecipeIndex, currentGrammageIndex )
    {
        grammageIndex = currentGrammageIndex;
        libraryIndex = currentLibraryRecipeIndex;
        if( grammageIndex === 0 )
        {
            grammageBig =    recipeControl.installedLibraryList[libraryIndex].grammage1Big;
            grammageComma =  recipeControl.installedLibraryList[libraryIndex].grammage1Comma;
            grammageIcon =   recipeControl.installedLibraryList[libraryIndex].grammage1Icon;
        }
        else
        {
            grammageBig = recipeControl.installedLibraryList[libraryIndex].grammage2Big;
            grammageComma =  recipeControl.installedLibraryList[libraryIndex].grammage2Comma;
            grammageIcon =   recipeControl.installedLibraryList[libraryIndex].grammage2Icon;
        }

       bigValue.setStartValues(grammageBig);
       commaValue.setStartValues(grammageComma);
       iconWheel.setStartValues(grammageIcon);
    }

    ShadowGroup
    {
        z: 1;
        shadowTopSize: 135;
        shadowBottomSize: 135;
    }

    IconWheel
    {
        id: iconWheel
        anchors.left: parent.left
        anchors.leftMargin: 110

        onCurrentValue:
        {
            grammageIcon = value;
        }
    }

    SettingsNumberWheel
    {
        id: bigValue
        z:0

        anchors.left: parent.left
        anchors.leftMargin: 207

        currentFontFamiliy: "D-DIN Condensed HEMRO _Mono_Numbers"

        itemOffset: 0

        model: 99
        interactive: true

        onCurrentValue:
        {
            grammageBig = value;
        }
    }

    Text
    {
        id: point
        text:  "."
        color: idMain.colorEkxTextWhite

        font.pixelSize: 81
        font.family:    "D-DIN Condensed HEMRO _Mono_Numbers"

        anchors.left: bigValue.right
        anchors.leftMargin: 3
        anchors.top: parent.top
        anchors.topMargin: 135

    }


    SettingsNumberWheel
    {
        id: commaValue
        z:0

        itemWidth: 36

        anchors.left: point.right
        anchors.leftMargin: 3

        currentFontFamiliy: "D-DIN Condensed HEMRO _Mono_Numbers"

        model: 10
        interactive: true

        informAboutChange: true

        onCurrentValue:
        {
            grammageComma = value;
        }

        onUpChange:
        {
           bigValue.incrementCurrentIndex();
        }

        onDownChange:
        {
           bigValue.decrementCurrentIndex();
        }
    }

    Text
    {
        id: unit
        text:  "t"
        color: idMain.colorEkxTextWhite

        width: 36
        height: 90

        font.pixelSize: 81
        font.family:    "D-DIN Condensed HEMRO _Mono_Numbers"

        horizontalAlignment: Text.AlignHCenter

        anchors.left: parent.left
        anchors.leftMargin: 347
        anchors.top: parent.top
        anchors.topMargin: 135

    }

    EkxButton
    {
        // id: acceptIcon
        z:4
        buttonType: EkxButton.ButtonType.OK;
        onClicked:
        {
            if( grammageIndex === 0 )
            {
                recipeControl.installedLibraryList[libraryIndex].grammage1Big =  grammageBig;
                recipeControl.installedLibraryList[libraryIndex].grammage1Comma = grammageComma;
                recipeControl.installedLibraryList[libraryIndex].grammage1Icon = grammageIcon;
            }
            else
            {
                recipeControl.installedLibraryList[libraryIndex].grammage2Big = grammageBig;
                recipeControl.installedLibraryList[libraryIndex].grammage2Comma = grammageComma;
                recipeControl.installedLibraryList[libraryIndex].grammage2Icon = grammageIcon;
            }

            recipeControl.saveRecipes();
            savedGrammageChanges();
        }
    }
}
