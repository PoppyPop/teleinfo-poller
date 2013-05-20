/*
 * teleinfo-message.cpp
 *
 *  Created on: 7 févr. 2013
 *      Author: poppy
 */

#include "teleinfo-message.h"

bool teleinfomessage::ajoutLigne(string libelle, string valeur, char cheksum) {
	if (!LigneExiste(libelle)) {
		struct teleinfoligne ligne;

		if (controlerCheksum(libelle, valeur, cheksum)) {
			ligne.Libelle = libelle;
			ligne.Valeur = valeur;
			ligne.Cheksum = cheksum;

			log(LOG_INFO, "Lecture de la ligne %s: %s / %c", libelle.c_str(),
					valeur.c_str(), cheksum);

			lignes.push_back(ligne);
			return true;
		}
	}

	return false;
}

bool teleinfomessage::ajoutLigne(string ligne) {
	string etiquette;
	string valeur;
	char checksum;

	lignesRaw.push_back(ligne);

	string word;
	stringstream stream(ligne);

	unsigned int pos = 0;

	while (getline(stream, word, ' ')) {
		switch (pos) {
		case 0:
			etiquette = word;
			break;
		case 1:
			valeur = word;
			break;
		case 2:
			checksum = word[0];
			break;
		default:
			break;
		}
		pos++;
	}

	return ajoutLigne(etiquette, valeur, checksum);
}

bool teleinfomessage::LigneExiste(string libelle) {
	list<teleinfoligne>::iterator i;

	for (i = lignes.begin(); i != lignes.end(); ++i) {
		if (i->Libelle == libelle) {
			return true;
		}
	}

	return false;
}

teleinfomessage::teleinfomessage() {
	// Constructeur

	// On garde l'heure de debut de reception
	time(&datetime);

	IsEnregistrer = false;
}

bool teleinfomessage::controlerCheksum(string etiquette, string valeur,
		char checksum) {
	unsigned char sum = 32; // Somme des codes ASCII du message + un espace
	unsigned int i;

	for (i = 0; i < etiquette.size(); i++)
		sum = sum + etiquette[i];
	for (i = 0; i < valeur.size(); i++)
		sum = sum + valeur[i];
	sum = (sum & 63) + 32;
	if (sum == checksum) {
		return true;
	} else {
		log(LOG_INFO, "Message %s/%s \t Checksum lu:%02x   calculé:%02x",
				etiquette.c_str(), valeur.c_str(), checksum, sum);
	}

	return false;
}

void teleinfomessage::LogRAW() {
	list<string>::iterator iter;

	string result;

	result.append("Raw teleinfo :\n\r");

	for (iter = lignesRaw.begin(); iter != lignesRaw.end(); ++iter) {
		result.append("\tLigne :");
		result.append(iter->c_str());
		result.append("\n\r");
	}

	log(LOG_ERR, result.c_str());
}

bool teleinfomessage::Enregistrer() {
	if (ModeVisu) {
		list<teleinfoligne>::iterator iter;

		string result;

		for (iter = lignes.begin(); iter != lignes.end(); ++iter) {
			result.append(iter->Libelle.c_str());
			result.append(" :");
			result.append(iter->Valeur.c_str());
			result.append(" - ");
		}

		log(LOG_ERR, result.c_str());

		return true;

	} else {

		if (!IsEnregistrer) {

			string columns;
			string values;

			// Ajout des informations statiques de l'objet
			columns.append("datecapture");
			char sdate[21];
			struct tm *dc;
			dc = localtime(&datetime);
			// strftime(sdate, sizeof sdate, "%s", dc);
			strftime(sdate, sizeof sdate, "%Y-%m-%d %H:%M:%S", dc);
			values.append(prepareSqlTxtParam(sdate));

			//
			columns.append(", ");
			values.append(", ");

			columns.append("NBESSAI");
			std::ostringstream oss;
			oss << nbessai;
			values.append(prepareSqlTxtParam(oss.str()));

			//
			columns.append(", ");
			values.append(", ");

			// Ajout des informations de teleinfo
			list<teleinfoligne>::iterator iter;

			list<teleinfoligne>::iterator finiter = lignes.end();
			--finiter;

			for (iter = lignes.begin(); iter != lignes.end(); ++iter) {

				columns.append(iter->Libelle.c_str());
				values.append(prepareSqlTxtParam(iter->Valeur.c_str()));

				if (iter != finiter) {
					columns.append(", ");
					values.append(", ");
				}
			}

			MYSQL mysql;
			char query[255];

			sprintf(query, "INSERT INTO %s (%s) VALUES (%s);",
					Mysql_TABLE.c_str(), columns.c_str(), values.c_str());

			log(LOG_INFO, query);

			/* INIT MYSQL AND CONNECT ----------------------------------------------------*/
			if (!mysql_init(&mysql)) {
				log(LOG_ERR, "Erreur: Initialisation MySQL impossible !");
				return 0;
			}
			if (!mysql_real_connect(&mysql, Mysql_HOST.c_str(),
					Mysql_LOGIN.c_str(), Mysql_PWD.c_str(), Mysql_DB.c_str(),
					0, NULL, 0)) {
				log(LOG_ERR, "Erreur connection %d: %s \n",
						mysql_errno(&mysql), mysql_error(&mysql));
				return 0;
			}

			if (mysql_query(&mysql, query)) {
				log(LOG_ERR, "Erreur INSERT %d: \%s \n", mysql_errno(&mysql),
						mysql_error(&mysql));
				mysql_close(&mysql);
				return 0;
			} else
				log(LOG_INFO, "Requete MySql ok.");

			mysql_close(&mysql);

			IsEnregistrer = true;
			return true;

		}
	}
	return false;
}

string teleinfomessage::prepareSqlTxtParam(string valeur) {

	std::ostringstream oss; // string as stream

	oss << "'" << valeur << "'"; //

	return oss.str();
}

string teleinfomessage::ObtenirPAPP() {
	list<teleinfoligne>::iterator i;

	for (i = lignes.begin(); i != lignes.end(); ++i) {
		if (i->Libelle == "PAPP") {
			return i->Valeur;
		}
	}

	return "";
}

bool teleinfomessage::EstEnregistrer() {
	return IsEnregistrer;
}

bool teleinfomessage::EstRenseigner() {
	return (lignes.size() > 0);
}

void teleinfomessage::EnregistrerNbEssai(int nb_essai) {
	nbessai = nb_essai;
}

