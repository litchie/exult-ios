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
#include "OggAudioSample.h"
//#include "databuf.h"

namespace Pentagram {

OggAudioSample::OggAudioSample(IDataSource *oggdata_)
	: AudioSample(0, 0), oggdata(oggdata_)
{
	frame_size = 4096;
	decompressor_size = sizeof(OggDecompData);
	bits = 16;

	callbacks.read_func = &read_func;
	callbacks.seek_func = 0;//&seek_func;
	callbacks.close_func = 0;//&close_func;
	callbacks.tell_func = 0;//&tell_func;

}

OggAudioSample::~OggAudioSample()
{
	delete oggdata;
	oggdata = 0;
}

size_t OggAudioSample::read_func  (void *ptr, size_t size, size_t nmemb, void *datasource)
{
	OggAudioSample *oas = (OggAudioSample*) datasource;
	if (oas->oggdata->eof()) return 0;
	oas->oggdata->read(ptr,size*nmemb);
	return nmemb;
}
void OggAudioSample::initDecompressor(void *DecompData) const
{
	OggDecompData *decomp = reinterpret_cast<OggDecompData *>(DecompData);

	if (locked) 
		throw exult_exception("Attempted to play OggAudioSample on more than one channel at the same time.");

	*const_cast<bool*>(&locked) = true;		

	oggdata->seek(0);
	ov_open_callbacks((void*)this,&decomp->ov,NULL,0,callbacks);
	decomp->bitstream = 0;
	
	vorbis_info *info = ov_info(&decomp->ov,-1);
	decomp->last_rate = info->rate;
	decomp->last_stereo = info->channels == 2;
}

void OggAudioSample::rewind(void *DecompData) const
{
	OggDecompData *decomp = reinterpret_cast<OggDecompData *>(DecompData);
	ov_clear(&decomp->ov);
	initDecompressor(DecompData);
}

uint32 OggAudioSample::decompressFrame(void *DecompData, void *samples) const
{
	OggDecompData *decomp = reinterpret_cast<OggDecompData *>(DecompData);

	vorbis_info *info = ov_info(&decomp->ov,-1);

	if (info == 0) return 0;

	*const_cast<uint32*>(&sample_rate) = decomp->last_rate;
	*const_cast<bool*>(&stereo) = decomp->last_stereo;
	decomp->last_rate = info->rate;
	decomp->last_stereo = info->channels == 2;

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	const int bigendianp = 0;
#else
	const int bigendianp = 1;
#endif

	long count = ov_read(&decomp->ov,(char*)samples,frame_size,bigendianp,2,1,&decomp->bitstream);

	//if (count == OV_EINVAL || count == 0) {
	if (count <= 0) {
		*const_cast<bool*>(&locked) = false;		
		return 0;
	}
	//else if (count < 0) {
	//	*(uint32*)samples = 0;
	//	return 1;
	//}

	return count;
}

}
