/*
Copyright (C) 2003  The Pentagram Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "pent_include.h"
#include "MT32EmuMidiDriver.h"

#ifdef USE_MT32EMU_MIDI

#include "mt32emu/synth.h"
#include "mt32emu/mt32_file.h"

#include <cstring>

#include "XMidiFile.h"
#include "XMidiEvent.h"
#include "XMidiEventList.h"

#ifdef PENTAGRAM_IN_EXULT
#include "databuf.h"
#include "utils.h"
#else
#include "IDataSource.h"
#include "FileSystem.h"
#endif

using namespace Pentagram;

static void nullprintDebug(void *userData, const char *fmt, va_list list)
{
}


const MidiDriver::MidiDriverDesc MT32EmuMidiDriver::desc = 
		MidiDriver::MidiDriverDesc ("MT32Emu", createInstance);

MT32EmuMidiDriver::MT32EmuMidiDriver() :
	LowLevelMidiDriver()
{
}

/*
 *	This file open proc redirects writes to a writable directory
 *	and looks in this and other directories for the MT32 data.
 */

static File *openFileProc
	(
	void *userData,
	const char *filename,
	File::OpenMode mode
	)
	{
	ANSIFile *file = new ANSIFile();
	std::string basedir;
	if (mode == File::OpenMode_read)
		{
#ifdef MACOSX
		// May be in bundle.
		basedir = std::string("<BUNDLE>/") + filename;
		if (file->open(get_system_path(basedir).c_str(), mode))
			return file;
#endif
		// Now try data dir.
		basedir = std::string("<DATA>/") + filename;
		if (file->open(get_system_path(basedir).c_str(), mode))
			return file;
		// We now fall back to a writable data dir, as the emulator
		// may have written something there.
		}
	basedir = std::string("<SAVEHOME>/data/") + filename;
	if (file->open(get_system_path(basedir).c_str(), mode))
		return file;
	// Nowhere we know about.
	delete file;
	return NULL;
	}

int MT32EmuMidiDriver::open()
{
	// Must be stereo
	if (!stereo) return 1;

	// Make sure dir exists; this is the dir where data will be saved.
	U7mkdir("<SAVEHOME>/data", 0755);

	SynthProperties	props;
	std::memset(&props,0,sizeof(props));

	props.sampleRate = sample_rate;
#ifdef DEBUG
	props.useReverb = false;
#else
	props.useReverb = true;
#endif
	props.useDefaultReverb = true;
	props.openFile = openFileProc;
	props.printDebug = nullprintDebug;

	mt32 = new Synth;

	if (!mt32->open(props)) {
		delete mt32;
		mt32 = 0;
		return 1;
	}

	return 0;
}

void MT32EmuMidiDriver::close()
{
	if (mt32) {
		mt32->close();
		delete mt32;
		mt32 = 0;
	}
}

void MT32EmuMidiDriver::send(uint32 b)
{
	mt32->playMsg(b);
}

void MT32EmuMidiDriver::send_sysex(uint8 status, const uint8 *msg, uint16 length)
{
	if (!msg || !length) return;

	if (status != 0xF0 && msg[length-1] != 0xF7) return;

	mt32->playSysexWithoutFraming(msg, length-1);
}

void MT32EmuMidiDriver::lowLevelProduceSamples(sint16 *samples, uint32 num_samples)
{
	mt32->render(samples,num_samples);
}

#endif //USE_MT32Emu_MIDI
