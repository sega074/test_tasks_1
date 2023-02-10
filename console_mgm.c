/*
 * Здесь определяем действия выполняемы при приме команд с консоли
 * 
 */

                                    
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/param.h>              // для MIN

#include "define_param.h"

int console_mgm(param_unit *uni ,  array_unit* arr_uni){
                    // param_unit*   uni   - в данной структуре хранятся нужные для работы данные 
                    // возвращает 0 - успешное завершение  иначе ошибка
    
    
    
    char cmd[16];
    char cmd_in[16];
    
    
    
    
    memset(uni->console_bf,0, DEFAULT_INSTR);
    
    
    if (fgets(uni->console_bf, DEFAULT_INSTR, stdin) == NULL) {

        return -1;

    }

    //printf("console_bf size %i \n", strlen(uni->console_bf));
    
    
    if (strlen(uni->console_bf) == 1 ){     // принята пустая строка 
        
        if(uni->is_print){
            uni->is_print = 0;
            printf("вход в режим воода данных\n");
            if(uni->is_panel){
                printf("Командвы quit - завершение работы; set температура освещенность - установка значеий; help - вывод справки \n");
            } else {
                printf("Командвы quit - завершение работы; text текст для ввывода - установка текста; help - вывод справки \n");
            }
        } else {
            uni->is_print = 1;
            printf("выход из режим воода данных\n");
        }
        return 0;
    }
    
    
    // определить команду 
    
    strncpy(cmd_in,uni->console_bf,16);                             // скопировать для распознавания команды

    
    if (sscanf(cmd_in,"%s", cmd) == EOF){
        
        printf("Eror input command \n");
        if(uni->is_panel){
            printf("Командвы quit - завершение работы; set температура освещенность - установка значеий; help - вывод справки \n");
        } else {
            printf("Командвы quit - завершение работы; text текст для ввывода - установка текста; help - вывод справки \n");
        }
       
        return 0;
    }
    
    
    if (strncmp("quit", cmd, MIN(strlen("quit"),strlen(cmd))) == 0){
        
        uni->is_work = 0;                       // завершение работы
       
        return 0;
    }
    
    if (uni->is_panel == 1 || strncmp("set", cmd, MIN(strlen("set"),strlen(cmd))) == 0 ){
        
        memset(cmd,0,16);
        float temp;
        float illu;
        if (sscanf(uni->console_bf,"%s %f %f",cmd,&temp, &illu) == EOF){
            printf("Eror parsed cmd \n");
            if(uni->is_panel){
                printf("Командвы quit - завершение работы; set температура освещенность - установка значеий; help - вывод справки \n");
            } else {
                printf("Командвы quit - завершение работы; text текст для ввывода - установка текста; help - вывод справки \n");
            }
          
            return 0;
        } else {
            
            uni->cur_temperature = temp;        // установка температуры
            uni->cur_illumination = illu;       // установка уровня освещенности
            
          
            return 0;
        }
    }
    
    if (strncmp("text", cmd, MIN(strlen("text"),strlen(cmd))) == 0){
        
         memset(cmd,0,16);
        
        if (strlen(uni->console_bf) < LEM_MESAGE && sscanf(uni->console_bf,"%s %s",cmd,uni->text) == EOF){
            printf("Eror parsed cmd \n");
            if(uni->is_panel){
                printf("Командвы quit - завершение работы; set температура освещенность - установка значеий; help - вывод справки \n");
            } else {
                printf("Командвы quit - завершение работы; text текст для ввывода - установка текста; help - вывод справки \n");
            }
            
            return 0;
        } else {
            
           
            return 0;
        }
    }
    
    if (strncmp("help", cmd, MIN(strlen("help"),strlen(cmd))) == 0){
        
        if(uni->is_panel){
            printf("Командвы quit - завершение работы; set температура освещенность - установка значеий; help - вывод справки \n");
        } else {
            printf("Командвы quit - завершение работы; text текст для ввывода - установка текста; help - вывод справки \n");
        }
       
        return 0;
        
        
    }
    
    return 0;
}
