
%define ver 0.49.13
%define prefix /usr/local/

Summary: FTE Text Editor (programmer oriented)
Name: fte
Version: %ver
Release: 1
Copyright: GPL/Artistic
Group: Applications/Editors
Source: fte-%ver.src.tar.gz
URL: http://www.kiss.uni-lj.si/~k4fr0235/fte/
Packager: Marko Macek <Marko.Macek@gmx.net>
BuildRoot: /tmp/build-fte-%ver

%description
FTE Text Editor

Folding.
Color syntax highlighting for many languages.
Smart indentation for C,C++,Java,Perl.
Fast. No mouse required :)
File/line size limited by virtual memory.

%prep

%setup

%build
make PREFIX=%{prefix}

%install
make PREFIX=$RPM_BUILD_ROOT%{prefix} install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc README COPYING Artistic CHANGES HISTORY TODO BUGS file_id.diz
%{prefix}/lib/fte/
%{prefix}/bin/fte
%{prefix}/bin/xfte
%{prefix}/bin/cfte
