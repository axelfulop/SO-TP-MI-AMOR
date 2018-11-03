#include <stdio.h>
#include <stdlib.h>
#include "libSAFA.h"


void encolarDTB(t_queue* c, DTB* d, pthread_mutex_t m){
	pthread_mutex_lock(&m);
    queue_push(c, d);
    pthread_mutex_unlock(&m);
}

DTB* desencolarDTB(t_queue* c, pthread_mutex_t m){
	pthread_mutex_lock(&m);
    DTB* d = queue_pop(c);
    pthread_mutex_unlock(&m);
	return d;
}

CPU* buscarCPU(int id){
	pthread_mutex_lock(&m_busqueda);
	idCpuABuscar = id;
	CPU* cpu = list_find(listaCPUs, (void*)closureIdCPU);
	pthread_mutex_unlock(&m_busqueda);
	return cpu;
}

bool closureIdCPU(CPU* cpu){
	return cpu->id == idCpuABuscar;
}

recurso* buscarRecurso(char* nombre){
	pthread_mutex_lock(&m_busqueda);
	nombreRecursoABuscar = malloc(strlen(nombre)+1);
	strcpy(nombreRecursoABuscar, nombre);
	recurso* r = list_find(listaDeRecursos, (void*)&closureBusquedaRecurso);
	free(nombreRecursoABuscar);
	pthread_mutex_unlock(&m_busqueda);
	return r;
}

bool closureBusquedaRecurso(void* r){
	recurso* rec = (recurso*) r;
	return strcmp(rec->nombre, nombreRecursoABuscar) == 0;
}

void destruirRecurso(recurso* r){
        free(r->nombre);
        list_destroy(r->GDTsEsperandoRecurso);
        free(r);
}

void crearRecurso(char* nombre, int valor){
	recurso* rec = malloc(sizeof(recurso));
	rec->nombre = malloc(strlen(nombre) + 1);
	strcpy(rec->nombre, nombre);
	rec->valor = valor;
	rec->GDTsEsperandoRecurso = list_create();
	pthread_mutex_lock(&m_listaDeRecursos);
	list_add(listaDeRecursos, rec);
	pthread_mutex_unlock(&m_listaDeRecursos);
}

//de la listaDeRecursos
void eliminarRecurso(recurso* r){
		pthread_mutex_lock(&m_busqueda);
		nombreRecursoABuscar = malloc(strlen(r->nombre)+1);
		strcpy(nombreRecursoABuscar, r->nombre);
		list_remove_and_destroy_by_condition(listaDeRecursos, &closureBusquedaRecurso,  (void*)&destruirRecurso);
		free(nombreRecursoABuscar);
		pthread_mutex_unlock(&m_busqueda);
}

DTB* buscarDTB(t_list* lista,int id){
	pthread_mutex_lock(&m_busqueda);
	DTB* p ;
	t_link_element* pElem = lista -> head ; 
	while(pElem != NULL){

		p = (DTB*)(pElem->data);
		if(id == p->id){
			pthread_mutex_unlock(&m_busqueda);
			return p;
		}
		pElem = pElem->next;
	}
	pthread_mutex_unlock(&m_busqueda);
	return NULL;
}

//Se usa sólo en la funcion FINALIZAR de la consola del SAFA
DTB * quitarDTBDeSuListaActual(int idDTB)
{
  DTB * dtb = NULL;

  pthread_mutex_lock(&m_listaEjecutando);
  dtb = get_and_remove_DTB_by_ID( listaEjecutando, idDTB);
  pthread_mutex_unlock(&m_listaEjecutando);

  if(!dtb)
  {
    pthread_mutex_lock(&m_colaReady);
    dtb = get_and_remove_DTB_by_ID( colaReady->elements, idDTB);
    pthread_mutex_unlock(&m_colaReady);
    //if(dtb) sem_wait(&cantProcesosEnReady); //cuando ejecuto el comando finalizar idGDT, se queda bloqueado
  }											//en este semáforo!!!!!!!

  if(!dtb)
  {
    pthread_mutex_lock(&m_colaBloqueados);
    dtb = get_and_remove_DTB_by_ID( colaBloqueados->elements, idDTB);
    pthread_mutex_unlock(&m_colaBloqueados); 
  }

  return dtb;
}

