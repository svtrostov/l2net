/******************************************************************************************
Базовые функции и утилиты
Copyright (с) Stanislav V. Tretyakov, svtrostov@yandex.ru
******************************************************************************************/

#include "sys_utils.h"
static void 	_su_thread_sleep(struct timespec *ti);


/*Массив значений для генерации CRC32*/
static const unsigned long su_crc32_table[256] = {
0x0,		0x77073096, 0xEE0E612C, 0x990951BA, 0x76DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
0xEDB8832, 	0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x9B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
0x76DC4190, 0x1DB7106, 	0x98D220BC, 0xEFD5102A, 0x71B18589, 0x6B6B51F, 0x9FBFE4A5, 0xE8B8D433,
0x7807C9A2, 0xF00F934, 	0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x86D3D2D, 0x91646C97, 0xE6635C01,
0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
0xEDB88320, 0x9ABFB3B6, 0x3B6E20C, 	0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x4DB2615, 0x73DC1683,
0xE3630B12, 0x94643B84, 0xD6D6A3E, 	0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0xA00AE27, 0x7D079EB1,
0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x26D930A, 	0x9C0906A9, 0xEB0E363F, 0x72076785, 0x5005713,
0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0xCB61B38, 	0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0xBDBDF21,
0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};


/*Символьный массив*/
static const char  su_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";



/*----------------------------------------------------------------
Математические функции
----------------------------------------------------------------*/


/*Расчет CRC32 текстовой строки*/
extern unsigned int
su_crc32(const char * buf){
    unsigned long crc= 0xFFFFFFFFUL;
	while (*buf) crc = su_crc32_table[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);
    return (unsigned int)(crc ^ 0xFFFFFFFFUL);
}




/*Расчет CRC32 перрвых n символов текстовой строки*/
extern unsigned int
su_crc32n(const char * buf, size_t len){
    unsigned long crc= 0xFFFFFFFFUL;
	while (len--) crc = su_crc32_table[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);
    return (unsigned int)(crc ^ 0xFFFFFFFFUL);
}




/*Вычисление псевдо-случайного числа*/
extern unsigned int
su_rand(unsigned int * seed){
	if(!*seed) *seed = su_nsec();
	unsigned int next = *(unsigned int *)seed;
	unsigned int result;
	next *= 1103515245;
	next += 12345;
	result = (unsigned int) (next / 65536) % 2048;
	next *= 1103515245;
	next += 12345;
	result <<= 10;
	result ^= (unsigned int) (next / 65536) % 1024;
	next *= 1103515245;
	next += 12345;
	result <<= 10;
	result ^= (unsigned int) (next / 65536) % 1024;
	*seed = next;        
	return (unsigned int)result;
}




/* Функция возвращает в double разницу между начальным и конечным временем */
extern double
su_timeout(struct timeval * beg_time, struct timeval * end_time){
	long sec_begin = 0L;
	long sec_end = 0L;
	double msec_begin = 0.0;
	double msec_end = 0.0;
	msec_begin  = (double) (beg_time->tv_usec / 1000000.00);
	msec_end    = (double) (end_time->tv_usec / 1000000.00);
	if (msec_begin >= 1.0) msec_begin -= (long) msec_begin;
	if (msec_end >= 1.0) msec_end -= (long) msec_end;
	sec_begin = (long)beg_time->tv_sec;
	sec_end = (long)end_time->tv_sec;
	return (double)((msec_end + sec_end)-(msec_begin + sec_begin));
}




/*Функция возвращает текущее значение наносекунд*/
extern unsigned int
su_nsec(void){
	struct timespec tmsp;
	clock_getres(CLOCK_REALTIME, &tmsp);
	return (unsigned int)tmsp.tv_nsec;
}




