/******************************************************************************************
Файл заголовков:
Функции работы с памятью
Безопасный FREE
Модуль и его функции являются thread-safe в многопоточной среде
Copyright (с) Stanislav V. Tretyakov, svtrostov@yandex.ru
******************************************************************************************/

#ifndef _SMEMORY_H
#define _SMEMORY_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


#ifdef __cplusplus
extern "C" {
#endif




/*----------------------------------------------------------------
Функции
----------------------------------------------------------------*/
	
extern void *		sm_malloc(size_t size);						//Выделение памяти malloc
extern void *		sm_calloc(size_t n, size_t m);				//Выделение памяти calloc
extern void *		sm_realloc(void * ptr, size_t size);		//Изменение размера блока памяти realloc
extern void *		sm_ifalloc(void * ptr, size_t size);		//Выделение памяти, если еще не выделена, либо, изменение размера
extern void			sm_free(void * ptr);						//Освобождение памяти free


#ifdef __cplusplus
}
#endif

#endif /*_SMEMORY_H*/





