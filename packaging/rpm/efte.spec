Summary: eFTE Text Editor
Name: efte
Version: 1.1
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

%build
cd src && make PREFIX=/usr

%install
install -d $RPM_BUILD_ROOT/etc/efte
cd src && make install PREFIX=/usr DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,sys)

/usr/bin/efte
/usr/bin/nefte
/usr/bin/vefte
/usr/share/doc/efte
/usr/share/efte/config
