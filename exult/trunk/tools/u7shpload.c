/* 
 * SHP loading file filter for The GIMP version 1.1
 *
 * (C) 2000 Tristan Tarrant
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <gtk/gtk.h>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

/* Declare some local functions.
 */
static void   query      (void);
static void   run        (gchar   *name,
                          gint     nparams,
                          GimpParam  *param,
                          gint    *nreturn_vals,
                          GimpParam **return_vals);
static void   load_palette(gchar *filename);
static void   choose_palette (void);
static gint32 load_image (gchar   *filename);
static gint32 save_image (gchar  *filename,
	    gint32  image_ID,
	    gint32  drawable_ID,
	    gint32  orig_image_ID);
static GimpRunModeType run_mode;
static guchar   gimp_cmap[768];

GimpPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

struct u7frame {
	gint16 leftX;
	gint16 leftY;
	gint16 rightX;
	gint16 rightY;
	gint16 width;
	gint16 height;
        gint32 datalen;
	guchar *pixels;
};

struct u7shape {
	int num_frames;
	struct u7frame *frames;
};


MAIN ()

static void
query (void)
{
	static GimpParamDef load_args[] = {
		{ GIMP_PDB_INT32, "run_mode", "Interactive, non-interactive" },
		{ GIMP_PDB_STRING, "filename", "The name of the file to load" },
		{ GIMP_PDB_STRING, "raw_filename", "The name entered" }
	};
	static GimpParamDef load_return_vals[] = {
		{ GIMP_PDB_IMAGE, "image", "Output image" }
	};
	static gint nload_args = sizeof (load_args) / sizeof (load_args[0]);
	static gint nload_return_vals = (sizeof (load_return_vals) /
		sizeof (load_return_vals[0]));

	static GimpParamDef save_args[] = {
		{ GIMP_PDB_INT32, "run_mode", "Interactive, non-interactive" },
		{ GIMP_PDB_IMAGE,    "image",           "Image to save" },
		{ GIMP_PDB_DRAWABLE, "drawable",        "Drawable to save" },
		{ GIMP_PDB_STRING, "filename", "The name of the file to save" },
		{ GIMP_PDB_STRING, "raw_filename", "The name entered" }
	};
	static gint nsave_args = sizeof (save_args) / sizeof (save_args[0]);

	gimp_install_procedure ("file_shp_load",
		"loads files in Ultima 7 SHP format",
		"FIXME: write help for shp_load",
		"Tristan Tarrant",
		"Tristan Tarrant",
		"2000",
		"<Load>/SHP",
		NULL,
		GIMP_PLUGIN,
		nload_args, nload_return_vals,
		load_args, load_return_vals);

	gimp_register_magic_load_handler ("file_shp_load",
		"shp",
		"",
		"");

	gimp_install_procedure ("file_shp_save",
		"Save files in Ultima 7 SHP format",
		"FIXME: write help for shp_save",
		"Tristan Tarrant",
		"Tristan Tarrant",
		"2000",
		"<Save>/SHP",
		"INDEXEDA",
		GIMP_PLUGIN,
		nsave_args, 0,
		save_args, NULL);

	gimp_register_save_handler ("file_shp_save",
		"shp",
		"");
}

static void
run (gchar   *name,
     gint     nparams,
     GimpParam  *param,
     gint    *nreturn_vals,
     GimpParam **return_vals)
{
	static GimpParam values[2];
	GimpPDBStatusType   status = GIMP_PDB_SUCCESS;
	gint32        image_ID;
	gint32        drawable_ID;
	gint32        orig_image_ID;

	run_mode = param[0].data.d_int32;

	*nreturn_vals = 1;
	*return_vals  = values;
	values[0].type          = GIMP_PDB_STATUS;
	values[0].data.d_status = GIMP_PDB_EXECUTION_ERROR;

	if (strcmp (name, "file_shp_load") == 0) {
		gimp_ui_init ("u7shp", FALSE);
		choose_palette();
		image_ID = load_image (param[1].data.d_string);

		if (image_ID != -1) {
			*nreturn_vals = 2;
			values[1].type         = GIMP_PDB_IMAGE;
			values[1].data.d_image = image_ID;
		} else {
			status = GIMP_PDB_EXECUTION_ERROR;
		}
	} else if (strcmp (name, "file_shp_save") == 0) {
		image_ID    = orig_image_ID = param[1].data.d_int32;
		drawable_ID = param[2].data.d_int32;
		save_image (param[3].data.d_string,
			image_ID,
			drawable_ID,
			orig_image_ID);
	} else{
		status = GIMP_PDB_CALLING_ERROR;
	}
	values[0].data.d_status = status;
}

