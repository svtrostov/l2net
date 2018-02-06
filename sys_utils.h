/******************************************************************************************
Файл заголовков:
Базовые функции и утилиты
Copyright (с) Stanislav V. Tretyakov, svtrostov@yandex.ru
******************************************************************************************/

#ifndef _SUTILS_H
#define _SUTILS_H


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/time.h>
#include <math.h>
#include <pthread.h>

#include "sys_memory.h"



#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------------------------
Математические функции
----------------------------------------------------------------*/

extern unsigned int		su_crc32(const char * buf); //Расчет CRC32 текстовой строки
extern unsigned int		su_crc32n(const char * buf, size_t len); //Расчет CRC32 перрвых n символов текстовой строки
extern unsigned int		su_rand(unsigned int * seed); //Вычисление псевдо-случайного числа
extern double			su_timeout(struct timeval * beg_time, struct timeval * end_time); //Возвращает разницу между начальным и конечным временем 
extern unsigned int		su_nsec(void); //Возвращает текущее значение наносекунд в секунде
extern void				su_sleep(unsigned int secs); //Усыпляет поток на secs секунд
extern void				su_msleep(unsigned int msecs); //Усыпляет поток на msecs миллисекунд
extern void				su_usleep(unsigned int usecs); //Усыпляет поток на secs микросекунд


/*----------------------------------------------------------------
Указатели
----------------------------------------------------------------*/

extern bool		su_is_null(const void * ptr); //Проверяет, является ли значение указателя равным нулю
extern bool		su_is_zero(const char * ptr); //Проверяет, является ли указатель на текстовую строку нулевым либо значение по данному указателю равно 0x00
extern void *	su_pick_null(const void * a, const void * b); //Проверяет указатель а, если он не NULL, возвращает его, в противном случае возвращает указатель b




/*----------------------------------------------------------------
Функции работы с текстом
----------------------------------------------------------------*/

extern bool		su_in_char(const char c, const char * array); //Проверяет, находится ли указанный символ в массиве символов
extern char *	su_skip(const char * ptr, const char * array); //Пропускает символы, указанные в array, слева->направо
extern char *	su_until(const char * ptr, const char * array); //Пропускает символы до любого символа из в array, слева->направо
extern char *	su_skipr(const char * ptr, const char * array, const char * begin); //Пропускает символы, указанные в array, справа->налево
extern char *	su_untilr(const char * ptr, const char * array, const char * begin); //Пропускает символы до любого символа из в array, справа->налево
extern char *	su_sskip(const char * ptr); //Пропускает символы пробела и \r\n\t\f\v слева->направо
extern char *	su_sskipr(const char * ptr, const char * begin); //Пропускает пробела и \r\n\t\f\v справа->налево

extern unsigned int		su_copy(char * to, const char * from); //Копирует в to строку from
extern unsigned int		su_copyn(char * to, const char * from, unsigned int n); //Копирует в to первые N символов строки from
extern unsigned int		su_copy_lower(char * to, const char * from); //Копирует в to строку from преобразуя символы строки к нижнему регистру
extern unsigned int		su_copyn_lower(char * to, const char * from, unsigned int n); //Копирует в to первые N символов строки from преобразуя символы строки к нижнему регистру
extern unsigned int		su_copy_until(char * to, const char * from, const char * array); //Копирует в to строку from пока не стретится один из символов из строки array
extern unsigned int		su_copyn_until(char * to, const char * from, const char * array, unsigned int n); //Копирует в to строку from пока не стретится один из символов из строки array либо не будет скопировано N символов

extern unsigned int		su_upd(char ** to, const char * from); //Обновление строки из from
extern unsigned int		su_updn(char ** to, const char * from, unsigned int n); //Обновление строки из первых n символов from
extern unsigned int		su_updn_lower(char ** to, const char * from, unsigned int n); //Обновление строки из первых n символов from в нижнем регистре
extern unsigned int		su_upd_until(char ** to, const char * from, const char * array); //Обновление строки из from до первого символа из array

extern char *	su_new(const char * from, unsigned int * n); //Новая строка из from
extern char *	su_newn(const char * from, unsigned int * n); //Новая строка из первых n символов from
extern char *	su_newn_lower(const char * from, unsigned int * n); //Новая строка из первых n символов from в нижнем регистре
extern char *	su_new_until(const char * from, const char * array, unsigned int * n); //Новая строка из from до первого символа из array

extern bool		su_cmp(const char * str1, const char * str2); //Сравнивает две строки, FALSE, если строки не равны
extern bool		su_cmpn(const char * str1, const char * str2, unsigned int len); //Сравнивает первые N символов двух строк
extern bool		su_cmpu(const char * str1, const char * str2); //Сравнивает две строки без учета регистра
extern bool		su_cmpun(const char * str1, const char * str2, unsigned int len); //Сравнивает первые N символов двух строк без учета регистра

extern char *			su_ltoa(unsigned int n, unsigned int * len); //преобразование числа типа unsigned int в текст
extern char *			su_itoa(int n, unsigned int * len); //преобразование числа типа int в текст
extern unsigned int		su_vltoa(unsigned int n, char * s); //преобразование числа типа unsigned int в текст
extern unsigned int		su_vitoa(int n, char * s); //преобразование числа типа int в текст

extern char *	su_sub(const char *haystack, const char *needle, const char * end); //Поиск первого вхождения подстроки needle в строке haystack
extern char *	su_subu(const char *haystack, const char *needle, const char * end); //Поиск первого вхождения подстроки needle в строке haystack без учета регистра

extern char *			su_explode(char **str, const char * delimers, unsigned int * len); //Разбивает стоку, используя в качестве разделителей символы массива delimer
extern int				su_uni2utf(char *r, unsigned int wc, int n); //преобразует числовое значение Юникода в символы UTF 8,16,32
extern int				su_utf2uni(unsigned int * pwc, const char *r, int n); //преобразует символьный набор UTF в числовое значение Юникода
extern bool				su_is_hex(const char * str, size_t count); //проверяет, является ли строка либо часть строки в количестве символов count записью в HEX формате.
extern char				su_hex2char(char * str); //Преобразует HEX представление в символ типа char
extern unsigned int		su_unescape(char * ptr, char * result); //Декодирует строку из HEX представления в символьное
extern char *			su_escape(char * str, unsigned int slen); //Преобразует строку из символьного представления в HEX представление
extern unsigned int		su_trim(char * str, const char * array); //Удаляет символы, указанные в массиве array в начале и в конце строки
extern char *			su_cfind(char c, const char * str, unsigned int len);
extern char *			su_cfindr(char c, const char * str, unsigned int from);



	
#ifdef __cplusplus
}
#endif

#endif /*_SUTILS_H*/






