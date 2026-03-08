# Maintainer: canadiangamer
pkgname=fastfetch-cgos
pkgver=2.0  # update to match your fork's version tag
pkgrel=1
pkgdesc="fastfetch with CGOS patches (temporary until upstreamed)"
arch=('x86_64')
url="https://github.com/EasyCanadianGamer/fastfetch-cgos"
license=('MIT')
depends=('glibc' 'pciutils' 'libxrandr' 'libpulse' 'dconf')
makedepends=('cmake' 'ninja' 'git')
conflicts=('fastfetch')
provides=('fastfetch')

source=("${pkgname}::git+https://github.com/EasyCanadianGamer/fastfetch-cgos.git")
sha256sums=('SKIP')

pkgver() {
    cd "$pkgname"
    git describe --tags --abbrev=0 | sed 's/^v//'
}

build() {
    cd "$pkgname"
    cmake -B build -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr
    cmake --build build
}

package() {
    cd "$pkgname"
    DESTDIR="$pkgdir" cmake --install build
}
