#!/bin/bash
if [ "$1" == "" ] ; then
  echo "usage: $0 certfile.der"
  echo
  echo "  prints PEM format to stdout"
  exit 1
fi

openssl x509 -in "$1" -inform der -outform pem
