# informix_connection.py

import pyodbc

# Connection string using the driver name
conn_str = (
        "DRIVER=/home/soufiane/Progress/DataDirect/ODBC_64bit/lib/ddifcl28.so;"
        "DATABASE=cmr;"
        "HOSTNAME=localhost;"
        "PORT=9088;"
        "PROTOCOL=TCPIP;"
        "UID=informix;"
        "PWD=mysecretpassword;"
    )

def create_connection():
    """
    Establish and return a connection to the Informix database.
    """
    try:
        conn = pyodbc.connect(conn_str)
        print("Connection successful!")
        conn.setdecoding(pyodbc.SQL_CHAR, encoding="utf-8")
        conn.setdecoding(pyodbc.SQL_WCHAR, encoding="utf-8")
        return conn
    except Exception as e:
        print(f"Connection failed: {e}")
        return None

