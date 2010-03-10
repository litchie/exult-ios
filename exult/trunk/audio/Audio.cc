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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef PENTAGRAM // Exult only at this stage. 


#include <SDL_audio.h>
#include <SDL_timer.h>

#include <fstream>
//#include "SDL_mapping.h"

#include "Audio.h"
#include "Configuration.h"
#include "Flex.h"
#include "conv.h"
#include "exult.h"
#include "fnames.h"
#include "game.h"
#include "utils.h"

#if !defined(ALPHA_LINUX_CXX)
#ifndef UNDER_CE
#  include <csignal>
#endif
#  include <cstdio>
#  include <cstdlib>
#  include <cstring>
#  include <iostream>
#endif
#ifndef UNDER_CE
#include <fcntl.h>
#include <unistd.h>
#endif

#if defined(MACOS)
#  include <stat.h>
#elif !defined(UNDER_CE)
#  include <sys/stat.h>
#  include <sys/types.h>
#endif

//#include <crtdbg.h>


#ifndef UNDER_EMBEDDED_CE
using std::cerr;
using std::cout;
using std::endl;
using std::exit;
using std::memcpy;
using std::memset;
using std::string;
using std::strncmp;
using std::vector;
using std::ifstream;
using std::ios;
#endif

// These MIGHT be macros!
#ifndef min
using std::min;
#endif
#ifndef max
using std::max;
#endif

#define	TRAILING_VOC_SLOP 32
#define	LEADING_VOC_SLOP 32
#define MIXER_CHANNELS 32

struct	Chunk
{
	size_t	length;
	uint8	*data;
	Chunk(size_t l, uint8 *d) : length(l),data(d) {}
};

static	size_t calc_sample_buffer(uint16 _samplerate);
static	uint8 *chunks_to_block(vector<Chunk> &chunks);
static	sint16 *resample_new(uint8 *sourcedata,
						size_t sourcelen, size_t &destlen,
						int current_rate, int wanted_rate);
static	sint16 *resample_new_mono(uint8 *sourcedata,
						size_t sourcelen, size_t &destlen,
						int current_rate, int wanted_rate);
#ifdef USE_OLD_RESAMPLE
static	void resample(uint8 *sourcedata, uint8 **destdata,
						size_t sourcelen, size_t *destlen,
						int current_rate, int wanted_rate);
#endif
static void decode_ADPCM_4(uint8* inBuf,	
						  int bufSize,				// Size of inbuf
						  uint8* outBuf,	// Size is 2x bufsize
						  int& reference,			// ADPCM reference value
						  int& scale);

Audio *Audio::self = 0;
int const *Audio::bg2si_sfxs = 0;

//----- Utilities ----------------------------------------------------

/*
 * Class that performs cubic interpolation on integer data.
 * It is expected that the data is equidistant, i.e. all have the same
 * horizontal distance. This is obviously the case for sampled audio.
 */
class CubicInterpolator {
protected:
	int x0, x1, x2, x3;
	int a, b, c, d;
	
public:
	CubicInterpolator(int a0, int a1, int a2, int a3) : x0(a0), x1(a1), x2(a2), x3(a3)
	{
		updateCoefficients();
	}
	
	CubicInterpolator(int a1, int a2, int a3) : x0(2*a1-a2), x1(a1), x2(a2), x3(a3)
	{
		// We use a simple linear interpolation for x0
		updateCoefficients();
	}
	
	inline void feedData()
	{
		x0 = x1;
		x1 = x2;
		x2 = x3;
		x3 = 2*x2-x1;	// Simple linear interpolation
		updateCoefficients();
	}

	inline void feedData(int xNew)
	{
		x0 = x1;
		x1 = x2;
		x2 = x3;
		x3 = xNew;
		updateCoefficients();
	}
	
	/* t must be a 16.16 fixed point number between 0 and 1 */
	inline int interpolate(uint32 fp_pos)
	{
		int result = 0;
		int t = fp_pos >> 8;
		result = (a*t + b) >> 8;
		result = (result * t + c) >> 8;
		result = (result * t + d) >> 8;
		result = (result/3 + 1) >> 1;
		
		return result;
	}
		
protected:
	inline void updateCoefficients()
	{
		a = ((-x0*2)+(x1*5)-(x2*4)+x3);
		b = ((x0+x2-(2*x1))*6) << 8;
		c = ((-4*x0)+x1+(x2*4)-x3) << 8;
		d = (x1*6) << 8;
	}
};

