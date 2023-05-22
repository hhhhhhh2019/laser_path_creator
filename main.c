#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include <image.h>


char* input_filename = NULL;
float scale = 1;
float speed = 1;
int x_offset = 0;
int y_offset = 0;

xmlDoc* xml_file;

RenderImage output;


void print_point(int, int);
void switch_laser();


void print_help(char*);
void parse_node(xmlNode*);
void parse_rect(xmlNode*);
void parse_circle(xmlNode*);
void parse_polygon(xmlNode*);
void parse_polyline(xmlNode*);
void parse_path(xmlNode*);

void line(float,float,float,float);


int main(int argc, char** argv) {
	// parsing args
	if (argc == 1)
		print_help(argv[0]);

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			print_help(argv[0]);
		} else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--scale") == 0) {
			scale = atof(argv[++i]);
		} else if (strcmp(argv[i], "-S") == 0 || strcmp(argv[i], "--speed") == 0) {
			speed = atof(argv[++i]);
		} else if (strcmp(argv[i], "-x") == 0 || strcmp(argv[i], "--xoffset") == 0) {
			x_offset = atof(argv[++i]);
		} else if (strcmp(argv[i], "-y") == 0 || strcmp(argv[i], "--yoffset") == 0) {
			y_offset = atof(argv[++i]);
		} else {
			input_filename = argv[i];
		}
	}

	if (input_filename == NULL)
		print_help(argv[0]);


	// parse xml

	xml_file = xmlReadFile(input_filename, NULL, 0);

	if (xml_file == NULL) {
		perror("xmlReadFile");
		exit(errno);
	}

	output = init_image(1001,1001);

	parse_node(xmlDocGetRootElement(xml_file));


	// free

	xmlFreeDoc(xml_file);

	save_output(output, "output.png");
	free_image(output);
}


void print_help(char* name) {
	printf("Usage: %s <filename> [-h] [-s --scale <float>] [-S --speed <float>] [-x --xoffset <int>] [-y --yoffset <int>]\n", name);
	exit(0);
}


void print_point(int x, int y) {
	//printf("1 %d %d ", x,y);
	set_pixel(output, x + x_offset, y + y_offset, 255,255,255);
}

void switch_laser() {
	//printf("2 ");
}


void line(float x1, float y1, float x2, float y2) {
	float dx = abs(x2 - x1);
	float sx = x1 < x2 ? 1 : -1;
	float dy = -abs(y2 - y1);
	float sy = y1 < y2 ? 1 : -1;

	float e = dx + dy;

	int i = 0;

	while (i++ < 1000) {
		print_point(x1,y1);

		if (abs(x1 - x2) < 0.1 && abs(y1 - y2) < 0.1)
			break;

		int e2 = e * 2;

		if (e2 >= dy) {
			if (x1 == x2)
				break;
			e += dy;
			x1 += sx;
		}

		if (e2 <= dx) {
			if (y1 == y2)
				break;
			e += dx;
			y1 += sy;
		}
	}
}


void parse_node(xmlNode* root) {
	xmlNode* node = root;

	for (; node; node = node->next) {
		if (node->type != XML_ELEMENT_NODE)
			continue;

		if (strcmp(node->name, "svg") == 0)
			asm("nop");
		else if (strcmp(node->name, "rect") == 0)
			parse_rect(node);
		else if (strcmp(node->name, "circle") == 0)
			parse_circle(node);
		else if (strcmp(node->name, "polygon") == 0)
			parse_polygon(node);
		else if (strcmp(node->name, "path") == 0)
			parse_path(node);
		else if (strcmp(node->name, "polyline") == 0)
			parse_polyline(node);
		/*else
			printf("Unknown tag: %s\n", node->name);*/

		parse_node(node->children);
	}
}


