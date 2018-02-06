/******************************************************************************************
Функции работы с сокетами
Copyright (с) Stanislav V. Tretyakov, svtrostov@yandex.ru
******************************************************************************************/

#include "sys_socket.h"

/*----------------------------------------------------------------
Общие функции
----------------------------------------------------------------*/


extern bool
ss_valid_sock(socket_t sock_fd){
	return (((sock_fd) >= 0) && ((sock_fd) < FD_SETSIZE) ? 1 : 0);
}



/*
Функция закрывает ранее открытый сокет
*/
extern void
ss_close(socket_t sock_fd){
	if(ss_valid_sock(sock_fd)){
		ss_nonblock(sock_fd, 0);
		shutdown(sock_fd, 0);
		shutdown(sock_fd, 1);
		close(sock_fd);
	}
	return;
}




/*
Закрытие сокета на чтение
*/
extern void
ss_close_read(socket_t sock_fd){
	if(ss_valid_sock(sock_fd)) shutdown(sock_fd, 0);
	return;
}




/*
Закрытие сокета на запись
*/
extern void
ss_close_write(socket_t sock_fd){
	if(ss_valid_sock(sock_fd)) shutdown(sock_fd, 1);
	return;
}




/*
Установка сокета в блокируемое (не блокируемое) состояние
возвращает 0 в случае успеха либо -1 в случае ошибки
*/
extern int
ss_nonblock(socket_t sock_fd, int as_nonblock){
	int flags;
	if(!ss_valid_sock(sock_fd)) return -1;
	
	if((flags = fcntl(sock_fd, F_GETFL, 0)) == -1) return -1;
	
	if(as_nonblock)
		return (fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK) == -1 ? -1 : 0);
	else
		return (fcntl(sock_fd, F_SETFL, flags & (~O_NONBLOCK)) == -1 ? -1 : 0);
}



/*
Ожидание соединения для начала записи / чтения
timeout - время ожидания наступлени события в миллисекундах (1000 миллисекунд = 1 секунде),
если событие не наступило по истечении заданного времени - возвращается 0
ev - события, которые должны произойти для успешного завершения функции
ev = POLLWRNORM - ждет начала доступности сокета на запись
ev = POLLRDNORM - ждет начала доступности сокета на чтение
*/
/* Ожидание соединения */

extern int
ss_connwait(socket_t sock_fd, long timeout, bool for_read){

	if(!ss_valid_sock(sock_fd)) return -1;
	
	int srv;
	fd_set fdset;
	fd_set fderr;
	struct timeval tv;
	struct timeval init_tv;
	struct timeval now_tv;
	gettimeofday(&init_tv, NULL);
	timeout = (timeout>0 ? timeout : _VSOCKET_CONNECT_WAIT);
	int wait_ms = timeout;
	
	FD_ZERO(&fdset);
	FD_ZERO(&fderr);
	FD_SET(sock_fd, &fdset);
	FD_SET(sock_fd, &fderr);
	
	do{
		tv.tv_sec = floor(wait_ms / 1000);
		tv.tv_usec = (wait_ms % 1000) * 1000;
		
		srv = select(sock_fd + 1, (for_read ? &fdset : NULL), (for_read ? NULL : &fdset), &fderr, &tv);
		if(srv != -1) break;
		if (errno && errno != EINTR) break;
		gettimeofday(&now_tv, NULL);
		wait_ms = timeout - floor((now_tv.tv_sec-init_tv.tv_sec)*1000+(now_tv.tv_usec-init_tv.tv_usec)/1000);
		if(wait_ms <= 0) break;
		
	}while(srv == -1);
	
	if(srv <  0) return -1;
	if(srv == 0) return  0;
	
	if(FD_ISSET(sock_fd, &fderr)) return -1;
	if(FD_ISSET(sock_fd, &fdset)) return 1;
	
	return 0;
}
/*
extern int 
ss_connwait(socket_t sock_fd, long timeout, bool for_read){
	
	struct pollfd fds[1];
	int pp 			= 0;
	short ev 		= (for_read ? POLLIN : POLLOUT);
	time_t start_time	= time(NULL);
	fds[0].fd 		= sock_fd;
	fds[0].events 	= ev;
	timeout = (!timeout ? _VSOCKET_CONNECT_WAIT : timeout);
	
	
	do{
		//Очень большой таймаут...
		if(time(NULL) - start_time > 10) return 0;
		pp = poll(fds, 1, timeout);
	}while (pp == -1 && errno == EINTR);
	
	
	if (pp == 0) return 0;  //если таймаут
	if (pp < 0)  return -1; //если ошибка
	return ((fds[0].revents & ev) ? 1 : 0); //успешно
}
*/

