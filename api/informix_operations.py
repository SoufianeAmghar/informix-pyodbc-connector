# informix_operations.py

from informix_connection import create_connection
from loguru import logger

# informix_operations.py
# Update this file with your custom informix operations

def get_columns_and_data():
    conn = create_connection()
    if conn:
        query = "SELECT * FROM employees"
        cursor = conn.cursor()
        try:
            cursor.execute(query)
            columns = [desc[0] for desc in cursor.description]
            logger.info(f"Columns: {columns}")
            
            rows = cursor.fetchall()
            if rows:
                logger.info("\nData in the 'employees' table:")
                for row in rows:
                    logger.info(row)
            else:
                logger.info("No data found.")
        except Exception as e:
            logger.error(f"Error: {e}")
        finally:
            conn.close()
