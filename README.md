loudgain
========

**loudgain** is a loudness normalizer that scans music files and calculates
loudness-normalized gain and loudness peak values according to the EBU R128
standard, and can optionally write ReplayGain-compatible metadata.

[EBU R128](https://tech.ebu.ch/loudness) is a set of recommendations regarding
loudness normalisation based on the algorithms to measure audio loudness and
true-peak audio level defined in the
[ITU BS.1770](http://www.itu.int/rec/R-REC-BS.1770/en) standard, and is used in
the (currently under construction) ReplayGain 2.0 specification.

loudgain implements a subset of mp3gain's command-line options, which means that
it can be used as a drop-in replacement in some situations.

**Note:** loudgain will _not_ modify the actual audio data, but instead just
write ReplayGain _tags_ if so requested. It is up to the player to interpret
these. (_Hint:_ In some players, you need to enable this feature.)

**Note:** loudgain can be used instead of `mp3gain`, `vorbisgain` and `metaflac`
in order to write ReplayGain 2.0 compatible loudness tags into MP3, Ogg Vorbis
and FLAC files, respectively.

**Note:** EBU R128 recommends a program (integrated) target loudness of -23 LUFS
and uses _LU_ and _LUFS_ units. The proposed ReplayGain 2.0 standard tries to
stay compatible with older software and thus uses EBU R128 loudness measuring
_but_ at a target of -18 LUFS (estimated to be equal to the old "89 dB" reference
loudness). The generated tags also still use the "dB" suffix (1 dB = 1 LU).

loudgain defaults to the ReplayGain 2.0 standard (-18 LUFS, "dB" units). Peak
values are measured using the True Peak algorithm.

## GETTING STARTED

loudgain is (mostly) compatible with mp3gain's command-line arguments (the `-r`
option is always implied). Here are a few examples:

```bash
$ loudgain *.mp3            # scan some mp3 files without tagging
$ loudgain -s i *.mp3       # scan and tag some mp3 files with ID3v2 tags
$ loudgain -d 13 -k *.mp3   # add a pre-amp gain and prevent clipping
$ loudgain -s r *.mp3       # remove ReplayGain tags from the files
```

See the [man page](docs/loudgain.1.md) for more information.

## DEPENDENCIES

 * `libavcodec`
 * `libavformat`
 * `libavutil`
 * `libebur128`
 * `libtag`

 On Ubuntu 18.04/Linux Mint 19.1, you can usually install CMake and the needed libraries using
 ```bash
 sudo apt-get install cmake libavcodec-dev libavformat-dev libavutil-dev libebur128-dev libtag1-dev
 ```

## BUILDING

loudgain is distributed as source code. Install with:

```bash
$ mkdir build && cd build
$ cmake ..
$ make
$ [sudo] make install
```

## COPYRIGHT

Copyright (C) 2014 Alessandro Ghedini <alessandro@ghedini.me>  
Modifications Copyright (C) 2019 Matthias C. Hormann <mhormann@gmx.de>

See COPYING for the license.