/*
Функция проводит операции по уcтановлению соединения
с удаленным сервером по протоколу TCP
В случае ошибки, если соединение установить не удалось,
возвращается INVALID_SOCKET.
В случае успешного соединения - его идентификатор.
*/

typedef struct{
	pthread_t 			thread_id;
	bool				completed;
	struct addrinfo 	hints;
	struct addrinfo	* 	servinfo;
	const char *		hostname;
	const char *		port;
	int					rv;
}_gai_thr;

static void *
ss_gai_thr_funct(void * data){
	_gai_thr * thr = (_gai_thr *)data;
	if(!thr) return NULL;
	thr->thread_id = pthread_self();
	(void) pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	thr->rv = getaddrinfo(thr->hostname, thr->port, &(thr->hints), &(thr->servinfo));
	(void) pthread_setcanceltype (PTHREAD_CANCEL_DEFERRED, NULL);
	thr->completed = true;
	return NULL;
}


extern socket_t
ss_connect(const char * hostname, const char * port, long timeout){

	socket_t			sock_fd;
	_gai_thr			gthr;
	unsigned int		iter  = 0;
	struct addrinfo *	p;
	int					rv;
	//const int 			on = 1;
	
	if(!hostname || !port) return INVALID_SOCKET;
	if(!timeout) timeout = _VSOCKET_CONNECT_WAIT;
	
	memset(&gthr, 0, sizeof(gthr));
    gthr.hints.ai_family	= AF_INET;
    gthr.hints.ai_socktype	= SOCK_STREAM;
    gthr.hints.ai_protocol	= IPPROTO_TCP;
	gthr.hostname 			= hostname;
	gthr.port 				= port;

	//Создание потока получения информации от GetAddrInfo
	if (pthread_create(&gthr.thread_id, NULL, ss_gai_thr_funct, (void *) &gthr) != 0) return INVALID_SOCKET;
	
	while(!gthr.completed && iter < 50){
		++iter;
		su_msleep(100);
	}
	
	if(!gthr.completed){
		(void) pthread_cancel(gthr.thread_id);
	}
	
	pthread_join( gthr.thread_id, NULL );
	
	
	if (gthr.rv != 0){
		if(gthr.servinfo) 
			freeaddrinfo(gthr.servinfo);
		return INVALID_SOCKET;
	}
	
	if(!gthr.servinfo){
		return INVALID_SOCKET;
	}
	
	//Просмотр всех полученных результатов адресов
	//в попытке установить соединение
	for(p = gthr.servinfo; p != NULL; p = p->ai_next){
		
		//Открытие сокета
		//if((sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) continue;
		if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) continue;
		
		//Установка сокета в неблокируемое состояние
		if( ss_nonblock(sock_fd, 1) == -1){
			ss_close(sock_fd);
			continue;
		}
		
		//Попытка установки соединения
		//rv = connect(sock_fd, p->ai_addr, p->ai_addrlen);
		rv = connect(sock_fd, (struct sockaddr *)p->ai_addr, sizeof(struct sockaddr));
		
		//Обработка ошибки устаноки соединения
		//Это стандартное явление для неблокируемых сокетов
		if(	rv == -1 ){
			if(	errno == EINPROGRESS || 
				#if defined(EAGAIN)
				#if (EAGAIN) != (EWOULDBLOCK)
				errno == EAGAIN ||
				#endif
				#endif
				errno == EALREADY
			){
				//Ожидание готовности соединения на запись данных
				rv = ss_connwait(sock_fd, timeout, false);
				//Соединение установлено
				if(rv == 1){
					break;
				}else{
					ss_nonblock(sock_fd, 0);
					ss_close(sock_fd);
					continue;
				}
			}
			else{
				ss_nonblock(sock_fd, 0);
				ss_close(sock_fd);
				continue;
			}
		}//Обработка ошибки устаноки соединения
		
		//Соединение установлено
		//Далее нет смысла просматривать адреса
		break;
	}
	
	freeaddrinfo(gthr.servinfo);
	
	//Если были просмотрены все доступные адреса и 
	//соединение не удалось установить - выход
	if(!p){
		#if _VSOCKET_DEBUG_ERROR != 0
		printf("_connect: getaddrinfo error: no records for: [%s] : [%s]\n", hostname, port);
		#endif
		return INVALID_SOCKET;
	}
	
    return sock_fd;
}



