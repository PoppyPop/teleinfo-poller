#include <iostream>
#include <list>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <cstdlib>
#include <mysql/mysql.h>
#include "global.h"

using namespace std;

#ifndef teleinfomessage_H
#define teleinfomessage_H

class teleinfomessage {
public:
	bool ajoutLigne(string libelle, string valeur, char cheksum);
	bool ajoutLigne(string ligne);

	bool Enregistrer();

	void LogRAW();

	void EnregistrerNbEssai(int nb_essai);

	bool EstEnregistrer();

	bool EstRenseigner();

	string ObtenirPAPP();

	teleinfomessage();

private:

	struct teleinfoligne {
		string Libelle;
		string Valeur;
		char Cheksum;

	};

	bool IsEnregistrer;

	time_t datetime;

	list<teleinfoligne> lignes;

	list<string> lignesRaw;

	int nbessai;

	bool LigneExiste(string libelle);

	bool controlerCheksum(string etiquette, string valeur, char checksum);

	string prepareSqlTxtParam(string valeur);
};

#endif
