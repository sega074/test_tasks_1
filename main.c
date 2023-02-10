/*!
 * Тестовое задание на языке C
 *
 */

#include <stdio.h>										// для разлиных консольных сообщений
#include <getopt.h>                                     // для чтения опций из ком. строки

#include <stdlib.h>										// для работы с сеткой
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

#include <signal.h>

#include <sys/time.h>

#include <sys/param.h>							// здесь используется для MIN MAX

#include <sys/poll.h>                           // для  создание возможности работать с одним потоком

#include <pthread.h>                            // видимо не простоуйти от потокв

#include <limits.h>                             // для получения коснтант огранияения типа - используем

#include "define_param.h"                       // определения приложения





// определение опций для разбора

const  struct option long_options[] = {
                {"srv" ,0,0,'s'},                                       // запустить в режиме контроллера
                {"madr",1,0,'a'},                                  	    // адрес для много адресной передачи
                {"port",1,0,'p'},                                    	// порт для многоадресной передачи
                {"szuni",1,0,'u'},                                      // допустимое количестов узлов в сети
                {"clkreq",1,0,'c'},                                     // период опроса данных с табло секунды
                {"lose",1,0,'l'},                                       // величина времени в втечении которой считается потерн контрллер секунды
				{"help",0,0,'h'},                                       // вывести список доступных параметров
                {0,0,0,0}                                               //завершающая строка
};


const char name_controller[]="contr_tab";					// имя ПО для запуска в режиме сервера



// для обработки сигнала от таймера 

volatile int coutn_alarm = 0;

 pthread_mutex_t mx = PTHREAD_MUTEX_INITIALIZER;



void alarm_handler(int sign){               // отсчитаь тик
    pthread_mutex_lock(&mx);
    if (coutn_alarm < INT_MAX){
        coutn_alarm++;
    }
    pthread_mutex_unlock(&mx);
    
  //  perror("timer tick from signal");
}


int get_and_decr(){                         // чтение факта тика  и декремент если есть факт 
    int ret;
    
    pthread_mutex_lock(&mx);
    
    ret = coutn_alarm;
    
    if (coutn_alarm > 0){
        -- coutn_alarm;
    }
    
    pthread_mutex_unlock(&mx);
    
    return ret;
}


// получение случайного логического значени 1 илми 0

int bool_rand(){
 
    return 10.0*rand()/(RAND_MAX+1.0) > 4.5 ? 1 : 0 ;       // это подссмотрено Numerical Recipes in C: The Art of Scientific Computing (William H. Press, Brian P. Flannery, Saul A. Teukolsky, William T. Vetterling; New York: Cambridge University Press, 1992 
    
}