//THREAD SLEEP
static void 
_su_thread_sleep(struct timespec *ti){
	pthread_mutex_t mtx;
    pthread_cond_t cnd;
    pthread_mutex_init(&mtx, 0);
    pthread_cond_init(&cnd, 0);
    pthread_mutex_lock(&mtx);
    (void) pthread_cond_timedwait(&cnd, &mtx, ti);
    pthread_mutex_unlock(&mtx);
    pthread_cond_destroy(&cnd);
    pthread_mutex_destroy(&mtx);
}




/*Усыпляет поток на secs секунд*/
extern void
su_sleep(unsigned int secs){
	struct timeval tv;
    gettimeofday(&tv, 0);
    struct timespec ti;
    ti.tv_sec = tv.tv_sec + secs;
    ti.tv_nsec = (tv.tv_usec * 1000);
    _su_thread_sleep(&ti);
	//secs = (secs > 2000 ? 2000000000 : secs*1000000);
	//usleep(secs);
	return;
}




/*Усыпляет поток на msecs миллисекунд*/
extern void
su_msleep(unsigned int msecs){
	//usleep(msecs*1000);
    struct timeval tv;
    gettimeofday(&tv, 0);
    struct timespec ti;
    ti.tv_nsec = (tv.tv_usec + (msecs % 1000) * 1000) * 1000;
    ti.tv_sec = tv.tv_sec + (msecs / 1000) + (ti.tv_nsec / 1000000000);
    ti.tv_nsec %= 1000000000;
    _su_thread_sleep(&ti);
	return;
}




/*Усыпляет поток на secs микросекунд*/
extern void
su_usleep(unsigned int usecs){
	//usleep(usecs);
    struct timeval tv;
    gettimeofday(&tv, 0);
    struct timespec ti;
    ti.tv_nsec = (tv.tv_usec + (usecs % 1000000)) * 1000;
    ti.tv_sec = tv.tv_sec + (usecs / 1000000) + (ti.tv_nsec / 1000000000);
    ti.tv_nsec %= 1000000000;
    _su_thread_sleep(&ti);
	return;
}




/*----------------------------------------------------------------
Указатели
----------------------------------------------------------------*/

/*Проверяет, является ли указатель нулевым либо значение указателя равно нулю*/
extern bool
su_is_null(const void * ptr){
	if(!ptr) return true;
	return false;
}




/*Проверяет, является ли указатель на текстовую строку 
нулевым либо значение по данному указателю равно 0x00*/
extern bool
su_is_zero(const char * ptr){
	if(!ptr) return true;
	if(!*ptr) return true;
	return false;
}




/*Проверяет указатель а, если он не NULL, возвращает его,
в противном случае возвращает указатель b*/
extern void *
su_pick_null(const void * a, const void * b){
	return (void *)(su_is_null(a) ? b : a);
}




/*Проверяет указатель а, если он не NULL и значение по указателю не 0x00, возвращает его,
в противном случае возвращает указатель b*/
extern char *
su_pick_zero(char * a, char * b){
	return (su_is_zero(a) ? b : a);
}




/*----------------------------------------------------------------
Функции работы с текстом
----------------------------------------------------------------*/


/*Проверяет, находится ли указанный символ в массиве символов*/
extern bool
su_in_char(const char c, const char * array){
	if(!array) return false;
	while(*array){
		if(c == *array) return true;
		++array;
	}
	return false;
}




/*Пропускает символы, указанные в array,
пока не найдет любой другой символ либо 0x00
слева->направо
*/
extern char *
su_skip(const char * ptr, const char * array){
	while(*ptr && su_in_char(*ptr,array))++ptr;
	return (char *)ptr;
}




/*Пропускает символы, пока не найдет любой из символов 
указанных в array либо 0x00
слева->направо
*/
extern char *
su_until(const char * ptr, const char * array){
	while(*ptr && !su_in_char(*ptr,array))++ptr;
	return (char *)ptr;
}




