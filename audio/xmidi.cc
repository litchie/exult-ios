#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include "utils.h"
#include "../files/U7file.h"
#include "xmidi.h"

#define PATCH_CONVERT_DEFAULT true

// This is a default set of patches to convert from MT32 to GM
// The index is the MT32 Patch nubmer and the value is the GM Patch
unsigned char XMIDI::mt32asgm[128] = {
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
	62,	// 24	Synthbrass 1
	63,	// 25	Synthbrass 2
	62,	// 26	Synthbrass 3 Bank 8
	63,	// 27	Synthbrass 4 Bank 8
	38,	// 28	Synthbass 1
	39,	// 29	Synthbass 2
	38,	// 30	Synthbass 3 Bank 8
	39,	// 31	Synthbass 4 Bank 8
	88,	// 32	Fantasy
	90,	// 33	Harmonic Pan - No equiv closest is polysynth(90) :(
	54,	// 34	Choral ?? Currently set to SynthVox(54). Should it be ChoirAhhs(52)???
	92,	// 35	Glass
	97,	// 36	Soundtrack
	99,	// 37	Atmosphere
	98,	// 38	Warmbell, sounds kind of link crystal(98) perhaps Tubular Bells(14) would be better 
	109,	// 39	FunnyVox, sounds alot like Bagpipe(109) and Shania(111)
	98,	// 40	EchoBell, no real equiv, sounds like Crystal(88
	96,	// 41	IceRain
	68,	// 42	Oboe 2001, no equiv, just patching it to normal oboe(68)
	95,	// 43	EchoPans, no equiv, setting to SweepPad
	81,	// 44	DoctorSolo Bank 8
	87,	// 45	SchoolDaze, no real equiv
	112,	// 46	Bell Singer
	80,	// 47	SquareWave
	48,	// 48	Strings 1
	49,	// 49	Strings 2
	50,	// 50	Strings 3 (Synth)
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
	121,	// 127	Jungle Tune set to Breath Noise
};

// Constructors
XMIDI::XMIDI(const char *fname) : events(NULL),timing(NULL), convert_from_mt32(PATCH_CONVERT_DEFAULT)
{
	ExtractTracksFromFile (fname);
}

XMIDI::XMIDI(const std::string &fname) : events(NULL),timing(NULL), convert_from_mt32(PATCH_CONVERT_DEFAULT)
{	
	ExtractTracksFromFile (fname.c_str());
}

XMIDI::XMIDI(const unsigned char *stream, std::size_t len):events(NULL), timing(NULL), convert_from_mt32(PATCH_CONVERT_DEFAULT)
{
	ExtractTracks (stream);
}

XMIDI::~XMIDI()
{
	if (events)
	{
		for (int i=0; i < info.tracks; i++)
			DeleteEventList (events[i]);
		delete [] events;
	}
	if (timing) delete [] timing;
}

int XMIDI::retrieve (uint32 track, unsigned char **buffer, std::size_t *len)
{
	*len = 0;
	
	if (!events)
	{
		cerr << "No midi data in loaded." << endl;
		return 0;
	}
	
	if ((info.type == 1 && track != 0) ||  (track >= info.tracks))
	{
		cerr << "Can't retrieve MIDI data, track out of range" << endl;
		return 0;
	}

	int	i;

	*len += 14;

	if (info.type == 1)
	{
		for (i = 0; i < info.tracks; i++)
			*len += ConvertListToMTrk (NULL, events[i]);
	}
	else
	{
		*len += ConvertListToMTrk (NULL, events[track]);
	}
	
	if (!*len) return 0;
		
	*buffer = new unsigned char[*len];

	(*buffer)[0] = 'M';
	(*buffer)[1] = 'T';
	(*buffer)[2] = 'h';
	(*buffer)[3] = 'd';
	
	(*buffer)[4] = 0;
	(*buffer)[5] = 0;
	(*buffer)[6] = 0;
	(*buffer)[7] = 6;

	(*buffer)[12] = timing[track] >> 8;
	(*buffer)[13] = timing[track] & 0xFF;

	if (info.type == 1)
	{
		(*buffer)[8] = 0;
		(*buffer)[9] = 1;

		(*buffer)[10] = 0;
		(*buffer)[11] = info.tracks;

		int count = 14;
		
		for (i = 0; i < info.tracks; i++)
			count += ConvertListToMTrk ((*buffer)+count, events[i]);
	}
	else
	{
		(*buffer)[8] = 0;
		(*buffer)[9] = 0;

		(*buffer)[10] = 0;
		(*buffer)[11] = 1;
		
		ConvertListToMTrk ((*buffer)+14, events[track]);
	}

	return *len;
}