DTB* get_and_remove_DTB_by_ID( t_list * lista, int id )
{
  t_link_element * pAct = lista->head , * pPrev = NULL;
  DTB* dtb;

  while( pAct != NULL )
  {
    dtb = (DTB *)(pAct->data);

    if( dtb->id == id )
    {
      if(!pPrev)
        lista->head = pAct->next;
      else
        pPrev->next = pAct->next;

	  lista->elements_count--;
      return dtb;
    }
    pPrev = pAct;
    pAct = pAct->next;
  }

  return NULL;
}

DTB* buscarDTBEnElSistema(int idGDT){
	DTB* dtb;
	dtb = buscarDTB(colaNew->elements, idGDT);
	if(dtb == NULL)
		dtb = buscarDTB(colaReady->elements, idGDT);
	if(dtb == NULL)
		dtb = buscarDTB(colaBloqueados->elements, idGDT);
	if(dtb == NULL)
		dtb = buscarDTB(listaEjecutando, idGDT);
	if(dtb == NULL)
		dtb = buscarDTB(colaFinalizados->elements, idGDT);

	return dtb;
}

//LOG
void configure_logger() {

	char * nombrePrograma = "SAFA.log";
	char * nombreArchivo = "SAFA";

	logger = log_create(nombrePrograma, nombreArchivo, true, LOG_LEVEL_TRACE);
	log_info(logger, "Se genero log de S-AFA");
}

void close_logger() {
	log_info(logger, "Cierro log de S-AFA");
	log_destroy(logger);
}

//CONFIG
t_config_SAFA * read_and_log_config(char* path) {
	log_info(logger, "Voy a leer el archivo S-AFA.config");

	t_config* archivo_Config = config_create(path);
	if (archivo_Config == NULL) {
		log_error(logger, "No existe archivo de configuracion");
		exit(1);
	}

	datosConfigSAFA = malloc(sizeof(t_config_SAFA));

	datosConfigSAFA->puerto = config_get_int_value(archivo_Config, "S-AFA_PUERTO");
	datosConfigSAFA->algoritmoPlanif = malloc(strlen(config_get_string_value(archivo_Config, "ALGORITMO_PLANIF")) + 1);
	strcpy(datosConfigSAFA->algoritmoPlanif, config_get_string_value(archivo_Config, "ALGORITMO_PLANIF"));
	datosConfigSAFA->quantum = config_get_int_value(archivo_Config, "QUANTUM");
	datosConfigSAFA->gradoMultiprog = config_get_int_value(archivo_Config, "GRADO_MULTIPROG");
	datosConfigSAFA->retardo = config_get_int_value(archivo_Config, "RETARDO");

	log_info(logger, "	PUERTO: %d", datosConfigSAFA->puerto);
	log_info(logger, "	ALGORITMO_PLANIFICACION: %s", datosConfigSAFA->algoritmoPlanif);
	log_info(logger, "	QUANTUM: %d", datosConfigSAFA->quantum);
	log_info(logger, "	GRADO_MULTIPROGRAMACION: %d", datosConfigSAFA->gradoMultiprog);
	log_info(logger, "	RETARDO: %d", datosConfigSAFA->retardo);

	log_info(logger, "Fin de lectura");

	config_destroy(archivo_Config);
       return datosConfigSAFA;
 
} // al final de esta funcion me queda la variable datosConfigSAFA con la config de SAFA

//CallableRemoteFunctions

//llamada por CPU al haber una sentencia de wait
//msg[0]: idCPU, msg[1]: idGDT, msg[2]: nombreRecurso, msg[3]: cantQuantoQueEjecutó, msg[4]:quantumAEjecutar de CPU
void waitRecurso(socket_connection* socketInfo, char** msg){
	recurso* rec = buscarRecurso(msg[1]);
	int idDTB = atoi(msg[1]);
	int idCPU = atoi(msg[0]);
	int quantumDesignadoACPU = atoi(msg[4]);
	int cantQuantoEjecutado = atoi(msg[3]);
	DTB* dtb = buscarDTBEnElSistema(idDTB);
	CPU* cpu = buscarCPU(idCPU);
	if(rec == NULL){ // si no existe
		int quantumEjecutado = atoi(msg[3]);
		crearRecurso(msg[2], 1);

		if(dtb != NULL){ 
			dtb->quantumFaltante = datosConfigSAFA->quantum - quantumEjecutado;
			dtb->PC += quantumEjecutado;
		}
		//ejecuta el mismo GDT
        char string_flagInicializacion[2];
        sprintf(string_flagInicializacion, "%i", dtb->flagInicializado); 
        char string_pc[2];
        sprintf(string_pc, "%i", dtb->PC);
        char string_quantumAEjecutar[2];
        sprintf(string_quantumAEjecutar, "%i", quantumDesignadoACPU - cantQuantoEjecutado);
		if(cpu!=NULL)
        	runFunction(cpu->socket,"ejecutarCPU",5, msg[1],
            										dtb->rutaScript,
													string_pc,
                                                	string_flagInicializacion,
                                                	string_quantumAEjecutar);
	}
	else{ //si existe
		pthread_mutex_lock(&m_recurso);
		rec->valor--;
		if(rec->valor < 0){
			pthread_mutex_unlock(&m_recurso);
			char** params = (char*[]){msg[0], msg[1], msg[3], "bloquear" };
			finalizacionProcesamientoCPU(NULL, params);
		}
		else{
			pthread_mutex_unlock(&m_recurso);
			int quantumEjecutado = atoi(msg[3]);
			if(dtb != NULL){ 
				dtb->quantumFaltante = datosConfigSAFA->quantum - quantumEjecutado;
				dtb->PC += quantumEjecutado;
			}
			//ejecuta el mismo GDT
     		char string_flagInicializacion[2];
        	sprintf(string_flagInicializacion, "%i", dtb->flagInicializado); 
        	char string_pc[2];
        	sprintf(string_pc, "%i", dtb->PC);
        	char string_quantumAEjecutar[2];
        	sprintf(string_quantumAEjecutar, "%i", quantumDesignadoACPU - cantQuantoEjecutado);
			if(cpu!=NULL)
        		runFunction(cpu->socket,"ejecutarCPU",5, msg[1],
            											dtb->rutaScript,
														string_pc,
                                                		string_flagInicializacion,
                                                		string_quantumAEjecutar);
		}

	}
}

//llamada por CPU al haber una sentencia de signal
//msg[0]: idCPU, msg[1]: idGDT, msg[2]: nombreRecurso, msg[3]: cantQuantoQueEjecutó, msg[4]:quantumAEjecutar de CPU
void signalRecurso(socket_connection* socketInfo, char** msg){
	recurso* rec = buscarRecurso(msg[2]);
	int idCPU = atoi(msg[0]);
	int idDTB = atoi(msg[1]);
	int quantumDesignadoACPU = atoi(msg[4]);
	int cantQuantoEjecutado = atoi(msg[3]);
	CPU* cpu = buscarCPU(idCPU);
	DTB* dtb = buscarDTBEnElSistema(idDTB);
	if(rec == NULL){ // si no existe
		crearRecurso(msg[2], 1);
	}
	else{ //si existe
		pthread_mutex_lock(&m_recurso);
		rec->valor++;
		pthread_mutex_unlock(&m_recurso);
		pthread_mutex_lock(&m_listaDeRecursos);
		DTB* dtbADesbloquear = list_get(rec->GDTsEsperandoRecurso, 0);
		pthread_mutex_unlock(&m_listaDeRecursos);
		if(dtbADesbloquear != NULL){
			pthread_mutex_lock(&m_colaBloqueados);
			get_and_remove_DTB_by_ID(colaBloqueados->elements, dtbADesbloquear->id);
			pthread_mutex_unlock(&m_colaBloqueados);
			dtbADesbloquear->status = READY;
			encolarDTB(colaReady, dtbADesbloquear, m_colaReady);
			sem_post(&cantProcesosEnReady);
		}	
		else{
			eliminarRecurso(rec);
		}
	}
	if(dtb != NULL){ 
		dtb->quantumFaltante = datosConfigSAFA->quantum - cantQuantoEjecutado;
		dtb->PC += cantQuantoEjecutado;
	}
	//ejecuta el mismo GDT
    char string_flagInicializacion[2];
    sprintf(string_flagInicializacion, "%i", dtb->flagInicializado); 
    char string_pc[2];
    sprintf(string_pc, "%i", dtb->PC);
    char string_quantumAEjecutar[2];
    sprintf(string_quantumAEjecutar, "%i", quantumDesignadoACPU - cantQuantoEjecutado);
	if(cpu != NULL)
		runFunction(cpu->socket,"ejecutarCPU",5, msg[1],
            									dtb->rutaScript,
												string_pc,
                                                string_flagInicializacion,
                                                string_quantumAEjecutar);
}

