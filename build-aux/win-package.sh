#!/bin/bash

copy_deps() {
  echo "Copy deps: $1"
  cp $(ldd $1 2>&1 | grep -Po '=> \K/mingw64/\S+' | sort | uniq | tr '\n' ' ') bin
}

rm -rf _build/win
DESTDIR=$(PWD)/_build/win ninja -C _build install
cd _build/win

copy_deps bin/auricle.exe

mkdir -p lib

echo 'Copy: pixbuf'
cp -r /mingw64/lib/gdk-pixbuf-2.0 lib
for dll in lib/gdk-pixbuf-2.0/*/loaders/*.dll; do
  copy_deps $dll
done

echo 'Copy: gstreamer'
cp -r /mingw64/lib/gstreamer-1.0 lib
for dll in lib/gstreamer-1.0/*.dll; do
  copy_deps $dll
done

mkdir -p share
echo 'Copy: icons'
cp -r /mingw64/share/icons share

mkdir -p share/glib-2.0
echo 'Copy: schemas'
cp -r /mingw64/share/glib-2.0/schemas share/glib-2.0

mkdir -p share/mime
echo 'Copy: mime types'
cp -r /mingw64/share/mime/{audio,image,mime.cache} share/mime
