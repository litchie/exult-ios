/**
 **	Npc.cc - Non-player characters.
 **
 **	Written: 4/5/99 - JSF
 **/

/*
Copyright (C) 1999  Jeffrey S. Freedman

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

#include <string.h>
#include "npc.h"
#include "fsm.h"
#include "convers.h"

/*
 *	Special sentence ID's:
 */
#define BYE_SENTENCE	10000

/*
 *	Create an attribute.
 */

Npc_attribute::Npc_attribute
	(
	char *nm,
	int mn,				// Min. value.
	int mx				// Max. value.
	) : name(strdup(nm)), min(mn), max(mx)
	{
	}

/*
 *	Create an NPC.
 */

Npc::Npc
	(
	char *nm			// Name.
	) : name(nm), responding_to_sentence(-1), base_topic(0)
	{
	bye = Sentence::create("Bye");
	}

/*
 *	Delete an NPC.
 */

Npc::~Npc
	(
	)
	{
	clear_sentences();
	//++++++++++
	}

/*
 *	Add an fsm.
 */

Fsm *Npc::add_fsm
	(
	char *nm			// Name to give it.
	)
	{
	Fsm *fsm = new Fsm(this, nm);
	if (!base_topic)
		base_topic = fsm;	// Assume first one is the base.
	fsms.append(fsm);
	return (fsm);
	}

/*
 *	Find an FSM by name.
 */

Fsm *Npc::find_fsm
	(
	char *nm
	)
	{
	int cnt = fsms.get_cnt();
	for (int i = 0; i < cnt; i++)
		{
		Fsm *fsm = (Fsm *) fsms.get(i);
		if (fsm && strcmp(fsm->get_name(), nm) == 0)
			return (fsm);
		}
	return (0);
	}

/*
 *	Create and add an attribute.
 *
 *	Output:	->new attribute.
 */

Npc_attribute *Npc::add_attribute
	(
	char *nm,
	int mi, int ma,			// Min., max. values.
	int v				// Starting value.
	)
	{
	Npc_attribute *att = new Npc_attribute(nm, mi, ma);
	att->set_val(v);
	attributes.append(att);		// Add to list.
	return (att);
	}

/*
 *	Find an attribute.
 *
 *	Output:	->att. if found, else 0.
 */

Npc_attribute *Npc::find_attribute
	(
	char *nm
	)
	{
	int cnt = attributes.get_cnt();
	for (int i = 0; i < cnt; i++)
		{
		Npc_attribute *att = (Npc_attribute *) attributes.get(i);
		if (att && strcmp(att->name, nm) == 0)
			return (att);
		}
	return (0);
	}

/*
 *	Clear out all sentences we're sensitive to.
 */

void Npc::clear_sentences
	(
	)
	{
	int cnt = sentences.get_cnt();	// Get # in list.
	for (int i = 0; i < cnt; i++)	// Go through list.
		{
		Sentence_response *sp = (Sentence_response *) sentences.get(i);
		if (sp)
			{
			delete sp;
			sentences.put(i, 0);
			}
		}
	}

/*
 *	Add a sentence we'll respond to, and its handler.
 */

void Npc::add_sentence
	(
	Sentence *s,
	Event_handler *eh
	)
	{
	int cnt = sentences.get_cnt();	// Get # in list.
	int freeindex = cnt;
	for (int i = 0; i < cnt; i++)	// Go through list.
		{
		Sentence_response *sp = (Sentence_response *) sentences.get(i);
		if (!sp)		// Empty slot?
			freeindex = i;
					// Sentence already in list?
		else if (sp->get_sentence() == s &&
					// But not what we're responding to?
			 responding_to_sentence != i)
			{
			sp->add_handler(eh);
			return;
			}
		}
					// Create new one.
	Sentence_response *newsp = new Sentence_response(s);
	newsp->add_handler(eh);
	sentences.put(freeindex, newsp);
	}

/*
 *	Remove a sentence responder.
 */

void Npc::remove_sentence_handler
	(
	Event_handler *eh
	)
	{
	int cnt = sentences.get_cnt();	// Get # in list.
	for (int i = 0; i < cnt; i++)	// Go through list.
		{
		Sentence_response *sp = (Sentence_response *) sentences.get(i);
		if (sp && !sp->remove_handler(eh) &&
					i != responding_to_sentence)
			{		// No more handlers for sentence?
			delete sp;	// Remove sentence.
			sentences.put(i, 0);
			}
		}
	}

