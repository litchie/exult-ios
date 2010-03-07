/*
Copyright (C) 2010 The Exult team

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
#ifndef OggAudioSample_H
#define OggAudioSample_H

#include "AudioSample.h"
#include <vorbis/vorbisfile.h>

namespace Pentagram {

// FIXME - OggAudioSample doesn't support playing the same sample on more than 
// one channel at the same time. Its really intended only for music playing 
// support

class OggAudioSample : public AudioSample
{
public:
	OggAudioSample(IDataSource *oggdata);
	OggAudioSample(uint8 *buffer, uint32 size);
	virtual ~OggAudioSample();

	virtual void initDecompressor(void *DecompData) const;
	virtual uint32 decompressFrame(void *DecompData, void *samples) const;
	virtual void rewind(void *DecompData) const;
	virtual void freeDecompressor(void *DecompData) const;

	static ov_callbacks callbacks;
	
	static size_t read_func  (void *ptr, size_t size, size_t nmemb, void *datasource);
	static int    seek_func  (void *datasource, ogg_int64_t offset, int whence);
    static long   tell_func  (void *datasource);

	static bool   isThis(IDataSource *oggdata);

protected:

	struct OggDecompData {
		OggVorbis_File	ov;
		int bitstream;
		int last_rate;
		bool last_stereo;
		IDataSource *datasource;
	};

	IDataSource *oggdata;
	bool locked;
};

}

#endif
