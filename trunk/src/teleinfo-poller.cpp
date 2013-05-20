//============================================================================
// Name        : teleinfo-poller.cpp
// Author      : Steve Holweg
// Version     :
// Copyright   : 
// Description : Teleinfo poller in C++
//============================================================================

#include <iostream>
#include <fstream>
#include <list>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <cstdlib>
#include <termios.h>
#include <sys/fcntl.h>
#include <getopt.h>
#include "teleinfo-message.h"
#include "global.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "configuration/configuration.h"

using namespace std;

// Gestion des erreurs
int nb_essai_max = 3;

string pidfile;

// Code Ascii
char STX = 0x02;
char ETX = 0x03;
char EOT = 0x04;

char LF = 0x0A;
char SP = 0x20;
char CR = 0x0D;

void daemonize(void) {
	pid_t pid;

	/* Clone ourselves to make a child */
	pid = fork();

	/* If the pid is less than zero,
	 something went wrong when forking */
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}

	/* If the pid we got back was greater
	 than zero, then the clone was
	 successful and we are the parent. */
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}

	/* If execution reaches this point we are the child */

	/* Set the umask to zero */
	umask(0);

	pid_t sid;

	/* Try to create our own process group */
	sid = setsid();
	if (sid < 0) {
		log(LOG_ERR, "Could not create process group\n");
		exit(EXIT_FAILURE);
	}

	/* Change the current working directory */
	if ((chdir("/")) < 0) {
		log(LOG_ERR, "Could not change working directory to /\n");
		exit(EXIT_FAILURE);
	}

	/* Write the PID */
	ofstream myfile;

	myfile.open(pidfile.c_str(), ios::trunc);

	if (myfile.good()) {

		if (myfile.is_open()) {
			myfile << sid << endl;
			myfile.close();
		} else {
			log(LOG_ERR, "Unable to write PID File to '%s'", pidfile.c_str());
			exit(EXIT_FAILURE);
		}
	}

	/* Here we go */
}

/*------------------------------------------------------------------------------*/
/* Init port rs232								*/
/*------------------------------------------------------------------------------*/
int initserie(string portSerie) {
	// Mode Non-Canonical Input Processing, Attend 1 caractère ou time-out(avec VMIN et VTIME).

	// Déclaration pour le port série.{
	int device;
	struct termios termiosteleinfo;

	// Ouverture de la liaison serie (Nouvelle version de config.)
	if ((device = open(portSerie.c_str(), O_RDWR | O_NOCTTY)) == -1) {
		log(LOG_ERR, "Erreur ouverture du port serie %s !", portSerie.c_str());
		exit(1);
	}

	tcgetattr(device, &termiosteleinfo); // Lecture des parametres courants.

	cfsetispeed(&termiosteleinfo, B1200);// Configure le débit en entrée/sortie.
	cfsetospeed(&termiosteleinfo, B1200);

	termiosteleinfo.c_cflag |= (CLOCAL | CREAD);// Active réception et mode local.

	// Format série "7E1"
	termiosteleinfo.c_cflag |= PARENB;// Active 7 bits de donnees avec parite pair.
	termiosteleinfo.c_cflag &= ~PARODD;
	termiosteleinfo.c_cflag &= ~CSTOPB;
	termiosteleinfo.c_cflag &= ~CSIZE;
	termiosteleinfo.c_cflag |= CS7;

	termiosteleinfo.c_iflag |= (INPCK | ISTRIP);// Mode de control de parité.

	termiosteleinfo.c_cflag &= ~CRTSCTS;// Désactive control de flux matériel.

	termiosteleinfo.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Mode non-canonique (mode raw) sans echo.

	termiosteleinfo.c_iflag &= ~(IXON | IXOFF | IXANY | ICRNL); // Désactive control de flux logiciel, conversion 0xOD en 0x0A.

	termiosteleinfo.c_oflag &= ~OPOST;// Pas de mode de sortie particulier (mode raw).

	termiosteleinfo.c_cc[VTIME] = 80; // time-out à ~8s.
	termiosteleinfo.c_cc[VMIN] = 0; // 1 car. attendu.

	tcflush(device, TCIFLUSH); // Efface les données reçues mais non lues.

	tcsetattr(device, TCSANOW, &termiosteleinfo);// Sauvegarde des nouveaux parametres

	return device;
}

char lireDonneeSerie(int *identifiantSerie, string portSerie) {
	char caractere[1];
	int res = read(*identifiantSerie, caractere, 1);
	if (!res) {
		log(LOG_ERR, "Erreur pas de réception début données Téléinfo !\n");

		if (ModeDeamon && CountSerialError < 10) {
			close(*identifiantSerie);

			CountSerialError++;

			int delaisreco = CountSerialError * 30;

			log(LOG_ERR, "Reconnection %i dans %i secondes.\n",
					CountSerialError, delaisreco);

			sleep(delaisreco);

			*identifiantSerie = initserie(portSerie);

			return lireDonneeSerie(identifiantSerie, portSerie);
		} else {
			exit(18);
		}
	} else {
		CountSerialError = 0;
	}

	return caractere[0];
}

