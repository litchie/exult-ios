
#if __GNUG__ >= 2
#  pragma implementation
#endif


#include "Midi.h"

#include <unistd.h>
#include <csignal>


static  void    playmidifile(void)
{
#if 0
        int     fh1=open("/dev/sequencer",O_WRONLY);
        char    c;
        if(fh1==-1)
                {
                perror("open");
                return;
                }
        int     fh2=open("/tmp/u7midi",O_RDONLY);
        if(fh2==-1)
                {
                perror("open2");
                close(fh1);
                return;
                }
        while(read(fh2,&c,1)>0)
                write(fh1,&c,1);
        close(fh1);
        close(fh2);
#else
        execlp("playmidi","-v","-v","-e","/tmp/u7midi",0);
#endif
}


void    MidiPlayer::start_track(int num)
{
        cout << "\007";
        cout << "Audio subsystem request: Music track # " << num << endl;
        uint32  length;
        char    *music=midi_tracks.read_object(num,length);
        if(!music)
                return;
        FILE    *fp;
        unlink("/tmp/u7midi");
        fp=fopen("/tmp/u7midi","wb");
        if(!fp)
                {
                delete [] music;
                return;
                }
        fwrite(music,length,1,fp);
        fclose(fp);
        if(forked_job!=-1)
                {
                kill(forked_job,SIGKILL);
                forked_job=-1;
                }
        forked_job=fork();
        if(!forked_job)
                {
                playmidifile();
                raise(SIGKILL);
                }
}

void	MidiPlayer::start_music(int num)
{
	current_track=num;
	start_track(num);
}


MidiPlayer::MidiPlayer()	: current_track(-1),forked_job(-1)
{
	midi_tracks=AccessFlexFile("static/adlibmus.dat");
}

MidiPlayer::~MidiPlayer()
{}

