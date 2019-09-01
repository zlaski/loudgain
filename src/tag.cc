/*
 * Loudness normalizer based on the EBU R128 standard
 *
 * Copyright (c) 2014, Alessandro Ghedini
 * All rights reserved.
 * 2019-06-30 - Matthias C. Hormann
 *   - Tag format in accordance with ReplayGain 2.0 spec
 *     https://wiki.hydrogenaud.io/index.php?title=ReplayGain_2.0_specification
 *   - Add Ogg Vorbis file handling
 * 2019-07-07 - v0.2.3 - Matthias C. Hormann
 *   - Write lowercase REPLAYGAIN_* tags to MP3 ID3v2, for incompatible players
 * 2019-07-08 - v0.2.4 - Matthias C. Hormann
 *   - add -s e mode, writes extra tags (REPLAYGAIN_REFERENCE_LOUDNESS,
 *     REPLAYGAIN_TRACK_RANGE and REPLAYGAIN_ALBUM_RANGE)
 *   - add "-s l" mode (like "-s e" but uses LU/LUFS instead of dB)
 * 2019-07-09 - v0.2.6 - Matthias C. Hormann
 *  - Add "-L" mode to force lowercase tags in MP3/ID3v2.
 * 2019-07-10 - v0.2.7 - Matthias C. Hormann
 *  - Add "-S" mode to strip ID3v1/APEv2 tags from MP3 files.
 *  - Add "-I 3"/"-I 4" modes to select ID3v2 version to write.
 * 2019-07-31 - v0.40 - Matthias C. Hormann
 *	- Add MP4 handling
 * 2019-08-02 - v0.5.1 - Matthias C. Hormann
 *  - avoid unneccessary double file write on deleting+writing tags
 *  - make tag delete/write functions return true on success, false otherwise
 * 2019-08-06 - v0.5.3 - Matthias C. Hormann
 *  - Add support for Opus (.opus) files.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <math.h>

#include <taglib.h>

#include <textidentificationframe.h>

#include <mpegfile.h>
#include <id3v2tag.h>

#include <flacfile.h>
#include <vorbisfile.h>
#include <xiphcomment.h>
#include <mp4file.h>
#include <opusfile.h>
#include <asffile.h>

#include "scan.h"
#include "tag.h"
#include "printf.h"

/*** MP3 ****/

static void tag_add_txxx(TagLib::ID3v2::Tag *tag, char *name, char *value) {
	TagLib::ID3v2::UserTextIdentificationFrame *frame =
		new TagLib::ID3v2::UserTextIdentificationFrame;

	frame -> setDescription(name);
	frame -> setText(value);

	tag -> addFrame(frame);
}

void tag_remove_mp3(TagLib::ID3v2::Tag *tag) {
	TagLib::ID3v2::FrameList::Iterator it;
	TagLib::ID3v2::FrameList frames = tag -> frameList("TXXX");

	for (it = frames.begin(); it != frames.end(); ++it) {
		TagLib::ID3v2::UserTextIdentificationFrame *frame =
		 dynamic_cast<TagLib::ID3v2::UserTextIdentificationFrame*>(*it);

		 // this removes all variants of upper-/lower-/mixed-case tags
		if (frame && frame -> fieldList().size() >= 2) {
			TagLib::String desc = frame -> description().upper();

			// also remove (old) reference loudness, it might be wrong after recalc
			if ((desc == "REPLAYGAIN_TRACK_GAIN") ||
			    (desc == "REPLAYGAIN_TRACK_PEAK") ||
					(desc == "REPLAYGAIN_TRACK_RANGE") ||
			    (desc == "REPLAYGAIN_ALBUM_GAIN") ||
			    (desc == "REPLAYGAIN_ALBUM_PEAK") ||
					(desc == "REPLAYGAIN_ALBUM_RANGE") ||
			    (desc == "REPLAYGAIN_REFERENCE_LOUDNESS"))
				tag -> removeFrame(frame);
		}
	}
}