//es llamada por CPU y DAM cuando se conectan, para poder manejar el estado corrupto
void newConnection(socket_connection* socketInfo, char** msg){
	if(estadoCorrupto){
		if(strcmp(msg[0], "CPU") == 0 ){
			unCpuConectado = true;
		}
		if(strcmp(msg[0], "DAM") == 0 ){
			damConectado = true;
		}
		estadoCorrupto = !unCpuConectado || !damConectado;		
	}	
}

//args[0]: idCPU, args[1]: idGDT, args[2]: cantidad de quanto que ejecutó, args[3]:"finalizar","continuar" ó "bloquear"
//								  							   			   "finalizar" => finalizo GDT,		
//								  							   			   "bloquear"  => bloqueo GDT,
//								   							   			   "continuar" => paso a Ready GDT
void finalizacionProcesamientoCPU(socket_connection* socketInfo, char** msg){
	int idCPU = atoi( msg[0] );
	int idDTB = atoi ( msg[1] );
	int quantumEjecutado = atoi( msg[2] );
	log_info(logger, "La CPU %d ha finalizado de procesar sentencias del GDT de id %d", idCPU, idDTB);
	CPU* cpu = buscarCPU(idCPU);

	pthread_mutex_lock(&m_listaEjecutando);
	DTB* dtb = get_and_remove_DTB_by_ID(listaEjecutando, idDTB);
	pthread_mutex_unlock(&m_listaEjecutando);

	pthread_mutex_lock(&m_colaNew);
	cantSentEsperadasASumar = quantumEjecutado;
	list_iterate(colaNew->elements, (void*) &aumentarCantSentenciasEsperadasEnNew);
	pthread_mutex_unlock(&m_colaNew);

	if(dtb != NULL){ 
		if(dtb->status != FINISHED){ 
			if( strcmp( msg[3], "finalizar") == 0){

				finalizarDTB(dtb);
				log_info(logger,"El GDT de id %d ha sido finalizado",dtb->id);

			} else if(strcmp( msg[3], "continuar") == 0){ 

				dtb->status = READY;
				dtb->quantumFaltante = datosConfigSAFA->quantum - quantumEjecutado;
				dtb->PC += quantumEjecutado;
				encolarDTB(colaReady, dtb, m_colaReady);
				sem_post(&cantProcesosEnReady);

			} else{ // "bloquear"
				dtb->status = BLOCKED;
				dtb->quantumFaltante = datosConfigSAFA->quantum - quantumEjecutado;
				dtb->PC += quantumEjecutado;
				encolarDTB(colaBloqueados, dtb, m_colaBloqueados);
    			log_info(logger, "El DTB paso al Estado Blocked");

			}
		} else{
			finalizarDTB(dtb);
		}
	} 

	sem_post(&cpu->aviso);
}

void aumentarCantSentenciasEsperadasEnNew(DTB* dtb){
	dtb->cantSentEsperadasEnNew += cantSentEsperadasASumar;
}

