#ifndef SRC_COMANDOSMEMORIA_H_
#define SRC_COMANDOSMEMORIA_H_

#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/list.h>

#include "socket.h"

typedef struct
{
	char * name;
	void(*fn)();
	int args;
	char * description;
} Command;


Command* 	cmd;
t_list* 	commands;

void loadCommands();

void executeCommand(char*);

#endif /* SRC_COMANDOSMEMORIA_H_ */