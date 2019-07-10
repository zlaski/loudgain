loudgain
========

**loudgain** is a loudness normalizer that scans music files and calculates
loudness-normalized gain and loudness peak values according to the EBU R128
standard, and can optionally write ReplayGain-compatible metadata.

[EBU R128](https://tech.ebu.ch/loudness) is a set of recommendations regarding
loudness normalisation based on the algorithms to measure audio loudness and
true-peak audio level defined in the
[ITU BS.1770](http://www.itu.int/rec/R-REC-BS.1770/en) standard, and is used in
the (currently under construction) [ReplayGain 2.0 specification](https://wiki.hydrogenaud.io/index.php?title=ReplayGain_2.0_specification).

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
loudness). The generated tags also still use the "dB" suffix (except for '-s l'
tag-mode which uses "LU"; 1 dB = 1 LU).

loudgain defaults to the ReplayGain 2.0 standard (-18 LUFS, "dB" units,
uppercase tags). Peak values are measured using the True Peak algorithm.

## GETTING STARTED

loudgain is (mostly) compatible with mp3gain's command-line arguments (the `-r`
option is always implied). Here are a few examples:

```bash
$ loudgain *.mp3                 # scan some mp3 files without tagging
$ loudgain -s i *.mp3            # scan and tag some mp3 files with ReplayGain 2.0 tags
$ loudgain -d 13 -k *.mp3        # add a pre-amp gain and prevent clipping
$ loudgain -s d *.mp3            # remove ReplayGain tags from the files
$ loudgain -a -s i *.flac        # scan & tag an album of FLAC files
$ loudgain -a -s e *.flac        # scan & tag an album of FLAC files, add extra tags (reference, ranges)
$ loudgain -d -5 -a -s l *.flac  # apply -5 LU pregain to reach -23 LUFS target for EBU compatibility, add reference & range information, use 'LU' units in tags
```

See the [man page](docs/loudgain.1.md) for more information.

## DEPENDENCIES

 * `libavcodec`
 * `libavformat`
 * `libavutil`
 * `libebur128` (v1.2.4+ recommended)
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

## TECHNICAL DETAILS (advanced users stuff)

Not an expert? **Don’t worry, loudgain comes with sensible default settings.** You might never need the "special options".

But _if_ you do, you might need them badly. Welcome to the expert sessions—read on!

### Uppercase or lowercase 'REPLAYGAIN_*' tags?

This has been a problem ever since, most notably in MP3 ID3v2 tags, because these are case-sensitive. FLAC and Ogg Vorbis use Vorbis Comments to store tags, these can be upper-, lower- or mixed case per definition and MUST be treated equal.

The ReplayGain 1 and 2.0 specs clearly state that the tags should be UPPERCASE but many taggers still write lowercase tags (foobar2000, metamp3, taggers using pre-1.2.2 Mutagen like older MusicBrainz Picard versions, and others).

Unfortunately, there are lots of audio players out there that only respect _one_ case. For instance, VLC only respects uppercase (_Edit: This seems to chave changed in versions 3.x._), IDJC only respects lowercase. Only a very few go the extra effort to check for both variants of tags.

It seems that out in the field, there are more players that respect the lowercase tags than players respecting the uppercase variant, maybe due to the fact that the majority of MP3s seem to be tagged using the lowercase ReplayGain tags—they simply adopted.

Now all this can lead to lots of problems and seems to be an unsolvable issue. In my opinion, _all_ players should respect _all_ variants of tags.

Since we don’t live in an ideal world, my approach to the problem is as follows:

1. loudgain uses the ReplayGain 2.0 "standard" as its default, in order not to confuse newbies. (That means: -18 LUFS reference, "dB" units, and uppercase 'REPLAYGAIN_*' tags.)

2. Upon deleting ReplayGain tags (using `-s d` or before writing new tags using `-s i`, `-s e` or `-s l`), loudgain will delete _all_ variants of the old tags. Thus, if you have like (may be due to mis-tagging):
    ```
    REPLAYGAIN_TRACK_GAIN -7.02 dB
    replaygain_track_gain -7.50 dB
    ```
    loudgain will remove _both_ then write _one_ new tag (of each sort), usually like:
    ```
    REPLAYGAIN_TRACK_GAIN -7.02 dB
    ```

3. For the seemingly unavoidable cases where you _do_ indeed need lowercase ReplayGain tags in MP3 ID3v2 tags, I introduced a new option `-L` (`--lowercase`) that will _force_ writing the lowercase variant (but _only_ in MP3 ID3v2; FLAC and Ogg Vorbis will still get standard uppercase tags):
    ```
    replaygain_track_gain -7.02 dB
    ```

### Tags written (and/or deleted)

#### Default mode `-s i` (`--tag-mode i`)

In its simplest, mp3gain-compatible, tag writing mode `-s i` (`--tag-mode i`), loudgain will only produce the bare minimum required by the ReplayGain 2.0 standard:
```
REPLAYGAIN_TRACK_GAIN [-]a.bb dB
REPLAYGAIN_TRACK_PEAK c.dddddd
```

If "album mode" `-a` is specified, these additional album tags will be produced:
```
REPLAYGAIN_ALBUM_GAIN [-]a.bb dB
REPLAYGAIN_ALBUM_PEAK c.dddddd
```

All other known ReplayGain tags (no matter what case) will be _deleted_, in order not to confuse other software. These are (if present):
```
REPLAYGAIN_TRACK_GAIN
REPLAYGAIN_TRACK_PEAK
REPLAYGAIN_TRACK_RANGE
REPLAYGAIN_ALBUM_GAIN
REPLAYGAIN_ALBUM_PEAK
REPLAYGAIN_ALBUM_RANGE
REPLAYGAIN_REFERENCE_LOUDNESS
```

#### Enhanced mode `-s e` (`--tag-mode e`)

In its "enhanced" tag-writing mode `s e` (`--tag-mode e`), loudgain will work like above but _in addition_ write the following tags:
```
REPLAYGAIN_TRACK_RANGE a.bb dB
REPLAYGAIN_REFERENCE_LOUDNESS [-]a.bb LUFS
```

If "album mode" `-a` is specified, this additional album tag will be produced:
```
REPLAYGAIN_ALBUM_RANGE a.bb dB
```

Since ReplayGain 2.0 works using the EBU R128 recommendation (but at the different target -18 LUFS), and -18 LUFS has been _estimated_ to be "equal" to the old 89 dB SPL ReplayGain 1 standard, it would make no sense to give pseudo-dB values as a reference loudness. This is possibly the reason why the RG2 spec dropped this tag altogether, and this is the reason why I stick with real LUFS units in the `REPLAYGAIN_REFERENCE_LOUDNESS` tag, even when writing "dB" units elsewhere for compatibility with the ReplayGain 2.0 standard and older software.


#### "LU" units mode `-s l` (`--tag-mode l`)

There is a third tag-writing mode in loudgain, that will probably not be used by many: `-s l` (`--tag-mode l`). It behaves like above (`-s e` mode), but uses "LU" units instead of "dB" (fortunately, 1 dB = 1 LU). This might not be of interest for many, but the few who need it, need it badly. :-)