/*Пропускает символы, указанные в array,
пока не найдет любой другой символ, либо пока не достигнет указателя begin
справа->налево.
*/
extern char *
su_skipr(const char * ptr, const char * array, const char * begin){
	while(ptr > begin && su_in_char(*ptr,array))--ptr;		
	return (char *)ptr;
}




/*Пропускает символы, пока не найдет любой символ указанный в array, 
либо пока не достигнет указателя begin
справа->налево.
*/
extern char *
su_untilr(const char * ptr, const char * array, const char * begin){
	while(ptr > begin && !su_in_char(*ptr,array))--ptr;		
	return (char *)ptr;
}




/*Пропускает символы пробела, табуляции и перевода строки,
пока не найдет любой другой символ либо 0x00
слева->направо
*/
extern char *
su_sskip(const char * ptr){
	return (char *)su_skip(ptr, "\r\n\t\f\v ");
}




/*Пропускает символы пробела, табуляции и перевода строки,
пока не найдет любой другой символ либо не достигнет указателя begin
слева->направо
*/
extern char *
su_sskipr(const char * ptr, const char * begin){
	return (char *)su_skipr(ptr, "\r\n\t\f\v ", begin);
}




/*Функция копирования символов из одной строки в другую
возвращает фактически скопированное количество символов*/
extern unsigned int
su_copy(char * to, const char * from){
	register unsigned int i = 0;
	while ((*to++ = *from++) != 0)++i;
	*to=0x00;
	return i;
}




/*Функция копирования определенного количества символов из одной строки в другую
возвращает фактически скопированное количество символов*/
extern unsigned int
su_copyn(char * to, const char * from, unsigned int n){
	register unsigned int i = 0;
	while (i<n && (*to++=*from++)!= 0)++i;
	*to=0x00;
	return i;
}




/*Функция копирования символов из одной строки в другую
возвращает фактически скопированное количество символов,
при этом текст преобразуется к нижнему регистру*/
extern unsigned int
su_copy_lower(char * to, const char * from){
	register unsigned int i = 0;
	while ((*to++=tolower(*from++))!= 0)++i;
	*to=0x00;
	return i;
}




/*Функция копирования определенного количества символов из одной строки в другую
возвращает фактически скопированное количество символов,
при этом текст преобразуется к нижнему регистру*/
extern unsigned int
su_copyn_lower(char * to, const char * from, unsigned int n){
	register unsigned int i = 0;
	while (i<n && (*to++=tolower(*from++))!= 0)++i;
	*to=0x00;
	return i;
}




/*Функция копирования символов из одной строки в другую, 
до тех пор, пока не встретится один из стоп-символов из массима array
возвращает фактически скопированное количество символов
Если ни один из стоп-символов не найден, строка копируется целиком
*/
extern unsigned int
su_copy_until(char * to, const char * from, const char * array){
	register unsigned int i = 0;
	while ( !su_in_char(*from, array) && (*to++ = *from++) != 0)++i;
	*to=0x00;
	return i;
}




/*Функция копирования символов из одной строки в другую, 
до тех пор, пока не встретится один из стоп-символов из массима array
либо не будет достигнут лимит в n символов
возвращает фактически скопированное количество символов
Если ни один из стоп-символов не найден, копируется первые n символов
*/
extern unsigned int
su_copyn_until(char * to, const char * from, const char * array, unsigned int n){
	register unsigned int i = 0;
	while (!su_in_char(*from, array) && i < n && (*to++ = *from++) != 0 )++i;
	*to=0x00;
	return i;
}




/*Функция очищает текущую текстовую строку, если она не пуста 
выделяет память под новую строку и копирует в нее данные из from*/
extern unsigned int
su_upd(char ** to, const char * from){
	if(*(char **)to){
		sm_free(*(char **)to);
		*(char **)to = NULL;
	}
	if(!from) return 0;
	*(char **)to = (char *)sm_malloc(strlen(from) + 1);
	if(*(char **)to) 
		return su_copy(*(char **)to, from);
	return 0;
}




