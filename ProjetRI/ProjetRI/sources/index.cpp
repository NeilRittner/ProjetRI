#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <cmath>
#include "mysql.h"
#include "params.h"
#include "index.h"

extern ParamIndex params;

/*
* Cette fonction :
*	- Initialise la base de données
*	- Récupère les différentes données à insérer en base
*	- Insère les données en base
*/
void IndexData()
{
	MYSQL *conn = nullptr;
	conn = mysql_init(conn);

	if (mysql_real_connect(conn, params.ServerName.c_str(), params.Login.c_str(), params.Password.c_str(), params.SchemeName.c_str(), 0, NULL, 0)) {
		// Init DB
		std::vector<std::string> sqlQueries = {
			"DROP TABLE IF EXISTS `page`"
			,"CREATE TABLE `page` (`id_page` bigint(20) NOT NULL auto_increment,`url` text NOT NULL,`pr` float NOT NULL default '0',`resume` text NOT NULL,PRIMARY KEY(`id_page`))"
			,"DROP TABLE IF EXISTS `word`"
			,"CREATE TABLE `word` (`id_word` bigint(20) NOT NULL auto_increment,`word` varchar(30) NOT NULL default '',PRIMARY KEY(`id_word`))"
			,"DROP TABLE IF EXISTS `word_page`"
			,"CREATE TABLE `word_page` (`id_word` bigint(20) NOT NULL default '0',`id_page` bigint(20) NOT NULL default '0',PRIMARY KEY(`id_page`,`id_word`))"
		};

		for (auto &sql : sqlQueries) {
			if (mysql_query(conn, sql.c_str())) std::cerr << mysql_error(conn) << std::endl;
		}

		// Get data
		std::vector<double> pageRanks = getPageRanks();
		std::vector<std::string> urls = getUrls();
		std::vector<std::string> resumes = getResumes();
		std::map<std::string, std::vector<int>> wordsInFiles = getInvertedIndex();

		// Insert data
		std::map<std::string, int> word_Idword;
		std::vector<int> idPage;
		std::string query;

		for (int i = 0; i < NBFILES; i++) {
			query = "INSERT INTO page (url, pr, resume) VALUES ('" + urls[i] + "', '" + std::to_string(pageRanks[i]) + "', '" + resumes[i] + "');";
			mysql_query(conn, query.c_str());
			idPage.push_back(mysql_insert_id(conn));
		}

		for (std::map<std::string, std::vector<int>>::iterator it = wordsInFiles.begin(); it != wordsInFiles.end(); it++) {
			query = "INSERT INTO word (word) VALUES ('" + (*it).first + "');";
			mysql_query(conn, query.c_str());
			word_Idword.emplace(std::make_pair(std::string((*it).first), mysql_insert_id(conn)));
		}

		for (std::map<std::string, std::vector<int>>::iterator it = wordsInFiles.begin(); it != wordsInFiles.end(); it++) {
			for (std::vector<int>::iterator iter = (*it).second.begin(); iter != (*it).second.end(); iter++) {
				query = "INSERT INTO word_page (id_word, id_page) VALUES ('" + std::to_string(word_Idword.at((*it).first)) + "', '" + std::to_string(idPage[*iter]) +"');";
				mysql_query(conn, query.c_str());
			}
		}

		mysql_close(conn);
	}
	else std::cerr << mysql_error(conn) << std::endl;
}

/*
* Cette fonction détermine les urls des fichiers (ici, représente ici le nom de chacun)
* Retour : le vecteur d'url
*/
std::vector<std::string> getUrls() {
	std::vector<std::string> urls;
	std::string url;

	for (int i = 0; i < NBFILES; i++) {
		url = "";
		url = std::to_string(i) + ".txt";
		urls.push_back(url);
	}

	return urls;
}

/*
* Cette fonction sert à déterminer les résumés
* Paramètre : le chemin donné dans config.txt
* Retour : le vecteur de résumés
*/
std::vector<std::string> getResumes() {
	std::ifstream myFile;
	std::string path;
	std::vector<std::string> resumes;
	std::string resume;
	char c;

	for (int i = 0; i < NBFILES; i++) {
		std::string path = params.BaseFiles.c_str();
		path.append("files/");
		path.append(std::to_string(i));
		path.append(".txt");
		myFile.open(path);

		if (myFile.is_open()) {
			for (int cpt = 0; cpt < 200; cpt++) {
				myFile.get(c);
				// Interdiction de l'apostrophe car cause des problèmes lors de l'insertion avec MySQL
				if (c != '\'') {
					resume.insert(resume.end(), c);
				}
			}
			resumes.push_back(resume);
			resume = "";
			myFile.close();
		}
		else {
			std::cout << "Can't open the file : " << std::to_string(i) << std::endl;
		}
	}
	return resumes;
}

