#!/bin/bash

BASE_DIR=`pwd`
DIR_TMP=tmp

mkdir -p tmp/bin

./compilar-ejecutables.sh

echo
echo "Construyendo imagen docker para el CnC..."
docker build --progress=plain -f Dockerfile-2-cnc -t mirai-cnc .

echo
echo "Construyendo imagen docker para el loader..."
docker build --progress=plain -f Dockerfile-3-loader -t mirai-loader .

echo
echo "Construyendo imagen docker para los bots..."
docker build --progress=plain -f Dockerfile-4-bot -t mirai-bot .

echo
echo "Construyendo imagen docker para cliente telnet..."
docker build --progress=plain -f Dockerfile-5-test-telnet -t telnet .

echo
echo "Descargando otras imágenes..."
docker pull nginx
docker pull nicolaka/netshoot

echo
echo "Limpiando docker..."
docker kill `docker ps -a -q`
docker system prune -f 

echo
echo "Creando subred de docker..."
docker network create --subnet=172.20.0.0/16 mirai-net

echo
echo "¡(Parcialmente) finalizado!"
echo "Aun quedan pasos que se deben ejecutar manualmente."
echo "Mire las instrucciones para más detalles."
