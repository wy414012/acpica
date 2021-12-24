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
cp generate/unix/bin/* $OSID/x86_64/
cp documents/* $OSID/x86_64/
CC=clang CFLAGS="-mmacosx-version-min=10.7 -O3" \
  LDFLAGS="-mmacosx-version-min=10.7" make clean
CFLAGS="-mmacosx-version-min=10.7 -O3 -target arm64-apple-darwin" \
  LDFLAGS="-mmacosx-version-min=10.7 -target arm64-apple-darwin" make all
cp generate/unix/bin/* $OSID/arm64/
cp documents/* $OSID/arm64/
cd $OSID
zip -qr -FS ./"acpica-$OSID-$NAME-x86_64.zip" ./x86_64/*
zip -qr -FS ./"acpica-$OSID-$NAME-arm64.zip" ./arm64/*
}
else
{
mkdir -p $OSID/x86_64
make clean
make all
cp generate/unix/bin/* $OSID/x86_64/
cp documents/* $OSID/x86_64/
cd $OSID
zip -qr -FS ./"acpica-$OSID-$NAME-x86_64.zip" ./x86_64/*
}
fi

exit 0
