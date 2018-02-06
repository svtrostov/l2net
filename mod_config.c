/******************************************************************************************
L2Net - Layer 2 Network Control Software
Модуль чтения конфигурационного файла:
Функции загрузки настроек при старте программы
Copyright (с) Stanislav V. Tretyakov, svtrostov@yandex.ru
******************************************************************************************/

#include "main.h"

/*Массив значений переменных опций*/
static struct _config_variable config_variables[] = {
	/*{"var_name", var_crc, var_type, "var_default", set_for, required, found(always false!)}*/
	{"eth_interface", 	0, VT_TEXT, "AUTO", 	&VAR_ETH_INTERFACE, 	false, false},
	{"duplex_mode", 	0, VT_BOOL, "1", 		&VAR_DUPLEX_CHECK_MODE, false, false},
	{"fake_mac_addr", 	0, VT_TEXT, "AUTO", 	&VAR_FAKE_MAC_TEXT, 	false, false},
	{"silent_mode", 	0, VT_BOOL, "1", 		&VAR_SILENT_MODE, 		false, false},
	{"fake_reply_count",0, VT_INT, 	"3",		&VAR_FAKE_REPLY_COUNT, 	false, false},
	{"fake_reply_timeo",0, VT_INT, 	"50",		&VAR_FAKE_REPLY_TIMEO, 	false, false},
	{"fix_broadcast", 	0, VT_BOOL, "1", 		&VAR_FIX_BROADCCAST, 	false, false},
	{NULL,0,VT_NULL,NULL,false,false}
};





/*Инициализация чтения конфигурации*/
void
f_config_init(void){
	f_var_init(config_variables);
	if(!f_config_read()){
		printf("can not read %s\n", CNT_CONFIG_FILE);
		exit(1);
	}
	if(!f_config_check()){
		printf("config check error\n");
		exit(1);
	}
}


/*Функция загрузки настроек*/
bool
f_config_read(void){
	
	FILE * fd;
	char ch;
	char line[CNT_MAXLINE];
	char * value = NULL;
	bool delimer_found = false;
	int pos = 0;
	
	if( (fd = fopen(CNT_CONFIG_FILE, "r")) == NULL )return false;
	
	/*Чтение файла настроек*/
	while( fread( &ch, 1, 1, fd ) == 1 ){
		if(ch == 0x20 || ch == '\r' || ch == '\t') continue;
		
		/*найден разделитель*/
		if(ch == '='){
			line[pos]='\0';
			++pos;
			delimer_found = true;
			value = &line[pos];
			continue;
		}//найден разделитель
		
		
		/*найден конец строки*/
		if(ch == '\n'){
			line[pos] = 0;
			if(line[0] != '#' && (line[0]!='/' || line[1]!='/' ) && value != NULL && delimer_found == true){
				f_var_set(config_variables, line, value);
			}
			pos = 0;
			value = NULL;
			delimer_found = false;
			continue;
		}//найден конец строки
		
		line[pos] = ch;
		if(pos<CNT_MAXLINE-1) ++pos;
	}//Чтение файла настроек
	
	return true;
}




/*Проверка получения всех обязательный настроек из конфигурационного файла*/
bool
f_config_check(void){
	bool res = true;
	struct _config_variable * variable;
	for (variable = config_variables; variable->var_name != NULL; variable++){
		if(variable->found == false ){
			if(variable->required == true){
				printf("%s: required variable \"%s\" not found (value type: %s)\n",CNT_CONFIG_FILE,variable->var_name, f_var_type_to_text(variable->var_type));
				res=false;
			}
			if(variable->required == false && res == true){
				f_var_set_for(variable, variable->var_default);
			}
		}
	}
	return res;
}

