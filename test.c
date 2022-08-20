/**
 * File              : test.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 03.05.2022
 * Last Modified Date: 21.08.2022
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "klib/chworkdir.h"
#include "cashflow.h"
#include <stdio.h>

int cashflow_callback(void *user_data, cashflow_t *cashflow, char *error){
	if (error) {
		printf("%s\n", error);
		return 1;
	}
	
	cashflow_t *player = user_data;
	*player = *cashflow;
	
	return 0;
}

int main(int argc, char *argv[])
{
	printf("CASHFLOW TEST\n");

	//change directory to app
	k_lib_chWorkDir(argv);

	//create database
	const char *filepath = "db.sqlite";
	cashflow_database_init(filepath);

	//create new player
	cashflow_t player;
	cashflow_new(filepath, "doctor", 13200, 3600, 680, 2800, &player, cashflow_callback); 

	printf("PLAYER: uuid:%s, prof: %s, sal: %d\n", player.uuid, player.profession, player.salary);

	cashflow_for_each(filepath, NULL, NULL, NULL);

	getchar();
	return 0;
}
