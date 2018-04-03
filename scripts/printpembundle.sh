#!/bin/bash

FILE=$1

if [ "${FILE}" = "" ] ; then
  echo "usage: $0 path/to/cabundle.pem"
  echo
  echo "  prints out details of all certs in bundle to stdout"
  echo
  exit 1
fi

# have to convert to pkcs7 to print them out

openssl crl2pkcs7 -nocrl -certfile ${FILE} | openssl pkcs7 -print_certs -text -noout
