#!/bin/bash
set -e # Exits on error

NAME=$(date "+%Y_%m_%d")
OSID=$(uname)
if [ "$OSID" = "Darwin" ]; then
{
CC=clang CFLAGS="-mmacosx-version-min=10.7 -O3" \
  LDFLAGS="-mmacosx-version-min=10.7" make all
mkdir -p $OSID/x86_64
mkdir -p $OSID/arm64
cp generate/unix/bin/iasl $OSID/x86_64/iasl-$NAME.x86_64
cp generate/unix/bin/acpibin $OSID/x86_64/acpibin-$NAME.x86_64
cp generate/unix/bin/acpidump $OSID/x86_64/acpidump-$NAME.x86_64
cp generate/unix/bin/acpiexamples $OSID/x86_64/acpiexamples-$NAME.x86_64
cp generate/unix/bin/acpiexec $OSID/x86_64/acpiexec-$NAME.x86_64
cp generate/unix/bin/acpihelp $OSID/x86_64/acpihelp-$NAME.x86_64
cp generate/unix/bin/acpisrc $OSID/x86_64/acpisrc-$NAME.x86_64
cp generate/unix/bin/acpixtract $OSID/x86_64/acpixtract-$NAME.x86_64

CC=clang CFLAGS="-mmacosx-version-min=10.7 -O3" \
  LDFLAGS="-mmacosx-version-min=10.7" make clean
CFLAGS="-mmacosx-version-min=10.7 -O3 -target arm64-apple-darwin" \
  LDFLAGS="-mmacosx-version-min=10.7 -target arm64-apple-darwin" make all
cp generate/unix/bin/iasl $OSID/arm64/iasl-$NAME.arm64
cp generate/unix/bin/acpibin $OSID/arm64/acpibin-$NAME.arm64
cp generate/unix/bin/acpidump $OSID/arm64/acpidump-$NAME.arm64
cp generate/unix/bin/acpiexamples $OSID/arm64/acpiexamples-$NAME.arm64
cp generate/unix/bin/acpiexec $OSID/arm64/acpiexec-$NAME.arm64
cp generate/unix/bin/acpihelp $OSID/arm64/acpihelp-$NAME.arm64
cp generate/unix/bin/acpisrc $OSID/arm64/acpisrc-$NAME.arm64
cp generate/unix/bin/acpixtract $OSID/arm64/acpixtract-$NAME.arm64
cd $OSID
zip -qr -FS ./"acpica-$OSID-x86_64.zip" ./x86_64/*.x86_64
zip -qr -FS ./"acpica-$OSID-arm64.zip" ./arm64/*.arm64
}
else
{
mkdir -p $OSID/x86_64
make clean
make all
cp generate/unix/bin/iasl $OSID/x86_64/iasl-$NAME.x86_64
cp generate/unix/bin/acpibin $OSID/x86_64/acpibin-$NAME.x86_64
cp generate/unix/bin/acpidump $OSID/x86_64/acpidump-$NAME.x86_64
cp generate/unix/bin/acpiexamples $OSID/x86_64/acpiexamples-$NAME.x86_64
cp generate/unix/bin/acpiexec $OSID/x86_64/acpiexec-$NAME.x86_64
cp generate/unix/bin/acpihelp $OSID/x86_64/acpihelp-$NAME.x86_64
cp generate/unix/bin/acpisrc $OSID/x86_64/acpisrc-$NAME.x86_64
cp generate/unix/bin/acpixtract $OSID/x86_64/acpixtract-$NAME.x86_64
cd $OSID
zip -qr -FS ./"acpica-$OSID-x86_64.zip" ./x86_64/*.x86_64
}
fi

exit 0
