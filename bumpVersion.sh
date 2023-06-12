makepkg -f

version=0
cd "fastfetch/"
version="$(git rev-list --count HEAD)"
cd ..

makepkg --printsrcinfo > .SRCINFO

git add .
git commit -m "bumped version to r${version}"
git push
