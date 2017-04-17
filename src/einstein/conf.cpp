#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "conf.h"


typedef struct _SField
{
    char *name;
    int type;
    union {
        char *str_value;
        int int_value;
        double double_value;
        HTable table_value;
    } value;
    struct _SField *next;
} SField;


typedef struct _STable
{
    SField *fields;
    char *stat_buf;
} STable;


typedef struct _STableIterator
{
    HTable table;
    SField *cur_field;
    int before_start;
    char *stat_buf;
} STableIterator;



HTable table_create()
{
    HTable table;

    table = (STable*)calloc(1, sizeof(STable));
    return table;
}


static void free_field(SField *field)
{
    if (! field)
        return;
    if (field->name)
        free(field->name);
    switch (field->type)
    {
        case TYPE_STRING: free(field->value.str_value); break;
        case TYPE_TABLE: table_free(field->value.table_value); break;
    }
    free(field);
}


static SField* find_field(HTable table, const char *name)
{
    SField *f;

    if ((! table) || (! name))
        return NULL;

    f = table->fields;
    while (f)
    {
        if (f->name && (! strcmp(f->name, name)))
            return f;
        f = f->next;
    }

    return NULL;
}


int table_remove_field(HTable table, char *field)
{
    SField *f, *pf;

    if ((! table) || (! field))
        return -1;

    f = find_field(table, field);
    if (f)
    {
        if (table->fields == f)
            table->fields = f->next;
        else
        {
            pf = table->fields;
            while (pf && (pf->next != f))
                pf = pf->next;
            if (pf)
                pf->next = f->next;
        }
        free_field(f);
    }

    return 0;
}


static SField* add_empty_field(HTable table, const char *field)
{
    SField *f, *nf;

    if ((! table) || (! field))
        return NULL;

    f = find_field(table, field);
    if (f)
        table_remove_field(table, f->name);

    nf = (SField*)calloc(1, sizeof(SField));
    if (! nf) return NULL;
    nf->name = strdup(field);

    f = table->fields;
    if (f)
    {
        while (f->next)
            f = f->next;
        f->next = nf;
    }
    else
        table->fields = nf;

    return nf;
}


int table_set_str(HTable table, const char *field, const char *value)
{
    SField *f;

    if ((! table) || (! field) || (! value))
        return -1;
    
    f = add_empty_field(table, field);
    if (! f) return -1;

    f->type = TYPE_STRING;
    f->value.str_value = strdup(value);
    return 0;
}


int table_set_int(HTable table, const char *field, int value)
{
    SField *f;

    if ((! table) || (! field))
        return -1;
    
    f = add_empty_field(table, field);
    if (! f) return -1;

    f->type = TYPE_INT;
    f->value.int_value = value;
    return 0;
}


int table_set_double(HTable table, const char *field, double value)
{
    SField *f;

    if ((! table) || (! field))
        return -1;
    
    f = add_empty_field(table, field);
    if (! f) return -1;

    f->type = TYPE_FLOAT;
    f->value.double_value = value;
    return 0;
}


int table_set_table(HTable table, const char *field, HTable value)
{
    SField *f;

    if ((! table) || (! field) || (! value))
        return -1;
    
    f = add_empty_field(table, field);
    if (! f) return -1;

    f->type = TYPE_TABLE;
    f->value.table_value = value;
    return 0;
}


static int skip_spaces(char *buf, long int *pos);


static int skip_multy_comment(char *buf, long int *pos)
{
    while (buf[*pos])
    {
        if (buf[*pos] == '*')
        {
            (*pos)++;
            if (buf[*pos] == '/')
            {
                (*pos)++;
                return skip_spaces(buf, pos);
            }
        }
        (*pos)++;
    }

    return -1;
}


static int skip_line_comment(char *buf, long int *pos)
{
    while (buf[*pos] && ((buf[*pos]) != '\n'))
        (*pos)++;
    return skip_spaces(buf, pos);
}