/*Функция очищает текущую текстовую строку, если она не пуста 
выделяет память под новую строку и копирует в нее данные из from*/
extern unsigned int
su_updn(char ** to, const char * from, unsigned int n){
	if(*(char **)to){
		sm_free(*(char **)to);
		*(char **)to = NULL;
	}
	if(!from) return 0;
	*(char **)to = (char *)sm_malloc(n + 1);
	if(*(char **)to) 
		return su_copyn(*(char **)to, from, n);
	return 0;
}




/*Функция очищает текущую текстовую строку, если она не пуста 
выделяет память под новую строку и копирует в нее данные из from,
при этом текст преобразуется к нижнему регистру*/
extern unsigned int
su_updn_lower(char ** to, const char * from, unsigned int n){
	if(*(char **)to){
		sm_free(*(char **)to);
		*(char **)to = NULL;
	}
	if(!from) return 0;
	*(char **)to = (char *)sm_malloc(n+1);
	if(*(char **)to) 
		return su_copyn_lower(*(char **)to, from, n);
	return 0;
}




/*Функция очищает текущую текстовую строку, если она не пуста 
выделяет память под новую строку и копирует в нее данные из from*/
extern unsigned int
su_upd_until(char ** to, const char * from, const char * array){
	if(*(char **)to){
		sm_free(*(char **)to);
		*(char **)to = NULL;
	}
	if(!from) return 0;
	unsigned int l = (unsigned int)(su_until(from, array) - from);
	*(char **)to = (char *)sm_malloc(l + 1);
	if(*(char **)to) 
		return su_copyn(*(char **)to, from, l);
	return 0;
}




/*Функция делает копию текстовой строки и возвращает
указатель на эту копию, также, если len не NULL,
возвращает длинну скопированной строки*/
extern char *
su_new(const char * from, unsigned int * n){
	if(!from){
		if(n) *n = 0;
		return NULL;
	}
	char * to = NULL;
	unsigned int l  = su_upd(&to, from);
	if(n)*n = l;
	return to;
}




/*Функция делает копию текстовой строки и возвращает
указатель на эту копию, 
если len не NULL, копирует из строки from в новую строку 
len символов, возвращает в len длинну полученной строки*/
extern char *
su_newn(const char * from, unsigned int * n){
	if(!from){
		if(n) *n = 0;
		return NULL;
	}
	char * to = NULL;
	if(n)
		*n = su_updn(&to, from, *n);
	else
		su_upd(&to, from);

	return to;
}




/*Функция делает копию текстовой строки, приводя все символы 
к нижнему регистру, и возвращает указатель на эту копию, 
если len не NULL, копирует из строки from в новую строку 
len символов, возвращает в len длинну полученной строки*/
extern char *
su_newn_lower(const char * from, unsigned int * n){
	if(!from){
		if(n) *n = 0;
		return NULL;
	}
	char * to = NULL;
	if(n){
		*n = su_updn_lower(&to, from, *n);
	}
	else{
		unsigned int l = strlen(from);
		su_updn_lower(&to, from, l);
	}

	return to;
}




/*Функция делает копию текстовой строки 
либо части строки до тех пор пока не встретится один из символов из массива array
и возвращает указатель на эту копию, также, если len не NULL,
возвращает длинну скопированной строки*/
extern char *
su_new_until(const char * from, const char * array, unsigned int * n){
	if(!from){
		if(n) *n = 0;
		return NULL;
	}
	char * to = NULL;
	unsigned int l = (unsigned int)(su_until(from, array) - from);
	l = su_updn(&to, from, l);
	if(n)*n = l;
	return to;
}




/*Функция сравнения строк, возвращает TRUE если строки 
идентичны либо FALSE если не идентичны*/
extern bool
su_cmp(const char * str1, const char * str2){
	while ( *str1 && *str2){
		if( *str1 != *str2 ) return false;
		++str1;
		++str2;
	}
	return true;
}




