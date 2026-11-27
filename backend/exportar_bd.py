import pandas as pd
import pymysql
from datetime import datetime

# Configuración de la Base de Datos
DB_HOST = "localhost"
DB_USER = "cibnor_user"
from config_privada import DB_PASS
DB_NAME = "liac"

def generar_excel():
    try:
        # 1. Conectar y extraer datos
        conexion = pymysql.connect(host=DB_HOST, user=DB_USER, password=DB_PASS, database=DB_NAME)
        query = "SELECT * FROM sensores ORDER BY id DESC"
        df = pd.read_sql(query, conexion)
        conexion.close()

        if df.empty:
            print("La tabla está vacía, no hay nada que exportar.")
            return

        # 2. Crear el archivo Excel en la carpeta temporal /tmp
        nombre_archivo = f"/tmp/Respaldo_Suelo_{datetime.now().strftime('%Y-%m-%d_%H-%M')}.xlsx"
        
        # Usamos xlsxwriter para el diseño
        writer = pd.ExcelWriter(nombre_archivo, engine='xlsxwriter')
        df.to_excel(writer, sheet_name='Datos CIBNOR', index=False)
        
        workbook  = writer.book
        worksheet = writer.sheets['Datos CIBNOR']

        # 3. Aplicar estilos (Formato bonito)
        header_format = workbook.add_format({
            'bold': True,
            'text_wrap': True,
            'valign': 'vcenter',
            'align': 'center',
            'fg_color': '#1F4E78', # Azul oscuro
            'font_color': 'white',
            'border': 1
        })

        # Aplicar el estilo a la primera fila
        for col_num, value in enumerate(df.columns.values):
            worksheet.write(0, col_num, value, header_format)

        # Auto-ajustar el ancho de las columnas
        for i, col in enumerate(df.columns):
            # Encontrar la longitud máxima en la columna
            column_len = max(df[col].astype(str).map(len).max(), len(col)) + 2
            worksheet.set_column(i, i, column_len)

        writer.close()
        print(f"Archivo Excel generado con éxito: {nombre_archivo}")

    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    generar_excel()
