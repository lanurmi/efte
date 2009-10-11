# Copyright 1999-2008 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: /var/cvsroot/gentoo-x86/app-editors/efte/efte-0.96.ebuild,v 1.1 2008/02/18 00:24:27 omp Exp $

DESCRIPTION="A fast text editor supporting folding, syntax highlighting, etc."
HOMEPAGE="http://efte.sourceforge.net"
SRC_URI="http://downloads.sourceforge.net/${PN}/${P}.tar.gz"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="~amd64 ~x86"
IUSE="gpm X"

RDEPEND="sys-libs/ncurses
	gpm? ( sys-libs/gpm )
	X? ( x11-base/xorg-x11
		x11-libs/libXpm )"
DEPEND="${RDEPEND}
	dev-util/cmake"

src_compile() {
	cmake \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX=/usr \
		-DBUILD_GPM=$(use gpm && echo ON || echo OFF) \
		-DBUILD_X=$(use X && echo ON || echo OFF) \
		./
	emake || die "emake failed"

	# Compile a default configuration file.
	src/cefte config/mymain.fte system.fterc
}

src_install() {
	emake DESTDIR="${D}" install || die "emake install failed"
	insinto /usr/share/efte
	doins system.fterc
}