static int is_cspace(char *buf, long int *pos)
{
    if (isspace(buf[*pos]))
        return 1;
    if (buf[*pos] == '/')
    {
        if ((buf[(*pos) + 1] == '*') || (buf[(*pos) + 1] == '/'))
            return 1;
    }
    return 0;
}


static int skip_spaces(char *buf, long int *pos)
{
    while (buf[*pos] && isspace(buf[*pos]))
        (*pos)++;
    
    if (buf[*pos] == '/')
    {
        (*pos)++;
        if (buf[*pos] == '*')
        {
            (*pos)++;
            return skip_multy_comment(buf, pos);
        }
        else if (buf[*pos] == '/')
        {
            (*pos)++;
            return skip_line_comment(buf, pos);
        }
        else
            return -1;
    }
    return 0;
}


static int is_symbol(char sym)
{
    static char syms[] = { '=', '{', '}', ';', '\'', '"', ',', '/', '\\' };
    unsigned int i;

    for (i = 0; i < sizeof(syms) / sizeof(char); i++)
    {
        if (sym == syms[i])
            return 1;
    }

    return 0;
}


static char* read_string(char* buf, long int *pos)
{
    int allocated=50, len=1;
    char *b;

    b = (char*)malloc(allocated);
    if (! b) return NULL;
    b[0] = buf[*pos];
    (*pos)++;
    while (buf[*pos])
    {
        if (len + 3 > allocated)
        {
            char *s = (char*)realloc(b, allocated+=20);
            if (! s)
            {
                free(b);
                return NULL;
            }
            b = s;
        }
        if ((buf[*pos] == '\\') && (buf[*pos + 1] == b[0]))
        {
            b[len++] = '\\';
            b[len] = b[0];
        }
        else
        {
            b[len++] = buf[*pos];
            if (buf[*pos] == b[0])
            {
                b[len] = 0;
                (*pos)++;
                return b;
            }
        }
        (*pos)++;
    }
    free(b);
    return NULL;
}


static char* read_word(char* buf, long int *pos)
{
    int allocated=50, len=0;
    char *b;

    b = (char*)malloc(allocated);
    if (! b) return NULL;
    while (buf[*pos] && (! is_cspace(buf, pos)) && (! is_symbol(buf[*pos])))
    {
        if (len + 3 > allocated)
        {
            char *s = (char*)realloc(b, allocated+=20);
            if (! s)
            {
                free(b);
                return NULL;
            }
            b = s;
        }
        b[len++] = buf[*pos];
        (*pos)++;
    }
    if (! len)
    {
        free(b);
        return NULL;
    }
    b[len] = 0;
    return b;
}


static char* read_token(char *buf, long int *pos)
{
    if (skip_spaces(buf, pos) || (! buf[*pos]))
        return NULL;

    if ((buf[*pos] == '\'') || (buf[*pos] == '"'))
        return read_string(buf, pos);

    if (is_symbol(buf[*pos]))
    {
        char *b = (char*)malloc(2);
        if (! b) return NULL;
        b[0] = buf[*pos];
        b[1] = 0;
        (*pos)++;
        return b;
    }

    return read_word(buf, pos);
}


static long int get_file_size(FILE *f)
{
    long int len;
    
    if (fseek(f, 0L, SEEK_END))
        return -1;

    len = ftell(f);
    
    if (fseek(f, 0L, SEEK_SET))
        return -1;

    return len;
}


static char* read_file(const char *filename, long int *length)
{
    FILE *f;
    char *buf;
    long int len;

    f = fopen(filename, "rb");
    if (! f) return NULL;

    len = get_file_size(f);
    if (len <= 0)
    {
        fclose(f);
        return NULL;
    }

    buf = (char*)malloc(len + 1);
    if (! buf)
    {
        fclose(f);
        return NULL;
    }

    if (fread(buf, 1, len, f) != (unsigned)len)
    {
        free(buf);
        fclose(f);
        return NULL;
    }

    buf[len] = 0;

    fclose(f);
    *length = len + 1;
    return buf;
}


