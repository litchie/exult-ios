#if __GNUG__ >= 2
#  pragma implementation
#endif

#include "Audio.h"

#include <SDL_audio.h>
#include <SDL_error.h>

#include <cstdio>
#include <unistd.h>

#define	TRAILING_VOC_SLOP 32

#include <SDL_audio.h>
#include <SDL_timer.h>
#include <csignal>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


//---- Audio ---------------------------------------------------------

Audio::~Audio()
{
	SDL_CloseAudio();
}




static 	void debug_speech(void)
{
	extern	Audio audio;
	return;

//	audio.start_speech(23,false);
//	return;
	for(int i=0;i<32;i++)
		{
		audio.start_speech(i,false);
		sleep(1);
		}
}

#include <iostream>
#include <cstdlib>
#include <cstring>

void	Audio::mix_audio(void)
{
}

void	Audio::clear(Uint8 *buf,int len)
{
	memset(buf,wanted.silence,len);
}

extern void fill_audio(void *udata, Uint8 *stream, int len);
#if 0
static void fill_audio(void *udata, Uint8 *stream, int len)
 {
	 extern	Audio	audio;
	 
	 while(len)
		{
         /* Only play if we have data left */
         if ( audio_len == 0 )
                 {
                 // Close current buffer
                 audio.buffers.num_samples[audio.buffers.now_playing]=0;
                 //audio.clear(audio.buffers.buffer[audio.buffers.now_playing],audio.buffers.buffer_length);
                 // Advance to next buffer
                 audio.buffers.advance_playback();
                 if(!audio.buffers.data_ready())
					 {
					 // SDL_PauseAudio(1);
					 return;
					 }
				audio_chunk=audio.buffers.buffer[audio.buffers.now_playing];
				audio_pos=audio_chunk;
				audio_len=audio.buffers.buffer_length;
                }
		 // cout << "Play: " << audio.buffers.now_playing <<"-"<<len << endl;
         /* Mix as much data as possible */
         Uint32	llen=( (unsigned long)len > audio_len ? audio_len : (Uint32)len );
         SDL_MixAudio(stream, audio_pos, llen, SDL_MIX_MAXVOLUME);
         // memcpy(stream,audio_pos,llen);
         audio_pos += llen;
         audio_len -= llen;
         stream+=llen;
         len-=llen;
         }
}
#endif
struct	Chunk
	{
	size_t	length;
	Uint8	*data;
	Chunk() : length(0),data(0) {}
	};

static	Uint8 *chunks_to_block(vector<Chunk> &chunks)
{
	Uint8 *unified_block;
	size_t	aggregate_length=0;
	size_t	working_offset=0;
	
	for(vector<Chunk>::iterator it=chunks.begin();
		it!=chunks.end(); ++it)
		{
		aggregate_length+=it->length;
		}
	unified_block=new Uint8[aggregate_length];
	for(vector<Chunk>::iterator it=chunks.begin();
		it!=chunks.end(); ++it)
		{
		memcpy(unified_block+working_offset,it->data,it->length);
		working_offset+=it->length;
		delete [] it->data; it->data=0; it->length=0;
		}
	
	return unified_block;
}

