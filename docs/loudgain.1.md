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
Lower track and/or album gain to avoid clipping.

`-d, --db-gain`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Apply the given pre-gain value (in dB/LU).

`-s d, --tag-mode d`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Delete ReplayGain tags from files.

`-s i, --tag-mode i`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Write ReplayGain 2.0 tags to files. ID3v2 for MP3, Vorbis Comments for FLAC and Ogg Vorbis.

`-s e, --tag-mode e`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
like '-s i', plus extra tags (reference, ranges).

`-s l, --tag-mode l`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
like '-s e', but LU units instead of dB.

`-s s, --tag-mode s`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Don't write ReplayGain tags (default).

`-L, --lowercase`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Force lowercase 'REPLAYGAIN_*' tags (MP3/ID3v2 only; non-standard)

`-o, --output`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Database-friendly tab-delimited list output.

`-q, --quiet`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Don't print status messages.

## AUTHOR ##

Alessandro Ghedini <alessandro@ghedini.me>  
Modifications: Matthias C. Hormann <mhormann@gmx.de>

## COPYRIGHT ##

Copyright (C) 2014 Alessandro Ghedini <alessandro@ghedini.me>  
Modifications: Copyright (C) 2019 Matthias C. Hormann <mhormann@gmx.de>

This program is released under the 2 clause BSD license.
