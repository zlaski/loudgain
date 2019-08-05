#!/bin/bash

# rgbpm.sh
# apply replay gain (mp3gain2) and BPM (bpm-calc) to all folders below and including specified one
# For correct album replay gain, we assume all files of one album are in the same folder.
# 2019-07-07 - new version looks for loudgain
# 2019-07-10 - new version compatible with loudgain v0.2.6/0.2.7
#            - added -k (clipping prevention) option to loudgain commands
# 2019-07-31 - add support for *.m4a (AAC audio) files (requires loudgain 0.4.0+)
# 2019-08-05 - add support for *.mp2 (MPEG-1 layer 2) audio files
#              (requires loudgain 0.5.2+ and new bpm-calc)

# define me
me=`basename "$0"`
#MUSICDIR="$HOME/Musik/Today/"
MUSICDIR=""

# check if we have loudgain available
LOUDGAIN=true
command -v loudgain >/dev/null 2>&1 || LOUDGAIN=false

# if loudgain not installed, we need mp3gain2, metaflac and vorbisgain, go check
if [ "$LOUDGAIN" != true ] ; then
  # check if mp3gain2 installed
  command -v mp3gain2 >/dev/null 2>&1 || { echo >&2 "$me requires \"mp3gain2\" but it's not installed. Aborting."; exit 2; }

  # check if metaflac installed
  command -v metaflac >/dev/null 2>&1 || { echo >&2 "$me requires \"metaflac\" but it's not installed. Aborting."; exit 2; }

  # check if vorbisgain installed
  command -v vorbisgain >/dev/null 2>&1 || { echo >&2 "$me requires \"vorbisgain\" but it's not installed. Aborting."; exit 2; }
else
  LOUDGAINVER=$(loudgain --version | head -n1 | awk '{print $2}')
  # we have the greatest and latest, yay!
  echo "Using 'loudgain' v${LOUDGAINVER} instead of mp3gain2, metaflac and vorbisgain."
fi

# check if bpm-calc installed
BPMCALC=true
#command -v bpm-calc >/dev/null 2>&1 || { echo >&2 "$me requires \"bpm-calc\" but it's not installed. Aborting."; exit 2; }
command -v bpm-calc >/dev/null 2>&1 || BPMCALC=false
if [ "$BPMCALC" != true ] ; then
  echo "No 'bpm-calc' found, BPM calculation will be skipped!"
fi

# default to MUSICDIR if no path(s) given
if [ "$1" = "" ] ; then
  # (re-)set arguments
  set -- "${@}" "$MUSICDIR"
fi

# go process each path
while [ "$1" != '' ] ; do

  # build an array with all folders within $1
  # need to change IFS (internal field separator), otherwise content would be split by space, tab, newline
  IFS=$'\n'
  FOLDERLIST=( $(find "$1" -depth -type d -print0 | xargs -0 -i echo {}) )
  unset $IFS
  echo "${#FOLDERLIST[@]} folders found in $1"

  for FOLDER in ${FOLDERLIST[@]} ; do
    echo ""
    echo "Working on $FOLDER ..."

    # calculate replay gain for all FLAC in this folder
    if [ "$LOUDGAIN" = true ] ; then
      find "$FOLDER" -maxdepth 1 -iname "*.flac" -type f -print0 | xargs -0 -r loudgain -a -k -s e
    else
      find "$FOLDER" -maxdepth 1 -iname "*.flac" -type f -print0 | xargs -0 -r metaflac --add-replay-gain
    fi

    # calculate replay gain for all OGG in this folder
    if [ "$LOUDGAIN" = true ] ; then
      find "$FOLDER" -maxdepth 1 -iname "*.ogg" -type f -print0 | xargs -0 -r loudgain -a -k -s e
    else
      find "$FOLDER" -maxdepth 1 -iname "*.ogg" -type f -print0 | xargs -0 -r vorbisgain -a
    fi

    # calculate replay gain for all MP2 in this folder
    if [ "$LOUDGAIN" = true ] ; then
      find "$FOLDER" -maxdepth 1 -iname "*.mp2" -type f -print0 | xargs -0 -r loudgain -I3 -S -L -a -k -s e
    fi

    # calculate replay gain for all MP3 in this folder
    if [ "$LOUDGAIN" = true ] ; then
      find "$FOLDER" -maxdepth 1 -iname "*.mp3" -type f -print0 | xargs -0 -r loudgain -I3 -S -L -a -k -s e
    else
      find "$FOLDER" -maxdepth 1 -iname "*.mp3" -type f -print0 | xargs -0 -r mp3gain2
    fi

    # calculate replay gain for all M4A in this folder
    if [ "$LOUDGAIN" = true ] ; then
      find "$FOLDER" -maxdepth 1 -iname "*.m4a" -type f -print0 | xargs -0 -r loudgain -L -a -k -s e
    fi

    # calculate BPM for all MP3 or FLAC in this folder
    echo ""
    if [ "$BPMCALC" = true ] ; then
      find "$FOLDER" -maxdepth 1 -type f \( -iname "*.mp2" -or -iname "*.mp3" -or -iname "*.flac" \) -print0 | xargs -0 -r bpm-calc
    else
      echo "Skipping BPM calculation ..."
    fi
  done

  shift
done