/*Функция сравнения строк, возвращает TRUE если строки 
идентичны либо FALSE если не идентичны*/
extern bool
su_cmpn(const char * str1, const char * str2, unsigned int len){
	register unsigned int n = 0;
	while ( *str1 && *str2 && n<len){
		if( *str1 != *str2 ) return false;
		++str1;
		++str2;
		++n;
	}
	return (n == len ? true: false);
}




/*Функция сравнения строк, возвращает TRUE если строки 
идентичны либо FALSE если не идентичны*/
extern bool
su_cmpu(const char * str1, const char * str2){
	while ( *str1 && *str2){
		if( toupper(*str1) != toupper(*str2) ) return false;
		++str1;
		++str2;
	}
	return true;
}




/*Функция сравнения строк, возвращает TRUE если строки 
идентичны либо FALSE если не идентичны*/
extern bool
su_cmpun(const char * str1, const char * str2, unsigned int len){
	register unsigned int n = 0;
	while ( *str1 && *str2 && n<len){
		if( toupper(*str1) != toupper(*str2) ) return false;
		++str1;
		++str2;
		++n;
	}
	return (n == len ? true: false);
}




/* Функция преобразования числа типа unsigned int в текст */
extern char *
su_ltoa(unsigned int n, unsigned int * len){
	char s[16];
	char * result = NULL;
	unsigned int c,x,j,i=0;
	do {
		s[i++] = n % 10 + '0';
	}while ((n /=10) > 0 && i < 15);
	s[i] = 0x00;
	for(x = 0, j = i - 1; x < j; x++, j--) {
		c = s[x];
		s[x] = s[j];
		s[j] = c;
	}
	i = su_updn(&result, s, i);
	if(len) *len = i;
	return result;
}




/* Функция преобразования числа типа int в текст */
extern char *
su_itoa(int n, unsigned int * len){
	char s[16];
	char * result = NULL;
	int c,x,j,i = 0,sign = n;
	if (sign < 0) n = -n;
	do {
		s[i++] = n % 10 + '0';
	} while ((n /=10) > 0 && i < 15);
	if (sign < 0) s[i++] = '-';
	s[i] = 0x00;
	for(x = 0, j = i - 1; x < j; x++, j--){
		c = s[x];
		s[x] = s[j];
		s[j] = c;
	}
	i = su_updn(&result, s, i);
	if(len) *len = i;
	return result;
}




/* Функция преобразования числа типа unsigned int в текст */
extern unsigned int
su_vltoa(unsigned int n, char * s){
	unsigned int c,x,j,i=0;
	do {
		s[i++] = n % 10 + '0';
	}while ((n /=10) > 0 && i < 15);
	s[i] = 0x00;
	for(x = 0, j = i - 1; x < j; x++, j--) {
		c = s[x];
		s[x] = s[j];
		s[j] = c;
	}
	return i;
}




/* Функция преобразования числа типа int в текст */
extern unsigned int
su_vitoa(int n, char * s){
	int c,x,j,i = 0,sign = n;
	if (sign < 0) n = -n;
	do {
		s[i++] = n % 10 + '0';
	} while ((n /=10) > 0 && i < 15);
	if (sign < 0) s[i++] = '-';
	s[i] = 0x00;
	for(x = 0, j = i - 1; x < j; x++, j--){
		c = s[x];
		s[x] = s[j];
		s[j] = c;
	}
	return i;
}




/*Возвращает первое вхождение needle в haystack либо NULL 
просматривается текст до end позиции,
либо до конца строки, если end = NULL*/
extern char *
su_sub(const char *haystack, const char *needle, const char * end){
	const char *temp, *c;
	temp = needle;
	while (*haystack && (!end || haystack < end)){
		c = haystack;
		while(*(haystack++) == *(needle++)){
			if (!(*needle)){
				return (char *) c;
			}
			if(!(*haystack)){
				return NULL;
			}
		}
		needle = temp;
	}
	return NULL;
}




