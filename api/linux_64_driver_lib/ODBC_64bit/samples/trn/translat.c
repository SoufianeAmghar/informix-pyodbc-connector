/*--------------------------------------------------------------+
|  Purpose:							|
|	Translate text strings between OEM and ANSI.		|
|	Convert to OEM for text passed to the database.		|
|	Convert to ANSI for text returned from the database.	|
|                                                               |
|  Author: John Hobson   	 	Date: April 1994	|
|                                                               |
|  Copyright: 1992-2010 Progress Software Corporation		|
|                                                               |
|       This software contains confidential and proprietary     |
|       information of Progress Software Corp.    		|
+--------------------------------------------------------------*/

/*----------------+
|  Include files  |
+----------------*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>

#if defined(__unix) || defined(QE_MAC) || defined(QE_MAC_PPC)
#if !defined (__STDC__) && !defined (__cplusplus)
#define const
#define signed
#endif

#include <sqlunx.h>
#define DLLOVSPROC
#define TRUE 1
#define FALSE 0
#define	BOOL	int
#define HWND	int
#define	DWORD	unsigned long
#define FAR
#define far
#define lstrlen strlen
#define	lstrcpyn strncpy
/*typedef	int (* FARPROC)();*/
#if defined(__aix) || defined(__unix)
#define TYPEPARAM1(x) ((char *) (x))
#define TYPEPARAM2(x) ((char *) (x))
#ifdef __cplusplus
#define	NULL	0
#endif
#else
#define TYPEPARAM1(x) (x)
#define TYPEPARAM2(x) (x)
#endif
#endif

#if defined (QE_MSDOS)
#include "windows.h"
#if defined (QE_WIN32)
#define DLLOVSPROC __stdcall
#define TYPEPARAM1(x) ((const char *)x)
#define TYPEPARAM2(x) ((char *)x)
#else
#define DLLOVSPROC __loadds __export PASCAL FAR
#define TYPEPARAM1(x) (x)
#define TYPEPARAM2(x) (x)
#endif
#endif

#if defined (QE_OS2)
#include <os2.h>
#include <qeodbc.h>
#define TYPEPARAM1(x) ((const char *) (x))
#define TYPEPARAM2(x) ((char *) (x))
#define DLLOVSPROC _Export _System
#define	DWORD	unsigned long
#define lstrlen strlen
#define	lstrcpyn strncpy
#endif

/*  Include the ODBC header files */
#ifdef __cplusplus
extern "C" {
#endif
#include "sql.h"
#include "sqlext.h"
#ifdef __cplusplus
/* prototypes required if compiling with C++ */
BOOL DLLOVSPROC
        ConfigTranslator (HWND hwndParent, DWORD FAR * pvOption);
BOOL DLLOVSPROC SQLDataSourceToDriver (
        UDWORD          fOption,
        SWORD           fSqlType,
        PTR             rgbValueIn,
        SDWORD          cbValueIn,
        PTR             rgbValueOut,
        SDWORD          cbValueOutMax,
        SDWORD  FAR *   pcbValueOut,
        UCHAR   FAR *   szErrorMsg,
        SWORD           cbErrorMsgMax,
        SWORD   FAR *   pcbErrorMsg
);
BOOL DLLOVSPROC SQLDriverToDataSource (
        UDWORD          fOption,
        SWORD           fSqlType,
        PTR             rgbValueIn,
        SDWORD          cbValueIn,
        PTR             rgbValueOut,
        SDWORD          cbValueOutMax,
        SDWORD  FAR *   pcbValueOut,
        UCHAR   FAR *   szErrorMsg,
        SWORD           cbErrorMsgMax,
        SWORD   FAR *   pcbErrorMsg
);
/* end of extern C prototypes, now do C++ prototypes */
}
#endif

#if defined (QE_WIN32)
WINUSERAPI BOOL WINAPI AnsiToOem (const char * temp, char * out);
WINUSERAPI BOOL WINAPI OemToAnsi (const char * temp, char * out);
#else
#ifndef QE_MSDOS
void AnsiToOem (const char * temp, char * out);
void OemToAnsi (const char * temp, char * out);
#endif
#endif

/*----------------------------------------------------------------------+
| ConfigTranslator							|
| Purpose:	Provide data source to driver conversions.		|
|		See Chapter 29 of the ODBC Programmer's reference V2.0	|
+----------------------------------------------------------------------*/

extern BOOL DLLOVSPROC
ConfigTranslator (
	HWND		hwndParent,	/* Parent window, for dialog	*/
	DWORD FAR  *	pvOption)	/* Translation option		*/
{

/* We don't have any options, so return zero.	*/

	*pvOption = 0;
	return TRUE;
}

