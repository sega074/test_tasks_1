/*
 *  Здесь определяем действия выполняемы при наступлении кажого тика от интервального таймера
 * 
 */

#include <string.h>         // в частности и для memeset

#include <stdio.h>          // в частности для perror


#include <sys/types.h>  // для работы с сокетами 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>



#include "define_param.h"


int task_timer_tick( param_unit* uni , array_unit* arr_uni ){
                    
                    // param_unit*   uni   - в данной структуре хранятся нужные для раблоты данные 
                    // возвращает 0 - успешное завершение  иначе ошибка


    
// добавить тики в о все узлы на связи  и удалить всех кто отвалился
    for (int i = 0 ; i < uni->nunit ;++i){
        
        if (arr_uni[i].tick_silence != -1){
            
            ++arr_uni[i].tick_silence;              //отсчитать тик
            
            if ( i != uni->id_num                  // исключить себя из удаления 
                && (uni->set_tick_los+ 3 * uni->set_tick_rec <  arr_uni[i].tick_silence)){
                arr_uni[i].tick_silence= -1;        // отметить узды которые ушли со связи 
            }
        }
    }
    
 
 // если есть адрес у узла то проверить возможность занять место контроллера 
   if (uni->id_num != -1) {// есди адрес у данного юнита есть
       
        if ( (arr_uni[uni->id_num].tick_silence >= uni->set_tick_rec) && ((uni->id_num == 0) || (uni->id_num == uni->id_num_cnt))  ) {  // сонтроллер или ранель с функциями контроллера
            
           // printf("Send req id_num: %i : tick: %i\n", uni->id_num, arr_uni[uni->id_num].tick_silence );
            
            if ( send_req(uni, arr_uni )) {                              // послать запрос
                   perror("Eroor: send request from tick");
            }
  
        }
  
        if ( uni->is_panel && arr_uni[uni->id_num_cnt].tick_silence >= uni->set_tick_los){    // у контроллера и панели выполнябющей обязанности коннтроллера превышено врея потери соедиенения
                                                                            // попробовать заменить собой замещало то стаь главным 
            
           
            int candidat = 1 ;                                           // текущая панель будет кандидатом стать ведущей панелью
            
            for (int i = 0; i < uni->id_num; ++i){
                if (arr_uni[i].tick_silence != -1){
                    candidat = 0;
                    break;
                }
            }
            
            if (candidat ){                                             // стать ведущим панелью или контроллером
                uni->id_num_cnt = uni->id_num;
                if ( send_req(uni, arr_uni )) {                         // послать запрос
                    perror("Eroor: send request from tick");
                }
            }
        }
 
 // адреса у узла нет попробовать его получить
   } else {
       
        // если наступил тик отправки запроса ( оринтируемя по нему ) то присвоить самый младщий из свободных адресов 
       
        if (uni->is_panel){
       
            int id_free = uni->nunit ;              // максимальная величина узлов
            int id_use = uni->nunit ;               
       
            for (int i = 0 ; i < uni->nunit ;++i){                                  // пербрать весь список и вбрать первый свободный для получения id
                                                                                    // и первый занятый для определения отчета времени
                if(arr_uni[i].tick_silence == -1 && id_free == (uni->nunit ) && i > 0){
                    id_free = i;
                    continue;
                }
                
                if (arr_uni[i].tick_silence != -1 && id_use == (uni->nunit )){
                    id_use = i;
                }
            
                if (id_free != (uni->nunit ) && id_use != (uni->nunit )){
                    break;                                                          // нашли свободный и занятый
                }
            }
       
            if (id_use == uni->nunit ){             // нет подключений 
                uni->id_num = 1;                    // определить как первый
                uni->id_num_cnt = 1;
                arr_uni[1].tick_silence = 0;
            } else if( (id_free != uni->nunit) &&  (uni->set_tick_rec > 0) && (arr_uni[id_use].tick_silence >= (uni->set_tick_rec -1) ) ) {
                uni->id_num = id_free;
                uni->id_num_cnt = id_use;
                arr_uni[id_free].tick_silence = 0;
                
                printf("Set panel N %i \n",uni->id_num );
            }   // иначе неи чего не присваиваем так как нет свободных идентификаторов 
            
        } else {    // это контрполлер 
            
            uni->id_num     = 0;    // начальная установка 
            uni->id_num_cnt = 0;
            arr_uni[0].tick_silence = 0;
            printf("Set controller\n" );
            
        }
   }
   
// вывести    сообщение на консоль

    //printf("DEBUG is_print: %i tick_silence: %i \n",uni->is_print, arr_uni[uni->id_num].tick_silence );

    if (uni->is_print && arr_uni[uni->id_num].tick_silence == 1){
        if (uni->is_panel){
            printf ("Panel N: %i text: %s ave.temperature: %f; ave.illumination: %f; cur.temperature: %f; cur.illumination: %f \n",
                    uni->id_num,uni->text,uni->ave_temperature,uni->ave_illumination,uni->cur_temperature,uni->cur_illumination);
        } else {
            printf ("Controller: text: %s ave.temperature: %f; ave.illumination: %f\n",uni->text,uni->ave_temperature,uni->ave_illumination); 
        }
    }
   
     return 0;           
}
