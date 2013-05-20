/*
 * global.cpp
 *
 *  Created on: 18 févr. 2013
 *      Author: poppy
 */
#include "global.h"

// Gestion des params
bool ModeDebug = false;
bool ModeDeamon = false;
bool ModeTest = false;
bool ModeVisu = false;
int CountSerialError = 0;
bool DryRun = false;

// Define mysql
string Mysql_HOST = "localhost";
string Mysql_DB = "teleinfo";
string Mysql_TABLE = "teleinfo";
string Mysql_LOGIN = "teleinfo";
string Mysql_PWD = "teleinfo";

void log(int priorite, string message, ...) {
	va_list args;
	va_start(args, message);

	char buffer[1024 * 16];
	vsnprintf(buffer, (1024 * 16) - 1, message.c_str(), args);

	if (ModeDebug) {
		printf("%s\n", buffer);
	}

	if (priorite != LOG_INFO && ModeVisu) {
		printf("%s\n", buffer);
	}

	// Par defaut pas de log des INFOS
	if (priorite != LOG_INFO || ModeDebug) {

		syslog(priorite, buffer);
	}
}
