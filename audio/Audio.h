//-*-Mode: C++;-*-
/*
Copyright (C) 2000  Dancer A.L Vesperman

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

#ifndef _Audio_h_
#define _Audio_h_

#if (__GNUG__ >= 2) && (!defined WIN32)
#  pragma interface
#endif

#if !AUTOCONFIGURED
#  include "../autoconfig.h"
#endif

#include "../alpha_kludges.h"

#include <vector>
#include "SDL_mapping.h"
#include <SDL_audio.h>
#include "Mixer.h"
#include "Midi.h"
#include "exceptions.h"
#include "utils.h"

/*
 *	Music:
 */
enum Combat_song
{
	CSBattle_Over,
	CSAttacked1,
	CSAttacked2,
	CSVictory,
	CSRun_Away,
	CSDanger,
	CSHidden_Danger
};

//---- Audio -----------------------------------------------------------

class Audio 
{
private:
	static	Audio	*self;
	bool speech_enabled, music_enabled, effects_enabled;
	bool SDL_open;
	UNREPLICATABLE_CLASS(Audio);
protected:
	Audio();
public:
    void	Init(void);
    void	Init(int _samplerate,int _channels);
    ~Audio();
	static	Audio	*get_ptr(void);
	Mixer	*mixer;

	void	cancel_raw(void);	// Cancel mixed samples
	void	cancel_streams(void);	// Dump any audio streams
	void	play(uint8 *sound_data,uint32 len,bool);
	void	playfile(const char *,bool);
	void	mix_audio(void);
	void	mix(uint8 *sound_data,uint32 len);
	void	mixfile(const char *fname);
	bool	playing(void);
	void	clear(uint8 *,int);
	bool	start_music(int num,bool continuous,int bank=0);
	void	start_music(const char *fname,int num,bool continuous);
	void	start_music(XMIDI *midfile,bool continuous);
	bool	start_music_combat(Combat_song song,bool continuous,int bank=0);
	void	stop_music();
	void	play_sound_effect (int num);
	bool	start_speech(int num,bool wait=false);
	void	set_external_signal(int);
	void	terminate_external_signal(void);
	bool	is_speech_enabled() { return speech_enabled; }
	void	set_speech_enabled(bool ena) { speech_enabled = ena; }
	bool	is_music_enabled() { return music_enabled; }
	void	set_music_enabled(bool ena) { music_enabled = ena; }
	bool	are_effects_enabled() { return effects_enabled; }
	void	set_effects_enabled(bool ena) { effects_enabled = ena; }

	ProducerConsumerBuf	*Create_Audio_Stream(void) { return !mixer?0:mixer->Create_Audio_Stream(); }
	void    Destroy_Audio_Stream(uint32 id) { if(mixer) mixer->Destroy_Audio_Stream(id); }
	bool	is_playing(uint32 id) { return !mixer?0:mixer->is_playing(id); }

	static	const	unsigned int	ringsize=3000;
//	static	const	int	samplerate=11025;
//	static	const	int	persecond=2;
//	static	const	int	buffering_unit=1024;

	MyMidiPlayer *get_midi() {return midi;}

	SDL_AudioSpec actual;
private:
	uint8 * convert_VOC(uint8 *,uint32 &);
	SDL_AudioSpec wanted;
	MyMidiPlayer	*midi;

	void build_speech_vector(void);

};

#endif