/*Возвращает первое вхождение needle в haystack либо NULL
регистр не учитывается просматривается текст до end позиции,
либо до конца строки, если end = NULL*/
extern char *
su_subu(const char *haystack, const char *needle, const char * end){
	const char *temp, *c;
	temp = needle;
	while (*haystack && (!end || haystack < end)){
		c = haystack;
		while(toupper(*(haystack++)) == toupper(*(needle++))){
			if (!(*needle)){
				return (char *) c;
			}
			if(!(*haystack)){
				return NULL;
			}
		}
		needle = temp;
	}
	return NULL;
}




/*
Разбивает стоку, используя в качестве разделителей символы массива delimer,
возвращает указатель на начало нового вхождения и также в *len - длинну строки
Пример:
char * my_text = "Hello! How are you? Thanks, i'm fine! Ok, buy.";
char * my_delimers = "!?";
char * my_ptr = my_text;
char * result_ptr;
unsigned int result_len;
while ( ( result_ptr = su_explode(&my_ptr, my_delimers, &result_len) ) != NULL){
	...
}
Результат работы функции, следующие вхождения:
"Hello"
" How are you"
" Thanks, i'm fine"
" Ok, buy."
*/
extern char *
su_explode(char **str, const char * delimers, unsigned int * len){
	char *ptr = *str;
	char *result;
	//Если ptr = NULL либо *ptr = 0x00
	if(su_is_zero(ptr)) return NULL;
	
	//Поис вхождения одного из символов, заданных в массиве delimers
	while(*ptr && !su_in_char(*ptr, delimers)) ++ptr;
	*len = (unsigned int)(ptr - *str);
	
	//Если значение *ptr не нулевой символ
	if(!su_is_zero(ptr)) ++ptr;
	
	result = *str;
	*str = ptr;
	return result;
}




/*
Функция преобразует числовое значение Юникода в
символы UTF 8,16,32
r - буфер, куда записываются полученные символы
wc - числовое значение по юникоду
n - максимальное количество символов, которое можно 
записать в буфер (количество символов в кодировке)
в случае ошибки функция возвращает -1
*/
extern int
su_uni2utf(char *r, unsigned int wc, int n){
	
	int count;
	
	if (wc < 0x80) count = 1;
	else if (wc < 0x800) count = 2;
	else if (wc < 0x10000) count = 3;
	else if (wc < 0x200000) count = 4;
	else if (wc < 0x4000000) count = 5;
	else if (wc <= 0x7fffffff) count = 6;
	else return -1;
	
	if (n < count) return -1;
	switch (count) {
		case 6: r[5] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0x4000000;
		case 5: r[4] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0x200000;
		case 4: r[3] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0x10000;
		case 3: r[2] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0x800;
		case 2: r[1] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0xc0;
		case 1: r[0] = wc;
	};
	
	return count;
}




