if ( -d './jre/lib/amd64' ) then
        set JRE_PATH="/jre/lib/amd64/server"
else
        set JRE_PATH="/usr/lib/jvm/java-11-openjdk-amd64/lib/server"
endif

if ($?LD_LIBRARY_PATH == 1) then
	setenv LD_LIBRARY_PATH	/home/soufiane/Progress/DataDirect/ODBC_64bit/lib:${JRE_PATH}:${LD_LIBRARY_PATH}
else
	setenv LD_LIBRARY_PATH	/home/soufiane/Progress/DataDirect/ODBC_64bit/lib:${JRE_PATH}
endif
setenv ODBCINI /home/soufiane/Progress/DataDirect/ODBC_64bit/odbc.ini
setenv ODBCINST /home/soufiane/Progress/DataDirect/ODBC_64bit/odbcinst.ini