void writeString (
	UCHAR	FAR *	szErrorMsg,	/* Pointer to error message storage  */
	SWORD		cbErrorMsgMax,	/* Space available for error message */
	SWORD	FAR *	pcbErrorMsg,	/* Length of error message           */
	UCHAR	FAR *	string)		/* String to add                     */
{
	short	currLen;
	short len = lstrlen ((const char *) string);


/* If we don't have an error msg string, return		*/

	if (szErrorMsg == NULL) return;

/* Determine current length of error msg		*/

	currLen = (pcbErrorMsg != NULL) ? *pcbErrorMsg :
				lstrlen ((const char *) szErrorMsg);

/* Add new string onto the end				*/

	lstrcpyn ((char *) (szErrorMsg + currLen), (const char *) string,
		(size_t) (cbErrorMsgMax - currLen));

/* Update error message length				*/

	if (pcbErrorMsg != NULL)
		if (len > (cbErrorMsgMax - currLen))
			*pcbErrorMsg = cbErrorMsgMax;
		else
			*pcbErrorMsg += len;
	return;
}

/*----------------------------------------------------------------------+
| SQLDataSourceToDriver							|
| Purpose:	Provide data source to driver conversions.		|
|		See Chapter 29 of the ODBC Programmer's reference V2.	|
| Parameters:								|
|		fOption		- Option value				|
|		fSqlType	- SQL data type				|
|		rgbValueIn	- Value to translate			|
|		cbValueIn	- Length of value to translate		|
|		rgbValueOut	- Translated value			|
|		cbValueOutMax	- Space available for translated value	|
|		pcbValueOut	- Length of translated value		|
|		szErrorMsg	- Pointer to error message storage	|
|		cbErrorMsgMax	- Space available for error message	|
|		pcbErrorMsg	- Length of error message		|
+----------------------------------------------------------------------*/

extern BOOL DLLOVSPROC
SQLDataSourceToDriver (
	UDWORD		fOption,
        SWORD		fSqlType,
        PTR		rgbValueIn,
	SDWORD		cbValueIn,
	PTR		rgbValueOut,
	SDWORD		cbValueOutMax,
	SDWORD	FAR *	pcbValueOut,
	UCHAR	FAR *	szErrorMsg,
	SWORD		cbErrorMsgMax,
	SWORD	FAR *	pcbErrorMsg)
{
	UCHAR	FAR *	temp;
	FILE	FAR *	file;

/* Initialize error message and length				*/

	if (pcbErrorMsg != NULL) *pcbErrorMsg = 0;
	if (szErrorMsg  != NULL)  *szErrorMsg = 0;

/* See if value is too large to translate			*/

	if ((cbValueIn > cbValueOutMax) || (cbValueIn >= 0xffff)) {
		writeString (szErrorMsg, cbErrorMsgMax, pcbErrorMsg,
			(UCHAR *) "Input data too large to translate");
		return FALSE;
	}

	if ((fSqlType == SQL_CHAR) || (fSqlType == SQL_VARCHAR) ||
	    (fSqlType == SQL_LONGVARCHAR)) {

/* We are only converting character data			*/

		temp = (UCHAR FAR *) malloc ((unsigned int) (cbValueIn + 1));
		if (temp == NULL) {
nomem:			writeString (szErrorMsg, cbErrorMsgMax, pcbErrorMsg,
				(UCHAR *) "Unable to allocate memory");
			return FALSE;
		}

/* Copy and null-terminate input				*/

		memcpy (temp, rgbValueIn, (unsigned int) cbValueIn);
		temp[cbValueIn] = 0;

		if (cbValueIn == cbValueOutMax) {
			char *	out = (char *)malloc (
				(unsigned int) (cbValueIn+1));

			unsigned short	len;

/* Not enough room into output buffer for null-terminator;	*/
/* use a temporary						*/

			if (out == NULL) goto nomem;
			OemToAnsi (TYPEPARAM1 (temp), TYPEPARAM2 (out));
			len = lstrlen (out);
			if (pcbValueOut != NULL) *pcbValueOut = len;

/* If there's room for the null-terminator, include it		*/

			if (len < ((unsigned int) cbValueOutMax)) len++;
			memcpy (rgbValueOut, out, len);
			free (out);
		} else {
			OemToAnsi (TYPEPARAM1 (temp),
				TYPEPARAM2 (rgbValueOut));
			if (pcbValueOut != NULL)
				*pcbValueOut =
					lstrlen (TYPEPARAM1 (rgbValueOut));
		}

		if (fOption == 210) {

/* As debugging aide, if the fOption is 210, copy before	*/
/* and after data values to a file.				*/

#if defined(__unix)
			file = fopen ("/tmp/interslv.txt", "a");
#else
			file = fopen ("c:\\interslv.txt", "a");
#endif
			if (file != NULL) {
				fprintf (file, "Input : %s\n", temp);
				fprintf (file, "Output: %s\n", rgbValueOut);
				fclose (file);
			}
		}

		free (temp);
	} else {

/* Copy input data to output					*/

		memcpy (rgbValueOut, rgbValueIn, (unsigned int) cbValueIn);

/* Set the return length					*/

		if (pcbValueOut != NULL) *pcbValueOut = cbValueIn;
	}

/* return success						*/

	return (TRUE);
}


