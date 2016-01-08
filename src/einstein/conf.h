/*
 *  Configuration file parser
 *  Copyright (c) 2002 Alexander Babichev
 */

#ifndef __CONF_H__
#define __CONF_H__

struct _STable;
typedef struct _STable* HTable;
struct _STableIterator;
typedef struct _STableIterator* HTableIterator;

#define TYPE_INT    1
#define TYPE_STRING 2
#define TYPE_FLOAT  3
#define TYPE_TABLE  4

/* create empty table */
HTable table_create();

/* read table from file */
HTable table_read(const char *filename);

/* free table */
void table_free(HTable table);

/* get table iterator positioned before table start */
HTableIterator table_get_iter(HTable table);

/* free table iterator */
void table_free_iter(HTableIterator iterator);

/* move iterator.  return 1 on success */
int table_iter_next(HTableIterator iterator);

/* get name of table field */
char* table_iter_get_name(HTableIterator iterator);

/* get type of table field */
int table_iter_get_type(HTableIterator iterator);

/* get current table field as newly allocated string */
char* table_iter_get_str(HTableIterator iterator, int *err);

/* get current table field as static string valid till next iterator operation */
char* table_iter_get_strs(HTableIterator iterator, int *err);

/* get current table field as int */
int table_iter_get_int(HTableIterator iterator, int *err);

/* get current table field as double */
double table_iter_get_double(HTableIterator iterator, int *err);

/* get current table field as table */
HTable table_iter_get_table(HTableIterator iterator, int *err);

/* return 0 if field not exists in table */
int table_is_field_exists(HTable table, char *name);

/* get type of table field or -1 if field not exists */
int table_get_field_type(HTable table, char *field);

/* get table field as newly allocated string */
char* table_get_str(HTable table, char *field, char *dflt, int *err);

/* get table field as static string */
char* table_get_strs(HTable table, char *field, char *dflt, int *err);

/* get table field as int */
int table_get_int(HTable table, char *field, int dflt, int *err);

/* get table field as double */
double table_get_double(HTable table, char *field, double dflt, int *err);

/* get table field as table */
HTable table_get_table(HTable table, char *field, HTable dflt, int *err);

/* print table to new allocated string */
char* table_to_str(HTable table, int print_braces, int butify, int spaces);

/* set table field as string */
int table_set_str(HTable table, const char *field, const char *val);

/* set table field as int */
int table_set_int(HTable table, const char *field, int val);

/* set table field as double */
int table_set_double(HTable table, const char *field, double val);

/* set table field as table */
int table_set_table(HTable table, const char *field, HTable val);

/* append string field to table */
int table_append_str(HTable table, const char *val);

/* append int field to table */
int table_append_int(HTable table, int val);

/* append double field to table */
int table_append_double(HTable table, double val);

/* append tablee field to table */
int table_append_table(HTable table, HTable val);

#endif

