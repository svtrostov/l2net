/******************************************************************************************
L2Net - Layer 2 Network Control Software
Файл заголовков:
Объявление глобальных переменных, структур, массивов, функций
Copyright (с) Stanislav V. Tretyakov, svtrostov@yandex.ru
******************************************************************************************/

#ifndef _L2NET_MAIN_H
#define _L2NET_MAIN_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pwd.h>
#include <signal.h>
#include <paths.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <netinet/if_ether.h>
#include <netinet/ether.h>
#include <pcap.h>
#include <libnet.h>
#include <pthread.h>
#include <mysql/mysql.h>

#include "sys_memory.h"
#include "sys_utils.h"
#include "sys_socket.h"

/*--------------------------------------------------------
К О Н С Т А Н Т Ы
--------------------------------------------------------*/

#define CNT_MAXLINE				4096U							//условно максимальная длина текстовой строки

#define CNT_FAKE_MAC_MASK		"de:ad:xx:xx:xx:xx"				//маска для генерации ложного MAC адреса (символы х заменяются на случайные значения)

#define CNT_CONFIG_FILE 		"./l2net.conf"						//путь к конфигурационному файлу с настройками доступа к базе данных






/*--------------------------------------------------------
С Т Р У К Т У Р Ы
--------------------------------------------------------*/

/*
Стадии функционирования программы
*/
typedef enum {
	SSS_STOPPED 		= 1,	//сервер остановлен
	SSS_LOADING_DATA 	= 2,	//Идет загрузка данных
	SSS_INITIALIZING 	= 3,	//сервер в процессе инициализации
	SSS_WORKING		 	= 4		//сервер запущен и работает
} SYS_STATE;



/*
Типы переменных:
Применяется в mod_config.c в массиве определений переменных конфигурации программы
*/
enum _variable_type {
	VT_NULL 	= 0,	//NULL
	VT_BOOL 	= 1,	//Булево значение
	VT_INT  	= 2,	//Целое число
	VT_UINT 	= 3,	//Целое положительное число
	VT_DOUBLE 	= 4,	//Вещественное число
	VT_TEXT 	= 5		//Текстовое значение
};



/*Структура определения конфигурационной переменной*/
struct _config_variable{
	char * 				var_name;		//Текстовое обозначение (название) переменной в файле конфигурации
	unsigned int 		var_crc;		//CRC названия переменной - числовое значение, рассчитываемое автоматически из названия переменной
	enum _variable_type var_type; 		//Тип переменной, задается значением из enum _variable_type
	char *	 			var_default;	//Значение по-умолчанию для данной переменной конфигурации
	void *				set_for;		//Указатель на адрес в памяти, в который будет записано значение переменной, полученное из файла конфигурации
	bool 				required;		//Признак, обязательно ли получить значение указанной переменной из конфигурационного файла
	bool 				found;			//Признак, если значение перемнной было найдено, устанавливается в True 
};



/*Структура элемента списка типа ARP пара MAC->IP*/
typedef struct _item_macip{
	struct ether_addr 		mac; 	//MAC адрес пары
	unsigned int			ip_from;//IP адрес пары - начиная от IP
	unsigned int			ip_to;	//IP адрес пары - заканчивая IP
	struct in_addr			ip;		//Текущий IP адрес пакета, вычисляется автоматически в функции f_arp_packet_check
	bool					deny;	//Признак, запрещено использование текущего сочетания MAC->IP
	struct _item_macip *	next;	//Следующая запись
} _item_macip;

typedef struct _list_macip{
	_item_macip * 	list;
	_item_macip * 	last;
	unsigned int  	count;
} _list_macip;



/*Структура заголовков ARP пакета*/
typedef struct _arp_packet{
	struct ether_addr	eth_mac_sender;
	struct ether_addr	eth_mac_target;
	struct ether_addr	arp_mac_sender;
	struct ether_addr	arp_mac_target;
	struct in_addr		arp_ip_sender;
	struct in_addr		arp_ip_target;
} _arp_packet;




/*--------------------------------------------------------
П Е Р Е М Е Н Н Ы Е - З Н А Ч Е Н И Я
--------------------------------------------------------*/

/*Системные*/
SYS_STATE			SYS_SERVER_STATE;	//Статус функционирования программы
_item_macip *		SYS_MACIP_INFO;	//Информация о MAC и IP адресе сервера, на котором запущен L2Net


