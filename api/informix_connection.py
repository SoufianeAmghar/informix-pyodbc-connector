# informix_connection.py
import pyodbc
from dotenv import load_dotenv
from loguru import logger
import os

# Load environment variables from .env file
load_dotenv()

# Resolve the relative path to an absolute path
driver_path = os.path.abspath(os.getenv('INFORMIX_DRIVER'))

# Connection string using the driver name
conn_str = (
    f"DRIVER={driver_path};"
    f"DATABASE={os.getenv('INFORMIX_DATABASE')};"
    f"HOSTNAME={os.getenv('INFORMIX_HOSTNAME')};"
    f"PORT={os.getenv('INFORMIX_PORT')};"
    f"PROTOCOL={os.getenv('INFORMIX_PROTOCOL')};"
    f"UID={os.getenv('INFORMIX_UID')};"
    f"PWD={os.getenv('INFORMIX_PWD')};"
)


def create_connection():
    """
    Establish and return a connection to the Informix database.
    """
    try:
        conn = pyodbc.connect(conn_str)
        logger.info("Connected successfully !")
        conn.setdecoding(pyodbc.SQL_CHAR, encoding="utf-8")
        conn.setdecoding(pyodbc.SQL_WCHAR, encoding="utf-8")
        return conn
    except Exception as e:
        logger.error(f"Connection failed: {e}")
        return None