// Even if the ReplayGain 2 standard proposes replaygain tags to be uppercase,
// unfortunately some players only respect the lowercase variant (still).
// So for the time being, we write non-standard lowercase tags to ID3v2.
bool tag_write_mp3(scan_result *scan, bool do_album, char mode, char *unit,
	bool lowercase, bool strip, int id3v2version) {
	char value[2048];

	TagLib::MPEG::File f(scan -> file);
	TagLib::ID3v2::Tag *tag = f.ID3v2Tag(true);

	// remove old tags before writing new ones
	tag_remove_mp3(tag);

	if (lowercase) {
		// use lowercase replaygain tags
		// (non-standard but used by foobar2000 and needed by some players like IDJC)
		snprintf(value, sizeof(value), "%.2f %s", scan -> track_gain, unit);
		tag_add_txxx(tag, const_cast<char *>("replaygain_track_gain"), value);

		snprintf(value, sizeof(value), "%.6f", scan -> track_peak);
		tag_add_txxx(tag, const_cast<char *>("replaygain_track_peak"), value);

		// Only write album tags if in album mode (would be zero otherwise)
		if (do_album) {
			snprintf(value, sizeof(value), "%.2f %s", scan -> album_gain, unit);
			tag_add_txxx(tag, const_cast<char *>("replaygain_album_gain"), value);

			snprintf(value, sizeof(value), "%.6f", scan -> album_peak);
			tag_add_txxx(tag, const_cast<char *>("replaygain_album_peak"), value);
		}

		// extra tags mode -s e or -s l
		if (mode == 'e' || mode == 'l') {
			snprintf(value, sizeof(value), "%.2f LUFS", scan -> loudness_reference);
			tag_add_txxx(tag, const_cast<char *>("replaygain_reference_loudness"), value);

			snprintf(value, sizeof(value), "%.2f %s", scan -> track_loudness_range, unit);
			tag_add_txxx(tag, const_cast<char *>("replaygain_track_range"), value);

			if (do_album) {
				snprintf(value, sizeof(value), "%.2f %s", scan -> album_loudness_range, unit);
				tag_add_txxx(tag, const_cast<char *>("replaygain_album_range"), value);
			}
		}
	} else {
		// use standard-compliant uppercase replaygain tags (default)
		// required, for instance, by VLC
		// unforunately not respected by others, like IDJC
		snprintf(value, sizeof(value), "%.2f %s", scan -> track_gain, unit);
		tag_add_txxx(tag, const_cast<char *>("REPLAYGAIN_TRACK_GAIN"), value);

		snprintf(value, sizeof(value), "%.6f", scan -> track_peak);
		tag_add_txxx(tag, const_cast<char *>("REPLAYGAIN_TRACK_PEAK"), value);

		// Only write album tags if in album mode (would be zero otherwise)
		if (do_album) {
			snprintf(value, sizeof(value), "%.2f %s", scan -> album_gain, unit);
			tag_add_txxx(tag, const_cast<char *>("REPLAYGAIN_ALBUM_GAIN"), value);

			snprintf(value, sizeof(value), "%.6f", scan -> album_peak);
			tag_add_txxx(tag, const_cast<char *>("REPLAYGAIN_ALBUM_PEAK"), value);
		}

		// extra tags mode -s e or -s l
		if (mode == 'e' || mode == 'l') {
			snprintf(value, sizeof(value), "%.2f LUFS", scan -> loudness_reference);
			tag_add_txxx(tag, const_cast<char *>("REPLAYGAIN_REFERENCE_LOUDNESS"), value);

			snprintf(value, sizeof(value), "%.2f %s", scan -> track_loudness_range, unit);
			tag_add_txxx(tag, const_cast<char *>("REPLAYGAIN_TRACK_RANGE"), value);

			if (do_album) {
				snprintf(value, sizeof(value), "%.2f %s", scan -> album_loudness_range, unit);
				tag_add_txxx(tag, const_cast<char *>("REPLAYGAIN_ALBUM_RANGE"), value);
			}
		}
	}

	// work around bug taglib/taglib#913: strip APE before ID3v1
	if (strip)
		f.strip(TagLib::MPEG::File::APE);

	return f.save(TagLib::MPEG::File::ID3v2, strip, id3v2version);
}

