/******************************************************************************************
L2Net - Layer 2 Network Control Software
Модуль работы с ARP пакетами
Copyright (с) Stanislav V. Tretyakov, svtrostov@yandex.ru
******************************************************************************************/

#include "main.h"

static pcap_t * 			pcap = NULL;
static libnet_t * 			lnet = NULL;

static struct ether_addr 	mac_big, mac_zero;
static struct in_addr 		ip_zero;

static struct ether_addr 	mac_arp_sender, mac_arp_target, mac_eth_sender, mac_eth_target;
static struct in_addr 		ip_arp_sender, ip_arp_target;

static struct ether_addr	VAR_FAKE_MAC;			//ложный MAC адрес (вычисляется на основе значения переменной VAR_FAKE_MAC_TEXT)

/*----------------------------------------------------
Вспомогательные функции
----------------------------------------------------*/


/*Функция ищет сетевой интерфейс, используемый по-умолчанию и возвращает указатель на его название*/
char *
f_arp_lookup_eth(void){
	char *dev, errbuf[PCAP_ERRBUF_SIZE];
	dev = pcap_lookupdev(errbuf);
	if (dev == NULL) {
		printf("Couldn't find default network interface: %s\n", errbuf);
		exit(1);
	}
	return dev;
}



/*Генерация ложного MAC адреса*/
char * 
f_arp_rand_mac(const char * mask){
	char c[] = "  ";
	register int l = strlen(mask);
	char * mac = (char *)sm_malloc(l+1);
	register int n;

	for (n = 0; n < l; n++){
		mac[n] = mask[n];
		if (mac[n] == 'x'){ 
			snprintf(c, 2, "%1x", rand() % 16); 
			mac[n] = c[0]; 
		}
	}
	mac[l] = '\0';
	return mac;
}





/*----------------------------------------------------
Функции работы с ARP пакетами
----------------------------------------------------*/



/*Инициализация интерфейса прослушивания ARP пакетов*/
void
f_arp_init(void){

	struct bpf_program arp_p;
	char ebuf[PCAP_ERRBUF_SIZE] = "\0";

	//Проверка имени сетевого интерфейса
	//Если сетевой интерфейс не задан, либо указан автоматический выбор
	//будет произведен поиск интерфейса, заданного по-умолчанию
	if(strlen(VAR_ETH_INTERFACE)<2 || su_cmpu(VAR_ETH_INTERFACE, "AUTO")){
		su_upd(&VAR_ETH_INTERFACE, f_arp_lookup_eth());
	}

	//Создание сессии для снифинга с использованием pcap
	if (!(pcap = pcap_open_live(VAR_ETH_INTERFACE, 100, 0, 10, ebuf))){
		printf("pcap_open_live() error: %s\n", ebuf);
		exit(1);
	}

	//При инициализации возникла ошибка либо предупреждение
	if (strlen(ebuf)) printf("warning: %s\n", ebuf);

	//Условная "Компиляция" фильтра PCAP для обработки ARP пакетов
	if (pcap_compile(pcap, &arp_p, "arp", 0, -1) == -1){ 
		printf("pcap_compile(): failed\n");
		exit(1);
	}

	//Установка фильтра
	if (pcap_setfilter(pcap, &arp_p) == -1){
		printf("pcap_setfilter(): failed");
        exit(1);
    }

	//Инициализация сессии libnet
	if (!(lnet = libnet_init(LIBNET_LINK_ADV, VAR_ETH_INTERFACE, ebuf))) {
		printf("libnet error: %s\n", ebuf);
		exit(1);
	}

	//Присвоение значений переменным
	memset(&mac_big, 0xff, sizeof(struct ether_addr));
	memset(&mac_zero, 0x00, sizeof(struct ether_addr));
	memset(&ip_zero, 0x00, sizeof(struct in_addr));

	//Вычисление ложного MAC адреса, если он не был задан в настройках программы
	//либо была выбрана автоматическая генерация MAC
	if(strlen(VAR_FAKE_MAC_TEXT)<4 || su_cmpu(VAR_FAKE_MAC_TEXT, "AUTO")){
		sm_free(VAR_FAKE_MAC_TEXT);
		VAR_FAKE_MAC_TEXT = f_arp_rand_mac(CNT_FAKE_MAC_MASK);
	}

	memcpy(&VAR_FAKE_MAC, ether_aton(VAR_FAKE_MAC_TEXT), sizeof(struct ether_addr));

	return;
}




