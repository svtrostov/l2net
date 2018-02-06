/******************************************************************************************
L2Net - Layer 2 Network Control Software
Модуль работы с глобальными переменными программы
Copyright (с) Stanislav V. Tretyakov, svtrostov@yandex.ru
******************************************************************************************/

#include "main.h"


/*Инициализация массива переменных*/
void
f_var_init(struct _config_variable * vars){
	struct _config_variable * variable;
	for (variable = vars; variable->var_name; variable++){
		variable->var_crc = su_crc32(variable->var_name);
		variable->found = false;
	}
	return;
}




/*Присваивает значение переменной согласно ее типу*/
void
f_var_set_for(struct _config_variable * var, char * value ){
	
	
	if(value == NULL){
		var->set_for = NULL;
		return;
	}
	switch(var->var_type){
		case VT_BOOL:
			*(bool*)var->set_for = ( su_cmpun(value,"1",1) || su_cmpun(value,"true",4) || su_cmpun(value,"on",2) ? true : false);
		break;
		case VT_INT:
			*(int*)var->set_for = (atoi(value));
		break;
		case VT_UINT:
			*(unsigned int*)var->set_for = (atoll(value));
		break;
		case VT_DOUBLE:
			*(double*)var->set_for = (atof(value));
		break;
		case VT_TEXT:;
			char **t = var->set_for;
			*t = su_new(value,NULL);
		break;
		default:
			var->set_for = NULL;
	}
	return;
}




/*присваивает глобальной переменной соответствующее значение*/
void
f_var_set_from_crc(struct _config_variable * vars, unsigned int param_crc, char * value){
	
	struct _config_variable * variable;
	
	for (variable = vars; variable->var_name; variable++){
		if(variable->var_crc == param_crc){
			f_var_set_for(variable, value);
			variable->found = true;
			break;	
		}
	}
	
	return;
}




/*присваивает глобальной переменной соответствующее значение*/
void
f_var_set(struct _config_variable * vars, char * param, char * value){
	unsigned int param_crc = su_crc32(param);
	f_var_set_from_crc(vars, param_crc, value);
	return;
}




/*Проыеряет, все ли обязательные опции найдены, если нет - возвращает false
также присваивает значения по-умолчанию всем остальным ненайденным опциям*/
bool
f_var_check(struct _config_variable * vars){
	bool res = true;
	struct _config_variable * variable;
	for (variable = vars; variable->var_name; variable++){
		if(variable->found == false ){
			if(variable->required == true){
				res=false;
			}else{
				f_var_set_for(variable, variable->var_default);
			}
		}
	}
	return res;
}




/*Возвращает текстовое значение типа переменной*/
char *
f_var_type_to_text(enum _variable_type var_type){
	switch(var_type){
	case VT_BOOL: return "bool";
	case VT_INT: return "integer";
	case VT_UINT: return "unsigned integer";
	case VT_DOUBLE: return "double";
	case VT_TEXT: return "text";
	case VT_NULL: return "NULL";
	}
	return "undefined";
}


