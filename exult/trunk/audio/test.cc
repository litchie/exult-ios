#include "Audio.h"

Audio	audio;
#include <linux/soundcard.h>
#include <linux/awe_voice.h>
#include "Configuration.h"
Configuration config;


#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "Table.h"

SEQ_DEFINEBUF(2048);

int	seqfd;

struct synth_info card_info;

void seqbuf_dump()
     {
             if (_seqbufptr)
                     if (write(seqfd, _seqbuf, _seqbufptr) == -1) {
                             perror("write /dev/sequencer");
                             exit(-1);
                     }
             _seqbufptr = 0;
     }

	int	device;
void seq_init()
{
        int nrsynths;

        if ((seqfd = open("/dev/sequencer", O_WRONLY, 0)) < 0) {
                perror("open /dev/sequencer");
                exit(-1);
        }

        if (ioctl(seqfd, SNDCTL_SEQ_NRSYNTHS, &nrsynths) == -1) {
                perror("there is no soundcard");
                exit(-1);
        }
        device = -1;
        for (int i = 0; i < nrsynths; i++) {
                card_info.device = i;
                if (ioctl(seqfd, SNDCTL_SYNTH_INFO, &card_info) == -1) {
                        perror("cannot get info on soundcard");
                        exit(-1);
                }
                if (card_info.synth_type == SYNTH_TYPE_SAMPLE
                    && card_info.synth_subtype == SAMPLE_TYPE_AWE32) {
                        device = i;
                        break;
                }
        }

        if (device < 0) {
                perror("No AWE synth device is found");
                exit(-1);
        }
}

void	load_mt32_patch(const char *d,size_t l)
{
	awe_patch_info	ap;
	awe_open_parm aop;

	ap.key=AWE_PATCH;
	ap.device_no=device;
	ap.len=sizeof(aop)+l;
	ap.type=AWE_OPEN_PATCH;

	// set up other parameters
	aop.type=AWE_PAT_TYPE_MT32;
	aop.name[0]=0;

	char *patch=new char[l+ap.len+sizeof(ap)];
	memcpy(patch,&ap,sizeof(ap));
	memcpy(patch+sizeof(ap),&aop,sizeof(aop));
	memcpy(patch+sizeof(ap)+sizeof(aop),d,l);
	SEQ_WRPATCH(patch,l+ap.len+sizeof(ap));
	delete [] patch;
}

int	main(void)
{
	Table instrument_patches=AccessTableFile("../../u7/static/xmidi.mt");
	cout << "Found " << instrument_patches.object_list.size() << " patches" << endl;

	for(size_t i=0;i<instrument_patches.object_list.size();i++)
		{
		uint32 length;
		string s;
		char *t=instrument_patches.read_object(i,length);
		cout << i <<" " << length << endl;
		s="/tmp/patches/patch";
		char	buf[32];
		sprintf(buf,"%d",i);
		cout << "open " << s << endl;
		s+=buf;
		FILE *fp=fopen(s.c_str(),"wb");
		fwrite(t,length,1,fp);
		fclose(fp);
		if(t)
			delete [] t;
		}

	
	seq_init();
	return 0;
	audio.Init(9615*2,2);
	return 0;
}