/*Функция получения ARP пакетов*/
void 
f_arp_packet_recv(void){
	if (pcap_loop(pcap, -1, (pcap_handler) f_arp_packet_handle, NULL) == -1) {
		printf("pcap_loop() failed\n");
		exit(1);
	}
	return;
}




/*Функция обработки пролученных ARP пакетов*/
void
f_arp_packet_handle(char *blah, struct pcap_pkthdr *blah2, u_char *recv){
	
	struct ether_header * 	eth_header = NULL;
	struct ether_arp * 		arp_header = NULL;
	int op;

	//Заголовок пакета
	eth_header = (struct ether_header *) recv;

	//Если полученный пакет не является пакетом ARP - выход из функции
	if (htons(eth_header->ether_type) != ETHERTYPE_ARP) return;

	//Получение MAC адреса отправителя пакета из заголовка пакета
	memcpy(&mac_eth_sender, &eth_header->ether_shost, sizeof(struct ether_addr));

	//Получение MAC адреса получателя пакета из заголовка пакета
	memcpy(&mac_eth_target, &eth_header->ether_dhost, sizeof(struct ether_addr));

	//ARP заголовок
	arp_header = (struct ether_arp *) ((char *) eth_header + ETHER_HDR_LEN);

	//Некорректный MAC
	if (htons(arp_header->ea_hdr.ar_hrd) != ARPHRD_ETHER) return;

	//Некорректный proto адрес
	if (htons(arp_header->ea_hdr.ar_pro) != ETHERTYPE_IP) return;

	//Определние типа полученного ARP пакета
	op = htons(arp_header->ea_hdr.ar_op);

	memcpy(&mac_arp_sender, &arp_header->arp_sha, sizeof(struct ether_addr)); //MAC адрес отправителя
	memcpy(&ip_arp_sender, &arp_header->arp_spa, sizeof(struct in_addr)); //IP адрес отправителя
	memcpy(&mac_arp_target, &arp_header->arp_tha, sizeof(struct ether_addr)); //MAC адрес получателя
	memcpy(&ip_arp_target, &arp_header->arp_tpa, sizeof(struct in_addr));  //IP адрес получателя

	//Тип пакета - ARP запрос
	if(op == ARPOP_REQUEST){
		printf("who has %s tell ", inet_ntoa(ip_arp_target));
		printf("%s (%s)\n", inet_ntoa(ip_arp_sender), ether_ntoa(&mac_arp_sender));
	}

	//Тип пакета - ARP ответ
	if(op == ARPOP_REPLY){
		printf("%s is at %s ", inet_ntoa(ip_arp_sender), ether_ntoa(&mac_arp_sender));
		printf("for %s (%s)\n", inet_ntoa(ip_arp_target), ether_ntoa(&mac_arp_target));
		return;
	}

	//Если тип пакета не ARP запрос, пропускаем проверку пакета
	if (op != ARPOP_REQUEST) return;
	
	//Проверка пакета ARPOP_REQUEST
	//packet_check();
	
	return;
}