//----- SFX ----------------------------------------------------------

/*
 *	For caching sound effects:
 */
class SFX_cached
	{
	int num;			// Sound-effects #.
	Mix_Chunk *data;	// The data.
	SFX_cached *next;	// Next in chain.
	int ref_cnt;		// Reference count of the sfx/voice.
public:
	friend class Audio;
	friend class SFX_cache_manager;
	SFX_cached(int sn, uint8 *b, uint32 l, SFX_cached *oldhead)
		: num(sn), next(oldhead), ref_cnt(1)
		{
			if (num >= 0 || !strncmp((const char *)b, "OggS", 4))
				{
				SDL_RWops *rwsrc = SDL_RWFromMem(b, l);
				data = Mix_LoadWAV_RW(rwsrc, 1);
				}
			else
				data = Mix_QuickLoad_RAW(b, l);
		}
	~SFX_cached()
		{
		Uint8 *chunkbuf=NULL;
		if(data->allocated == 0)
			chunkbuf = data->abuf;
		Mix_FreeChunk(data);

		//Must be freed after the Mix_FreeChunk
		if(chunkbuf)
			delete[] chunkbuf;
		}
	void add_ref(void)
		{ ++ref_cnt; }
	void release(void)
		{
		// Free when refcount is 0
		if (!--ref_cnt)
			delete this;
		}
	int get_num_refs(void)
		{ return(ref_cnt); }
	};

/*
 *	This is a resource-management class for SFX. Maybe make it a
 *	template class and use for other resources also?
 *	Based on code by Sam Lantinga et al on:
 *	http://www.ibm.com/developerworks/library/l-pirates2/
 */
class SFX_cache_manager
	{
protected:
	SFX_cached *cache;

	// Tries to locate a sfx in the cache based on sfx num.
	SFX_cached *find_sfx(int id)
		{
		SFX_cached *prev = 0;
		for (SFX_cached *each = cache; each; each = each->next)
			{
			if (each->num == id)
				{
				// Move to head of chain.
				if (prev)
					{
					prev->next = each->next;
					each->next = cache;
					cache = each;
					}
				return each;
				}
			prev = each;
			}
		return 0;
		}
	
	// Loads a wave SFX from its digital file and num.
	SFX_cached *load_sfx(Flex *sfx_file, int id)
		{
		size_t wavlen;			// Read .wav file.
		unsigned char *wavbuf =
				(unsigned char *) sfx_file->retrieve(id, wavlen);
		if (wavlen)
			cache = new SFX_cached(id, wavbuf, wavlen, cache);
		delete[] wavbuf;

		// Perform garbage collection here.
		garbage_collect();
		return wavlen ? cache : (SFX_cached *)0;
		}
	
	// Unloads a given SFX, based on data pointer.
	void unload(Mix_Chunk *data)
		{
		SFX_cached *prev = 0, *each;
		for (each = cache; each; each = each->next)
			{
			if (data == each->data)
				{
				// Free the object if not in use
				if (each->ref_cnt != 1)
					{
					CERR("Unloading cached SFX " << each->num
						<< " while it was still in use! This better be happening because Exult is closing!");
					while (each->ref_cnt > 1)
						each->release();
					}

				// Unlink
				if (prev)
					prev->next = each->next;
				else
					cache = each->next;

				each->release();
				break;
				}
			prev = each;
			}
		}
public:
	SFX_cache_manager()
		{ cache = 0; }
	~SFX_cache_manager()
		{ flush(); }
	
	// For sounds played through 'play', which include voice and some SFX (?)
	// in the intro sequences.
	Mix_Chunk *add_from_data(unsigned char *wavbuf, size_t wavlen)
		{
		cache = new SFX_cached(-1, wavbuf, wavlen, cache);
		cache->add_ref();

		// Perform garbage collection here.
		garbage_collect();
		return cache->data;
		}

	// For SFX played through 'play_wave_sfx'. Searched cache for
	// the sfx first, then loads from the sfx file if needed.
	Mix_Chunk *request(Flex *sfx_file, int id)
		{
		SFX_cached *sfx = 0;
		if (id > -1 && sfx_file)
			{
			sfx = find_sfx(id);
			if (!sfx)
				sfx = load_sfx(sfx_file, id);
			if (!sfx)
				return (Mix_Chunk *)0;
			sfx->add_ref();
			return sfx->data;
			}
		return (Mix_Chunk *)0;
		}
	
	// Reduces the ref-count of the data, to a minimum of 1.
	// Does *not* remove from cache, ever.
	void release(Mix_Chunk *data)
		{
		if (!data)
			return;
		SFX_cached *prev = 0, *each;
		for (each = cache; each; each = each->next)
			{
			if (data == each->data)
				{
				// Free the object if not in use
				if (each->ref_cnt == 1)
					CERR("Tried to release cached but unused SFX");
				else
					each->release();
				break;
				}
			prev = each;
			}
		}
	
	// Empties the cache.
	void flush(void)
		{
		while (cache)
			unload(cache->data);
		cache = 0;
		}

	// Remove unused sounds from the cache.
	void garbage_collect()
		{
		// Maximum 'stable' number of sounds we will cache (actual
		// count may be higher if all of the cached sounds are
		// being played).
		const int max_fixed = 6;
		SFX_cached *each = cache;
		SFX_cached *prev = 0;
		int cnt = 0;
		while (each)
			{
			if (each->ref_cnt == 1 &&
				(each->num < 0 || cnt >= max_fixed))
				{	// Remove from cache and unlink.
				prev->next = each->next;
				each->release();
				}
			else
				prev = each;
			cnt++;
			// Causes occasional segfault:
			//each = each->next;
			each = prev->next;
			}
		}
	};

