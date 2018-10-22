#ifndef libCPU_H
#define libCPU_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include "../../sample-socket/socket.h" 
#include "../../Utils/gestionArchConf.h"

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

int quantum;
int idCPU;

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
	FIN //para fin de script
}palabraReservada_t;

typedef struct{   
    palabraReservada_t palabraReservada; //concentrar
    char* p1; //abrir, wait, signal, flush, close, borrar
    char* p2; //crear
    char* p3; //asignar
}operacion_t;

typedef struct{
	int idGDT;
	FILE* scriptf;
}scriptGDT;

//VARIABLES GLOBALES
t_log* logger;
t_config* archivo_Config;
t_config_CPU* datosCPU;
t_dictionary * callableRemoteFunctionsCPU;
pthread_mutex_t m_main;
pthread_mutex_t m_busqueda;
pthread_mutex_t m_listaScriptsGDT;
int idGDTScriptABuscar;

t_list* listaScriptsGDT;

//VAR GLOB SOCKETS
int socketDAM;
int socketSAFA;
int socketFM9;

//FUNCIONES

void configure_logger();
t_config_CPU* read_and_log_config(char*);
void close_logger();

void* intentandoConexionConSAFA(int* );
void* intentandoConexionConDAM(int* );
void* intentandoConexionConFM9(int* );
void disconnect();

scriptGDT* verificarSiYaSeAbrioElScript(int idGDT, char* ruta);
bool closureBusquedaScript(scriptGDT* el);
FILE * abrirScript(char * scriptFilename);
operacion_t obtenerSentenciaParseada(FILE* script);

//PARSER
operacion_t parse(char* line);

//callable remote functions
void permisoConcedidoParaEjecutar(socket_connection * connection ,char** args); //SAFA
void establecerQuantumYID(socket_connection * connection ,char** args); //SAFA


#endif
