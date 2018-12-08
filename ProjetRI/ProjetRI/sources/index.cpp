#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include "mysql.h"
#include "params.h"
#include "index.h"

extern ParamIndex params;

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

		// Ouverture des textes + parsage
		for (int i = 0; i < 39; i++) {
			std::ifstream myFile;	// Le fichier
			std::string path = params.BaseFiles.c_str();
			path.append("files/");
			path.append(std::to_string(i));
			path.append(".txt");	// Le path
			std::vector<std::string> words;	// Le vecteur de mots
			std::string word;	// Le mot courant
			myFile.open(path);	// Ouverture du fichier

			// Requete pour ajouter le fichier dans la table page
			if (myFile.is_open()) {
				std::cout << "File open" << std::endl;
				while (myFile >> word) {
					if (std::find(words.begin(), words.end(), word) != words.end()) {
						// TEST POUR SAVOIR SI MOT DEJA VU DANS FICHIER
						// Requete pour ajouter dans word_page
						// std::string query1 = "INSERT INTO word_page (id_word, id_page) VALUES (word, )";
						std::string query1 = "UPDATE word_page SET id_page = id_page + '" + std::to_string(i) + "' WHERE id_word = '" + word + "';";

					}
					else {
						// Requete pour ajouter dans word_page + word (car mot nouveau)
						std::string query1 = "INSERT INTO word (word) VALUES ('" + word + "');";
						std::string query2 = "INSERT INTO word_page (id_word, id_page) VALUES ('" + word + "','" + std::to_string(i) + "');";
					}
				}
			}
			else {
				std::cout << "Can't open the file : " << std::to_string(i) << std::endl;
			}
		}

		mysql_close(conn);
	}
	else std::cerr << mysql_error(conn) << std::endl;
}