static int get_next_index(HTable table)
{
    SField *f;
    int v, max_v=-1;
    char *endptr;
    
    for (f = table->fields; f; f = f->next)
    {
        v = strtol(f->name, &endptr, 10);
        if ((! f->name[0]) || (endptr[0])) 
            continue;
        if (v > max_v)
            max_v = v;
    }

    return max_v + 1;
}


static int is_ident(char *str)
{
    char *p;
    
    if (! str)
        return 0;
    if (strlen(str) < 1)
        return 0;
    if (! isalpha(str[0]))
        return 0;
    
    for (p = str; *p; p++)
        if (! (isalnum(*p) || (*p == '_') || (*p == '.')))
            return 0;
    
    return 1;
}


static int is_string_value(char *value)
{
    int len;

    if ((value[0] == '\'') || (value[0] == '"'))
    {
        if ((len = strlen(value)) >= 2)
        {
            if (value[len - 1] == value[0])
                return 1;
        }
    }
    
    return 0;
}


static int is_int_value(char *value)
{
    char *p;

    for (p = value; *p; p++)
    {
        if (! isdigit(*p))
        {
            if (p != value)
                return 0;
            else
                if (! ((*p == '+') || (*p == '-')))
                    return 0;
        }
    }
    return 1;
}


static int is_float_value(char *value)
{
    char *p;
    int pcount = 0;

    for (p = value; *p; p++)
    {
        if (! isdigit(*p))
        {
            if (p != value)
            {
                if (*p != '.')
                    return 0;
                else
                {
                    pcount++;
                    if (pcount > 1)
                        return 0;
                }
            }
            else
            {
                if (! ((*p == '+') || (*p == '-')))
                    return 0;
            }
        }
    }
    return 1;
}


static int get_value_type(char *value)
{
    if ((! value) || (! value[0]))
        return -1;

    if (is_string_value(value))
        return TYPE_STRING;
    if (! strcmp(value, "{"))
        return TYPE_TABLE;
    if (is_int_value(value))
        return TYPE_INT;
    if (is_float_value(value))
        return TYPE_FLOAT;

    return -1;
}


static char* unescape_string(char *str)
{
    char *buf, quote, *p;
    int allocated=50, len=0;

    buf = (char*)malloc(allocated);

    quote = str[0];
    p = str + 1;
    while ((*p) && (*p != quote))
    {
        if (len + 10 > allocated)
        {
            char *s = (char*)realloc(buf, allocated+=20);
            if (! s)
            {
                free(buf);
                return NULL;
            }
            buf = s;
        }
        if (*p == '\\')
        {
            p++;
            switch (*p)
            {
                case 'n': buf[len++] = '\n'; break;
                case 't': buf[len++] = '\t'; break;
                case 'r': buf[len++] = '\r'; break;
                default: buf[len++] = *p;
            }
        }
        else
            buf[len++] = *p;
        p++;
    }
    buf[len] = 0;

    if ((*p == quote) && (*(p + 1)))
    {
        free(buf);
        return NULL;
    }
    return buf;
}


static int parse_table(HTable table, char *buf, long int *pos, int top_level);


static int add_field(HTable table, char *name, char *value, char *buf, long int *pos)
{
    int value_type;
    char *s;
    HTable tbl;

    value_type = get_value_type(value);
    switch (value_type)
    {
        case TYPE_INT:
            table_set_int(table, name, atoi(value));
            break;
        case TYPE_STRING:
            s = unescape_string(value);
            if (! s)
                return -1;
            table_set_str(table, name, s);
            free(s);
            break;
        case TYPE_FLOAT:
            table_set_double(table, name, atof(value));
            break;
        case TYPE_TABLE:
            tbl = table_create();
            if (table_set_table(table, name, tbl))
            {
                table_free(tbl);
                return -1;
            }
            if (parse_table(tbl, buf, pos, 0))
                return -1;
            break;
        default:
            return -1;
    }
    return 0;
}


