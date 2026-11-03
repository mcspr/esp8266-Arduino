#!/usr/bin/env bash

# `make install` reversed
# `make uninstall` only removes libs, no reason re-configuring and staging the old tree

set -e -u

srcdir="$1"  # newlib-4.0.0 repo
destdir="$2"  # usually, $installdir/xtensa-lx106-elf

# path to remove, relative to $installdir
seen=()

# binary blobs
for i in crt0.o libg.a libc.a libm.a ; do
  dest="$i"
  if test -f "$destdir"/lib/"$dest" ; then
    seen+=("lib/$dest")
  fi
done

function seen_include() {
  if test -f "$destdir"/include/"$1" ; then
    seen+=("include/$1")
  fi
}

# xtensa sys headers, seen before include/sys/
for i in "${srcdir}"/newlib/libc/sys/xtensa/sys/*.h ; do
  dest=sys/"$(basename "$i")"
  seen_include "$dest"
done

# xtensa sys machine, seen before include/machine/
for i in "${srcdir}"/newlib/libc/sys/xtensa/include/machine/*.h ; do
  dest=machine/"$(basename "$i")"
  seen_include "$dest"
done

# xtensa config, deployed from the repo
# todo: just ignore as known path
for i in "${srcdir}"/newlib/libc/sys/xtensa/include/xtensa/config/*.h ; do
  dest=xtensa/config/"$(basename "$i")"
  seen_include "$dest"
done

# build dir dynamically generated headers
for i in newlib.h _newlib_version.h ; do
  dest="$i"
  seen_include "$dest"
done

# other base headers
for i in "${srcdir}"/newlib/libc/include/*.h ; do
  dest="$(basename "$i")"
  seen_include "$dest"
done

for i in "${srcdir}"/newlib/libc/include/machine/*.h ; do
  dest=machine/"$(basename "$i")"
  seen_include "$dest"
done

for i in "${srcdir}"/newlib/libc/include/rpc/*.h ; do
  dest=rpc/"$(basename "$i")"
  seen_include "$dest"
done

for i in "${srcdir}"/newlib/libc/include/ssp/*.h ; do
  dest=ssp/"$(basename "$i")"
  seen_include "$dest"
done

for i in "${srcdir}"/newlib/libc/include/sys/*.h ; do
  dest=sys/"$(basename "$i")"
  seen_include "$dest"
done

for i in "${srcdir}"/newlib/libc/include/bits/*.h ; do
  dest=bits/"$(basename "$i")"
  seen_include "$dest"
done

for s in "${seen[@]}"; do
  rm -vf "$destdir"/"$s"
done