//msg[0] = idDTB ,msg[1] = "ok" ó "error"
void avisoDeDamDeResultadoDTBDummy(socket_connection* socketInfo, char** msg){
	int idDTB = atoi(msg[0]);
	pthread_mutex_lock(&m_colaBloqueados);
	DTB* dtb = get_and_remove_DTB_by_ID(colaBloqueados->elements, idDTB);
	pthread_mutex_unlock(&m_colaBloqueados);

	if(dtb != NULL){ //si no ha sido finalizado...		
		if( strcmp(msg[1], "ok") == 0 ){
			log_info(logger,"Se finalizo OK el DTB-Dummy del GDT de id %d",dtb->id);
			dtb->status = READY;
			encolarDTB(colaReady, dtb, m_colaReady);
			sem_post(&cantProcesosEnReady);
		} else{ // "error"
			log_info(logger,"Se finalizo con ERROR el DTB-Dummy del GDT de id %d",dtb->id);
			finalizarDTB(dtb);
		}				
	}
}
//Se debe verificar si no es nulo, puesto que si es nulo significa que el dtb con dicho id ya fue finalizado 
// (por consola), y si no se verificara la nulidad tiraría segmentationFault.
// msgs[0]: idDTB
void desbloquearDTB(socket_connection* connection, char** msgs){
	int idDTB = atoi(msgs[0]);
	pthread_mutex_lock(&m_colaBloqueados);
	DTB* dtb = get_and_remove_DTB_by_ID(colaBloqueados->elements, idDTB);
	pthread_mutex_unlock(&m_colaBloqueados);
	if(dtb != NULL){ 
		log_info(logger,"Se va a desbloquear el ID del DTB: %d", dtb->id);
		dtb->status = READY;
		encolarDTB(colaReady, dtb, m_colaReady);
		sem_post(&cantProcesosEnReady);
	}
}

//Caso cuando ocurre un fallo y pasa a abortarse para la cola FINISHED
//msgs[0]: idDTB
void pasarDTBAExit(socket_connection* connection, char** msgs){
	int idDTB = atoi(msgs[0]);
	pthread_mutex_lock(&m_colaBloqueados);
	DTB* dtb = get_and_remove_DTB_by_ID(colaBloqueados->elements, idDTB);
	pthread_mutex_unlock(&m_colaBloqueados);
	if(dtb != NULL){ 
		log_trace(logger,"Se va a abortar al GDT de id: %d", idDTB);
		finalizarDTB(dtb);
	}
}

//FIN callable remote functions
void finalizarDTB(DTB* dtb){
	dtb->status = FINISHED;
	encolarDTB(colaFinalizados, dtb, m_colaFinalizados);
	sem_post(&puedeEntrarAlSistema);
}

static inline char *stringFromState(status_t status) { //Agarra un estado del enum y retorna el valor como String
    static const char *strings[] = { "NEW", "READY", "BLOCKED", "RUNNING", "FINISHED"};

    return strings[status];
}

void * statusDTB(int idGDT){ //Idea de Buscar Por Cada Cola Hasta Encontrar El Id Especifico
	DTB* dtb;
	dtb = buscarDTBEnElSistema(idGDT);
	
	if(dtb != NULL)
		mostrarInformacionDTB(dtb);
	else
		printf("No exite un GDT con id %d en el sistema\n", idGDT);

}

void * buscarDTBEnColasMostrandoInfo(int idDTB, t_queue* colaBusqueda) {

	int index = 0;
	DTB* elemento;


	if(queue_size(colaBusqueda) == 0) {printf("La Cola Esta Vacia\n");return 0;}

    elemento = list_get(colaBusqueda->elements, index);

    while(index < queue_size(colaBusqueda)){

    if(idDTB == 0) {mostrarInformacionDTB(elemento);
					index++;
				    elemento = list_get(colaBusqueda->elements, index);}

	else if(idDTB != elemento->id) {

		printf("No Se Encontro En La Cola\n");
		index++;
	    elemento = list_get(colaBusqueda->elements, index);
	}

	else {

	mostrarInformacionDTB(elemento);
	index++;
	elemento = list_get(colaBusqueda->elements, index);
	}
   }
    return 0;
}

void mostrarInformacionDTB(DTB* unDTB){
		char* estado;

    	printf("Los Datos Almacenados En El DTB Son:\n");
    	printf("Id DTB: %d\n", unDTB->id);
    	printf("Ruta del Escriptorio: %s\n", unDTB->rutaScript);
    	printf("Program Counter: %d\n", unDTB->PC);
    	printf("Flag Inicializado: %d\n", unDTB->flagInicializado);
    	estado = stringFromState(unDTB->status);
    	printf("Estado: %s\n", estado);
    	printf("----------------------\n");

    }