/*----------------------------------------------------------------------+
|  SQLDriverToDataSource						|
|  Purpose:	Provide driver to data source conversions.		|
| 		See Chapter 29 of the ODBC Programmer's reference V2.0	|
+----------------------------------------------------------------------*/

extern BOOL DLLOVSPROC
SQLDriverToDataSource (
	UDWORD		fOption,
        SWORD		fSqlType,
        PTR		rgbValueIn,
	SDWORD		cbValueIn,
	PTR		rgbValueOut,
	SDWORD		cbValueOutMax,
	SDWORD	FAR *	pcbValueOut,
	UCHAR	FAR *	szErrorMsg,
	SWORD		cbErrorMsgMax,
	SWORD	FAR *	pcbErrorMsg)
{
	UCHAR	FAR *	temp;
	FILE	FAR *	file;

/* Initialize error message and length				*/

	if (pcbErrorMsg != NULL) *pcbErrorMsg = 0;
	if (szErrorMsg  != NULL)  *szErrorMsg = 0;

/* See if value is too large to translate			*/

	if ((cbValueIn > cbValueOutMax) || (cbValueIn >= 0xffff)) {
		writeString (szErrorMsg, cbErrorMsgMax, pcbErrorMsg,
			(UCHAR *) "Input data too large to translate");
		return FALSE;
	}

	if ((fSqlType == SQL_CHAR) || (fSqlType == SQL_VARCHAR) ||
	    (fSqlType == SQL_LONGVARCHAR)) {

/* We are only converting character data			*/

		temp = (UCHAR FAR *) malloc ((unsigned int) (cbValueIn + 1));
		if (temp == NULL) {
nomem:			writeString (szErrorMsg, cbErrorMsgMax, pcbErrorMsg,
				(UCHAR *) "Unable to allocate memory");
			return FALSE;
		}

/* Copy and null-terminate input				*/

		memcpy (temp, rgbValueIn, (unsigned int) cbValueIn);
		temp[cbValueIn] = 0;

		if (cbValueIn == cbValueOutMax) {
			UCHAR FAR *	out = (UCHAR FAR *)malloc (
				(unsigned int) (cbValueIn + 1));
			unsigned short	len;

/* Not enough room into output buffer for null-terminator;	*/
/* use a temporary						*/

			if (out == NULL) goto nomem;
			AnsiToOem (TYPEPARAM1 (temp), TYPEPARAM2 (out));
			len = lstrlen ((const char *) out);
			if (pcbValueOut != NULL) *pcbValueOut = len;

/* If there's room for the null-terminator, include it		*/

			if (len < ((unsigned int) cbValueOutMax)) len++;
			memcpy (rgbValueOut, out, len);
			free (out);
		} else {
			AnsiToOem (TYPEPARAM1 (temp),
				TYPEPARAM2 (rgbValueOut));
			if (pcbValueOut != NULL)
				*pcbValueOut =
					lstrlen (TYPEPARAM1 (rgbValueOut));
		}

		if (fOption == 210) {

/* As debugging aide, if the fOption is 210, copy before	*/
/* and after data values to a file.				*/

#if defined(__unix)
			file = fopen ("/tmp/interslv.txt", "a");
#else
			file = fopen ("c:\\interslv.txt", "a");
#endif
			if (file != NULL) {
				fprintf (file, "Input : %s\n", temp);
				fprintf (file, "Output: %s\n", rgbValueOut);
				fclose (file);
			}
		}

		free (temp);
	} else {

/* Copy input data to output					*/

		memcpy (rgbValueOut, rgbValueIn, (unsigned int) cbValueIn);

/* Set the return length					*/

		if (pcbValueOut != NULL) *pcbValueOut = cbValueIn;
	}

/* return success						*/

	return (TRUE);
}

#if defined (QE_OS2) || defined (__unix) || defined(QE_MAC) || defined (QE_MAC_PPC)
/*	dummy these out since they're not meaningful for os2.	*/
void AnsiToOem (const char * temp, char * out)
{
	strcpy(out, temp);
}

void OemToAnsi (const char * temp, char * out)
{
	strcpy(out, temp);
}

#endif



