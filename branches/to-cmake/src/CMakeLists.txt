#
# Build system for eFTE
#
# Copyright (c) 2008, eFTE SF Group
#

# =============================================================================
#
# OPTION DEFINITIONS
#
# =============================================================================

OPTION( USE_DEFAULT_CONFIG
  "Use a simple default configuration when system config cannot be found" ON)
OPTION( USE_GPM "Compile console versions with gpm support" ON )
OPTION( USE_XMB "Use libxmb to draw fonts with locale support" ON)
OPTION( USE_LOCALE "Use locale" ON )
OPTION( USE_XICON "Use libxicon to supply icons for the X version" ON )
OPTION( USE_HARD_REMAP "Keyboard remapping by XEFTE" OFF )
OPTION( USE_XTINIT "Use XtInitialize on init" OFF )
OPTION( USE_LOGGING "Enable trace logging within efte" ON )

# Setup global options right now platform dependent options will appear later
# more near thier actual use

IF( USE_DEFAULT_CONFIG )
  ADD_DEFINITIONS( -DDEFAULT_INTERNAL_CONFIG )
ENDIF( USE_DEFAULT_CONFIG )

IF( USE_LOCALE )
  ADD_DEFINITIONS( -DUSE_LOCALE )
ENDIF( USE_LOCALE )

IF( NOT USE_LOGGING )
  ADD_DEFINITIONS( -DFTE_NO_LOGGING )
ENDIF( NOT USE_LOGGING )

# =============================================================================
#
# SOURCE FILES
#
# =============================================================================

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
SET( NFTE_SOURCES clip_no.cpp con_ncurses.cpp g_nodlg.cpp g_text.cpp g_unix_pipe.cpp
  menu_text.cpp )
SET( VFTE_SOURCES clip_no.cpp con_linux.cpp g_nodlg.cpp g_text.cpp )
SET( WFTE_SOURCES clip_os2.cpp con_nt.cpp e_win32.cpp g_nodlg.cpp g_text.cpp
  menu_text.cpp )

# =============================================================================
#
# PLATFORM SETUP
#
# =============================================================================

IF( UNIX AND ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
  SET( OSX 1 )
ENDIF( UNIX AND ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )

IF( CMAKE_COMPILER_IS_GNUCC )
  SET( CMAKE_CXX_FLAGS "-pedantic -Wall -Wpointer-arith -Wconversion -Wwrite-strings -Winline -fno-rtti -fno-exceptions" )
ENDIF( CMAKE_COMPILER_IS_GNUCC )

IF( WIN32 )

  IF( BORLAND )
  ENDIF( BORLAND )

  IF( CYGWIN )
  ENDIF( CYGWIN)

  IF( MINGW )
  ENDIF( MINGW)

  IF( MSVC )
  ENDIF( MSVC )

  IF( WATCOM )
  ENDIF( WATCOM )

ENDIF( WIN32 )

# =============================================================================
#
# PLATFORM INDEPENDENT BUILDS
#
# =============================================================================

# Configuration compiler
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

IF( OSX )

  # TODO: Create OSX build

ELSEIF( UNIX )

  IF( USE_XMB )
    ADD_DEFINITIONS( -DUSE_XMB )
  ENDIF( USE_XMB)

  IF( USE_XICON )
    ADD_DEFINITIONS( -DUSE_XICON )
  ENDIF( USE_XICON )

  ADD_DEFINITIONS( -DUNIX )
  IF( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
    ADD_DEFINITIONS( -DLINUX )
  ENDIF( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )

  INCLUDE(FindX11)
  IF( X11_LIBRARIES )
    MESSAGE( STATUS "X11 found on system, X11 enabled efte will be built" )
    INCLUDE_DIRECTORIES( ${X11_INCLUDE_DIR} )
    ADD_EXECUTABLE( efte defcfg.h ${BASE_FTE_SOURCES} ${UNIX_SOURCES} ${XFTE_SOURCES} )

    FIND_LIBRARY( XPM_LIBRARY Xpm )
    IF( NOT XPM_LIBRARY )
      MESSAGE( SEND_ERROR "Could not find Xpm library which is required for X11 version" )
    ENDIF( NOT XPM_LIBRARY )
    TARGET_LINK_LIBRARIES( efte ${X11_LIBRARIES} ${XPM_LIBRARY} ) # TODO: Search for XPM
  ENDIF( X11_LIBRARIES )

  INCLUDE(FindCurses)
  IF( CURSES_LIBRARY )
    MESSAGE( STATUS "Curses found on system, curses enabled nefte will be built" )
    ADD_EXECUTABLE( nefte defcfg.h ${BASE_FTE_SOURCES} ${UNIX_SOURCES} ${NFTE_SOURCES} )
    TARGET_LINK_LIBRARIES( nefte ${CURSES_LIBRARY} gpm ) #TODO: Search for GPM
  ENDIF( CURSES_LIBRARY )

ELSEIF( WIN32 )

  # TODO: This will only work with MSVC... change

  ADD_DEFINITIONS( -DNT -D_CRT_NONSTDC_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DNTCONSOLE -DMSVC -D_CONSOLE -DWIN32 )
  ADD_EXECUTABLE( efte defcfg.h ${BASE_FTE_SOURCES} ${WFTE_SOURCES} )
  TARGET_LINK_LIBRARIES( efte user32.lib )

ENDIF( OSX )