/*
Функция преобразует символьный набор UTF в числовое значение Юникода
s - буфер, откуда читаются символы
pwc - указатель, куда будет записано числовое значение юникода
n - максимальное количество символов, которое можно 
прочитать из буфера (количество символов в кодировке)
в случае ошибки функция возвращает -1
*/
extern int
su_utf2uni(unsigned int * pwc, const char *r, int n){
	
	const unsigned char   c = (unsigned char)r[0];
	const unsigned char * s = (unsigned char *)r;
	
	if(c < 0x80){
		*pwc = (unsigned int) c;
		return 1;
	} 
	else 
	if(c < 0xc2) {
		return -1;
	} 
	else 
	if (c < 0xe0){
		if (n < 2) return -1;
		if (!((s[1] ^ 0x80) < 0x40)) return -1;
		*pwc = ((unsigned int) (c & 0x1f) << 6) | (unsigned int) (s[1] ^ 0x80);
		return 2;
	} 
	else 
	if (c < 0xf0){
		if (n < 3) return -1;
		if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40 && (c >= 0xe1 || s[1] >= 0xa0))) return -1;
		*pwc = ((unsigned int) (c & 0x0f) << 12) | ((unsigned int) (s[1] ^ 0x80) << 6) | (unsigned int) (s[2] ^ 0x80);
		return 3;
	} 
	else 
	if (c < 0xf8 && sizeof(unsigned int)*8 >= 32){
		if (n < 4) return -1;
		if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40 && (s[3] ^ 0x80) < 0x40 && (c >= 0xf1 || s[1] >= 0x90))) return -1;
		*pwc = ((unsigned int) (c & 0x07) << 18) | ((unsigned int) (s[1] ^ 0x80) << 12) | ((unsigned int) (s[2] ^ 0x80) << 6) | (unsigned int) (s[3] ^ 0x80);
		return 4;
	}
	else 
	if (c < 0xfc && sizeof(unsigned int)*8 >= 32){
		if (n < 5) return -1;
		if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40 && (s[3] ^ 0x80) < 0x40 && (s[4] ^ 0x80) < 0x40 && (c >= 0xf9 || s[1] >= 0x88))) return -1;
		*pwc = ((unsigned int) (c & 0x03) << 24) | ((unsigned int) (s[1] ^ 0x80) << 18) | ((unsigned int) (s[2] ^ 0x80) << 12) | ((unsigned int) (s[3] ^ 0x80) << 6) | (unsigned int) (s[4] ^ 0x80);
		return 5;
	}
	else
	if (c < 0xfe && sizeof(unsigned int)*8 >= 32){
		if (n < 6) return -1;
		if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40 && (s[3] ^ 0x80) < 0x40 && (s[4] ^ 0x80) < 0x40 && (s[5] ^ 0x80) < 0x40 && (c >= 0xfd || s[1] >= 0x84))) return -1;
		*pwc = ((unsigned int) (c & 0x01) << 30) | ((unsigned int) (s[1] ^ 0x80) << 24) | ((unsigned int) (s[2] ^ 0x80) << 18) | ((unsigned int) (s[3] ^ 0x80) << 12) | ((unsigned int) (s[4] ^ 0x80) << 6) | (unsigned int) (s[5] ^ 0x80);
		return 6;
	} 
	else 
		return -1;
}




/*
Функция проверяет, является ли строка либо часть строки в количестве символов count
записью в HEX формате.
если строка в HEX формате - возвращает TRUE, либо FALSE если нет
*/
extern bool
su_is_hex(const char * str, size_t count){
	
	int len	= 0;
	const char * ptr = str;
	
	while(*ptr != 0 && (!count || len < count)){
		if( (*ptr < '0' || *ptr > '9') &&
			(*ptr < 'a' || *ptr > 'f') &&
			(*ptr < 'A' || *ptr > 'F') ) return false;
		++ptr;
		++len;
	}
	return true;
}




/*Преобразует HEX представление в символ типа char*/
extern char
su_hex2char(char * str){

	char ch;
	
	if(str[0]>='A')
		ch = ((str[0]&0xdf)-'A')+10;
	else
		ch = str[0] - '0';

	ch <<= 4;
	if(str[1]>='A')
		ch += ((str[1]&0xdf)-'A')+10;
	else
		ch += str[1] - '0';
	
	return ch;
}




