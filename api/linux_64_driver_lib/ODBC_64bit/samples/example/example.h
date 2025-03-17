/*
** Copyright: (c) 2002-2022, 2024 Progress Software Corporation
**                and/or its subsidiaries or affiliates.
**                All Rights Reserved.
*/

/*
** File:	example.h
** Contains #defines, structure and function definitions for the example.c program
*/

/* same typedef OK for most platforms except PowerPC: error: redefinition of `BOOL', previously declared in sqlunx.h:42 */
#if !defined(__SQLUNX)
typedef int BOOL;
#endif

#if !defined(TRUE)
#define TRUE    1
#define FALSE   0
#endif

#define OPT1_LEN        	255
#define OPT2_LEN        	255
#define TMPSTR_LEN      	255
#define SQL_STATEMENT_LEN 	1024
#define MAX_COLUMNS 		1024 
#define COL_NAME_LEN    	255
#define COL_VALUE_LEN    	255
#define MAX_DISPLAY_SIZE 	200
#define INFO_BUFFER_LEN	1024
#define COLUMN_SEPERATOR_OFFSET 4
#define DSN_LEN         	64	
#define PWD_MSG1 _T("Requested password exceeds compiled limit of %d.\n")
#define PWD_ERR1 _T("Password not found after keyword -pwd on command line.\n")
#define UID_MSG1 _T("Requested username exceeds compiled limit of %d.\n")
#define UID_ERR1 _T("Username not found after keyword -uid on command line.\n")
#define OPT1_ERR1 _T("Required value not found after keyword -opt1 on command line.\n")
#define OPT1_ERR2 _T("Option 1(-opt1) exceeds compiled limit of %d.\n")
#define OPT2_ERR1 _T("Required value not found after keyword -opt2 on command line.\n")
#define OPT2_ERR2 _T("Option 2(-opt2) exceeds compiled limit of %d.\n")
#define BANNER _T("%s DataDirect Technologies, Inc. ODBC Example Application.\n")
#define MAX_COL_ERROR	_T("Requested result set exceeds the compiled limit of %i. \n")

/*
** Predefined commands
*/

#define EX_CMD_COLDUMP _T("columninfo")
#define EX_CMD_RUN _T("run")
#define EX_CMD_DBINFO _T("dbinfo")
#define EX_CMD_DRVINFO _T("driverinfo")
#define EX_CMD_HELP _T("help")
#define EX_CMD_HEXDUMP _T("hexdump")
#define EX_CMD_QUIT _T("quit")
#define EX_CMD_DEMOINSERT _T("demoinsert")

typedef struct 
{
	SQLTCHAR  szColName [COL_NAME_LEN] ;
	SQLSMALLINT cbColName ;
	SQLSMALLINT fSqlType ;
	SQLULEN cbColDef ;
	SQLSMALLINT ibScale ;
	SQLSMALLINT fNullable ;
} ColInfoStruct ;

typedef struct 
{
	SQLTCHAR charCol [COL_VALUE_LEN];
	SQLLEN length ;
} DataInfoStruct ;

DataInfoStruct dataStruct [MAX_COLUMNS] ;
ColInfoStruct  infoStruct [MAX_COLUMNS] ;

void PrintHelp();

SQLRETURN BindColumns (SQLHSTMT hstmt, ColInfoStruct *colInfo, DataInfoStruct *dataInfo, SQLUSMALLINT icol) ;

SQLRETURN DisplayResults (SQLHSTMT hstmt, SQLSMALLINT numCols) ;

SQLRETURN ExecuteSQL(SQLHSTMT hstmt, SQLTCHAR * strSQL, FILE *timeFile);

SQLRETURN DumpColumnInfo(SQLHSTMT hstmt);

SQLRETURN ExecuteFile(SQLHSTMT hstmt, const TCHAR* szFileName);

SQLRETURN PrintDatabaseInfo(SQLHDBC hdbc);

SQLRETURN PrintDriverInfo(SQLHDBC hdbc);

SQLRETURN demoInsert(SQLHSTMT hstmt, FILE* timeFile);

BOOL g_fHexDump;

void HexDump(void *addr, SQLLEN len);