//---- Audio ---------------------------------------------------------
void Audio::Init(void)
{
	// Crate the Audio singleton object
	if (!self)
	{
		self = new Audio();
#ifdef UNDER_CE
		self->Init(SAMPLERATE/2,1);
#else
		self->Init(SAMPLERATE,2);
#endif
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
	effects_enabled(true), SDL_open(false),/*mixer(0),*/midi(0),
	initialized(false), sfx_file(0)
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
	sfxs = new SFX_cache_manager();
}

void Audio::Init(int _samplerate,int _channels)	
{
	if (!audio_enabled) return;

	// Initialise the speech vectors
	uint32 _buffering_unit=calc_sample_buffer(_samplerate);
	build_speech_vector();

	delete midi;
	midi=0;

	// Avoid closing SDL audio. This seems to trigger a segfault
	if(SDL_open)
		SDL_QuitSubSystem(SDL_INIT_AUDIO);

	// Init the SDL audio system
	SDL_InitSubSystem(SDL_INIT_AUDIO);

	/* Open the audio device, forcing the desired format */

	if ( Mix_OpenAudio(_samplerate, AUDIO_S16SYS, _channels, _buffering_unit) < 0 )
		{
		cerr << "Couldn't open audio: " << Mix_GetError() << endl;
		audio_enabled = false;	// Prevent crashes.
		return;
		}
	int art_freq;
	Uint16 art_format;
	int art_channels;
	
	Mix_QuerySpec(&art_freq,&art_format,&art_channels);

	actual.freq = art_freq;
	actual.format = art_format;
	actual.channels = art_channels;
	
#ifdef DEBUG
	cout << "Audio requested frequency " << _samplerate << ", channels " << _channels << endl;
	cout << "Audio actual frequency " << actual.freq << ", channels " << (int) actual.channels << endl;
#endif

	// Allocate a moderate number of mixer channels.
	Mix_AllocateChannels(MIXER_CHANNELS);
	//SDL_mixer will always go here when it has played a sound, we want to free up
	//the memory used as we don't re-play the sound.
	Mix_ChannelFinished(channel_complete_callback);

	// Disable playing initially.
	Mix_Pause(-1);

	SDL_open=true;

	midi=new MyMidiPlayer();

	COUT("Audio initialisation OK");

	initialized = true;

}

void Audio::channel_complete_callback(int chan)
{
	Mix_Chunk *done_chunk = Mix_GetChunk(chan);
	SFX_cache_manager *sfxs = Audio::get_ptr()->get_sfx_cache();
	sfxs->release(done_chunk);
	/*
	Mix_Chunk *done_chunk = Mix_GetChunk(chan);
	Uint8 *chunkbuf=NULL;

	//We need to free these chunks as they were allocated by us and not SDL_Mixer
	//This happens when Mix_QuickLoadRAW is used.
	if(done_chunk->allocated == 0)
		chunkbuf = done_chunk->abuf;
		
	Mix_FreeChunk(done_chunk);

	//Must be freed after the Mix_FreeChunk
	if(chunkbuf)
		delete[] chunkbuf;
	*/
}

bool	Audio::can_sfx(const std::string &file, std::string *out)
{
	if (file == "")
		return false;
	string d = file;
	// Full path?
	if (U7exists(d.c_str()))
		{
		if (out)
			*out = d;
		return true;
		}

#ifdef MACOSX
	// Check in the app bundle:
	d = "<BUNDLE>/" + file;
	if (is_system_path_defined("<BUNDLE>") && U7exists(d.c_str()))
		{
		if (out)
			*out = d;
		return true;
		}
#endif

	// Also just check in the actual data dir
	d = "<DATA>/" + file;
	if (U7exists(d.c_str()))
		{
		if (out)
			*out = d;
		return true;
		}

	return false;
}

bool Audio::have_roland_sfx(Exult_Game game, std::string *out)
	{
	if (game == BLACK_GATE)
		return can_sfx(SFX_ROLAND_BG, out);
	else if (game == SERPENT_ISLE)
		return can_sfx(SFX_ROLAND_SI, out);
	return false;
	}
	
bool Audio::have_sblaster_sfx(Exult_Game game, std::string *out)
	{
	if (game == BLACK_GATE)
		return can_sfx(SFX_BLASTER_BG, out);
	else if (game == SERPENT_ISLE)
		return can_sfx(SFX_BLASTER_SI, out);
	return false;
	}
	
bool Audio::have_midi_sfx(std::string *out)
	{
#ifdef ENABLE_MIDISFX
	return can_sfx(SFX_MIDIFILE, out);
#else
	return false;
#endif
	}
	
bool Audio::have_config_sfx(const std::string &game, std::string *out)
	{
	string s;
	string d = "config/disk/game/" + game + "/waves";
	config->value(d.c_str(), s, "---");
	return (s != "---") && can_sfx(s, out);
	}
	
void	Audio::Init_sfx()
{
	FORGET_OBJECT(sfx_file);

	Exult_Game game = Game::get_game_type();
	if (game == SERPENT_ISLE)
		bg2si_sfxs = bgconv;
	else
		bg2si_sfxs = 0;
					// Collection of .wav's?
	string flex;
#ifdef ENABLE_MIDISFX
	string v;
	config->value("config/audio/effects/midi", v, "no");
	if (have_midi_sfx(&flex) && v != "no")
		{
		cout << "Opening midi SFX's file: \"" << flex << "\"" << endl;
		sfx_file = new FlexFile(flex.c_str());
		return;
		}
#endif
	if (!have_config_sfx(Game::get_gametitle(), &flex))
		{
		if (have_roland_sfx(game, &flex) || have_sblaster_sfx(game, &flex))
			{
			string d = "config/disk/game/" + Game::get_gametitle() + "/waves";
			size_t sep = flex.rfind('/');
			if (sep != string::npos)
				{
				sep++;
				flex = flex.substr(sep);
				}
			config->set(d.c_str(), flex, true);
			}
		else
			{
			cerr << "Digital SFX's file specified: " << flex
				<< "... but file not found, and fallbacks are missing" << endl;
			return;
			}
		}
	cout << "Opening digital SFX's file: \"" << flex << "\"" << endl;
	sfx_file = new FlexFile(flex.c_str());
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

	if(midi)
	{
		delete midi;
		midi = 0;
	}

	if (effects_enabled) 
		Mix_HaltChannel(-1);

	delete sfxs;
	delete sfx_file;
	CERR("~Audio:  deleted midi");

	CERR("~Audio:  about to quit subsystem");
	SDL_QuitSubSystem(SDL_INIT_AUDIO); // SDL 1.1 lets us diddle with
						// subsystems
	CERR("~Audio:  closed audio");

	// Avoid closing SDL audio. This seems to trigger a segfault
	// SDL::CloseAudio();
	SDL_open = false;
	self = 0;
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
#ifdef FUDGE_SAMPLE_RATES
				if (sample_rate == 11111) sample_rate = 11025;
				else if (sample_rate == 22222) sample_rate = 22050;
#endif
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
				//workaround here to exit this loop, it fixes start speech which was
				//causing this function to go exit too early.
				last_chunk=true;

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
		
		// Our input is 8 bit mono unsigned; but want to output 16 bit stereo signed.
		// In addition, the rates don't match, we have to upsample.
#ifndef USE_OLD_RESAMPLE
		// New code: Do it all in one step with cubic interpolation

		sint16 *stereo_data;
		if (is_stereo())
			stereo_data = resample_new(dec_data, dec_len, l, sample_rate, actual.freq);
		else
			stereo_data = resample_new_mono(dec_data, dec_len, l, sample_rate, actual.freq);
#else
		// Old code: resample using pseudo-breshenham, then in a second step convert
		// to 16 bit stereo.

		// Resample to the current rate
		uint8 *new_data;
		size_t new_len;
		resample(dec_data, &new_data, dec_len, &new_len, sample_rate, actual.freq);
		l = new_len;

		COUT("Have " << l << " bytes of resampled data");


		// And convert to 16 bit stereo
		sint16 *stereo_data = new sint16[l*2];
		for(size_t i = 0, j = 0; i < l; i++)
		{
			stereo_data[j++] = (new_data[i] - 128)<<8;
			stereo_data[j++] = (new_data[i] - 128)<<8;
		}
		l <<= 2; // because it's 16bit

		delete [] new_data;
#endif
		// Delete temp buffer
		if (compression == 1) {
			delete [] dec_data;
		}

		chunks.push_back(Chunk(l,(uint8 *)stereo_data));

		data_offset += chunk_length;
	}
	COUT("Turn chunks to block");
	visible_len = l;

	return chunks_to_block(chunks);
}

