/******************************************************************************************
Файл заголовков:
Функции работы с сокетами
Copyright (с) Stanislav V. Tretyakov, svtrostov@yandex.ru
******************************************************************************************/
#ifndef _VSOCKET_H
#define _VSOCKET_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>
#include <math.h>
#include <ctype.h>
#include <pthread.h>
#include <signal.h>

#include "sys_memory.h"
#include "sys_utils.h"


/*----------------------------------------------------------------
Определения
----------------------------------------------------------------*/

/*
Ошибка сокета
*/
#define INVALID_SOCKET (-1)



/*
При установленном значении _VSOCKET_DEBUG_ERROR = 0x01
при возникновении ошибок будут выводиться сообщения
по-умолчанию значение 0x00 - выключено
*/
#define _VSOCKET_DEBUG_ERROR 	0



/*
Лимит ожидания соединения с удаленным сервером
в миллисекундах.
1000 миллисекунд = 1 секунде
*/
#define _VSOCKET_CONNECT_WAIT 	3000



/*
Лимит ожидания готовности сокета на чтегин данных
в миллисекундах.
1000 миллисекунд = 1 секунде
*/
#define _VSOCKET_READ_WAIT 	30000



/*
Лимит ожидания готовности сокета на запись данных
в миллисекундах.
1000 миллисекунд = 1 секунде
*/
#define _VSOCKET_WRITE_WAIT 	1000



/*
Размер блока данных, выделяемый для чтения
данных из сокета.
*/
#define _VSOCKET_BLOCK_SIZE 	32768



/*
Максимальный размер данных (буфера данных),
которые можно прчитать из сокета,
актуально для функции _sget
*/
#define _VSOCKET_SCONTENT_LIMIT	1048576



/*
максимальная очередь предварительных соединений ( аргумент для listen() )
это максимальный размер очереди предварительных еще не готовых соединений перед accept()
*/
#define	_VSOCKET_LISTENQ     	4096



/*
Тип socket_t служит для визуального представления
в программе переменных, в которых хранятся идентификаторы сокетов
*/
typedef int socket_t;




/*----------------------------------------------------------------
Общие функции
----------------------------------------------------------------*/
extern bool			ss_valid_sock(socket_t sock_fd); //Проверяет корректность сокета
extern void			ss_close(socket_t sock_fd); //закрывает ранее открытый сокет
extern void			ss_close_read(int sock_fd); //Закрытие сокета на чтение
extern void			ss_close_write(int sock_fd); //Закрытие сокета на запись
extern int			ss_nonblock(int sock_fd, int as_nonblock); //Установка сокета в блокируемое (не блокируемое) состояние
extern int 			ss_connwait(int sock_fd, long timeout, bool for_read); //Ожидание соединения для начала записи / чтения
extern socket_t		ss_connect(const char * hostname, const char * port, long timeout); //уcтановление соединения с удаленным сервером по протоколу TCP
extern size_t		ss_read(socket_t sock_fd, void * buf, size_t nleft, int * status, long timeout); //Читает данные из сокета
extern char *		ss_get(socket_t sock_fd, size_t * len, int * status, long timeout); //Читает данные из сокета и возвращает указатель на начало данных
extern int			ss_write(socket_t sock_fd, const void * buf, size_t n); //Отправка данных в сокет



/*----------------------------------------------------------------
Функции сервера
----------------------------------------------------------------*/

extern socket_t	ss_accept(socket_t sock_fd, struct sockaddr *saptr, socklen_t *salenptr);
extern socket_t	ss_listen(const char *host, const char *port, socklen_t * addrlenp);




#endif /*_VSOCKET_H*/


