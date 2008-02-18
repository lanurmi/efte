Summary: eFTE Text Editor
Name: efte
Version: 0.96
Release: 1
License: GPLv2+, Artistic
Group: Applications/Editors
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
Source: http://downloads.sourceforge.net/efte/%{name}-%{version}.tar.gz
Requires: gpm
Requires: ncurses
Requires: libX11
Requires: libXpm
BuildRequires: gcc-c++
BuildRequires: cmake
BuildRequires: gpm-devel
BuildRequires: ncurses-devel
BuildRequires: libX11-devel
BuildRequires: libXpm-devel


%description
eFTE Text Editor

Folding.
Color syntax highlighting for many languages.
Smart indentation for C,C++,Java,Perl.
Fast. No mouse required :)
File/line size limited by virtual memory.

%prep
rm -rf $RPM_BUILD_ROOT

%setup -q
cmake . -DCMAKE_INSTALL_PREFIX=/usr

%build
make

pushd config
../src/cefte mymain.fte system.fterc
popd

%install
install -d $RPM_BUILD_ROOT/etc/efte
install -m644 config/system.fterc $RPM_BUILD_ROOT/etc/efte
make install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,sys)

/usr/bin/cefte
/usr/bin/efte
/usr/bin/nefte
/usr/bin/vefte
/etc/efte/system.fterc
/usr/share
/usr/share/doc
/usr/share/doc/efte
/usr/share/doc/efte/README
/usr/share/doc/efte/AUTHORS
/usr/share/doc/efte/COPYING
/usr/share/doc/efte/Artistic
/usr/share/doc/efte/HISTORY
/usr/share/efte
/usr/share/efte/config
/usr/share/efte/config/slang
/usr/share/efte/config/slang/simple.keylist
/usr/share/efte/config/slang/slangkbd.map
/usr/share/efte/config/m_msg.fte
/usr/share/efte/config/m_perl.fte
/usr/share/efte/config/global.fte
/usr/share/efte/config/m_idl.fte
/usr/share/efte/config/m_sl.fte
/usr/share/efte/config/m_falcon.fte
/usr/share/efte/config/pal_bluez.fte
/usr/share/efte/config/m_c.fte
/usr/share/efte/config/m_pascal.fte
/usr/share/efte/config/mymain.fte
/usr/share/efte/config/m_gawk.fte
/usr/share/efte/config/ui_ws.fte
/usr/share/efte/config/color.fte
/usr/share/efte/config/ui_brief.fte
/usr/share/efte/config/main.fte
/usr/share/efte/config/ab_c_os2.fte
/usr/share/efte/config/rgbcolor.fte
/usr/share/efte/config/ui_vi.fte
/usr/share/efte/config/m_bin.fte
/usr/share/efte/config/m_catbs.fte
/usr/share/efte/config/ab_sh.fte
/usr/share/efte/config/m_php.fte
/usr/share/efte/config/ui_k_joe.fte
/usr/share/efte/config/m_fort90.fte
/usr/share/efte/config/m_xp.fte
/usr/share/efte/config/m_resdlg.fte
/usr/share/efte/config/ui_m_fte.fte
/usr/share/efte/config/kbd
/usr/share/efte/config/kbd/k_java.fte
/usr/share/efte/config/kbd/k_perl.fte
/usr/share/efte/config/kbd/k_fte.fte
/usr/share/efte/config/kbd/k_rexx.fte
/usr/share/efte/config/kbd/k_sgml.fte
/usr/share/efte/config/kbd/k_rst.fte
/usr/share/efte/config/kbd/k_c.fte
/usr/share/efte/config/kbd/k_html.fte
/usr/share/efte/config/m_asm370.fte
/usr/share/efte/config/m_merge.fte
/usr/share/efte/config/pal_blk.fte
/usr/share/efte/config/m_clario.fte
/usr/share/efte/config/m_source.fte
/usr/share/efte/config/m_vi.fte
/usr/share/efte/config/m_plain.fte
/usr/share/efte/config/ui_ne.fte
/usr/share/efte/config/m_tex.fte
/usr/share/efte/config/ab_c.fte
/usr/share/efte/config/m_asm.fte
/usr/share/efte/config/m_diff.fte
/usr/share/efte/config/m_xslt.fte
/usr/share/efte/config/m_siod.fte
/usr/share/efte/config/m_sql.fte
/usr/share/efte/config/uicstyle.fte
/usr/share/efte/config/ui_k_ne.fte
/usr/share/efte/config/m_a51.fte
/usr/share/efte/config/m_mod3.fte
/usr/share/efte/config/m_xml.fte
/usr/share/efte/config/m_unrealscript.fte
/usr/share/efte/config/m_sh.fte
/usr/share/efte/config/m_texi.fte
/usr/share/efte/config/m_html.fte
/usr/share/efte/config/m_rexx.fte
/usr/share/efte/config/pal_gray.fte
/usr/share/efte/config/ui_k_fte.fte
/usr/share/efte/config/m_ipf.fte
/usr/share/efte/config/m_ruby.fte
/usr/share/efte/config/systemmain.fte
/usr/share/efte/config/m_ldsgml.fte
/usr/share/efte/config/m_tcl.fte
/usr/share/efte/config/m_mvsasm.fte
/usr/share/efte/config/ab_java.fte
/usr/share/efte/config/m_markup.fte
/usr/share/efte/config/m_rst.fte
/usr/share/efte/config/m_text.fte
/usr/share/efte/config/m_4gl.fte
/usr/share/efte/config/ui_m_ws.fte
/usr/share/efte/config/pal_nce.fte
/usr/share/efte/config/m_py.fte
/usr/share/efte/config/menu
/usr/share/efte/config/menu/m_c.fte
/usr/share/efte/config/menu/m_html.fte
/usr/share/efte/config/menu/m_rexx.fte
/usr/share/efte/config/menu/m_rst.fte
/usr/share/efte/config/menu/m_sgml.fte
/usr/share/efte/config/m_ebnf.fte
/usr/share/efte/config/ab_perl.fte
/usr/share/efte/config/pal_b_kb.fte
/usr/share/efte/config/ui_m_ne.fte
/usr/share/efte/config/m_sml.fte
/usr/share/efte/config/m_ada.fte
/usr/share/efte/config/m_make.fte
/usr/share/efte/config/ui_k_ws.fte
/usr/share/efte/config/ab_rexx.fte
/usr/share/efte/config/pal_wht.fte
/usr/share/efte/config/m_java.fte
/usr/share/efte/config/ui_mew.fte
/usr/share/efte/config/ui_fte.fte
/usr/share/efte/config/pal_blue.fte
/usr/share/efte/config/k_brief.fte
/usr/share/efte/config/m_fte.fte
/usr/share/efte/config/pal_base.fte
/usr/share/efte/config/m_cmake.fte
/usr/share/efte/config/m_sgml.fte
