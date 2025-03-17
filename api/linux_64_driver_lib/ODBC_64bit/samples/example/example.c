/*
** Copyright: (c) 2002-2022, 2024 Progress Software Corporation
**                and/or its subsidiaries or affiliates.
**                All Rights Reserved.
*/

/*
** File:	example.c
**
** Purpose:	To demonstrate some of the ODBC function calls and to allow
**		statements to be entered by the user at will.
**
** Upgraded and enhanced by Eugene Liu on Oct. 2024
**		* to use ODBC3 behavior and APIs by default.
**		* to hide password input
**		* to support Unicode
*/

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <tchar.h>		/* for TCHAR, _T(x) stuff */
#include <io.h>			/* for _setmode */
#include <fcntl.h>		/* for _O_U16TEXT */
#else
#define TCHAR			char
#define _T(x)			x
#define _fgetts			fgets
#define _ftprintf		fprintf
#define _stat			stat
#define _stprintf		sprintf
#define _taccess		access
#define _tchmod			chmod
#define _tcschr			strchr
#define _tcscmp			strcmp
#define _tcscpy			strcpy
#define _tcslen			strlen
#define _tcsncmp		strncmp
#define _tfopen			fopen
#define _tmain			main
#define _tprintf		printf
#define	_tsetlocale		setlocale
#define _tstat			stat
#define _ttoi			atoi
#define _timeb			timeb
#define _ftime			ftime
#define _tctime			ctime
#endif

#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <locale.h>
#include <sqlext.h>
#include <time.h>
#include <errno.h>
#if defined(__sun) && defined(__SVR4)
#include <sys/inttypes.h>
#else
#include <stdint.h>
#endif
#include <sys/timeb.h>

#ifdef __cplusplus
}
#endif

#include "example.h"

/*
** Define Some useful defines
*/
#if !defined (NULL)
#define	NULL	0
#endif

/* Reads a line from stdin, without the trailing newline-TCHAR */
/* return 0 if stdin error or buf empty and the entered text length otherwise */
/* note that size is for INPUT and OUTPUT: the buffer size on INPUT and the entered text length on OUTPUT */
/* define SHOW_INPUT below to show the input string if stdin redirected in from a file */
/* #define SHOW_INPUT */
#if defined(SHOW_INPUT)
#define PRINT_INPUT(buf)		&& _tprintf(_T("%s\n"), buf)
#else
#define PRINT_INPUT(buf)
#endif
#define readLine(buf,size) ((NULL != _fgetts((TCHAR *)buf, size, stdin) && (buf[(size=(int)_tcslen((const TCHAR*)buf)-1)]=0, size) PRINT_INPUT(buf)), size)

/*
** function: ODBC_error
**
** Purpose:	Display to stdout current ODBC Errors
**
** Arguments:
**		handleType - ODBC Handle type, can be one of these:
**			SQL_HANDLE_ENV
**			SQL_HANDLE_DBC
**			SQL_HANDLE_STMT
**		handle     - ODBC Handle error generated on, can be one of these:
**`			henv    _ ODBC Environment handle.
**			hdbc    - ODBC Connection Handle error generated on.
**			hstmt	- ODBC SQL Handle error generated on.
**
** Returns:	void
**
*/

void ODBC_error(SQLSMALLINT handleType, SQLHANDLE handle)
{
	SQLTCHAR	sqlstate[6];
	SQLTCHAR	errmsg[SQL_MAX_MESSAGE_LENGTH];
	SQLINTEGER nativeerr;
	SQLSMALLINT returnmsglen;
	SQLRETURN	rc;
	SQLSMALLINT	recNumber = 1;

	while (TRUE)
	{
		rc = SQLGetDiagRec(handleType, handle, recNumber, sqlstate, &nativeerr, errmsg, (sizeof(errmsg)/sizeof(errmsg[0])), &returnmsglen); /* returnmsglen is the required length if it's >= SQL_MAX_MESSAGE_LENGTH */
		if (rc == SQL_ERROR)
		{
			_tprintf(_T("SQLGetDiagRec failed!\n"));
			break;
		}
		if (rc == SQL_NO_DATA_FOUND)
		{
			break;
		}
		if (recNumber > 1) _tprintf(_T("\n"));
		_tprintf(_T("	SQLSTATE = %s\n"), sqlstate);
		_tprintf(_T("	NATIVE ERROR = %d\n"), nativeerr);
		errmsg[returnmsglen >= (sizeof(errmsg) / sizeof(errmsg[0])) ? (sizeof(errmsg) / sizeof(errmsg[0])) - 1 : returnmsglen] = '\0';
		_tprintf(_T("	MSG = %s\n"), errmsg);
		if (returnmsglen >= (sizeof(errmsg) / sizeof(errmsg[0])))
		{
			_tprintf(_T("	MSG is truncated. The required length = %d\n"), returnmsglen);
		}
		++recNumber;
	}
}

#if defined(_WIN32) || defined(_WIN64)
#define DRIVER_COMPLETION SQL_DRIVER_COMPLETE
#else
#define DRIVER_COMPLETION SQL_DRIVER_NOPROMPT
#endif

/*
** function: ODBC_Connect
**
** Purpose:	Allocates ODBC SQLHENV and SQLHDBC.
**
** Arguments:	henv    _ Pointer to environment handle
**		hdbc    - Pointer to connection handle
**
** Returns:	SQLRETURN - Return status from last ODBC Function.
**
*/

SQLRETURN ODBC_Connect(			/* Perform Driver Connection	*/
	SQLHENV henv,			/* ODBC Environment Handle	*/
	SQLHDBC hdbc,			/* ODBC Connection Handle	*/
	SQLTCHAR* dataSourceName,		/* Data Source Name		*/
	SQLTCHAR* uid,			/* User ID			*/
	SQLTCHAR* pwd,			/* User Password		*/
	SQLTCHAR* opt1,			/* User Specified Option 1	*/
	SQLTCHAR* opt2)			/* User Specified Option 2	*/
{
	SQLRETURN	rc;
	int	retries;

	/* Use SQLDriverConnect if the dataSourceName is a connection string; otherwise use SQLConnect */
	BOOL bConnStr = (NULL != _tcschr((const TCHAR*)dataSourceName, '='));

	/*
	** try connecting up to 3 times
	*/
	for (retries = 1; retries <= 3; retries++) {
		if (bConnStr)
		{
			rc = SQLDriverConnect(hdbc, NULL, dataSourceName, SQL_NTS, NULL, 0, NULL, DRIVER_COMPLETION);
		}
		else
		{
			rc = SQLConnect(hdbc, dataSourceName, SQL_NTS,
				uid, SQL_NTS, pwd, SQL_NTS);
		}
		if ((rc == SQL_SUCCESS) && (opt1 != NULL)) {
			return rc;
		}
		else if ((rc == SQL_SUCCESS_WITH_INFO) && (opt1 != NULL)) {
			ODBC_error(SQL_HANDLE_DBC, hdbc);
			return rc;
		}
		else {
			ODBC_error(SQL_HANDLE_DBC, hdbc);
			_tprintf(_T("SQL%sConnect: Retrying Connect.\n"), bConnStr? _T("Driver") : _T(""));
		}
	}
	/*
	** Attempt to obtain a meaningful error as to why connect failed.
	*/
	ODBC_error(SQL_HANDLE_DBC, hdbc);
	return SQL_INVALID_HANDLE;
}

