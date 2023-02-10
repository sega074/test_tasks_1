/*
 * Здесь определяем действия выполняемы при приме сообщения
 * 
 */

 

#include <string.h>         // в частности и для memeset

#include <stdio.h>          // в частности для perror


#include <sys/types.h>  // для работы с сокетами 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/param.h>              // для MIN

#include "define_param.h"

int receive_msg(param_unit *uni , array_unit* arr_uni){
                    // param_unit*   uni   - в данной структуре хранятся нужные для работы данные
                    // array_unit* arr_uni - массив памяти хранящий параметры узлов
                    // возвращает 0 - успешное завершение  иначе ошибка
                    // -1 - ошибка в получении фрейма переданных данных recvfrom
                    // -2 - получен неизвестный кадр
                    // -3 - несмогли отправить сообшщение
                    // -4 - включена обратная петля
    
   
    
    
        socklen_t addr_len = 0;     // в документации говрится что длина адреса для корреспондента должна передавться по ссылке ( 
        
        memset (uni-> pkg_rcv, 0,LEN_PKG);                                                      // подготовка буфера для приема
      
        int n_rcv = recvfrom(uni->sockfd, uni->pkg_rcv, LEN_PKG, 0, NULL, &addr_len);       // получить данные пердполоается что данные будут без блокировки
                                                                                                // в далнейшем установить это с использованием fctl()
        if (n_rcv == -1) {
            perror("Error: when calling the function recvfrom");
            return  -1;
        }
       
     //  printf("n_rcv = %i sz_m = %i   sz_r = %i type = %x \n",n_rcv, sizeof(ans_msg), sizeof(req_msg), ((ans_msg*)(uni->pkg_rcv))->type);
        
// СООБЩЕНИЕ
        if (n_rcv == sizeof(ans_msg) &&  ((ans_msg*)(uni->pkg_rcv))->type == ID_CADR_ANS){          // получен ответ от одного из табло
            // просто обновляем таблицу ответов
            
            if(((ans_msg*)(uni->pkg_rcv))->id_num < uni->nunit) {                   // проверить на вхождение в дипазон
                
            // сохранить полученный ответ в своем массиве
                
                arr_uni[((ans_msg*)(uni->pkg_rcv))->id_num].tick_silence     = 0;                                               // сбросить таймер молчания для данной подсистемы
                arr_uni[((ans_msg*)(uni->pkg_rcv))->id_num].id_num           = ((ans_msg*)(uni->pkg_rcv))->id_num;              // идетификатор системы отправившей сообщение
                arr_uni[((ans_msg*)(uni->pkg_rcv))->id_num].id_num_cnt       = ((ans_msg*)(uni->pkg_rcv))->id_num_cnt;          // идентификатор системы на чей запрос этот ответ
                arr_uni[((ans_msg*)(uni->pkg_rcv))->id_num].cur_temperature  = ((ans_msg*)(uni->pkg_rcv))->cur_temperature;     // текущее значение температуры 
                arr_uni[((ans_msg*)(uni->pkg_rcv))->id_num].cur_illumination = ((ans_msg*)(uni->pkg_rcv))->cur_illumination;    // текущее значение осещенности 
                   
                
                // проверить на конфликт 
                // если идентификатор системы отправившей ответ совпадает с идентификатором данной системой, то надо принимать решение (случайно) отказаться от своего 
                // идентификатора или продолжить работу  (установлено что отправленные сообщения даненой системой не попадают на данный интерфайс) и пусть другая систем примет случайное решение
                
                if ( uni->is_panel  &&  uni->id_num == ((ans_msg*)(uni->pkg_rcv))->id_num  &&  bool_rand()){    // если это панель !
                    // произвести сброс идентификатора системы 
                    uni->id_num = -1;                                   // установить признак что идентификатор данного узла  не определен
                    //uni->count_req = 0;                                 // сбросить счетчики идентификатора запроса (используется к огда панель всает вместо контроллера )
                }
                
                
                return 0;
                
            } else {     // принятый номер устройства не входит в диапазон
                
                perror("Waring: this device has an incorrect configuration for the number of nodes");
                return 0;   // просто отбрасываем ( это не достатки конфигурации 
            }
            
// ЗАПРОС Которрый принимает панель
        } else if ( uni->is_panel && n_rcv == sizeof(req_msg)  
                    &&  ((req_msg*)(uni->pkg_rcv))->type == ID_CADR_REQ ) {
            
            if ( ((req_msg*)(uni->pkg_rcv))->id_num != 0 ) {                                // получен запрос от панели которая встала вместо контроллера
                                                                                            // контроллеру это не нужно
                        
                if (uni->id_num == -1  ){                                                   // если у локального устройства нет идентификатора
                                                                                            // эдействительно для только что включенной панели
                    return 0;
                }
            
            
                int candidat = 1 ;                                                          // текущая панель будет кандидатом стать ведущей панелью
            
                for (int i = 0; i < uni->id_num; ++i){
                    if (arr_uni[i].tick_silence != -1){
                        candidat = 0;
                        break;
                    }
                }
            
                if (candidat  && uni->id_num < ((req_msg*)(uni->pkg_rcv))->id_num ) {       // если запрос пришол не от контроллера а от панели
                                                                                            // и есть резон локальному стать управляющим
                                                                                            // запросы от самого себя не должны поступать петля запрещена
                                                                                        
                    uni->id_num_cnt = uni->id_num;                                          // сделать себя ведущим 
                
          
                    return  send_req(uni , arr_uni);
                
                } 
            }
            
    // просто принять данные из запроса ( не менять своего состояния
                
                uni->id_num_cnt = ((req_msg*)(uni->pkg_rcv))->id_num;                   // установить номер ведущего ( откогопринят запрос данных)
                    
                uni->ave_temperature = ((req_msg*)(uni->pkg_rcv))->ave_temperature;     // установить среднее значение температуры
                    
                uni->ave_illumination = ((req_msg*)(uni->pkg_rcv))->ave_illumination;   // установить среденне значение освещенности
                
              
                    
                if (uni->id_num_cnt == 0){                                              // если это запрос от пульта уст. текст и сброси его счетчик молчания
                    memset(uni->text,0,LEM_MESAGE);
                    memcpy(uni->text,((req_msg*)(uni->pkg_rcv))->text , MIN(LEM_MESAGE-1, strlen(((req_msg*)(uni->pkg_rcv))->text)));   // скопироватьотображаемый текст
                }    
                arr_uni[uni->id_num_cnt].tick_silence = 0;                                      // сбросить счетчик молчания для контроллера (который запросил данные)
                arr_uni[uni->id_num].tick_silence = 0;                                          // сбрпосить счетчик для себя
                    
                
                
            // сформировать ответ на запрос
            
                memset (uni->pkg_send, 0,LEN_PKG);                                              // подготовка буфера для передачи
            
                ((ans_msg*)(uni->pkg_send))->type               =  ID_CADR_ANS;                 // идетификатор ответа
                ((ans_msg*)(uni->pkg_send))->id_num             = uni->id_num;                  // идетификатор системы отправившей сообщение 
                ((ans_msg*)(uni->pkg_send))->id_num_cnt         = uni->id_num_cnt;              // идентификатор системы откоторой пришол запрос
                ((ans_msg*)(uni->pkg_send))->cur_temperature    = uni->cur_temperature;         // текущее значение температуры 
                ((ans_msg*)(uni->pkg_send))->cur_illumination   = uni->cur_illumination;        // текущее значение освещенности
                
                struct sockaddr_in multi_addr_snd; 												// целевой ip для могоадресной передачи

                memset(&multi_addr_snd, 0, sizeof(multi_addr_snd));								// подготовка к использованию

                // инициализация целевой IP-информации

                multi_addr_snd.sin_family = AF_INET;
                multi_addr_snd.sin_addr.s_addr = inet_addr(uni->group_adr); // адрес назначения является адресом многоадресной рассылки
                multi_addr_snd.sin_port = htons((u_int16_t)(uni->port));   // Порт многоадресного сервера также 8000
                                
                 int n_snd = sendto(uni->sockfd, uni->pkg_send, sizeof(ans_msg), 0,(struct sockaddr*)&multi_addr_snd, sizeof(multi_addr_snd));
                
                 if (n_snd == -1) {
                    perror("Error: when calling the function sendto");
                    return  -3;
                }
                
                return 0;
               
 // ЗАПРОС Которрый дошол до контроллера         
        } else if (n_rcv == sizeof(req_msg)  
                    &&  ((req_msg*)(uni->pkg_rcv))->type == ID_CADR_REQ){
                
// ЧТОТО ИНОЕ        
        } else {                                                                                    // получено чтото другое
            perror("Error: the accepted data type is not defined");
            return  -2;
        }
    
    return 0;
}