int main(int argc, char **argv) {
    
    
    //int port;

	//char group_adr[LEN_GROUP_ADR];													// групповая IP-адреса
    
    // определение разделяемых данных
    
    int tmp_sz =  sizeof(param_unit);
    
    param_unit *uni = malloc(sizeof(param_unit));

    if (uni == NULL){
        perror("Eroor allocate parameter unit");
        exit (-1);
    }
    
     memset(uni, 0, sizeof(param_unit));

    // установить значения по умолчанию 
    
    uni->nunit = DEFAUL_UNIT_COUNT;                                    // количестов узлов по умолчанию 32
    uni->set_tick_rec = DEFAULT_TIME_REC;                              // по умолчанию период опроса 5 секунт
    uni->set_tick_los = DEFAULT_TIVE_LOS;                              // по умолчанию период потери управления от контроллера 50 секунд
    
   
    strncpy (uni->group_adr, DEFAULT_MULTI_ADDR, MIN(LEN_GROUP_ADR, strlen(DEFAULT_MULTI_ADDR)));	// адрес для много адресной передачи по умолчанию
    
    uni->port = DEFAULT_MULTI_PORT;                                                                 // порт для многоадресной по умолчанию
 
    uni->nunit =  DEFAUL_UNIT_COUNT;                                    // количестов узлов по умолчанию
    
    uni->set_tick_rec = DEFAULT_TIME_REC;                               // по умолчанию период опроса 5 секунт
    
    uni->set_tick_los = DEFAULT_TIVE_LOS;                               // по умолчанию период потери управления от контроллера 50 секунд
 

	uni->is_panel = 1;		                                            // определим чтего хотели запустить контроллер или табло
                                                                        // 0 - система работает в качестве контроллера 
                                                                        // 1 - система работает в качестве в качестве панели
                                                                                    
    uni->id_num = -1;                   // для панели пока неопределено
   
	
	

    // проверить необходимые внешние параметры
    int index_arg = 0;
    int ret_arg = 0;

    for (ret_arg = getopt_long(argc,argv,"s:a:p:u:c:l:h",long_options,&index_arg);
                    ret_arg != -1 ;
                    ret_arg = getopt_long(argc,argv,"s:a:p:u:c:l:h",long_options,&index_arg)){

            switch(ret_arg){
                
            case 's':
                    uni->is_panel = 0;	      // 0 - система работает в качестве контроллера 
                    uni->id_num = 0;          // для контроллера идентификатор адреса всегда 0
                    break;

            case 'a':
            		memset(uni->group_adr, 0, LEN_GROUP_ADR);
            		strncpy (uni->group_adr, optarg, MIN(LEN_GROUP_ADR, strlen(optarg)));	// адрес для много адресной передачи
            		if (strlen(uni->group_adr) == 0){
            			perror ("Waring parameter --madr or -a , set to default");
            			memset(uni->group_adr, 0, LEN_GROUP_ADR);
                        strncpy (uni->group_adr, DEFAULT_MULTI_ADDR, MIN(LEN_GROUP_ADR, strlen(DEFAULT_MULTI_ADDR)));	// адрес для много адресной передачи по умолчанию
            		}
                    break;

            case 'p':
                    uni->port = atoi(optarg);    											// порт для многоадресной передачи
                    if (uni->port <=0 || uni->port > 65535){
                    	perror ("Waring parameter --port or -p , set to default" );
                    	uni->port = DEFAULT_MULTI_PORT <0 || DEFAULT_MULTI_PORT > 65535 ? 20122 : DEFAULT_MULTI_PORT;
                    }
                    break;

           
            case 'u':
                    uni->nunit = atoi(optarg);    											// допустимое количестов узлов в сети
                    if ( uni->nunit <=0 || uni->nunit > MAX_UNIT_COUNT ){
                    	perror ("Waring parameter --szuni or -u out of range , set to default");
                    	uni->nunit = DEFAUL_UNIT_COUNT;                                        // количестов узлов по умолчанию 32
                    }
                    break;         
                    
            
            case 'c':
                    uni->set_tick_rec = atoi(optarg);    									  // период опроса данных с табло в секундах
                    if (uni->set_tick_rec <=0 || uni->set_tick_rec > MAX_TIME_REC){
                    	perror ("Waring parameter --clkreq or -c out of range, set to default");
                    	uni->set_tick_rec = DEFAULT_TIME_REC;                                  // по умолчанию период опроса 5 секунт
                    }
                    break;
                    
           
            case 'l':
                    uni->set_tick_los = atoi(optarg);    									  // допустимое время потери для контроллера в секундах
                    if (uni->set_tick_los <=0 || uni->set_tick_los > MAX_TIME_LOS){
                    	perror ("Waring parameter --lose or -l");
                    	uni->set_tick_los = DEFAULT_TIVE_LOS;                                  // по умолчанию период потери управления от контроллера 50 секунд
                    }
                    break;         

            case 'h':
                    printf (" Используйьте для программы  следующие параметры:\n");
                    printf ("              --srv или -s запуск в режиме контроллера \n");
                    printf ("              --madr или -a адрес группы многоадресной рассылки\n");                   // ip фдрес для групповых рассылок от 224.0.0.3- 224.0.0.255
                    printf ("              --port или -p порт приема групповых рассылок\n" );                                 
                    printf ("              --szuni или -u допустимое количестов узлов в сети\n" );  
                    printf ("              --clkreq или -с период опроса данных с табло в секундах\n" );  
                    printf ("              --lose bkb -l допустимое время потери для контроллера в секундах\n" );  
                    printf ("              --help вывод данной подсказки и звершение выполения программы\n");        // вывести список доступных параметров
                    free(uni);
                    exit(0);
            default:
            	perror ("Error parameter");
                free(uni);
                exit(0);
            }
    }


    // ропредление данных используемых  в алгоритмах работы системы
    
    array_unit *arr_unit = calloc(uni->nunit,sizeof(array_unit));
    
    if (arr_unit ==  NULL){
        free(uni);
        perror("Error allocate array for array_unit");
        exit (-1);
    }
    
    // изначально определить что в массиве нет  установленных данных
    
    for (int i = 0; i < uni->nunit; ++i){
        if (uni->id_num ==  i){
            arr_unit[i].tick_silence = 0;
        } else {
            arr_unit[i].tick_silence = -1;
        }
    }
    
    
    int err = -1;																	// для контроля ошибок и удобства в отладке


    // определить процесс для приема из сокета

	//int sockfd_rcv;																// сокет для приема

	struct sockaddr_in multi_addr_rcv;												// структра для приема сообщений


	uni->sockfd = socket(AF_INET, SOCK_DGRAM, 0);								     // создать сокет
	if (uni->sockfd < 0) {
	        perror("Error open socket for recive");
            free(arr_unit);
            free(uni);
	        exit(-1);
	}


    memset(&multi_addr_rcv,0, sizeof(multi_addr_rcv));								      // Инициализация информации, связанной с IP
    multi_addr_rcv.sin_family = AF_INET;
    multi_addr_rcv.sin_addr.s_addr = htonl(INADDR_ANY);
    multi_addr_rcv.sin_port = htons((uint16_t)uni->port);

    err = bind(uni->sockfd, (struct sockaddr*)&multi_addr_rcv, sizeof(multi_addr_rcv));	    // сокет привязки IP-адреса
    if (err < 0) {
        perror("Error execute bind");
        free(arr_unit);
        free(uni);
        exit(-1);
    }

    int loop =0;                                                                            // не будем получать отправленные через этот интерфейс данные ( так устроен алгоритм)
    
    err = setsockopt(uni->sockfd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));	     // запретить возвращение отправленного на данный интерфейс
    if (err < 0) {
           perror("Eroor setsockopt loop");
           free(arr_unit);
           free(uni);
           exit(-1);
     }

    struct ip_mreq mreq;														              // структура для многоадресного адреса

    mreq.imr_multiaddr.s_addr = inet_addr(uni->group_adr);						               // уст ip группы многоадресной рассылки ( из строки)

    mreq.imr_interface.s_addr = htonl(INADDR_ANY);									            // добавить адр. прослушивания ( из целого числа)

    err = setsockopt(uni->sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
    if (err < 0)
    {
        perror("setsockopt");
        free(arr_unit);
        free(uni);
        exit(-1);
    }



    // определить таймер
  
    signal (SIGALRM,alarm_handler);                             // установить обработку сигнала SiGALARM
  
    struct itimerval delay;
    delay.it_value.tv_sec = 1;                                  // устанвливаем секунду так как работаем с секундами
    delay.it_value.tv_usec = 0;
    delay.it_interval.tv_sec = 1;
    delay.it_interval.tv_usec = 0;                              // это в микросекундах
    
    err = setitimer(ITIMER_REAL,&delay,NULL);
    

    if (err){
        perror("Eroor set timer\n");
    }
  
  
  
    // запустить pool

        struct pollfd plfd[2];                                      //только сокет для чтения и консоль В/В
        
        
        plfd[0].fd = uni->sockfd;                                   // для приема сообщений из сокета
        plfd[0].events = POLLIN;                                    // установить признак достумности по чтеинию
        
        
        plfd[1].fd=STDIN_FILENO;                                    // это прием сообщений из консоли
        plfd[1].events = POLLIN;                                    // установить признак достумности по чтеинию
        
        
        uni->is_work = 1;                                           // установить признак работы системы
        
        uni->is_print = 1;                                          // установить признак печати собщений  при каждом тике
        
        while(uni->is_work){
        
            //err = poll (plfd,2,vl_tm.it_interval.tv_sec *1000);
            err = poll (plfd,2,2*1000 );                            // сторожевой таймер должен быть больше
        
            if (err == -1 && errno != EINTR){                       // разбор ошибки
                perror("Eroor poll execute");
                free(arr_unit);
                free(uni);
                exit(-1);
            }

            // цепочка обработки заданий 
            
//             
            if (get_and_decr() > 0){
                
                if ( task_timer_tick( uni,arr_unit)){                   // задачи выполняемые по каждому тику
                    // если чтото пошло не так
                     perror("Eroor poll task_timer_tick");
                }
                
            }
            
            
            if (plfd[0].revents != 0){
                if(receive_msg(uni,arr_unit)) {                         // действия по обработки принятых данных
                    // если что то пошло нетак
                    perror("Eroor poll task_timer_tick");
                }
            }

            if (plfd[1].revents != 0){
                if (console_mgm(uni,arr_unit)){                         // действия по обработке консоли
                    // если чтото пошло неетак
                }
            }
            
            // проека и обработка таймера
        }

    // действия по закрытию и завершениею
    
        // при закрытии приложения сокет освобоится от группового адреса сам 
        
            free(arr_unit);
            free(uni);
    
        
	return 0;
}
