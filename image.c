#include <image.h>
#include <png.h>
#include <errno.h>
#include <stdlib.h>





RenderImage init_image(int width, int height) {
	char** image_data = malloc(sizeof(char*) * height);

	for (int i = 0; i < height; i++) {
		image_data[i] = calloc(sizeof(char) * width * 3, 1);
	}

	return (RenderImage){
		width,
		height,
		image_data,
		calloc(sizeof(char) * width * height, 1)
	};
}


void save_output(RenderImage image, char* name) {
	FILE* f = fopen(name, "wb");

	if (!f) {
		perror("fopen");
		exit(errno);
	}


	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr) {
		perror("png_create_write_struct");
		exit(errno);
	}


	png_infop info_ptr = png_create_info_struct(png_ptr);

	if (!info_ptr) {
		perror("png_create_info_struct");
		exit(errno);
	}


	png_init_io(png_ptr, f);

	png_set_IHDR(png_ptr, info_ptr, image.width, image.height, 8,
	  PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
	  PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);


	png_write_info(png_ptr, info_ptr);
	png_write_image(png_ptr, (unsigned char**)image.data);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);


	fclose(f);
}


void free_image(RenderImage image) {
	for (int i = 0; i < image.height; i++)
		free(image.data[i]);
	free(image.data);
	free(image.zbuffer);
}


void set_pixel(RenderImage image, int x, int y, char r, char g, char b) {
	if (x < 0 || y < 0 || x >= image.width || y >= image.height)
		return;

	image.data[y][x * 3 + 0] = r;
	image.data[y][x * 3 + 1] = g;
	image.data[y][x * 3 + 2] = b;
}


void get_pixel(RenderImage image, int x, int y, char* r, char* g, char* b) {
	*r = image.data[y][x * 3 + 0];
	*g = image.data[y][x * 3 + 1];
	*b = image.data[y][x * 3 + 2];
}
