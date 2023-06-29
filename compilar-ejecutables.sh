#!/bin/bash

IMAGE_NAME=mirai-gen-bins-cc
DOCKERFILE=Dockerfile-1-compilar

DIR_XCOMPS=tmp/xcomps
DIR_EJECUTABLES=tmp/bin

BASE_DIR=`pwd`

echo
echo "Descargando cross-compilers..."
echo

mkdir -p $DIR_XCOMPS
cd $DIR_XCOMPS

archs=(i586 armv4l m68k mips mipsel powerpc sh4 sparc)

for arch in "${archs[@]}"; do
  file=cross-compiler-$arch.tar.bz2
  url=https://www.uclibc.org/downloads/binaries/0.9.30.1/$file
  if [[ ! -f $file ]]; then
    echo Descargando $url ...
    wget $url --no-check-certificate
  else
    echo $file ya está descargado.
  fi
done

cd "$BASE_DIR"

echo
echo "Construyendo imagen docker para compilar los ejecutables..."
echo

# añadir "--progress=plain" para ver el progreso completo
docker build --progress=plain -f $DOCKERFILE -t $IMAGE_NAME .
RET_CODE=$?

if [ ! $RET_CODE -eq 0 ]; then
  echo
  echo "ERROR: No se ha podido construir la imagen docker o compilar los binarios."
  exit 1
fi

echo docker run --rm -it -v "${PWD}/$DIR_EJECUTABLES:/bins" --entrypoint /export-bins.sh --cap-add=NET_ADMIN --platform=linux/amd64 $IMAGE_NAME

docker run --rm -it -v "${PWD}/$DIR_EJECUTABLES:/bins" --entrypoint /export-bins.sh \
    --cap-add=NET_ADMIN --platform=linux/amd64 $IMAGE_NAME

RET_CODE=$?

if [ ! $RET_CODE -eq 0 ]; then
  echo
  echo "ERROR: No se han podido copiar los binarios de la imagen docker."
  echo $RET_CODE
  exit 1
fi

ls -laFh $DIR_EJECUTABLES

echo
echo "Los binarios se encuentran en la carpeta \"$DIR_EJECUTABLES\"."

echo
echo "La imagen docker utilizada para generar los binarios ya no es necesaria. Para eliminarla, ejecute:"
echo "docker rmi -f $IMAGE_NAME && docker system prune -f"

echo
echo "Finalizado."
