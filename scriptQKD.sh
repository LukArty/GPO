#!/bin/bash
fileFindMK='/usr/lib/udev/rules.d/85-brltty.rules'
fileRedirect='/etc/udev/rules.d/10-udev.rules'
pathSoft="./QKDStand"

searchString="ENV{PRODUCT}==\"1a86/7523/*\", ENV{BRLTTY_BRAILLE_DRIVER}=\"bm\", GOTO=\"brltty_usb_run\""

LinkString='KERNEL=="tty*", SUBSYSTEM=="tty", ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="7523", SYMLINK+="ttyStandQKD"'
LinkString2='KERNEL=="tty*", SUBSYSTEM=="tty", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6001", SYMLINK+="ttyStandQKD"'

 
 
 
while true; do
	read -p "Вы уверены, что хотите провести инициализацию[Y/N]: " answer 
	if ( echo "$answer" | grep -q '^[YyДд]\{1\}$\|^[YyДд][EeАа][Ss]' ) then break;
	elif (echo "$answer" | grep -q '^[NnНн]\{1\}$\|^[NnНн][OoЕе][Тт]' ) then exit;
	else echo "Дайте ответ Y или N" 
	fi
done

#Пак для техПО
sudo apt install libqt5printsupport5 -y

#Комент строки для отображения ttyUSB
if [ -f $fileFindMK ] && ! grep -qF "# $searchString" $fileFindMK
then
sed -i "s@ENV{PRODUCT}==\"1a86\/7523\/\*\", *ENV{BRLTTY_BRAILLE_DRIVER}=\"bm\", *GOTO=\"brltty_usb_run\"@# $searchString@g" $fileFindMK 
fi

#Добавление ссылки на порт подключения стенда
if [ -f $fileRedirect ] 
then 
touch $fileRedirect 
fi

if ! grep -qF "$LinkString" $fileRedirect 
then
	echo $LinkString >> $fileRedirect
fi

if ! grep -qF "$LinkString2" $fileRedirect
then
 	echo $LinkString2 >> $fileRedirect 
fi

#Перезагрузка
while true; do
	echo "Для корректной работы стенда, необходимо произвести перезагрузку компьютера."
read -p "	
Хотите перезагрузить? [Y/N]: " answer 
	if ( echo "$answer" | grep -q '^[YyДд]\{1\}$\|^[YyДд][EeАа][Ss]' ) then reboot;
	elif (echo "$answer" | grep -q '^[NnНн]\{1\}$\|^[NnНн][OoЕе][Тт]' ) then exit;
	else echo "Дайте ответ Y или N" 
	fi
done

