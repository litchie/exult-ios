/*
 *  Copyright (C) 2000-2001  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#if (__GNUG__ >= 2) && (!defined WIN32)
#  pragma implementation
#endif

#include <SDL_audio.h>
#include <SDL_timer.h>
#include "SDL_mapping.h"

#include "Audio.h"
#include "Configuration.h"
#include "Flex.h"
#include "conv.h"
#include "exult.h"
#include "fnames.h"
#include "game.h"
#include "utils.h"

#if !defined(ALPHA_LINUX_CXX)
#  include <csignal>
#  include <cstdio>
#  include <cstdlib>
#  include <cstring>
#  include <iostream>
#endif
#include <fcntl.h>
#include <unistd.h>

#if defined(MACOS)
#  include <stat.h>
#else
#  include <sys/stat.h>
#  include <sys/types.h>
#endif


using std::cerr;
using std::cout;
using std::endl;
using std::exit;
using std::memcpy;
using std::memset;
using std::string;
using std::strncmp;
using std::vector;


#if defined(WIN32) && !defined(FORCE_44KHZ)
/* 44100 caused the freeze upon exit in Win! */
#define SAMPLERATE	22050
#else
#define SAMPLERATE	44100
#endif


#define	TRAILING_VOC_SLOP 32
#define	LEADING_VOC_SLOP 32


struct	Chunk
{
	size_t	length;
	uint8	*data;
	Chunk(size_t l, uint8 *d) : length(l),data(d) {}
};


static	size_t calc_sample_buffer(uint16 _samplerate);
static	uint8 *chunks_to_block(vector<Chunk> &chunks);
static	void resample(uint8 *sourcedata, uint8 **destdata,
						size_t sourcelen, size_t *destlen,
						int current_rate, int wanted_rate);
static void decode_ADPCM_4(uint8* inBuf,	
						  int bufSize,				// Size of inbuf
						  uint8* outBuf,	// Size is 2x bufsize
						  int& reference,			// ADPCM reference value
						  int& scale);

Audio *Audio::self = 0;
int *Audio::bg2si_sfxs = 0;


//-----SFX -------------------------------------------------------------

/*
 *	For caching sound effects:
 */
class SFX_cached
	{
	int num;			// Sound-effects #.
	uint8 *buf;			// The data.  It's passed in to us,
					//   and then we own it.
	uint32 len;
	SFX_cached *next;		// Next in chain.
public:
	friend class Audio;
	SFX_cached(int sn, uint8 *b, uint32 l, SFX_cached *oldhead)
		: num(sn), buf(b), len(l), next(oldhead)
		{  }
	~SFX_cached()
		{ delete [] buf; }
	};

//---- Audio ---------------------------------------------------------

void Audio::Init(void)
{
	// Crate the Audio singleton object
	if (!self)
	{
		self = new Audio();
		self->Init(SAMPLERATE,2);
	}
}

void Audio::Destroy(void)
{
	delete self;
	self = 0;
}

Audio	*Audio::get_ptr(void)
{
	// The following assert here might be too harsh, maybe we should leave
	// it to the caller to handle non-inited audio-system?
	assert(self != NULL);

	return self;
}


Audio::Audio() :
	truthful_(false),speech_enabled(true), music_enabled(true),
	effects_enabled(true), SDL_open(false),mixer(0),midi(0), sfxs(0),
	sfx_file(0), initialized(false)
{
	assert(self == NULL);

	string s;

	config->value("config/audio/enabled",s,"yes");
	audio_enabled = (s!="no");
	config->set("config/audio/enabled", audio_enabled?"yes":"no",true);

	config->value("config/audio/speech/enabled",s,"yes");
	speech_enabled = (s!="no");
	config->value("config/audio/midi/enabled",s,"---");
	music_enabled = (s!="no");
	config->value("config/audio/effects/enabled",s,"---");
	effects_enabled = (s!="no");
	config->value("config/audio/midi/looping",s,"yes");
	allow_music_looping = (s!="no");

	midi = 0;
	mixer = 0;
}