/*Проверка ARP пакета*/
void 
f_arp_packet_check(void){

	struct _item_macip * p = NULL;
	struct _item_macip * mac_sender = NULL;
	struct _item_macip * mac_target = NULL;
	struct _item_macip * ip_sender  = NULL;
	struct _item_macip * ip_target  = NULL;
	bool 	bad_sender = false;
	u_long ipsw;
	char ntoasw[16];
	unsigned int ip_sender_int;
	unsigned int ip_target_int;

	//Ложный пакет
	//Несоответствие MAC адресов получателя в заголовке пакета и заголовке ARP
	if(memcmp(&mac_arp_sender, &mac_eth_sender, sizeof(struct ether_addr))){
		strncpy(ntoasw, inet_ntoa(ip_arp_target), 16);
		printf("e!=h: %-17s %-15s %-15s\n", ether_ntoa(&mac_arp_sender), inet_ntoa(ip_arp_sender), ntoasw);
	}

	//Указано ненулевое значение MAC в пакете ARP типа who-has
	if(memcmp(&mac_zero, &mac_eth_target, sizeof(struct ether_addr))){
		strncpy(ntoasw, inet_ntoa(ip_arp_target), 16);
		printf("!z h: %-17s %-15s %-15s\n", ether_ntoa(&mac_arp_sender), inet_ntoa(ip_arp_sender), ntoasw);
	}

	//Не broadcast запрос в пакете ARP типа who-has
	if(memcmp(&mac_big, &mac_eth_target, sizeof(struct ether_addr))) {
		strncpy(ntoasw, inet_ntoa(ip_arp_target), 16);
		printf("!b e: %-17s %-15s %-15s\n", ether_ntoa(&mac_arp_sender), inet_ntoa(ip_arp_sender), ntoasw);
	}

	//Игнорировать проверку собственного MAC адреса
	if(!memcmp(&mac_arp_sender, &SYS_MACIP_INFO->mac, sizeof(struct ether_addr))) {
		return;
    }

	//Игнорировать проверку собственного ложного MAC адреса
	if(!memcmp(&mac_arp_sender, &VAR_FAKE_MAC, sizeof(struct ether_addr))) {
		return;
	}

	//в самообращенном ARP пакете IP отправителя указан как 0.0.0.0 (Linux/MacOS/Vista),
	//ARP RFC не предусматривает использование таких запросов.
	//Устанавливаем в качастве IP отправителя IP адрес получателя
    if(!memcmp(&ip_arp_sender, &ip_zero, sizeof(struct in_addr))){
		memcpy(&ip_arp_sender, &ip_arp_target, sizeof(struct in_addr));
	}

	//Вычисление числовых значений IP адресов
	ip_sender_int = ntohl(ip_arp_sender.s_addr);
	ip_target_int = ntohl(ip_arp_target.s_addr);

	//Проверка наличия пары MAC->IP
	for(p = VAR_ARP_PAIRS->list; p; p = p->next){

		//Если MAC адрес отправителя соответствует MAC адресу, указанному в паре
		if (!memcmp(&mac_eth_sender, &p->mac, sizeof(struct ether_addr))) {
			if (!mac_sender){
				mac_sender = p;
				memcpy(&p->ip, &ip_arp_sender, sizeof(struct in_addr));
			}
			//Если IP адрес отправителя соответствует IP адресу, указанному в паре
			if(ip_sender_int >= p->ip_from && ip_sender_int <= p->ip_to){
				if(mac_sender != p) memcpy(&p->ip, &ip_arp_sender, sizeof(struct in_addr));
				mac_sender = ip_sender = p;
			}
		}

		//Если MAC адрес получателя соответствует MAC адресу, указанному в паре
		if (!memcmp(&mac_eth_target, &p->mac, sizeof(struct ether_addr))) {
			if (!mac_target){
				mac_target = p;
				memcpy(&p->ip, &ip_arp_target, sizeof(struct in_addr));
			}
			//Если IP адрес получателя соответствует IP адресу, указанному в паре
			if(ip_target_int >= p->ip_from && ip_target_int <= p->ip_to){
				if(mac_target != p) memcpy(&p->ip, &ip_arp_target, sizeof(struct in_addr));
				mac_target = ip_target = p;
			}
		}

		//Если MAC адрес не сопоставлен, а IP адрес сопоставлен успешно
		if (!ip_sender && ip_sender_int >= p->ip_from && ip_sender_int <= p->ip_to){
			ip_sender = p;
			memcpy(&p->ip, &ip_arp_sender, sizeof(struct in_addr));
		}

		if (!ip_target && ip_target_int >= p->ip_from && ip_target_int <= p->ip_to){
			ip_target = p;
			memcpy(&p->ip, &ip_arp_target, sizeof(struct in_addr));
		}

	}

	//самообращенный ARP пакет
	//В рамках протокола ARP возможны самообращенные запросы (gratuitous ARP). 
	//При таком запросе отправитель формирует пакет, где в качестве IP цели используется его собственный адрес. 
	//Это бывает нужно, когда осуществляется стартовая конфигурация сетевого интерфейса. 
	//В таком запросе IP-адреса отправителя и получателя совпадают
	if(mac_sender && (mac_sender == ip_target) && !memcmp(&ip_arp_sender, &ip_arp_target, sizeof(struct in_addr))) return;

	//MAC получателя = 00:00:00:00:00:00 - для этого IP адреса допустим любой MAC
	if (ip_sender && !memcmp(&ip_sender->mac, &mac_zero, sizeof(struct ether_addr))) {
		return;
	}

	//IP отправителя =  0.0.0.0 - для этого MAC адреса допустим любой IP
	if (mac_sender && !memcmp(&mac_sender->ip, &ip_zero, sizeof(struct in_addr))) {
		return;
	}

	//Корректная пара MAC->IP у отправителя и указан IP цели либо не задан режим двойной проверки
	if (mac_sender && (mac_sender == ip_sender) && (ip_target || !VAR_DUPLEX_CHECK_MODE)){
		if(!mac_sender->deny){
			return; //пара явно не запрещена
		}else{
			if (VAR_SILENT_MODE) return;
			bad_sender = true; //пара запрещена
		}
	}

	//Событие: Новая пара MAC->IP
	//Пара MAC->IP отправителя не обнаружена
	if (!mac_sender && (mac_sender == ip_sender)){
		if (VAR_SILENT_MODE) return;
		bad_sender = true;
	}

	//Событие: смена IP адреса зарегистрированной машиной
	//MAC адрес отправителя указан в списке пар, но IP адрес - нет
	if (mac_sender && (mac_sender != ip_sender)){
		if (VAR_SILENT_MODE) return;
		bad_sender = true;
	}

	//Событие: использование зарегистрированного IP адреса неизвестным устройством
	//IP адрес отправителя указан в списке пар, но MAC адрес - нет
	if (ip_sender && (mac_sender != ip_sender)) {
		if (VAR_SILENT_MODE) return;
		bad_sender = true;
	}

	//Если установлен признак проверки отправителя и получателя (VAR_DUPLEX_CHECK_MODE) и 
	//IP получателя отсутствует в списке пар - блокировать
	if (!ip_target) {
		if (!bad_sender && !VAR_DUPLEX_CHECK_MODE) return;
	}

	//Событие: блокировка пиратской(несанкционированной) пары MAC->IP
	if (bad_sender){
		
	}

	/* 	Блокировка пиратской пары:
		eth fake MAC -> pirate MAC
		sdr fake MAC - target IP
		trg pirate MAC - pirate IP
	*/
	memcpy(&mac_eth_target, &mac_eth_sender, sizeof(struct ether_addr));
	memcpy(&mac_eth_sender, &VAR_FAKE_MAC, sizeof(struct ether_addr));
	memcpy(&mac_arp_target, &mac_arp_sender, sizeof(struct ether_addr));
	memcpy(&mac_arp_sender, &VAR_FAKE_MAC, sizeof(struct ether_addr));
	memcpy(&ipsw, &ip_arp_target, sizeof(struct in_addr));
	memcpy(&ip_arp_target, &ip_arp_sender, sizeof(struct in_addr));
	memcpy(&ip_arp_sender, &ipsw, sizeof(struct in_addr));

	//Отправка ложных пакетов несанкционированному отправителю ARP пакетов
	f_arp_packet_send(ARPOP_REPLY, VAR_FAKE_REPLY_COUNT);

    if (bad_sender) {
		/* Контрольный выстрел ;)
			eth fake MAC -> pirate MAC
			sdr fake MAC - pirate IP
			trg pirate MAC - pirate IP
		*/
		memcpy(&ip_arp_sender, &ip_arp_target, sizeof(struct in_addr));
		f_arp_packet_send(ARPOP_REPLY, 1);
	}

	//Восстановление нарушенного ARP кеша посылкой broadcast
	if (bad_sender && VAR_FIX_BROADCCAST) {
		memcpy(&mac_eth_target, &mac_big, sizeof(struct ether_addr));
		memcpy(&mac_arp_target, &mac_zero, sizeof(struct ether_addr));
		if (ip_sender) {
			/* IP адрес отправителя корректен
				eth ethers MAC -> broadcast
				sdr ethers MAC - ethers IP
				trg zero MAC - ethers IP
			*/
			memcpy(&mac_eth_sender, &ip_sender->mac, sizeof(struct ether_addr));
			memcpy(&mac_arp_sender, &ip_sender->mac, sizeof(struct ether_addr));
			memcpy(&ip_arp_sender, &ip_sender->ip, sizeof(struct in_addr));
		} else if (mac_sender) {
			/* MAC адрес отправителя корректен
				eth ethers MAC -> broadcast
				sdr ethers MAC - ethers IP
				trg zero MAC - ethers IP
			** отключено, поскольку, вероятнее всего, бесполезно **
			memcpy(&mac_eth_sender, &mac_sender->mac, sizeof(struct ether_addr));
			memcpy(&mac_arp_sender, &mac_sender->mac, sizeof(struct ether_addr));
			memcpy(&ip_arp_sender, &mac_sender->ip, sizeof(struct in_addr));
			*/
		} else {
			/* Некорректный отправитель ARP
				eth fake MAC -> broadcast
				sdr fake MAC - pirate IP
				trg zero MAC - pirate IP
			*/
			memcpy(&mac_eth_sender, &VAR_FAKE_MAC, sizeof(struct ether_addr));
			memcpy(&mac_arp_sender, &VAR_FAKE_MAC, sizeof(struct ether_addr));
		}
		memcpy(&ip_arp_target, &ip_arp_sender, sizeof(struct in_addr));
		f_arp_packet_send(ARPOP_REQUEST, 1);
	}

	return;
}