static int add_field_to_array(HTable table, char *token, char *buf, long int *pos)
{
    int idx = get_next_index(table);
    char b[100];
    sprintf(b, "%i", idx);
    return add_field(table, b, token, buf, pos);
}



/*
 * TODO: add field as in array highly unoptimized.  it scans for max index
 *       number on every element addition, converting idexes from strings
 *       to numbers where it possible.  optimizations method: process
 *       indexes in batch
 */

static int parse_table(HTable table, char *buf, long int *pos, int top_level)
{
    char *token=NULL;
    char *name;
    int res;

    while (1)
    {
        if (! token)
            token = read_token(buf, pos);
        if (! token)
        {
            if (top_level)
                return 0;
            else
                return -1;
        }

        if (! strcmp(token, "{"))
        {
            if (add_field_to_array(table, token, buf, pos))
            {
                free(token);
                return -1;
            }
            free(token);
            token = read_token(buf, pos);
            if (! token)
            {
                if (top_level)
                    return 0;
                else
                    return -1;
            }
            if ((! strcmp(token, ";")) || (! strcmp(token, ",")))
            {
                free(token);
                token = NULL;
            }
            continue;
        } 
        else if (! strcmp(token, "}"))
        {
            free(token);
            if (top_level)
                return -1;
            else
                return 0;
        }

        name = token;
        
        token = read_token(buf, pos);
        
        if (token && (! strcmp(token, "=")))
        {
            free(token);

            if (! is_ident(name))
            {
                free(name);
                return -1;
            }

            token = read_token(buf, pos);
            if (! token)
            {
                free(name);
                return -1;
            }

            res = add_field(table, name, token, buf, pos);
            free(name);
            free(token);
            if (res)
            {
                return -1;
            }

            token = read_token(buf, pos);
            if (! token)
            {
                if (top_level)
                    return 0;
                else
                    return -1;
            }
            if ((! strcmp(token, ";")) || (! strcmp(token, ",")))
            {
                free(token);
                token = NULL;
            }
            else
                if (strcmp(token, "}"))
                {
                    free(token);
                    return -1;
                }
        }
        else
        {
            if ((! token) && (! top_level))
            {
                free(name);
                return -1;
            }
            if (! (token && ((! strcmp(token, ";")) || (! strcmp(token, ",")) 
                        || (! strcmp(token, "}")))))
            {
                free(name);
                if (token) free(token);
                return -1;
            }

            res = add_field_to_array(table, name, buf, pos);
            free(name);
            if (res)
            {
                free(token);
                return -1;
            }

            if ((! token) || (! strcmp(token, "}")))
            {
                if (token) free(token);
                return 0;
            }

            if (token && ((! strcmp(token, ",")) || ((! strcmp(token, ";")))))
            {
                free(token);
                token = NULL;
            }
        }
    }
    
    return 0;
}


HTable table_read(const char *filename)
{
    long int len, pos;
    char *buf;
    HTable table;
    int res;

    buf = read_file(filename, &len);
    if (! buf) return NULL;

    pos = 0;
    table = table_create();
    res = parse_table(table, buf, &pos, 1);
    if (res)
    {
        table_free(table);
        table = NULL;
    }

    free(buf);
    return table;
}


void table_free(HTable table)
{
    SField *f, *nf;

    if (! table) return;

    f = table->fields;
    while (f)
    {
        nf = f->next;
        free_field(f);
        f = nf;
    }
    if (table->stat_buf)
        free(table->stat_buf);
    free(table);
}


HTableIterator table_get_iter(HTable table)
{
    HTableIterator it;

    it = (HTableIterator)calloc(sizeof(STableIterator), 1);
    if (! it) return NULL;

    it->table = table;
    it->before_start = 1;
    
    return it;
}


