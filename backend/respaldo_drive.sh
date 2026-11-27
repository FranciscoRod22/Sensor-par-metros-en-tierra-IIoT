#!/bin/bash
source "$(dirname "$0")/config_privada.sh"

# 1. Definir la fecha para que el archivo no se sobrescriba
FECHA=$(date +"%Y-%m-%d_%H-%M")
ARCHIVO="/home/francisco-rod/Proyecto_CIBNOR/respaldo_sensores_$FECHA.csv"

# 2. Exportar la tabla 'sensores' de MariaDB a un archivo CSV
# Usamos el usuario cibnor_user y su contraseña
MYSQL_PWD="$DB_PASS" mysql -u cibnor_user -D liac -e "SELECT * FROM sensores;" > $ARCHIVO

# 3. Subir el archivo a Google Drive (creará la carpeta automáticamente)
rclone copy $ARCHIVO gdrive:Respaldos_CIBNOR/

# 4. Borrar el archivo CSV local para no llenar el disco de la máquina virtual
rm $ARCHIVO

# 5. Imprimir un mensaje de confirmación
echo "Respaldo subido a Drive exitosamente: $FECHA"