int XMIDI::retrieve (uint32 track, const char *fname)
{
	FILE *mfile;
	
	unsigned char	*buffer;
	size_t		len;
	
	if (!events)
	{
		cerr << "No midi data in loaded." << endl;
		return 0;
	}

	if (retrieve (track, &buffer, &len))
	{
		mfile = U7open (fname, "wb");
	
		if (!mfile)
		{
			cerr << "Unable to open midi '" << fname << "' file for writing" << endl;
			delete [] buffer;
			return 0;
		}
		
		fwrite (buffer, len, 1, mfile);

		delete [] buffer;
		fclose (mfile);
	}
	return len;
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
		if (event->stream) delete [] event->stream;
		delete event;
	}
}

// Sets current to the new event and updates list
void XMIDI::CreateNewEvent (int time)
{
	if (!list)
	{
		list = current = new midi_event;
		current->next = NULL;
		current->time = time;
		current->stream = NULL;
		return;
	}

	if (current->time > time)
		current = list;

	while (current->next)
	{
		if (current->next->time > time)
		{
			midi_event *event = new midi_event;
			
			event->next = current->next;
			current->next = event;
			current = event;
			current->time = time;
			current->stream = NULL;
			return;
		}
		
		current = current->next;
	}

	current->next = new midi_event;
	current = current->next;
	current->next = NULL;
	current->time = time;
	current->stream = NULL;
}


int XMIDI::GetVLQ (const unsigned char *stream, uint32 *quant)
{
	int i;
	*quant = 0;
	
	for (i = 0; i < 4; i++)
	{
		*quant <<= 7;
		*quant |= stream[i] & 0x7F;
		
		if (!(stream[i] & 0x80))
		{
			i++;
			break;
		}
	}

	return i;
}

int XMIDI::GetVLQ2 (const unsigned char *stream, uint32 *quant)
{
	int i;
	*quant = 0;
	
	for (i = 0; i < 4 && !(stream[i] & 0x80); i++)
		*quant += stream[i];
	
	return i;
}

int XMIDI::PutVLQ(unsigned char *stream, uint32 value)
{
	int buffer;
	int i = 1;
	int j = 0;
	buffer = value & 0x7F;

	while ( (value >>= 7) )
	{
		buffer <<= 8;
		buffer |= ((value & 0x7F) | 0x80);
		i++;
	}

	while (stream)
	{
		stream[j] = buffer;// & 0x7F;
		if (buffer & 0x80)
			buffer >>= 8;
		else
			break;
		j++;
	}

	return i;
}


// Converts Events
//
// Pointer to stream is at the status byte
// size 1 is single data byte
// size 2 is dual data byte
// size 3 is XMI Note on
// Returns bytes converted

int XMIDI::ConvertEvent (const int time, const unsigned char status, const unsigned char *stream, const int size)
{
	int i;
	uint32 delta = 0;
	
	CreateNewEvent (time);
	current->status = status;

	if (((status >> 4) == 0xC) && convert_from_mt32) 
		current->data[0] = mt32asgm[stream[0]];
	else
		current->data[0] = stream[0];

	if (size == 1)
		return 1;

	current->data[1] = stream[1];

	if (size == 2)
		return 2;

	i = GetVLQ (stream+2, &delta);

	CreateNewEvent (time+delta);

	current->status = status;
	current->data[0] = stream[0];
	current->data[1] = 0;
	
	return i + 2;
}

// Simple routine to convert system messages
int XMIDI::ConvertSystemMessage (const int time, const unsigned char *stream)
{
	int i=1;
	
	CreateNewEvent (time);
	current->status = stream[0];
	
	// Handling of Meta events
	if (stream[0] == 0xFF)
	{
		current->data[0] = stream[1];
		i++;	
	}

	i += GetVLQ (stream+i, &current->len);

	if (!current->len) return i;

	current->stream = new unsigned char[current->len];	

	memcpy (current->stream, stream+i, current->len);

	return i+current->len;
}

