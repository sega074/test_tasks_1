/*
 * Здесь описывается код функции отправляющей запрос для получения данных с панелей
 * запрос может отправляться как контроллером так и панелью выполняющей функции контроллера
 * 
 */


#include <string.h>                 // в частности и для memeset

#include <stdio.h>                  // в частности для perror


#include <sys/types.h>              // для работы с сокетами 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/param.h>              // для MIN

#include "define_param.h"

int send_req(param_unit* uni , array_unit* arr_uni) {
                // param_unit*   uni   - в данной структуре хранятся нужные для работы данные
                    // array_unit* arr_uni - массив памяти хранящий параметры узлов
                    // возвращает 0 - успешное завершение  иначе ошибка
                    // -1 - ошибка в получении фрейма переданных данных recvfrom
                    // -2 - получен неизвестный кадр
                    // -3 - несмогли отправить сообшщение
                    // -4 - включена обратная петля
    
    
    
       // uni->id_req =  incr_uint(  &(uni->count_req));                          // установить номер зпроса принятого на обработку 
                
// произвести расчет среднего значения для запроса
               
                
        float tt = 0;   // среднее значение для температуры
        float il = 0;   // среденне значение для освещенности 
        int j = 0;      // количестов элементов
                
        for (int i = 1 ; i < uni->nunit; ++i){
            if (arr_uni[i].tick_silence !=  -1 ) {
                tt += arr_uni[i].cur_temperature;
                il += arr_uni[i].cur_illumination;
                j++;
            }
        }
                
        tt = j != 0 ? tt/(float)j : uni->cur_temperature;                                  // Средние значения параметров
        il = j != 0 ? il/(float)j : uni->cur_illumination;
                
// выполнить операции как будто принялиответ отсебя ( перенести это после формироования запроса)
// это относится и к контроллеру
                
        arr_uni[uni->id_num].tick_silence     = 0;                                   // сбросить счетчик тиков молчания
        arr_uni[uni->id_num].id_num           = uni->id_num;                         // идетификатор системы отправившей сообщение
        arr_uni[uni->id_num].id_num_cnt       = uni->id_num_cnt;                     // идентификатор системы на чей запрос этот ответ
        arr_uni[uni->id_num].cur_temperature  = uni->cur_temperature;                // уст текущего
        arr_uni[uni->id_num].cur_illumination = uni->cur_illumination;               // уст текущего
        
        uni->ave_temperature =  tt;                                                 // уст среднее  в струкутру
        uni->ave_illumination = il;
                
                
// отправить запрос
               
        memset (uni-> pkg_send, 0,LEN_PKG);                                          // подготовка буфера для передачи
                
        ((req_msg*)(uni->pkg_send))->type =  ID_CADR_REQ ;                          // идетификатор запроса
        ((req_msg*)(uni->pkg_send))->id_num = uni->id_num;                          // идетификатор системы отправившей сообщение 
        ((req_msg*)(uni->pkg_send))->ave_temperature = tt;                          // в запросе отправляется среденне вычисленное значение
        ((req_msg*)(uni->pkg_send))->ave_illumination = il;                         // 
                
        if (uni->id_num_cnt == 0){  // если контроллер то отправляем и транспорант                                                              
            strncpy(((req_msg*)(uni->pkg_send))->text, uni->text, MIN(LEM_MESAGE, strlen(uni->text)) );   // скопироватьотображаемый текст
        }
                
        struct sockaddr_in multi_addr_snd;                                          // целевой ip для могоадресной передачи

        memset(&multi_addr_snd, 0, sizeof(multi_addr_snd));                         // подготовка к использованию

// инициализация целевой IP-информации

        multi_addr_snd.sin_family = AF_INET;
        multi_addr_snd.sin_addr.s_addr = inet_addr(uni->group_adr); // адрес назначения является адресом многоадресной рассылки
        multi_addr_snd.sin_port = htons((uint16_t)(uni->port));   // Порт многоадресного сервера также 8000
                
        int n_snd = sendto(uni->sockfd, uni->pkg_send, sizeof(req_msg), 0,(struct sockaddr*)&multi_addr_snd, sizeof(multi_addr_snd));
                
        if (n_snd == -1) {
            perror("Error: when calling the function sendto");
            return  -3;
        }
                
        return 0;
}
