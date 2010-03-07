/*
Copyright (C) 2005 The Pentagram team
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

#include "pent_include.h"
#include "RawAudioSample.h"
#include "databuf.h"

namespace Pentagram {

RawAudioSample::RawAudioSample(uint8* buffer_, uint32 size_, uint32 rate_,
							   bool signeddata_, bool stereo_)
	: AudioSample(buffer_, size_), signeddata(signeddata_)
{
	sample_rate = rate_;
	bits = 8;
	stereo = stereo_;
	frame_size = 512;
	decompressor_size = sizeof(RawDecompData);
	length = size_;
	start_pos = 0;
	byte_swap = false;
}

RawAudioSample::~RawAudioSample()
{

}

void RawAudioSample::initDecompressor(void *DecompData) const
{
	RawDecompData *decomp = reinterpret_cast<RawDecompData *>(DecompData);
	decomp->pos = start_pos;
}

void RawAudioSample::rewind(void *DecompData) const
{
	initDecompressor(DecompData);
}

uint32 RawAudioSample::decompressFrame(void *DecompData, void *samples) const
{
	RawDecompData *decomp = reinterpret_cast<RawDecompData *>(DecompData);

	if (decomp->pos == buffer_size) return 0;

	uint32 count = frame_size;
	if (decomp->pos + count > buffer_size)
		count = buffer_size - decomp->pos;

	// 8 bit unsigned, or 16 Bit signed
	if ((!signeddata && bits==8) || (signeddata && bits==16 && !byte_swap)) {	
		std::memcpy(samples, buffer+decomp->pos, count*(bits/16));
		decomp->pos += count;
	// 8 bit signed data
	} else if (bits == 8) {
		uint8* dest = static_cast<uint8*>(samples);
		for (unsigned int i = 0; i < count; ++i)
			*dest++ = buffer[decomp->pos++] + 128;
	}
	// 16 bit signed with byte swap required
	else if (signeddata && bits==16 && byte_swap) {
		uint8 *dest = static_cast<uint8*>(samples);
		for (unsigned int i = 0; i < count; ++i) {
			*dest++ = buffer[++decomp->pos];
			*dest++ = buffer[(decomp->pos++)-1];
		}
	}

	return count;
}



}
