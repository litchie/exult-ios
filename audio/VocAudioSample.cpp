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

#include "pent_include.h"
#include "VocAudioSample.h"

#define	TRAILING_VOC_SLOP 32
#define	LEADING_VOC_SLOP 32

namespace Pentagram {

VocAudioSample::VocAudioSample(uint8* buffer_, uint32 size_)
	: AudioSample(buffer_, size_)
{
	bool	last_chunk=false;

	size_t	data_offset=0x1a;
	size_t  l = 0;
	int		compression = 0;
	int		adpcm_reference = -1;
	int		adpcm_scale = 0;

	// Parse the file, get the 'important' details
	sample_rate = 0;
	frame_size = 0;
	length = 0;
	while (!last_chunk && data_offset < size_)
	{
		int chunk_length;
		switch(buffer[data_offset++])
		{
		case 0:
			COUT("Terminator");
			last_chunk = true;
			continue;

		case 1:
			COUT("Sound data");
			l = (buffer[2+data_offset])<<16;
			l |= (buffer[1+data_offset])<<8;
			l |= (buffer[0+data_offset]);
			data_offset+=3;
			COUT("Chunk length appears to be " << l);
			if (!sample_rate)
			{
				sample_rate=1000000/(256-(buffer[data_offset]));
#ifdef FUDGE_SAMPLE_RATES
				if (sample_rate == 11111) sample_rate = 11025;
				else if (sample_rate == 22222) sample_rate = 22050;
#endif
				COUT("Sample rate ("<< sample_rate<<") = _real_rate");
			}
			compression = buffer[1+data_offset];
			COUT("compression type " << compression);
			if (compression) {
				adpcm_reference = -1;
			}

			l -= 2;
			chunk_length = l;
			data_offset += 2;

			break;
		case 2:
			COUT("Sound continues");
			l=(buffer[2+data_offset])<<16;
			l|=(buffer[1+data_offset])<<8;
			l|=(buffer[0+data_offset]);
			data_offset+=3;
			COUT("Chunk length appears to be " << l);
			chunk_length = l;
			break;
		case 3:
			COUT("Silence");
			l=(buffer[1+data_offset])<<8;
			l|=(buffer[0+data_offset]);
			l++;
			if (!sample_rate)
			{
				sample_rate=1000000/(256-(buffer[2+data_offset]));
#ifdef FUDGE_SAMPLE_RATES
				if (sample_rate == 11111) sample_rate = 11025;
				else if (sample_rate == 22222) sample_rate = 22050;
#endif
				COUT("Sample rate ("<< sample_rate<<") = _real_rate");
			}
			data_offset+=3;
			chunk_length=0;
			break;

		default:
			COUT("Other chunk type");
			l=(buffer[2+data_offset])<<16;
			l|=(buffer[1+data_offset])<<8;
			l|=(buffer[0+data_offset]);
			data_offset+=3;
			COUT("Chunk length appears to be " << l);
			chunk_length = l;
			l = 0;
			break;
		}

		if(chunk_length==0)
			break;

		if (l)
		{
			size_t dec_len = l;

			// Decompress data
			if (compression == 1) {
				if (adpcm_reference == -1) dec_len = (dec_len-1)*2;
				else dec_len *= 2;
				adpcm_reference = 0;
			}
			else if (compression != 0) {
				CERR("Can't handle VOC compression type"); 
			}

			if (dec_len > frame_size) frame_size = dec_len;
			length += dec_len;
		}

		data_offset += chunk_length;
	}

	bits = 8;
	stereo = false;
	decompressor_size = sizeof(VocDecompData);
	length = size_;
}

VocAudioSample::~VocAudioSample()
{

}

void VocAudioSample::initDecompressor(void *DecompData) const
{
	VocDecompData *decomp = reinterpret_cast<VocDecompData *>(DecompData);
	decomp->pos = 0x1a;
	decomp->compression = 0;
	decomp->adpcm_reference = -1;
	decomp->adpcm_scale = 0;
}

void VocAudioSample::rewind(void *DecompData) const
{
	initDecompressor(DecompData);
}

//
// Decode 4bit ADPCM vocs (thunder in SI intro)
//
// Code grabbed from VDMS
//

inline int VocAudioSample::decode_ADPCM_4_sample(uint8 sample,
	int& reference,
	int& scale)
{
	static int scaleMap[8] = { -2, -1, 0, 0, 1, 1, 1, 1 };

	if (sample & 0x08) {
		reference = max(0x00, reference - ((sample & 0x07) << scale));
	} else {
		reference = min(0xff, reference + ((sample & 0x07) << scale));
	}

	scale = max(2, min(6, scaleMap[sample & 0x07]));

	return reference;
}

//
// Performs 4-bit ADPCM decoding in-place.
//
void VocAudioSample::decode_ADPCM_4(uint8* inBuf,	
	int bufSize,				// Size of inbuf
	uint8* outBuf,			// Size is 2x bufsize
	int& reference,			// ADPCM reference value
	int& scale)
{
	if (reference < 0) {
		reference = inBuf[0] & 0xff;   // use the first byte in the buffer as the reference byte
		bufSize--;                          // remember to skip the reference byte
	}

	for (int i = 0; i < bufSize; i++) {
		outBuf[i * 2 + 0] = decode_ADPCM_4_sample(inBuf[i] >> 4, reference, scale);
		outBuf[i * 2 + 1] = decode_ADPCM_4_sample(inBuf[i] >> 0, reference, scale);
	}
}

uint32 VocAudioSample::decompressFrame(void *DecompData, void *samples) const
{
	VocDecompData *decomp = reinterpret_cast<VocDecompData *>(DecompData);

	if (decomp->pos == buffer_size) return 0;

	size_t  l = 0;
	size_t chunk_length = 0;

	// Look at the chunk type
	switch (buffer[decomp->pos++])
	{
	case 0:
		return 0;

	case 1: // Sound data
		l = (buffer[2+decomp->pos])<<16;
		l |= (buffer[1+decomp->pos])<<8;
		l |= (buffer[0+decomp->pos]);
		decomp->pos += 3;
		decomp->compression = buffer[1+decomp->pos];

		if (decomp->compression) decomp->adpcm_reference = -1;

		l -= 2;
		chunk_length = l;
		decomp->pos += 2;

		break;
	case 2: // Sound continue
		l=(buffer[2+decomp->pos])<<16;
		l|=(buffer[1+decomp->pos])<<8;
		l|=(buffer[0+decomp->pos]);
		decomp->pos += 3;
		chunk_length = l;
		break;

	case 3:	// Silence
		l  =(buffer[1+decomp->pos])<<8;
		l |=(buffer[0+decomp->pos]);
		l++;
		decomp->pos += 3;
		chunk_length=0;
		break;

		// Skip all other chunk types
	default:
		l=(buffer[2+decomp->pos])<<16;
		l|=(buffer[1+decomp->pos])<<8;
		l|=(buffer[0+decomp->pos]);
		decomp->pos += 3 + l;
		return decompressFrame(DecompData,samples); 
	};

	// This is just silence
	if (!chunk_length)
	{
		std::memset(samples,0,l);
	}
	else if (decomp->compression == 0)
	{
		std::memcpy(samples, buffer+decomp->pos, l);
	}
	else if (decomp->compression == 1)
	{
		size_t dec_len = l;
		if (decomp->adpcm_reference == -1) dec_len = (dec_len-1)*2;
		else dec_len *= 2;		
		decode_ADPCM_4(buffer+decomp->pos, l, (uint8*)samples, decomp->adpcm_reference, decomp->adpcm_scale);
		l = dec_len;
	}
	else
	{
		// Unhandled chunk types set to silence
		std::memset(samples,0,l);
	}
	decomp->pos += chunk_length;
	return l;
}


bool VocAudioSample::isVoc(IDataSource *ds)
{
	char buffer[19];
	ds->read(buffer,19);

	if(!strncmp(buffer,"Creative Voice File",19))
		return true;

	return false;
}


}