void table_free_iter(HTableIterator iterator)
{
    if (! iterator) return;
    if (iterator->stat_buf) free(iterator->stat_buf);
    free(iterator);
}


int table_iter_next(HTableIterator iterator)
{
    if (! iterator)
        return 0;

    if (iterator->before_start)
    {
        iterator->before_start = 0;
        iterator->cur_field = iterator->table->fields;
    }
    else
    {
        if (iterator->cur_field)
            iterator->cur_field = iterator->cur_field->next;
    }
    
    if (! iterator->cur_field)
        return 0;
    
    return 1;
}


char* table_iter_get_name(HTableIterator iterator)
{
    if ((! iterator) || (! iterator->cur_field))
        return NULL;

    return iterator->cur_field->name;
}


int table_iter_get_type(HTableIterator iterator)
{
    if ((! iterator) || (! iterator->cur_field))
        return -1;

    return iterator->cur_field->type;
}


static char* encode_string(char *str)
{
    char *buf, *s;
    int allocated, len;

    if (! str)
        return strdup("''");

    allocated = 20;
    buf = (char*)malloc(allocated);
    if (! buf) return NULL;
    buf[0] = '\'';
    len = 1;
    for (s = str; *s; s++)
    {
        if (len + 2 < allocated - 2)
        {
            char *m = (char*)realloc(buf, allocated += 20);
            if (! m)
            {
                free(buf);
                return NULL;
            }
            buf = m;
        }
        switch (*s)
        {
            case '\n':  buf[len++] = '\\';  buf[len++] = 'n';  break;
            case '\r':  buf[len++] = '\\';  buf[len++] = 'r';  break;
            case '\'':  buf[len++] = '\\';  buf[len++] = '\'';  break;
            case '\\':  buf[len++] = '\\';  buf[len++] = '\\';  break;
            default:
                buf[len++] = *s;
        }
    }
    buf[len++] = '\'';
    buf[len] = 0;
    return buf;
}


static char* print_field(SField *field, int butify, int spaces)
{
    char buf[100];
    
    if (! field) return NULL;
    
    switch (field->type)
    {
        case TYPE_STRING:
            return encode_string(field->value.str_value);
        case TYPE_INT:
            sprintf(buf, "%i", field->value.int_value);
            return strdup(buf);
        case TYPE_FLOAT:
            sprintf(buf, "%g", field->value.double_value);
            if (! strchr(buf, '.'))
                strcat(buf, ".0");
            return strdup(buf);
        case TYPE_TABLE:
            return table_to_str(field->value.table_value, 1, butify, spaces);
    }

    return NULL;
}


static int isSimpleArray(HTable table)
{
    SField *f;
    char buf[50];
    int i;
    
    for (f = table->fields, i = 0; f; f = f->next, i++)
    {
        sprintf(buf, "%i", i);
        if (! (f && f->name && (! strcmp(f->name, buf))))
            return 0;
    }
    return 1;
}


#define APPEND_BUF(buf, s) { int l = strlen(s); \
        if (len + l > allocated - 5) { \
            char *nb; \
            allocated += l + 50; \
            nb = (char*)realloc(buf, allocated); \
            if (! nb) { \
                free(buf); \
                return NULL; \
            } \
            buf = nb; \
        } \
        strcat(buf, s); \
        len += l; \
    }

#define APPEND_SPACES(buf, n) \
        for (int i = 0; i < n; i++) { APPEND_BUF(buf, " "); }