bool tag_clear_mp3(scan_result *scan, bool strip, int id3v2version) {
	TagLib::MPEG::File f(scan -> file);
	TagLib::ID3v2::Tag *tag = f.ID3v2Tag(true);

	tag_remove_mp3(tag);

	// work around bug taglib/taglib#913: strip APE before ID3v1
	if (strip)
		f.strip(TagLib::MPEG::File::APE);

	return f.save(TagLib::MPEG::File::ID3v2, strip, id3v2version);
}


/*** FLAC ****/

void tag_remove_flac(TagLib::Ogg::XiphComment *tag) {
	tag -> removeField("REPLAYGAIN_TRACK_GAIN");
	tag -> removeField("REPLAYGAIN_TRACK_PEAK");
	tag -> removeField("REPLAYGAIN_TRACK_RANGE");
	tag -> removeField("REPLAYGAIN_ALBUM_GAIN");
	tag -> removeField("REPLAYGAIN_ALBUM_PEAK");
	tag -> removeField("REPLAYGAIN_ALBUM_RANGE");
	tag -> removeField("REPLAYGAIN_REFERENCE_LOUDNESS");
}

bool tag_write_flac(scan_result *scan, bool do_album, char mode, char *unit) {
	char value[2048];

	TagLib::FLAC::File f(scan -> file);
	TagLib::Ogg::XiphComment *tag = f.xiphComment(true);

	// remove old tags before writing new ones
	tag_remove_flac(tag);

	snprintf(value, sizeof(value), "%.2f %s", scan -> track_gain, unit);
	tag -> addField("REPLAYGAIN_TRACK_GAIN", value);

	snprintf(value, sizeof(value), "%.6f", scan -> track_peak);
	tag -> addField("REPLAYGAIN_TRACK_PEAK", value);

	// Only write album tags if in album mode (would be zero otherwise)
	if (do_album) {
		snprintf(value, sizeof(value), "%.2f %s", scan -> album_gain, unit);
		tag -> addField("REPLAYGAIN_ALBUM_GAIN", value);

		snprintf(value, sizeof(value), "%.6f", scan -> album_peak);
		tag -> addField("REPLAYGAIN_ALBUM_PEAK", value);
	}

	// extra tags mode -s e or -s l
	if (mode == 'e' || mode == 'l') {
		snprintf(value, sizeof(value), "%.2f LUFS", scan -> loudness_reference);
		tag -> addField("REPLAYGAIN_REFERENCE_LOUDNESS", value);

		snprintf(value, sizeof(value), "%.2f %s", scan -> track_loudness_range, unit);
		tag -> addField("REPLAYGAIN_TRACK_RANGE", value);

		if (do_album) {
			snprintf(value, sizeof(value), "%.2f %s", scan -> album_loudness_range, unit);
			tag -> addField("REPLAYGAIN_ALBUM_RANGE", value);
		}
	}

	return f.save();
}

bool tag_clear_flac(scan_result *scan) {
	TagLib::FLAC::File f(scan -> file);
	TagLib::Ogg::XiphComment *tag = f.xiphComment(true);

	tag_remove_flac(tag);

	return f.save();
}


/*** Ogg Vorbis ****/

void tag_remove_vorbis(TagLib::Ogg::XiphComment *tag) {
	tag -> removeField("REPLAYGAIN_TRACK_GAIN");
	tag -> removeField("REPLAYGAIN_TRACK_PEAK");
	tag -> removeField("REPLAYGAIN_TRACK_RANGE");
	tag -> removeField("REPLAYGAIN_ALBUM_GAIN");
	tag -> removeField("REPLAYGAIN_ALBUM_PEAK");
	tag -> removeField("REPLAYGAIN_ALBUM_RANGE");
	tag -> removeField("REPLAYGAIN_REFERENCE_LOUDNESS");
}

