#!/bin/sh
# jehsom's and dn's glftpd installer script
  VERSION="2.0.10"

# REPORT ANY BUGS TO: #glftpd@efnet

# CHANGELOG
#
# v2.0.9
# fixed:   Fix ipv6 and ipv4 support on macos
# fixed:   FreeBSD version detection
# v2.0.6
# new:     Now works with rlinetd
# v2.0.3
# fixed:   freebsd version detection and devfs mount fix from tittof
# new:     support for launchd on macos from tittof
# v2.0.2 (???)
# fixed:   Empty-dir creation had a bug (bugid #11), thanks to mz for reporting
# change:  As psxc isn't active anymore, the report-to is now generalized
# fixed:   Resolving tests were still against glftpd.com
# fixed:   Due to changing to git, some empty dirs are not present in the dirtree
#          anymore, and thus not copied to the installation location.
#          The script now creates these.
# new:     Creation of dev/full on linux and freebsd
# fixed:   Made creation of x509 certificate work on NetBSD - NetBSD by default
#          is missing the openssl.cnf file.
# change:  If an only_from line is found in xinetd.conf, it will be commented
#          out automatically. (Too few actually read the output from the
#          installer)
# change:  Checked for xinetd or inetd before we started asking questions - this
#          way people don't have to do almost a full install before it fails.
#
# v2.0.1 (x-mas 2005)
# fixed:   mknod is no longer supported on fbsd6 - added workaround.
# fixed:   European week was added to crontab but not (x)inetd.
# fixed:   Added chmod 777 site/ since this is a repeating Q in channel.
# new:     Added a test to make sure there is no only_from line in xinetd.conf.
# fixed:   Check that the 'bash' we found (and use) is actually a binary, and
#          also make sure it's not a faked sh renamed to bash
# change:  Changed the suggestion when choosing tcpd or not.
#
#------------------------------------------------------------------------------
#
# v2.0.1RC5 (Nov. 2005)
# change:  Changed default port from 21 to 1337 - no port should be <1024.
# fixed:   Did a few minor fixes with the resolv sequence.
# change:  The cert is now read from the conf and not the (x)inetd.
# v2.0.1RC2 (May 2005)
# fixed:   The sequence grabbing the necessary libs did not work on newer linux
#          versions.
# v2.0.1RC1
# fixed:   Small bug on osx install
# v2.0.0
# fixed:   Fixed missing jailpart in timezone area.
# new:     Made sure the linux run-time linker is copied.
# fix:     In case 'which' doesn't exists or is not in the PATH variable, an
#          alternate method has been added.
# change:  RC8 does no longer support non-tls - changed the installer to
#          reflect that change.
# v2.0.RC6
# new:     Added cpying of some necessary libs for osx/darwin.
# fixed:   If the bin 'dig' didn't exists, it would not check the content of
#          resolv.conf - added support for nslookup as well.
# v2.0.RC5:
# new:     Added support for fixing (potential) localtime problems.
# v2.0.RC3:
# fix:     Script checked ip's within commented lines in resolv.conf.
# change:  The routine to create device nodes have changed.
# v2.0.RC2:
# new:     Now copies resolv.conf into glftpd's etc/ dir (so bnc's etc work).
# change:  SSL-cert is now valid for 1024 days (was 365).
# change:  Renamed glftpd.conf in the dest. dir to glftpd.conf.dist.
# new:     Added output on compiliation of sources.
# fix:     Fixed warning on the osx/darwin platform.
# fix:     Cleanup and optimizing.
# fix:     The killghost bin was compiled with the wrong IPC-key.
# fix:     The routine to copy dynamic libs on OpenBSD v3.3 (and below) did
#          not work. Not sure if this is important, as the interpreter changed
#          from a.out to elf as well in 3.4 (ie, glftpd won't work anyway).
# fix:     Did not report if a file failed to compile.
# new:     Added support for european weeks for the reset binary.
# new:     Makes sure groupdir/files are chmod'ed ok.
# change:  Dynamic libs are now copied to a mirrored dir-layout instead of
#          glroot's default /lib dir.
# change:  Changed the version number to match the version of glftpd.
# fix:     TCPD under fbsd/nbsd was used either way. fixed. (iwdisb)
# fix:     Some sources under bin/sources was modified a bit. That lead to
#          a minor change in the installer.
# fix:     GCC was hardcoded as the compiler used. Now uses either CC or GCC.
# change:  Now compiles bins with "-Wall -O2 -o" if GCC is the compiler
#          used. (If it fails it will try again with just "-o".)
# change:  When creating a ssl cert, the servicename is used as servbase if
#          nothing else is specified.
# new:     Added "sections" to the installer.
# v1.2.4:
# fix:     Redid the ldd sequence which reads dynamic libs - should fix
#          warnings on fedora core 2 and others.
# fix:     If you installed multiple instances of glftpd, the crontab entry
#          was overwritten.
# new:     L33T banner!
# fix:     iwdisb fixed lots of bugs (mainly for *bsd) in the jail part.
# new:     initial (experimental) support for OS-X (darwin).
# new:     initial (experimental) support for NetBSD.
# fix:     creation of /dev/null & /dev/zero on *bsd didn't work.
# change:  now shows progressbar when creating a cert, and gives some debug
#          info in installgl.debug if the cert creation fails.
# v1.2.3:
# fix:     Fixed a bug affecting FreeBSD 5.0 and 5.1 owners (elf interpreter
#          did not change dir until 5.2).
# new:     Added "real" support for multiple instances of glftpd.
# fix:     Fixed a minor bug when openssl is not found.
# v1.2.2:
# new:     Support for OpenBSD.
# new:     Support for FreeBSD 5.x. (and 4.x)
# new:     Support for multiple instances of glftpd.
# fix:     Fixed a bug in the creation of jailgroup under FreeBSD.
# change:  If bash does not exist on system, the script will now cry for help.
# v1.2.1:
# change:  Changed the text for TLS certificate creation since its no longer
#          required for the installation.
# new:     Copy egrep to the rest of the binaries as well.
# new:     Added www.glftpd.at to recomended sites =)
# v1.2:
# new:     Included installation support for TLS / Turranius.
#
# v1.14:
# fix:     There was a bug in adding the proper -s string for glstrings if
#          you didn't use the default install dir of /glftpd.  This has now
#          been fixed.
# v1.13:
# fix:     there was the possibity of your xinetd.conf file getting erased.
#          Code was changed to first verify if the /etc/xinetd.d directory exists,
#          if not, it will be created and the includedir line added to your
#          xinetd.conf
# v1.12
# change:  updated a few warning messages.
# new:     you will now be told if the private group you are trying to use
#          already exists and options to change it.
# change:  if your system contains both inetd and xinetd config files, you
#          will be asked what you want to use.  If only one can be found, then
#          that will be used automatically.
# v1.11
# fix:     changed the code when restarting xinetd, it was incompatible with
#          some systems.
# change:  will now look at /etc/xinetd.conf and verify you have set the
#          includedir for xinetd.d, if so a separate glftpd file will
#          be created, else the information will be added to /etc/xinetd.conf
# v1.10
# change:  added some more binaries to the required binaries list, mainly
#          to make Bloody_A happy.
# fix:     script will now look for a compiler to user before compiling
#          the sources, you will get an error message if one can not be
#          found.
# v1.09
# fix:     some more bugs in the code.
# v1.08
# fix:     some of my own grammar mistakes.
# change:  update a lot of the code to improve things, thanks to our good
#          friend, Jehsom.
# v1.07 -  dn
# new:     will now ask if you want to run glftpd in a jailed environment and
#          set it all up accordingly.
# change:  the output when copying the needed binaries or compiling the source
#          files.  Now it just tells you which have failed, or if all were
#          successful
# fix:     Now the glroot path and the jaildir path must start with a /
# change:  Instead of exiting if the glroot path or jaildir path is a /,
#          you will now get to enter a new dir name
# fix:     Crontab entries will be removed on a reinstall so you don't get
#          duplicates
# new/fix: will now configure glftpd properly for use with or without tcpd
# fix:     there was a bug in the [T]ry Again option when creating the install
#          dir for glftpd
# change:  updated the final message to include jehsom's new website
#          information and added mine. :)
# fix:     some spelling mistakes (has Archimede been in here?)
# change:  removed -r /etc/glftpd.conf and -n 1 from the inetd and xinetd lines as
#          they are they default.
# change:  remove -r /etc/glftpd.conf from the reset line in crontab as that is the
#          default
# v1.06 -  maestro^
# new:     permissions set on ftp-data/users
# change:  glftpd.conf now moved to glftpd.conf.dist after install
# fix:     not properly reinstalling on top of itself from the glroot dir
# fix:     some error messages werent printing properly

