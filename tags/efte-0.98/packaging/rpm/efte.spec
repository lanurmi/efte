Summary: eFTE Text Editor
Name: efte
Version: 1.0
Release: 0.1.rc1
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
cmake . -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release

%build
make

%install
install -d $RPM_BUILD_ROOT/etc/efte
make install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,sys)

/usr/bin/efte
/usr/bin/nefte
/usr/bin/vefte
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
