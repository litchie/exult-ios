/*
Copyright (C) 2000 The Exult Team

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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


#include "Audio.h"

Audio	audio;
#include <linux/soundcard.h>
#include <linux/awe_voice.h>
#include "Configuration.h"
Configuration *config;


#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "Table.h"
#include "../fnames.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>


SEQ_DEFINEBUF(2048);


int	main(void)
{
	config->read_config_file(USER_CONFIGURATION_FILE);
        string  data_directory;
        config->value("config/disk/u7path",data_directory,".");
        cout << "chdir to " << data_directory << endl;
        chdir(data_directory.c_str());

	Table instrument_patches=AccessTableFile("static/xmidi.mt");
	Flex midi_files=AccessFlexFile("static/adlibmus.dat");

	cout << "Found " << instrument_patches.object_list.size() << " patches" << endl;
	cout << "Found " << midi_files.object_list.size() << " tracks" << endl;

	mkdir("/tmp/u7patch",0755);
	for(size_t i=0;i<instrument_patches.object_list.size();i++)
		{
		uint32 length;
		string s;
		char *t=instrument_patches.read_object(i,length);
		if(t)
			{
			char	buf[64];
			sprintf(buf,"/tmp/u7patch/patch%d",i);
			FILE	*fp=fopen(buf,"wb");
			if(!fp) continue;
			fwrite(t,length,1,fp);
			fclose(fp);
			delete [] t;
			}
		}

	mkdir("/tmp/u7mid",0755);
	for(size_t i=0;i<midi_files.object_list.size();i++)
		{
		uint32 length;
		string s;
		char *t=midi_files.read_object(i,length);
		if(t)
			{
			char	buf[64];
			sprintf(buf,"/tmp/u7mid/midi%d",i);
			FILE	*fp=fopen(buf,"wb");
			if(!fp) continue;
			fwrite(t,length,1,fp);
			fclose(fp);
			delete [] t;
			}
		}

	
	return 0;
}