#DO NOT TOUCH ANYTHING BELOW THIS LINE
#--------------------------------------------------------------------------------------

# Ensure we have all useful paths in our $PATH
PATH="$PATH:/bin:/usr/bin:/usr/local/bin:/sbin:/usr/sbin:/usr/local/sbin:\
/usr/libexec:/usr/compat/linux/bin:/usr/compat/linux/sbin:/usr/compat/linux/usr/bin:\
/usr/compat/linux/usr/sbin"

# Run the script with bash
[ -z "$bashbin" ] && {
  searchbin="bash"
  unset binname; for p in `echo $PATH | tr -s ':' '\n'`; do test -x $p/$searchbin && { binname=$p/$searchbin; break; }; done;

  [ ! -z "$binname" ] && {
    bashbin=$binname
    export bashbin
    exec $bashbin -x "$0" "$@" 2>./installgl.debug
  } || {
    echo "HELP!! NEED BASH!"
    exit 1
  }
}

# Make sure the 'bash' we have is the real deal.
[ -z "`exec $bashbin --version | grep -i "gnu bash"`" ] && {
  echo "HELP!! NEED REAL BASH - NOT A FAKE ONE!"
  exit 1
}


# Set null glob options
allow_null_glob_expansion=1
shopt -s nullglob

# Be sure we're root
[ "$UID" = "0" ] || {
    echo "You must be root to install glftpd."
    exit 1
}

# Make sure we're in the right place
cd "$(dirname $0)"
[ -f "bin/glftpd" ] || {
    echo "Please run $(basename $0) from the glftpd installation dir."
    exit 1
}

# homemade 'which' command by js
mywhich() {
  unset binname; for p in `echo $PATH | tr -s ':' '\n'`; do test -x $p/$searchbin && { binname=$p/$searchbin; break; }; done;
}

searchbin="which"
mywhich
mywhichbin=$binname

# Set system type
case $(uname -s) in
    Linux) os=linux ;;
    *[oO][pP][eE][nN][bB][sS][dD]*) os=openbsd ;;
    *[Dd][Aa][Rr][Ww][Ii][Nn]*) os=darwin ;;
    *[Nn][Ee][Tt][Bb][Ss][Dd]*) os=netbsd ;;
    *[fF][rR][eE][eE][bB][sS][dD]*)
        bsdversionmaj=`uname -r | cut -d'.' -f1`
        bsdversionmin=`uname -r | cut -d'-' -f1 | cut -d'.' -f2 | tr -cd '0-9'`
        if [ $bsdversionmaj -ge 6 ]; then
           os=freebsd6
        elif [ $bsdversionmaj -eq 5 ] && [ $bsdversionmin -ge 2 ]; then
           os=freebsd5
        else
           os=freebsd4
        fi
        ;;
    *)
        echo "Sorry, but this installer does not support the $(uname -s) platform."
        echo "You will have to install glftpd manually."
        exit 1
        ;;
esac

if [ ! -f /etc/inetd.conf -a ! -f /etc/xinetd.conf -a ! -f /etc/rlinetd.conf -a ! -d /etc/systemd/system -a "$os" != "darwin" ]; then
  echo "You do not have xinetd, systemd, rlinetd or (openbsd-)inetd installed on your system."
  echo "Please install and start one of them, then re-run this installer."
  exit 1
fi

# Important bins to have in the glftpd bin dir
BINS="sh cat grep egrep unzip wc find ls bash mkdir rmdir rm mv cp awk ln basename dirname head tail cut tr wc sed date sleep touch gzip zip"
if [ "$os" = "darwin" ]; then
  BINS="$BINS tcsh"
else
  BINS="$BINS ldconfig"
fi


clear 2>/dev/null
echo -e "\n\033[1m             ###  #     ##### ##### ####  ####        ###   \033[0m"
echo -e "\033[1m            #   # #     #       #   #   # #   #      #   #  \033[0m"
echo -e "\033[1m            #     #     ###     #   #   # #   #         #   \033[0m"
echo -e "\033[1m            #  ## #     #       #   ####  #   #        #    \033[0m"
echo -e "\033[1m            #   # #     #       #   #     #   #       #     \033[0m"
echo -e "\033[1m             ###  ##### #       #   #     ####       #####  \033[0m\n"
echo -e "\n\033[1m                  --== WE MAKE FILES TRANSFER ==--\033[0m\n\n"
echo "     -----------------------------------------------------------"
echo -e "\033[1m                  GLFTPD INSTALLER v$VERSION ($os)\033[0m\n"
echo "          Originally done by jehsom and dn."
echo "          Made ready for the new era by turranius and psxc.  "
echo "     -----------------------------------------------------------"
echo ""

echo ""
echo "Before we begin: If this installer fails on your system, please"
echo "let the devs know. You find us on irc (efnet) in #glftpd. Thank you."
echo "Also, any bugs found in glftpd itself should be reported either to"
echo "the board @ http://www.glftpd.eu, in the irc channel, or both."
echo ""
echo -n "Press <enter> to continue. "
read randomkey

echo -e "\n\n\033[1m1. TCPD SETUP:\033[0m\n\033[1m--------------\033[0m\n"

case $os in
  freebsd*)
    echo "To enable tcpd on FreeBSD, inetd should be initialized with"
    echo "the 'w' flag. As this is the default setting, it's probably"
    echo "set already. This installer will skip the manual setup of tcpd."
    echo ""
    ;;
  netbsd)
    echo "NetBSD has tcpd built into the inetd daemon. This installer"
    echo "will skip the manual setup of tcpd."
    echo ""
    ;;
  *)

  echo "Do you wish to use tcpd? If you are not sure then you should not"
  echo "use it. If you decided to change this at a later time, please"
  echo "search for tcpd in glftpd.docs for the required changes."
  echo ""

#-----------
until [ -n "$usetcpd" ]; do
    if [ "$os" == "darwin" ]; then
        echo -n "Checking wether you have installed a compiler..."
        /usr/bin/xcode-select -p &>/dev/null || {
            echo "NO."
            echo "Chose install then start installer again."
            /usr/bin/xcode-select --install
            exit 0
        }
        echo "Compiler is installed. Macos deprecated tcpd since 10.7."
        unset usetcpd
        break
    fi
    echo -n "Use tcpd? [Y]es [N]o: "
    read usetcpd
    case $usetcpd in
        [Nn])
            echo "Installing glftpd without tcpd."
            usetcpd="n"
        ;;
        [Yy])
            [ ! -z "$mywhichbin" ] && {
              tcpd="$(which tcpd | grep "^/")"
            } || {
              searchbin="tcpd"
              mywhich
              tcpd=$binname
            }
            [ -z "$tcpd" ] && {
                echo ""
                echo "It seems you don't have tcpd installed. Either exit this"
                echo "installer, install tcpd and try again, or choose not to"
                echo "use it."
                unset usetcpd
                continue
            }
            echo "Installing glftpd with tcpd."
            usetcpd="y"
        ;;
        *)
            unset usetcpd
            continue
        ;;
    esac
