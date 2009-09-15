# Contributor: Benoit Rouits <brouits@free.fr>
pkgname=nall
pkgver=0.4
pkgrel=1
pkgdesc="a user script-based everything notifier on the systray"
arch=(i686)
url="http://brouits.free.fr/nall"
license=('GPL')
groups=()
depends=('gtk2' 'glib')
makedepends=('make' 'gcc' 'pkgconfig')
provides=()
conflicts=()
replaces=()
backup=()
install=
source=(http://brouits.free.fr/nall/releases/$pkgname-$pkgver.tar)
noextract=()

build() {
  cd "$srcdir/$pkgname-$pkgver"

  make || return 1
  make DESTDIR="$pkgdir" install
}
md5sums=('1f675b4d9fc82e38cc7201cac0bef3b2')