void parse_rect(xmlNode* node) {
	int width = 0;
	int height = 0;
	int x = 0;
	int y = 0;

	xmlAttr* prop = node->properties;

	for (; prop; prop = prop->next) {
		if (strcmp(prop->name, "x") == 0)
			x = atoi(prop->children->content) * scale;
		else if (strcmp(prop->name, "y") == 0)
			y = atoi(prop->children->content) * scale;
		else if (strcmp(prop->name, "width") == 0)
			width = atoi(prop->children->content) * scale;
		else if (strcmp(prop->name, "height") == 0)
			height = atoi(prop->children->content) * scale;
	}

	switch_laser();

	for (int i = x; i < x + width; i++) {
		print_point(i, y);
	}

	for (int i = y + 1; i < y + height; i++) {
		print_point(x + width - 1, i);
	}

	for (int i = x + width - 2; i >= x; i--) {
		print_point(i, y + height - 1);
	}

	for (int i = y + height - 2; i >= y; i--) {
		print_point(x, i);
	}

	switch_laser();
}


void parse_circle(xmlNode* node) {
	int cx = 0;
	int cy = 0;
	int r = 0;

	xmlAttr* prop = node->properties;

	for (; prop; prop = prop->next) {
		if (strcmp(prop->name, "cx") == 0)
			cx = atoi(prop->children->content) * scale;
		else if (strcmp(prop->name, "cy") == 0)
			cy = atoi(prop->children->content) * scale;
		else if (strcmp(prop->name, "r") == 0)
			r = atoi(prop->children->content) * scale;
	}

	int x,y,d;


	switch_laser();


	x = 0;
	y = r;
	d = 3 - 2 * r;

	while (y >= x) {
		x++;
		print_point(cx + x, cy + y);

		if (d > 0) {
			y--;
			d += 4 * (x - y) + 10;
		} else
			d += 4 * x + 6;
	}


	x = -1;
	y = r;
	d = 3 - 2 * r;

	while (y >= x) {
		x++;
		print_point(cx - x, cy + y);

		if (d > 0) {
			y--;
			d += 4 * (x - y) + 10;
		} else
			d += 4 * x + 6;
	}


	x = 0;
	y = r;
	d = 3 - 2 * r;

	while (y >= x) {
		x++;
		print_point(cx - y, cy + x);

		if (d > 0) {
			y--;
			d += 4 * (x - y) + 10;
		} else
			d += 4 * x + 6;
	}


	x = -1;
	y = r;
	d = 3 - 2 * r;

	while (y >= x) {
		x++;
		print_point(cx - y, cy - x);

		if (d > 0) {
			y--;
			d += 4 * (x - y) + 10;
		} else
			d += 4 * x + 6;
	}


	x = 0;
	y = r;
	d = 3 - 2 * r;

	while (y >= x) {
		x++;
		print_point(cx - x, cy - y);

		if (d > 0) {
			y--;
			d += 4 * (x - y) + 10;
		} else
			d += 4 * x + 6;
	}


	x = -1;
	y = r;
	d = 3 - 2 * r;

	while (y >= x) {
		x++;
		print_point(cx + x, cy - y);

		if (d > 0) {
			y--;
			d += 4 * (x - y) + 10;
		} else
			d += 4 * x + 6;
	}


	x = 0;
	y = r;
	d = 3 - 2 * r;

	while (y >= x) {
		x++;
		print_point(cx + y, cy - x);

		if (d > 0) {
			y--;
			d += 4 * (x - y) + 10;
		} else
			d += 4 * x + 6;
	}


	x = -1;
	y = r;
	d = 3 - 2 * r;

	while (y >= x) {
		x++;
		print_point(cx + y, cy + x);

		if (d > 0) {
			y--;
			d += 4 * (x - y) + 10;
		} else
			d += 4 * x + 6;
	}


	switch_laser();
}


void parse_polygon(xmlNode* node) {
	float* points = malloc(0);
	int points_count = 0;

	xmlAttr* prop = node->properties;

	for (; prop; prop = prop->next) {
		if (strcmp(prop->name, "points") == 0) {
			char* points_str = prop->children->content;
			char* str = malloc(1);
			str[0] = 0;

			while (1) {
				if (('0' <= *points_str && *points_str <= '9') || *points_str == '.' || *points_str == '-') {
					int len = strlen(str);
					str = realloc(str, len+2);
					str[len] = *points_str;
					str[len+1] = 0;
				} else {
					if (strlen(str) > 0) {
						points = realloc(points, (++points_count) * sizeof(float));
						points[points_count-1] = atof(str) * scale;

						free(str);
						str = malloc(1);
						str[0] = 0;
					}
				}

				if (*points_str == 0)
					break;

				points_str++;
			}

			free(str);
		}
	}

	for (int i = 0; i < points_count - 2; i+=2) {
		line(points[i + 0], points[i + 1], points[i + 2], points[i + 3]);
	}

	line(points[points_count - 2], points[points_count - 1], points[0], points[1]);


	switch_laser();
	switch_laser();

	free(points);
}