/*
Чтение блока данных из сокета,
функция пытается прочесть nleft байт из сокета sock_fd
в буффер buf, возвращает количество прочитанных байт.
в переменную status записывается результат работы функции
-1 в случае ошибки
0 - если таймаут либо все данные были прочитаны
1 - если выполнено успешно

На заметку: 
если прочитаны не все nleft байт, 
а меньшее количество, то с большой степенью вероятности,
больше данных на чтение не будет, поэтому повторно вызывать
_sread нет смысла.
Разумеется, могут быть задержки при получении данных от 
удаленного сервера, но, поскольку функция будет ожидать новые данные 
еще как минимум интервал времени равный _VSOCKET_CONNECT_WAIT до момента
своего завершения, то можно предположить, что следующий вызов  _sread
вернет ноль байт и статус завершения то таймауту.
*/

extern size_t
ss_read(socket_t sock_fd, void * buf, size_t nleft, int * status, long timeout){
	
	if(!ss_valid_sock(sock_fd)){
		*status = -1;
		return 0;
	}
	
	void * ptr = buf;
	int ss,n;
	int nlefttoread = nleft;
	int flags = MSG_DONTWAIT;
	
	struct timeval init_tv;
	struct timeval now_tv;	
	gettimeofday(&init_tv, NULL);
	timeout = (timeout > 0 ? timeout : _VSOCKET_READ_WAIT);
	int wait_ms = timeout;
	
	//Ожидание сокета на чтение
	ss = ss_connwait(sock_fd, wait_ms, true);
	if( ss < 1 ){
		*status = ss;
		return 0;
	}
	
	for(;;){

		//читаем данные
		do{
			n = recv(sock_fd, ptr, nlefttoread, flags);
		} while (n == -1 && errno == EINTR);
		
		if( n < 0 ){
			if(errno != EWOULDBLOCK && errno != EAGAIN){
				*status = -1;
				return (size_t)(ptr-buf);
			}
		}
		//если данные получены
		else
		if( n > 0 ){
			ptr += n;
			nlefttoread -= n;
			if(nlefttoread <= 0){
				*status = 1;
				return (size_t)(ptr-buf);
			}

		}
		//все данные получены
		else 
		if( n == 0 && nlefttoread < nleft) break;
		
		gettimeofday(&now_tv, NULL);
		wait_ms = timeout - floor((now_tv.tv_sec-init_tv.tv_sec)*1000+(now_tv.tv_usec-init_tv.tv_usec)/1000);
		if(wait_ms <= 0) break;
		
		//Ожидание сокета на чтение
		ss = ss_connwait(sock_fd, 1500, true);
		
		//если ошибка select
		if( ss < 1 ){
			*status = ss;
			return (size_t)(ptr-buf);
		}
		
	}

	*status = 0;
	return (size_t)(ptr-buf);
}



/*
Функция создает буфер для чтения данных,
читает данные из сокета и возвращает указатель на начало данных
Также записывает количество прочтенных данных в переменную len
В переменную status записывается результат чтения данных
1  - все данные прочитаны
0  - таймаут
-1 - ошибка чтения данных
Примечание: функция возвращает указатель на 
выделенный блок памяти для буффера чтения, поэтому вызавающая
функция сама должна заботиться об ее освобождении.
Также иногда возвращается NULL
*/
extern char *
ss_get(socket_t sock_fd, size_t * len, int * status, long timeout){

	char * 			buf			= NULL;
	char * 			ptr			= NULL;
	size_t 			buf_size	= 0;
	size_t 			need_size	= 0;
	int 			to_read		= 0;
	size_t			readed		= 0;
	size_t 			n			= 0;
	*status						= 1;
	
	/*
	//Ожидение готовности сокета на чтение
	//Если сокет не готов на чтение, выход по таймауту
	if ( ss_connwait(sock_fd, timeout, true) != 1){
		*len	= 0;
		*status = 0;
		printf("_vsock_get: timeout\n");
		return NULL;
	}
	*/
	
	//Чтение данных из сокета
	for(;;){
		
		//Рассчет размера буфера
		need_size += _VSOCKET_BLOCK_SIZE;
		
		//Если размер буффера более допустимого лимита
		if(need_size > _VSOCKET_SCONTENT_LIMIT){
			need_size = _VSOCKET_SCONTENT_LIMIT;
		}
		
		//Выделение либо увеличение памяти под буффер чтения
		if(buf_size < need_size){
			buf = (char *)sm_realloc(buf, need_size);
			//Если не удалось выделить / увеличить объем памяти
			//немедленно завершение функции
			if(!buf){
				*len 	= readed;
				*status = -1;
				printf("_vsock_get: can not allocated memory for buffer\n");
				return NULL;
			}
			
			buf_size = need_size;
			
			//Ставим указатель для начала записи на последнюю позицию
			ptr =  (char *)(buf + readed);
					
			//Количество байт, которые надо прочитать
			//-1 байт для символа 0x00
			to_read = buf_size - readed - 1;
			
			//Если to_read = 0 - выход
			if(to_read < 1) break;
			
		}
		//Если текуший размер буфера равен либо больше требуемого
		//то для избежания переполнения буфера - прерываем чтение
		else{
			break;
		}
		//Чтение данных в буфер
		n = ss_read(sock_fd, ptr, to_read, status, timeout);
		
		//+n прочитанных байт
		readed 	+= n;
		buf[readed] = 0x00;
		//статус не 1 (выполнено успешно)
		//завершаем чтение
		if(n < to_read || *status != 1) break;
		
	}//Чтение данных из сокета
	
	*len = readed;
	buf[readed] = 0x00;
	
	return buf;
}



