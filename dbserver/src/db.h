#ifndef _DB_H_
#define _DB_H_


void open_db();
void close_db();
static int query_callback(void *data, int argc, char **argv, char **col_name);
void query_db(char *query, int(*callback)(void *data, int argc, char **argv, char **col_name), void * data);



#endif