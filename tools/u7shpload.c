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

/* Declare some local functions.
 */
static void   query      (void);
static void   run        (gchar   *name,
                          gint     nparams,
                          GParam  *param,
                          gint    *nreturn_vals,
                          GParam **return_vals);
static gint32 load_image (gchar   *filename);

static GRunModeType run_mode;
static guchar   gimp_cmap[768];

GPlugInInfo PLUG_IN_INFO =
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
  static GParamDef load_args[] =
  {
    { PARAM_INT32, "run_mode", "Interactive, non-interactive" },
    { PARAM_STRING, "filename", "The name of the file to load" },
    { PARAM_STRING, "raw_filename", "The name entered" }
  };
  static GParamDef load_return_vals[] =
  {
    { PARAM_IMAGE, "image", "Output image" }
  };
  static gint nload_args = sizeof (load_args) / sizeof (load_args[0]);
  static gint nload_return_vals = (sizeof (load_return_vals) /
				   sizeof (load_return_vals[0]));

  gimp_install_procedure ("file_shp_load",
                          "loads files in Ultima 7 SHP format",
                          "FIXME: write help for shp_load",
                          "Tristan Tarrant",
                          "Tristan Tarrant",
                          "2000",
                          "<Load>/SHP",
			  NULL,
                          PROC_PLUG_IN,
                          nload_args, nload_return_vals,
                          load_args, load_return_vals);

  gimp_register_magic_load_handler ("file_shp_load",
				    "shp",
				    "",
				    "");
}

static void
run (gchar   *name,
     gint     nparams,
     GParam  *param,
     gint    *nreturn_vals,
     GParam **return_vals)
{
  static GParam values[2];
  GStatusType   status = STATUS_SUCCESS;
  gint32        image_ID;

  run_mode = param[0].data.d_int32;

  *nreturn_vals = 1;
  *return_vals  = values;
  values[0].type          = PARAM_STATUS;
  values[0].data.d_status = STATUS_EXECUTION_ERROR;

  if (strcmp (name, "file_shp_load") == 0)
    {
      image_ID = load_image (param[1].data.d_string);

      if (image_ID != -1)
        {
	  *nreturn_vals = 2;
          values[1].type         = PARAM_IMAGE;
          values[1].data.d_image = image_ID;
        }
      else
        {
          status = STATUS_EXECUTION_ERROR;
        }
    }
  else
    {
      status = STATUS_CALLING_ERROR;
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

unsigned int read4(FILE *f)
{
  unsigned char b0, b1, b2, b3;
  b0 = fgetc(f);
  b1 = fgetc(f);
  b2 = fgetc(f);
  b3 = fgetc(f);
  return (b0 + (b1<<8) + (b2<<16) + (b3<<24));
}

static gint32 load_image (gchar *filename)
{
 	FILE *fp;
	gint32 shape_size;
	gint32 hdr_size;
	guchar *pixptr;
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
	gint16 offsetX;
	gint16 offsetY;
	gchar *name_buf;
	gchar *framename;
	guchar block;
	guchar pix;
	GDrawable *drawable;
	GPixelRgn pixel_rgn;
	int i;
	int j;

	struct u7shape shape;
	struct u7frame *frame;
  
	fp = fopen (filename, "rb");
	if (!fp) {
		g_message ("SHP: can't open \"%s\"\n", filename);
		return -1;
	}

	if (run_mode != RUN_NONINTERACTIVE) {
		name_buf = g_strdup_printf ("Loading %s:", filename);
		gimp_progress_init (name_buf);
		g_free (name_buf);
	}

	shape_size = read4(fp);
	hdr_size = read4(fp);
	shape.num_frames = (hdr_size-4)/4;
	shape.frames = (struct u7frame *)malloc(sizeof(struct u7frame)*shape.num_frames);
	max_width = -1;
	max_height = -1;
	
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
		printf(" Frame #%d - (%d,%d)-(%d,%d)\n",i, frame->leftX, frame->leftY, frame->rightX, frame->rightY );
		frame->pixels = (char *)malloc(frame->width*frame->height*2);
		memset(frame->pixels, 0, frame->width*frame->height*2);
		while((slice=read2(fp))!=0) {
			slice_type = slice & 0x1;
			slice_length = slice >> 1;
			offsetX = read2(fp);
			offsetY = read2(fp);
			pixptr = frame->pixels+(offsetY*frame->width+offsetX)*2;
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
	for(i=0;i<256;i++) {
		gimp_cmap[i*3]=i;
		gimp_cmap[i*3+1]=i;
		gimp_cmap[i*3+2]=i;
	}
	image_ID = gimp_image_new (max_width, max_height, INDEXED);
	gimp_image_set_filename (image_ID, filename);
	gimp_image_set_cmap (image_ID, gimp_cmap, 256);
	for(i=0; i<shape.num_frames; i++) {
		frame = &shape.frames[i];
		framename = g_strdup_printf ("Frame %d", i);
		layer_ID = gimp_layer_new (image_ID, framename,
			frame->width, frame->height,
			INDEXEDA_IMAGE, 100, NORMAL_MODE);
		g_free (framename);
		gimp_image_add_layer (image_ID, layer_ID, 0);
		gimp_layer_translate (layer_ID, (gint)frame->leftX, (gint)frame->leftY);

		drawable = gimp_drawable_get (layer_ID);
		
		gimp_pixel_rgn_init (&pixel_rgn, drawable, 0, 0, drawable->width, drawable->height, TRUE, FALSE);
		gimp_pixel_rgn_set_rect (&pixel_rgn, frame->pixels, 0, 0, drawable->width, drawable->height);

		gimp_drawable_flush (drawable);
		gimp_drawable_detach (drawable);
	}


	fclose(fp);
	
	for(i=0; i<shape.num_frames; i++) {
		free(shape.frames[i].pixels);
	}

	free(shape.frames);

	return 0;
}
