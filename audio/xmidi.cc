#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include "utils.h"
#include "xmidi.h"

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
	14,	// 38	Warmbell, sounds kind of like crystal(98) perhaps Tubular Bells(14) would be better. It is!
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

// Constructor
XMIDI::XMIDI(DataSource *source, bool pconvert) : events(NULL),timing(NULL)
{
	convert_from_mt32 = pconvert;
	ExtractTracks (source);
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

int XMIDI::retrieve (uint32 track, DataSource *dest)
{
	int len = 0;
	
	if (!events)
	{
		cerr << "No midi data in loaded." << endl;
		return 0;
	}
	
	if (info.type == 1 && track != 0)
		cerr << "Warning: Type 1 midi's only have one effective track. Result may be unexpected" << endl;
	
	if (track >= info.tracks)
	{
		cerr << "Can't retrieve MIDI data, track out of range" << endl;
		return 0;
	}

	// Header is 14 bytes long
	len += 14;

	if (info.type == 1)
	{
		for (int i = 0; i < info.tracks; i++)
			len += ConvertListToMTrk (NULL, events[i]);
	}
	else
	{
		len += ConvertListToMTrk (NULL, events[track]);
	}
	
	// This is so if using buffer datasource, the caller can know how big to make the buffer
	if (!len || ! dest) return len;
		
	dest->write1 ('M');
	dest->write1 ('T');
	dest->write1 ('h');
	dest->write1 ('d');
	
	dest->write4high (6);

	if (info.type == 1)
	{
		dest->write2high (1);
		dest->write2high (info.tracks);
		dest->write2high (timing[0]);

		for (int i = 0; i < info.tracks; i++)
			ConvertListToMTrk (dest, events[i]);
	}
	else
	{
		dest->write2high (0);
		dest->write2high (1);
		dest->write2high (timing[track]);
		
		ConvertListToMTrk (dest, events[track]);
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
		if (event->buffer) delete [] event->buffer;
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
		current->buffer = NULL;
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
			current->buffer = NULL;
			return;
		}
		
		current = current->next;
	}

	current->next = new midi_event;
	current = current->next;
	current->next = NULL;
	current->time = time;
	current->buffer = NULL;
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
	
	CreateNewEvent (time);
	current->status = status;

	data = current->data[0] = source->read1();

	// Handling for patch change mt32 conversion, probably should go elsewhere
	if (((status >> 4) == 0xC) && convert_from_mt32) 
		current->data[0] = mt32asgm[current->data[0]];

	if (size == 1)
		return 1;

	current->data[1] = source->read1();

	if (size == 2)
		return 2;

	// XMI Note On handling
		
	int i = GetVLQ (source, delta);
	CreateNewEvent (time+delta*3);

	current->status = status;
	current->data[0] = data;
	current->data[1] = 0;
	
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

	if (!current->len) return i;

	current->buffer = new unsigned char[current->len];	

	source->read ((char *) current->buffer, current->len);

	return i+current->len;
}

// XMIDI to List
// Returns PPQN
int XMIDI::ConvertEVNTtoList (DataSource *source)
{
	int 		time = 0;
	uint32 		delta;
	int		end = 0;
	int		tempo = 500000;
	int		tempo_set = 0;
	uint32		status;
	
	while (!end && source->getPos() < source->getSize())
	{
		GetVLQ2 (source, delta);
		time += delta*3;

		status = source->read1();
		
		switch (status >> 4)
		{
			// Note Off
			case 0x8:
			cerr << "Note off is not valid in XMIDI" << endl;
			DeleteEventList (list);
			list = NULL;
			return 0;
			
			
			// Note On
			case 0x9:
			ConvertEvent (time, status, source, 3);
			break;

			// 2 byte data
			// Aftertouch, Controller and Pitch Wheel
			case 0xA: case 0xB: case 0xE:
			ConvertEvent (time, status, source, 2);
			break;
			

			// 1 byte data
			// Program Change and Channel Pressure
			case 0xC: case 0xD:
			ConvertEvent (time, status, source, 1);
			break;
			

			// SysEx
			case 0xF:
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
				else if (data == 0x51 && tempo_set) // Skip any other tempo changes
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

// MIDI to List
// Returns 0 if failed
int XMIDI::ConvertMTrktoList (DataSource *source)
{
	int time = 0;
	int end = 0;
	unsigned char	status = 0;
	uint32		data;
	
	// This is now safe!! Yeah!
	while (!end && source->getPos() < source->getSize())
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
	
		switch (status >> 4)
		{
			// 2 byte data
			// Note Off, Note On, Aftertouch, Controller and Pitch Wheel
			case 0x8: case 0x9: case 0xA: case 0xB: case 0xE:
			ConvertEvent (time, status, source, 2);
			break;
			

			// 1 byte data
			// Program Change and Channel Pressure
			case 0xC: case 0xD:
			ConvertEvent (time, status, source, 1);
			break;
			

			// SysEx
			case 0xF:
			data = source->read1();
			if (status == 0xFF && data == 0x2F) // End of data
				end = 1;
			source->skip(-1);
			ConvertSystemMessage (time, status, source);
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
uint32 XMIDI::ConvertListToMTrk (DataSource *dest, midi_event *mlist)
{
	int time = 0;
	midi_event	*event;
	uint32	delta;
	unsigned char	last_status = 0;
	uint32 	i = 8;
	uint32 	j;
	uint32	size_pos;
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
		if (!(ppqn = ConvertEVNTtoList (source)))
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
		if (!ConvertMTrktoList (source))
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

		events = new (midi_event*)[info.tracks];
		timing = new short[info.tracks];
		info.type = 0;
		
		for (i = 0; i < info.tracks; i++)
			events[i] = NULL;

		count = ExtractTracksFromXmi (source);

		if (count != info.tracks)
		{
			cerr << "Error: unable to extract all (" << info.tracks << ") tracks specified from XMIDI. Only ("<< count << ")" << endl;
			
			int i = 0;
			
			for (i = 0; i < info.tracks; i++)
				DeleteEventList (events[i]);
			
			delete [] events;
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
		
		events = new (midi_event*)[info.tracks];
		timing = new short[info.tracks];
		timing[0] = source->read2high();
		for (i = 0; i < info.tracks; i++)
		{
			timing[i] = timing[0];
			events[i] = NULL;
		}

		count = ExtractTracksFromMid (source);

		if (count != info.tracks)
		{
			cerr << "Error: unable to extract all (" << info.tracks << ") tracks specified from MIDI. Only ("<< count << ")" << endl;
			
			for (i = 0; i < info.tracks; i++)
				DeleteEventList (events[i]);
			
			delete [] events;
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

