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
if [ -f $fileRedirect ] && ! grep -qF "$LinkString" $fileRedirect
then
	echo $LinkString >> $fileRedirect
 	echo $LinkString2 >> $fileRedirect 
else 
	touch $fileRedirect 
	echo $LinkString >> $fileRedirect
 	echo $LinkString2 >> $fileRedirect
fi

#Запуск Программного обеспечения
IFS=$'\n'
exsist=$(ls -1A | grep -iEc ".*QKD.*") 
folder=( $(ls -1A | grep -iE ".*QKD.*") )
files=( $(ls -1A | grep -iE ".*QKD.*" | grep -E  "") )

if [ "$exsist" -lt 1 ]; #Файлы не найден
then 
	echo "QKD файлов нет в данном каталоге"
elif [ "$exsist" -gt 1 ]; #Если найдено больше 1 файла
then
	while true; 
	do 
		echo "Выберите файл для открытия:"
		ls "${files[@]}" | grep -nE  "" #Вывод пронумерованных файлов 
		
		read -p "Введите номер файла >" number
		
		if [ "$number" == "q" ] 
		then
			break;
		elif [ "$number" -gt 0 ] && [ "$number" -le "$exsist" ] ; 
		then
 			$(./${files[$number-1]})
 			break;
 		else 
 			echo -e "\n!!!>> Введен некорректный номер файла, попробуйте ещё раз. <<!!!\n"
 			continue;
 		fi
 	done
else #Найден один файл
	$(./${files[0]})
fi

