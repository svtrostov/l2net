/******************************************************************************************
L2Net - Layer 2 Network Control Software
Основной модуль:
Инициализация глобальных переменных, массивов данных, сетевых подключений и старт L2Net
Copyright (с) Stanislav V. Tretyakov, svtrostov@yandex.ru
******************************************************************************************/

#include "main.h"


/*Стартовая функция*/
int
main(int argc, char **argv){

	//Статус сервера - загрузка данных
	SYS_SERVER_STATE = SSS_LOADING_DATA;

	//Инициализация глобальных переменных
	f_main_init();

	//Инициализаци прослушивания ARP пакетов на сетевом интерфейсе
	f_arp_init();

	//Получение информации о MAC->IP адресе сервера
	f_arp_servinfo_init();

	//Начало обработки ARP
	while (1) f_arp_packet_recv();

	return(0);
}





/*Инициализация глобальных переменных*/
void
f_main_init( void ){

	//Загрука настроек из файла конфигурации
	f_config_init();

	//Создание списка пар MAC->IP
	VAR_ARP_PAIRS  = f_list_macip_create();

	return;
}







