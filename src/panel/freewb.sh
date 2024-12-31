#!/bin/bash
if [ ! -d $HOME/.local/freewb ];then
mkdir $HOME/.local/freewb -p
fi

if [ ! -d $HOME/.local/freewb/config ];then
cp /usr/share/freewb/config $HOME/.local/freewb/ -r
fi

if [ ! -d $HOME/.local/freewb/data ];then
cp /usr/share/freewb/data $HOME/.local/freewb/ -r
fi

if [ ! -d $HOME/.local/freewb/skin ];then
cp /usr/share/freewb/skin $HOME/.local/freewb/ -r
fi

if [ ! -d $HOME/.local/freewb/help ]; then
cp /usr/share/freewb/help $HOME/.local/freewb/ -r
fi

if [ "$1" != "poweron" ]; then
	sed -i "s/[#,]SkinType[^,]*/SkinType=dark/" $HOME/.config/fcitx/conf/fcitx-classic-ui.config
fi

# run freewb
Freewb