done
#-----------
  ;;
esac

echo -e "\n\033[1m2. JAIL SETUP:\033[0m\n\033[1m--------------\033[0m\n"

# openbsd/netbsd do not have jail ability, so let's just skip this part.

case $os in
    openbsd)
         echo "Jail is not supported on this platform. Skipping this step."
         echo ""
         ;;
    netbsd)
         echo "Jail is not supported on this platform. Skipping this step."
         echo ""
         ;;
    *)

#-----------
echo "Do you want to run glftpd in a \"Jailed\" environment?  In this"
echo "environment a private directory will be created and glftpd will"
echo "be installed inside.  Regular shell users will not be able to get"
echo "inside this private directory.  The glftpd.conf is also moved"
echo "inside for added security and a new group will be created so"
echo "you and other users you specify can access glftpd through the shell."
case $os in
  *bsd*)
    echo "(FYI: This is the linux definition of jail, not FreeBSD's ... ;)"
    ;;
esac
echo ""

until [ -n "$jail" ]; do
    echo -n "Use a jailed environment? [Y]es [N]o: "
    read jail
    case $jail in
        [Nn])
            jail="n"
        ;;
        [Yy])
            jail="y"
            echo ""
            echo "Creating the jailed environment."
            echo ""
            unset jaildir
            until [ -n "$jaildir" ]; do
                echo -n "Please enter the private directory to install glftpd inside [/jail]: "
                read jaildir
                case $jaildir in
                    /)
                        echo "You can't have / as your private dir!  Try again."
                        echo ""
                        unset jaildir
                        continue
                    ;;
                    /*|"")
                        [ -z "$jaildir" ] && jaildir="/jail"
                        [ -d "$jaildir" ] && {
                            echo -n "Path already exists. [D]elete it, [A]bort, [T]ry again, [I]gnore? "
                            read reply
                            case $reply in
                                [dD]*) rm -rf "$jaildir" ;;
                                [tT]*) unset jaildir; continue ;;
                                [iI]*) ;;
                                *) echo "Aborting."; exit 1 ;;
                            esac
                        }
                        mkdir -p "$jaildir"
                        continue
                    ;;
                    *)
                        echo "The private directory must start with a \"/\".  Try again."
                        echo ""
                        unset jaildir
                        continue
                    ;;
                esac
            done
        ;;
    esac
done

[ "$jail" = "y" ] && {
    echo ""
    echo "Do you want to create a private group?  If you say no then only root will"
    echo "be able to access glftpd.  Otherwise you can add other shell users to the"
    echo "group so they can access glftpd from the shell."
    echo ""

    until [ -n "$useprivgroup" ]; do
        echo -n "Use a private group? [Y]es [N]o: "
        read useprivgroup
        case $useprivgroup in
            [Nn])
                useprivgroup="n"
                echo -n "Setting permissions on $jaildir . . . "
                chmod 700 "$jaildir"
                echo "Done."
                echo ""
            ;;
            [Yy])
                useprivgroup="y"
                case $os in
                    freebsd*)
                        groupcmd="pw group"
                    ;;
                    darwin)
                        echo "On Macos the private group for the jail will be glftpd with GID 1337"
                        echo "And $(logname) will be added to that group"
                        /usr/bin/dscl /Local/Default -create /Groups/glftpd
                        /usr/bin/dscl /Local/Default -create /Groups/glftpd PrimaryGroupID 1337
                        /usr/bin/dscl /Local/Default -create /Groups/glftpd Password \*
                        /usr/sbin/dseditgroup -o edit -a "$(logname)" -t user glftpd
                        echo "Using group glftpd.  The following user has access: $(logname)"
                        echo -n "Setting Permissions on $jaildir . . . "
                        chmod 750 "$jaildir"
                        chown root:"glftpd" "$jaildir"
                        echo "Done."
                        echo ""
                    ;;
                    *)
                        groupcmd="group"
                    ;;
                esac
                until [ -n "$privgroup" ]; do
                    echo -n "What would you like your private group to be called? [glftpd]: "
                    read privgroup
                    [ -z "$privgroup" ] && privgroup="glftpd"
                    makegroup="1"
                    [ -n "$(grep $privgroup /etc/group)" ] && {
                        groupusers="$(grep ^$privgroup /etc/group | cut -d ':' -f4)"
                        [ -z "$groupusers" ] && groupusers="No users previously added"
                        echo -e '\033[1;31mWarning\033[0m:' Group already exists with the following users: $groupusers
                        echo ""
                        echo -n "What would you like to do: [D]elete it, [A]bort, [T]ry again, [I]gnore? "
                        read reply
                        case $reply in
                            [dD]*) ${groupcmd}del $privgroup; ;;
                            [tT]*) unset privgroup; continue ;;
                            [iI]*) makegroup="0";;
                             *) echo "Aborting."; exit 1 ;;
                        esac
                    }
                done

                if [ "$makegroup" -eq "1" ]; then
                    echo -n "Creating private group . . . "
                    ${groupcmd}add $privgroup

                    if [ $? -eq 0 ]; then
                        echo "Done."
                        echo ""
                        echo -n "Who should have access to glftpd? (separate with ,): "
                        read jailusers
                        if [ -z "$jailusers" ]; then
                            echo -e '\033[1;31mWarning\033[0m:' No users added, only root can access glftpd.
                        else
                            oIFS=$IFS
                            IFS=,
                            for user in $jailusers; do
                            grep "^$user:" /etc/passwd > /dev/null || {
                                echo -e '\033[1;31mWarning\033[0m:' user $user does not exist.  Adding anyway.
                            }
                            done
                            IFS=$oIFS
                            sed "s/^$privgroup:.*/&$jailusers/" /etc/group > /etc/group.new
                            mv -f /etc/group.new /etc/group
                        fi
                        echo -n "Setting permissions on $jaildir . . . "
                        chmod 750 "$jaildir"
                        chown root:"$privgroup" "$jaildir"
                        echo "Done."
                        echo ""
                    else
                        echo -e '\033[1;31mFailed!\033[0m'
                        echo ""
                        echo "The private group could not be created."
                        echo "Please create this group manually and add yourself and any other shell"
                        echo "users to the group who should have access to glftpd.  You will also"
                        echo "need to chown root:$privgroup $jaildir and chmod 750 $jaildir to set the"
                        echo "proper permissions.  If this is not done, only root can access glftpd."
                        echo ""
                        echo -n "Setting permissions on $jaildir . . . "
                        chmod 700 "$jaildir"
                        echo "Done."
                        echo ""
                    fi
                else
                    echo "Using group \"$privgroup\".  The following users have access: $groupusers"
                    echo -n "Setting Permissions on $jaildir . . . "
                    chmod 750 "$jaildir"
                    chown root:"$privgroup" "$jaildir"
                    echo "Done."
                    echo ""
                fi
            ;;
            *)
                unset useprivgroup
                continue
            ;;
        esac
    done
}

#---------------
    ;;
esac

echo -e "\n\n\033[1m3. GLFTPD BASE SETUP:\033[0m\n\033[1m---------------------\033[0m\n"