/*
** function:	EnvInit
**
** Purpose:	Allocates ODBC SQLHENV and SQLHDBC.
**
** Arguments:
**		henv    - Pointer to environment handle
**		hdbc    - Pointer to connection handle
**		odbcVer - ODBC version to use. Can be 2 for SQL_OV_ODBC2, 3 for SQL_OV_ODBC3, or 380 for SQL_OV_ODBC3_80
** Returns:	SQLRETURN status from ODBC Functions.
*/
SQLRETURN EnvInit(SQLHENV* henv, SQLHDBC* hdbc, SQLUINTEGER odbcVer)
{
	SQLRETURN rc;

	rc = SQLAllocHandle(SQL_HANDLE_ENV, NULL, henv);
	if (SQL_SUCCEEDED(rc))
	{
		rc = SQLSetEnvAttr(*henv, SQL_ATTR_ODBC_VERSION, (void*)(size_t)odbcVer, SQL_IS_UINTEGER);
		if (SQL_SUCCEEDED(rc))
		{
			rc = SQLAllocHandle(SQL_HANDLE_DBC, *henv, hdbc);
		}
	}
	return rc;
}

/*
** function:	EnvClose
**
** Arguments:	henv    _ environment handle
**		hdbc    - connection to handle
*/
void EnvClose(SQLHENV henv, SQLHDBC hdbc)
{
	SQLDisconnect(hdbc);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

#define PASSWORD_INVISIBLE
#if defined(PASSWORD_INVISIBLE)
#define INVISIBLE "(invisible)"
#if defined(_WIN32) || defined(_WIN64)
#include <conio.h>
#define GETCH _gettch
#define ENTERCH '\r'
#define enableEcho(x)
#else
#include <termios.h>
#include <unistd.h>
#define ENTERCH '\n'
#define GETCH getchar
void enableEcho(BOOL bEnableEcho)
{
	struct termios tmpt;
	tcgetattr(STDIN_FILENO, &tmpt);
	bEnableEcho? (tmpt.c_lflag |= ECHO) : (tmpt.c_lflag &= ~ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &tmpt);
}
#endif
int getPassword(SQLTCHAR* password, int passwordSize) /* return the length of the password entered */
{
	int i = 0, ch;
	enableEcho(FALSE);

#if defined(_WIN32) || defined(_WIN64) 
	// Check if stdin is redirected from a file
	if (!_isatty(_fileno(stdin))) {
#else
	if (!isatty(fileno(stdin))) {
#endif
		readLine(password, passwordSize);
	}
	
	else {
		while ((ch = GETCH()) != ENTERCH && i < passwordSize - 1) {
			password[i++] = (SQLTCHAR)ch;
			_tprintf(_T("*")); /* Print asterisk for each character */
		}
		password[i] = 0; /* Null-terminate the string */
	}
	enableEcho(TRUE);
	return i;
}
#else
#define INVISIBLE "           "
int getPassword(SQLTCHAR* password, int passwordSize) /* return the length of the password entered */
{
	return readLine(password, passwordSize);
}
#endif

/* This is the only place to increase or decrease the sql/cmd buffer size to be allocated */
#define MAX_SQL_CMD_BUFFER_SIZE (SQL_STATEMENT_LEN * 512)

/*
** Program:	example
**
** Purpose:	To enable driver testing by allowing online queries.
**
** Written By:  Anantanarayanan Iyengar
*/
int _tmain(int argc, TCHAR* argv[])
{
	SQLHENV		henv;
	SQLHDBC		hdbc;
	SQLHSTMT	hstmt;
	SQLRETURN	rc;
	SQLTCHAR*	uid = NULL;
	SQLTCHAR*	pwd = NULL;
	int     	dsnSqlBufLen = 0;
	SQLTCHAR*	dsnSqlBuf = NULL;
	SQLTCHAR*	dataSource = NULL;
	SQLTCHAR*	sqlStatement = NULL;
	SQLTCHAR	opt1[OPT1_LEN], opt2[OPT2_LEN];
	FILE*		timeFile = NULL;

	SQLUINTEGER odbcVer = SQL_OV_ODBC3; /* Default to use ODBC3 behavior */
	int argcOffset = 1;
	int exitCode = 0;

#if defined(_UNICODE)
	/* A must for _UNICODE!
	 * Setting either set seems working the same. 
	 * Without it, I could not figure out how to read/display the Unicode characters correctly.
	 * Besides this, the client window needs to use true font like NSimSun and SimSun-ExtB
	*/
#pragma warning(disable:6031) /* no need to check the function's return value */
	_setmode(_fileno(stdin),  _O_U16TEXT);
	_setmode(_fileno(stdout), _O_U16TEXT);
#pragma warning(default:6031)
#endif

#if !defined (__cplusplus) && defined (hppa)
	/*
	** C programs must call the HP C++ Object initializer function.
	*/
	_main();
#endif

	/* This is make sure that OS string functions work correctly with DBCS */
	_tsetlocale(LC_ALL, _T(""));

	/* allocate memory for sql/cmd to use */
	dsnSqlBuf = (SQLTCHAR*)malloc(MAX_SQL_CMD_BUFFER_SIZE * sizeof(TCHAR));
	if (!dsnSqlBuf)
	{
		_tprintf(_T("Memory allocation failed!\n"));
		exitCode = 1;
		goto errRet;
	}

	/* init */
	memset(&dataStruct, 0, sizeof(dataStruct));
	memset(&infoStruct, 0, sizeof(infoStruct));

	/*
	** Define Table and Driver
	*/
	_tprintf(BANNER, argv[0]);
	opt1[0] = '\0';
	opt2[0] = '\0';

	/*
	** Process command line arguments starting with '-'
	*/
	if (argc > 1)
	{
		while (argcOffset < argc && '-' == argv[argcOffset][0])
		{
			/*
			** Open a timed result file if -t is given, such as -t or -tMyOutFile.txt
			*/
			if ('t' == argv[argcOffset][1] || 'T' == argv[argcOffset][1])
			{
#if defined(_WIN32) || defined(_WIN64)
				if (0 != _tfopen_s(&timeFile, argv[argcOffset][2] ? argv[argcOffset] + 2 : _T("timedResults.csv"), _T("a"))) timeFile = NULL;
#else
				timeFile = fopen(argv[argcOffset][2] ? argv[argcOffset] + 2 : _T("timedResults.csv"), _T("a"));
#endif
				if (!timeFile) {
					_tprintf(_T("WARNING: Could not create/open result file %s\n"), argv[argcOffset][2] ? argv[argcOffset] + 2 : _T("timedResults.csv"));
					_tprintf(_T("ERRNO: %d\n"), errno);
				}
			}
			/* -o2, -o3, -o380, ... for ODBC odbcVer behavior */
			else if ('o' == argv[argcOffset][1] || 'O' == argv[argcOffset][1])
			{
				if (argv[argcOffset][3] == 0)
				{
					odbcVer = 0;
					if ('2' == argv[argcOffset][2])
					{
						odbcVer = SQL_OV_ODBC2;
					}
					else if ('3' == argv[argcOffset][2])
					{
						odbcVer = SQL_OV_ODBC3;
					}
				}
				else
				{
					/* only 380 for SQL_OV_ODBC3_80 supported as of 2024-09. !!!Any unsupported value may cause the DM crash!!! */
					odbcVer = _ttoi(argv[argcOffset] + 2);
				}
				if (!odbcVer)
				{
					_tprintf(_T("Invalid ODBC version specified! Default to SQL_OV_ODBC3\n"));
					odbcVer = SQL_OV_ODBC3;
				}
			}
			argcOffset++;
		}
	}

	/*
	** Prompt for the data source name, the user name and the password.
	*/
	if (argc == argcOffset)
	{
		PrintHelp();

		// Read Data Source Name
		_tprintf(_T("\nEnter the data source name      : "));
		dataSource = dsnSqlBuf;
		dsnSqlBufLen = MAX_SQL_CMD_BUFFER_SIZE;
		readLine(dataSource, dsnSqlBufLen); /* dsnSqlBufLen is changed from the buffer size on INPUT to the sql/cmd text length on OUTPUT */

		// Read User ID
		_tprintf(_T("\nEnter the user name             : "));
		uid = dataSource + dsnSqlBufLen + 1;
		dsnSqlBufLen = (int)(dsnSqlBuf + MAX_SQL_CMD_BUFFER_SIZE - uid); /* use the remaining buffer */
		readLine(uid, dsnSqlBufLen);

		// Read Password
		_tprintf(_T("\nEnter the password " INVISIBLE "  : "));
		pwd = uid + dsnSqlBufLen + 1;
		dsnSqlBufLen = (int)(dsnSqlBuf + MAX_SQL_CMD_BUFFER_SIZE - pwd); /* use the remaining buffer */
		getPassword(pwd, dsnSqlBufLen);
		_tprintf(_T("\n"));
	}

	/* use command line argument*/
	else
	{
		dataSource = (SQLTCHAR*)argv[argcOffset++];
		if (!_tcschr((const TCHAR*)dataSource, '='))
		{
			if (argc > argcOffset) uid = (SQLTCHAR*)argv[argcOffset++];
			if (argc > argcOffset) pwd = (SQLTCHAR*)argv[argcOffset++];
		}
	}

	EnvInit(&henv, &hdbc, odbcVer);

	rc = ODBC_Connect(henv, hdbc, dataSource, uid, pwd, opt1, opt2);

	/* clear the UID and PWD in memory if not NULL */
	if (dsnSqlBufLen != MAX_SQL_CMD_BUFFER_SIZE && uid && pwd)
	{
		memset(uid, 0, pwd + dsnSqlBufLen - uid);
	}

	/* check connection error */
	if (!SQL_SUCCEEDED(rc))
	{
		exitCode = 2;
		goto errRet;
	}

	/*
	** Show dbinfo and driverinfo if no other command line argument is given
	*/
	if (argc == argcOffset)
	{
		_tprintf(_T("\n-------------------- Database info from the internal command dbinfo:\n"));
		rc = PrintDatabaseInfo(hdbc);
		if (!SQL_SUCCEEDED(rc))
		{
			ODBC_error(SQL_HANDLE_DBC, hdbc);
		}

		_tprintf(_T("\n-------------------- Driver info from the internal command driverinfo:\n"));
		rc = PrintDriverInfo(hdbc);
		if (!SQL_SUCCEEDED(rc))
		{
			ODBC_error(SQL_HANDLE_DBC, hdbc);
		}

	}

	/*
	** Allocate a SQLHSTMT to communicate with ODBC DB Driver.
	*/
	rc = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (!SQL_SUCCEEDED(rc)) {
		ODBC_error(SQL_HANDLE_DBC, hdbc);
		EnvClose(henv, hdbc);
		exitCode = 3;
		goto errRet;
	}

	if (timeFile) {
		_ftprintf(timeFile, _T("\nDATASOURCE = %s\n\n"), dataSource);
		_ftprintf(timeFile, _T("QUERY, EXECUTE TIME (milliseconds), EXECUTE+FETCH TIME (milliseconds)\n"));
	}

	/*
	** Start the infinite loop for accepting statements from the user or command line if given
	*/
	while (TRUE)
	{
		if (argc > argcOffset)
		{
			sqlStatement = (SQLTCHAR * )argv[argcOffset++];
			_tprintf(_T("\n-------------------- SQL|Internal Command: \"%s\" --------------------\n\n"), sqlStatement);
		}
		else
		{
			sqlStatement = dsnSqlBuf;
			dsnSqlBufLen = MAX_SQL_CMD_BUFFER_SIZE;

			_tprintf(_T("\nEnter SQL statements (Press ENTER to QUIT)\n"));
			_tprintf(_T("SQL> "));
			if (!readLine(sqlStatement, dsnSqlBufLen))
			{
				_tprintf(_T("Exiting from the Example ODBC program\n"));
				break;
			}
		}

		/*
		** Determine if input is a known Example command
		*/

		if (!_tcsncmp((const TCHAR*)sqlStatement, EX_CMD_HELP, sizeof(EX_CMD_HELP) / sizeof(TCHAR) - 1)) {
			PrintHelp();
			continue;
		}

		if (!_tcsncmp((const TCHAR*)sqlStatement, EX_CMD_COLDUMP, sizeof(EX_CMD_COLDUMP) / sizeof(TCHAR) - 1)) {
			rc = DumpColumnInfo(hstmt);
			if (rc != SQL_SUCCESS) {
				ODBC_error(SQL_HANDLE_STMT, hstmt);
				SQLFreeStmt(hstmt, SQL_CLOSE);
			}
			continue;
		}

		if (!_tcsncmp((const TCHAR*)sqlStatement, EX_CMD_RUN, sizeof(EX_CMD_RUN) / sizeof(TCHAR) - 1)) {
			rc = ExecuteFile(hstmt, (const TCHAR*)sqlStatement + (sizeof(EX_CMD_RUN) / sizeof(TCHAR)));

			if (!SQL_SUCCEEDED(rc))
			{
				ODBC_error(SQL_HANDLE_STMT, hstmt);
				SQLFreeStmt(hstmt, SQL_CLOSE);
			}

			continue;
		}

		if (!_tcsncmp((const TCHAR*)sqlStatement, EX_CMD_DBINFO, sizeof(EX_CMD_DBINFO) / sizeof(TCHAR) - 1)) {
			rc = PrintDatabaseInfo(hdbc);

			if (!SQL_SUCCEEDED(rc))
			{
				ODBC_error(SQL_HANDLE_DBC, hdbc);
			}

			continue;
		}

		if (!_tcsncmp((const TCHAR*)sqlStatement, EX_CMD_DRVINFO, sizeof(EX_CMD_DRVINFO) / sizeof(TCHAR) - 1)) {
			rc = PrintDriverInfo(hdbc);

			if (!SQL_SUCCEEDED(rc))
			{
				ODBC_error(SQL_HANDLE_DBC, hdbc);
			}

			continue;
		}

		if (!_tcsncmp((const TCHAR*)sqlStatement, EX_CMD_QUIT, sizeof(EX_CMD_QUIT) / sizeof(TCHAR) - 1)) {
			_tprintf(_T("Exiting from the Example ODBC program\n"));
			break;
		}

		if (!_tcsncmp((const TCHAR*)sqlStatement, EX_CMD_HEXDUMP, sizeof(EX_CMD_HEXDUMP) / sizeof(TCHAR) - 1)) {
			g_fHexDump = g_fHexDump ? FALSE : TRUE;
			_tprintf(_T("Hex dump : %s\n"), g_fHexDump ? _T("on") : _T("off"));
			continue;
		}

		if (!_tcsncmp((const TCHAR*)sqlStatement, EX_CMD_DEMOINSERT, sizeof(EX_CMD_DEMOINSERT) / sizeof(TCHAR) - 1)) {
			rc = demoInsert(hstmt, timeFile);
			if (!SQL_SUCCEEDED(rc))
			{
				ODBC_error(SQL_HANDLE_STMT, hstmt);
			}
			continue;
		}

		/*
		** Unbind all the columns which were bound earlier before calling SQLFreeStmt
		** with the SQL_UNBIND option. Also close any open cursors on the
		** statement handle. This is done by the SQLFreeStmt with the SQL_CLOSE
		** option. This is done here (instead of within the ExecuteSQL function, or even
		** after the call to ExecuteSQL, because we may want to access the previous hstmt
		** results within, for example, DumpColumnInfo
		*/
		SQLFreeStmt(hstmt, SQL_UNBIND);
		SQLFreeStmt(hstmt, SQL_CLOSE);

		rc = ExecuteSQL(hstmt, sqlStatement, timeFile);

		if (!SQL_SUCCEEDED(rc))
		{
			ODBC_error(SQL_HANDLE_STMT, hstmt);
			SQLFreeStmt(hstmt, SQL_CLOSE);
		}
	}

	SQLFreeStmt(hstmt, SQL_DROP);

	EnvClose(henv, hdbc);

	if (timeFile) {
		fclose(timeFile);
	}

	exitCode = 0;

errRet:
	free(dsnSqlBuf);

	return exitCode;
}

void PrintHelp()
{
	_tprintf(_T("\n"));
	_tprintf(_T("Built-in Example commands:\n"));
	_tprintf(_T("\n"));
	_tprintf(_T("  hexdump       : Toggle hex dump output for character data.\n"));
	_tprintf(_T("  run <file>    : Run the SQL Statements contained in <file>.\n"));
	_tprintf(_T("                  Each line of the file is executed as a complete statement.\n"));
	_tprintf(_T("  dbinfo        : Print database info.\n"));
	_tprintf(_T("  columninfo    : Print column information about current results.\n"));
	_tprintf(_T("  driverinfo    : Print driver info.\n"));
	_tprintf(_T("  demoinsert    : Demo parameter array insert.\n"));
	_tprintf(_T("  help          : Display this help menu.\n"));
	_tprintf(_T("  quit          : Exit this application.\n"));
	_tprintf(_T("\n"));
	_tprintf(_T("  SQL>          : (default) Execute a SQL statement or any command listed above.\n"));
	_tprintf(_T("\nBesides prompting you to enter DSN, UID and PWD, you can also pass them or connect string and/or sql as command line arguments.\n"));
	_tprintf(_T("Below are some examples of the usage:\n"));
	_tprintf(_T("	example\n"));
	_tprintf(_T("	example -t[timedResults.csv] -tMyOutputFile.txt ...\n"));
	_tprintf(_T("	example -o3	for ODBC3 behavior (default), can be -o2 -o380 ...\n"));
	_tprintf(_T("	example \"DSN\" \"UID\" \"PWD\" ...\n"));
	_tprintf(_T("	example \"DSN\" \"UID\" \"PWD\" demoinsert\n"));
	_tprintf(_T("	example \"DSN\" \"UID\" \"PWD\" \"Sql1\" \"Sql2\" ...\n"));
	_tprintf(_T("	example \"DSN\" \"UID\" \"PWD\" driverinfo dbinfo ... quit\n"));
	_tprintf(_T("	example \"ConnectString start with DSN= or Driver=\" demoinsert\n"));
	_tprintf(_T("	example \"ConnectString start with DSN= or Driver=\" \"Sql1\" \"Sql2\" ...\n"));
	_tprintf(_T("	example \"ConnectString start with DSN= or Driver=\" driverinfo dbinfo ... quit\n"));
	_tprintf(_T("\n"));
}

/*
** function: BindColumns
**
** Purpose : Binds the buffers to the proper columns specified by the column no
**
** Arguments:	SQLHSTMT hstmt  _ The statement handle
**		ColInfoStruct *colInfo - Pointer to the ColInfoStruct
**		DataInfoStruct *colInfo - Pointer to the DataInfoStruct
**		SQLSMALLINT icol - The current column number
**
** Returns:	SQLRETURN - Return status from last ODBC Function.
**
*/
SQLRETURN BindColumns(SQLHSTMT hstmt, ColInfoStruct* colInfo,
	DataInfoStruct* dataInfo, SQLUSMALLINT icol)
{
	SQLSMALLINT fCType;
	SQLRETURN rc;
	SQLLEN* colDisplaySize = &dataInfo->length;

	fCType = SQL_C_TCHAR;

	rc = SQLBindCol(hstmt,
			(icol + 1),
			fCType,
			&(dataInfo->charCol[0]),
			COL_VALUE_LEN,
			colDisplaySize);
	/*
	** Get the column display size by using the SQLColAttribute funtion
	*/
	if (SQL_SUCCEEDED(rc))
	{
		rc = SQLColAttribute(hstmt,
				(icol + 1),
				SQL_DESC_DISPLAY_SIZE,
				NULL, 0, NULL,
				colDisplaySize);
		if (*colDisplaySize > MAX_DISPLAY_SIZE)
		{
			*colDisplaySize = MAX_DISPLAY_SIZE;
		}

	}
	return rc;
}

/*
** function: DisplayResults
**
** Purpose : Displays the results returned by SQLFetch
**
** Arguments:	SQLHSTMT hstmt		- The statement handle
**				SQLSMALLINT hdbc    - The total no of columns
**
** Returns:	SQLRETURN - Return status from last ODBC Function.
**
*/
SQLRETURN DisplayResults(SQLHSTMT hstmt, SQLSMALLINT noOfColumns)
{
	SQLRETURN rc;
	SQLULEN rows = 0;

	_tprintf(_T("\n"));

	while (TRUE)
	{
		int index = 0;
		rc = SQLFetch(hstmt);
		if (!SQL_SUCCEEDED(rc))
		{
			break;
		}
		while (index < noOfColumns)
		{
			if (dataStruct[index].length == SQL_NULL_DATA)
			{
				dataStruct[index].length =
					infoStruct[index].cbColName +
					COLUMN_SEPERATOR_OFFSET;
#if defined(_WIN32) || defined(_WIN64)
				_tcscpy_s((TCHAR*)dataStruct[index].charCol, sizeof(dataStruct[index].charCol) / sizeof(dataStruct[index].charCol[0]), _T("IS_NULL"));
#else
				strcpy((TCHAR*)dataStruct[index].charCol, "IS_NULL");
#endif
			}
			else if (dataStruct[index].charCol[0] == '\0' ||
						dataStruct[index].charCol[0] == ' ')
			{
				dataStruct[index].length =
					infoStruct[index].cbColName +
					COLUMN_SEPERATOR_OFFSET;
#if defined(_WIN32) || defined(_WIN64)
				_tcscpy_s((TCHAR*)dataStruct[index].charCol, sizeof(dataStruct[index].charCol) / sizeof(dataStruct[index].charCol[0]), _T("IS_EMPTY"));
#else
				strcpy((TCHAR*)dataStruct[index].charCol, "IS_EMPTY");
#endif
			}
			/*
			** Check the display length of the value. If it is lesser than the column name
			** length added to the column seperator length, then display the value left
			** aligned by the proper length.
			*/
			if (dataStruct[index].length <
				(infoStruct[index].cbColName +
					COLUMN_SEPERATOR_OFFSET))
			{
				_tprintf(_T("%-*s"), (infoStruct[index].cbColName
					+ COLUMN_SEPERATOR_OFFSET),
					dataStruct[index].charCol);
			}
			else
			{
				_tprintf(_T("%-*s"), (unsigned int)dataStruct[index].length,
					dataStruct[index].charCol);
			}

			if (g_fHexDump)
			{
				HexDump((TCHAR*)dataStruct[index].charCol, dataStruct[index].length);
			}

			index++;
		}
		_tprintf(_T("\n"));
		rows++;
	}

	_tprintf(_T("\n"));

	_tprintf(_T("%ld rows and %d cols fetched. rc=%d\n"), (long)rows, noOfColumns, rc);

	if (rc == SQL_NO_DATA_FOUND)
	{
		rc = SQL_SUCCESS;
	}
	return rc;
}

/*
** function:	GetColumnInfo
**
** Arguments:	(SQLHSTMT)hstmt	- statement handle
**
*/
SQLRETURN GetColumnInfo(SQLHSTMT hstmt, SQLSMALLINT *numCols, ColInfoStruct *pColumnInfo, int maxCols)
{
	SQLRETURN rc = SQL_SUCCESS;
	int i = 0;
	rc = SQLNumResultCols(hstmt, numCols);
	for (i = 0; SQL_SUCCEEDED(rc) && i < *numCols && i < maxCols; i++)
	{
		rc = SQLDescribeCol(hstmt, i + 1,
				pColumnInfo[i].szColName,
				COL_NAME_LEN,
				&pColumnInfo[i].cbColName,
				&pColumnInfo[i].fSqlType,
				&pColumnInfo[i].cbColDef,
				&pColumnInfo[i].ibScale,
				&pColumnInfo[i].fNullable);
	}
	return rc;
}

/*
** function:	DumpColumnInfo
**
** Arguments:	(SQLHSTMT)hstmt	- statement handle
**
*/
SQLRETURN DumpColumnInfo(SQLHSTMT hstmt)
{
	SQLRETURN rc = SQL_SUCCESS;
	SQLSMALLINT wNumCols = 0;
	int i = 0;

	rc = GetColumnInfo(hstmt, &wNumCols, infoStruct, (int)(sizeof(infoStruct)/sizeof(infoStruct[0])));

	if (!SQL_SUCCEEDED(rc)) {
		return rc;
	}

	for (i = 0; i < wNumCols; i++) {
		if (SQL_SUCCEEDED(rc)) {
			_tprintf(_T("ColumnNumber  : %d\n"), i + 1);
			_tprintf(_T("ColumnName    : %s\n"), infoStruct[i].szColName);
			_tprintf(_T("NameLength    : %d\n"), infoStruct[i].cbColName);
			_tprintf(_T("DataType      : %d\n"), infoStruct[i].fSqlType);
			_tprintf(_T("ColumnSize    : %lu\n"), (unsigned long)(infoStruct[i].cbColDef));
			_tprintf(_T("DecimalDigits : %d\n"), infoStruct[i].ibScale);
			_tprintf(_T("Nullable      : %d\n"), infoStruct[i].fNullable);
			_tprintf(_T("\n"));
		}
	}

	return rc;
}

/*
** function:	ExecuteFile
**
** arguments:	(SQLHSTMT)hstmt		- statement handle
**				(UCHAR)szFileName	- path to sql file
**
*/
SQLRETURN ExecuteFile(SQLHSTMT hstmt, const TCHAR* szFileName)
{
	SQLRETURN rc = SQL_SUCCESS;
	FILE* pfile = NULL;

	if (szFileName == NULL) {
		return SQL_SUCCESS;
	}

#if defined(_WIN32) || defined(_WIN64)
	if (0 != _tfopen_s(&pfile, (const TCHAR*)szFileName, _T("r"))) pfile = NULL;
#else
	pfile = fopen((const TCHAR*)szFileName, _T("r"));
#endif

	if (pfile) {
		SQLTCHAR line[SQL_STATEMENT_LEN];

		while (_fgetts((TCHAR*)line, SQL_STATEMENT_LEN, pfile) != NULL) {
			size_t lastCharPos = _tcslen((const TCHAR*)line) - 1;
			if (line[lastCharPos] == '\n') {
				line[lastCharPos] = '\0';
			}
			_tprintf(_T("SQL> %s\n"), line);
			rc = ExecuteSQL(hstmt, line, NULL);

			if (!SQL_SUCCEEDED(rc)) {
				return rc;
			}

			SQLFreeStmt(hstmt, SQL_UNBIND);
			SQLFreeStmt(hstmt, SQL_CLOSE);
		}

		return rc;
	}

	return SQL_SUCCESS;
}

/*
** function:	PrintDatabaseInfo
**
** arguments:	(SQLHDBC)hdbc	- statement handle
**
*/
SQLRETURN PrintDatabaseInfo(SQLHDBC hdbc)
{
	SQLRETURN rc = SQL_SUCCESS;
	TCHAR sz[INFO_BUFFER_LEN];
	SQLSMALLINT cb;

	rc = SQLGetInfo(hdbc, SQL_DATA_SOURCE_NAME, (SQLPOINTER)sz, INFO_BUFFER_LEN, &cb);
	if (SQL_SUCCEEDED(rc)) _tprintf(_T("Data source name : %s\n"), sz);

	rc = SQLGetInfo(hdbc, SQL_DATABASE_NAME, (SQLPOINTER)sz, INFO_BUFFER_LEN, &cb);
	if (SQL_SUCCEEDED(rc)) _tprintf(_T("Database name : %s\n"), sz);

	rc = SQLGetInfo(hdbc, SQL_DBMS_NAME, (SQLPOINTER)sz, INFO_BUFFER_LEN, &cb);
	if (SQL_SUCCEEDED(rc)) _tprintf(_T("DBMS name : %s\n"), sz);

	rc = SQLGetInfo(hdbc, SQL_DBMS_VER, (SQLPOINTER)sz, INFO_BUFFER_LEN, &cb);
	if (SQL_SUCCEEDED(rc)) _tprintf(_T("DBMS Version : %s\n"), sz);

	rc = SQLGetInfo(hdbc, SQL_SERVER_NAME, (SQLPOINTER)sz, INFO_BUFFER_LEN, &cb);
	if (SQL_SUCCEEDED(rc)) _tprintf(_T("Server name : %s\n"), sz);

	rc = SQLGetInfo(hdbc, SQL_USER_NAME, (SQLPOINTER)sz, INFO_BUFFER_LEN, &cb);
	if (SQL_SUCCEEDED(rc)) _tprintf(_T("User name : %s\n"), sz);

	rc = SQLGetInfo(hdbc, SQL_KEYWORDS, (SQLPOINTER)sz, INFO_BUFFER_LEN, &cb);
	if (SQL_SUCCEEDED(rc)) _tprintf(_T("Keywords : %s\n"), sz);

	rc = SQLGetInfo(hdbc, SQL_IDENTIFIER_QUOTE_CHAR, (SQLPOINTER)sz, INFO_BUFFER_LEN, &cb);
	if (SQL_SUCCEEDED(rc)) _tprintf(_T("Quote TCHAR : %s\n"), sz);

	return rc;
}

/*
** function:	PrintDatabaseInfo
**
** arguments:	(SQLHDBC)hdbc	- statement handle
**
*/
SQLRETURN PrintDriverInfo(SQLHDBC hdbc)
{
	SQLRETURN rc = SQL_SUCCESS;
	TCHAR sz[INFO_BUFFER_LEN];
	SQLSMALLINT cb;

	rc = SQLGetInfo(hdbc, SQL_DRIVER_NAME, (SQLPOINTER)sz, INFO_BUFFER_LEN, &cb);
	if (SQL_SUCCEEDED(rc)) _tprintf(_T("Driver name : %s\n"), sz);

	rc = SQLGetInfo(hdbc, SQL_DRIVER_VER, (SQLPOINTER)sz, INFO_BUFFER_LEN, &cb);
	if (SQL_SUCCEEDED(rc)) _tprintf(_T("Driver version : %s\n"), sz);

	rc = SQLGetInfo(hdbc, SQL_DRIVER_ODBC_VER, (SQLPOINTER)sz, INFO_BUFFER_LEN, &cb);
	if (SQL_SUCCEEDED(rc)) _tprintf(_T("ODBC version driver supports : %s\n"), sz);

	rc = SQLGetInfo(hdbc, SQL_ODBC_VER, (SQLPOINTER)sz, INFO_BUFFER_LEN, &cb);
	if (SQL_SUCCEEDED(rc)) _tprintf(_T("Driver manager ODBC version : %s\n"), sz);

	rc = SQLGetInfo(hdbc, SQL_DM_VER, (SQLPOINTER)sz, INFO_BUFFER_LEN, &cb);
	if (SQL_SUCCEEDED(rc)) _tprintf(_T("Driver Manager version : %s\n"), sz);

	return rc;
}

/*
** function:	ExecuteSQL
**
** arguments:	(SQLHSTMT)hstmt			- statement handle
**				(SQLTCHAR*)strSQL		- sql to execute
**				(FILE*)timeFile			- file for capturing timing data
**
*/
SQLRETURN ExecuteSQL(SQLHSTMT hstmt, SQLTCHAR* strSQL, FILE* timeFile)
{
	SQLRETURN rc;
	uint64_t startTime, stopTime, executeTime;
	uint64_t runtime;
	struct timeb timebuffer;
	SQLSMALLINT  numCols = 0;
	SQLUSMALLINT icol;

	/*
	** Since the SQL statements would be typed online, here SQLPrepare is
	** not required. So we can directly use the SQLExecDirect function
	*/
	ftime(&timebuffer);
	startTime = timebuffer.time * 1000 + timebuffer.millitm;
	rc = SQLExecDirect(hstmt, strSQL, SQL_NTS);
	ftime(&timebuffer);
	executeTime = timebuffer.time * 1000 + timebuffer.millitm;
	runtime = executeTime - startTime;
	_tprintf(_T("milliseconds elapsed for execute: %llu\n"), runtime);

	if (!SQL_SUCCEEDED(rc))
	{
		return rc;
	}

	if (timeFile) {
		_ftprintf(timeFile, _T("\"%s\", %llu"), strSQL, runtime);
		fflush(timeFile);
	}

	/*
	** We use SQLNumResultCols to return the number of columns in the
	** result set. If the number of result columns is 0 then it can be safely
	** assumed that the statement entered by the user was not a select statement
	*/
	numCols = 0;
	rc = GetColumnInfo(hstmt, &numCols, infoStruct, (int)(sizeof(infoStruct) / sizeof(infoStruct[0])));

	if (!SQL_SUCCEEDED(rc))
	{
		return rc;
	}

	/* Not a result set */
	if (numCols == 0) {
		return rc;
	}

	/* Look for a statement with too many result columns. */
	if (numCols > MAX_COLUMNS) {
		_tprintf(MAX_COL_ERROR, MAX_COLUMNS);
		SQLFreeStmt(hstmt, SQL_CLOSE);
		return rc;
	}

	_tprintf(_T("\n"));

	for (icol = 0; icol < numCols; icol++)
	{
		rc = BindColumns(hstmt,
				&infoStruct[icol],
				&dataStruct[icol],
				icol);
		if (!SQL_SUCCEEDED(rc))
		{
			return rc;
		}
		_tprintf(_T("%-*s"),
			(infoStruct[icol].cbColName + COLUMN_SEPERATOR_OFFSET),
			infoStruct[icol].szColName);
	}

	rc = DisplayResults(hstmt, numCols);

	ftime(&timebuffer);
	stopTime = timebuffer.time * 1000 + timebuffer.millitm;
	_tprintf(_T("milliseconds elapsed: %llu\n"), stopTime - startTime);
	if (timeFile) {
		_ftprintf(timeFile, _T(", %llu\n"), stopTime - startTime);
		fflush(timeFile);
	}

	return rc;
}

/*
** function:	HexDump
**
** arguments:	(void*)addr				- data to hex dump
**				(SQLLEN)len				- length of data
**
*/
void HexDump(void* addr, SQLLEN len)
{
	SQLLEN indexNo;
	TCHAR buff[17];
	TCHAR* pc = (TCHAR*)addr;

	if (len == 0) {
		_tprintf(_T("  ZERO LENGTH\n"));
		return;
	}
	if (len < 0) {
		_tprintf(_T("  NEGATIVE LENGTH: %i\n"), (int)len);
		return;
	}

	/* Process every byte in the data. */
	for (indexNo = 0; indexNo < len; indexNo++) {
		SQLLEN colNo = indexNo & 0xF;

		/* 
		** Multiple of 16 means new line (with line offset).
		** Just don't print ASCII for the zeroth line.
		*/
		if (colNo == 0 && indexNo != 0) {
			_tprintf(_T("  %s\n"), buff);
		}

		/*  Now the hex code for the specific character. */
		_tprintf(_T(" %02x"), pc[indexNo]);

		/* And store a printable ASCII character for later. */
		if ((pc[indexNo] < 0x20) || (pc[indexNo] > 0x7e))
		{
			buff[colNo] = '.';
		}
		else
		{
			buff[colNo] = pc[indexNo];
		}

		buff[colNo + 1] = '\0';
	}

	/* Pad out last line if not exactly 16 characters. */
	while ((indexNo++ % 16) != 0) {
		_tprintf(_T("   "));
	}

	_tprintf(_T("\n"));
}

/* demoInsert
**	Demonstrate the use of parameter array for inserting multiple (should be at least 2) rows
**	Optionally specify -o2 (for ODBC2 vs. ODBC3 behavior) or other connect options such as EnableBulkLoad
**	  at the command line to see how different drivers handle
**		parameter array insert, and
**		values longer than the column size for character types
*/
SQLRETURN demoInsert(SQLHSTMT hstmt, FILE* timeFile)
{
	SQLRETURN rc;
	uint64_t startTime, stopTime;
	struct timeb timebuffer;

	/* Values and length indicators for integer column 1 */
	SQLINTEGER intNum[] = { 1, 2, 3, 4, 5 };
	SQLLEN intLenArr[] = { 0, 0, SQL_NULL_DATA, 0, 0 };

	/* Values and length indicators for varchar column 2 */
	SQLTCHAR	charValue[][20] = { _T("r1c2_v"), _T("ro2co2_v"), _T("row3col2_v   "), _T("row_4_col_2_long"), _T("row_5col_5") };
	SQLLEN charLenArr[] = { SQL_NTS, 6 * sizeof(SQLTCHAR), SQL_NTS, SQL_NTS, SQL_NULL_DATA};

	/* Parameter array insert row status and results */
	SQLUSMALLINT rowStatus[] = { 251, 252, 253, 254, 255 }; /* some initial, invalid values */
	SQLULEN rows_processed = 0;
	SQLLEN rowInserted = -1;

	/* run 2 times */
	SQLULEN run, runs[] = { 2, (sizeof(intNum) / sizeof(intNum[0])) }; /* insert first 2 rows and then all rows (the same 2 rows again plus more rows) */

	ColInfoStruct columnInfo[2];
	SQLSMALLINT numCols = 0;

	SQLTCHAR* sql, * str;
	int i;
	BOOL bNeedCleanup;

	/* get ready for insert operation */
	SQLFreeStmt(hstmt, SQL_UNBIND);
	SQLFreeStmt(hstmt, SQL_CLOSE);

	ftime(&timebuffer);
	startTime = timebuffer.time * 1000 + timebuffer.millitm;

	_tprintf(_T("\n-------------------- Demo Parameter Array Insert Operation:\n"));

	/* create a table; this may fail if the table already exists */
	sql = (SQLTCHAR*)_T("create table tmpDemoIntVC10Tbl (intcol int, varcharcol varchar(10))");
	rc = SQLExecDirect(hstmt, sql, SQL_NTS);
	_tprintf(_T("%s	rc=%d\n"), sql, rc);
	if (!(bNeedCleanup = (SQL_SUCCEEDED(rc))))
	{
		ODBC_error(SQL_HANDLE_STMT, hstmt);
	}

	/*
	sql = (SQLTCHAR*)"delete from tmpDemoIntVC10Tbl";
	rc = SQLExecDirect(hstmt, sql, SQL_NTS);
	_tprintf(_T("%s	rc=%d\n"), sql, rc);
	if (!SQL_SUCCEEDED(rc))
	{
		ODBC_error(SQL_HANDLE_STMT, hstmt);
	}
	*/

	/* get the column size and sql type for SQLBindParameter */
	/*
	sql = (SQLTCHAR*)"select * from tmpDemoIntVC10Tbl where 1=2";
	rc = SQLExecDirect(hstmt, sql, SQL_NTS);
	_tprintf(_T("Get the column size and sql type for SQLBindParameter to use: \"%s\"	rc=%d\n"), sql, rc);
	if (!SQL_SUCCEEDED(rc))
	{
		ODBC_error(SQL_HANDLE_STMT, hstmt);
	}
	rc = GetColumnInfo(hstmt, &numCols, columnInfo, (int)(sizeof(columnInfo) / sizeof(columnInfo[0])));
	if (!SQL_SUCCEEDED(rc))
	{
		ODBC_error(SQL_HANDLE_STMT, hstmt);
		columnInfo[0].fSqlType = SQL_INTEGER; columnInfo[0].cbColDef = 10;
		columnInfo[1].fSqlType = SQL_VARCHAR; columnInfo[1].cbColDef = 10;
	}
	SQLFreeStmt(hstmt, SQL_UNBIND);
	SQLFreeStmt(hstmt, SQL_CLOSE);
	*/

	sql = (SQLTCHAR*)_T("insert into tmpDemoIntVC10Tbl values (?,?)");
	rc = SQLPrepare(hstmt, sql, SQL_NTS);
	_tprintf(_T("SQLPrepare: %s	rc=%d\n"), sql, rc);
	if (rc != SQL_SUCCESS)
	{
		ODBC_error(SQL_HANDLE_STMT, hstmt);
		return rc;
	}

	str = (SQLTCHAR*)_T("Set SQL_ATTR_PARAM_STATUS_PTR to rowStatus");
	rc = SQLSetStmtAttr(hstmt, SQL_ATTR_PARAM_STATUS_PTR, rowStatus, SQL_IS_POINTER);
	_tprintf(_T("%s	rc=%d\n"), str, rc);
	if (rc != SQL_SUCCESS)
	{
		ODBC_error(SQL_HANDLE_STMT, hstmt);
		return rc;
	}

	str = (SQLTCHAR*)_T("Set SQL_ATTR_PARAMS_PROCESSED_PTR to rows_processed");
	rc = SQLSetStmtAttr(hstmt, SQL_ATTR_PARAMS_PROCESSED_PTR, &rows_processed, 0);
	_tprintf(_T("%s	rc=%d\n"), str, rc);
	if (rc != SQL_SUCCESS)
	{
		ODBC_error(SQL_HANDLE_STMT, hstmt);
		return rc;
	}

	_tprintf(_T("SQLDescribeParam for parameter 1:\n"));
	rc = SQLDescribeParam(hstmt, 1, &columnInfo[0].fSqlType, &columnInfo[0].cbColDef, &columnInfo[0].ibScale, &columnInfo[0].fNullable);
	if (rc != SQL_SUCCESS)
	{
		ODBC_error(SQL_HANDLE_STMT, hstmt);
		columnInfo[0].fSqlType = SQL_INTEGER; columnInfo[0].cbColDef = 10;
	}
	rc = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, columnInfo[0].fSqlType, columnInfo[0].cbColDef, 0, intNum, 0, intLenArr);
	_tprintf(_T("SQLBindParameter for column 1: intCol binding C type=%d columnSize=%ld	rc=%d\n"), SQL_C_LONG, (long)columnInfo[0].cbColDef, rc);
	if (rc != SQL_SUCCESS)
	{
		ODBC_error(SQL_HANDLE_STMT, hstmt);
		return rc;
	}

	_tprintf(_T("column 1 values (length indicators): "));
	for (i = 0; i < (int)(sizeof(intNum) / sizeof(intNum[0])); i++)
	{
		_tprintf(_T("%s%d"), (i? _T(", ") : _T("")), intNum[i]);
	}
	_tprintf(_T("	("));
	for (i = 0; i < (int)(sizeof(intLenArr) / sizeof(intLenArr[0])); i++)
	{
		_tprintf(_T("%s%ld"), (i ? _T(", ") : _T("")), (long)intLenArr[i]);
	}
	_tprintf(_T(")\n"));

	_tprintf(_T("SQLDescribeParam for parameter 2:\n"));
	rc = SQLDescribeParam(hstmt, 2, &columnInfo[1].fSqlType, &columnInfo[1].cbColDef, &columnInfo[1].ibScale, &columnInfo[1].fNullable);
	if (rc != SQL_SUCCESS)
	{
		columnInfo[1].fSqlType = SQL_VARCHAR; columnInfo[1].cbColDef = 10;
	}
	rc = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_TCHAR, columnInfo[1].fSqlType, columnInfo[1].cbColDef, 0, charValue, sizeof(charValue[0]), charLenArr);
	_tprintf(_T("SQLBindParameter for column 2: varcharCol binding C type=%d columnSize=%ld	rc=%d\n"), SQL_C_TCHAR, (long)columnInfo[1].cbColDef, rc);
	if (rc != SQL_SUCCESS)
	{
		ODBC_error(SQL_HANDLE_STMT, hstmt);
		return rc;
	}

	_tprintf(_T("The number in () following each string value is the actual string length in bytes for display only, which is not part of the value\n"));
	_tprintf(_T("column 2 values (length indicators): "));
	for (i = 0; i < (int)(sizeof(charValue) / sizeof(charValue[0])); i++)
	{
		_tprintf(_T("%s\"%s\"(%ld)"), (i ? _T(", ") : _T("")), charValue[i], (long)(_tcslen((const TCHAR*)charValue[i]) * sizeof(TCHAR)));
	}
	_tprintf(_T("	("));
	for (i = 0; i < (int)(sizeof(charLenArr) / sizeof(charLenArr[0])); i++)
	{
		_tprintf(_T("%s%ld"), (i ? _T(", ") : _T("")), (long)charLenArr[i]);
	}
	_tprintf(_T(")\n"));

	/* insert first 2 rows and then all rows (the same 2 rows again plus more rows) */
	for (run = 0; run < sizeof(runs) / sizeof(runs[0]); run++)
	{
		str = (SQLTCHAR*)_T("Set SQL_ATTR_PARAMSET_SIZE to");
		rc = SQLSetStmtAttr(hstmt, SQL_ATTR_PARAMSET_SIZE, (void*)runs[run], SQL_IS_UINTEGER);
		_tprintf(_T("Run %ld of %ld: %s %ld	rc=%d\n"), (long)run+1, (long)(sizeof(runs) / sizeof(runs[0])), str, (long)runs[run], rc);
		if (rc != SQL_SUCCESS)
		{
			ODBC_error(SQL_HANDLE_STMT, hstmt);
			return rc;
		}

		str = (SQLTCHAR*)_T("SQLExecute the parameter array insert...");
		rc = SQLExecute(hstmt);
		_tprintf(_T("%s	rc=%d\n"), str, rc);
		if (rc != SQL_SUCCESS)
		{
			ODBC_error(SQL_HANDLE_STMT, hstmt);
		}

		_tprintf(_T("rows_processed=%ld\n"), (long)rows_processed);
		_tprintf(_T("Parameter Set  Row Status\n"));
		_tprintf(_T("-------------  -------------\n"));
		for (i = 1; i <= (int)rows_processed; i++)
		{
			_tprintf(_T("%13d  "), i);
			switch (rowStatus[i - 1])
			{
			case SQL_PARAM_SUCCESS:
				_tprintf(_T("SQL_PARAM_SUCCESS"));
				break;

			case SQL_PARAM_SUCCESS_WITH_INFO:
				_tprintf(_T("SQL_PARAM_SUCCESS_WITH_INFO"));
				break;

			case SQL_PARAM_ERROR:
				_tprintf(_T("SQL_PARAM_ERROR"));
				break;

			case SQL_PARAM_UNUSED:
				_tprintf(_T("SQL_PARAM_UNUSED"));
				break;

			case SQL_PARAM_DIAG_UNAVAILABLE:
				_tprintf(_T("SQL_PARAM_DIAG_UNAVAILABLE"));
				break;

			default:
				_tprintf(_T("Default unknown"));
				break;
			}
			_tprintf(_T(" (%u)\n"), rowStatus[i - 1]);
		}

		if (SQL_SUCCEEDED(rc))
		{
			rc = SQLRowCount(hstmt, &rowInserted);
			if (rc != SQL_SUCCESS)
			{
				ODBC_error(SQL_HANDLE_STMT, hstmt);
			}
			_tprintf(_T("Rows inserted: %ld\n"), (long)rowInserted);
		}
	}

	SQLFreeStmt(hstmt, SQL_RESET_PARAMS);

	ftime(&timebuffer);
	stopTime = timebuffer.time * 1000 + timebuffer.millitm;
	_tprintf(_T("demoInsert milliseconds elapsed: %llu\n\n"), stopTime - startTime);

	if (timeFile)
	{
		_ftprintf(timeFile, _T("\"insert into tmpDemoIntVC10Tbl values (?,?) for 5 rows\",%llu\n"), stopTime - startTime);
		fflush(timeFile);
	}

	str = (SQLTCHAR*)_T("Set SQL_ATTR_PARAMSET_SIZE back to default 1");
	rc = SQLSetStmtAttr(hstmt, SQL_ATTR_PARAMSET_SIZE, (void*)(size_t)1, SQL_IS_UINTEGER);
	_tprintf(_T("%s	rc=%d\n"), str, rc);
	if (rc != SQL_SUCCESS)
	{
		ODBC_error(SQL_HANDLE_STMT, hstmt);
	}

	_tprintf(_T("Fetch all rows in the table. Should return at least 2 rows, up to 7 rows if the table created internally.\n"));
	sql = (SQLTCHAR*)_T("select * from tmpDemoIntVC10Tbl");
	_tprintf(_T("%s\n"), sql);
	rc = ExecuteSQL(hstmt, sql, timeFile);
	_tprintf(_T("ExecuteSQL returns rc=%d\n"), rc);

	/* show the result set metadata */
	_tprintf(_T("\nThe resultset metadata:\n"));
	if (SQL_SUCCEEDED(rc))
	{
		rc = DumpColumnInfo(hstmt);
	}

	/* cleanup */
	if (bNeedCleanup)
	{
		/* show the last error before dropping the table */
		if (rc != SQL_SUCCESS)
		{
			ODBC_error(SQL_HANDLE_STMT, hstmt);
		}
		SQLFreeStmt(hstmt, SQL_UNBIND);
		SQLFreeStmt(hstmt, SQL_CLOSE);
		sql = (SQLTCHAR*)_T("drop table tmpDemoIntVC10Tbl");
		rc = SQLExecDirect(hstmt, sql, SQL_NTS);
		_tprintf(_T("%s	rc=%d\n"), sql, rc);
	}

	/* The caller will show the last error if any */
	return rc;
}
