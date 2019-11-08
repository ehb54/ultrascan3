#!/bin/bash

DESTDIR=/cygdrive/c/Users/Admin/Documents/ultrascan3
US3SOMO=/cygdrive/c/Users/Admin/Documents/us3_somo

echo "Copy somo bin,etc,doc,demo directories"

cd /cygdrive/c/Users/Admin/Documents/us3_somo
cp -rp ${US3SOMO}/bin ${DESTDIR}/
cp -rp ${US3SOMO}/etc ${DESTDIR}/
cp -rp ${US3SOMO}/doc/manual/somo ${DESTDIR}/doc/manual/
cp -rp ${US3SOMO}/somo/demo ${DESTDIR}/somo/
cp -p ${US3SOMO}/LICENSE.txt ${DESTDIR}/

