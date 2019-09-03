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
FLAC (.flac), Ogg (.ogg, .oga, .spx, .opus), MP2 (.mp2), MP3 (.mp3), MP4 (.mp4, .m4a).

Experimental support, use with care: ASF/WMA (.asf, .wma), WAV (.wav), WavPack (.wv).


## OPTIONS

* `-h, --help`:
  Show this help.

* `-v, --version`:
  Show version number.

* `-r, --track`:
  Calculate track gain only (default).

* `-a, --album`:
  Calculate album gain (and track gain).

* `-c, --clip`:
  Ignore clipping warnings.

* `-k, --noclip`:
  Lower track/album gain to avoid clipping (<= -1 dBTP).

* `-K n, --maxtpl=n`:
  Avoid clipping; max. true peak level = n dBTP.

* `-d n, --pregain=n`:
  Apply n dB/LU pre-gain value (-5 for -23 LUFS target).

* `-s d, --tagmode=d`:
  Delete ReplayGain tags from files.

* `-s i, --tagmode=i`:
  Write ReplayGain 2.0 tags to files. ID3v2 for MP2/MP3; Vorbis Comments for FLAC,
  Ogg, Speex and Opus; iTunes-type metadata for MP4/M4A; WMA tags for ASF/WMA;
  APEv2 tags for WavPack.

* `-s e, --tagmode=e`:
  like '-s i', plus extra tags (reference, ranges).

* `-s l, --tagmode=l`:
  like '-s e', but LU units instead of dB.

* `-s s, --tagmode=s`:
  Don't write ReplayGain tags (default).

* `-L, --lowercase`:
  Force lowercase 'REPLAYGAIN_*' tags (MP2/MP3/MP4/ASF/WMA/WAV only; non-standard).

* `-S, --striptags`:
  Strip tag types other than ID3v2 from MP2/MP3 files (i.e. ID3v1, APEv2).
  Strip tag types other than APEv2 from WavPack files (i.e. ID3v1).

* `-I 3, --id3v2version=3`:
  Write ID3v2.3 tags to MP2/MP3/WAV files.

* `-I 4, --id3v2version=4`:
  Write ID3v2.4 tags to MP2/MP3/WAV files (default).

* `-o, --output`:
  Database-friendly tab-delimited list output (mp3gain-compatible).

* `-O, --output-new`:
  Database-friendly new format tab-delimited list output. Ideal for analysis
  of files if redirected to a CSV file.

* `-q, --quiet`:
  Don't print scanning status messages.


## RECOMMENDATIONS

To give you a head start, here are my personal recommendations for being (almost)
universally compatible.

Use loudgain on a »one album per folder« basis; standard RG2 settings but
lowercase ReplayGain tags; clipping prevention on; strip obsolete tag types
from MP3 and WavPack files; use ID3v2.3 for MP3s; store extended tags:

    $ loudgain -a -k -s e *.flac
    $ loudgain -a -k -s e *.ogg
    $ loudgain -I3 -S -L -a -k -s e *.mp3
    $ loudgain -L -a -k -s e *.m4a
    $ loudgain -a -k -s e *.opus
    $ loudgain -L -a -k -s e *.wma
    $ loudgain -I3 -L -a -k -s e *.wav
    $ loudgain -S -a -k -s e *.wv

I’ve been happy with these settings for many years now. Your mileage may vary.


## BUGS

**loudgain** is maintained on GitHub. Please report all bugs to the issue tracker
at https://github.com/Moonbase59/loudgain/issues.


## AUTHORS

Matthias C. Hormann <mhormann@gmx.de>  
Alessandro Ghedini <alessandro@ghedini.me>


## COPYRIGHT

Copyright (C) 2019 Matthias C. Hormann <mhormann@gmx.de> (versions > 0.1)  
Copyright (C) 2014 Alessandro Ghedini <alessandro@ghedini.me> (v0.1)

This program is released under the 2 clause BSD license.
