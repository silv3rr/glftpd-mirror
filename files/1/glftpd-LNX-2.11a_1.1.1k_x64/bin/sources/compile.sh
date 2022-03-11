#!/bin/sh
glroot=/glftpd

#comment this line once you've edited the glftpd_bin_path
echo "You did not edit the file first!" && exit 1

echo "Compiling source files in `pwd` to $glroot/bin:" 
echo "(NOTE: Compiling can take a couple of minutes, please be patient.)"
if [ -n "$(which gcc)" ]; then
    usegcc="$(which gcc)"
    gccflags="-O2 -Wall -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -o"
elif [ -n "$(which cc)" ]; then
    usegcc="$(which cc)"
    gccflags="-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -o"
else
   echo "Error: Can't find a compiler to use. Please install one and compile all"
   echo "       the scripts in `pwd` and place them in"
   echo "       $glroot/bin.  Note these files are not necessary for"
   echo "       glftpd to operate, they are extra external scripts."
fi
[ -n "$usegcc" ] && {
    for cfile in *.c; do
        base="$(basename "${cfile%.c}")"
        [ -f "$glroot/bin/$base" ] && rm -f "$glroot/bin/$base"
        $usegcc $gccflags "$glroot/bin/$base" "$cfile" >/dev/null 2>&1 || \
          $usegcc -o "$glroot/bin/$base" "$cfile" >/dev/null 2>&1 || \
          failedc="$failedc   Failed to compile: \033[1;31m$cfile\033[0m\n"
    done
    if [ -z "$failedc" ]; then
        echo "All source files successfully compiled."
    else
        echo -e "$failedc"
    fi
}

exit 0

