
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

#include <iostream.h>
#include "npc.h"
#include "fsm.h"
#include "convers.h"
#include "cond.h"

/*
 *	An action to print a response:
 */
class Print_action : public Action
	{
	char *msg;			// What to print (without a NL).
public:
	Print_action(char *m) : msg(m)
		{  }
	virtual void execute(Npc *npc)
		{ cout << msg << '\n'; }
	};

/*
 *	Maint test program.
 */
int main
	(
	int argc,
	char *argv[]
	)
	{
	Sentence::init();		// Initialize.
	Npc *sara = new Npc("Sara");
					// Set up mood attribute.
	Npc_attribute *mood = sara->add_attribute("mood", 0, 100, 50);
	Fsm *ask_name = sara->add_fsm("ask_name");
	Fsm *ask_job = sara->add_fsm("ask_job");
	/*
	 *	Base:
	 */
	Fsm *base = sara->add_fsm("base");
	Fsm_state *base0 = base->add_state("base0");
	Fsm_transition *base_start = base->add_transition(
			new Start_condition(), base0, base0);
	base_start->add_action(new Print_action(
					"I'm busy, so make it quick!"));
					// Name?
	Fsm_transition *base_tr0 = base->add_transition(
			new Sentence_condition(Sentence::create("Name?")),
				base0, base0);
	base_tr0->add_action(new Conversation_topic_action(ask_name));
					// Job?
	Fsm_transition *base_tr1 = base->add_transition(
			new Sentence_condition(Sentence::create("Job?")),
				base0, base0);
	base_tr1->add_action(new Conversation_topic_action(ask_job));
	base->set_cur_state(base0);	// Set initial state.

	/*
	 *	Name-asking:
	 */
	Fsm_state *name0 = ask_name->add_state("name0");
	Fsm_state *name1 = ask_name->add_state("name1");
	Fsm_state *name2 = ask_name->add_state("annoyed");
	Fsm_transition *name_start = ask_name->add_transition(
			new Start_condition(), name0, name1);
	name_start->add_action(new Print_action("My name is Sara."));
	name_start->add_action(new Conversation_prev_action);
	Fsm_transition *name_tr1 = ask_name->add_transition(
			new Start_condition(), name1, name1);
	name_tr1->add_action(new Print_action("How many times do I have to tell you?  It's Sara."));
					// Gets annoyed each time.
	name_tr1->add_action(new Npc_attribute_action(mood, 
		new Binary_expr(Expr::minus, new Npc_att_expr(mood),
						new Int_expr(10))));
	name_tr1->add_action(new Conversation_prev_action);
	Fsm_transition *name_annoyed = ask_name->add_transition(
			new Expr_condition(new Binary_expr(Expr::lt,
				new Npc_att_expr(mood),
				new Int_expr(20))),
			name1, name2);
	Fsm_transition *name_tr2 = ask_name->add_transition(
			new Start_condition(), name2, name2);
	name_tr2->add_action(new Print_action("You're really starting to get on my nerves!  Is 'Sara' that hard to remember?"));
	name_tr2->add_action(new Conversation_prev_action);
	ask_name->set_cur_state(name0);	// Set to initial state.
#if 0
	/*
	 *	Job asking:
	 */
	Fsm_state *job0 = ask_job->add_state("job0");
	Fsm_state *job1 = ask_job->add_state("job1");
	Fsm_transition *job_tr0 = ask_job->add_transition(
			new Sentence_condition(Sentence::create("Job?")),
					job0, job1);
	job_tr0->add_action(new Print_action(
			"I wander the streets of Trinsic..."));
					// Just talk about jobs.
	job_tr0->add_action(new Conversation_topic_action(1));
	Fsm_transition *job_tr1 = ask_job->add_transition(
			new Sentence_condition(Sentence::create("Job?")),
					job1, job1);
	job_tr1->add_action(new Print_action(
			"Would you let me finish?"));
	Fsm_state *job2 = ask_job->add_state("job2");
	Fsm_transition *job_tr2 = ask_job->add_transition(
			new Sentence_condition(Sentence::create("Doing...?")),
					job1, job2);
	job_tr2->add_action(new Print_action(
			"...looking for an honest man."));
	Fsm_transition *job_tr3 = ask_job->add_transition(
			new Sentence_condition(Sentence::create(
				"Never mind.")), job1, job0);
	job_tr3->add_action(new Print_action("Then why did you ask?"));
	job_tr3->add_action(new Conversation_level_action(-1));
	Fsm_transition *job_tr4 = ask_job->add_transition(
			new Sentence_condition(Sentence::create(
				"I am an honest man.")), job2, job0);
	job_tr4->add_action(new Print_action(
			"That's what they all say."));
	job_tr4->add_action(new Conversation_level_action(-1));
	ask_job->set_cur_state(job0);
#endif
	int quit = 0;
	sara->start(base);		// Do any starting actions.
					// Main loop.
	do
		{
					// What can we say?
		Npc_sentence_iterator next_sent(sara);
		int index;
		Sentence *sent;
		cout << "Say:";
		int indices[10];
		int num_indices = 0;
		while (next_sent(sent, index))
			{
			cout << "  " << sent->get_text() << 
				"(" << num_indices << ")";
			indices[num_indices++] = index;
			}
		char response;
		cin >> response;
		if (response >= '0' && response <= '9')
			{
			index = response - '0';
			if (index < num_indices)
				if (!sara->respond_to_sentence(indices[index]))
					quit = 1;
			}
		}
	while (!quit);
	return (0);
	}
