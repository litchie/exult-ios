/*
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef ALPHA_LINUX_CXX
#  include <unistd.h>
#  include <cstdio>
#  include <cmath>
#  include <iostream>
#  include <cmath>
#endif
#include "utils.h"
#include "xmidi.h"
#include "Configuration.h"
extern	Configuration	*config;

using std::atof;
using std::atoi;
using std::cerr;
using std::endl;
using std::free;
using std::malloc;
using std::memcmp;
using std::memcpy;
using std::snprintf;
using std::string;

#include "gamma.h"


// This is used to correct incorrect patch, vol and pan changes in midi files
// The bias is just a value to used to work out if a vol and pan belong with a 
// patch change. This is to ensure that the default state of a midi file is with
// the tracks centred, unless the first patch change says otherwise.
#define PATCH_VOL_PAN_BIAS	5


// This is a default set of patches to convert from MT32 to GM
// The index is the MT32 Patch nubmer and the value is the GM Patch
// This is only suitable for music that doesn'tdo timbre changes
// XMIDIs that contain Timbre changes will not convert properly
const char XMIDI::mt32asgm[128] = {
	0,	// 0	Piano 1
	1,	// 1	Piano 2
	2,	// 2	Piano 3 (synth)
	4,	// 3	EPiano 1
	4,	// 4	EPiano 2
	5,	// 5	EPiano 3
	5,	// 6	EPiano 4
	3,	// 7	Honkytonk
	16,	// 8	Organ 1
	17,	// 9	Organ 2
	18,	// 10	Organ 3
	16,	// 11	Organ 4
	19,	// 12	Pipe Organ 1
	19,	// 13	Pipe Organ 2
	19,	// 14	Pipe Organ 3
	21,	// 15	Accordion
	6,	// 16	Harpsichord 1
	6,	// 17	Harpsichord 2
	6,	// 18	Harpsichord 3
	7,	// 19	Clavinet 1
	7,	// 20	Clavinet 2
	7,	// 21	Clavinet 3
	8,	// 22	Celesta 1
	8,	// 23	Celesta 2
	62,	// 24	Synthbrass 1 (62)
	63,	// 25	Synthbrass 2 (63)
	62,	// 26	Synthbrass 3 Bank 8
	63,	// 27	Synthbrass 4 Bank 8
	38,	// 28	Synthbass 1
	39,	// 29	Synthbass 2
	38,	// 30	Synthbass 3 Bank 8
	39,	// 31	Synthbass 4 Bank 8
	88,	// 32	Fantasy
	90,	// 33	Harmonic Pan - No equiv closest is polysynth(90) :(
	52,	// 34	Choral ?? Currently set to SynthVox(54). Should it be ChoirAhhs(52)???
	92,	// 35	Glass
	97,	// 36	Soundtrack
	99,	// 37	Atmosphere
	14,	// 38	Warmbell, sounds kind of like crystal(98) perhaps Tubular Bells(14) would be better. It is!
	54,	// 39	FunnyVox, sounds alot like Bagpipe(109) and Shania(111)
	98,	// 40	EchoBell, no real equiv, sounds like Crystal(98)
	96,	// 41	IceRain
	68,	// 42	Oboe 2001, no equiv, just patching it to normal oboe(68)
	95,	// 43	EchoPans, no equiv, setting to SweepPad
	81,	// 44	DoctorSolo Bank 8
	87,	// 45	SchoolDaze, no real equiv
	112,	// 46	Bell Singer
	80,	// 47	SquareWave
	48,	// 48	Strings 1
	48,	// 49	Strings 2 - should be 49
	44,	// 50	Strings 3 (Synth) - Experimental set to Tremollo Strings - should be 50
	45,	// 51	Pizzicato Strings
	40,	// 52	Violin 1
	40,	// 53	Violin 2 ? Viola
	42,	// 54	Cello 1
	42,	// 55	Cello 2
	43,	// 56	Contrabass
	46,	// 57	Harp 1
	46,	// 58	Harp 2
	24,	// 59	Guitar 1 (Nylon)
	25,	// 60	Guitar 2 (Steel)
	26,	// 61	Elec Guitar 1
	27,	// 62	Elec Guitar 2
	104,	// 63	Sitar
	32,	// 64	Acou Bass 1
	32,	// 65	Acou Bass 2
	33,	// 66	Elec Bass 1
	34,	// 67	Elec Bass 2
	36,	// 68	Slap Bass 1
	37,	// 69	Slap Bass 2
	35,	// 70	Fretless Bass 1
	35,	// 71	Fretless Bass 2
	73,	// 72	Flute 1
	73,	// 73	Flute 2
	72,	// 74	Piccolo 1
	72,	// 75	Piccolo 2
	74,	// 76	Recorder
	75,	// 77	Pan Pipes
	64,	// 78	Sax 1
	65,	// 79	Sax 2
	66,	// 80	Sax 3
	67,	// 81	Sax 4
	71,	// 82	Clarinet 1
	71,	// 83	Clarinet 2
	68,	// 84	Oboe
	69,	// 85	English Horn (Cor Anglais)
	70,	// 86	Bassoon
	22,	// 87	Harmonica
	56,	// 88	Trumpet 1
	56,	// 89	Trumpet 2
	57,	// 90	Trombone 1
	57,	// 91	Trombone 2
	60,	// 92	French Horn 1
	60,	// 93	French Horn 2
	58,	// 94	Tuba	
	61,	// 95	Brass Section 1
	61,	// 96	Brass Section 2
	11,	// 97	Vibes 1
	11,	// 98	Vibes 2
	99,	// 99	Syn Mallet Bank 1
	112,	// 100	WindBell no real equiv Set to TinkleBell(112)
	9,	// 101	Glockenspiel
	14,	// 102	Tubular Bells
	13,	// 103	Xylophone
	12,	// 104	Marimba
	107,	// 105	Koto
	111,	// 106	Sho?? set to Shanai(111)
	77,	// 107	Shakauhachi
	78,	// 108	Whistle 1
	78,	// 109	Whistle 2
	76,	// 110	Bottle Blow
	76,	// 111	Breathpipe no real equiv set to bottle blow(76)
	47,	// 112	Timpani
	117,	// 113	Melodic Tom
	116,	// 114	Deap Snare no equiv, set to Taiko(116)
	118,	// 115	Electric Perc 1
	118,	// 116	Electric Perc 2
	116,	// 117	Taiko
	115,	// 118	Taiko Rim, no real equiv, set to Woodblock(115)
	119,	// 119	Cymbal, no real equiv, set to reverse cymbal(119)
	115,	// 120	Castanets, no real equiv, in GM set to Woodblock(115)
	112,	// 121	Triangle, no real equiv, set to TinkleBell(112)
	55,	// 122	Orchestral Hit
	124,	// 123	Telephone
	123,	// 124	BirdTweet
	94,	// 125	Big Notes Pad no equiv, set to halo pad (94)
	98,	// 126	Water Bell set to Crystal Pad(98)
	121	// 127	Jungle Tune set to Breath Noise
};

// Same as above, except include patch changes
// so GS instruments can be used
const char XMIDI::mt32asgs[256] = {
	0, 0,	// 0	Piano 1
	1, 0,	// 1	Piano 2
	2, 0,	// 2	Piano 3 (synth)
	4, 0,	// 3	EPiano 1
	4, 0,	// 4	EPiano 2
	5, 0,	// 5	EPiano 3
	5, 0,	// 6	EPiano 4
	3, 0,	// 7	Honkytonk
	16, 0,	// 8	Organ 1
	17, 0,	// 9	Organ 2
	18, 0,	// 10	Organ 3
	16, 0,	// 11	Organ 4
	19, 0,	// 12	Pipe Organ 1
	19, 0,	// 13	Pipe Organ 2
	19, 0,	// 14	Pipe Organ 3
	21, 0,	// 15	Accordion
	6, 0,	// 16	Harpsichord 1
	6, 0,	// 17	Harpsichord 2
	6, 0,	// 18	Harpsichord 3
	7, 0,	// 19	Clavinet 1
	7, 0,	// 20	Clavinet 2
	7, 0,	// 21	Clavinet 3
	8, 0,	// 22	Celesta 1
	8, 0,	// 23	Celesta 2
	62, 0,	// 24	Synthbrass 1 (62)
	63, 0,	// 25	Synthbrass 2 (63)
	62, 0,	// 26	Synthbrass 3 Bank 8
	63, 0,	// 27	Synthbrass 4 Bank 8
	38, 0,	// 28	Synthbass 1
	39, 0,	// 29	Synthbass 2
	38, 0,	// 30	Synthbass 3 Bank 8
	39, 0,	// 31	Synthbass 4 Bank 8
	88, 0,	// 32	Fantasy
	90, 0,	// 33	Harmonic Pan - No equiv closest is polysynth(90) :(
	52, 0,	// 34	Choral ?? Currently set to SynthVox(54). Should it be ChoirAhhs(52)???
	92, 0,	// 35	Glass
	97, 0,	// 36	Soundtrack
	99, 0,	// 37	Atmosphere
	14, 0,	// 38	Warmbell, sounds kind of like crystal(98) perhaps Tubular Bells(14) would be better. It is!
	54, 0,	// 39	FunnyVox, sounds alot like Bagpipe(109) and Shania(111)
	98, 0,	// 40	EchoBell, no real equiv, sounds like Crystal(98)
	96, 0,	// 41	IceRain
	68, 0,	// 42	Oboe 2001, no equiv, just patching it to normal oboe(68)
	95, 0,	// 43	EchoPans, no equiv, setting to SweepPad
	81, 0,	// 44	DoctorSolo Bank 8
	87, 0,	// 45	SchoolDaze, no real equiv
	112, 0,	// 46	Bell Singer
	80, 0,	// 47	SquareWave
	48, 0,	// 48	Strings 1
	48, 0,	// 49	Strings 2 - should be 49
	44, 0,	// 50	Strings 3 (Synth) - Experimental set to Tremollo Strings - should be 50
	45, 0,	// 51	Pizzicato Strings
	40, 0,	// 52	Violin 1
	40, 0,	// 53	Violin 2 ? Viola
	42, 0,	// 54	Cello 1
	42, 0,	// 55	Cello 2
	43, 0,	// 56	Contrabass
	46, 0,	// 57	Harp 1
	46, 0,	// 58	Harp 2
	24, 0,	// 59	Guitar 1 (Nylon)
	25, 0,	// 60	Guitar 2 (Steel)
	26, 0,	// 61	Elec Guitar 1
	27, 0,	// 62	Elec Guitar 2
	104, 0,	// 63	Sitar
	32, 0,	// 64	Acou Bass 1
	32, 0,	// 65	Acou Bass 2
	33, 0,	// 66	Elec Bass 1
	34, 0,	// 67	Elec Bass 2
	36, 0,	// 68	Slap Bass 1
	37, 0,	// 69	Slap Bass 2
	35, 0,	// 70	Fretless Bass 1
	35, 0,	// 71	Fretless Bass 2
	73, 0,	// 72	Flute 1
	73, 0,	// 73	Flute 2
	72, 0,	// 74	Piccolo 1
	72, 0,	// 75	Piccolo 2
	74, 0,	// 76	Recorder
	75, 0,	// 77	Pan Pipes
	64, 0,	// 78	Sax 1
	65, 0,	// 79	Sax 2
	66, 0,	// 80	Sax 3
	67, 0,	// 81	Sax 4
	71, 0,	// 82	Clarinet 1
	71, 0,	// 83	Clarinet 2
	68, 0,	// 84	Oboe
	69, 0,	// 85	English Horn (Cor Anglais)
	70, 0,	// 86	Bassoon
	22, 0,	// 87	Harmonica
	56, 0,	// 88	Trumpet 1
	56, 0,	// 89	Trumpet 2
	57, 0,	// 90	Trombone 1
	57, 0,	// 91	Trombone 2
	60, 0,	// 92	French Horn 1
	60, 0,	// 93	French Horn 2
	58, 0,	// 94	Tuba	
	61, 0,	// 95	Brass Section 1
	61, 0,	// 96	Brass Section 2
	11, 0,	// 97	Vibes 1
	11, 0,	// 98	Vibes 2
	99, 0,	// 99	Syn Mallet Bank 1
	112, 0,	// 100	WindBell no real equiv Set to TinkleBell(112)
	9, 0,	// 101	Glockenspiel
	14, 0,	// 102	Tubular Bells
	13, 0,	// 103	Xylophone
	12, 0,	// 104	Marimba
	107, 0,	// 105	Koto
	111, 0,	// 106	Sho?? set to Shanai(111)
	77, 0,	// 107	Shakauhachi
	78, 0,	// 108	Whistle 1
	78, 0,	// 109	Whistle 2
	76, 0,	// 110	Bottle Blow
	76, 0,	// 111	Breathpipe no real equiv set to bottle blow(76)
	47, 0,	// 112	Timpani
	117, 0,	// 113	Melodic Tom
	116, 0,	// 114	Deap Snare no equiv, set to Taiko(116)
	118, 0,	// 115	Electric Perc 1
	118, 0,	// 116	Electric Perc 2
	116, 0,	// 117	Taiko
	115, 0,	// 118	Taiko Rim, no real equiv, set to Woodblock(115)
	119, 0,	// 119	Cymbal, no real equiv, set to reverse cymbal(119)
	115, 0,	// 120	Castanets, no real equiv, in GM set to Woodblock(115)
	112, 0,	// 121	Triangle, no real equiv, set to TinkleBell(112)
	55, 0,	// 122	Orchestral Hit
	124, 0,	// 123	Telephone
	123, 0,	// 124	BirdTweet
	94, 0,	// 125	Big Notes Pad no equiv, set to halo pad (94)
	98, 0,	// 126	Water Bell set to Crystal Pad(98)
	121, 0	// 127	Jungle Tune set to Breath Noise
};

GammaTable<unsigned char> XMIDI::VolumeCurve(128);

// Constructor
XMIDI::XMIDI(DataSource *source, int pconvert) : events(NULL),timing(NULL),
						convert_type(pconvert),fixed(NULL),
						do_reverb(false), do_chorus(false)
{
	int i = 16;
	while (i--) bank127[i] = 0;
	
	ExtractTracks (source);
}

XMIDI::~XMIDI()
{
	if (events)
	{
		for (int i=0; i < info.tracks; i++)
			DeleteEventList (events[i]);
		//delete [] events;
		free (events);
	}
	if (timing) delete [] timing;
	if (fixed) delete [] fixed;
}

int XMIDI::retrieve (uint32 track, DataSource *dest)
{
	int len = 0;
	
	if (!events)
	{
		cerr << "No midi data in loaded." << endl;
		return 0;
	}

	// Convert type 1 midi's to type 0
	if (info.type == 1)
	{
		DuplicateAndMerge(-1);
		
		for (int i=0; i < info.tracks; i++)
			DeleteEventList (events[i]);
			
		//delete [] events;
		free (events);
		//events = new midi_event *[1];
		events = (midi_event **) malloc (sizeof (midi_event *));
		events[0] = list;

		info.tracks = 1;
		info.type = 0;
	}

	if (track >= info.tracks)
	{
		cerr << "Can't retrieve MIDI data, track out of range" << endl;
		return 0;
	}

	// And fix the midis if they are broken
	if (!fixed[track])
	{
		list = events[track];
		MovePatchVolAndPan ();
		fixed[track] = true;
		events[track] = list;
	}

	// This is so if using buffer datasource, the caller can know how big to make the buffer
	if (!dest)
	{
		// Header is 14 bytes long and add the rest as well
		len = ConvertListToMTrk (NULL, events[track]);
		return 14 + len;
	}
		
	dest->write1 ('M');
	dest->write1 ('T');
	dest->write1 ('h');
	dest->write1 ('d');
	
	dest->write4high (6);

	dest->write2high (0);
	dest->write2high (1);
	dest->write2high (timing[track]);
		
	len = ConvertListToMTrk (dest, events[track]);

	return len + 14;
}

int XMIDI::retrieve (uint32 track, midi_event **dest, int &ppqn)
{
	if (!events)
	{
		cerr << "No midi data in loaded." << endl;
		return 0;
	}

	if ((info.type == 1 && track != 0) || (track >= info.tracks))
	{
		cerr << "Can't retrieve MIDI data, track out of range" << endl;
		return 0;
	}
	DuplicateAndMerge(track);
	MovePatchVolAndPan ();

	*dest = list;
	ppqn = timing[track];

	return 1;
}

void XMIDI::DeleteEventList (midi_event *mlist)
{
	midi_event *event;
	midi_event *next;
	
	next = mlist;
	event = mlist;

	while ((event = next))
	{
		next = event->next;
		if (event->buffer) 
			//delete [] event->buffer;
			free (event->buffer);
		//delete event;
		free (event);
	}
}

// Sets current to the new event and updates list
void XMIDI::CreateNewEvent (int time)
{
	if (!list)
	{
		list = current = (midi_event *) malloc (sizeof (midi_event)); //new midi_event;
		current->next = NULL;
		if (time < 0)
			current->time = 0;
		else
			current->time = time;
		current->buffer = NULL;
		current->len = 0;
		return;
	}

	if (time < 0)
	{
		midi_event *event = (midi_event *) malloc (sizeof (midi_event)); //new midi_event;
		event->next = list;
		list = current = event;
		current->time = 0;
		current->buffer = NULL;
		current->len = 0;
		return;
	}

	if (current->time > time)
		current = list;

	while (current->next)
	{
		if (current->next->time > time)
		{
			midi_event *event = (midi_event *) malloc (sizeof (midi_event)); //new midi_event;
			
			event->next = current->next;
			current->next = event;
			current = event;
			current->time = time;
			current->buffer = NULL;
			current->len = 0;
			return;
		}
		
		current = current->next;
	}

	current->next = (midi_event *) malloc (sizeof (midi_event)); //new midi_event;
	current = current->next;
	current->next = NULL;
	current->time = time;
	current->buffer = NULL;
	current->len = 0;
}


// Conventional Variable Length Quantity
int XMIDI::GetVLQ (DataSource *source, uint32 &quant)
{
	int i;
	quant = 0;
	unsigned int data;

	for (i = 0; i < 4; i++)
	{
		data = source->read1();
		quant <<= 7;
		quant |= data & 0x7F;

		if (!(data & 0x80))
		{
			i++;
			break;
		}

	}
	return i;
}

// XMIDI Delta Variable Length Quantity
int XMIDI::GetVLQ2 (DataSource *source, uint32 &quant)
{
	int i;
	quant = 0;
	int data = 0;
	
	for (i = 0; i < 4; i++)
	{
		data = source->read1();
		if (data & 0x80)
		{
			source->skip(-1);
			break;
		}
		quant += data;
	}
	return i;
}

int XMIDI::PutVLQ(DataSource *dest, uint32 value)
{
	int buffer;
	int i = 1;
	buffer = value & 0x7F;
	while (value >>= 7)
	{
		buffer <<= 8;
		buffer |= ((value & 0x7F) | 0x80);
		i++;
	}
	if (!dest) return i;
	for (int j = 0; j < i; j++)
	{
		dest->write1(buffer & 0xFF);
		buffer >>= 8;
	}
	
	return i;
}

// MovePatchVolAndPan
//
// This little function attempts to correct errors in midi files
// that relate to patch, volume and pan changing
void XMIDI::MovePatchVolAndPan (int channel)
{
	if (channel == -1)
	{
		for (int i = 0; i < 16; i++)
			MovePatchVolAndPan (i);
			
		return;
	}
		
	midi_event *patch = NULL;
	midi_event *vol = NULL;
	midi_event *pan = NULL;
	midi_event *bank = NULL;
	midi_event *reverb = NULL;
	midi_event *chorus = NULL;
	midi_event *temp;
	
	for (current = list; current; )
	{
		if (!patch && (current->status >> 4) == 0xC && (current->status & 0xF) == channel)
			patch = current;
		else if (!vol && (current->status >> 4) == 0xB && current->data[0] == 7 && (current->status & 0xF) == channel)
			vol = current;
		else if (!pan && (current->status >> 4) == 0xB && current->data[0] == 10 && (current->status & 0xF) == channel)
			pan = current;
		else if (!bank && (current->status >> 4) == 0xB && current->data[0] == 0 && (current->status & 0xF) == channel)
			bank = current;

		if (pan && vol && patch) break;

		if (current) current = current->next;
		else current = list;
	}

	// Got no patch change, return and don't try fixing it
	if (!patch) return;


	// Copy Patch Change Event
	temp = patch;
	patch = (midi_event *) malloc (sizeof (midi_event)); //new midi_event;
	patch->time = temp->time;
	patch->status = channel|(MIDI_STATUS_PROG_CHANGE << 4);
	patch->len = 0;
	patch->buffer = NULL;
	patch->data[0] = temp->data[0];


	// Copy Volume
	if (vol && (vol->time > patch->time+PATCH_VOL_PAN_BIAS || vol->time < patch->time-PATCH_VOL_PAN_BIAS))
		vol = NULL;

	temp = vol;
	vol = (midi_event *) malloc (sizeof (midi_event)); //new midi_event;
	vol->status = channel|(MIDI_STATUS_CONTROLLER << 4);
	vol->data[0] = 7;
	vol->len = 0;
	vol->buffer = NULL;

	if (!temp)
	{
		if (convert_type) vol->data[1] = VolumeCurve[64];
		else vol->data[1] = 64;
	}
	else
		vol->data[1] = temp->data[1];


	// Copy Bank
	if (bank && (bank->time > patch->time+PATCH_VOL_PAN_BIAS || bank->time < patch->time-PATCH_VOL_PAN_BIAS))
		bank = NULL;

	temp = bank;
	
	bank = (midi_event *) malloc (sizeof (midi_event)); //new midi_event;
	bank->status = channel|(MIDI_STATUS_CONTROLLER << 4);
	bank->data[0] = 0;
	bank->len = 0;
	bank->buffer = NULL;

	if (!temp)
		bank->data[1] = 0;
	else
		bank->data[1] = temp->data[1];

	// Copy Pan
	if (pan && (pan->time > patch->time+PATCH_VOL_PAN_BIAS || pan->time < patch->time-PATCH_VOL_PAN_BIAS))
		pan = NULL;

	temp = pan;
	pan = (midi_event *) malloc (sizeof (midi_event)); //new midi_event;
	pan->status = channel|(MIDI_STATUS_CONTROLLER << 4);
	pan->data[0] = 10;
	pan->len = 0;
	pan->buffer = NULL;

	if (!temp)
		pan->data[1] = 64;
	else
		pan->data[1] = temp->data[1];

	if (do_reverb)
	{
		reverb = (midi_event *) malloc (sizeof (midi_event)); //new midi_event;
		reverb->time = 0;
		reverb->status = channel|(MIDI_STATUS_CONTROLLER << 4);
		reverb->len = 0;
		reverb->buffer = NULL;
		reverb->data[0] = 91;
		reverb->data[1] = reverb_value;
	}

	if (do_chorus)
	{
		chorus = (midi_event *) malloc (sizeof (midi_event)); //new midi_event;
		chorus->time = 0;
		chorus->status = channel|(MIDI_STATUS_CONTROLLER << 4);
		chorus->len = 0;
		chorus->buffer = NULL;
		chorus->data[0] = 93;
		chorus->data[1] = chorus_value;
	}

	vol->time = 0;
	pan->time = 0;
	patch->time = 0;
	bank->time = 0;
	
	if (do_reverb && do_chorus) reverb->next = chorus;
	else if (do_reverb) reverb->next = bank;
	if (do_chorus) chorus->next = bank;
	bank->next = vol;
	vol->next = pan;
	pan->next = patch;
	
	patch->next = list;
	if (do_reverb) list = reverb;
	else if (do_chorus) list = chorus;
	else list = bank;
}

// DuplicateAndMerge
void XMIDI::DuplicateAndMerge (int num)
{
	int		i;
	midi_event	**track;
	int		time = 0;
	int		start = 0;
	int		end = 1;
	
	if (info.type == 1)
	{
		start = 0;
		end = info.tracks;
	}
	else if (num >= 0 && num < info.tracks)
	{
		start += num;
		end += num;
	}
	
	track = (midi_event **) malloc (sizeof (midi_event*)*info.tracks); //new midi_event *[info.tracks];
	
	for (i = 0; i < info.tracks; i++) track[i] = events[i];
	
	current = list = NULL;

	
	while (1)
	{
		int	lowest = 1 << 30;
		int	selected = -1;
		int	num_na = end-start;
		
		// Firstly we find the track with the lowest time
		// for it's current event
		for (i = start; i < end; i++)
		{
			if (!track[i])
			{
				num_na--;
				continue;
			}
			if (track[i]->time < lowest)
			{
				selected = i;
				lowest = track[i]->time;			
			}
		}
		
		// This is just so I don't have to type [selected] all the time
		i = selected;
		
		// None left to convert
		if (!num_na) break;
	
		// Only need 1 end of track
		// So take the last one and ignore the rest;
		if ((num_na != 1) && (track[i]->status == 0xff) && (track[i]->data[0] == 0x2f))
		{
			track[i] = NULL;
			continue;
		}
		
		if (current)
		{
			current->next = (midi_event *) malloc (sizeof (midi_event)); //new midi_event;
			current = current->next;
		}
		else
			list = current = (midi_event *) malloc (sizeof (midi_event)); //new midi_event;
			
		current->next = NULL;
		
		time = track[i]->time;
		current->time = time;

		current->status = track[i]->status;
		current->data[0] = track[i]->data[0];
		current->data[1] = track[i]->data[1];

		current->len = track[i]->len;
		
		if (current->len)
		{
			current->buffer = (unsigned char *) malloc(current->len);
			memcpy (current->buffer, track[i]->buffer, current->len);
		}
		else
			current->buffer = NULL;
		
		track[i] = track[i]->next;
	}
}


// Converts Events
//
// Source is at the first data byte
// size 1 is single data byte
// size 2 is dual data byte
// size 3 is XMI Note on
// Returns bytes converted

int XMIDI::ConvertEvent (const int time, const unsigned char status, DataSource *source, const int size)
{
	uint32	delta = 0;
	int	data;

	data = source->read1();


	// Bank changes are handled here
	if ((status >> 4) == 0xB && data == 0)
	{
		data = source->read1();
		
		bank127[status&0xF] = false;
		
		if (convert_type == XMIDI_CONVERT_MT32_TO_GM || convert_type == XMIDI_CONVERT_MT32_TO_GS
			|| convert_type == XMIDI_CONVERT_MT32_TO_GS127)
			return 2;

		CreateNewEvent (time);
		current->status = status;
		current->data[0] = 0;
		current->data[1] = data;

		if (convert_type == XMIDI_CONVERT_GS127_TO_GS && data == 127)
			bank127[status&0xF] = true;

		return 2;
	}

	// Handling for patch change mt32 conversion, probably should go elsewhere
	if ((status >> 4) == 0xC && (status&0xF) != 9 && convert_type != XMIDI_CONVERT_NOCONVERSION)
	{
		if (convert_type == XMIDI_CONVERT_MT32_TO_GM)
		{
			data = mt32asgm[data];
		}
		else if ((convert_type == XMIDI_CONVERT_GS127_TO_GS && bank127[status&0xF]) ||
				convert_type == XMIDI_CONVERT_MT32_TO_GS)
		{
			CreateNewEvent (time);
			current->status = 0xB0 | (status&0xF);
			current->data[0] = 0;
			current->data[1] = mt32asgs[data*2+1];

			data = mt32asgs[data*2];
		}
		else if (convert_type == XMIDI_CONVERT_MT32_TO_GS127)
		{
			CreateNewEvent (time);
			current->status = 0xB0 | (status&0xF);
			current->data[0] = 0;
			current->data[1] = 127;
		}
	}// Disable patch changes on Track 10 is doing a conversion
	else if ((status >> 4) == 0xC && (status&0xF) == 9 && convert_type != XMIDI_CONVERT_NOCONVERSION)
	{
		return size;
	}

	CreateNewEvent (time);
	current->status = status;

	current->data[0] = data;

	if (size == 1)
		return 1;

	current->data[1] = source->read1();

	// Volume modify the note on's, only if converting
	if (convert_type && (current->status >> 4) == MIDI_STATUS_NOTE_ON && current->data[1])
		current->data[1] = VolumeCurve[current->data[1]];

	// Volume modify the volume controller, only if converting
	if (convert_type && (current->status >> 4) == MIDI_STATUS_CONTROLLER && current->data[0] == 7)
		current->data[1] = VolumeCurve[current->data[1]];

	if (size == 2)
		return 2;

	// XMI Note On handling

	// This is an optimization
	midi_event *prev = current;
		
	int i = GetVLQ (source, delta);
	CreateNewEvent (time+delta*3);

	current->status = status;
	current->data[0] = data;
	current->data[1] = 0;
	
	// Optimization
	current = prev;

	return i + 2;
}

// Simple routine to convert system messages
int XMIDI::ConvertSystemMessage (const int time, const unsigned char status, DataSource *source)
{
	int i=0;
	
	CreateNewEvent (time);
	current->status = status;
	
	// Handling of Meta events
	if (status == 0xFF)
	{
		current->data[0] = source->read1();
		i++;	
	}

	i += GetVLQ (source, current->len);

	if (!current->len)
	{
		current->buffer = NULL;
		return i;
	}
	
	current->buffer = (unsigned char *) malloc(current->len);

	source->read ((char *) current->buffer, current->len);

	return i+current->len;
}

// XMIDI and Midi to List
// Returns XMIDI PPQN
int XMIDI::ConvertFiletoList (DataSource *source, const bool is_xmi)
{
	int 		time = 0;
	uint32 		data;
	int		end = 0;
	int		tempo = 500000;
	int		tempo_set = 0;
	uint32		status = 0;
	int		play_size = 2;
	int		file_size = source->getSize();
	
	if (is_xmi) play_size = 3;

	while (!end && source->getPos() < file_size)
	{

		if (!is_xmi)
		{
			GetVLQ (source, data);
			time += data;

			data = source->read1();
		
			if (data >= 0x80)
			{
				status = data;
			}
			else
				source->skip (-1);
		}	
		else
		{
			GetVLQ2 (source, data);
			time += data*3;

			status = source->read1();
		}
		
		switch (status >> 4)
		{
			case MIDI_STATUS_NOTE_ON:
			ConvertEvent (time, status, source, play_size);
			break;

			// 2 byte data
			case MIDI_STATUS_NOTE_OFF:
			case MIDI_STATUS_AFTERTOUCH:
			case MIDI_STATUS_CONTROLLER:
			case MIDI_STATUS_PITCH_WHEEL:
			ConvertEvent (time, status, source, 2);
			break;
			

			// 1 byte data
			case MIDI_STATUS_PROG_CHANGE:
			case MIDI_STATUS_PRESSURE:
			ConvertEvent (time, status, source, 1);
			break;
			

			case MIDI_STATUS_SYSEX:
			if (status == 0xFF)
			{
				int	pos = source->getPos();
				uint32	data = source->read1();
				
				if (data == 0x2F) // End
					end = 1;
				else if (data == 0x51 && !tempo_set) // Tempo. Need it for PPQN
				{
					source->skip(1);
					tempo = source->read1() << 16;
					tempo += source->read1() << 8;
					tempo += source->read1();
					tempo *= 3;
					tempo_set = 1;
				}
				else if (data == 0x51 && tempo_set && is_xmi) // Skip any other tempo changes
				{
					GetVLQ (source, data);
					source->skip(data);
					break;
				}
				
				source->seek (pos);
			}
			ConvertSystemMessage (time, status, source);
			break;

			default:
			break;
		}

	}
	return (tempo*3)/25000;
}

// Converts and event list to a MTrk
// Returns bytes of the array
// buf can be NULL
uint32 XMIDI::ConvertListToMTrk (DataSource *dest, midi_event *mlist)
{
	int time = 0;
	midi_event	*event;
	uint32	delta;
	unsigned char	last_status = 0;
	uint32 	i = 8;
	uint32 	j;
	uint32	size_pos=0;
	bool end = false;

	if (dest)
	{
		dest->write1('M');
		dest->write1('T');
		dest->write1('r');
		dest->write1('k');

		size_pos = dest->getPos();
		dest->skip(4);
	}

	for (event = mlist; event && !end; event=event->next)
	{
		delta = (event->time - time);
		time = event->time;

		i += PutVLQ (dest, delta);

		if ((event->status != last_status) || (event->status >= 0xF0))
		{
			if (dest) dest->write1(event->status);
			i++;
		}
		
		last_status = event->status;
		
		switch (event->status >> 4)
		{
			// 2 bytes data
			// Note off, Note on, Aftertouch, Controller and Pitch Wheel
			case 0x8: case 0x9: case 0xA: case 0xB: case 0xE:
			if (dest)
			{
				dest->write1(event->data[0]);
				dest->write1(event->data[1]);
			}
			i += 2;
			break;
			

			// 1 bytes data
			// Program Change and Channel Pressure
			case 0xC: case 0xD:
			if (dest) dest->write1(event->data[0]);
			i++;
			break;
			

			// Variable length
			// SysEx
			case 0xF:
			if (event->status == 0xFF)
			{
				if (event->data[0] == 0x2f) end = true;
				if (dest) dest->write1(event->data[0]);
				i++;
			}
	
			i += PutVLQ (dest, event->len);
			
			if (event->len)
			{
				for (j = 0; j < event->len; j++)
				{
					if (dest) dest->write1(event->buffer[j]); 
					i++;
				}
			}

			break;
			

			// Never occur
			default:
			cerr << "Not supposed to see this" << endl;
			break;
		}
	}

	if (dest)
	{
		int cur_pos = dest->getPos();
		dest->seek (size_pos);
		dest->write4high (i-8);
		dest->seek (cur_pos);
	}
	return i;
}

// Assumes correct xmidi
int XMIDI::ExtractTracksFromXmi (DataSource *source)
{
	int		num = 0;
	signed short	ppqn;
	uint32		len = 0;
	char		buf[32];

	while (source->getPos() < source->getSize() && num != info.tracks)
	{
		// Read first 4 bytes of name
		source->read (buf, 4);
		len = source->read4high();

		// Skip the FORM entries
		if (!memcmp(buf,"FORM",4))
		{
			source->skip (4);
			source->read (buf, 4);
			len = source->read4high();
		}

		if (memcmp(buf,"EVNT",4))
		{
			source->skip ((len+1)&~1);
			continue;
		}

		list = NULL;
		int begin = source->getPos ();
		
		// Convert it
		if (!(ppqn = ConvertFiletoList (source, true)))
		{
			cerr << "Unable to convert data" << endl;
			break;
		}
		timing[num] = ppqn;
		events[num] = list;
	
		// Increment Counter
		num++;

		// go to start of next track
		source->seek (begin + ((len+1)&~1));
	}


	// Return how many were converted
	return num;
}

int XMIDI::ExtractTracksFromMid (DataSource *source)
{
	int	num = 0;
	uint32	len = 0;
	char	buf[32];

	while (source->getPos() < source->getSize() && num != info.tracks)
	{
		// Read first 4 bytes of name
		source->read (buf, 4);
		len = source->read4high();

		if (memcmp(buf,"MTrk",4))
		{
			source->skip (len);
			continue;
		}

		list = NULL;
		int begin = source->getPos ();

		// Convert it
		if (!ConvertFiletoList (source, false))
		{
			cerr << "Unable to convert data" << endl;
			break;
		}
		
		events[num] = list;
		
		// Increment Counter
		num++;		
		source->seek (begin+len);
	}


	// Return how many were converted
	return num;
}

int XMIDI::ExtractTracks (DataSource *source)
{
	uint32		i = 0;
	int		start;
	uint32		len;
	uint32		chunk_len;
	int 		count;
	char		buf[32];

	string s;
	
	config->value("config/audio/midi/reverb/enabled",s,"no");
	if (s == "yes") do_reverb = true;
	config->set("config/audio/midi/reverb/enabled",s,true);

	config->value("config/audio/midi/reverb/level",s,"---");
	if (s == "---") config->value("config/audio/midi/reverb",s,"64");
	reverb_value = atoi(s.c_str());
	if (reverb_value > 127) reverb_value = 127;
	else if (reverb_value < 0) reverb_value = 0;
	config->set("config/audio/midi/reverb/level",reverb_value,true);
	
	config->value("config/audio/midi/chorus/enabled",s,"no");
	if (s == "yes") do_chorus = true;
	config->set("config/audio/midi/chorus/enabled",s,true);

	config->value("config/audio/midi/chorus/level",s,"---");
	if (s == "---") config->value("config/audio/midi/chorus",s,"16");
	chorus_value = atoi(s.c_str());
	if (chorus_value > 127) chorus_value = 127;
	else if (chorus_value < 0) chorus_value = 0;
	config->set("config/audio/midi/chorus/level",chorus_value,true);
	
	config->value("config/audio/midi/volume_curve",s,"---");
	if (s == "---") config->value("config/audio/midi/gamma",s,"1");
	VolumeCurve.set_gamma (atof(s.c_str()));
	snprintf (buf, 32, "%f", VolumeCurve.get_gamma ());
	config->set("config/audio/midi/volume_curve",buf,true);
	

	// Read first 4 bytes of header
	source->read (buf, 4);

	// Could be XMIDI
	if (!memcmp (buf, "FORM", 4))
	{
		// Read length of 
		len = source->read4high();

		start = source->getPos();
		
		// Read 4 bytes of type
		source->read (buf, 4);

		// XDIRless XMIDI, we can handle them here.
		if (!memcmp (buf, "XMID", 4))
		{	
			cerr << "Warning: XMIDI doesn't have XDIR" << endl;
			info.tracks = 1;
			
		} // Not an XMIDI that we recognise
		else if (memcmp (buf, "XDIR", 4))
		{	
			cerr << "Not a recognised XMID" << endl;
			return 0;
			
		} // Seems Valid
		else 
		{
			info.tracks = 0;
		
			for (i = 4; i < len; i++)
			{
				// Read 4 bytes of type
				source->read (buf, 4);

				// Read length of chunk
				chunk_len = source->read4high();
			
				// Add eight bytes
				i+=8;
				
				if (memcmp (buf, "INFO", 4))
				{	
					// Must allign
					source->skip((chunk_len+1)&~1);
					i+= (chunk_len+1)&~1;
					continue;
				}

				// Must be at least 2 bytes long
				if (chunk_len < 2)
					break;
				
				info.tracks = source->read2();
				break;
			}
		
			// Didn't get to fill the header
			if (info.tracks == 0)
			{
				cerr << "Not a valid XMID" << endl;
				return 0;
			}
		
			// Ok now to start part 2
			// Goto the right place
			source->seek (start+((len+1)&~1));
		
			// Read 4 bytes of type
			source->read (buf, 4);

			// Not an XMID
			if (memcmp (buf, "CAT ", 4))
			{
				cerr << "Not a recognised XMID (" << buf[0] << buf[1] << buf[2] << buf[3] << ") should be (CAT )" << endl;
				return 0;	
			}
			
			// Now read length of this track
			len = source->read4high();
			
			// Read 4 bytes of type
			source->read (buf, 4);

			// Not an XMID
			if (memcmp (buf, "XMID", 4))
			{
				cerr << "Not a recognised XMID (" << buf[0] << buf[1] << buf[2] << buf[3] << ") should be (XMID)" << endl;
				return 0;	
			}

		}

		// Ok it's an XMID, so pass it to the ExtractCode

		events = (midi_event **) malloc (sizeof (midi_event*)*info.tracks); //new midi_event *[info.tracks];
		timing = new short[info.tracks];
		fixed = new bool[info.tracks];
		info.type = 0;
		
		for (i = 0; i < info.tracks; i++)
		{
			events[i] = NULL;
			fixed[i] = false;
		}

		count = ExtractTracksFromXmi (source);

		if (count != info.tracks)
		{
			cerr << "Error: unable to extract all (" << info.tracks << ") tracks specified from XMIDI. Only ("<< count << ")" << endl;
			
			int i = 0;
			
			for (i = 0; i < info.tracks; i++)
				DeleteEventList (events[i]);
			
			//delete [] events;
			free (events);
			delete [] timing;
			
			return 0;		
		}

		return 1;
		
	}// Definately a Midi
	else if (!memcmp (buf, "MThd", 4))
	{
		// Simple read length of header
		len = source->read4high();

		if (len < 6)
		{
			cerr << "Not a valid MIDI" << endl;
			return 0;
		}

		info.type = source->read2high();
		
		info.tracks = source->read2high();
		
		events = (midi_event **) malloc (sizeof (midi_event*)*info.tracks); //new midi_event *[info.tracks];
		timing = new short[info.tracks];
		fixed = new bool[info.tracks];
		timing[0] = source->read2high();
		for (i = 0; i < info.tracks; i++)
		{
			timing[i] = timing[0];
			events[i] = NULL;
			fixed[i] = false;
		}

		count = ExtractTracksFromMid (source);

		if (count != info.tracks)
		{
			cerr << "Error: unable to extract all (" << info.tracks << ") tracks specified from MIDI. Only ("<< count << ")" << endl;
			
			for (i = 0; i < info.tracks; i++)
				DeleteEventList (events[i]);
			
			//delete [] events;
			free (events);
			delete [] timing;
			
			return 0;
				
		}

		return 1;
		
	}// A RIFF Midi, just pass the source back to this function at the start of the midi file
	else if (!memcmp (buf, "RIFF", 4))
	{
		// Read len
		len = source->read4();

		// Read 4 bytes of type
		source->read (buf, 4);
		
		// Not an RMID
		if (memcmp (buf, "RMID", 4))
		{
			cerr << "Invalid RMID" << endl;
			return 0;
		}

		// Is a RMID

		for (i = 4; i < len; i++)
		{
			// Read 4 bytes of type
			source->read (buf, 4);
			
			chunk_len = source->read4();
			
			i+=8;
				
			if (memcmp (buf, "data", 4))
			{	
				// Must allign
				source->skip ((chunk_len+1)&~1);
				i+= (chunk_len+1)&~1;
				continue;
			}
			
			return ExtractTracks (source);

		}
		
		cerr << "Failed to find midi data in RIFF Midi" << endl;
		return 0;
	}
	
	return 0;	
}
