#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

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
	size_t	patchlen=l-9+sizeof(awe_patch_info)+sizeof(awe_open_parm);
	char *patch=new char[patchlen];
	awe_patch_info	*ap=(awe_patch_info *)patch;
	awe_open_parm *aop=(awe_open_parm *)(patch+sizeof(awe_patch_info));

#if 0
#define AWE_PAT_TYPE_MISC       0
#define AWE_PAT_TYPE_GM         1
#define AWE_PAT_TYPE_GS         2
#define AWE_PAT_TYPE_MT32       3
#define AWE_PAT_TYPE_XG         4
#define AWE_PAT_TYPE_SFX        5
#define AWE_PAT_TYPE_GUS        6
#define AWE_PAT_TYPE_MAP        7
#endif

	ap->key=AWE_PATCH;
	ap->device_no=device;
	ap->len=sizeof(awe_open_parm)+l-9;
	ap->sf_id=1;
	ap->type=AWE_OPEN_PATCH;
	cout << "Want to write patch to device #" << device << endl;
	cout << "Patch is " << l << "bytes long" << endl;
	cout << "sizeof(ap) = " << sizeof(awe_patch_info) << endl;

	// set up other parameters
	aop->type=AWE_PAT_TYPE_MT32;
	memset(aop->name,0,10);
	memcpy(aop->name,d,9);
	cout << "sizeof(aop) = " << sizeof(awe_open_parm) << endl;

	memcpy(patch+sizeof(awe_open_parm)+sizeof(awe_patch_info),d+9,l-9);
	cout << "Want to write " <<  patchlen << " bytes" << endl;
	// assert(ap->len-9+sizeof(awe_patch_info)=(l-9+sizeof(awe_open_parm)+sizeof(awe_patch_info)));
	SEQ_WRPATCH(patch,patchlen);
	delete [] patch;
	sleep(1);
}

int	main(void)
{
	Table instrument_patches=AccessTableFile("../../u7/static/xmidi.mt");
	cout << "Found " << instrument_patches.object_list.size() << " patches" << endl;
	seq_init();
	cout << "Sequencer initialised" << endl;

	for(size_t i=0;i<instrument_patches.object_list.size();i++)
		{
		uint32 length;
		string s;
		char *t=instrument_patches.read_object(i,length);
		if(t)
			{
			FILE	*fp=fopen("/tmp/patch","wb");
			fwrite(t,length,1,fp);
			fclose(fp);
			return 0;
			load_mt32_patch(t,length);
			delete [] t;
			}
		}

	
	return 0;
	audio.Init(9615*2,2);
	return 0;
}