static	sint16 *resample_new(uint8 *src,
						size_t sourcelen, size_t &size,
						int rate, int wanted_rate)
{
	int fp_pos = 0;
	int fp_speed = (1 << 16) * rate / wanted_rate;
	size = sourcelen;

	// adjust the magnitudes of size and rate to prevent division error
	while (size & 0xFFFF0000)
		size >>= 1, rate = (rate >> 1) + 1;
	
	// Compute the output size (times 4 since it is 16 stereo)
	size = (size * wanted_rate / rate) << 2;

	sint16 *stereo_data = new sint16 [size];
	sint16 *data = stereo_data;
	uint8 *src_end = src + sourcelen;

	int result;
	
	// Compute the initial data feed for the interpolator. We don't simply
	// shift by 8, but rather duplicate the byte, this way we cover the full
	// range. Probably doesn't make a big difference, listening wise :-)
	int a = *(src+0); a = (a|(a << 8))-32768;
	int b = *(src+1); b = (a|(b << 8))-32768;
	int c = *(src+2); c = (a|(c << 8))-32768;
	
	// We divide the data by 2, to prevent overshots. Imagine this sample pattern:
	// 0, 65535, 65535, 0. Now you want to compute a value between the two 65535.
	// Obviously, it will be *bigger* than 65535 (it can get to about 80,000).
	// It is possibly to clamp it, but that leads to a distored wave form. Compare
	// this to turning up the volume of your stereo to much, it will start to sound
	// bad at a certain level (depending on the power of your stereo, your speakers 
	// etc, this can be quite loud, though ;-). Hence we reduce the original range.
	// A factor of roughly 1/1.2 = 0.8333 is sufficient. Since we want to avoid 
	// floating point, we approximate that by 27/32
	#define RANGE_REDUX(x)	(((x) * 27) >> 5)
//	#define RANGE_REDUX(x)	((x) >> 1)
//	#define RANGE_REDUX(x)	((x) / 1.2)

	CubicInterpolator	interp(RANGE_REDUX(a), RANGE_REDUX(b), RANGE_REDUX(c));
	
	do {
		do {
			// Convert to signed data
			result = interp.interpolate(fp_pos);

			// Enforce range in case of an "overshot". Shouldn't happen since we
			// scale down already, but safe is safe.
			if (result < -32768)
				result = -32768;
			else if (result > 32767)
				result = 32767;
	
			*data++ = result;
			*data++ = result;
	
			fp_pos += fp_speed;
		} while (!(fp_pos & 0xFFFF0000));
		src++;
		fp_pos &= 0x0000FFFF;
		

		if (src+2 < src_end) {
			c = *(src+2);
			c = (c|(c << 8))-32768;
			interp.feedData(RANGE_REDUX(c));
		} else
			interp.feedData();

	} while (src < src_end);
	
	return stereo_data;
}

