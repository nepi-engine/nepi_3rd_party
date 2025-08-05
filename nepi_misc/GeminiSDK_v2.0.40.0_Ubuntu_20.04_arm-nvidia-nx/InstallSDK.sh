#!/bin/sh

# This script would uninstall previous version of GeminiSDK and install a new version of the SDK

CURRENT_BIN_PATH=$(pwd)/bin
INSTALL_FILES="libDataProcessor libGeminiComms libSvs5SeqLib libGenesisSerializer libavcodec libavutil"
CURRENT_FONT_PATH=$(pwd)/fonts
NEW_FONT_PATH=$CURRENT_BIN_PATH/fonts
FONT_FILES="arial.ttf"

if [ ! -d $CURRENT_BIN_PATH ]
then
    echo "$CURRENT_BIN_PATH does not exist !!!"
    echo "Please contact Tritech customer support team.."
    exit 1;
fi

if [ ! -d $CURRENT_FONT_PATH ]
then
    echo "$CURRENT_FONT_PATH does not exist !!!"
    echo "Please contact Tritech customer support team.."
    exit 1;
fi

if [ ! -d $NEW_FONT_PATH ]
then
    echo "$NEW_FONT_PATH does not exist, creating it."
    mkdir $NEW_FONT_PATH
fi

for i in $INSTALL_FILES
do
    [ -f /usr/local/lib/$i.so ] && sudo rm /usr/local/lib/$i.so*
    [ -f /usr/lib/$i.so ] && sudo rm /usr/lib/$i.so* 
done

for i in $INSTALL_FILES
do
    [ -f $CURRENT_BIN_PATH/$i.so ] && sudo cp -P $CURRENT_BIN_PATH/$i.so* /usr/local/lib/
done

# Update links 
sudo ldconfig

#copy the font files
for i in $FONT_FILES
do
    [ -f $CURRENT_FONT_PATH/$i ] && sudo cp -p $CURRENT_FONT_PATH/$i $NEW_FONT_PATH/
done