void test() {
	teleinfomessage teleinfo;

	teleinfo.ajoutLigne("ADCO 700801422425 :");
	teleinfo.ajoutLigne("BDCO 700801422425 :");
	teleinfo.ajoutLigne("OPTARIF BASE 0");
	teleinfo.ajoutLigne("ISOUSC 30 9");
	teleinfo.ajoutLigne("BASE 008528238 /");
	teleinfo.ajoutLigne("PTEC TH.. $");
	teleinfo.ajoutLigne("IINST 002 Y");
	teleinfo.ajoutLigne("IMAX 090 H");
	teleinfo.ajoutLigne("PAPP 00360 *");

	teleinfo.EnregistrerNbEssai(1);
	teleinfo.Enregistrer();

	teleinfomessage teleinfo2;

	teleinfo2.ajoutLigne("ADCO 700801422425 :");
	teleinfo2.ajoutLigne("OPTARIF BASE 0");
	teleinfo2.ajoutLigne("ISOUSC 30 9");
	teleinfo2.ajoutLigne("BASE 008528239 0");
	teleinfo2.ajoutLigne("PTEC TH.. $");
	teleinfo2.ajoutLigne("IINST 002 Y");
	teleinfo2.ajoutLigne("IMAX 090 H");
	teleinfo2.ajoutLigne("PAPP 00370 +");

	teleinfo2.EnregistrerNbEssai(1);
	teleinfo2.Enregistrer();
}