void parse_polyline(xmlNode* node) {
	float* points = malloc(0);
	int points_count = 0;

	xmlAttr* prop = node->properties;

	for (; prop; prop = prop->next) {
		if (strcmp(prop->name, "points") == 0) {
			char* points_str = prop->children->content;
			char* str = malloc(1);
			str[0] = 0;

			while (1) {
				if (('0' <= *points_str && *points_str <= '9') || *points_str == '.' || *points_str == '-') {
					int len = strlen(str);
					str = realloc(str, len+2);
					str[len] = *points_str;
					str[len+1] = 0;
				} else {
					if (strlen(str) > 0) {
						points = realloc(points, (++points_count) * sizeof(float));
						points[points_count-1] = atof(str) * scale;

						free(str);
						str = malloc(1);
						str[0] = 0;
					}
				}

				if (*points_str == 0)
					break;

				points_str++;
			}

			free(str);
		}
	}

	for (int i = 0; i < points_count-2; i+=2) {
		line(points[i + 0], points[i + 1], points[i + 2], points[i + 3]);
	}


	switch_laser();
	switch_laser();

	free(points);
}


void parse_path(xmlNode* node) {
	float* data = malloc(0);
	int data_size = 0;
	char* cmds = malloc(0);
	int cmds_count = 0;


	xmlAttr* prop = node->properties;

	for (; prop; prop = prop->next) {
		if (strcmp(prop->name, "d") != 0)
			continue;

		char* d = prop->children->content;
		char* str = malloc(1);
		str[0] = 0;

		char last_cmd = 0;

		while (1) {
			if (('0' <= *d && *d <= '9') || *d == '.' || *d == '-') {
				int len = strlen(str);
				str = realloc(str, len+2);
				str[len] = *d;
				str[len+1] = 0;
			} else {
				if (strlen(str) > 0) {
					data = realloc(data, (++data_size) * sizeof(float));
					data[data_size-1] = atof(str) * scale;

					free(str);
					str = malloc(1);
					str[0] = 0;

					cmds = realloc(cmds, (++cmds_count) * sizeof(char));
					cmds[cmds_count-1] = last_cmd;
				}

				if (*d != ' ' && *d != ',' && *d != 0) {
					if (*d == 'z' || *d == 'Z') {
						cmds = realloc(cmds, (++cmds_count) * sizeof(char));
						cmds[cmds_count-1] = *d;
					}
					last_cmd = *d;
				}
			}

			if (*d == 0)
				break;

			d++;
		}

		free(str);
	}


	int di = 0;
	int x = 0;
	int y = 0;
	int sx = 0;
	int sy = 0;

	char last_cmd = 0;

	for (int i = 0; i < cmds_count; i+=2) {
		if (cmds[i] == 'M' && last_cmd != 'M') {
			x = data[di++];
			y = data[di++];
			sx = x;
			sy = y;
		} else if (cmds[i] == 'm') {
			x += data[di++];
			y += data[di++];
			sx = x;
			sy = y;
		} else if (cmds[i] == 'L' || (cmds[i] == 'M' && last_cmd == 'M')) {
			int x1 = data[di++];
			int y1 = data[di++];
			line(x,y, x1,y1);
			x = x1;
			y = y1;
		} else if (cmds[i] == 'l') {
			int x1 = x + data[di++];
			int y1 = y + data[di++];
			line(x,y, x1,y1);
			x = x1;
			y = y1;
		} else if (cmds[i] == 'Z' || cmds[i] == 'z') {
			line(x,y, sx,sy);
			x = sx;
			y = sy;
		}

		last_cmd = cmds[i];
	}


	free(data);
	free(cmds);
}
