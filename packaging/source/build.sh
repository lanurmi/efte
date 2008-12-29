#!/bin/bash

NAME=efte

VERSIONFILE="../../src/ftever.h"
DEFAULTVERSION="$(sed -n 's/^\#define VERSION *\"//p' $VERSIONFILE |sed 's/\".*$//')"
VERSION=${1:-$DEFAULTVERSION}        # use DEFAULT version if not specified on command line
[[ -z $VERSION ]] && exit 1          # no version, so exit if none specified and none found

BASEPACKNAME=${NAME}-${VERSION}

if [[ -d ${BASEPACKNAME} ]] ; then
  echo "Destination already exists. Previous packages with the same version number"
  echo "will be removed and existing destination will be renamed."
  echo "CTRL-C to abort, ENTER to continue..."
  read
  mv "$BASEPACKNAME"  "${BASEPACKNAME}.$(date +%s)"
  rm -f "$BASEPACKNAME.tar.gz" "$BASEPACKNAME.tar.bz2" "$BASEPACKNAME.zip"
fi

svn co http://$NAME.svn.sourceforge.net/svnroot/$NAME/tags/${BASEPACKNAME} ${BASEPACKNAME} &&
find "$BASEPACKNAME" -type d -name .svn | while read DIR ; do rm -r "$DIR" ; done

tar czf ${BASEPACKNAME}.tar.gz ${BASEPACKNAME}
tar cjf ${BASEPACKNAME}.tar.bz2 ${BASEPACKNAME}
zip -r ${BASEPACKNAME}.zip ${BASEPACKNAME}

echo ""
echo "Upload to SourceForge? CTRL-C to abort, ENTER to continue..."
read
ncftpput upload.sourceforge.net /incoming \
  ${BASEPACKNAME}.tar.bz2 ${BASEPACKNAME}.tar.gz ${BASEPACKNAME}.zip