void Audio::Init(int _samplerate,int _channels)	
{
	if (!audio_enabled) return;

	// Initialise the speech vectors
	uint32 _buffering_unit=calc_sample_buffer(_samplerate);
	build_speech_vector();

	delete midi;
	midi=0;

	delete mixer;
	mixer=0;


	/* Set the audio format */
	wanted.freq = _samplerate;
	wanted.format = AUDIO_S16SYS;
	wanted.channels = _channels;		/* 1 = mono, 2 = stereo */
	wanted.samples = _buffering_unit;	/* Good low-latency value for callback */
	wanted.callback = Audio::fill_audio;
	wanted.userdata = NULL;

	// Avoid closing SDL audio. This seems to trigger a segfault
	if(SDL_open)
		SDL::QuitSubSystem(SDL_INIT_AUDIO);

#ifdef WIN32
	string s;
	config->value("config/audio/force_waveout",s,"yes");
	if (s == "yes") SDL_AudioInit("waveout");
	else
#endif
	// Init the SDL audio system
	SDL::InitSubSystem(SDL_INIT_AUDIO);

    /* Open the audio device, forcing the desired format */
	if ( SDL::OpenAudio(&wanted, &actual) < 0 )
		{
		cerr << "Couldn't open audio: " << SDL_GetError() << endl;
		audio_enabled = false;	// Prevent crashes.
		return;
		}
	// Disable playing initially.
	SDL_PauseAudio(1);

	wanted=actual;
	_buffering_unit=actual.size;

	SDL_open=true;

	COUT("Audio system assembled. Audio buffer at " <<_buffering_unit);

	SDL::LockAudio();
	midi=new MyMidiPlayer();
	mixer=new Mixer(this, _buffering_unit,_channels,actual.silence);
	SDL::UnlockAudio();

	COUT("Audio initialisation OK");

	initialized = true;
}

bool	Audio::can_sfx(const std::string &game) const
{
	string s;
	string d = "config/disk/game/" + game + "/waves";
	config->value(d.c_str(), s, "---");
	if (s != "---" && U7exists(s.c_str()))
		return true;

	// Also just check in the actual data dir
	d = "<DATA>/" + s;
	if (U7exists(d.c_str()))
		return true;

#ifdef ENABLE_MIDISFX
	if (U7exists("<DATA>/midisfx.flx"))
		return true;
#endif
	
	return false;
}

void	Audio::Init_sfx()
{
	if (sfx_file)
		delete sfx_file;

	if (Game::get_game_type() == SERPENT_ISLE)
		bg2si_sfxs = bgconv;
	else
		bg2si_sfxs = 0;
					// Collection of .wav's?
	string s;
	string d = "config/disk/game/" + Game::get_gametitle() + "/waves";
	config->value(d.c_str(), s, "---");
	if (s != "---")
	{
		if (!U7exists(s.c_str()))
		{
			d = "<DATA>/" + s;
			if (!U7exists(d.c_str()))
			{
				cerr << "Digital SFX's file specified: " << s << "... but file not found" << endl;
				return;
			}
		}
		else
			d = s;

		sfx_file = new Flex(d);
	}
}

Audio::~Audio()
{ 
	if (!initialized)
	{
		self = 0;
		SDL_open = false;
		return;
	}

	CERR("~Audio:  about to stop_music()");
	stop_music();

	CERR("~Audio:  about to quit subsystem");
	SDL::QuitSubSystem(SDL_INIT_AUDIO); // SDL 1.1 lets us diddle with
						// subsystems
	CERR("~Audio:  closed audio");

	if(mixer)
	{
		CERR("~Audio:  about to cancel_streams()");
		cancel_streams();
		delete mixer;
		mixer = 0;
		CERR("~Audio:  deleted mixer");
	}
	if(midi)
	{
		delete midi;
		midi = 0;
	}
	while (sfxs)			// Cached sound effects.
	{
		SFX_cached *todel = sfxs;
		sfxs = todel->next;
		delete todel;
	}
	delete sfx_file;
	CERR("~Audio:  deleted midi");

	// Avoid closing SDL audio. This seems to trigger a segfault
	// SDL::CloseAudio();
	SDL_open = false;
	self = 0;
}


/* The audio function callback takes the following parameters:
     stream:  A pointer to the audio buffer to be filled
     len:     The length (in bytes) of the audio buffer
 */
 
void Audio::fill_audio(void *udata, uint8 *stream, int len)
{
	if( self && self->mixer )
		self->mixer->fill_audio_func(udata,stream,len);
}