int main(int argc, char* argv[]) {

	int c;
	int longIndex;
	unsigned int elapse = 30;
	unsigned int nbCycle = 10;
	unsigned int CycleCounter = 0;
	string portSerie = "/dev/ttyS0";
	bool optimisationBdd = true;

	ifstream inFile("/etc/conf.d/teleinfo-poller");

	if (inFile.good()) {
		Configuration config;
		config.Load(inFile);

		config.Get("serial", portSerie);
		config.Get("mysql-host", Mysql_HOST);
		config.Get("mysql-db", Mysql_DB);
		config.Get("mysql-table", Mysql_TABLE);
		config.Get("mysql-user", Mysql_LOGIN);
		config.Get("mysql-pwd", Mysql_PWD);
		config.Get("elapse", elapse);
		config.Get("pidfile", pidfile);
		config.Get("nombre-cycle", nbCycle);
	}

	static const struct option longOpts[] = { { "serial", required_argument,
			NULL, 's' }, { "help", no_argument, NULL, 'h' }, { NULL,
			no_argument, NULL, 0 }, { "mysql-host", required_argument, NULL,
			'H' }, { "mysql-db", required_argument, NULL, 'D' }, {
			"mysql-table", required_argument, NULL, 'T' }, { "mysql-user",
			required_argument, NULL, 'L' }, { "mysql-pwd", required_argument,
			NULL, 'P' }, { "elapse", required_argument, NULL, 'E' }, {
			"pidfile", required_argument, NULL, 'p' }, { "visualisation",
			no_argument, NULL, 'v' }, { "nombre-cycle",
					required_argument, NULL, 'n' }, { "no-bdd-optimisation",
							no_argument, NULL, 'o' } };

	// Traitement des paramètres
	while ((c = getopt_long(argc, argv, "dXs:hYH:D:T:L:P:E:p:vn:o", longOpts,
			&longIndex)) != -1)
		switch (c) {
		case 'X':
			ModeDebug = true;
			break;
		case 'd':
			ModeDeamon = true;
			break;
		case 's':
			portSerie = string(optarg);
			break;
		case 'H':
			Mysql_HOST = string(optarg);
			break;
		case 'D':
			Mysql_DB = string(optarg);
			break;
		case 'T':
			Mysql_TABLE = string(optarg);
			break;
		case 'L':
			Mysql_LOGIN = string(optarg);
			break;
		case 'P':
			Mysql_PWD = string(optarg);
			break;
		case 'Y':
			ModeTest = true;
			break;
		case 'E':
			elapse = atoi(optarg);
			break;
		case 'p':
			pidfile = optarg;
			break;
		case 'v':
			ModeVisu = true;
			break;
		case 'n':
			nbCycle = atoi(optarg);
			break;
		case 'o':
			optimisationBdd = false;
			break;
		case 'h':

			printf("--help \n");
			printf("-h \t\t: Affiche l'aide \n");
			printf("-d \t\t: Mode daemon \n");
			printf("-X \t\t: Mode Debug\n");
			printf("--serial {PortSerie}\n");
			printf("-s {PortSerie} \t: Port série\n");

			printf("--pidfile {PIDFile}\n");
			printf("-p {PIDFile} \t: Fichier contenant le PID\n");

			printf("--mysql-host {Mysql_HOST}\n");
			printf("-H {Mysql_HOST} \t: Hote Mysql\n");

			printf("--mysql-db {Mysql_DB}\n");
			printf("-D {Mysql_DB} \t: Base de données Mysql\n");

			printf("--mysql-table {Mysql_TABLE}\n");
			printf("-T {Mysql_TABLE} \t: Table Mysql\n");

			printf("--mysql-user {Mysql_LOGIN}\n");
			printf("-L {Mysql_LOGIN} \t: Login Mysql\n");

			printf("--mysql-pwd {Mysql_PWD}\n");
			printf("-P {Mysql_PWD} \t: Mot de passe Mysql\n");

			printf("--elapse {Delai en secondes}\n");
			printf(
					"-E {Delai en secondes} \t: Modifie le delai en secondes entre chaque lecture en mode daemon. (Defaut: 10)\n");

			printf("--visualisation\n");
			printf("-v \t: Mode visualisation (Test de la ligne teleinfo)\n");

			printf("--nombre-cycle {int}\n");
			printf("-n {int} \t: Nombre de cycle maximum sans modification en mode daemon. (Defaut: 10)\n");

			printf("--no-bdd-optimisation\n");
			printf("-o \t: Desactive l'optimisation des trames en bdd (Ne pas enregistrer 2 trames identiques consécutives)\n");

			exit(0);
		case '?':

			switch (optopt) {
			case 's':
			case 'H':
			case 'D':
			case 'T':
			case 'L':
			case 'P':
			case 'n':
				fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				return 1;
			default:
				if (isprint(optopt)) {
					fprintf(stderr, "Unknown option `-%c'.\n", optopt);
				} else {
					fprintf(stderr, "Unknown option character `\\x%x'.\n",
							optopt);
				}
				return 1;
			}

		default:
			abort();
		}

	log(LOG_INFO, "ModeDebug : %s", (ModeDebug) ? "Oui" : "Non");
	log(LOG_INFO, "ModeDeamon : %s", (ModeDeamon) ? "Oui" : "Non");
	log(LOG_INFO, "portSerie : %s", portSerie.c_str());
	log(LOG_INFO, "elapse : %i", elapse);
	log(LOG_INFO, "pidfile : %s", pidfile.c_str());

	if (ModeTest) {
		test();
		return 0;
	}

	// Mode fork
	if (ModeDeamon) {
		daemonize();
	}

	bool erreur_checksum;
	int identifiantSerie = initserie(portSerie);

	teleinfomessage ancienneconso;

	do {
		int nb_essai = 1;

		// Coeur metier
		do {
			erreur_checksum = true;

			// Variable
			char caractere_courant;
			string ligne;
			teleinfomessage teleinfo;

			// On efface les anciennes trame
			tcflush(identifiantSerie, TCIFLUSH);

			// Attend le debut d'une nouvelle trame
			do {
				caractere_courant = lireDonneeSerie(&identifiantSerie,
						portSerie);
			} while (!(caractere_courant == STX || caractere_courant == EOT)); // On attend le début d'une trame ou le caractere d'interruption

			// Mode interruption donc trame courte ou telereport
			if (caractere_courant == EOT) {
				caractere_courant = lireDonneeSerie(&identifiantSerie,
						portSerie);
				if (caractere_courant != STX) {
					// On a une interruption mais pas de STX : on coupe la boucle
					nb_essai++;
					break;
				}
			}

			// La on est dans de bonne conditions pour lire les données
			do {
				caractere_courant = lireDonneeSerie(&identifiantSerie,
						portSerie);
				if (caractere_courant != LF) {
					if (caractere_courant != CR) {
						ligne += caractere_courant;
					} else {

						if (teleinfo.ajoutLigne(ligne)) {
							erreur_checksum = false;
						} else {
							break;
						}

						ligne.clear();
					}
				}
			} while (caractere_courant != ETX); // Attend code fin trame téléinfo.

			if (!erreur_checksum && teleinfo.ObtenirPAPP() == "") {
				teleinfo.LogRAW();
				erreur_checksum = true;
			} else if (!erreur_checksum) {

				teleinfo.EnregistrerNbEssai(nb_essai);

				if (!optimisationBdd
						|| (ancienneconso.ObtenirPAPP() != teleinfo.ObtenirPAPP())
						|| (CycleCounter>=nbCycle)
						) {
					if (!ancienneconso.EstEnregistrer()
							&& ancienneconso.EstRenseigner()) {
						ancienneconso.Enregistrer();
					}

					teleinfo.Enregistrer();
					CycleCounter = 0;
				} else {
					CycleCounter++;
					log(LOG_INFO, "Conso %s identique à %s, on fait rien.",
							teleinfo.ObtenirPAPP().c_str(),
							ancienneconso.ObtenirPAPP().c_str());
				}

				ancienneconso = teleinfo;
			} else {
				nb_essai++;
			}

		} while ((erreur_checksum) && (nb_essai <= nb_essai_max));

		if (ModeDeamon) {
			sleep(elapse);
		}

	} while (ModeDeamon);

	close(identifiantSerie);

	return 0;
}
