#!/bin/bash
################################################################################
# test-SeqGen.sh
#  Shell script to test running SeqGen using sample signals in the data directory.
#
# Format: test-SeqGen.sh [signal-desc-file]
#
# Options:
#   signal-desc-file  Signal descriptor file, without the .xml extension.
#                     Assumed to be in the data directory.
#                     If no arguments given, a selection table will be displayed.
#
# Author: J. Parziale
# Date: November 2021
################################################################################

# Debug mode is on if set to 1.
DEBUG=0

# If debug mode is on, we want to print commands rather than execute them.
if [[ $DEBUG -eq 1 ]]; then
  debugPrint=echo
fi

function RestoreIFS {
    sig=$?
    if [[ $sig -ge 128 ]]; then sig=$(($? - 128)); fi
    export IFS=$SAVEIFS
    [[ $sig != 0 ]] && echo "Signal caught; $sig"
    echo
}

SAVEIFS=$IFS

trap RestoreIFS EXIT SIGINT SIGTERM SIGABRT SIGQUIT

################################################################################

# If no signal descriptor specified on command line, prompt for one from list.
if [[ "$1" == "" ]]; then
  IFS=$(echo -en "\n\b")

  # Create array of descriptor files found in ../data
  for df in $(ls ../data/*.xml)
  do
    descFiles=( ${descFiles[*]} $(basename -s .xml "$df") )
  done

  if [[ ${#descFiles[*]} -gt 1 ]]; then
    echo "CHOOSE:"
    select descFile in ${descFiles[*]}; do
      echo
      break
    done
  else
    descFile=$descFiles
  fi

else
  # Descriptor file specified on command line
  descFile=$1;
fi # end if ($1 == "")

################################################################################

if [[ -z $descFile ]]; then
  echo "No descriptor file selected"
else
  descFile="../data/${descFile}.xml"
  echo "Running seqgen using ${descFile} ..."
  $debugPrint ../bin/seqgen "${descFile}"
fi

################################################################################
