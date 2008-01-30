#
# Build system for eFTE
#
# Copyright (c) 2008, eFTE SF Group
#

SET( CFTE_SOURCES cfte.cpp s_files.cpp s_string.cpp )
SET( BASE_FTE_SOURCES
  c_bind.cpp c_color.cpp c_config.cpp c_desktop.cpp c_hilit.cpp c_history.cpp
  c_mode.cpp commands.cpp e_block.cpp e_buffer.cpp e_cmds.cpp e_cvslog.cpp
  e_file.cpp e_fold.cpp e_line.cpp e_loadsave.cpp e_mark.cpp e_print.cpp
  e_redraw.cpp e_regex.cpp e_search.cpp e_svnlog.cpp e_tags.cpp e_trans.cpp
  e_undo.cpp egui.cpp fte.cpp g_draw.cpp g_menu.cpp gui.cpp h_ada.cpp
  h_c.cpp h_catbs.cpp h_falcon.cpp h_fte.cpp h_ipf.cpp h_make.cpp h_msg.cpp
  h_pascal.cpp h_perl.cpp h_plain.cpp h_rexx.cpp h_sh.cpp h_simple.cpp
  h_tex.cpp i_ascii.cpp i_choice.cpp i_complete.cpp i_input.cpp i_key.cpp
  i_modelview.cpp i_oview.cpp i_search.cpp i_view.cpp indent.cpp log.cpp
  o_buffer.cpp o_buflist.cpp o_cvs.cpp o_cvsbase.cpp o_cvsdiff.cpp o_directory.cpp
  o_list.cpp o_messages.cpp o_model.cpp o_modemap.cpp o_routine.cpp o_svn.cpp
  o_svnbase.cpp o_svndiff.cpp s_direct.cpp s_files.cpp s_string.cpp
  s_util.cpp view.cpp )
SET( UNIX_SOURCES e_unix.cpp memicmp.cpp )
SET( XFTE_SOURCES clip_x11.cpp con_i18n.cpp con_x11.cpp g_nodlg.cpp g_text.cpp
  menu_text.cpp )
SET( QFTE_SOURCES clib_x11.cpp g_qt.cpp g_qt_dlg.cpp )
SET( NFTE_SOURCES clip_no.cpp con_ncurses.cpp g_nodlg.cpp g_text.cpp g_unix_pipe.cpp
  menu_text.cpp )
SET( VFTE_SOURCES clip_no.cpp con_linux.cpp g_nodlg.cpp g_text.cpp )
SET( MFTE_SOURCES clip_no.cpp g_motif.cpp g_nodlg.cpp )
SET( WFTE_SOURCES clip_os2.cpp con_nt.cpp e_win32.cpp g_nodlg.cpp g_text.cpp
  menu_text.cpp )

OPTION( DEFAULT_CONFIG
  "Use a simple default configuration when system config cannot be found" ON)
IF( DEFAULT_CONFIG )
  ADD_DEFINITIONS( -DDEFAULT_INTERNAL_CONFIG )
ENDIF( DEFAULT_CONFIG )

# =============================================================================
#
# PLATFORM INDEPENDENT BUILDS
#
# =============================================================================

# Configuration compiler
SET_SOURCE_FILES_PROPERTIES( ${CFTE_SOURCES}
  PROPERTIES COMPILE_FLAGS "${COMPILER_FLAGS}" )
ADD_EXECUTABLE( cefte ${CFTE_SOURCES} )
GET_TARGET_PROPERTY( CEFTE_EXE cefte LOCATION )

# Program to convert our default config into an includable .h
ADD_EXECUTABLE( bin2c bin2c.cpp )
GET_TARGET_PROPERTY( BIN2C_EXE bin2c LOCATION )

# Builds our default config
ADD_CUSTOM_COMMAND(
  OUTPUT defcfg.cnf
  COMMAND ${CEFTE_EXE} defcfg.fte defcfg.cnf
  DEPENDS cefte )

# Convert our default config to ain includable .h
ADD_CUSTOM_COMMAND(
  OUTPUT defcfg.h
  COMMAND ${BIN2C_EXE} defcfg.cnf > defcfg.h
  DEPENDS bin2c defcfg.cnf )

# =============================================================================
#
# PLATFORM SPECIFIC BUILDS
#
# =============================================================================

IF( UNIX )
  SET( CMAKE_CXX_FLAGS "-pedantic -Wall -Wpointer-arith -Wconversion -Wwrite-strings -Winline -fno-rtti -fno-exceptions" )
  SET( XLIBS Xpm )

  OPTION( USE_XMB "Use libxmb to draw fonts with locale support" ON)
  IF( USE_XMB )
    ADD_DEFINITIONS( -DUSE_XMB )
  ENDIF( USE_XMB)

  OPTION( USE_LOCALE "Use locale" ON )
  IF( USE_LOCALE )
    ADD_DEFINITIONS( -DUSE_LOCALE )
  ENDIF( USE_LOCALE )

  OPTION( USE_XICON "Use libxicon to supply icons for the X version" ON )
  IF( USE_XICON )
    ADD_DEFINITIONS( -DUSE_XICON )
  ENDIF( USE_XICON )

  ADD_DEFINITIONS( -DUNIX -DUSE_XICON )

  INCLUDE(FindX11)
  ADD_EXECUTABLE( efte defcfg.h ${BASE_FTE_SOURCES} ${UNIX_SOURCES} ${XFTE_SOURCES} )
  TARGET_LINK_LIBRARIES( efte ${X11_LIBRARIES} Xpm ) # TODO: Search for XPM

  INCLUDE(FindCurses)
  ADD_EXECUTABLE( nefte defcfg.h ${BASE_FTE_SOURCES} ${UNIX_SOURCES} ${NFTE_SOURCES} )
  TARGET_LINK_LIBRARIES( nefte ${CURSES_LIBRARY} gpm ) #TODO: Search for GPM

ELSEIF( MAC )
ELSEIF( WIN32 )
ENDIF(UNIX)