bool tag_write_vorbis(scan_result *scan, bool do_album, char mode, char *unit) {
	char value[2048];

	TagLib::Ogg::Vorbis::File f(scan -> file);
	TagLib::Ogg::XiphComment *tag = f.tag();

	// remove old tags before writing new ones
	tag_remove_vorbis(tag);

	snprintf(value, sizeof(value), "%.2f %s", scan -> track_gain, unit);
	tag -> addField("REPLAYGAIN_TRACK_GAIN", value);

	snprintf(value, sizeof(value), "%.6f", scan -> track_peak);
	tag -> addField("REPLAYGAIN_TRACK_PEAK", value);

	// Only write album tags if in album mode (would be zero otherwise)
	if (do_album) {
		snprintf(value, sizeof(value), "%.2f %s", scan -> album_gain, unit);
		tag -> addField("REPLAYGAIN_ALBUM_GAIN", value);

		snprintf(value, sizeof(value), "%.6f", scan -> album_peak);
		tag -> addField("REPLAYGAIN_ALBUM_PEAK", value);
	}

	// extra tags mode -s e or -s l
	if (mode == 'e' || mode == 'l') {
		snprintf(value, sizeof(value), "%.2f LUFS", scan -> loudness_reference);
		tag -> addField("REPLAYGAIN_REFERENCE_LOUDNESS", value);

		snprintf(value, sizeof(value), "%.2f %s", scan -> track_loudness_range, unit);
		tag -> addField("REPLAYGAIN_TRACK_RANGE", value);

		if (do_album) {
			snprintf(value, sizeof(value), "%.2f %s", scan -> album_loudness_range, unit);
			tag -> addField("REPLAYGAIN_ALBUM_RANGE", value);
		}
	}

	return f.save();
}

bool tag_clear_vorbis(scan_result *scan) {
	TagLib::Ogg::Vorbis::File f(scan -> file);
	TagLib::Ogg::XiphComment *tag = f.tag();

	tag_remove_vorbis(tag);

	return f.save();
}


/*** MP4 ****/

void tag_remove_mp4(TagLib::MP4::Tag *tag) {
	tag -> removeItem("----:com.apple.iTunes:REPLAYGAIN_TRACK_GAIN");
	tag -> removeItem("----:com.apple.iTunes:REPLAYGAIN_TRACK_PEAK");
	tag -> removeItem("----:com.apple.iTunes:REPLAYGAIN_TRACK_RANGE");
	tag -> removeItem("----:com.apple.iTunes:REPLAYGAIN_ALBUM_GAIN");
	tag -> removeItem("----:com.apple.iTunes:REPLAYGAIN_ALBUM_PEAK");
	tag -> removeItem("----:com.apple.iTunes:REPLAYGAIN_ALBUM_RANGE");
	tag -> removeItem("----:com.apple.iTunes:REPLAYGAIN_REFERENCE_LOUDNESS");

	tag -> removeItem("----:com.apple.iTunes:replaygain_track_gain");
	tag -> removeItem("----:com.apple.iTunes:replaygain_track_peak");
	tag -> removeItem("----:com.apple.iTunes:replaygain_track_range");
	tag -> removeItem("----:com.apple.iTunes:replaygain_album_gain");
	tag -> removeItem("----:com.apple.iTunes:replaygain_album_peak");
	tag -> removeItem("----:com.apple.iTunes:replaygain_album_range");
	tag -> removeItem("----:com.apple.iTunes:replaygain_reference_loudness");
}