char* table_to_str(HTable table, int print_braces, int butify, int spaces)
{
    char *b, *fs;
    int len=0, allocated;
    SField *f;

    if (! table) return NULL;

    allocated = 100;
    b = (char*)malloc(allocated);
    if (! b) return NULL;

    if (print_braces) {
        if (butify)
            strcpy(b, "{\n");
        else
            strcpy(b, "{ ");
    } else
        strcpy(b, "");
    len = strlen(b);
    int printNames = ! isSimpleArray(table);
    for (f = table->fields; f; f = f->next) {
        if (butify) {
            APPEND_SPACES(b, spaces);
        } else
            if (f != table->fields) {
                APPEND_BUF(b, " ");
            }
        if (printNames) {
            APPEND_BUF(b, f->name);
            APPEND_BUF(b, " = ");
        }
        fs = print_field(f, butify, spaces + 4);
        APPEND_BUF(b, fs);
        free(fs);
        if (butify) {
            APPEND_BUF(b, ";\n");
        } else {
            APPEND_BUF(b, ";");
        }
    }
    if (print_braces) {
        if (! butify) {
            APPEND_BUF(b, " }");
        } else {
            APPEND_SPACES(b, spaces);
            APPEND_BUF(b, "}");
        }
    }
    return b;
}


static char* get_field_str(SField *field, int *err)
{
    char buf[100];
    
    if (err) *err = 0;

    if (! field) 
    {
        if (err) *err = 1;
        return NULL;
    }

    switch (field->type)
    {
        case TYPE_STRING:
            return strdup(field->value.str_value);
        case TYPE_INT:
            sprintf(buf, "%i", field->value.int_value);
            return strdup(buf);
        case TYPE_FLOAT:
            sprintf(buf, "%f", field->value.double_value);
            return strdup(buf);
        case TYPE_TABLE:
            return table_to_str(field->value.table_value, 1, 0, 0);;
    }
    
    if (err) *err = 1;
    return NULL;
}


char* table_iter_get_str(HTableIterator iterator, int *err)
{
    if ((! iterator) || (! iterator->cur_field))
    {
        if (err) *err = 1;
        return NULL;
    }

    return get_field_str(iterator->cur_field, err);
}


char* table_iter_get_strs(HTableIterator iterator, int *err)
{
    if ((! iterator) || (! iterator->cur_field))
    {
        if (err) *err = 1;
        return NULL;
    }

    if (iterator->stat_buf)
        free(iterator->stat_buf);
    iterator->stat_buf = get_field_str(iterator->cur_field, err);
    return iterator->stat_buf;
}


static int get_field_int(SField *field, int *err)
{
    char *endptr;
    int n;

    if (err) *err = 0;

    if (! field) 
    {
        if (err) *err = 1;
        return 0;
    }

    switch (field->type)
    {
        case TYPE_STRING:
            n = strtol(field->value.str_value, &endptr, 10);
            if ((! field->value.str_value[0]) || (endptr[0])) 
            {
                if (err) *err = 1;
                return 0;
            }
            return n;
        case TYPE_INT:
            return field->value.int_value;
        case TYPE_FLOAT:
            return (int)field->value.double_value;
    }
    
    if (err) *err = 1;
    return 0;
}


int table_iter_get_int(HTableIterator iterator, int *err)
{
    if ((! iterator) || (! iterator->cur_field))
    {
        if (err) *err = 1;
        return 0;
    }

    return get_field_int(iterator->cur_field, err);
}


static double get_field_double(SField *field, int *err)
{
    char *endptr;
    double n;

    if (err) *err = 0;

    if (! field) 
    {
        if (err) *err = 1;
        return 0.0;
    }

    switch (field->type)
    {
        case TYPE_STRING:
            n = strtod(field->value.str_value, &endptr);
            if ((! field->value.str_value[0]) || (endptr[0])) 
            {
                if (err) *err = 1;
                return 0.0;
            }
            return n;
        case TYPE_INT:
            return field->value.int_value;
        case TYPE_FLOAT:
            return field->value.double_value;
    }
    
    if (err) *err = 1;
    return 0.0;
}


double table_iter_get_double(HTableIterator iterator, int *err)
{
    if ((! iterator) || (! iterator->cur_field))
    {
        if (err) *err = 1;
        return 0.0;
    }

    return get_field_double(iterator->cur_field, err);
}


