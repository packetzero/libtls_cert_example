#!/bin/bash
FILE=$1

if [ "${FILE}" == "" ] ; then
  echo "usage: $0 file.pem"
  echo
  echo "   outputs to stdout, each line ending in a newline and backslash,"
  echo "   so the entire pem can be placed in a single C string declaration"
  exit 1
fi

cat "${FILE}" | sed 's/\(.*\)/\1\\n\\/g'