/*
Декодирует строку из HEX представления в символьное
Функция может декодировать как представлени типа %3A%2F%2F
так и представления в формате Юникода, типа %u041F%u0440%u0438

Если нужно, чтобы результат писался в тот же буфер, откуда читается,
значение result должно быть равно NULL
если result не NULL, результат пишется в буфер по заданному указателю,
в этом случае размер результирующего буфера должен быть не меньше размера исходного буфера.
*/
extern unsigned int
su_unescape(char * ptr, char * result){
	
	size_t 	len		= 0;
	int 	n		= 0;
	char * 	rptr 	= (!result ? ptr : result);
	
	while(*ptr){
		
		if(*ptr=='+') *ptr = 0x20;
		else
		if(*ptr=='%' && su_is_hex((ptr+1), 2)){
			*rptr++ = su_hex2char(ptr+1);
			ptr += 3;
			++len;
			continue;
		}
		else
		if(*ptr=='%' && (*(ptr+1)=='u' || *(ptr+1)=='U') && su_is_hex((ptr+2), 4)){
			n  = su_hex2char(ptr+2) * 256;
			n += su_hex2char(ptr+4);
			su_uni2utf(rptr, n, 2);
			rptr += 2;
			len	 += 2;
			ptr  += 6;
			continue;
		}
		*rptr++ = *ptr++;
		++len;
	
	}
	
	*rptr = 0x00;
	
	return len;
}




/*
Преобразует строку из символьного представления в HEX представление, 
возвращает новую строку,
В случае ошибки возвращает NULL, иначе - указатель на закодированную строку
str - исходная строка
slen - количество символов, которое надо кодировать, либо 0 чтобы закодировать строку целиком
*/
extern char *
su_escape(char * str, unsigned int slen){
	
	unsigned int 	len 	= (!slen ? strlen(str) : slen);
	unsigned int 	n		= 0;
	unsigned int 	value	= 0;
	unsigned int 	from	= 0;
	unsigned int 	to		= 0;
	char *			result	= NULL;
	
	//Максимальная длинна закодированной строки
	//при учете, что все символы в кодировке UTF
	//равняется шести кратам от исходного размера
	char * 	buf = (char *) sm_malloc(len*6 + 1);
	
	if(!buf) return NULL;
	
	while(from < len){
		n = su_utf2uni(&value, &str[from], 2);
		if(n > 0){
			if(n == 2){
				to += sprintf(&buf[to],"%%u%0*x", 4, value);
			}else{
				to += sprintf(&buf[to],"%%%hhX", value);
			}
			from += n;
		}else{
			from++;
		}
	}
	
	buf[to] = 0x00;
	
	su_upd(&result, buf);
	sm_free(buf);
	
	return result;
}




/*
Удаляет символы, указанные в массиве array
в начале и в конце строки
напрмер, _trim("  !!!hello!!! ", "!\s") = "hello"
Функция возвращает длинну полученной строки
*/
extern unsigned int
su_trim(char * str, const char * array){
	char *	b 	= str;
	char *	ptr	= str;
	
	ptr = su_skip(str, array);
	while( (*b++ = *ptr++) != 0x00 );
	ptr = su_skipr((b-2), array, str); // b-1 = 0x00
	*(ptr+1) = 0x00;
	
	return (ptr - str + 1);
}



/*
Ищет первое вхождение заданного символа в строке,
возвращает указатель на искомый символ, либо
NULL если символ не был найден
Функция просматривает первые len символов строки, 
либо если значение len установлено в 0 - просматривает
строку до конца.
*/
extern char *
su_cfind(char c, const char * str, unsigned int len){
	
	const char * end = (len > 0 ? str+len : NULL);
	
	while(*str != 0x00 && (!end || str < end)){
		if(*str == c) return (char *)str;
		++str;
	}
	
	return NULL;
}



/*
Ищет первое вхождение заданного символа в строке,
при этом просматривает строку справа налево,
возвращает указатель на искомый символ, либо
NULL если символ не был найден
Функция просматривает начиная с from символа строки,
либо если значение len установлено в 0 - просматривает
строку с конца.
*/
extern char *
su_cfindr(char c, const char * str, unsigned int from){
	
	const char * ptr = (from > 0 ? str+from : str + strlen(str) - 1);
	
	while(ptr >= str){
		if(*ptr == c) return (char *)ptr;
		--ptr;
	}
	
	return NULL;
}