static	sint16 *resample_new_mono(uint8 *src,
						size_t sourcelen, size_t &size,
						int rate, int wanted_rate)
{
	int fp_pos = 0;
	int fp_speed = (1 << 16) * rate / wanted_rate;
	size = sourcelen;

	// adjust the magnitudes of size and rate to prevent division error
	while (size & 0xFFFF0000)
		size >>= 1, rate = (rate >> 1) + 1;
	
	// Compute the output size (times 2 since it is 16 stereo)
	size = (size * wanted_rate / rate) << 1;

	sint16 *stereo_data = new sint16 [size];
	sint16 *data = stereo_data;
	uint8 *src_end = src + sourcelen;

	int result;
	
	// Compute the initial data feed for the interpolator. We don't simply
	// shift by 8, but rather duplicate the byte, this way we cover the full
	// range. Probably doesn't make a big difference, listening wise :-)
	int a = *(src+0); a = (a|(a << 8))-32768;
	int b = *(src+1); b = (a|(b << 8))-32768;
	int c = *(src+2); c = (a|(c << 8))-32768;
	
	// We divide the data by 2, to prevent overshots. Imagine this sample pattern:
	// 0, 65535, 65535, 0. Now you want to compute a value between the two 65535.
	// Obviously, it will be *bigger* than 65535 (it can get to about 80,000).
	// It is possibly to clamp it, but that leads to a distored wave form. Compare
	// this to turning up the volume of your stereo to much, it will start to sound
	// bad at a certain level (depending on the power of your stereo, your speakers 
	// etc, this can be quite loud, though ;-). Hence we reduce the original range.
	// A factor of roughly 1/1.2 = 0.8333 is sufficient. Since we want to avoid 
	// floating point, we approximate that by 27/32
	#define RANGE_REDUX(x)	(((x) * 27) >> 5)
//	#define RANGE_REDUX(x)	((x) >> 1)
//	#define RANGE_REDUX(x)	((x) / 1.2)

	CubicInterpolator	interp(RANGE_REDUX(a), RANGE_REDUX(b), RANGE_REDUX(c));
	
	do {
		do {
			// Convert to signed data
			result = interp.interpolate(fp_pos);

			// Enforce range in case of an "overshot". Shouldn't happen since we
			// scale down already, but safe is safe.
			if (result < -32768)
				result = -32768;
			else if (result > 32767)
				result = 32767;
	
			*data++ = result;
	
			fp_pos += fp_speed;
		} while (!(fp_pos & 0xFFFF0000));
		src++;
		fp_pos &= 0x0000FFFF;
		

		if (src+2 < src_end) {
			c = *(src+2);
			c = (c|(c << 8))-32768;
			interp.feedData(RANGE_REDUX(c));
		} else
			interp.feedData();

	} while (src < src_end);
	
	return stereo_data;
}
		
