#include <stdio.h>
#include <stdlib.h>
#include "gestionProcesos.h"


char* identificarProceso(socket_connection * connection ,char** args)
{
     proc * pro = malloc(sizeof(proc));
     pro->proceso = args[0];
     printf("Se conecto %s en el socket NRO %d  con IP %s  PUERTO %d", pro->proceso,connection->socket,connection->ip,connection-> port);
     free(pro);
     return args[0];
} 