bool tag_write_mp4(scan_result *scan, bool do_album, char mode, char *unit,
	bool lowercase) {
	char value[2048];

	TagLib::MP4::File f(scan -> file);
	TagLib::MP4::Tag *tag = f.tag();

	// remove old tags before writing new ones
	tag_remove_mp4(tag);

	if (lowercase) {
		// use lowercase replaygain tags
		// (non-standard but used by foobar2000 and needed by some players)
		snprintf(value, sizeof(value), "%.2f %s", scan -> track_gain, unit);
		tag -> setItem("----:com.apple.iTunes:replaygain_track_gain", TagLib::StringList(value));

		snprintf(value, sizeof(value), "%.6f", scan -> track_peak);
		tag -> setItem("----:com.apple.iTunes:replaygain_track_peak", TagLib::StringList(value));

		// Only write album tags if in album mode (would be zero otherwise)
		if (do_album) {
			snprintf(value, sizeof(value), "%.2f %s", scan -> album_gain, unit);
			tag -> setItem("----:com.apple.iTunes:replaygain_album_gain", TagLib::StringList(value));

			snprintf(value, sizeof(value), "%.6f", scan -> album_peak);
			tag -> setItem("----:com.apple.iTunes:replaygain_album_peak", TagLib::StringList(value));
		}

		// extra tags mode -s e or -s l
		if (mode == 'e' || mode == 'l') {
			snprintf(value, sizeof(value), "%.2f LUFS", scan -> loudness_reference);
			tag -> setItem("----:com.apple.iTunes:replaygain_reference_loudness", TagLib::StringList(value));

			snprintf(value, sizeof(value), "%.2f %s", scan -> track_loudness_range, unit);
			tag -> setItem("----:com.apple.iTunes:replaygain_track_range", TagLib::StringList(value));

			if (do_album) {
				snprintf(value, sizeof(value), "%.2f %s", scan -> album_loudness_range, unit);
				tag -> setItem("----:com.apple.iTunes:replaygain_album_range", TagLib::StringList(value));
			}
		}
	} else {
		// use standard-compliant uppercase replaygain tags (default)
		snprintf(value, sizeof(value), "%.2f %s", scan -> track_gain, unit);
		tag -> setItem("----:com.apple.iTunes:REPLAYGAIN_TRACK_GAIN", TagLib::StringList(value));

		snprintf(value, sizeof(value), "%.6f", scan -> track_peak);
		tag -> setItem("----:com.apple.iTunes:REPLAYGAIN_TRACK_PEAK", TagLib::StringList(value));

		// Only write album tags if in album mode (would be zero otherwise)
		if (do_album) {
			snprintf(value, sizeof(value), "%.2f %s", scan -> album_gain, unit);
			tag -> setItem("----:com.apple.iTunes:REPLAYGAIN_ALBUM_GAIN", TagLib::StringList(value));

			snprintf(value, sizeof(value), "%.6f", scan -> album_peak);
			tag -> setItem("----:com.apple.iTunes:REPLAYGAIN_ALBUM_PEAK", TagLib::StringList(value));
		}

		// extra tags mode -s e or -s l
		if (mode == 'e' || mode == 'l') {
			snprintf(value, sizeof(value), "%.2f LUFS", scan -> loudness_reference);
			tag -> setItem("----:com.apple.iTunes:REPLAYGAIN_REFERENCE_LOUDNESS", TagLib::StringList(value));

			snprintf(value, sizeof(value), "%.2f %s", scan -> track_loudness_range, unit);
			tag -> setItem("----:com.apple.iTunes:REPLAYGAIN_TRACK_RANGE", TagLib::StringList(value));

			if (do_album) {
				snprintf(value, sizeof(value), "%.2f %s", scan -> album_loudness_range, unit);
				tag -> setItem("----:com.apple.iTunes:REPLAYGAIN_ALBUM_RANGE", TagLib::StringList(value));
			}
		}
	}

	return f.save();
}

bool tag_clear_mp4(scan_result *scan) {
	TagLib::MP4::File f(scan -> file);
	TagLib::MP4::Tag *tag = f.tag();

	tag_remove_mp4(tag);

	return f.save();
}


/*** Opus ****/