# Get a valid installation path
until [ -n "$glroot" ]; do
    if [ "$jail" = "y" ]; then
        echo -n "Please enter the directory inside $jaildir to install glftpd to [/glftpd]: "
    else
        echo -n "Please enter the directory to install glftpd to [/glftpd]: "
    fi
    read glroot
    case $glroot in
        /)
            echo "You can't have / as your private dir!  Try again."
            echo ""
            unset glroot
            continue
        ;;
        /*|"")
            [ -z "$glroot" ] && glroot="/glftpd"
            [ -d "$jaildir$glroot" ] && {
                echo -n "Path already exists. [D]elete it, [A]bort, [T]ry again, [I]gnore? "
                read reply
                case $reply in
                    [dD]*) rm -rf "$jaildir$glroot" ;;
                    [tT]*) unset glroot; continue ;;
                    [iI]*) ;;
                    *) echo "Aborting."; exit 1 ;;
                esac
            }
            mkdir -p "$jaildir$glroot" || unset glroot; continue
        ;;
        *)
            echo "The private directory must start with a \"/\".  Try again."
            echo ""
            unset glroot
            continue
        ;;
    esac
done

echo -n "Copying glftpd files to $jaildir$glroot . . . "

cp -Rf * .[^.]* "$jaildir$glroot"
## manually make emptydirs
mkdir -m755 "$jaildir$glroot/"{dev,site,ftp-data/msgs}
echo "Done."

if [ "$jail" = "y" ]; then
    confdir="$jaildir"
else
    confdir="/etc"
fi

echo -e "\nCopying required binaries to $jaildir$glroot/bin:"
for bin in $BINS; do
    type $bin > /dev/null || {
        failed="$failed   Failed to copy: \033[1;31m$bin\033[0m\n"
        continue
    }
    [ ! -z "$mywhichbin" ] && {
      cp -f "$(which $bin)" "$jaildir$glroot/bin"
    } || {
      searchbin="$bin"
      mywhich
      cp -f "$binname" "$jaildir$glroot/bin"
    }

done

if [ -n "$failed" ]; then
    echo -e "$failed"
    echo "Please find and copy these manually to $jaildir$glroot/bin"
else
    echo "All binaries successfully copied."
fi

echo -ne "\nMaking glftpd's /dev/null , /dev/full , /dev/zero & /dev/urandom . . . "
if [ ! "$os" = "freebsd6" ]; then
  case $os in
    linux)     nods="null-1-3 full-1-7 zero-1-5 urandom-1-9" ;;
    *freebsd*) nods="null-2-2 full-2-3 zero-2-12 urandom-2-4" ;;
    *openbsd*) nods="null-2-2 zero-2-12 urandom-45-2" ;;
    *netbsd*)  nods="null-2-2 zero-2-12 urandom-46-1" ;;
    darwin)    nods="null-3-2 zero-3-3 urandom-7-1" ;;
  esac

  for nod in $nods; do
    nodname=`echo $nod | cut -d '-' -f 1-1`
    noda=`echo $nod | cut -d '-' -f 2-2`
    nodb=`echo $nod | cut -d '-' -f 3-3`
    mknod "$jaildir$glroot/dev/$nodname" c $noda $nodb
    chmod 666 "$jaildir$glroot/dev/$nodname"
  done
else
  echo "STOPPED."
  echo "FreeBSD6.x does no longer support mknod - to make the neccessary"
  echo "changes we have to add a couple of lines in your /etc/rc.conf,"
  echo "/etc/fstab and /etc/devfs.rules."
  echo -n "Add changes? [y/N]> "
  read keystroke
  case $keystroke in
  [Yy])
    echo -n "Adding the necessary lines . . . "
    echo "devfs_set_rulesets=\"$jaildir$glroot/dev/=devfsrules_$servicename\"" >>/etc/rc.conf
    echo "none $jaildir$glroot/dev devfs rw 0 0" >>/etc/fstab
    echo "#glftpd ruleset start" >>/etc/devfs.rules
    echo "[devfsrules_$servicename=$RANDOM]" >>/etc/devfs.rules
    echo "add include \$devfsrules_hide_all" >>/etc/devfs.rules
    echo "add include \$devfsrules_unhide_basic" >>/etc/devfs.rules
    echo "#glftpd ruleset end" >>/etc/devfs.rules
    echo "Done."
    echo -n "Applying changes . . . "
    if [ $bsdversion -ge 70 ]; then mount -t devfs devfs $jaildir$glroot/dev
    else mount_devfs devfs $jaildir$glroot/dev
    fi
    /etc/rc.d/devfs start
    ;;
  *)
    echo "Not changing config files - you must do this manually."
    echo -n "Devicenode-changes not "
    ;;
  esac
fi
echo "Done."
echo ""


echo -e "\n\033[1m4. SERVICE SETUP & MULTI-INSTALL:\033[0m\n\033[1m---------------------------------\033[0m\n"

servicedef="`basename ${glroot}`"
servicename=""
while [ -z "$servicename" ]; do
    echo "Enter a service name for glftpd. This name will be used as the"
    echo "service name mapped to the port in /etc/services, the name"
    echo "used in your (x)inetd settings, and the name of your config-file."
    echo -e "\033[1mNOTE:\033[0m If you (wish to) have multiple instances of glftpd on the"
    echo "same box, you *must* to change this."
    echo -n "Press <enter> for the default (${servicedef})> "
    read servicename
    if [ -z "$servicename" ]; then
        servicename="${servicedef}"
    else
        if [ ! -z "`echo "$servicename" | tr -d '[a-zA-Z0-9]'`" ]; then
            echo "Invalid chars in name. Please use numbers and letters only."
            echo ""
            servicename=""
        fi
    fi
    if [ ! -z "$servicename" ]; then
        servicename="`echo $servicename | tr 'A-Z' 'a-z'`"
        if [ ! -z "$(grep -e "^${servicename}$" /etc/services)" ]; then
            echo "The service \"$servicename\" already exists in /etc/services."
            echo -n "Replace it? (y/n)> "
            read keystroke
            keystroke="`echo $keystroke | cut -b 1-1 | tr 'A-Z' 'a-z'`"
            if [ ! "$keystroke" = "y" ]; then
                servicename=""
            fi
        fi
    fi
done

if [ ! "${servicename}" = "glftpd" ]; then
  keystroke=""
  ipckey="0x0000DEAD"
  echo ""
  echo "You did not choose the default servicename for glftpd. If you plan on"
  echo "having more than one instance of glftpd, you should change the ipc_key."
  echo "The ipc_key can be anything you want, but it must be unique. To make"
  echo "things easy in this installer, you will be provided with 10 choices."
  while [ "$ipckey" = "0x0000DEAD" ]; do
    echo -e "\033[1m1\033[0m: 0x0000BABE   \033[1m2\033[0m: 0xDEADBABE   \033[1m3\033[0m: 0x00C0FFEE  \033[1m4\033[0m: 0x12345678"
    echo -e "\033[1m5\033[0m: 0x87654321   \033[1m6\033[0m: 0xBEEFBABE   \033[1m7\033[0m: 0xBADCOFEE  \033[1m8\033[0m: 0x0000BEEF"
    echo -e "\033[1m9\033[0m: 0xDEADBEEF   \033[1m0\033[0m: 0x0000DEAD (default)"
    echo -n "Choose a number (0-9) > "
    read keystroke
    case $keystroke in
      1) ipckey=0x0000BABE ;;
      2) ipckey=0xDEADBABE ;;
      3) ipckey=0x00C0FFEE ;;
      4) ipckey=0x12345678 ;;
      5) ipckey=0x87654321 ;;
      6) ipckey=0xBEEFBABE ;;
      7) ipckey=0xBADC0FEE ;;
      8) ipckey=0x0000BEEF ;;
      9) ipckey=0xDEADBEEF ;;
      0) ipckey="" ;;
    esac
  done
fi


echo -e "\n\n\033[1m5. COMPILING SOURCES & COPYING LIBS:\033[0m\n\033[1m------------------------------------\033[0m\n"

# modify source(s) - change the default location of glftpd.conf
confsource="$jaildir$glroot/bin/sources/glconf.h"
echo -n "modifying source ($confsource) ... "
echo "#define GLCONF  \"$confdir/${servicename}.conf\"" >${confsource}.tmp && \
grep -v "^#define GLCONF" ${confsource} >> ${confsource}.tmp && \
mv ${confsource}.tmp ${confsource}
[ $? -eq 0 ] && echo "OK." || echo -e "\033[1;31mFailed!\033[0m"

echo "Compiling source files in $jaildir$glroot/bin/sources to $jaildir$glroot/bin:"
[ ! -z "$mywhichbin" ] && {
  usegcc="$(which gcc)"
} || {
  searchbin="gcc"
  mywhich
  usegcc=$binname
}
gccflags="-O2 -Wall -o"
if [ -z "$usegcc" ]; then
  [ ! -z "$mywhichbin" ] && {
    usegcc="$(which cc)"
  } || {
    searchbin="cc"
    mywhich
    usegcc=$binname
  }
  gccflags="-o"
fi
if [ -z "$usegcc" ]; then
   echo "Error: Can't find a compiler to use.  Please install one and compile all"
   echo "       the scripts in $jaildir$glroot/bin/sources and place them in"
   echo "       $jaildir$glroot/bin.  Note that these files are not necessary for"
   echo "       glftpd to operate, they are extra external scripts."
fi
[ -n "$usegcc" ] && {
    for cfile in $jaildir$glroot/bin/sources/*.c; do
        base="$(basename "${cfile%.c}")"
        [ -f "$jaildir$glroot/bin/$base" ] && rm -f "$jaildir$glroot/bin/$base"
        echo -n "   $base .. "
        $usegcc $gccflags "$jaildir$glroot/bin/$base" "$cfile" >/dev/null 2>&1 || \
          $usegcc -o "$jaildir$glroot/bin/$base" "$cfile" >/dev/null 2>&1
        if [ $? -eq 0 ]; then
            echo "OK."
        else
            failedc="$failedc   Failed to compile: \033[1;31m$cfile\033[0m\n"
            echo "FAILED!"
        fi
    done
    if [ -z "$failedc" ]; then
        echo -e "All source files successfully compiled."
    else
        echo -e "$failedc"
    fi
}

lddsequence() {
    echo -n "   $(basename $lib): "
    libdir="$(dirname $lib)"
    if [ -f "$lib" ]; then
       mkdir -p "$jaildir$glroot$libdir"
       cp -f "$lib" "$jaildir$glroot$libdir"
       echo "$libdir" >> "$jaildir$glroot/etc/ld.so.conf"
       echo "OK"
    elif [ -f "/usr/compat/linux/$lib" ]; then
       mkdir -p "$jaildir$glroot/usr/compat/linux"
       cp -f "/usr/compat/linux/$lib" "$jaildir$glroot/usr/compat/linux/"
       echo "$libdir" >> "$jaildir$glroot/etc/ld.so.conf"
       echo "OK"
    else
       echo -e '\033[1;31mFailed!\033[0m'" You must find & copy $(basename $lib) to $jaildir$glroot$libdir manually."
    fi
}

echo -e "\nCopying required shared library files:"
echo -n "" > "$jaildir$glroot/etc/ld.so.conf"
case $os in
    openbsd)
      openrel=`uname -r | tr -cd '0-9' | cut -b 1-2`
      if [ $openrel -ge 34 ]; then
        ldd $jaildir$glroot/bin/* 2>/dev/null | awk '{print $5, $1}' | grep -e "^/" | grep -v "00000000$" | awk '{print $1}' |
        sort | uniq | while read lib; do
            lddsequence
        done
      else
        ldd $jaildir$glroot/bin/* 2>/dev/null | awk '{print $3, $4}' | grep -e "^/" | grep -v "00000000)$" | awk '{print $1}' |
        sort | uniq | while read lib; do
            lddsequence
        done
      fi
    ;;
    darwin)
        otool -L $jaildir$glroot/bin/* 2>/dev/null | awk '{print $1}' | grep -e "^/" | grep -v "$jaildir$glroot/bin/" |
        sort | uniq | while read lib; do
            lddsequence
        done
    ;;
    *)
        bindir="`echo $jaildir$glroot/bin | tr -s '/'`"
        ldd $bindir/* 2>/dev/null | grep -v "^$bindir" | tr ' \t' '\n' | grep -e "^/" |
        sort | uniq | while read lib; do
            lddsequence
        done
esac

case $os in darwin|*bsd*|linux)
    echo -e "\nCopying your system's run-time library linker(s):"
    echo "(NOTE: Searches can take a couple of minutes, please be patient.)"
    case $os in
        freebsd4)
            bsdlibs="/usr/libexec/ld-elf.so.1"
            ;;
        freebsd5)
            bsdlibs="/libexec/ld-elf.so.1"
            ;;
        freebsd6)
            bsdlibs="/libexec/ld-elf.so.1"
            ;;
        openbsd)
            bsdlibs="/usr/libexec/ld.so"
            ;;
        netbsd)
            bsdlibs="/usr/libexec/ld.so /usr/libexec/ld.elf_so"
            ;;
        darwin)
            bsdlibs="/usr/lib/dyld /usr/lib/dylib1.o /usr/lib/system/libmathCommon.A.dylib"
            ;;
        linux)
            bsdlibs=""
            ;;
        esac
        libfailed=0
        for bsdlib in $bsdlibs; do
            bsdlibdir=${bsdlib%/*}
            mkdir -p "$jaildir$glroot$bsdlibdir"
            echo -n "   $(basename $bsdlib): "
            if [ -e "$bsdlib" ]; then
                cp -f "$bsdlib" "$jaildir$glroot$bsdlibdir"
                echo "OK"
            else
                echo -n "Searching . . . "
                file=$(find / -name $(basename $bsdlib) | head -1)
                if [ -n "$file" ]; then
                    cp -f "$file" "$jaildir$glroot$bsdlibdir"
                    echo "OK"
                else
                    echo -e '\033[1;31mFailed!\033[0m'
                    libfailed="1"
                fi
            fi
        done
        [ $libfailed -eq 1 ] && echo "You must install and copy the missing libraries to $jaildir$glroot$bsdlibdir manually."
    ;;
esac

echo -ne "\nConfiguring the shared library cache . . . "
sort "$jaildir$glroot/etc/ld.so.conf" | uniq >"$jaildir$glroot/etc/ld.so.temp" && mv "$jaildir$glroot/etc/ld.so.temp" "$jaildir$glroot/etc/ld.so.conf"
lddlist="`cat $jaildir$glroot/etc/ld.so.conf | tr '\n' ' '`"
case $os in
    linux)
        chroot "$jaildir$glroot" /bin/ldconfig
        ;;
    *bsd*)
        mkdir -p "$jaildir$glroot/usr/lib"
        mkdir -p "$jaildir$glroot/var/run"
        chroot "$jaildir$glroot" /bin/ldconfig $lddlist
        ;;
    esac
echo "Done."

echo -e "\n\n\033[1m6. PORT AND SYSTEM SETUP:\033[0m\n\033[1m-------------------------\033[0m\n"

until echo "$port" | grep -E "^[0-9]+$" > /dev/null && [ "$port" -lt 65535 ]; do
    echo -en "Enter the port you would like glftpd to listen on [1337]: "
    read port
    [ -z "$port" ] && port=1337
done

echo -ne "Setting userfile permissions . . . "
chmod 755 "$jaildir$glroot/ftp-data/users"
chmod 644 "$jaildir$glroot/ftp-data/users/"*
echo "Done."
echo -ne "Setting groupfile permissions . . . "
chmod 755 "$jaildir$glroot/ftp-data/groups"
chmod 644 "$jaildir$glroot/ftp-data/groups/"*
echo "Done."

echo ""
echo -n "Adding glftpd service to /etc/services (as ${servicename}) . . . "

{ grep -v ^${servicename} /etc/services;
  echo "$servicename   $port/tcp"
} > /etc/services.new
mv -f /etc/services.new /etc/services
echo "Done."

echo -n "Copying glftpd.conf to $confdir/${servicename}.conf . . . "

if [ -f glftpd.conf ]; then
    conffile="glftpd.conf"
elif [ -f glftpd.conf.dist ]; then
    conffile="glftpd.conf.dist"
else
    echo -e "\033[1;31mFailed!\033[0m"
    echo "You must find glftpd.conf and copy it to $confdir manually"
    echo "and change the rootpath to $confdir$glroot"
fi

# rename glftpd.conf in the glftpd dir (so people don't start editing it)
if [ -f $jaildir$glroot/glftpd.conf ]; then
  mv "$jaildir$glroot/glftpd.conf" "$jaildir$glroot/glftpd.conf.dist"
fi

[ -n "$conffile" ] && {
    [ -f "$confdir/${servicename}.conf" ] && {
        echo -en "Found old conf\nBacking up up old ${servicename}.conf first . . . "
        mv "$confdir/${servicename}.conf" "$confdir/${servicename}.conf.bak" &&
          echo -en "Done.\nCopying new glftpd.conf to $confdir/${servicename}.conf . . . "
    }
    sed "s:^\([         ]*rootpath[     ]*\).*$:\1$jaildir$glroot:" $conffile > "$confdir/${servicename}.conf" &&
      echo "Done."
    if [ ! -z "$ipckey" ]; then
      echo -e "\n#ipc_key - please read docs on this one\nipc_key         $ipckey" >> "$confdir/${servicename}.conf"
      [ $? -gt 0 ] && echo -e "\033[1;31mFailed!\033[0m to copy ip_key to your conf. You need to do this manually."
    fi
    [ -f glftpd.conf ] && mv glftpd.conf glftpd.conf.dist
#    echo "Done."
}
echo ""


# Configure inetd or xinetd or rlinetd as appropriate
[ "$jail" = "y" -o "$servicename" != "glftpd" ] && confpath="-r $confdir/${servicename}.conf"

[ -f /etc/inetd.conf ] && SUPERSERVERS="${SUPERSERVERS} inetd"
[ -f /etc/xinetd.conf ] && SUPERSERVERS="${SUPERSERVERS} xinetd"
[ -f /etc/rlinetd.conf ] && SUPERSERVERS="${SUPERSERVERS} rlinetd"
[ -d /etc/systemd/system ] && SUPERSERVERS="${SUPERSERVERS} systemd"
[ "${os}" = "darwin" ] && SUPERSERVERS="${SUPERSERVERS} launchd"
COUNT_SUPERSERVERS=( $SUPERSERVERS )
if [ ${#COUNT_SUPERSERVERS[@]} -eq 0 ]; then
    echo "ERROR: Can't determine if you are using (openbsd-)inetd, rlinetd, xinetd or launchd!"
    echo "This is usually because neither is running at the moment -"
    echo "install/start one of them and re-run this installer."
    exit 1
elif [ ${#COUNT_SUPERSERVERS[@]} -eq 1 ]; then
    netd="${SUPERSERVERS:1:1}";
else
    unset whichnetd
    for superserver in ${SUPERSERVERS}; do
        s_options="${s_options}${superserver:0:1}";
    done
    until [ -n "$whichnetd" ]; do
        for superserver in ${SUPERSERVERS}; do
            printf "[\033[1m${superserver:0:1}\033[0m]${superserver:1}\n"
        done
        echo -n "choose superserver: "
        read whichnetd
        whichnetd=$(echo ${whichnetd}|tr '[:upper:]' '[:lower:]')
        case "${s_options}" in
            *${whichnetd}*) netd=${whichnetd} ;;
            *) echo "pick one!"; unset whichnetd; continue ;;
        esac
    done
fi

# Only a dummyflag needed when not using tcpd and using inetd or rlinetd
[ ! "$usetcpd" = "y" ] && ( [ "$netd" = "i" ] || [ "$netd" = "r" ] ) && dummyflag="glftpd"

## Setup path to glstrings.bin if we're in a jailed enviroment.
if [ -n "$jaildir" ]; then
    glstrings="-s$jaildir$glroot/bin/glstrings.bin"
elif [ "$glroot" != "/glftpd" ]; then
    glstrings="-s$glroot/bin/glstrings.bin"
fi

## Ask if european week is used
unset keystroke
until [ -n "$keystroke" ]; do
   echo "Do you wish to use European weeks? European weeks starts with a Monday."
   echo -n "This is for glftpd's 'reset' binary (see docs for more info) [Y/N]: "
   read keystroke
   case $keystroke in
       [Yy]) euroweek="-e " ;;
       [Nn]) euroweek="" ;;
       *) unset keystroke; continue ;;
   esac
done

## Ask if localtime fixes should be added
ltime="-"
if [ -e /etc/localtime ]; then
  ltime=/etc/localtime
else
  unset keystroke
  until [ -n "$keystroke" ]; do
    echo ""
    echo "Should I try to fix potential localtime problems?"
    echo -n "This may take some time... [Y/N]: "
    read keystroke
    case $keystroke in
       [Yy]) ltime=`find / -name localtime | head -n 1` ;;
       [Nn]) ltime="-" ;;
       *) unset keystroke; continue ;;
    esac
  done
fi
if [ ! -z "$ltime" ]; then
  if [ -e $ltime ]; then
    echo ""
    echo "Fixing (potential) localtime problems ..."
    echo "  Creating $jaildir$glroot/etc/localtime"
    cp /etc/localtime $jaildir$glroot/etc/
    mkdir -p $jaildir$glroot/usr/lib
    echo "  Creating $jaildir$glroot/usr/lib/zoneinfo"
    cp /etc/localtime $jaildir$glroot/usr/lib/zoneinfo
    mkdir -p $jaildir$glroot/usr/share
    cp /etc/localtime $jaildir$glroot/usr/share/zoneinfo
    echo "  Creating $jaildir$glroot/usr/share/zoneinfo"
    if [ ! -e /usr/lib/localtime ]; then
      echo "  Creating /usr/lib/localtime"
      cp /etc/localtime /usr/lib/
    fi
    echo "Done."
  fi
else
  echo "Could not find 'localtime' on your system. Fix not done."
fi

echo -e "\n\n\033[1m7. SSL/TLS SETUP:\033[0m\n\033[1m-----------------\033[0m\n"

### TLS INSTALLATION / 2003-09-01 - Turranius
### Mod 2004-01-04 - Fixed help text for 2.0+ / Turranius

proc_createcert() {
  echo " "
  echo "Please specify a generic name for this certificate."
  echo "This can be any name but should say something about the ftp server"
  echo -n "like the name for it perhaps (press enter for $servicename): "

  until [ -n "$tlsname" ]; do
    read tlsname
    if [ -z "$tlsname" ]; then
      tlsname=$servicename
    fi
  done

  echo ""
  echo "Using servbase: $tlsname  Using openssl: $opensslbin"
  echo "Please wait while creating certificate... (will take time!)"
  echo ""

  type=ecdsa

  servbase=$tlsname
  base=ftpd-$type

  $opensslbin ecparam -out $base.key -name secp384r1 -genkey 2>&1

  $opensslbin req -new -x509 -days 9000 -key $base.key -out $base.crt << EOF
.
.
.
.
.
$servbase
.
EOF

  if [ ! -e "$base.key" ]; then
    echo "Error: Could not create $base.key. You need to read README.TLS and do this manually."
    TLSERROR=TRUE
  else
    cat $base.key > $base.pem
  fi

  if [ "$TLSERROR" != "TRUE" ]; then
    if [ ! -e "$base.crt" ]; then
      echo "Error: Could not create $base.crt. You need to read README.TLS and do this manually."
      TLSERROR=TRUE
      rm -f $base.pem
    else
      cat $base.crt >> $base.pem
    fi
  fi

  rm -f $base.key $base.crt

  if [ -e "$base.pem" ]; then
    chmod 600 $base.pem
    mv -f $base.pem $tls/
    echo "Moving $base.pem to $tls . . . Done"
    tlsfull="$tls/$base.pem"
    if [ `cat $confdir/${servicename}.conf | grep "CERT_FILE /glftpd/ftpd-ecdsa.pem" | wc -l` -ne 0 ]; then
      cat $confdir/${servicename}.conf | sed "s|CERT_FILE /glftpd/ftpd-ecdsa.pem|CERT_FILE $tls/$base.pem|" >$confdir/${servicename}.conf.tmp && mv $confdir/${servicename}.conf.tmp $confdir/${servicename}.conf
    else
      echo "# Location of TLS certificate" >>$confdir/${servicename}.conf
      echo "CERT_FILE $tls/$base.pem" >>$confdir/${servicename}.conf
    fi
  else
    if [ "$TLSERROR" != "TRUE" ]; then
      echo "Error. Could not copy $base.pem to $tls. Have a look at README.TLS and set this up manually."
    fi
  fi
}

[ ! -z "$mywhichbin" ] && {
  opensslbin="$(which openssl)"
} || {
  searchbin="openssl"
  mywhich
  opensslbin=$binname
}
if [ -z "$opensslbin" ]; then

  echo "It would appear that you do not have openssl installed."
  echo "The installation will continue, but TLS support will not be installed."
  echo "To add SSL/TLS support (required), install openssl and read README.TLS."
  echo " "
  echo "Hit <enter> to continue."
  echo " "
  read tls

else

  [[ "$os" == "netbsd" && ! -f /etc/openssl/openssl.cnf ]] && {
    echo "It seems you do not have openssl.cnf installed. This file is needed"
    echo "to generate a x509 certificate for glftpd. Do you want me to try and"
    echo -n "find it and copy it to the correct location? [Y/n]> "
    read line
    echo ""
    [[ "$line" == "" || "$line" == "y" || "$line" == "Y" ]] && {
      [[ -f /usr/share/examples/openssl/openssl.cnf && -d /etc/openssl ]] && {
        cp /usr/share/examples/openssl/openssl.cnf /etc/openssl/
        echo "OK!"
      } || {
        echo "Failed to copy /usr/share/examples/openssl/openssl.cnf to /etc/openssl/ -"
        echo "you need to do this manually and create a cert."
      }
    } || {
      echo "Will not modify system. Beware that glftpd will not work unless a cert"
      echo "is created."
    }
    echo ""
  }
  echo "We will now create a certificate for SSL/TLS support. This step is"
  echo "required."
  echo " "

  until [ -n "$tls" ]; do
    echo " "
    echo "Please specify location, inside $jaildir$glroot,"
    echo -n "to install the cert (ftpd-ecdsa.pem) [/etc]: "
    read tls
    case $tls in
      /)
        echo "ftpd-ecdsa.pem should not reside in / - Try again."
        echo ""
        unset tls
        continue
        ;;
      /*)
        if [ ! -d "$jaildir$glroot/$tls" ]; then
          echo " "
          echo "$jaildir$glroot/$tls does not exist. Try again."
          unset tls
          continue
        else
          tls="$jaildir$glroot/$tls"
        fi
        ;;
      *)
        tls="$jaildir$glroot/etc"
        ;;
    esac
  done

  ## Jump to cert creation phase.
  proc_createcert
  echo -e "\n-> \033[1mIMPORTANT\033[0m !!!!"
  echo "-> If you get TLS errors of any kind, read instructions in README.TLS"
  echo "-> included in this package!"
  echo ""
  echo "Press <ENTER> to continue"
  read line
fi
unset opensslbin

### END TLS INSTALLATION / 2003-09-01 - Turranius

echo -e "\n\n\033[1m8. STARTING GLFTPD:\033[0m\n\033[1m-------------------\033[0m\n"

if [ -e /etc/resolv.conf ]; then
  [ ! -z "$mywhichbin" ] && {
    mydig="$(which dig)"
    mynslookup="$(which nslookup)"
  } || {
    searchbin="dig"
    mywhich
    mydig=$binname
    searchbin="nslookup"
    mywhich
    mynslookup=$binname
  }
  unset resfail
  echo -n "Copying /etc/resolv.conf to $jaildir$glroot/etc/resolv.conf . . . "
  cp -f /etc/resolv.conf $jaildir$glroot/etc/ >/dev/null 2>&1
  if [ $? -eq 0 ]; then
    echo "Done."
    if [ ! -z "$mydig" ]; then
      echo "Testing entries in resolv.conf (can take time):"
      for myres in `grep -v "^#" $jaildir$glroot/etc/resolv.conf | tr ' \t' '\n' | grep -e "^[0-9]*\.[0-9]*\.[0-9]*\.[0-9]*"`; do
        echo -n "   Testing $myres . . . "
        $mydig @${myres} in ANY glftpd.eu >/dev/null 2>&1
        if [ $? = 0 ]; then
          echo "OK."
        else
          echo "FAILED!"
          resfail=1
        fi
      done
    elif [ ! -z "$mynslookup" ]; then
      echo "Testing entries in resolv.conf (can take time):"
      for myres in `grep -v "^#" $jaildir$glroot/etc/resolv.conf | tr ' \t' '\n' | grep -e "^[0-9]*\.[0-9]*\.[0-9]*\.[0-9]*"`; do
        echo -n "   Testing $myres . . . "
        resultip=`${mynslookup} glftpd.eu ${myres} 2>/dev/null | grep -ie "^Address:" | awk '{print $2}' | tail -n 1`
        if [ ! "$resultip" = "$myres" ]; then
          echo "OK."
        else
          echo "FAILED!"
          resfail=1
        fi
      done
    fi
    if [ $resfail ]; then
      echo "Some or all of the entries in the resolv.conf failed to look up glftpd.eu."
      echo "It is reccommended to fix this problem if you rely on dns lookups."
      echo
    fi
  else
    echo "FAILED!"
  fi
fi

if [ "$netd" = "i" ]; then
    echo -n "Configuring inetd for glftpd . . . "
    # inetutils-inetd works fine with just the tcp6 line, but openbsd-inetd needs the tcp4 as well for dualstack
    { grep -v ^${servicename} /etc/inetd.conf
        echo "$servicename stream tcp4 nowait root $tcpd $jaildir$glroot/bin/glftpd $dummyflag -l -o -i $confpath $glstrings $euroweek" | tr -s ' '
        echo "$servicename stream tcp6 nowait root $tcpd $jaildir$glroot/bin/glftpd $dummyflag -l -o -i $confpath $glstrings $euroweek" | tr -s ' '
    } > /etc/inetd.conf.new
    mv -f /etc/inetd.conf.new /etc/inetd.conf
    echo "Done."
    echo -n "Restarting inetd . . . "

    case $os in
        openbsd)
            if kill -HUP `ps ax | grep inetd | grep -v grep | awk '{print $1}' | tr '\n' ' '`; then
               echo "Success."
            else
               echo -e '\033[1;31mFailed!\033[0m You must restart inetd before using glftpd.'
            fi
        ;;
        netbsd)
            if kill -HUP `ps ax | grep inetd | grep -v grep | awk '{print $1}' | tr '\n' ' '`; then
               echo "Success."
            else
               echo -e '\033[1;31mFailed!\033[0m You must restart inetd before using glftpd.'
            fi
        ;;
        *)
            if killall -HUP inetd; then
                echo "Success."
            else
                echo -e '\033[1;31mFailed!\033[0m You must start inetd before using glftpd.'
            fi
        ;;
    esac
elif [ "$netd" = "x" ]; then
    echo -n "Configuring xinetd for glftpd . . . "
    [ ! -d "/etc/xinetd.d" ] && {
        mkdir /etc/xinetd.d
        echo "includedir /etc/xinetd.d" >> /etc/xinetd.conf
    }
    cat <<EOF > /etc/xinetd.d/$servicename
service $servicename
{
    disable = no
    flags           = REUSE IPv6 ${tcpd:+NAMEINARGS}
    socket_type     = stream
    protocol        = tcp
    wait            = no
    user            = root
    server          = ${tcpd:-$jaildir$glroot/bin/glftpd}
    server_args     = ${tcpd:+$jaildir$glroot/bin/glftpd} -l -i -o $confpath $glstrings $euroweek
}
EOF
    echo "Done."
    [ ! -z "`grep -ie "^\ *only_from" /etc/xinetd.conf`" ] && {
      echo -e '\033[1;31mWARNING!\033[0m'
      echo "Found a 'only_from' line in your /etc/xinetd.conf file - I will"
      echo "comment this out - the original xinetd.conf will be renamed."
      mv /etc/xinetd.conf /etc/xinetd.conf.orig && cat /etc/xinetd.conf.orig | sed "s/^ *only_from/#&/" >/etc/xinetd.conf
      echo
    }
    echo -n "Restarting xinetd . . . "
    if killall -USR2 xinetd; then
        echo "Success."
        echo ""
    else
        echo -e '\033[1;31mFailed!\033[0m You must start xinetd before using glftpd.\033[0m'
    fi
elif [ "$netd" = "r" ]; then
    echo -n "Configuring rlinetd for glftpd . . . "
    [ ! -d "/etc/rlinetd.d" ] && {
        mkdir /etc/rlinetd.d
        echo "directory \"/etc/rlinetd.d\"" >> /etc/rlinetd.conf
    }
    cat <<EOF > /etc/rlinetd.d/$servicename
service "$servicename"
{
	# allows unlimited instances
	instances 0;
	family ipv6;
	port "$servicename";
	user "root";
	exec "${tcpd:+$jaildir$glroot/bin/glftpd} $dummyflag -l -o -i $confpath $glstrings $euroweek";
	server "${tcpd:-$jaildir$glroot/bin/glftpd}";
}
EOF
    echo "Done."
    echo -n "Restarting rlinetd . . . "
    if killall -HUP rlinetd; then
        echo "Success."
        echo ""
    else
        echo -e '\033[1;31mFailed!\033[0m You must start rlinetd before using glftpd.\033[0m'
    fi
elif [ "$netd" = "s" ]; then
    echo -n "Configuring systemd for glftpd . . . "
cat <<EOF > "/etc/systemd/system/$servicename@.service"
[Unit]
Description=$servicename per connection server
[Service]
ExecStart=-$jaildir$glroot/bin/glftpd -l -i -o $confpath $glstrings $euroweek
StandardInput=socket
SyslogFacility=ftp
EOF
cat <<EOF > "/etc/systemd/system/$servicename.socket"
[Unit]
Description=$servicename socket
[Socket]
ReusePort=true
ListenStream=[::]:$port
Accept=yes
#MaxConnections=64
[Install]
WantedBy=sockets.target
EOF
systemctl daemon-reload
systemctl enable "$servicename.socket"
systemctl start "$servicename.socket"
echo done && systemctl status "$servicename.socket"
elif [ "$netd" = "l" ]; then
    echo -n "Configuring launchd for glftpd . . . "

    [ ! -z "$dummyflag" ] && dummyflag="<string>$dummyflag</string>"
    [ ! -z "$confpath" ] && confpath=${confpath//[[:space:]]/}
    [ ! -z "$confpath" ] && confpath="<string>$confpath</string>"
    [ ! -z "$glstrings" ] && glstrings="<string>$glstrings</string>"
    [ ! -z "$euroweek" ] && euroweek="${euroweek%"${euroweek##*[![:space:]]}"}"
    [ ! -z "$euroweek" ] && euroweek="<string>$euroweek</string>"

    cat <<EOF > /Library/LaunchDaemons/eu.glftpd.$servicename.plist
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>Label</key>
    <string>eu.glftpd.$servicename</string>
    <key>ProgramArguments</key>
    <array>
        <string>${tcpd:-$jaildir$glroot/bin/glftpd}</string>
        <string>-l</string>
        <string>-o</string>
        <string>-i</string>
        $confpath
        $glstrings
        $euroweek
    </array>
    <key>inetdCompatibility</key>
    <dict>
        <key>Wait</key>
        <false/>
    </dict>
    <key>Sockets</key>
    <dict>
        <key>Listeners</key>
        <dict>
            <key>SockServiceName</key>
            <string>$servicename</string>
            <key>SockType</key>
            <string>stream</string>
        </dict>
    </dict>
</dict>
</plist>
EOF
    echo "Done."
    echo -n "Loading /Library/LaunchDaemons/eu.glftpd.$servicename.plist . . ."
    /bin/launchctl load /Library/LaunchDaemons/eu.glftpd.$servicename.plist
    echo "Done."

fi

echo -ne "\nAdding crontab entry to tabulate site stats nightly . . . "

{ crontab -l | grep -v "$jaildir$glroot/bin/reset"
  echo "0  0 * * *      $jaildir$glroot/bin/reset ${euroweek}${confpath}"
} | crontab - > /dev/null

echo "Done."

echo -ne "\nchmod'ing the site/ dir . . . "
chmod 4777 $jaildir$glroot/site
echo "Done."
echo ""
echo "Press <ENTER> to continue."
read line

echo -e "\n\n\033[1m9. FINISH:\033[0m\n\033[1m----------\033[0m\n"

echo "Congratulations, glFtpD has been installed. Scroll up and note any errors"
echo "that needs fixing. ./installgl.debug contains a log of the installation process."

echo -e "\nTo get your site running, you must edit \033[1m$confdir/${servicename}.conf\033[0m according to"
echo -e "the instructions in \033[1m$jaildir$glroot/docs/glftpd.docs.\033[0m"
echo -e "For help, visit #glftpd on EFnet \033[1mAFTER\033[0m you've read (not skimmed) the docs/faq."
echo ""
echo "After configuring glftpd, visit the following websites for additional"
echo "scripts to give your site some style!:"
echo "	Turranius - http://www.grandis.nu/glftpd"
echo "	Jehsoms - http://runslinux.net/"
echo "	dn's and ip's - http://www.chimera-coding.com"
echo "	D-ViBE's collection - http://www.glftpd.at"
echo ""
echo "The official glftpd homepage is located at http://www.glftpd.eu"
echo ""
echo "                                 Thanks for your support!"
echo "                                 the glFtpD team"
if [ ! "$os" = "linux" ]; then
  echo -e "\n\033[1mPS! Don't forget to download/install the binaries made for your os!\033[0m"
fi
echo ""
exit 0
