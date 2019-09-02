loudgain(1) -- loudness normalizer based on the EBU R128 standard
=================================================================

## SYNOPSIS

`loudgain [OPTIONS] FILES...`

## DESCRIPTION

**loudgain** is a loudness normalizer that scans music files and calculates
loudness-normalized gain and loudness peak values according to the EBU R128
standard, and can optionally write ReplayGain-compatible metadata.

loudgain implements a subset of mp3gain's command-line options, which means that
it can be used as a drop-in replacement in some situations.

loudgain will _not_ modify the actual audio data, but instead just
write ReplayGain _tags_ if so requested. It is up to the player to interpret
these. (In some players, you need to enable this feature.)

loudgain currently supports writing tags to the following file types:  
  FLAC (.flac), Ogg Vorbis (.ogg), MP2 (.mp2), MP3 (.mp3), MP4 (.mp4, .m4a).  
  Opus (.opus) -- experimental support, use with care!  
  ASF/WMA (.asf, .wma) -- experimental support, use with care!  
  WAV (.wav) -- experimental support, use with care!


## OPTIONS

`-h, --help`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Show this help.

`-v, --version`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Show version number.

`-r, --track`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Calculate track gain only (default).

`-a, --album`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Calculate album gain (and track gain).

`-c, --clip`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Ignore clipping warnings.

`-k, --noclip`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Lower track/album gain to avoid clipping (<= -1 dBTP).

`-K n, --maxtpl=n`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Avoid clipping; max. true peak level = n dBTP.

`-d n, --pregain=n`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Apply n dB/LU pre-gain value (-5 for -23 LUFS target).

`-s d, --tagmode=d`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Delete ReplayGain tags from files.

`-s i, --tagmode=i`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Write ReplayGain 2.0 tags to files. ID3v2 for MP2/MP3, Vorbis Comments for FLAC, Ogg Vorbis and Opus, iTunes-type metadata for MP4, WMA tags for ASF/WMA.

`-s e, --tagmode=e`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
like '-s i', plus extra tags (reference, ranges).

`-s l, --tagmode=l`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
like '-s e', but LU units instead of dB.

`-s s, --tagmode=s`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Don't write ReplayGain tags (default).

`-L, --lowercase`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Force lowercase 'REPLAYGAIN_*' tags (MP2/MP3/MP4/ASF/WMA/WAV only; non-standard)

`-S, --striptags`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Strip tag types other than ID3v2 from MP2/MP3 files (i.e. ID3v1, APEv2).

`-I 3, --id3v2version=3`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Write ID3v2.3 tags to MP2/MP3/WAV files.

`-I 4, --id3v2version=4`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Write ID3v2.4 tags to MP2/MP3/WAV files (default).

`-o, --output`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Database-friendly tab-delimited list output (mp3gain-compatible).

`-O, --output-new`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Database-friendly new format tab-delimited list output.

`-q, --quiet`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Don't print scanning status messages.

## AUTHORS ##

Alessandro Ghedini <alessandro@ghedini.me>  
Matthias C. Hormann <mhormann@gmx.de>

## COPYRIGHT ##

Copyright (C) 2014 Alessandro Ghedini <alessandro@ghedini.me>  
Everything after v0.1: Copyright (C) 2019 Matthias C. Hormann <mhormann@gmx.de>

This program is released under the 2 clause BSD license.