// Opus Notes:
//
// 1. Opus ONLY uses R128_TRACK_GAIN and (optionally) R128_ALBUM_GAIN
//    as an ADDITIONAL offset to the header's 'output_gain'.
// 2. Encoders and muxes set 'output_gain' to zero, so a non-zero 'output_gain' in
//    the header i supposed to be a change AFTER encoding/muxing.
// 3. We assume that FFmpeg's avformat does already apply 'output_gain' (???)
//    so we get get pre-gained data and only have to calculate the difference.
// 4. Opus adheres to EBU-R128, so the loudness reference is ALWAYS -23 LUFS.
//    This means we have to adapt for possible `-d n` (`--pregain=n`) changes.
//    This also means players have to add an extra +5 dB to reach the loudness
//    ReplayGain 2.0 prescribes (-18 LUFS).
// 5. Opus R128_* tags use ASCII-encoded Q7.8 numbers with max. 6 places including
//    the minus sign, and no unit.
//    See https://en.wikipedia.org/wiki/Q_(number_format)
// 6. RFC 7845 states: "To avoid confusion with multiple normalization schemes, an
//    Opus comment header SHOULD NOT contain any of the REPLAYGAIN_TRACK_GAIN,
//    REPLAYGAIN_TRACK_PEAK, REPLAYGAIN_ALBUM_GAIN, or REPLAYGAIN_ALBUM_PEAK tags, […]"
//    So we remove REPLAYGAIN_* tags if any are present.
// 7. RFC 7845 states: "Peak normalizations are difficult to calculate reliably
//    for lossy codecs because of variation in excursion heights due to decoder
//    differences. In the authors' investigations, they were not applied
//    consistently or broadly enough to merit inclusion here."
//    So there are NO "Peak" type tags. The (oversampled) true peak levels that
//    libebur128 calculates for us are STILL used for clipping prevention if so
//    requested. They are also shown in the output, just not stored into tags.

int gain_to_q78num(double gain) {
	// convert float to Q7.8 number: Q = round(f * 2^8)
	return (int) round(gain * 256.0);		// 2^8 = 256
}

void tag_remove_opus(TagLib::Ogg::XiphComment *tag) {
	// RFC 7845 states:
	// To avoid confusion with multiple normalization schemes, an Opus
  // comment header SHOULD NOT contain any of the REPLAYGAIN_TRACK_GAIN,
  // REPLAYGAIN_TRACK_PEAK, REPLAYGAIN_ALBUM_GAIN, or
  // REPLAYGAIN_ALBUM_PEAK tags, […]"
	// so we remove these if present
	tag -> removeField("REPLAYGAIN_TRACK_GAIN");
	tag -> removeField("REPLAYGAIN_TRACK_PEAK");
	tag -> removeField("REPLAYGAIN_TRACK_RANGE");
	tag -> removeField("REPLAYGAIN_ALBUM_GAIN");
	tag -> removeField("REPLAYGAIN_ALBUM_PEAK");
	tag -> removeField("REPLAYGAIN_ALBUM_RANGE");
	tag -> removeField("REPLAYGAIN_REFERENCE_LOUDNESS");
	tag -> removeField("R128_TRACK_GAIN");
	tag -> removeField("R128_ALBUM_GAIN");
}

bool tag_write_opus(scan_result *scan, bool do_album, char mode, char *unit) {
	char value[2048];

	TagLib::Ogg::Opus::File f(scan -> file);
	TagLib::Ogg::XiphComment *tag = f.tag();

	// remove old tags before writing new ones
	tag_remove_opus(tag);

	snprintf(value, sizeof(value), "%d", gain_to_q78num(scan -> track_gain));
	tag -> addField("R128_TRACK_GAIN", value);

	// Only write album tags if in album mode (would be zero otherwise)
	if (do_album) {
		snprintf(value, sizeof(value), "%d", gain_to_q78num(scan -> album_gain));
		tag -> addField("R128_ALBUM_GAIN", value);
	}

	// extra tags mode -s e or -s l
	// no extra tags allowed in Opus

	return f.save();
}

bool tag_clear_opus(scan_result *scan) {
	TagLib::Ogg::Opus::File f(scan -> file);
	TagLib::Ogg::XiphComment *tag = f.tag();

	tag_remove_opus(tag);

	return f.save();
}

/*** ASF/WMA ****/

void tag_remove_asf(TagLib::ASF::Tag *tag) {
	tag -> removeItem("REPLAYGAIN_TRACK_GAIN");
	tag -> removeItem("REPLAYGAIN_TRACK_PEAK");
	tag -> removeItem("REPLAYGAIN_TRACK_RANGE");
	tag -> removeItem("REPLAYGAIN_ALBUM_GAIN");
	tag -> removeItem("REPLAYGAIN_ALBUM_PEAK");
	tag -> removeItem("REPLAYGAIN_ALBUM_RANGE");
	tag -> removeItem("REPLAYGAIN_REFERENCE_LOUDNESS");

	tag -> removeItem("replaygain_track_gain");
	tag -> removeItem("replaygain_track_peak");
	tag -> removeItem("replaygain_track_range");
	tag -> removeItem("replaygain_album_gain");
	tag -> removeItem("replaygain_album_peak");
	tag -> removeItem("replaygain_album_range");
	tag -> removeItem("replaygain_reference_loudness");
}

