#ifndef libCPU_H
#define libCPU_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/dictionary.h>
#include "../../sample-socket/socket.h" 
#include "../../Utils/gestionArchConf.h"
#include "../../Utils/gestionProcesos.h"

//ESTRUCTURA
typedef struct {
	char* ipS;
	int puertoS;
	char*  ipD;
	int puertoD;
	char* ipF;
	int puertoF;
	int retardo;
} t_config_CPU;

//EJEMPLOS DE SENTENCIAS
/*
-abrir /equipos/Racing.txt
-concentrar
-asignar /equipos/Racing.txt 9 GustavoBou
-wait Conmebol
-signal Conmebol
-flush /equipos/Racing.txt
-close /equipos/Racing.txt
-crear /equipos/Racing.txt 11
-borrar /equipos/Racing.txt
*/

typedef enum{
	ABRIR, 
	CONCENTRAR, 
	ASIGNAR, 
	WAIT, 
	SIGNAL, 
	FLUSH, 
	CLOSE, 
	CREAR, 
	BORRAR, 
	NUMERAL, //para comentarios
	BLANCO  //para señalar fin de programa
}palabraReservada_t;

typedef struct{   
    palabraReservada_t palabraReservada; //concentrar
    char* p1; //abrir, wait, signal, flush, close, borrar
    char* p2; //crear
    char* p3; //asignar
}operacion_t;

//VARIABLES GLOBALES
t_log* logger;
t_config* archivo_Config;
t_config_CPU* datosCPU;
t_dictionary * callableRemoteFunctionsCPU;

//FUNCIONES

void configure_logger();
t_config_CPU* read_and_log_config(char*);
void close_logger();

void* intentandoConexionConSAFA(int* );
void* intentandoConexionConDAM(int* );
void* intentandoConexionConFM9(int* );

void SAFA_CPU_handshake(socket_connection *, char **);
void DAM_CPU_handshake(socket_connection *, char **);
void FM9_CPU_handshake(socket_connection *, char **);

void saliendo_por_error(int , char* , void* );
void disconnect();
void exit_gracefully(int );

//PARSER
operacion_t parse(char* line);


#endif
