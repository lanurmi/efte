#
# Build system for eFTE
#
# Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
#
# You may distribute under the terms of either the GNU General Public
# License or the Artistic License, as specified in the README file.
#
#

PROJECT( eFTE )
ADD_SUBDIRECTORY( src )

INSTALL( DIRECTORY config
         DESTINATION share/efte
         PATTERN ".svn" EXCLUDE
         PATTERN "*~" EXCLUDE
         PATTERN "config/*"
         PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ )

INSTALL( FILES AUTHORS Artistic COPYING HISTORY README
         DESTINATION share/doc/efte )