bool tag_write_asf(scan_result *scan, bool do_album, char mode, char *unit,
	bool lowercase) {
	char value[2048];

	TagLib::ASF::File f(scan -> file);
	TagLib::ASF::Tag *tag = f.tag();

	// remove old tags before writing new ones
	tag_remove_asf(tag);

	if (lowercase) {
		// use lowercase replaygain tags
		// (non-standard but used by foobar2000 & WinAmp and needed by some players)
		snprintf(value, sizeof(value), "%.2f %s", scan -> track_gain, unit);
		tag -> setAttribute("replaygain_track_gain", TagLib::String(value));

		snprintf(value, sizeof(value), "%.6f", scan -> track_peak);
		tag -> setAttribute("replaygain_track_peak", TagLib::String(value));

		// Only write album tags if in album mode (would be zero otherwise)
		if (do_album) {
			snprintf(value, sizeof(value), "%.2f %s", scan -> album_gain, unit);
			tag -> setAttribute("replaygain_album_gain", TagLib::String(value));

			snprintf(value, sizeof(value), "%.6f", scan -> album_peak);
			tag -> setAttribute("replaygain_album_peak", TagLib::String(value));
		}

		// extra tags mode -s e or -s l
		if (mode == 'e' || mode == 'l') {
			snprintf(value, sizeof(value), "%.2f LUFS", scan -> loudness_reference);
			tag -> setAttribute("replaygain_reference_loudness", TagLib::String(value));

			snprintf(value, sizeof(value), "%.2f %s", scan -> track_loudness_range, unit);
			tag -> setAttribute("replaygain_track_range", TagLib::String(value));

			if (do_album) {
				snprintf(value, sizeof(value), "%.2f %s", scan -> album_loudness_range, unit);
				tag -> setAttribute("replaygain_album_range", TagLib::String(value));
			}
		}
	} else {
		// use standard-compliant uppercase replaygain tags (default)
		snprintf(value, sizeof(value), "%.2f %s", scan -> track_gain, unit);
		tag -> setAttribute("REPLAYGAIN_TRACK_GAIN", TagLib::String(value));

		snprintf(value, sizeof(value), "%.6f", scan -> track_peak);
		tag -> setAttribute("REPLAYGAIN_TRACK_PEAK", TagLib::String(value));

		// Only write album tags if in album mode (would be zero otherwise)
		if (do_album) {
			snprintf(value, sizeof(value), "%.2f %s", scan -> album_gain, unit);
			tag -> setAttribute("REPLAYGAIN_ALBUM_GAIN", TagLib::String(value));

			snprintf(value, sizeof(value), "%.6f", scan -> album_peak);
			tag -> setAttribute("REPLAYGAIN_ALBUM_PEAK", TagLib::String(value));
		}

		// extra tags mode -s e or -s l
		if (mode == 'e' || mode == 'l') {
			snprintf(value, sizeof(value), "%.2f LUFS", scan -> loudness_reference);
			tag -> setAttribute("REPLAYGAIN_REFERENCE_LOUDNESS", TagLib::String(value));

			snprintf(value, sizeof(value), "%.2f %s", scan -> track_loudness_range, unit);
			tag -> setAttribute("REPLAYGAIN_TRACK_RANGE", TagLib::String(value));

			if (do_album) {
				snprintf(value, sizeof(value), "%.2f %s", scan -> album_loudness_range, unit);
				tag -> setAttribute("REPLAYGAIN_ALBUM_RANGE", TagLib::String(value));
			}
		}
	}

	return f.save();
}

bool tag_clear_asf(scan_result *scan) {
	TagLib::ASF::File f(scan -> file);
	TagLib::ASF::Tag *tag = f.tag();

	tag_remove_asf(tag);

	return f.save();
}