// XMIDI to List
// Returns PPQN
int XMIDI::ConvertEVNTtoList (const unsigned char *stream)
{
	int time = 0;
	uint32 delta;
	int end = 0;
	int	tempo = 500000;
	int	tempo_set = 0;
	
	int i = 0;
	
	for (i = 0; !end; )
	{
		switch (stream[i] >> 4)
		{
			// Note Off
			case 0x8:
			cerr << "Note off not valid in XMIDI " << endl;
			DeleteEventList (list);
			list = NULL;
			return 0;
			
			
			// Note On
			case 0x9:
			i+= 1+ConvertEvent (time, stream[i], stream+i+1, 3);
			break;

			// 2 byte data
			// Aftertouch, Controller and Pitch Wheel
			case 0xA: case 0xB: case 0xE:
			i+= 1+ConvertEvent (time, stream[i], stream+i+1, 2);
			break;
			

			// 1 byte data
			// Program Change and Channel Pressure
			case 0xC: case 0xD:
			i+= 1+ConvertEvent (time, stream[i], stream+i+1, 1);
			break;
			

			// SysEx
			case 0xF:
			if (stream[i] == 0xFF)
			{
				if (stream[i+1] == 0x2F) // End
					end = 1;
				else if (stream[i+1] == 0x51 && !tempo_set) // Tempo. Need it for PPQN
				{
					tempo = stream[i+5];
					tempo |= stream[i+4] << 8;
					tempo |= stream[i+3] << 16;
					tempo_set = 1;
				}
			}
			i+= ConvertSystemMessage (time, stream+i);
			break;


			// Delta T
			default:
			i+= GetVLQ2 (stream+i, &delta);
			time += delta;
			break;
		}

	}
	
	return ((tempo*3)+1)/25000;
}

// MIDI to List
// Returns 0 if failed
int XMIDI::ConvertMTrktoList (const unsigned char *stream)
{
	int time = 0;
	uint32 delta;
	int end = 0;
	unsigned char	status = 0;
	
	int i = 0;
	
	for (i = 0; !end; )
	{
		i+= GetVLQ (stream+i, &delta);
		time += delta;

		switch (stream[i] >> 4)
		{
			case 0x8: case 0x9: case 0xA: case 0xB:
			case 0xC: case 0xD: case 0xE:
			status = stream[i];
			i++;
			break;
			
			case 0xF:
			status = stream[i];
			break;
			
			default:
			break;
		}
		
		switch (status >> 4)
		{
			// 2 byte data
			// Note Off, Note On, Aftertouch, Controller and Pitch Wheel
			case 0x8: case 0x9: case 0xA: case 0xB: case 0xE:
			i+= ConvertEvent (time, status, stream+i, 2);
			break;
			

			// 1 byte data
			// Program Change and Channel Pressure
			case 0xC: case 0xD:
			i+= ConvertEvent (time, status, stream+i, 1);
			break;
			

			// SysEx
			case 0xF:
			if (stream[i] == 0xFF && stream[i+1] == 0x2F) // End of data
				end = 1;
			i+= ConvertSystemMessage (time, stream+i);
			break;

			default:
			break;
		}

	}
	
	return 1;
}


