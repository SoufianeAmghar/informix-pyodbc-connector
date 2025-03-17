if [ -d './jre/lib/amd64' ]; then
	JRE_PATH="/jre/lib/amd64/server"
else
	JRE_PATH="/usr/lib/jvm/java-11-openjdk-amd64/lib/server"
fi

if [ "$LD_LIBRARY_PATH" = "" ]; then
	LD_LIBRARY_PATH=/home/soufiane/Progress/DataDirect/ODBC_64bit/lib:$JRE_PATH
else
	LD_LIBRARY_PATH=/home/soufiane/Progress/DataDirect/ODBC_64bit/lib:$JRE_PATH:$LD_LIBRARY_PATH
fi
export LD_LIBRARY_PATH

ODBCINI=/home/soufiane/Progress/DataDirect/ODBC_64bit/odbc.ini
export ODBCINI
ODBCINST=/home/soufiane/Progress/DataDirect/ODBC_64bit/odbcinst.ini
export ODBCINST