void	Audio::mix_audio(void)
{
}

void	Audio::clear(uint8 *buf,int len)
{
	memset(buf,actual.silence,len);
}

uint8 *Audio::convert_VOC(uint8 *old_data,uint32 &visible_len)
{
	vector<Chunk> chunks;
	size_t	data_offset=0x1a;
	bool	last_chunk=false;
	uint16	sample_rate;
	size_t  l = 0;
	size_t	chunk_length;
	int		compression = 0;
	int		adpcm_reference = -1;
	int		adpcm_scale = 0;

	while(!last_chunk)
	{
		switch(old_data[data_offset]&0xff)
		{
			case 0:
				COUT("Terminator");
				last_chunk = true;
				continue;
			case 1:
				COUT("Sound data");
				l = (old_data[3+data_offset]&0xff)<<16;
				l |= (old_data[2+data_offset]&0xff)<<8;
				l |= (old_data[1+data_offset]&0xff);
				COUT("Chunk length appears to be " << l);
				sample_rate=1000000/(256-(old_data[4+data_offset]&0xff));
				COUT("Original sample_rate is " << sample_rate << ", hw rate is " << actual.freq);
				COUT("Sample rate ("<< sample_rate<<") = _real_rate");
				compression = old_data[5+data_offset]&0xff;
				COUT("compression type " << compression);
				if (compression) {
					adpcm_reference = -1;
					adpcm_scale = 0;
				}
				COUT("Channels " << (old_data[6+data_offset]&0xff));
				chunk_length=l+4;
				break;
			case 2:
				COUT("Sound continues");
				l=(old_data[3+data_offset]&0xff)<<16;
				l|=(old_data[2+data_offset]&0xff)<<8;
				l|=(old_data[1+data_offset]&0xff);
				COUT("Chunk length appears to be " << l);
				chunk_length = l+4;
				break;
			case 3:
				COUT("Silence");
				chunk_length=0;
				break;
			case 5:		// A null terminated string
				COUT("Text string chunk");
				chunk_length=0;
				break;
			default:
				cerr << "Unknown VOC chunk " << (*(old_data+data_offset)&0xff) << endl;
				throw exult_exception("Unknown VOC chunk");
		}

		if(chunk_length==0)
			break;


		l -= (TRAILING_VOC_SLOP+LEADING_VOC_SLOP);

		// 
		uint8 *dec_data = old_data+LEADING_VOC_SLOP;
		size_t dec_len = l;

		// Decompress data
		if (compression == 1) {
			// Allocate temp buffer
			if (adpcm_reference == -1) dec_len = (dec_len-1)*2;
			else dec_len *= 2;
			dec_data = new uint8[dec_len];
			decode_ADPCM_4(old_data+LEADING_VOC_SLOP, l, dec_data, adpcm_reference, adpcm_scale);
		}
		else if (compression != 0) {
			CERR("Can't handle VOC compression type"); 
		}

		// Resample to the current rate
		uint8 *new_data;
		size_t new_len;
		resample(dec_data,&new_data,dec_len,&new_len,
						sample_rate,actual.freq);
		l = new_len;

		// Delete temp buffer
		if (compression == 1) {
			delete [] dec_data;
		}

		COUT("Have " << l << " bytes of resampled data");

		// And convert to 16 bit stereo
		sint16 *stereo_data=new sint16[l*2];
		for(size_t i=0,j=0;i<l;i++)
		{
			stereo_data[j++]=(new_data[i]-128)<<8;
			stereo_data[j++]=(new_data[i]-128)<<8;
		}
		l *= 4; // because it's 16bit
		delete [] new_data;

		chunks.push_back(Chunk(l,(uint8 *)stereo_data));
		data_offset+=chunk_length;
	}
	COUT("Turn chunks to block");
	visible_len = l;

	return chunks_to_block(chunks);
}

		
void	Audio::play(uint8 *sound_data,uint32 len,bool wait)
{
	if (!audio_enabled || !speech_enabled) return;

	bool	own_audio_data=false;
	if(!strncmp((const char *)sound_data,"Creative Voice File",19))
	{
		sound_data=convert_VOC(sound_data,len);
		own_audio_data=true;
	}

	if(mixer)
		mixer->play(sound_data,len);
	if(own_audio_data)
		delete [] sound_data;
}

