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
#include "headers/exceptions.h"
//#include "databuf.h"

namespace Pentagram {

ov_callbacks OggAudioSample::callbacks = {
  &read_func,
  &seek_func,
  0,
  &tell_func
};

OggAudioSample::OggAudioSample(IDataSource *oggdata_)
	: AudioSample(0, 0), oggdata(oggdata_)
{
	frame_size = 4096;
	decompressor_size = sizeof(OggDecompData);
	bits = 16;
	locked = false;


}

OggAudioSample::~OggAudioSample()
{
	delete oggdata;
	oggdata = 0;
}

size_t OggAudioSample::read_func  (void *ptr, size_t size, size_t nmemb, void *datasource)
{
	IDataSource *ids = (IDataSource*) datasource;
	//if (ids->eof()) return 0;
	size_t limit = ids->getSize() - ids->getPos();
	if (limit <= 0) return 0;
	else if (limit < size*nmemb) nmemb = limit/size;
	ids->read(ptr,size*nmemb);
	return nmemb;
}
int    OggAudioSample::seek_func  (void *datasource, ogg_int64_t offset, int whence)
{
	IDataSource *ids = (IDataSource*) datasource;
	switch(whence)
	{
	case SEEK_SET:
		ids->seek((size_t)offset);
		return 0;
	case SEEK_END:
		ids->seek(ids->getSize()-(size_t)offset);
		return 0;
	case SEEK_CUR:
		ids->skip((size_t)offset);
		return 0;
	}
	return -1;
}
long   OggAudioSample::tell_func  (void *datasource)
{
	IDataSource *ids = (IDataSource*) datasource;
	return ids->getPos();
}

bool OggAudioSample::is_ogg(IDataSource *oggdata)
{
	OggVorbis_File vf;
	oggdata->seek(0);
	int res = ov_test_callbacks((void*)oggdata,&vf,0,0,callbacks);
	ov_clear(&vf);

	return res == 0;
}


void OggAudioSample::initDecompressor(void *DecompData) const
{
	OggDecompData *decomp = reinterpret_cast<OggDecompData *>(DecompData);

	if (locked) 
		throw exult_exception("Attempted to play OggAudioSample on more than one channel at the same time.");

	*const_cast<bool*>(&locked) = true;		

	oggdata->seek(0);
	ov_open_callbacks((void*)this->oggdata,&decomp->ov,NULL,0,callbacks);
	decomp->bitstream = 0;
	
	vorbis_info *info = ov_info(&decomp->ov,-1);
	*const_cast<uint32*>(&sample_rate) = decomp->last_rate = info->rate;
	*const_cast<bool*>(&stereo) = decomp->last_stereo = info->channels == 2;
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