/*Отправка ARP пакета*/
void 
f_arp_packet_send(int op, int num){

	register int n = num;

	if (libnet_build_arp(	ARPHRD_ETHER, ETHERTYPE_IP, 6, 4, op,
							(u_char *) &mac_arp_sender, (u_char *) &ip_arp_target,
							(u_char *) &mac_arp_target, (u_char *) &ip_arp_target,
							NULL, 0, lnet, 0) == -1)
	{
		printf("libnet_build_arp error: %s\n", libnet_geterror(lnet));
		exit(1);
	}

	if (libnet_build_ethernet(
			(u_char *) &mac_eth_target, (u_char *) &mac_eth_sender,
			ETHERTYPE_ARP, NULL, 0, lnet, 0) == -1) 
	{
		printf("libnet_build_ethernet error: %s\n", libnet_geterror(lnet));
		exit(1);
	}

	if (n > 0){
		while (1){
			if (libnet_write(lnet) == -1) {
				printf("libnet_build_ethernet error: %s\n", libnet_geterror(lnet));
				exit(1);
			}
			if (--n <= 0) break; else usleep(VAR_FAKE_REPLY_TIMEO * 1000);
		}
	}

	libnet_clear_packet(lnet);
}




/*Получение информации о MAC->IP адресе сервера*/
void 
f_arp_servinfo_init(void){

	struct libnet_ether_addr *ha = NULL;
	u_long pa = 0;

	SYS_MACIP_INFO = f_list_macip_item_create();

	//Получение собственного MAC адреса
	if((ha = libnet_get_hwaddr(lnet)) == NULL) {
		printf("libnet_get_hwaddr error: %s\n", libnet_geterror(lnet));
		exit(1);
	}

	//Получение собственного IP адреса
	if (!(pa = libnet_get_ipaddr4(lnet))) {
		printf("libnet_get_ipaddr4 error: %s\n", libnet_geterror(lnet));
		exit(1);
	}

	memcpy(&SYS_MACIP_INFO->mac, ha, sizeof(struct ether_addr));
	SYS_MACIP_INFO->ip_from = ntohl(pa);
	SYS_MACIP_INFO->ip_to = SYS_MACIP_INFO->ip_from;

	//Добавление собственной пары MAC->IP в список
	f_list_macip_add(VAR_ARP_PAIRS, SYS_MACIP_INFO->mac, SYS_MACIP_INFO->ip_from, SYS_MACIP_INFO->ip_to, false);

	return;
}