/*
* Cette fonction calcul l'index inversé
* Retour : l'index inversé
*/
std::map<std::string, std::vector<int>> getInvertedIndex() {
	std::ifstream myFile;
	std::string path;
	std::regex pattern(R"#(\w[\w\-]*)#");
	std::string line;
	std::string word;
	std::map<std::string, std::vector<int>> wordsInFiles;
	std::vector<int> vect;

	for (int i = 0; i < NBFILES; i++) {
		path = params.BaseFiles.c_str();
		path.append("files/");
		path.append(std::to_string(i));
		path.append(".txt");
		myFile.open(path);

		if (myFile.is_open()) {
			while (getline(myFile, line)) {
				for (std::sregex_iterator j = std::sregex_iterator(line.begin(), line.end(), pattern); j != std::sregex_iterator(); j++) {
					word = (*j).str();
					try {
						vect = wordsInFiles.at(word);
						if (std::find(vect.begin(), vect.end(), i) == vect.end()) {
							vect.push_back(i);
						}
					}
					catch (const std::exception&) {
						wordsInFiles.emplace(std::make_pair(std::string(word), std::vector<int>()));
						wordsInFiles.at(word).push_back(i);
					}
				}
			}
			myFile.close();
		}
		else {
			std::cout << "Can't open the file : " << std::to_string(i) << std::endl;
		}
	}

	return wordsInFiles;
}

/*
* Cette fonction :
*	- lit le fichier links.txt
*	- détermine le page rank des fichiers
* Retour : le vecteur des page ranks
*/
std::vector<double> getPageRanks() {
	std::vector<double> exPageRanks (NBFILES, 0); // Le vecteur de page ranks à T-1
	std::vector<double> newPageRanks (NBFILES, 0);	// Le vecteur de page ranks à T
	std::vector<int> nombreEnfants; // Nombre de fichiers pointés par fichier
	std::map<int, std::vector<int>> fichiersParents; // Les fichiers parents (pointant) par fichier
	int val = 0;
	double sommeNorm = 0;
	double somme = 0.0; // Contiendra la somme de la formule du page rank
	double distance = 0.0; // La distance entre deux vecteurs de page ranks
	double distanceTmp = 0.0;

	std::ifstream linksFile; // Le fichier contenant les liens
	std::string path = params.BaseFiles.c_str();
	path.append("links.txt");
	std::regex pattern(R"#(\w[\w\-]*)#");
	std::string line;
	int lineNumber = -1;
	int colNumber = 0;
	int nbNoeuds = 0; // Nombre de noeuds sortants d'un document
	
	// On lit la matrice des noeuds sortants
	linksFile.open(path);
	if (linksFile.is_open()) {
		while (getline(linksFile, line)) {
			if (lineNumber >= 0) {
				for (std::sregex_iterator j = std::sregex_iterator(line.begin(), line.end(), pattern); j != std::sregex_iterator(); j++) {
					val = std::stoi((*j).str());
					if (val == 1) {
						nbNoeuds += val;
						try {
							fichiersParents.at(colNumber).push_back(lineNumber);
						}
						catch (const std::exception&) {
							fichiersParents.emplace(std::make_pair(colNumber, std::vector<int>()));
							fichiersParents.at(colNumber).push_back(lineNumber);
						}
					}
					colNumber += 1;
				}
				nombreEnfants.push_back(nbNoeuds);
				nbNoeuds = 0;
				colNumber = 0;
			}
			lineNumber += 1;
		}
	}
	else {
		std::cout << "Can't open the file links.txt" << std::endl;
	}

	do {
		distance = 0;
		sommeNorm = 0.0;
		for (int i = 0; i < NBFILES; i++) {
			try {
				std::vector<int>& vectParents = fichiersParents.at(i);
				for (std::vector<int>::iterator ite = vectParents.begin(); ite != vectParents.end(); ite++) {
					somme = somme + (exPageRanks.at(*ite) / nombreEnfants.at(*ite));
				}
			}
			catch (const std::exception&) {
				somme = 0;
			}
			newPageRanks.at(i) = (1 - D) + D * somme;
			somme = 0;
		}

		for (std::vector<double>::iterator it = newPageRanks.begin(); it != newPageRanks.end(); it++) {
			sommeNorm = sommeNorm + (*it);
		}
		for (std::vector<double>::iterator it = newPageRanks.begin(); it != newPageRanks.end(); it++) {
			(*it) = (*it) / sommeNorm;
		}

		for (int j = 0; j < newPageRanks.size(); j++) {
			distanceTmp = newPageRanks.at(j) - exPageRanks.at(j);
			if (abs(distanceTmp) > distance) {
				distance = abs(distanceTmp);
			}
		}
		exPageRanks = newPageRanks;
	} while (distance > CONVERGENCE);

	return newPageRanks;
}
