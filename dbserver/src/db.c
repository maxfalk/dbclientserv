
#include "sqlite3.h" 
#include <stdio.h>

sqlite3 *db = NULL;

int open_db(){

	int rc;
	int outcome = 0;

	if (db == NULL){
		rc = sqlite3_open("database/data.db", &db);

		if (rc){
			fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
			outcome = 1;

		}
		else{
			fprintf(stderr, "Opened database successfully\n");
		}

	}
	else{
		fprintf(stderr, "Database already open\n");
		outcome = 1;
	}


	return outcome;
}

void close_db(){
	sqlite3_close(db);
}


void query_db(char *query, int(*callback)(void *data, int argc, char **argv, char **col_name), void *data){

	int rc;
	char *error_msg = 0;


	rc = sqlite3_exec(db, query, callback, (void *)data, &error_msg);
	if (rc != SQLITE_OK){
		fprintf(stderr, "SQL error: %s\n", error_msg);
		sqlite3_free(error_msg);
	}
	else{
		fprintf(stdout, "Operation done successfully\n");
	}

}
