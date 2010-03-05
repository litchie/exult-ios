/*
Copyright (C) 2005 The Pentagram team

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
	virtual ~OggAudioSample();

	virtual void initDecompressor(void *DecompData) const;
	virtual uint32 decompressFrame(void *DecompData, void *samples) const;
	virtual void rewind(void *DecompData) const;

	ov_callbacks callbacks;

	static size_t read_func  (void *ptr, size_t size, size_t nmemb, void *datasource);

protected:

	struct OggDecompData {
		OggVorbis_File	ov;
		int bitstream;
		int last_rate;
		bool last_stereo;
	};

	IDataSource *oggdata;
	bool locked;
};

}

#endif