unsigned int read1(FILE *f)
{
	unsigned char b0;
	b0 = fgetc(f);
	return b0;
}

unsigned int read2(FILE *f)
{
	unsigned char b0, b1;
	b0 = fgetc(f);
	b1 = fgetc(f);
	return (b0 + (b1<<8));
}

/* Flauschepelz */
signed int read2signed(FILE *f)
{
        unsigned char b0, b1;
        signed int i0;
        b0 = fgetc(f);
        b1 = fgetc(f);
        i0 = b0 + (b1<<8);
        if (i0 >= 32768) { i0 = i0 - 65536; }
        return (i0);
}

unsigned int read4(FILE *f)
{
	unsigned char b0, b1, b2, b3;
	b0 = fgetc(f);
	b1 = fgetc(f);
	b2 = fgetc(f);
	b3 = fgetc(f);
	return (b0 + (b1<<8) + (b2<<16) + (b3<<24));
}

void write1(FILE *f, unsigned char b)
{
	fputc(b, f);
}

void write2(FILE *f, unsigned int b)
{
	fputc(b & 0xFF, f);
	fputc((b >> 8) & 0xFF, f);
}

void write4(FILE *f, unsigned int b)
{
	fputc(b & 0xFF, f);
	fputc((b >> 8) & 0xFF, f);
	fputc((b >> 16) & 0xFF, f);
	fputc((b >> 24) & 0xFF, f);
}

unsigned char *out1(unsigned char *p, unsigned char b)
{
    *p++ = b;
    return p;
}

unsigned char *out2(unsigned char *p, unsigned int b)
{
    *p++ = b & 0xFF;
    *p++ = (b >> 8) & 0xFF;
    return p;
}

unsigned char *out4(unsigned char *p, unsigned int b)
{
    *p++ = b & 0xFF;
    *p++ = (b >> 8) & 0xFF;
    *p++ = (b >> 16) & 0xFF;
    *p++ = (b >> 24) & 0xFF;
    return p;
}

static void set_gray_palette()
{
	int i;

	for(i=0;i<256;i++) {
		gimp_cmap[i*3]=i;
		gimp_cmap[i*3+1]=i;
		gimp_cmap[i*3+2]=i;
	}
}

static void load_palette (gchar *filename)
{
	FILE *fp;
	long len;
	int i;
	
	fp = fopen (filename, "rb");
	if (!fp) {
		set_gray_palette();
		return;
	}
	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if(len==768) {
		for(i=0;i<256;i++) {
			gimp_cmap[i*3]=read1(fp) << 2;
			gimp_cmap[i*3+1]=read1(fp) << 2;
			gimp_cmap[i*3+2]=read1(fp) << 2;
		}
	} else if(len==1536) {
		for(i=0;i<256;i++) {
			gimp_cmap[i*3]=read1(fp) << 2;
			read1(fp);
			gimp_cmap[i*3+1]=read1(fp) << 2;
			read1(fp);
			gimp_cmap[i*3+2]=read1(fp) << 2;
			read1(fp);
		}
	} else {
		set_gray_palette();	
	}
	fclose(fp);
}

static void file_sel_delete( GtkWidget *widget, GtkWidget **file_sel )
{
	gtk_widget_destroy( *file_sel );
	*file_sel = NULL;
}

static void file_selected( GtkWidget *widget, gboolean *selected )
{
	*selected = TRUE;
}

gchar* file_select( gchar *title )
{
	GtkWidget *file_sel;
	gchar *filename;
	gboolean selected = FALSE;

	file_sel = gtk_file_selection_new( title );
	gtk_window_set_modal( GTK_WINDOW( file_sel ), TRUE );

	gtk_signal_connect( GTK_OBJECT( file_sel ), "destroy", 
                            GTK_SIGNAL_FUNC( file_sel_delete ), &file_sel );
	
        gtk_signal_connect( GTK_OBJECT( GTK_FILE_SELECTION( file_sel )->cancel_button ), "clicked", GTK_SIGNAL_FUNC( file_sel_delete ), &file_sel );
        
        gtk_signal_connect( GTK_OBJECT( GTK_FILE_SELECTION( file_sel )->ok_button ), "clicked", GTK_SIGNAL_FUNC( file_selected ), &selected );

        gtk_widget_show( file_sel );

	while( ( ! selected ) && ( file_sel ) )
		gtk_main_iteration();

	/* canceled or window was closed */
	if( ! selected )
		return NULL;

	/* ok */
	filename = g_strdup( gtk_file_selection_get_filename( GTK_FILE_SELECTION( file_sel ) ) );
	gtk_widget_destroy( file_sel );
	return filename;
}



