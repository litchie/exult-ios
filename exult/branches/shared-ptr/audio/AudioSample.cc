/*
Copyright (C) 2005 The Pentagram team
Copyright (C) 2010-2011 The Exult Team

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
#include "AudioSample.h"
#include "OggAudioSample.h"
#include "WavAudioSample.h"
#include "VocAudioSample.h"

namespace Pentagram {

AudioSample::AudioSample(uint8 *buffer_, uint32 size_) : 
		sample_rate(0), bits(0), stereo(false), 
		frame_size(0), decompressor_size(0), length(0), 
		buffer_size(size_), buffer(buffer_), refcount(1)
{
}

AudioSample::~AudioSample(void)
{
	delete [] buffer;
}

AudioSample *AudioSample::createAudioSample(uint8 *data, uint32 size)
{
	BufferDataSource ds(data,size);

	if (VocAudioSample::isThis(&ds))
	{
		return new VocAudioSample(data,size);
	}
	else if (WavAudioSample::isThis(&ds))
	{
		return new WavAudioSample(data,size);
	}
	else if (OggAudioSample::isThis(&ds))
	{
		return new OggAudioSample(data,size);
	}
	else
	{
		delete [] data;
	}

	return 0;
}

};
