#
# Build system for eFTE
#
# Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
#
# You may distribute under the terms of either the GNU General Public
# License or the Artistic License, as specified in the README file.
#

cmake_minimum_required(VERSION 2.4)

PROJECT( eFTE )
ADD_SUBDIRECTORY( src )

INSTALL( DIRECTORY config
         DESTINATION share/efte
         FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
         DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
                               WORLD_READ WORLD_EXECUTE
         PATTERN ".svn" EXCLUDE
         PATTERN "*~" EXCLUDE
         PATTERN "config/*" )

INSTALL( FILES AUTHORS Artistic COPYING HISTORY README
         DESTINATION share/doc/efte
         PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ )

INSTALL( FILES packaging/shared/efte.desktop
         DESTINATION share/applications
         PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ )
INSTALL( FILES
         packaging/shared/efte16x16.xpm
         packaging/shared/efte32x32.xpm
         packaging/shared/efte48x48.xpm
         packaging/shared/efte64x64.xpm
         DESTINATION share/pixmaps
         PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ )

