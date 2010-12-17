#
# Build system for eFTE
#
# Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
#
# You may distribute under the terms of either the GNU General Public
# License or the Artistic License, as specified in the README file.
#

# =============================================================================
#
# OPTION DEFINITIONS
#
# =============================================================================

# Build choices
OPTION( BUILD_X        "Build X version of eFTE" ON )
OPTION( BUILD_CONSOLE  "Build console version of eFTE" ON )

# Optional use flags
OPTION( USE_GPM        "Compile console versions with gpm support" UNKNOWN )
OPTION( USE_LOCALE     "Use normal locale support" ON )
OPTION( USE_XMB        "Use libxmb to draw fonts with locale support" ON)
OPTION( USE_XLOCALE    "Use X11 compiled in support" OFF )
OPTION( USE_XICON      "Use libxicon to supply icons for the X version" ON )
OPTION( USE_XTINIT     "Use XtInitialize on init" OFF )
OPTION( USE_HARD_REMAP "Keyboard remapping by XEFTE" OFF )
OPTION( USE_LOGGING    "Enable trace logging within efte" ON )
OPTION( USE_DEFAULT_CONFIG "Use default config built into efte" ON )

# Setup global options right now platform dependent options will appear later
# more near thier actual use

IF( USE_LOCALE )
  ADD_DEFINITIONS( -DUSE_LOCALE )
ENDIF( USE_LOCALE )
IF( USE_XLOCALE )
  ADD_DEFINITIONS( -DX_LOCALE )
ENDIF( USE_XLOCALE )
IF( NOT USE_LOGGING )
  ADD_DEFINITIONS( -DFTE_NO_LOGGING )
ENDIF( NOT USE_LOGGING )
IF( USE_GPM )
  ADD_DEFINITIONS( -DUSE_GPM )
ENDIF( USE_GPM )

# =============================================================================
#
# SOURCE FILES
#
# =============================================================================

SET( BASE_FTE_SOURCES
  cfte.cpp c_bind.cpp c_color.cpp c_config.cpp c_desktop.cpp c_hilit.cpp c_history.cpp
  c_mode.cpp commands.cpp e_block.cpp e_buffer.cpp e_cmds.cpp e_cvslog.cpp
  e_file.cpp e_fold.cpp e_line.cpp e_loadsave.cpp e_mark.cpp e_print.cpp
  e_redraw.cpp e_regex.cpp e_search.cpp e_svnlog.cpp e_tags.cpp e_trans.cpp
  e_undo.cpp egui.cpp fte.cpp g_draw.cpp g_menu.cpp gui.cpp h_ada.cpp
  h_c.cpp h_cont.cpp h_catbs.cpp h_fte.cpp h_ipf.cpp h_make.cpp h_msg.cpp
  h_pascal.cpp h_perl.cpp h_plain.cpp h_rexx.cpp h_sh.cpp h_simple.cpp
  h_tex.cpp i_ascii.cpp i_choice.cpp i_complete.cpp i_input.cpp i_key.cpp
  i_modelview.cpp i_oview.cpp i_search.cpp i_view.cpp indent.cpp log.cpp
  o_buffer.cpp o_buflist.cpp o_cvs.cpp o_cvsbase.cpp o_cvsdiff.cpp o_directory.cpp
  o_list.cpp o_messages.cpp o_model.cpp o_modemap.cpp o_routine.cpp o_svn.cpp
  o_svnbase.cpp o_svndiff.cpp s_direct.cpp s_files.cpp s_string.cpp
  s_util.cpp view.cpp ${CMAKE_CURRENT_BINARY_DIR}/defcfg.h )
SET( UNIX_SOURCES e_unix.cpp memicmp.cpp )
SET( XFTE_SOURCES clip_x11.cpp con_i18n.cpp con_x11.cpp g_nodlg.cpp g_text.cpp
  menu_text.cpp )
SET( NFTE_SOURCES clip_no.cpp con_ncurses.cpp g_nodlg.cpp g_text.cpp g_unix_pipe.cpp
  menu_text.cpp )
SET( VFTE_SOURCES clip_no.cpp con_linux.cpp g_nodlg.cpp g_text.cpp menu_text.cpp )
SET( WFTE_SOURCES clip_os2.cpp con_nt.cpp e_win32.cpp g_nodlg.cpp g_text.cpp
  menu_text.cpp ftewin32.rc )

# =============================================================================
#
# PLATFORM SETUP
#
# =============================================================================

SET( SYSTEM_LIBS "" )

IF( UNIX AND ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
  SET( OSX 1 )
ENDIF( UNIX AND ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )

IF( CMAKE_COMPILER_IS_GNUCC )
  SET( CMAKE_CXX_FLAGS "-pedantic -Wall -Wno-long-long -Wpointer-arith -Wconversion -Wwrite-strings -Winline -fno-rtti -fno-exceptions" )
ENDIF( CMAKE_COMPILER_IS_GNUCC )

IF( CMAKE_SYSTEM_NAME MATCHES "HP/UX" )
  ADD_DEFINITIONS( -DHPUX -D_HPUX_SOURCE -DCAST_FD_SET_INT )
ENDIF( CMAKE_SYSTEM_NAME MATCHES "HP/UX" )
IF( CMAKE_SYSTEM_NAME MATCHES "AIX" )
  ADD_DEFINITIONS( -DAIX -D_BSD_INCLUDES -DNO_NEW_CPP_FEATURES )
ENDIF( CMAKE_SYSTEM_NAME MATCHES "AIX" )
IF( CMAKE_SYSTEM_NAME MATCHES "Irix" )
  ADD_DEFINITIONS( -DIRIX -DNO_NEW_CPP_FEATURES )
  SET( CMAKE_CXX_FLAGS "-OPT:Olimit=3000" ) # -xc++
ENDIF( CMAKE_SYSTEM_NAME MATCHES "Irix" )
IF( CMAKE_SYSTEM_NAME MATCHES "SunOS" ) # Solaris
  ADD_DEFINITIONS( -DSUNOS -DNO_NEW_CPP_FEATURES )
ENDIF( CMAKE_SYSTEM_NAME MATCHES "SunOS" )
IF( CMAKE_SYSTEM_NAME MATCHES "SCO" )
  ADD_DEFINITIONS( -DSCO )
  SET( CMAKE_CXX_FLAGS "-b elf" )
  SET( SYSTEM_LIBS ${SYSTEM_LIBS} socket )
ENDIF( CMAKE_SYSTEM_NAME MATCHES "SCO" )
IF( CMAKE_SYSTEM_NAME MATCHES "NCR" )
  ADD_DEFINITIONS( -DNCR )
  SET( CMAKE_CXX_FLAGS "-w3" )
  SET( SYSTEM_LIBS ${SYSTEM_LIBS} socket nsl c ucb )
ENDIF( CMAKE_SYSTEM_NAME MATCHES "NCR" )

IF( WIN32 )
  IF( BORLAND )
    ADD_DEFINITIONS( -DBCPP )
  ENDIF( BORLAND )

  IF( CYGWIN )
  ENDIF( CYGWIN)

  IF( MINGW )
  ENDIF( MINGW)

  IF( MSVC )
    ADD_DEFINITIONS( -D_CRT_NONSTDC_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DMSVC )
  ENDIF( MSVC )

  IF( WATCOM )
  ENDIF( WATCOM )
ENDIF( WIN32 )

CONFIGURE_FILE(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")

# =============================================================================
#
# PLATFORM INDEPENDENT BUILDS
#
# =============================================================================

ADD_LIBRARY( eftebase STATIC ${BASE_FTE_SOURCES} )

ADD_EXECUTABLE( bin2c bin2c.cpp )
GET_TARGET_PROPERTY( BIN2C_EXE bin2c LOCATION )

# =====
#
# Binary config compiler that will soon disappear
#
# =====

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/defcfg.h
  COMMAND ${BIN2C_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/simple.fte > ${CMAKE_CURRENT_BINARY_DIR}/defcfg.h
  DEPENDS bin2c simple.fte )

# =============================================================================
#
# PLATFORM SPECIFIC BUILDS
#
# =============================================================================

IF( UNIX )
  IF( USE_XMB )
    ADD_DEFINITIONS( -DUSE_XMB )
  ENDIF( USE_XMB)

  IF( USE_XICON )
    ADD_DEFINITIONS( -DUSE_XICON )
  ENDIF( USE_XICON )

  ADD_DEFINITIONS( -DUNIX )
  IF( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
    ADD_DEFINITIONS( -DLINUX )
    SET( LINUX ON )
  ENDIF( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )

  IF( BUILD_X )
    INCLUDE(FindX11)
    IF( X11_LIBRARIES )
      MESSAGE( STATUS "X11 found on system, X11 enabled efte will be built" )
      INCLUDE_DIRECTORIES( ${X11_INCLUDE_DIR} )
      ADD_EXECUTABLE( efte ${UNIX_SOURCES} ${XFTE_SOURCES} )

      FIND_LIBRARY( XPM_LIBRARY Xpm )
      IF( NOT XPM_LIBRARY )
        MESSAGE( SEND_ERROR "Could not find Xpm library which is required for X11 version" )
      ENDIF( NOT XPM_LIBRARY )
      TARGET_LINK_LIBRARIES( efte eftebase ${X11_LIBRARIES} ${XPM_LIBRARY} ${SYSTEM_LIBS} )
      INSTALL( TARGETS efte RUNTIME DESTINATION bin )
    ELSE( X11_LIBRARIES )
      MESSAGE( STATUS "X11 not found, X11 enabled efte will not be built" )
    ENDIF( X11_LIBRARIES )
  ENDIF( BUILD_X )

  IF( BUILD_CONSOLE )
    IF( USE_GPM )
      FIND_LIBRARY( FOUND_GPM_LIBRARY gpm )
      IF( FOUND_GPM_LIBRARY )
        SET( GPM_LIBRARY ${FOUND_GPM_LIBRARY} )
      ELSE( FOUND_GPM_LIBRARY )
        MESSAGE( SEND_ERROR "Could not find gpm library" )
        MESSAGE( SEND_ERROR "Please install or disable the use of gpm:" )
        MESSAGE( SEND_ERROR "Add: -DUSE_GPM=OFF to your cmake command line" )
      ENDIF( FOUND_GPM_LIBRARY )
    ELSE( USE_GPM )
      SET( GPM_LIBRARY "" )
    ENDIF( USE_GPM )

    INCLUDE(FindCurses)
    IF( CURSES_LIBRARY )
      MESSAGE( STATUS "Curses found on system, nefte will be built" )

      ADD_EXECUTABLE( nefte ${UNIX_SOURCES} ${NFTE_SOURCES} )
      TARGET_LINK_LIBRARIES( nefte eftebase ${CURSES_LIBRARY} ${GPM_LIBRARY} ${SYSTEM_LIBS} )
      INSTALL( TARGETS nefte RUNTIME DESTINATION bin )
    ELSE( CURSES_LIBRARY )
      MESSAGE( SEND_ERROR "Could not find curses library" )
      MESSAGE( SEND_ERROR "Please install or disable building a console version:" )
      MESSAGE( SEND_ERROR "Add: -DBUILD_CONSOLE=OFF to your cmake command line" )
    ENDIF( CURSES_LIBRARY )

    IF( LINUX )
      MESSAGE( STATUS "Using Linux, vefte will be build" )
      ADD_EXECUTABLE( vefte ${UNIX_SOURCES} ${VFTE_SOURCES} )
      TARGET_LINK_LIBRARIES( vefte eftebase ${GPM_LIBRARY} ${SYSTEM_LIBS} )
      INSTALL( TARGETS vefte RUNTIME DESTINATION bin )
    ENDIF( LINUX )
  ENDIF( BUILD_CONSOLE )
ELSEIF( WIN32 )
  ADD_DEFINITIONS( -DNT -DNTCONSOLE -DWIN32 -D_CONSOLE )
  ADD_EXECUTABLE( efte ${CMAKE_CURRENT_BINARY_DIR}/defcfg.h ${WFTE_SOURCES} )
  TARGET_LINK_LIBRARIES( efte eftebase ${SYSTEM_LIBS} )
ENDIF( UNIX )

# =============================================================================
#
# PLATFORM INDEPENDENT BUILDS
#
# =============================================================================
