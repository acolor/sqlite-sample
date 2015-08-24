//
//  main.c
//  sqlite_sample
//
//  Created by 陳逸青 on 2015/8/24.
//  Copyright (c) 2015年 ___org_name___. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

#ifndef bool
#define bool  int
#endif
#ifndef TRUE
#define TRUE (1==1)
#endif
#ifndef FALSE
#define FALSE (1==0)
#endif

static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
    printf("ok\n");
    return 0;
}


void run_sql_method2(void)
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    
    rc = sqlite3_open("123", &db);
    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
    }
    rc = sqlite3_exec(db, "command", callback, 0, &zErrMsg);
    if( rc!=SQLITE_OK ) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    sqlite3_close(db);
}


#define DEFAULT_DB_FILENAME             "test.db"
#define SQLITE_API_ERROR(db, api_string)    fprintf(stderr, "[%s] error:%s\n", api_string, sqlite3_errmsg(db))
void run_sql_method1(void)
{
    sqlite3 *db_test;
    sqlite3_stmt *stmt = NULL;
    int rc, i, row_count, column_count, column_type, is_read_more;
    char sql_string[512], sqlite_filename[32];
    const char *p_sql_remain;
    int int_value, value_length;
    double float_value;
    const unsigned char *p_text_value;
    const void *p_blob_value;
    
    //1.create database object
    printf("Enter filename(default:%s) :", DEFAULT_DB_FILENAME);
    gets(sqlite_filename);
    if (!strlen(sqlite_filename))
        strncpy(sqlite_filename, DEFAULT_DB_FILENAME, 32);
    rc = sqlite3_open(sqlite_filename, &db_test);
    if (rc != SQLITE_OK) {
        SQLITE_API_ERROR(db_test, "sqlite3_open");
        goto exit_close_database;
    }
    
    printf("Enter SQL statement, 'quit' to exit.\n");
    for ( ; ; ) {
        printf("SQL > ");
        gets(sql_string);
        if (!strlen(sql_string))
            continue;
        else if (!strcmp(sql_string, "quit")) {
            printf("exit test!\n");
            break;
        }
        
        //2.create statement object
        rc = sqlite3_prepare_v2(db_test, sql_string, -1, &stmt, &p_sql_remain);
        if (rc != SQLITE_OK) {
            SQLITE_API_ERROR(db_test, "sqlite3_prepare_v2");
            goto reset_statement;
        }
        
        row_count = 1;
        do {
            //3. Execute SQL statement
            rc = sqlite3_step(stmt);
            if (rc == SQLITE_BUSY) {
                /* if stmt is "COMMIT"  then retry, else rollback the transaction */
            } else if (rc == SQLITE_DONE || rc == SQLITE_ROW) {
            } else if (rc == SQLITE_ERROR) {
                SQLITE_API_ERROR(db_test, "sqlite3_step");
                goto free_stmt;
            } else if (rc == SQLITE_MISUSE) {
                SQLITE_API_ERROR(db_test, "sqlite3_step");
            } else {
                SQLITE_API_ERROR(db_test, "sqlite3_step");
            }
            is_read_more = FALSE;
            if (rc == SQLITE_ROW) {
                column_count = sqlite3_column_count(stmt);
                for (i = 0; i < column_count; i ++) {
                    column_type = sqlite3_column_type(stmt, i);
                    switch (column_type) {
                        case SQLITE_INTEGER:
                            int_value = sqlite3_column_int(stmt, i);
                            printf("%d.%d I(%d)\n", row_count, i, int_value);
                            break;
                        case SQLITE_FLOAT:
                            float_value = sqlite3_column_double(stmt, i);
                            printf("%d.%d F(%f)\n", row_count, i, float_value);
                            break;
                        case SQLITE_TEXT:
                            p_text_value = sqlite3_column_text(stmt, i);
                            value_length = sqlite3_column_bytes(stmt, i);
                            printf("%d.%d T(%s)\n", row_count, i, p_text_value);
                            break;
                        case SQLITE_BLOB:
                            p_blob_value = sqlite3_column_blob(stmt, i);
                            value_length = sqlite3_column_bytes(stmt, i);
                            printf("%d.%d B(%d,%p)\n", row_count, i, value_length, p_blob_value);
                            break;
                        case SQLITE_NULL:
                            printf("%d.%d Null\n", row_count, i);
                            break;
                        default:
                            break;
                    }
                }
                is_read_more = TRUE;
                row_count ++;
            }
        } while (is_read_more);
        
    reset_statement:
        //4. reset statement object
        rc = sqlite3_reset(stmt);
        if (rc != SQLITE_OK)
            SQLITE_API_ERROR(db_test, "sqlite3_reset");
    }
    
free_stmt:
    if (stmt) {
        //5. free statement object
        rc = sqlite3_finalize(stmt);
        if (rc != SQLITE_OK)
            SQLITE_API_ERROR(db_test, "sqlite3_finalize");
    }
    
exit_close_database:
    //6. close database
    sqlite3_close(db_test);
    if (rc != SQLITE_OK)
        SQLITE_API_ERROR(db_test, "sqlite3_close");
    
}


int main(int argc, const char * argv[]) {

#ifdef SQL_EXE_MODE
    run_sql_method2();
#else
    run_sql_method1();
#endif
    return 0;
}
