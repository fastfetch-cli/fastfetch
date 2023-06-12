# New Garuda-Linux ASCII Logo's
# forked from https://github.com/LinusDierheimer/fastfetch
# Maintainer: Linus Dierheimer <Linus@Dierheimer.de>
# Garuda-Fastfetch
# Maintainer: SGS <sgs@garudalinux.org>

pkgname=garuda-fastfetch-git
pkgver=1.7.1
pkgrel=1
pkgdesc="Like neofetch, but much faster because written in c"
arch=("x86_64" "i686" "pentium4" "armv5" "armv6h" "armv7h" "aarch64")
url="https://github.com/LinusDierheimer/fastfetch"
license=("MIT")

depends=(
  "glibc"
)
makedepends=(
  "git"
  "cmake"
  "pciutils"
  "vulkan-headers"
  "vulkan-icd-loader"
  "wayland"
  "libxcb"
  "libxrandr" # Depends on libX11, which headers are also needed
  "dconf"     # Depends on glib2, which headers are also needed
  "imagemagick"
  "chafa"
  "zlib"
  "dbus"
  "xfconf"
  "mesa"
  "opencl-headers"
  "ocl-icd"
  # "libmagick6"
  # "sqlite3"
  # "rpm-tools"
)
optdepends=(
  "pciutils: GPU output"
  "vulkan-icd-loader: GPU fallback"
  "wayland: Improved Wayland support (resolution + multi monitor)"
  "libxcb: Improved X11 support (resolution + multi monitor)"
  "libx11: Improved X11 support (resolution)"
  "libxrandr: Improved X11 support (resolution + multi monitor)"
  "glib2: Output for values that are only stored in GSettings"
  "dconf: Output for values that are only stored in DConf"
  "dbus: Media player and song output"
  "imagemagick: Image output using sixel or kitty graphics protocol"
  "chafa: Image output as ascii art"
  "zlib: Faster image output when using kitty graphics protocol"
  "xfconf: XFWM theme + xfce-terminal font"
  "mesa: OpenGL module"
  "ocl-icd: OpenCL module"
  # "libmagick6: ImageMagick 6 support"
  # "sqlite3: rpm package count"
  # "rpm-tools: slower rpm package count fallback"
)

_provides_and_conflicts=(
  "fastfetch"
)
provides=("${_provides_and_conflicts[@]}")
conflicts=("${_provides_and_conflicts[@]}")

_src_dir="${pkgname}"
source=("${_src_dir}::git+https://github.com/sgse/fastfetch.git")
sha256sums=("SKIP")

_build_dir="build"

pkgver() {
  cd "${_src_dir}"
  printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
}

build() {
  cmake \
    -B "${_build_dir}" \
    -S "${_src_dir}" \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DENABLE_SQLITE3=OFF \
    -DENABLE_RPM=OFF \
    -DENABLE_IMAGEMAGICK6=OFF \
    -Wno-dev

  cmake \
    --build "${_build_dir}"
}

package() {
  DESTDIR="${pkgdir}" \
  cmake \
    --install "${_build_dir}"
}