/*
 *	Add a start responder.
 */

void Npc::add_start_handler
	(
	Event_handler *eh
	)
	{
	int cnt = starts.get_cnt();	// Get # in list.
	int i;
	for (i = 0; i < cnt; i++)	// Go through list.
		{
		void *each = starts.get(i);
		if (!each || each == eh)
			break;		// Already there, or empty space.
		}
	starts.put(i, eh);
	}

/*
 *	Remove a start responder.
 */

void Npc::remove_start_handler
	(
	Event_handler *eh
	)
	{
	int cnt = starts.get_cnt();	// Get # in list.
	for (int i = 0; i < cnt; i++)	// Go through list.
		{
		void *each = starts.get(i);
		if (each == eh)
			starts.put(i, 0);
		}
	}

/*
 *	Start conversation.
 *
 *	Output:	0 to end conversation.
 */

int Npc::start
	(
	Fsm *topic			// Topic to start with, or 0 for base.
	)
	{
	if (!topic)
		topic = base_topic;	// Better be non-zero.
	new_topic(topic, 1);		// Start it with a clear stack.
	}

/*
 *	Respond when a given sentence has been spoken.
 *
 *	Output:	0 to end conversation (user said Bye).
 */

int Npc::respond_to_sentence
	(
	int i				// Ith sentence (0 is first).
	)
	{
	if (i == BYE_SENTENCE)
		return (0);
	if (i < 0 || i >= sentences.get_cnt())
		return (0);		// Out of range.
	Sentence_response *sp = (Sentence_response *) sentences.get(i);
	if (!sp)
		return (0);
	int resp_save = responding_to_sentence;
	responding_to_sentence = i;
	if (!sp->respond())		// Did we respond?
		;			// ++++++Need a default response.
	if (sp->is_empty())		// No more use for it?
		{
		delete sp;
		sentences.put(i, 0);
		}
	responding_to_sentence = resp_save;
	return (1);
	}

/*
 *	Check the sensitivity list to see if there are any FSM transitions
 *	that can be followed now.
 */

void Npc::check_sensitivity
	(
	)
	{
	Slist copy(sensitivity_list);	// Work with a copy.
	Slist_iterator next(copy);
	Event_handler *each;
	while ((each = (Event_handler *) next()) != 0)
		each->handle();
	}

/*
 *	Change to a new conversation topic.
 */

void Npc::new_topic
	(
	Fsm *topic,
	int clear_stack			// 1 to clear rest off stack.
	)
	{
	Fsm *prev = get_topic();	// Get current topic.
	if (prev)
		prev->deactivate();	// Remove its sentences.
	if (clear_stack)
		topics.clear();		// Clear stack.
	topics.push(topic);
	topic->activate();		// Add its sentences.
	int cnt = starts.get_cnt();	// Go through start actions.
	for (int i = 0; i < cnt; i++)
		{
		Event_handler *eh = (Event_handler *) starts.get(i);
		if (eh)
			eh->handle_start();
		}
	}

/*
 *	Return to the previous topic (without doing its start actions).
 */

void Npc::prev_topic
	(
	)
	{
					// Pop current from stack.
	Fsm *cur = (Fsm *) topics.pop();
	Fsm *prev = get_topic();	// Get prev.
	if (!prev)			// Empty?
		topics.push(cur);	// Just push back.
	else
		{
		cur->deactivate();
		prev->activate();
		}
	}

/*
 *	Get next sentence the NPC is sensitive to.
 *
 *	Output:	0 if no more.
 */

int Npc_sentence_iterator::operator()
	(
	Sentence *& sent,		// Sentence returned.
	int& num			// # of sentence returned.
	)
	{
	if (said_bye)
		return (0);
	Sentence_response *sr;
	while (index < cnt)
		if ((sr = (Sentence_response *) npc->sentences.get(index++))
			!= 0)
			{
			num = index - 1;
			sent = sr->get_sentence();
			return (1);
			}
	if (npc->bye)			// Can we say goodbye?
		{
		num = BYE_SENTENCE;
		sent = npc->bye;
		said_bye = 1;
		return (1);
		}
	return (0);
	}