/*
Отправка данных
функция пытается отправить nleft байт из буффера buf, 
возвращает количество прочитанных байт.
Возвращает в результате 
-1 в случае ошибки
0 - если таймаут
1 - выполнено успешно
*/
extern int
ss_write(socket_t sock_fd, const void * buf, size_t n){

	if(!ss_valid_sock(sock_fd)) return -1;
	
	int nleft = n;
	ssize_t nwritten;
	const void * ptr = buf;
	int ss;
	struct timeval init_tv;
	struct timeval now_tv;
	gettimeofday(&init_tv, NULL);
	int wait_ms = _VSOCKET_WRITE_WAIT;
	
	//Ожидание сокета на запись
	ss = ss_connwait(sock_fd, wait_ms, false);
	if( ss < 1 ){
		return ss;
	}
	
	for(;;){
		do{
			nwritten = write(sock_fd, ptr, nleft);
		} while (nwritten == -1 && errno == EINTR);
		if(nwritten < 1){
			if(nwritten == -1 && errno != EWOULDBLOCK && errno != EAGAIN)  return -1;// error
		}
		else{
			nleft -= nwritten;
			ptr   += nwritten;
		}
		
		if(nleft <= 0) return 1;
		
		gettimeofday(&now_tv, NULL);
		wait_ms = _VSOCKET_WRITE_WAIT - floor((now_tv.tv_sec-init_tv.tv_sec)*1000+(now_tv.tv_usec-init_tv.tv_usec)/1000);
		if(wait_ms <= 0) break;
		
		//Ожидание сокета на чтение
		ss = ss_connwait(sock_fd, wait_ms, false);
		
		//если ошибка либо таймаут
		if( ss < 1 ) return ss;
		
	}//for

	return 1;
}


/*----------------------------------------------------------------
Функции сервера
----------------------------------------------------------------*/


/*
Принять входящее соединение]
*/
extern socket_t
ss_accept(socket_t sock_fd, struct sockaddr *saptr, socklen_t *salenptr){
	
	socket_t n;
	again:
	if ( (n = accept(sock_fd, saptr, salenptr)) < 0) {
		#ifdef	EPROTO
		if (errno == EPROTO || errno == ECONNABORTED)
		#else
		if (errno == ECONNABORTED)
		#endif
		goto again;
        else return INVALID_SOCKET;
	}
	
	return n;
}




/*
Инициализация TCP прослушивающего сокета,
возвращает идентификатор прослушивающего сокета в случае успеха
либо INVALID_SOCKET, если не удалось инициализировать прослушивающий сокет
*/
extern socket_t
ss_listen(const char *host, const char *port, socklen_t * addrlenp){
	
	socket_t			listen_fd;
	int 				rv;
	const int 			on = 1;
	struct addrinfo		hints;
	struct addrinfo *	res;
	struct addrinfo *	ressave;

	//Получение информации о хосте
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ( (rv = getaddrinfo(host, port, &hints, &res)) != 0){
		#if _VSOCKET_DEBUG_ERROR != 0
		printf("_stcp_listen: getaddrinfo error: %s\n", gai_strerror(rv));
		#endif
		return INVALID_SOCKET;
	}
	
	ressave = res;

	do {
		
		listen_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (listen_fd < 0) continue;
		setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
		setsockopt(listen_fd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on));
		if (bind(listen_fd, res->ai_addr, res->ai_addrlen) == 0) break;
		ss_close(listen_fd);
		
	} while ( (res = res->ai_next) != NULL);

	if(!res) return INVALID_SOCKET;
	
	listen(listen_fd, _VSOCKET_LISTENQ);
	
	if (addrlenp) 
		*addrlenp = res->ai_addrlen;
	
	freeaddrinfo(ressave);
	
	return listen_fd;
}










