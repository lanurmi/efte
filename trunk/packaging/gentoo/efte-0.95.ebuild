# Copyright 1999-2007 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $
 
DESCRIPTION="eFTE is a fast text editor supporting folding, syntax highlighting and much more..."
HOMEPAGE="http://efte.sourceforge.net"
SRC_URI="http://downloads.sourceforge.net/efte/${PF}.tar.gz"
SLOT="0"
LICENSE="GPL"
KEYWORDS="~x86"
IUSE="gpm X"
 
DEPEND="dev-util/cmake
        sys-libs/ncurses
        gpm? ( sys-libs/gpm )
        X? ( x11-base/xorg-x11 )
        X? ( x11-libs/libXpm )"
 
RDEPEND=${DEPEND}
 
src_compile() {
        FLAGS="-DCMAKE_INSTALL_PREFIX=/usr"
        if ! use gpm ; then FLAGS="-DBUILD_GPM=OFF $FLAGS" ; fi
        if ! use X ; then FLAGS="-DBUILD_X=OFF $FLAGS" ; fi
        cmake $FLAGS .
        emake || die "Error while make process"
}
 
src_install() {
        DESTDIR=${WORKDIR}
        emake install DESTDIR="${D}" || die "Error while installation"
}
