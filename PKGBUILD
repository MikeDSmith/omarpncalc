# Maintainer: Mike Smith
pkgname=omarpncalc
pkgver=0.1.0
pkgrel=1
pkgdesc="RPN calculator for Omarchy, built with Qt Quick"
arch=('x86_64')
url="https://github.com/mikedsmith/omarpncalc"
license=('MIT')
depends=('hicolor-icon-theme' 'qt6-base' 'qt6-declarative')
makedepends=('git')
source=("git+file://${startdir}")
sha256sums=('SKIP')

build() {
  cd "$srcdir/$pkgname"
  qmake6 omarpncalc.pro
  make
}

check() {
  cd "$srcdir/$pkgname"
  mkdir -p build-tests && cd build-tests
  qmake6 ../tests/tests.pro
  make
  QT_QPA_PLATFORM=offscreen ./tst_calculator

  # The engine tests never load the interface, so smoke that separately.
  QT_QPA_PLATFORM=offscreen "$srcdir/$pkgname/omarpncalc" --self-test
}

package() {
  cd "$srcdir/$pkgname"
  make INSTALL_ROOT="$pkgdir" install
  install -Dm644 LICENSE "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
}