HTable table_iter_get_table(HTableIterator iterator, int *err)
{
    if ((! iterator) || (! iterator->cur_field))
    {
        if (err) *err = 1;
        return NULL;
    }

    if (iterator->cur_field->type != TYPE_TABLE)
    {
        if (err) *err = 1;
        return NULL;
    }
    
    if (err) *err = 0;
    return iterator->cur_field->value.table_value;
}


int table_is_field_exists(HTable table, char *name)
{
    SField *f;

    f = find_field(table, name);
    if (f)
        return 1;
    else
        return 0;
}


int table_get_field_type(HTable table, char *field)
{
    SField *f;

    f = find_field(table, field);
    if (! f)
        return -1;
        
    return f->type;
}


char* table_get_str(HTable table, char *field, char *dflt, int *err)
{
    SField *f;
    char *s;
    int e;
    
    f = find_field(table, field);
    if (! f)
    {
        if (err) *err = 1;
        if (dflt)
            return strdup(dflt);
        else
            return NULL;
    }

    s = get_field_str(f, &e);
    if (err) *err = e;
    if (e)
    {
        if (dflt)
            return strdup(dflt);
        else
            return NULL;
    }
    else
    {
        if (s)
            return strdup(s);
        else
            return NULL;
    }
}


char* table_get_strs(HTable table, char *field, char *dflt, int *err)
{
    SField *f;
    char *s;
    int e;

    if (! table)
    {
        if (err) *err = 1;
        return dflt;
    }

    if (table->stat_buf)
    {
        free(table->stat_buf);
        table->stat_buf = NULL;
    }
    
    f = find_field(table, field);
    if (! f)
    {
        if (err) *err = 1;
        return dflt;
    }

    s = get_field_str(f, &e);
    table->stat_buf = s;
    if (err) *err = e;
    if (e)
        s = dflt;
        
    return s;
}


int table_get_int(HTable table, char *field, int dflt, int *err)
{
    SField *f;
    int v, e;
    
    f = find_field(table, field);
    if (! f)
    {
        if (err) *err = 1;
        return dflt;
    }

    v = get_field_int(f, &e);
    if (err) *err = e;
    if (e)
        v = dflt;
        
    return v;
}


double table_get_double(HTable table, char *field, double dflt, int *err)
{
    SField *f;
    double v;
    int e;
    
    f = find_field(table, field);
    if (! f)
    {
        if (err) *err = 1;
        return dflt;
    }

    v = get_field_double(f, &e);
    if (err) *err = e;
    if (e)
        v = dflt;
        
    return v;
}


HTable table_get_table(HTable table, char *field, HTable dflt, int *err)
{
    SField *f;
    HTable v;
    
    f = find_field(table, field);
    if ((! f) || (f->type != TYPE_TABLE))
    {
        if (err) *err = 1;
        return dflt;
    }

    v = f->value.table_value;
    if (err) *err = 0;
        
    return v;
}


int table_append_str(HTable table, const char *val)
{
    int idx;
    char b[100];

    if ((! table) || (! val))
        return -1;

    idx = get_next_index(table);
    sprintf(b, "%i", idx);
    table_set_str(table, b, val);
    return 0;
}


int table_append_int(HTable table, int val)
{
    int idx;
    char b[100];

    if ((! table) || (! val))
        return -1;

    idx = get_next_index(table);
    sprintf(b, "%i", idx);
    table_set_int(table, b, val);
    return 0;
}


int table_append_double(HTable table, double val)
{
    int idx;
    char b[100];

    if ((! table) || (! val))
        return -1;

    idx = get_next_index(table);
    sprintf(b, "%i", idx);
    table_set_double(table, b, val);
    return 0;
}


int table_append_table(HTable table, HTable val)
{
    int idx;
    char b[100];

    if ((! table) || (! val))
        return -1;

    idx = get_next_index(table);
    sprintf(b, "%i", idx);
    table_set_table(table, b, val);
    return 0;
}