void	Audio::cancel_streams(void)
{
	if (!audio_enabled) return;
	if(mixer)
		mixer->cancel_streams();
}

void	Audio::mix(uint8 *sound_data,uint32 len)
{
	if (!audio_enabled) return;
	if(mixer)
		mixer->play(sound_data,len);
}
	

void	Audio::playfile(const char *fname,bool wait)
{
	if (!audio_enabled)
		return;

	FILE	*fp;
	size_t	len;
	uint8	*buf;
	
	fp = U7open(fname,"r");
	if(!fp)
	{
		perror(fname);
		return;
	}
	fseek(fp,0L,SEEK_END);
	len=ftell(fp);
	fseek(fp,0L,SEEK_SET);
	if(len<=0)
	{
		perror("seek");
		fclose(fp);
		return;
	}
	buf=new uint8[len];
	fread(buf,len,1,fp);
	fclose(fp);
	play(buf,len,wait);
	delete [] buf;
}

/*
 *	Play a wave file.
 */
void	Audio::playwave(const char *fname, bool wait)
{
	if (!audio_enabled) return;

	uint8 *buf;
	Uint32 len;
	SDL_AudioSpec src;
	SDL_AudioCVT cvt;		// Got to convert.

	if (!SDL_LoadWAV(fname, &src, &buf, &len))
	{
		CERR("Couldn't play file '" << fname << "'");
		return;
	}

	if (SDL_BuildAudioCVT(&cvt, src.format, src.channels, src.freq,
			actual.format, actual.channels, actual.freq) < 0)
	{
		CERR("Couldn't convert wave data");
		return;
	}
	cvt.len = len;
	cvt.buf = new uint8[len*cvt.len_mult];
	memcpy(cvt.buf, buf, len);
	SDL_FreeWAV(buf);
	SDL_ConvertAudio(&cvt);
	if(mixer)
		mixer->play(cvt.buf,cvt.len_cvt);
	delete[] cvt.buf;
}

void	Audio::mixfile(const char *fname)
{
}


bool	Audio::playing(void)
{
	return false;
}


void	Audio::start_music(int num, bool continuous, int bank)
{
	if(audio_enabled && music_enabled && midi != 0)
		midi->start_music(num,continuous && allow_music_looping,bank);
}

void	Audio::start_music(const char *fname, int num, bool continuous)
{
	if(audio_enabled && music_enabled && midi != 0)
		midi->start_music(fname,num,continuous && allow_music_looping);
}

void Audio::start_music(XMIDI *mid_file,bool continuous)
{
	if(audio_enabled && music_enabled && midi != 0)
		midi->start_track(mid_file,continuous && allow_music_looping);
}

void	Audio::start_music_combat (Combat_song song, bool continuous, int bank)
{
	if(!audio_enabled || !music_enabled || midi == 0)
		return;

	int num = -1;
	
	if (Game::get_game_type()!=SERPENT_ISLE) switch (song)
	{
		case CSBattle_Over:
		num = 9;
		break;
		
		case CSAttacked1:
		num = 11;
		break;
		
		case CSAttacked2:
		num = 12;
		break;
		
		case CSVictory:
		num = 15;
		break;
		
		case CSRun_Away:
		num = 16;
		break;
		
		case CSDanger:
		num = 10;
		break;
		
		case CSHidden_Danger:
		num = 18;
		break;
		
		default:
		CERR("Error: Unable to Find combat track for song " << song << ".");
		break;
	}
	else switch (song)
	{
		case CSBattle_Over:
		num = 0;
		break;
		
		case CSAttacked1:
		num = 2;
		break;
		
		case CSAttacked2:
		num = 3;
		break;
		
		case CSVictory:
		num = 6;
		break;
		
		case CSRun_Away:
		num = 7;
		break;
		
		case CSDanger:
		num = 1;
		break;
		
		case CSHidden_Danger:
		num = 9;
		break;
		
		default:
		CERR("Error: Unable to Find combat track for song " << song << ".");
		break;
	}
	
	midi->start_music(num,continuous && allow_music_looping,bank);
}

void	Audio::stop_music()
{
//	if (!audio_enabled) return;

	if(midi)
		midi->stop_music();
}

