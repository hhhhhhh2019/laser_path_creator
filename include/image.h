#ifndef RENDER_H
#define RENDER_H


typedef struct {
	int width;
	int height;
	char** data;
	char* zbuffer;
} RenderImage;


RenderImage init_image(int,int);
void save_output(RenderImage, char*);
void free_image(RenderImage);

void set_pixel(RenderImage, int,int, char,char,char);
void get_pixel(RenderImage, int,int, char*,char*,char*);


#endif // RENDER_H
