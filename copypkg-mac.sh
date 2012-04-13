#!/bin/sh
#		copypkg-mac.sh  - copy ultrascan3 directories to pkg/ultrascan3

ME=`whoami`
echo "Enter Admin password --"
sudo chown -R $ME:admin ~/us3/pkg/ultrascan3 ~/us3/pkg/somo ~/us3/pkg/Frameworks

for D in doc etc bin lib; do
  SDIR=~/us3/ultrascan3/${D}
  DDIR=~/us3/pkg/ultrascan3/${D}
  echo "(cd "${SDIR}";tar cvf - *) 2>/dev/null | (cd "$DDIR";tar xvf -)"
  (cd "${SDIR}";tar cvf - * .??*) | (cd "$DDIR";tar xvf -)
done

for D in bin demo doc lib saxs structures; do
  SDIR=~/us3/ultrascan3/somo/${D}
  DDIR=~/us3/pkg/somo/${D}
  echo "(cd "${SDIR}";tar cvf - *) 2>/dev/null | (cd "$DDIR";tar xvf -)"
  (cd "${SDIR}";tar cvf - * .??*) | (cd "$DDIR";tar xvf -)
done

SDIR=~/us3/ultrascan3/Frameworks
DDIR=~/us3/pkg/Frameworks
echo "(cd "${SDIR}";tar cvf - *) 2>/dev/null | (cd "$DDIR";tar xvf -)"
(cd "${SDIR}";tar cvf - * .??*) | (cd "$DDIR";tar xvf -)

cd ~/us3/pkg/ultrascan3
find ./ -name '.svn' | xargs rm -Rf
cd ~/us3/pkg/somo
find ./ -name '.svn' | xargs rm -Rf
cd ~/us3/pkg/Frameworks
find ./ -name '.svn' | xargs rm -Rf

cd ~/us3/pkg
pwd
echo "find ./ -name '.svn' -print"
find ./ -name '.svn' -print

/bin/rm -rf ~/us3/pkg/ultrascan3/somo/develop ~/us3/pkg/ultrascan3/somo/*.pl

echo "(Possibly) Enter Admin password --"
sudo chown -R root:admin ~/us3/pkg/ultrascan3 ~/us3/pkg/somo ~/us3/pkg/Frameworks