void	Audio::play(uint8 *sound_data,uint32 len,bool wait)
	{
	Mix_Chunk *wavechunk;

	if (!audio_enabled || !speech_enabled || !len) return;

	if(!strncmp((const char *)sound_data,"Creative Voice File",19))
		sound_data = convert_VOC(sound_data,len);
	
	//Play voice sample using RAW sample we created above.
	// ConvertVOC() produced a stereo, 22KHz, 16bit sample.
	// Currently SDL does not resample very well so we do
	// it in ConvertVOC()
	wavechunk = sfxs->add_from_data(sound_data, len);
	
	int channel = Mix_PlayChannel(-1, wavechunk, 0);
	if (channel < 0)
		{
		sfxs->release(wavechunk);
		CERR("No channel was available to play SFX/voice in Audio::play");
		return;
		}
	Mix_SetPosition(channel, 0, 0);
	//Voice is loud compared to other SFX,music so adjust to match volumes.
	Mix_Volume(channel, MIX_MAX_VOLUME - 40);
	}

void	Audio::cancel_streams(void)
{
	if (!audio_enabled) return;

	Mix_HaltChannel(-1);
}

void	Audio::pause_audio(void)
{
	if (!audio_enabled) return;

	Mix_Pause(-1);
	Mix_PauseMusic();
}

void 	Audio::resume_audio(void)
{
	if (!audio_enabled) return;

	Mix_Resume(-1);
	Mix_ResumeMusic();
}


