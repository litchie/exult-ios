int img_read(char *filein);
int img_write(char *img_out);
Uint8 getpixel(SDL_Surface *surface, int x, int y);
void putpixel(SDL_Surface *surface,int x, int y, Uint8 pixel);
char *transform(int index);
Uint8 palette_rw(char *col);
int process_image();