bool	Audio::start_speech(int num,bool wait)
{
	if (!audio_enabled || !speech_enabled)
		return false;

	char	*buf=0;
	size_t	len;
	const char	*filename;
	
	if (Game::get_game_type() == SERPENT_ISLE)
		filename = SISPEECH;
	else
		filename = U7SPEECH;
	
	U7object	sample(filename,num);
	try
	{
		buf = sample.retrieve(len);
	}
	catch( const std::exception & err )
	{
		return false;
	}
	play(reinterpret_cast<uint8*>(buf),len,wait);
	delete [] buf;
	return true;
}

void	Audio::build_speech_vector(void)
{
}

/*
 *	This returns a 'unique' ID, but only for .wav SFX's (for now).
 */
AudioID	Audio::play_sound_effect (int num, int volume, int dir, bool repeat)
{
	if (!audio_enabled || !effects_enabled) return AudioID(0, 0);

	// Where sort of sfx are we using????
	if (sfx_file != 0)		// Digital .wav's?
		return play_wave_sfx(num, volume, dir, repeat);
#ifdef ENABLE_MIDISFX
	else if (midi != 0) 
		midi->start_sound_effect(num);
#endif
	return AudioID(0, 0);
}

/*
 *	Play a .wav format sound effect.
 */
AudioID Audio::play_wave_sfx
	(
	int num,
	int volume,			// 0-128.
	int dir,			// 0-15, from North, clockwise.
	bool repeat			// Keep playing.
	)
{
	if (!effects_enabled || !sfx_file || !mixer) 
		return AudioID(0, 0);  // no .wav sfx available
	CERR("Playing SFX: " << num);
#if 0
	if (Game::get_game_type() == BLACK_GATE)
		num = bgconv[num];
	CERR("; after bgconv:  " << num);
#endif
	if (num < 0 || num >= sfx_file->number_of_objects())
	{
		cerr << "SFX " << num << " is out of range" << endl;
		return AudioID(0, 0);
	}

	const int max_cached = 12;	// Max. we'll cache.

	SFX_cached *each = sfxs, *prev = 0;
	int cnt = 0;
	size_t wavlen;			// Read .wav file.
	Uint8 *buf;
	Uint32 len;
	SDL_AudioSpec src;		// Load .wav data (& free rwsrc).
	SDL_AudioCVT cvt;		// Got to convert.
	SDL_RWops *rwsrc;

	// First see if we have it already in our cache
	while (each && each->num != num && each->next)
	{
		cnt++;
		prev = each;
		each = each->next;
	}
	if (each && each->num == num)	// Found it?
	{
		// Move to head of chain.
		if (prev)
		{
			prev->next = each->next;
			each->next = sfxs;
			sfxs = each;
		}
		// Return the cached data
		return mixer->play(each->buf, each->len, volume, dir, repeat);
	}
	if (cnt == max_cached)		// Hit our limit?  Remove last.
	{
		prev->next = 0;
		delete each;
	}
	
	// Retrieve the .wav data from the SFX file
	char *wavbuf = sfx_file->retrieve(num, wavlen);

	// Load .wav data; this implictly deallocates the SDL_RWops
	rwsrc = SDL_RWFromMem(wavbuf, wavlen);
	if (!SDL_LoadWAV_RW(rwsrc, 1, &src, &buf, &len))
	{
		cerr << "Couldn't play sfx '" << num << "'" << endl;
		return AudioID(0, 0);
	}

	if (SDL_BuildAudioCVT(&cvt, src.format, src.channels, src.freq,
			actual.format, actual.channels, actual.freq) < 0)
	{
		cerr << "Couldn't convert wave data" << endl;
		return AudioID(0, 0);
	}
	cvt.len = len;
	cvt.buf = new uint8[len*cvt.len_mult];
	memcpy(cvt.buf, buf, len);
	SDL_FreeWAV(buf);
	SDL_ConvertAudio(&cvt);
					// Cache at head of chain.
	sfxs = new SFX_cached(num, cvt.buf, cvt.len_cvt, sfxs);
	return mixer->play(cvt.buf, cvt.len_cvt, volume, dir, repeat);
}

/*
 *	Halt sound effects.
 */