static void choose_palette()
{
	load_palette(file_select("Choose palette"));
}


static gint32 load_image (gchar *filename)
{
 	FILE *fp;
	gint32 file_size;
	gint32 shape_size;
	gint32 hdr_size;
	guchar *pixptr, *eod;
	gint32 frame_offset;
	gint16  slice;
	gint32 image_ID = -1;
	gint32 layer_ID = -1;
	gint16 slice_type;
	gint16 slice_length;
	gint16 block_type;
	gint16 block_length;
	gint32 max_width;
	gint32 max_height;
	signed int offsetX;
	signed int offsetY;
	gchar *framename;
	guchar block;
	guchar pix;
	GimpDrawable *drawable;
	GimpPixelRgn pixel_rgn;
	GimpImageType image_type;
	int i;
	int j;

        /* Flauschepelz */
        signed int temp_int;

	struct u7shape shape;
	struct u7frame *frame;
  
	fp = fopen (filename, "rb");
	if (!fp) {
		g_message ("SHP: can't open \"%s\"\n", filename);
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	shape_size = read4(fp);
	
	if(file_size!=shape_size) {	/* 8x8 tile */
		image_type = GIMP_INDEXED_IMAGE;
		shape.num_frames = file_size/64;
		fseek(fp, 0, SEEK_SET);		/* Return to start of file */
		printf("num_frames = %d\n", shape.num_frames);
		shape.frames = g_new(struct u7frame, shape.num_frames);
		max_width = 8;
		max_height = 8;
		for(i=0; i<shape.num_frames; i++) {
			frame = &shape.frames[i];
			frame->width = 8;
			frame->height = 8;
			frame->leftX = 0;
			frame->leftY = 0;
			frame->pixels = (char *)g_malloc(64);
			fread(frame->pixels, 1, 64, fp);
		}
	} else {
		image_type = GIMP_INDEXEDA_IMAGE;
		hdr_size = read4(fp);
		shape.num_frames = (hdr_size-4)/4;
		max_width = -1;
		max_height = -1;
		printf("num_frames = %d\n", shape.num_frames);
		shape.frames = g_new(struct u7frame, shape.num_frames);
	
		for(i=0; i<shape.num_frames; i++) {
			frame = &shape.frames[i];
			// Go to where frame offset is stored
			fseek(fp, (i+1)*4, SEEK_SET);
			frame_offset = read4(fp);
			fseek(fp, frame_offset, SEEK_SET);
			frame->rightX = read2(fp);
			frame->leftX = read2(fp);
			frame->leftY = read2(fp);
			frame->rightY = read2(fp);
			frame->width = frame->leftX+frame->rightX+1;
			if(frame->width>max_width)
				max_width = frame->width;
			frame->height = frame->leftY+frame->rightY+1;
			if(frame->height>max_height)
				max_height = frame->height;
			pixptr = frame->pixels = g_new0(char, frame->width*frame->height*2);
			eod = frame->pixels+frame->width*frame->height*2;
			while((slice=read2(fp))!=0) {
				slice_type = slice & 0x1;
				slice_length = slice >> 1;

                                /* Flauschepelz */
				offsetX = read2signed(fp);
				offsetY = read2signed(fp);

                                temp_int = (frame->leftY + offsetY)*frame->width*2 +
                                           (frame->leftX + offsetX)*2;

                                pixptr = frame->pixels;
                                pixptr = pixptr + temp_int;

                                /*
				pixptr = frame->pixels+(offsetY*frame->width+offsetX)*2;
                                */

				if(pixptr<frame->pixels)
					pixptr = frame->pixels;
				if(slice_type) {	// Compressed
					while(slice_length>0) {
						block = read1(fp);
						block_type = block & 0x1;
						block_length = block >> 1;
						if(block_type) {
							pix = read1(fp);
							for(j=0;j<block_length;j++) {
								*pixptr++ = pix;
								*pixptr++ = 255;
							}
						} else {
							for(j=0;j<block_length;j++) {
								pix = read1(fp);
								*pixptr++ = pix;
								*pixptr++ = 255;
							}
						}
						slice_length -= block_length;
					}
				} else {		// Uncompressed
					// Just read the pixels
					for(j=0;j<slice_length;j++) {
						pix = read1(fp);
						*pixptr++ = pix;
						*pixptr++ = 255;
					}
				}
			}
		}
	}
	image_ID = gimp_image_new (max_width, max_height, GIMP_INDEXED);
	gimp_image_set_filename (image_ID, filename);
	gimp_image_set_cmap (image_ID, gimp_cmap, 256);
	for(i=0; i<shape.num_frames; i++) {
		framename = g_strdup_printf ("Frame %d", i);
		frame = &shape.frames[i];
		layer_ID = gimp_layer_new (image_ID, framename,
			frame->width, frame->height,
			image_type, 100, GIMP_NORMAL_MODE);
		g_free (framename);
		gimp_image_add_layer (image_ID, layer_ID, 0);
		gimp_layer_translate (layer_ID, (gint)0, (gint)0);

		drawable = gimp_drawable_get (layer_ID);
		
		gimp_pixel_rgn_init (&pixel_rgn, drawable, 0, 0, drawable->width, drawable->height, TRUE, FALSE);
		gimp_pixel_rgn_set_rect (&pixel_rgn, frame->pixels, 0, 0, drawable->width, drawable->height);

		gimp_drawable_flush (drawable);
		gimp_drawable_detach (drawable);
	}


	fclose(fp);
	
	for(i=0; i<shape.num_frames; i++) {
		g_free(shape.frames[i].pixels);
	}

	g_free(shape.frames);

	return image_ID;
}

static int find_runs(short *runs, unsigned char *pixptr, int x,	int w)
{
    int runcnt = 0;
    while (x < w && pixptr[1] != 0) {	// Stop at first transparent pixel.
	int run = 0;		// Look for repeat.
	while (x < w - 1 && pixptr[0] == pixptr[2] && pixptr[3]!=0) {
	    if(pixptr[3]==0)
		printf("Warning: found pixel pair, but second is transparent\n");
	    x++;
	    pixptr+=2;
	    run++;
	}
	if (run) {		// Repeated?  Count 1st, shift, flag.
	    run = ((run + 1)<<1)|1;
	    x++;		// Also pass the last one.
	    pixptr+=2;
	} else {
		do {			/* Pass non-repeated run of */
		    x++;
		    pixptr+=2;
		    run += 2;	// So we don't have to shift.
		} while (x < w && pixptr[1] != 0 &&
		 (x == w - 1 || pixptr[0] != pixptr[2]));
	}
	// Store run length.
	runs[runcnt++] = run;
    }
    runs[runcnt] = 0;		/* 0-delimit list. */
    return x;
}

static int skip_transparent(unsigned char **pixptr, int x, int w)
{
    unsigned char *pixel = *pixptr;
    while (x < w && pixel[1] == 0) {	/* Pixel is transparent if alpha is 0 */
	x++;
	pixel+=2;
    }
    *pixptr = pixel;
    return x;
}

static gint32 save_image (gchar  *filename,
	    gint32  image_ID,
	    gint32  drawable_ID,
	    gint32  orig_image_ID)
{
 	FILE *fp;
	gint32 shape_size;
	gint32 hdr_size;
	gint32 max_width;
	gint32 max_height;
	gint offsetX;
	gint offsetY;
	gchar *name_buf;
	GimpDrawable *drawable;
	GimpPixelRgn pixel_rgn;
	guchar *pixptr;
	guchar *pix;
	guchar *out;
	guchar *outptr;
	int i;
	int j;
	int k;
	int newx;
	int x;
	int y;
	gint32 *layers;   
	int nlayers;
	int pos;

	struct u7shape shape;
	struct u7frame *frame;
  
	if (run_mode != GIMP_RUN_NONINTERACTIVE) {
		name_buf = g_strdup_printf ("Saving %s:", filename);
		gimp_progress_init (name_buf);
		g_free (name_buf);
	}
	/* get a list of layers for this image_ID */
	layers = gimp_image_get_layers (image_ID, &nlayers);  

	max_width = gimp_image_width (image_ID);
	max_height = gimp_image_height (image_ID);

	shape.num_frames = nlayers;
	hdr_size = (nlayers+1)*4;
	shape.frames = (struct u7frame *)malloc(sizeof(struct u7frame)*shape.num_frames);

	for (k=0;k<nlayers;k++) {
		frame = &shape.frames[k];
		drawable = gimp_drawable_get (layers[k]);
		gimp_drawable_offsets (layers[k], &offsetX, &offsetY);
		frame->width = drawable->width;
		frame->height = drawable->height;
		frame->leftX = offsetX;
		frame->leftY = offsetY;
		frame->rightX = frame->width-frame->leftX-1;
		frame->rightY = frame->height-frame->leftY-1;
		pix = (gchar *)malloc(frame->width*frame->height*2);
		pixptr = pix;
		out = (gchar *)malloc(frame->width*frame->height*8);
		outptr = out;
		gimp_pixel_rgn_init (&pixel_rgn, drawable, 0, 0,
				     drawable->width, drawable->height, FALSE, FALSE);
		gimp_pixel_rgn_get_rect (&pixel_rgn, pixptr, 0, 0,
					 drawable->width, drawable->height);

		newx = 0;
		for (y = 0; y < frame->height; y++) {
		    for (x = 0; (x = skip_transparent(&pixptr, x, frame->width)) < frame->width; x = newx) {
			short runs[100];
			newx = find_runs(runs, pixptr, x, frame->width);
			if (!runs[1] && !(runs[0]&1)) {
			    int len = runs[0] >> 1;
			    outptr = out2(outptr, runs[0]);
			    outptr = out2(outptr, x - frame->leftX);
			    outptr = out2(outptr, y - frame->leftY);
			    for(i=0; i<len; i++) {
				*outptr++ = *pixptr;
				pixptr +=2;
			    }
			} else {
			    outptr = out2(outptr, ((newx - x)<<1)|1);
			    outptr = out2(outptr, x - frame->leftX);
			    outptr = out2(outptr, y - frame->leftY);
			    for (i = 0; runs[i]; i++) {
				int len = runs[i]>>1;
				if (runs[i]&1) {
				    while (len) {
					int c = len > 127 ? 127 : len;
					*outptr++ = (c<<1)|1;
					*outptr++ = *pixptr;
					pixptr += (c*2);
					len -= c;
				    }
				} else while (len > 0) {
				    int c;
				    c = len > 127 ? 127 : len;
				    *outptr++ = c<<1;
				    for(j=0; j<c; j++) {
					*outptr++ = *pixptr;
					pixptr +=2;
				    }
				    len -= c;
				}
			    }
			}
		    }
		}
		outptr = out2(outptr, 0);			// End with 0 length.
		frame->datalen = outptr - out;
		frame->pixels = (gchar *) malloc(frame->datalen);
		memcpy(frame->pixels, out, frame->datalen);
		free(out);
		free(pix);
		gimp_drawable_detach(drawable);
	}

	fp = fopen (filename, "wb");
	if (!fp) {
		g_message ("SHP: can't create \"%s\"\n", filename);
		return -1;
	}

	shape_size = 0;
	write4(fp, shape_size);	// Fill in later
	for(i=0; i<shape.num_frames; i++) 
	    write4(fp, 0);	// Fill in later
	for(i=0; i<shape.num_frames; i++) {
	    frame = &shape.frames[shape.num_frames-i-1];
	    pos = ftell(fp);	// Get the frame offset
	    fseek(fp, (i+1)*4, SEEK_SET);
	    write4(fp, pos);	// Write it in the right place
	    fseek(fp, pos, SEEK_SET);
	    write2(fp, frame->rightX);
	    write2(fp, frame->leftX);
	    write2(fp, frame->leftY);
	    write2(fp, frame->rightY);
	    fwrite(frame->pixels, 1, frame->datalen, fp);
	}

	pos = ftell(fp);	// Get the file size
	fseek(fp, 0, SEEK_SET);
	write4(fp, pos);	// Write it in the right place

	fclose(fp);
	
	for(i=0; i<shape.num_frames; i++) {
	    free(shape.frames[i].pixels);
	}
	free(shape.frames);

	return 0;
}
