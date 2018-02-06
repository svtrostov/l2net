/******************************************************************************************
Функции работы с памятью
Copyright (с) Stanislav V. Tretyakov, svtrostov@yandex.ru
******************************************************************************************/

#include "sys_memory.h"


/*----------------------------------------------------------------
Функции - Работа с памятью
----------------------------------------------------------------*/


/*Выделение памяти malloc*/
extern void *
sm_malloc(size_t size){
	if(size<32)size=32;
	return malloc(size);
}




/*Выделение памяти calloc*/
extern void *
sm_calloc(size_t n, size_t m){
	return calloc(n, m);
}




/*Изменение размера памяти realloc*/
extern void *
sm_realloc(void * ptr, size_t size){
	if(size<32)size=32;
	return (!ptr ? malloc(size) : realloc(ptr, size));
}




/*
Выделение памяти, если память еще не выделена,
либо, если память уже была выделена - изменение размера блока с помощью realloc
*/
extern void *
sm_ifalloc(void * ptr, size_t size){
	if(!ptr) 
		return sm_malloc(size);
	else 
		return sm_realloc(ptr, size);
}




/*Освобождение памяти free*/
extern void
sm_free(void * ptr){
	if(ptr) free(ptr);
	return;
}




