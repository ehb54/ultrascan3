#!/bin/bash
#	make-wraps.sh  -  wrap thin HTML around PNG files

#  Run with no arguments to create thin HTML wrapper files
#   for all the images named in wrapped-images.lst.
#  Give a set of image names as arguments to create the
#   HTML files for only those images.

# File with list of wrapped images to create
LISTFILE=wrapped-images.lst
# Template file for wrapped img.html
TEMPLATEF=template.img.body

IMGNAMES=`cat ${LISTFILE}`
if [ $# -gt 0 ]; then
  # Use a given list of image names if provided
  IMGNAMES="$@"
fi

for IMG in ${IMGNAMES}; do
  # PNG image file to put a wrapper around
  PNGF=${IMG}.png
  # Wrapper file to create
  HTMLF=".staging/${IMG}.img.html"
  mkdir -p .staging
  
  echo "${PNGF}  WRAPPED TO:  ${HTMLF}"
  # Substitute image name in template and write to file
  sed -e "s/__IMGNAME__/${IMG}/g" ${TEMPLATEF} >${HTMLF}
done

