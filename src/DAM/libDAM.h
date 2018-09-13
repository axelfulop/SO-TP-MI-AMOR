#ifndef libDAM_H_
#define libDAM_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/dictionary.h>
#include "../sample-socket/socket.h"

//ESTRUCTURAS
typedef struct {
	int puertoEscucha;
	char* IPSAFA;
	int puertoSAFA;
    char* IPMDJ;
    int puertoMDJ;
    char* IPFM9;
    int puertoFM9;
    int transferSize;
} t_config_DAM;

//VARIABLES
t_log* logger;
t_config_DAM* _datosDAM;

int socketSAFA;


//FUNCIONES
//LOGS
void configure_logger();
t_config_DAM* read_and_log_config(char*);
void close_logger();

//CONFIG

#endif