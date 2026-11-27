# Configuración de Grafana

Grafana se conecta directamente a MariaDB. Los datos de la fuente de datos
(Connections > Data sources > MySQL) son:

| Campo     | Valor                                   |
|-----------|-----------------------------------------|
| Host      | localhost:3306                          |
| Database  | liac                                    |
| User      | cibnor_user                             |
| Password  | (la que definas al crear el usuario)    |

Se recomienda usar un usuario con permisos únicamente de lectura (SELECT)
sobre la tabla `sensores`.

Acceso web: http://IP_DE_LA_VM:3000

El dashboard exportado se encuentra en `dashboard_suelo.json`
y se importa desde Dashboards > New > Import.