/*Переменные программы, читаемые из таблицы базы данных CNT_DB_TABLE_CONFIG*/
char *				VAR_ETH_INTERFACE;		//сетевой интерфейс для работы с ARP пакетами (параметр eth_interface)
char *				VAR_FAKE_MAC_TEXT;		//ложный MAC адрес текстом (параметр fake_mac_addr)
int					VAR_FAKE_REPLY_COUNT;	//количество ложных ARP ответов, посылаемых несанкционированному отправителю ARP пакетов (параметр fake_reply_count)
int					VAR_FAKE_REPLY_TIMEO;	//таймаут в миллисекудах между отправками ARP ответов (параметр fake_reply_timeo)
bool				VAR_DUPLEX_CHECK_MODE;	//признак, режим двойственной проверки при котором проверяются адреса отправителя и получателя ARP пакетов (параметр duplex_mode)
bool				VAR_SILENT_MODE;		//признак, режим "тишины", в этом режиме программа не проводит атаку на неизвестные пары MAC->IP (параметр silent_mode)
bool				VAR_FIX_BROADCCAST;		//признак, после обнаружения несанкционированных ARP пакетов сделать рассылку корректных ARP пакетов (параметр fix_broadcast)


/*--------------------------------------------------------
П Е Р Е М Е Н Н Ы Е - О Б Ъ Е К Т Ы
--------------------------------------------------------*/

/*DB*/
MYSQL * _var_db_client; //переменная объекта базы данных


/*Списки*/
_list_macip * VAR_ARP_PAIRS; //переменная списка пар MAC->IP





/*--------------------------------------------------------
Ф У Н К Ц И И
--------------------------------------------------------*/


/*main.c - Головной модуль -------------------------------------------------------------------------------*/
int		main( int, char ** ); //стартовая функция
void	f_main_init(void); //Инициализация глобальных переменных


/*mod_vars.c - Модуль работы с переменными ---------------------------------------------------------------*/
void	f_var_init( struct _config_variable * );
void	f_var_set_from_crc(struct _config_variable *, unsigned int, char *);
void	f_var_set(struct _config_variable *, char *, char *);
void	f_var_set_for(struct _config_variable *, char *);
bool	f_var_check(struct _config_variable *);
char *	f_var_type_to_text(enum _variable_type var_type);


/*mod_config.c - настройки сервера -----------------------------------------------------------------------*/
void	f_config_init( void ); //Инициализация глобальных переменных
bool	f_config_read( void ); //Функция загрузки настроек
bool	f_config_check(void); //Функция проверки глобальных переменных


/*mod_db.c - работа с базой данных -----------------------------------------------------------------------*/
void	f_db_init(void);		//Создание соединения
bool	f_db_check(void);		//Проверка соединения с базой данных
void	f_db_free(void);		//Закрытие соединения
void	f_db_load_data(void);	//Загрузка данных из базы данных
void	f_db_options_load(void);//Загрузка настроек
bool	f_db_options_check(void);//Загрузка настроек


/*mod_arp.c - работа с ARP -------------------------------------------------------------------------------*/
char * 	f_arp_rand_mac(const char *); //Генерация ложного MAC адреса
char *	f_arp_lookup_eth(void); //Функция ищет сетевой интерфейс, используемый по-умолчанию и возвращает указатель на его название
void	f_arp_init(void); //Инициализация интерфейса прослушивания ARP пакетов
void 	f_arp_packet_recv(void); //Функция получения ARP пакетов
void	f_arp_packet_handle(char *, struct pcap_pkthdr *, u_char *); //Функция обработки пролученных ARP пакетов
void 	f_arp_packet_check(void); //Проверка ARP пакета
void 	f_arp_packet_send(int, int); //Отправка ARP пакета
void 	f_arp_servinfo_init(void);//Получение информации о MAC->IP адресе сервера


/*mod_list.c - работа со списками ------------------------------------------------------------------------*/
_list_macip *	f_list_macip_create(void); //Создать список _list_macip
_item_macip *	f_list_macip_item_create(void); //Создать пустую структуру _item_macip
void			f_list_macip_item_free(_item_macip *); //Освободить структуру _item_macip
void			f_list_macip_flush(_list_macip *); //Обнулить список
void			f_list_macip_free(_list_macip *); //Удалить список
_item_macip *	f_list_macip_add(_list_macip *, struct ether_addr, unsigned int, unsigned int, bool); //Добавление элемента в список



#endif /* _L2NET_MAIN_H */



