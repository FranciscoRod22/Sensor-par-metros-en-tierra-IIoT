from flask import Flask
import subprocess

app = Flask(__name__)

@app.route('/run-backup', methods=['GET', 'POST'])
def ejecutar_respaldo():
    print("\n>>> ¡Petición recibida desde Google Sheets! Iniciando respaldo...")
    
    # El comando que ya sabemos que funciona
    comando = 'python3 /home/francisco-rod/Proyecto_CIBNOR/exportar_bd.py && rclone copy "$(ls -t /home/francisco-rod/Proyecto_CIBNOR/Respaldo_Suelo_*.xlsx | head -n 1)" cibnor_drive:Respaldos_CIBNOR -P'
    
    # Ejecuta el comando en segundo plano para no bloquear el servidor web
    subprocess.Popen(comando, shell=True)
    
    return "Respaldo iniciado correctamente en la VM", 200

if __name__ == '__main__':
    # Escucha en el puerto 5000
    app.run(host='0.0.0.0', port=5000)