void Audio::stop_sound_effects()
{
	if (sfx_file != 0)		// .Wav's?
		mixer->Destroy_Audio_Stream(Mixer_Sample_Magic_Number);
#ifdef ENABLE_MIDISFX
	else if (midi)
		midi->stop_sound_effects();
#endif
	}


void Audio::set_audio_enabled(bool ena)
{
	if (ena && audio_enabled && initialized)
	{

	}
	else if (!ena && audio_enabled && initialized)
	{
		stop_sound_effects();
		stop_music();
		audio_enabled = false;
	}
	else if (ena && !audio_enabled && initialized)
	{
		audio_enabled = true;
	}
	else if (!ena && !audio_enabled && initialized)
	{

	}
	else if (ena && !audio_enabled && !initialized)
	{
		audio_enabled = true;

		Init(SAMPLERATE,2);
	}
	else if (!ena && !audio_enabled && !initialized)
	{

	}
}


static	size_t calc_sample_buffer(uint16 _samplerate)
{
	uint32 _buffering_unit=1;
	while(_buffering_unit<_samplerate/10U)
		_buffering_unit<<=1;
	// _buffering_unit=128;
	return _buffering_unit;
}


static	uint8 *chunks_to_block(vector<Chunk> &chunks)
{
	uint8 *unified_block;
	size_t	aggregate_length=0;
	size_t	working_offset=0;
	
	for(std::vector<Chunk>::iterator it=chunks.begin();
		it!=chunks.end(); ++it)
		{
		aggregate_length+=it->length;
		}
	unified_block=new uint8[aggregate_length];
	{
		for(std::vector<Chunk>::iterator it=chunks.begin();
			it!=chunks.end(); ++it)
			{
			memcpy(unified_block+working_offset,it->data,it->length);
			working_offset+=it->length;
			delete [] it->data; it->data=0; it->length=0;
			}
	}
	
	return unified_block;
}


static	void resample(uint8 *sourcedata, uint8 **destdata,
						size_t sourcelen, size_t *destlen,
						int current_rate, int wanted_rate)
{
	// I have no idea what I'm doing here - Dancer
	// This is really Breshenham's line-drawing algorithm in
	// a false nose, and clutching a crude smoothing loop.

	float	ratio= (static_cast<float>(wanted_rate))/(static_cast<float>(current_rate));
	*destlen = static_cast<unsigned int> ((sourcelen*ratio)+1);
	if(!*destlen||current_rate==wanted_rate)
	{
		// Least work
		*destlen=sourcelen;
		*destdata=new uint8[sourcelen];
		memcpy(*destdata,sourcedata,sourcelen);
		return;
	}
	*destdata=new uint8[*destlen];
	size_t last=0;
	for(size_t i=0;i<sourcelen;i++)
		{
		size_t pos = (size_t) (i*ratio);
		assert(pos<=*destlen);
		(*destdata)[pos]=sourcedata[i];
		// Interpolate if need be
		if(last!=pos&&last!=pos-1)
			for(size_t j=last+1;j<=pos-1;j++)
				{
				unsigned int x=(unsigned char)sourcedata[i];
				unsigned int y=(unsigned char)sourcedata[i-1];
				x=(x+y)/2;
				(*destdata)[j]=(uint8) x;
				}
		last=pos;
		}
	CERR("End resampling. Resampled " << sourcelen << " bytes to " << *destlen << " bytes");
}

//
// Decode 4bit ADPCM vocs (thunder in SI intro)
//
// Code grabbed from VDMS
//

inline int decode_ADPCM_4_sample(uint8 sample,
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
static void decode_ADPCM_4(uint8* inBuf,	
						  int bufSize,				// Size of inbuf
						  uint8* outBuf,			// Size is 2x bufsize
						  int& reference,			// ADPCM reference value
						  int& scale)
{
	int i, skip = 0;
	
	if (reference < 0) {
		reference = inBuf[0] & 0xff;   // use the first byte in the buffer as the reference byte
		bufSize--;                          // remember to skip the reference byte
	}
	
	for (i = 0; i < bufSize; i++) {
		outBuf[i * 2 + 0] = decode_ADPCM_4_sample(inBuf[i] >> 4, reference, scale);
		outBuf[i * 2 + 1] = decode_ADPCM_4_sample(inBuf[i] >> 0, reference, scale);
	}
}
