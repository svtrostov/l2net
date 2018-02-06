/******************************************************************************************
L2Net - Layer 2 Network Control Software
Модуль работы со списками
Copyright (с) Stanislav V. Tretyakov, svtrostov@yandex.ru
******************************************************************************************/

#include "main.h"


/*----------------------------------------------------
Функции работы со списками пар MAC->IP
----------------------------------------------------*/

/*Создать список _list_macip*/
_list_macip *
f_list_macip_create(void){
	_list_macip * list = (_list_macip *)sm_calloc(1, sizeof(_list_macip));
	if(!list) return NULL;
	list->list	= NULL;
	list->last	= NULL;
	list->count	= 0;
	return list;
}


/*Создать пустую структуру _item_macip*/
_item_macip *
f_list_macip_item_create(void){
	_item_macip * item = (_item_macip *)sm_calloc(1, sizeof(_item_macip));
	if(!item) return NULL;
	item->next  = NULL;
	return item;
}


/*Освободить структуру _item_macip*/
void
f_list_macip_item_free(_item_macip * item){
	if(!item) return;
	sm_free(item);
	return;
}


/*Обнулить список*/
void
f_list_macip_flush(_list_macip * list){
	if(!list) return;
	if(!list->list){
		list->list	= NULL;
		list->last	= NULL;
		list->count	= 0;
		return;
	}
	_item_macip * 	item;
	_item_macip * 	item_next = list->list;
	while(item_next){
		item = item_next;
		item_next = item->next;
		//Освобождение элемента списка
		sm_free(item);
	}
	//Обнуление значений списка
	list->list	= NULL;
	list->last	= NULL;
	list->count	= 0;
	return;
}


/*Удалить список*/
void
f_list_macip_free(_list_macip * list){
	if(!list) return;
	f_list_macip_flush(list);
	sm_free(list);
	return;
}


/*Добавление элемента в список, 
возвращает указатель на структкру _item_macip в случае успеха,
либо NULL если запись не удалось добавить*/
_item_macip *
f_list_macip_add(_list_macip * list, struct ether_addr mac, unsigned int ip_from, unsigned int ip_to, bool deny){

	if(!list) return NULL;

	//Создание структуры _item_pair элемента 
	_item_macip * item = (_item_macip *)sm_calloc(1, sizeof(_item_macip));
	if(!item) return NULL;

	memcpy(&item->mac, &mac, sizeof(struct ether_addr));
	item->ip_from	= ip_from;
	item->ip_to		= ip_to;
	item->deny 		= deny;
	item->next		= NULL;

	if(!list->list){
		list->list = item;
	}else{
		list->last->next = item;
	}
	list->last  = item;

	list->count++;

	return item;
}