Uint8 *Audio::convert_VOC(Uint8 *old_data)
{
	vector<Chunk> chunks;
	SDL_AudioCVT	cvt;
	size_t data_offset=0x1a;
	bool	last_chunk=false;
	Uint16	sample_rate;
        size_t  l=0;
	size_t	chunk_length;
	

			

	while(!last_chunk)
		{
		switch(old_data[data_offset]&0xff)
			{
			case 0:
				cout << "Terminator" << endl;
				last_chunk=true;
				continue;
			case 1:
				cout << "Sound data" << endl;
				l=(old_data[3+data_offset]&0xff)<<16;
				l|=(old_data[2+data_offset]&0xff)<<8;
				l|=(old_data[1+data_offset]&0xff);
				cout << "Chunk length appears to be " << l << endl;
				sample_rate=1000000/(256-(old_data[4+data_offset]&0xff));
				cout << "Sample rate ("<< sample_rate<<") = _real_rate"<<endl;
				cout << "compression type " << (old_data[5+data_offset]&0xff) << endl;
				cout << "Channels " << (old_data[6+data_offset]&0xff) << endl;
				chunk_length=l+4;
				break;
			case 2:
				cout << "Sound continues" << endl;
				l=(old_data[3+data_offset]&0xff)<<16;
				l|=(old_data[2+data_offset]&0xff)<<8;
				l|=(old_data[1+data_offset]&0xff);
				cout << "Chunk length appears to be " << l << endl;
				chunk_length=l+4;
				break;
			case 3:
				cout << "Silence" << endl;
				chunk_length=0;
				break;
			default:
				cout << "Unknown VOC chunk " << (*(old_data+data_offset)&0xff) << endl;
				exit(1);
			}

		if(chunk_length==0)
			break;
	if(SDL_BuildAudioCVT(&cvt,AUDIO_U8,1,sample_rate,wanted.format,1,wanted.freq)!=0)
		{
		cerr << "SDL cannot convert formats" << endl;
		}
	else
		{
		cout << "Conversion desriptor built:"<< endl<<
			"Format: " << cvt.src_format << " -> " << cvt.dst_format << endl <<
			"Channels: " << 1 << " -> " << 1 << endl <<
			"Rate: " << sample_rate << " -> " << wanted.freq << endl ;
			cout << "Rate incr: " << cvt.rate_incr << endl ;
		}

		cvt.len=l-TRAILING_VOC_SLOP;
		cvt.buf=new Uint8[cvt.len*cvt.len_mult];
		memcpy(cvt.buf,old_data+6+data_offset,l-TRAILING_VOC_SLOP);
		if(cvt.rate_incr)
			{
			if(SDL_ConvertAudio(&cvt)!=0)
				{
				cerr << "Actual sample conversion didn't work" << endl;
				}
			}
		else
			cvt.len_cvt=cvt.len;
			
		Chunk	c;
		c.data=cvt.buf;
		c.length=cvt.len_cvt;
		chunks.push_back(c);
		data_offset+=chunk_length;
	}
	Uint8 *single_buffer=chunks_to_block(chunks);
	return single_buffer;
}

		
void	Audio::play(Uint8 *sound_data,Uint32 len,bool wait)
{
	bool	own_audio_data=false;
	if(!strncmp((const char *)sound_data,"Creative Voice File",19))
		{
		sound_data=convert_VOC(sound_data);
		own_audio_data=true;
		}
#if 0
	SDL_LockAudio();
	Uint32	offset=0;
	Uint32	walk,first=buffers.now_playing+1;
	if(first==ringsize)
		first=0;
	Uint32 blen=buffers.buffer_length;
	Uint32	rlen;
	walk=first;
	while(len&&walk!=buffers.now_playing)
		{
		// cout << (unsigned long)buffers.buffer[8] << endl;
			rlen=len>blen?blen:len;
			if(buffers.num_samples[walk]==0)
			{
			memset(buffers.buffer[walk],wanted.silence,blen);
			memcpy(buffers.buffer[walk],sound_data+offset,rlen);
			}
		else
		if(buffers.num_samples[walk]<8)
			{
			SDL_MixAudio(buffers.buffer[walk],sound_data+offset,rlen,SDL_MIX_MAXVOLUME);
			}
		else
			{
			++walk;
			if(walk==ringsize)
				walk=0;
			continue;
			}
		len-=rlen; offset+=rlen;
		++buffers.num_samples[walk];
		++walk;
		if(walk==ringsize)
			walk=0;
		}
	if(!buffers.data_ready())
		{
		// cout << "Start sound buffer: " << first << endl;
		buffers.now_playing=first;
		audio_chunk=buffers.buffer[buffers.now_playing];
		audio_pos=audio_chunk;
		audio_len=buffers.buffer_length;
		SDL_PauseAudio(0);
		}
	SDL_UnlockAudio();
	

	if(wait)
		{
		// cout << "Wait sound buffer: " << walk << endl;
         while ( buffers.now_playing!=walk-1 ) {
                 SDL_Delay(100);         /* Sleep 1/10 second */
         }
		}
	if(own_audio_data)
		delete [] sound_data;
#else
	mixer->play(sound_data,len);
	if(own_audio_data)
		delete [] sound_data;
#endif
}

void	Audio::mix(Uint8 *sound_data,Uint32 len)
{
}

static	size_t calc_sample_buffer(Uint16 _samplerate)
{
	Uint32 _buffering_unit=1;
	// while(_buffering_unit<_samplerate/10)
		// _buffering_unit<<=1;
	_buffering_unit=128;
	return _buffering_unit;
}
	
Audio::Audio()
{
	Uint16 _rate=11025;
	int	_channels=2;
	Uint32 _buffering_unit=calc_sample_buffer(_rate);
	// Initialise the speech vectors
	build_speech_vector();
         
SDL_Init(SDL_INIT_AUDIO);

         /* Set the audio format */
         wanted.freq = _rate;
         wanted.format = AUDIO_U8;
         wanted.channels = _channels;    /* 1 = mono, 2 = stereo */
         wanted.samples = _buffering_unit/_channels;  /* Good low-latency value for callback */
         cout << "Stream buffer = " << wanted.samples << endl;
         wanted.callback = fill_audio;
         wanted.userdata = NULL;

         /* Open the audio device, forcing the desired format */
         if ( SDL_OpenAudio(&wanted, NULL) < 0 ) {
                 fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
         }
	mixer=new Mixer(_buffering_unit,ringsize,wanted.silence);
}