void Audio::playfile(const char *fname, const char *fpatch, bool wait)
{
	if (!audio_enabled)
		return;

	char *buf=0;
	size_t len;

	U7multiobject sample(fname, fpatch, 1);
	buf = sample.retrieve(len);
	if (!buf || len <= 0)
		{
		// Failed to find file in patch or static dirs.
		CERR("Audio::playfile: Error reading file '" << fname << "'");
		delete [] buf;
		return;
		}

	play(reinterpret_cast<uint8*>(buf), len, wait);
	delete [] buf;
}


bool	Audio::playing(void)
{
	return false;
}


void	Audio::start_music(int num, bool continuous,std::string flex)
{
	if(audio_enabled && music_enabled && midi != 0)
		midi->start_music(num,continuous && allow_music_looping,flex);
}

void	Audio::start_music(std::string fname, int num, bool continuous)
{
	if(audio_enabled && music_enabled && midi != 0)
		midi->start_music(fname,num,continuous && allow_music_looping);
}

void	Audio::start_music_combat (Combat_song song, bool continuous)
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
	
	midi->start_music(num,continuous && allow_music_looping);
}

void	Audio::stop_music()
{
	if (!audio_enabled) return;

	if(midi)
		midi->stop_music();
}

bool Audio::start_speech(int num, bool wait)
{
	if (!audio_enabled || !speech_enabled)
		return false;

	char *buf=0;
	size_t len;
	const char *filename;
	const char *patchfile;

	if (Game::get_game_type() == SERPENT_ISLE)
		{
		filename = SISPEECH;
		patchfile = PATCH_SISPEECH;
		}
	else
		{
		filename = U7SPEECH;
		patchfile = PATCH_U7SPEECH;
		}
	
	U7multiobject sample(filename, patchfile, num);
	buf = sample.retrieve(len);
	if (!buf || len <= 0)
		{
		delete [] buf;
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
int	Audio::play_sound_effect (int num, int volume, int dir, int repeat)
{
	if (!audio_enabled || !effects_enabled) return -1;

	// Where sort of sfx are we using????
	if (sfx_file != 0)		// Digital .wav's?
		return play_wave_sfx(num, volume, dir, repeat);
#ifdef ENABLE_MIDISFX
	else if (midi != 0) 
		midi->start_sound_effect(num);
#endif
	return -1;
}

/*
 *	Play a .wav format sound effect, 
 *  return the channel number playing on or -1 if not playing, (0 is a valid channel in SDL_Mixer!)
 */
int Audio::play_wave_sfx
	(
	int num,
	int volume,			// 0-128.
	int dir,			// 0-15, from North, clockwise.
	int repeat			// Keep playing.
	)
{
	if (!effects_enabled || !sfx_file /*|| !mixer*/) 
		return -1;  // no .wav sfx available

#if 0
	if (Game::get_game_type() == BLACK_GATE)
		num = bgconv[num];
	CERR("; after bgconv:  " << num);
#endif
	if (num < 0 || (unsigned)num >= sfx_file->number_of_objects())
	{
		cerr << "SFX " << num << " is out of range" << endl;
		return -1;
	}
	wave = sfxs->request(sfx_file, num);
	if (!wave)
	{
		cerr << "Couldn't play sfx '" << num << "'" << endl;
		return -1;
	}

	int sfxchannel;
	sfxchannel = Mix_PlayChannel(-1, wave, repeat);
	if (sfxchannel < 0)
		{
		sfxs->release(wave);
		CERR("No channel was available to play sfx '" << num << "'");
		return -1;
		}

	CERR("Playing SFX: " << num);
	Mix_Volume(sfxchannel, volume);
	Mix_SetPosition(sfxchannel, (dir * 22), 0);
	return sfxchannel;
}

/*
 *	Halt sound effects.
 */

void Audio::stop_sound_effects()
{
	if (sfx_file != 0)		// .Wav's?
		Mix_HaltChannel(-1);	
		
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

#ifdef USE_OLD_RESAMPLE
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
#endif

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
	if (reference < 0) {
		reference = inBuf[0] & 0xff;   // use the first byte in the buffer as the reference byte
		bufSize--;                          // remember to skip the reference byte
	}
	
	for (int i = 0; i < bufSize; i++) {
		outBuf[i * 2 + 0] = decode_ADPCM_4_sample(inBuf[i] >> 4, reference, scale);
		outBuf[i * 2 + 1] = decode_ADPCM_4_sample(inBuf[i] >> 0, reference, scale);
	}
}

#endif // PENTAGRAM
