# informix_operations.py

from informix_connection import create_connection

# db_operations.py
def get_columns_and_data():
    conn = create_connection()
    if conn:
        cursor = conn.cursor()
        try:
            cursor.execute("SELECT * FROM employees")
            columns = [desc[0] for desc in cursor.description]
            print(f"Columns: {columns}")
            
            rows = cursor.fetchall()
            if rows:
                print("\nData in the 'employees' table:")
                for row in rows:
                    print(row)  # No manual decoding needed
            else:
                print("No data found.")
        except Exception as e:
            print(f"Error: {e}")
        finally:
            conn.close()