The generated tags look like this:
```
REPLAYGAIN_TRACK_GAIN [-]a.bb LU
REPLAYGAIN_TRACK_PEAK c.dddddd
REPLAYGAIN_TRACK_RANGE a.bb LU
REPLAYGAIN_ALBUM_GAIN [-]a.bb LU
REPLAYGAIN_ALBUM_PEAK c.dddddd
REPLAYGAIN_ALBUM_RANGE a.bb LU
REPLAYGAIN_REFERENCE_LOUDNESS [-]a.bb LUFS
```

This is a variant that is handled perfectly by IDJC (the Internet DJ Console), and others. Fully EBU-compliant (-23 LUFS target) files can be made by adding a -5 LU pregain to loudgain’s commandline arguments: `-d -5` (`--db-gain -5`).

Interestingly enough, I found that many players can actually handle the "LU" format, because they simply use something like C’s _atof()_ in their code, thus ignoring whatever comes after the numeric value!


### Track and Album Peak

When comparing to other (older) tools, you might find the peak values are often _higher_ than what you were used to.

This is due to the fact that loudgain uses _true peak_ (not RMS peak) values, which are much more appropriate in the digital world, especially when using lossy audio formats.

loudgain uses the `libebur128` [library](https://github.com/jiixyj/libebur128) for this. It currently (v1.2.4) provides a true peak calculated using a custom polyphase FIR interpolator that will oversample 4x for sample rates < 96000 Hz, 2x for sample rates < 192000 Hz and leave the signal unchanged for 192000 Hz.


### MP3 ID3v2.3, ID3v2.4, and APE tags

MP3 files can come with lots of tagging versions: _ID3v1_, _ID3v1.1_, _ID3v2.2_, _ID3v2.3_ and _ID3v2.4_. If people have been using `mp3gain` on the files, they might even contain an additional _APEv2_ set of tags. It’s a mess, really.

Fortunately, nowadays, all older standards are obsolete and _ID3v2.3_ and _ID3v2.4_ are almost universally used.

Older players usually don’t understand ID3v2.4, and ID3v2.3 doesn’t know about _UTF-8 encoding_. Important frames like TYER, TDAT, and TDRC have also changed. A mess again.

My current solution to this (and loudgain’s default behaviour) is as follows:

1. The overall philosophy is: "The user might know what he is doing. So try not to touch more than needed."

2. loudgain will try to read important information from all of the above mentioned tag types, preferring ID3v2 variants.

3. Upon saving, loudgain will preserve ID3v1/ID3v1.1 and APE tags, but upgrade IDv2.x tags to version _ID3v2.4_.

I realize that a) people might _need_ ID3v2.3 and b) some of us _want_ to strip unneccessary ID3v1 and APE tags from MP3 files. There is a solution to that:

#### Force writing ID3v2.3 or ID3v2.4 tags

The default tag type loudgain writes to MP3 files is _ID3v2.4_ which is the most modern and UTF-8-capable version. There might be reasons to _force_ loudgain to write ID3v2.3 tags instead (maybe for an older audio player that doesn’t handle ID3v2.4).

Using the options `-I 3` (`--id3v2version 3`) or `-I 4` (`--id3v2version 4`), you can force loudgain to write ID3v2.3 or ID3v2.4 tags, respectively.

This option has no effect on FLAC or Ogg vorbis files.

#### Strip unwanted ID3v1/APEv2 tags `-S` (`--striptags`)

More than one kind of tags in an MP3 file is generally a bad idea. It might confuse other tagging software and audio players. Historically, `mp3gain` used _APEv2_ tags to "hide" ReplayGain data from normal ID3 taggers—a bad idea. ID3v1, ID3v1.1 and ID3v2.2 are obsolete due to their restricted possibilities and lack of support.

Using loudgain’s `-S` (`--striptags`) option, these other tag types can be removed from MP3 files when writing new ReplayGain tags, and a clean ID3v2-only file is left. In this process, loudgain will try to keep content from other tag types as far as possible (like 'MP3GAIN_MINMAX', for instance) and transport it over into appropriate ID3v2 tags. For tags that are a duplicate of existing ID3v2 tags, the ID3v2 tag content is preferred.

This option has no effect on FLAC or Ogg Vorbis files.


## COPYRIGHT

Copyright (C) 2014 Alessandro Ghedini <alessandro@ghedini.me>  
Everything after v0.1: Copyright (C) 2019 Matthias C. Hormann <mhormann@gmx.de>

See COPYING for the license.
