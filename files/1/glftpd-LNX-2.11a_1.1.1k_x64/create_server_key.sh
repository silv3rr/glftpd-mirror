#!/bin/sh
ssldirs="$OPENSSLDIR /usr /usr/local/openssl /usr/lib/openssl /usr/local/ssl /usr/ssl
         /usr/share /usr/lib /usr/lib/ssl /usr/pkg /opt /opt/ssl"

opensslbin=`which openssl`

keysize=2048
days=9000
eccurve=secp384r1

if [ -z $opensslbin ]; then
 for x in `echo $ssldirs`; do
  if [ -f $x/bin/openssl ]; then
    opensslbin=$x/bin/openssl
  fi
 done
 if [ -z $opensslbin ]; then
  echo "Sorry openssl binary not found !"
  exit 1
 fi
fi

echo "create_server_key.sh v1.1 by Slask&HoE"
if [ -z "$1" ] ; then
	echo "Usage: ./create_server_key.sh [rsa/ecdsa(default)] info"
        echo "info - can be any word, and it should inform the client"
        echo "       about the server he is logging in (for example servername)"
        echo "rsa/ecdsa - if you dont specify this then ECDSA key will be created"
        echo "certificate is for $days days and is self-signed"
        exit 1
fi

if [ -z "$2" ] ; then
	type=ecdsa
	servbase="$1"
else
	type="$1"
	servbase="$2"
fi

base=ftpd-$type

if [ $type = "rsa" ]; then
	$opensslbin gen$type -out $base.key $keysize
elif [ $type = "ecdsa" ]; then
	$opensslbin ecparam -out $base.key -name $eccurve -genkey
	if [ ! "$?" -eq "0" ]; then
         echo "generating of ecparam failed. Maybe your openssl version is too old. try to generate DSA cert instead."
	 exit 2
	fi
else
	echo "unknown cert type"
	exit 1
fi

$opensslbin req -new -x509 -days $days -key $base.key -out $base.crt 2>/dev/null << EOF
.
.
.
.
.
$servbase
.
EOF

cat $base.key $base.crt > $base.pem

rm -f $base.key $base.crt $base.dh $base.dsaparam
echo 
echo $base.pem created.
echo "Copy the file to some safe place and tell glftpd where to find it with CERT_FILE glftpd.conf option."