// Converts and event list to a MTrk
// Returns bytes of the array
// buf can be NULL
uint32 XMIDI::ConvertListToMTrk (unsigned char *buf, midi_event *mlist)
{
	int time = 0;
	midi_event	*event;
	uint32	delta;
	unsigned char	last_status = 0;
	uint32 	i = 8;
	uint32 	j;
	uint32	size;

	if (buf)
	{
		size = ConvertListToMTrk (NULL, mlist) - 8;
		
		buf[0] = 'M';
		buf[1] = 'T';
		buf[2] = 'r';
		buf[3] = 'k';

		buf[4] = (unsigned char) ((size >> 24) & 0xFF);
		buf[5] = (unsigned char) ((size >> 16) & 0xFF);
		buf[6] = (unsigned char) ((size >> 8) & 0xFF);
		buf[7] = (unsigned char) (size & 0xFF);
	}

	for (event = mlist; event; event=event->next)
	{
		delta = (event->time - time);
		time = event->time;

		if (buf) i += PutVLQ (buf+i, delta);
		else i += PutVLQ (NULL, delta);

		if ((event->status != last_status) || (event->status >= 0xF0))
		{
			if (buf) buf[i] = event->status;
			i++;
		}
		
		last_status = event->status;
		
		switch (event->status >> 4)
		{
			// 2 bytes data
			// Note off, Note on, Aftertouch, Controller and Pitch Wheel
			case 0x8: case 0x9: case 0xA: case 0xB: case 0xE:
			if (buf)
			{
				buf[i] = event->data[0];
				buf[i+1] = event->data[1];
			}
			i += 2;
			break;
			

			// 1 bytes data
			// Program Change and Channel Pressure
			case 0xC: case 0xD:
			if (buf) buf[i] = event->data[0];
			i++;
			break;
			

			// Variable length
			// SysEx
			case 0xF:
			if (event->status == 0xFF)
			{
				if (buf) buf[i] = event->data[0];
				i++;
			}
	
			if (buf) i += PutVLQ (buf+i, event->len);
			else i += PutVLQ (NULL, event->len);
			
			if (event->len)
			{
				for (j = 0; j < event->len; j++)
				{
					if (buf) buf[i] = event->stream[j]; 
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
		
	return i;
}

int XMIDI::ExtractTracksFromXmi (const unsigned char *stream, const uint32 size)
{
	uint32		i = 0;
	int		num = 0;
	signed short	ppqn;
	
	uint32	len = 0;

	while (i < size && num != info.tracks)
	{
		// Allign
		i+= (len+1)&~1;

		// Skip the FORM entries
		if (!memcmp(stream+i,"FORM",4))
			i+= 12;

		len = stream[i+7];
		len |= stream[i+6] << 8;
		len |= stream[i+5] << 16;
		len |= stream[i+4] << 24;

		i+= 8;

		if (memcmp(stream+i-8,"EVNT",4))
			continue;

		list = NULL;

		// Convert it
		if (!(ppqn = ConvertEVNTtoList (stream+i)))
		{
			cerr << "Unable to convert data" << endl;
			break;
		}

		timing[num] = ppqn;
		events[num] = list;
	
		// Increment Counter
		num++;
	}


	// Return how many were converted
	return num;
}

int XMIDI::ExtractTracksFromMid (const unsigned char *stream, const uint32 size)
{
	uint32	i = 0;
	int		num = 0;
	
	uint32	len = 0;

	while (i < size && num != info.tracks)
	{
		i+= len;

		len = stream[i+7];
		len |= stream[i+6] << 8;
		len |= stream[i+5] << 16;
		len |= stream[i+4] << 24;

		i+= 8;

		if (memcmp(stream+i-8,"MTrk",4))
			continue;

		list = NULL;

		// Convert it
		if (!ConvertMTrktoList (stream+i))
		{
			cerr << "Unable to convert data" << endl;
			break;
		}
		
		events[num] = list;
		
		// Increment Counter
		num++;		
	}


	// Return how many were converted
	return num;
}

int XMIDI::ExtractTracks (const unsigned char *stream)
{
	uint32	i = 0;
	int		j = 0;
	uint32	len;
	uint32	chunk_len;
	int 		count;
	
	// Could be XMIDI
	if (!memcmp (stream, "FORM", 4))
	{
		// XDIRless XMIDI
		if (!memcmp (stream+8, "XMID", 4))
		{	
			cerr << "Warning: Xmidi might not be valid" << endl;
			info.tracks = 1;
			i+=4;
			
		} // Not an XMIDI
		else if (memcmp (stream+8, "XDIR", 4))
		{	
			cerr << "Not a XMID" << endl;
			return 0;
			
		} // Seems Valid
		else 
		{
			// Is an XMIDI
			// Assume correct length??
			// No way!
			
			len = stream[7];
			len |= stream[6] << 8;
			len |= stream[5] << 16;
			len |= stream[4] << 24;
			
			info.tracks = 0;
		
			for (i = 12; i < len; i++)
			{
				chunk_len = stream[i+3];
				chunk_len |= stream[i+2] << 8;
				chunk_len |= stream[i+1] << 16;
				chunk_len |= stream[i] << 24;
			
				i+=8;
				
				if (memcmp (stream+i-8, "INFO", 4))
				{	
					// Must allign
					i+= (chunk_len+1)&~1;
					continue;
				}

				// Must be at least 2 bytes long
				if (chunk_len < 2)
					break;
				
				info.tracks = stream[i];
				info.tracks |= stream[i+1] << 8;
			
				break;
			}
		
			// Didn't get to fill the header
			if (info.tracks == 0)
			{
				cerr << "Not a valid XMID" << endl;
				return 0;
			}
		
			// Ok now to start part 2

			i = len + 8;
		
			// Not an XMID
			if (memcmp (stream+i, "CAT ", 4))
			{
				cerr << "Not a recognised XMID (" << stream[i] << stream[i+1] << stream[i+2] << stream[i+3] << ")" << endl;
				return 0;	
			}
			i += 4;
		}
		
		len = stream[i+3];
		len |= stream[i+2] << 8;
		len |= stream[i+1] << 16;
		len |= stream[i] << 24;
	
		i+=4;

		// Not an XMID
		if (memcmp (stream+i, "XMID", 4))
		{
			cerr << "Not a recognised XMID (" << stream[i] << stream[i+1] << stream[i+2] << stream[i+3] << ")" << endl;
			return 0;	
		}
		
		i+=4;

		// Ok it's an XMID, so pass it to the ExtractCode

		events = new midi_event*[info.tracks];
		timing = new short[info.tracks];
		info.type = 0;
		
		for (j = 0; j < info.tracks; j++)
			events[j] = NULL;

		count = ExtractTracksFromXmi (stream+i, len);

		if (count != info.tracks)
		{
			cerr << "Error: unable to extract all (" << info.tracks << ") tracks specified from XMIDI. Only ("<< count << ")" << endl;
			
			int j = 0;
			
			for (j = 0; j < info.tracks; j++)
				DeleteEventList (events[i]);
			
			delete [] events;
			delete [] timing;
			
			return 0;
				
		}

		return 1;
		
	}// Definately a Midi
	else if (!memcmp (stream, "MThd", 4))
	{
		// Simple read length of header
		len = stream[7];
		len |= stream[6] << 8;
		len |= stream[5] << 16;
		len |= stream[4] << 24;

		if (len < 6)
		{
			cerr << "Not a valid MIDI" << endl;
			return 0;
		}

		info.type = stream[8] << 8;
		info.type |= stream[9];
		
		info.tracks = stream[10] << 8;
		info.tracks |= stream[11];
		
		
		events = new midi_event*[info.tracks];
		timing = new short[info.tracks];
		for (j = 0; j < info.tracks; j++)
		{
			timing[j] = stream[12] << 8;
			timing[j] |= stream[13];
			events[j] = NULL;
		}

		count = ExtractTracksFromMid (stream+14, 0xFFFFFFFF);

		if (count != info.tracks)
		{
			cerr << "Error: unable to extract all (" << info.tracks << ") tracks specified from MIDI. Only ("<< count << ")" << endl;
			
			int j = 0;
			
			for (j = 0; j < info.tracks; j++)
				DeleteEventList (events[i]);
			
			delete [] events;
			delete [] timing;
			
			return 0;
				
		}
		
		return 1;
		
	}// A RIFF Midi, just pass the stream back to this function at an offset
	else if (!memcmp (stream, "RIFF", 4))
	{
		// Not an RMID
		if (memcmp (stream+8, "RMID", 4))
		{
			cerr << "Invalid RMID" << endl;
			return 0;
		}


		// Is a RMID
		// Assume correct length??
		// No way!
			
		len = stream[4];
		len |= stream[5] << 8;
		len |= stream[6] << 16;
		len |= stream[7] << 24;

		for (i = 12; i < len; i++)
		{
			chunk_len = stream[i];
			chunk_len |= stream[i+1] << 8;
			chunk_len |= stream[i+2] << 16;
			chunk_len |= stream[i+3] << 24;
			
			i+=8;
				
			if (memcmp (stream+i-8, "data", 4))
			{	
				// Must allign
				i+= (chunk_len+1)&~1;
				continue;
			}
			
			return ExtractTracks (stream+i);

		}
		
		cerr << "Failed to find midi data in RMI" << endl;
		return 0;
	}
	
	return 0;	
}

int XMIDI::ExtractTracksFromFile (const char *fname)
{
	FILE	*mfile;
	
	mfile = U7open (fname, "rb");
	
	if (!mfile)
	{
		cerr << "Unable to open midi file" << endl;
		return 1;
	}
	
	fseek (mfile, 0, SEEK_END);	
	int size = ftell (mfile);
	rewind (mfile);
	
	unsigned char *stream = new unsigned char[size];
	
	fread (stream, size, 1, mfile);
	
	fclose (mfile);
	
	int ret_valu = ExtractTracks (stream);
	
	delete [] stream;
	
	return ret_valu;
}