Audio::Audio(int _samplerate,int _channels)	
{
	chdir("/home/dancer/projects/exult/u7");
	// Initialise the speech vectors
	Uint32 _buffering_unit=calc_sample_buffer(_samplerate);
	build_speech_vector();
         
SDL_Init(SDL_INIT_AUDIO);

         /* Set the audio format */
         wanted.freq = _samplerate;
         wanted.format = AUDIO_U8;
         wanted.channels = _channels;    /* 1 = mono, 2 = stereo */
         wanted.samples = _buffering_unit/_channels;  /* Good low-latency value for callback */
         cout << "Stream buffer = " << wanted.samples << endl;
         wanted.callback = fill_audio;
         wanted.userdata = NULL;

         /* Open the audio device, forcing the desired format */
         if ( SDL_OpenAudio(&wanted, NULL) < 0 ) {
                 fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
         }
	cout << "Audio system assembled. Ring buffers at "<<_buffering_unit<<endl;
	mixer=new Mixer(_buffering_unit,ringsize,wanted.silence);
	debug_speech();
}

void	Audio::playfile(const char *fname,bool wait)
{
	FILE	*fp;
	
	fp=fopen(fname,"r");
	if(!fp)
		{
		perror(fname);
		return;
		}
	fseek(fp,0L,SEEK_END);
	size_t	len=ftell(fp);
	fseek(fp,0L,SEEK_SET);
	if(len<=0)
		{
		perror("seek");
		fclose(fp);
		return;
		}
	Uint8 *buf=new Uint8[len];
	fread(buf,len,1,fp);
	fclose(fp);
	play(buf,len,wait);
	delete [] buf;
}


void	Audio::mixfile(const char *fname)
{
}


bool	Audio::playing(void)
{
	return false;
}


void	Audio::start_music(int num)
{
	midi.start_music(num);
}

static void	load_buffer(char *buffer,const char *filename,size_t start,size_t len)
{
	FILE	*fp=fopen(filename,"rb");
	if(!fp)
		{
		memset(buffer,0,len);
		return;
		}
	fseek(fp,start,SEEK_SET);
	fread(buffer,len,1,fp);
	fclose(fp);
}

void	Audio::start_speech(int num,bool wait)
{
	start_speech(&speech_tracks,num,wait);
}

void	Audio::start_speech(Flex *f,int num,bool wait)
{
	if((unsigned)num>=f->object_list.size())
		{
		// Out of bounds
		return;	
		}
	if(f->object_list[num].size==0||f->object_list[num].offset==0)
		{
		// Size is null or offset is null
		return;
		}
	char *buf=new char[f->object_list[num].size];
	load_buffer(buf,f->filename.c_str(),f->object_list[num].offset,f->object_list[num].size);
	play((Uint8*)buf,f->object_list[num].size,wait);
	delete [] buf;
}

void	Audio::build_speech_vector(void)
{
	speech_tracks=AccessFlexFile("static/u7speech.spc");
	return;
#if 0
        FILE    *fp=fopen("static/u7speech.spc","rb");
        if(!fp)
                {
                perror("fopen");
                exit(1);
                }
        fseek(fp,0L,SEEK_END);
        size_t  filesize=ftell(fp);
        fseek(fp,0L,SEEK_SET);
        cout << "Speech file is " << filesize << " bytes long" << endl;
        char    *buf=new char[filesize];
        fread(buf,filesize,1,fp);
        fclose(fp);

        for(size_t i=0;i<filesize-8;i++)
        	{
	  if(!memcmp(buf+i,"Creative",8))
          	{
            SpeechPos	sp;
            sp.pos=i;
            speech_tracks.push_back(sp);
            }
          }
        cout << "Finished search"<<endl;
        cout << "Number of discrete samples located: " << speech_tracks.size() << endl;
        cout << "Fixing up lengths"<<endl;
        for(size_t i=0;i<speech_tracks.size()-1;i++)
        	{
        	// do { ++speech_tracks[i].pos; } while ((buf[speech_tracks[i].pos]&0xff)!=0x80);
		speech_tracks[i].pos+=48;
        	speech_tracks[i].len=speech_tracks[i+1].pos-speech_tracks[i].pos;
        	speech_tracks[i].len-=32;
        	}
        cout << "Length fixed and headers passed over." << endl;
        cout << "Discarding temporary audio buffer" << endl;
        delete [] buf;
#endif
}

