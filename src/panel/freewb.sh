#!/bin/bash

if [ "$1"=="UOS" ]; then
	export LD_LIBRARY_PATH=/opt/apps/org.freewb.freeime/files/lib
	export QT_PLUGIN_PATH=/opt/apps/org.freewb.freeime/files/plugins

	if [ ! -d $HOME/.local/freewb ];then
	mkdir $HOME/.local/freewb -p
	fi

	if [ ! -d $HOME/.local/freewb/config ];then
	cp /opt/apps/org.freewb.freeime/files/config $HOME/.local/freewb/ -r
	fi

	if [ ! -d $HOME/.local/freewb/data ];then
	cp /opt/apps/org.freewb.freeime/files/data $HOME/.local/freewb/ -r
	fi

	if [ ! -d $HOME/.local/freewb/skin ];then
	cp /opt/apps/org.freewb.freeime/files/skin $HOME/.local/freewb/ -r
	fi

	if [ ! -d $HOME/.local/freewb/help ]; then
	cp /opt/apps/org.freewb.freeime/files/help $HOME/.local/freewb/ -r
	fi

	if [ "$1" == "poweron" ]; then
		#killall Freewb > /dev/null 2>&1
		FCITX=`ps aux | grep fcitx`
		if [[ ! $FCITX =~ "fcitx-dbus-watcher" ]]; then
				fcitx
		fi
	#elif [ ! -f $HOME/.config/autostart/fcitx-freewb-panel.desktop ]; then
	#	cp /usr/share/freewb/so/fcitx-freewb-panel.desktop $HOME/.config/autostart/fcitx-freewb-panel.desktop
	fi

	if [ "$1" != "poweron" ]; then
		sed -i "s/[#,]SkinType[^,]*/SkinType=dark/" $HOME/.config/fcitx/conf/fcitx-classic-ui.config
	fi
else
	export LD_LIBRARY_PATH=/usr/share/freewb/lib
	export QT_PLUGIN_PATH=/usr/share/freewb/plugins

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

	if [ "$1" == "poweron" ]; then
		#killall Freewb > /dev/null 2>&1
		FCITX=`ps aux | grep fcitx`
		if [[ ! $FCITX =~ "fcitx-dbus-watcher" ]]; then
				fcitx
		fi
	#elif [ ! -f $HOME/.config/autostart/fcitx-freewb-panel.desktop ]; then
	#	cp /usr/share/freewb/so/fcitx-freewb-panel.desktop $HOME/.config/autostart/fcitx-freewb-panel.desktop
	fi

	if [ "$1" != "poweron" ]; then
		sed -i "s/[#,]SkinType[^,]*/SkinType=dark/" $HOME/.config/fcitx/conf/fcitx-classic-ui.config
	fi
fi
Freewb
