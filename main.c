#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>


#if defined(__APPLE__) || defined(MACOSX)
# include <OpenGL/gl.h>
# include <GLUT/glut.h>
#elif defined(USE_GTK)
# include <gtk/gtk.h>
# include <gtk/gtkgl.h>
# include <GL/gl.h>
#else
# include <GL/gl.h>
# include <GL/glut.h>
#endif


/* rotation value */
static GLfloat angle = 0.0;

typedef struct col_t {
  GLubyte red, green, blue, alpha;
} col_t;

typedef struct pos_t {
	GLfloat x, y, z;
  col_t c;
} pos_t;

typedef struct item_t {
	unsigned int count;
	pos_t *blocks;
} item_t;

typedef struct dude_t {
	item_t dude_item;
} dude_t;

static dude_t *createDude(void) {
	/*
	  z = 0.0
	                 +----+
	                 | 14 |
	              +--+----+--+
	              | 12  | 13 |
	              +--+----+--+
	                 | 11 |
	       +----+----+----+----+----+
	  +----| 6  | 5  | 10 | 19 | 21 |----+
	  | 7  |----+----+----+----+----| 22 |
	  +----+    | 4  | 9  | 18 |    +----+
	            +----+----+----+
	            | 3  | 8  | 17 |
	            +----+----+----+
	            | 2  |    | 16 |
	            +----+    +----+
	            | 1  |    | 15 |
	            +----+    +----+

	   z = 1.0
	                 +----+
	                 | 14 |
	                 +----+









	            +----+    +----+
	            | 0  |    | 20 |
	            +----+    +----+

	 */

#if 0
	const pos_t b[] = {
		{-1, 0, 1},
		{-1, 0, 0},
		{-1, 1.0, 0},
		{-1, 2.0, 0},
		{-1, 3.0, 0},
		{-1, 4.0, 0}, /* 5 */
		{-2, 4.0, 0},
		{-3, 3.5, 0},
		{0, 2.0, 0.0},
		{0, 3.0, 0.0},
		{0, 4.0, 0.0},
		{0, 5.0, 0.0}, /* 11 */
		{-0.5, 6.0, 0.0},
		{0.5, 6.0, 0.0},
		{0, 6.0, 1.0},
		{0, 7.0, 0.0},
		{1, 0, 0.0},
		{1, 1.0, 0.0},
		{1, 2.0, 0.0},
		{1, 3.0, 0.0},
		{1, 4.0, 0.0}, /* 19 */
		{1, 0, 1},
		{2, 4.0, 0},
		{3, 3.5, 0}
	};
#else
	#include "data/gir.cotton"
#endif

	dude_t *d = (dude_t *)calloc(1, sizeof(dude_t));
	if(d == NULL) {
		return NULL;
	}
	d->dude_item.blocks = (pos_t *)calloc(sizeof(b)/sizeof(b[0]), sizeof(pos_t));
	if(d->dude_item.blocks == NULL) {
		free(d);
		return NULL;
	}
	d->dude_item.count = sizeof(b) / sizeof(b[0]);

	memcpy(d->dude_item.blocks, b, sizeof(b));
	return d;
}

static dude_t *dude;
#define M -0.5
#define P 0.5

static void drawCube(void)
{
	GLfloat vertices[24] = {
		M, M, M,
		M, P, M,
		P, P, M,
		P, M, M,
		M, M, P,
		M, P, P,
		P, P, P,
		P, M, P
	};

	/*
	static GLfloat colors[] = {0.0f, 0.0f, 0.0f, 1.0f,
	                           1.0f, 0.0f, 0.0f, 1.0f,
	                           1.0f, 1.0f, 0.0f, 1.0f,
	                           0.0f, 1.0f, 0.0f, 1.0f,
	                           0.0f, 0.0f, 1.0f, 1.0f,
	                           1.0f, 0.0f, 1.0f, 1.0f,
	                           1.0f, 1.0f, 1.0f, 1.0f,
	                           0.0f, 1.0f, 1.0f, 1.0f };*/

	static GLubyte indices[] = {
		0, 1, 2, 0, 2, 3, /* FRONT */
		7, 6, 5, 7, 5, 4, /* BACK */
		3, 2, 6, 3, 6, 7, /* RIGHT */
		4, 5, 1, 4, 1, 0, /* LEFT */
		1, 5, 6, 1, 6, 2, /* TOP */
		4, 0, 3, 4, 3, 7  /* BOTTOM */
	};

#if 0
	unsigned int i, j, k;

	for( k = 0; k < 10; ++k )
	{
		glTranslatef( 0.0, 0.0, (P-M)+P );
		glPushMatrix();
		for( j = 0; j < 10; ++j )
		{
			glTranslatef( 0.0, (P-M)+P, 0.0 );
			glPushMatrix();
			for( i = 0; i < 10; ++i )
			{
				glTranslatef( (P-M)+P, 0.0, 0.0 );
				glColorPointer(4, GL_FLOAT, 0 , colors);
				glVertexPointer(3, GL_FLOAT, 0, vertices);
				glDrawElements(GL_TRIANGLES, sizeof( indices ) / sizeof( indices[0] ), GL_UNSIGNED_BYTE, indices);
			}
			glPopMatrix();
		}
		glPopMatrix();
	}
#else
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glDrawElements(GL_TRIANGLES, sizeof( indices ) / sizeof( indices[0] ), GL_UNSIGNED_BYTE, indices);
#endif
}

static void drawDude(dude_t *d) {
  unsigned int i, j;
  for(i=0;i<d->dude_item.count;++i) {
    /* TODO:
       figure out a better way of doing this color setting.
       i think that using GLubyte rather than the GLfloat is fine for the colors?
     */
    GLubyte *colors = (GLubyte *)calloc(32, sizeof(GLubyte));
    if(colors == NULL) {
      fprintf(stderr, "Error: calloc colors\n");
      exit(-1);
    }
    for(j=0;j<8;j++) {
      memcpy(colors+(j * 4), &(d->dude_item.blocks[i].c), 4);
    }
		glPushMatrix();
		glTranslatef(d->dude_item.blocks[i].x, d->dude_item.blocks[i].y, d->dude_item.blocks[i].z);
		glColorPointer(4, GL_UNSIGNED_BYTE, 0 , colors);
		drawCube();
		glPopMatrix();
		free(colors);
	}
}



static void display (void)
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
		
#if defined(USE_GTK)
# warning TODO
#else
	gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
#endif
	glRotatef(angle, 1.0, 0.0, 0.0);
	glRotatef(angle, 0.0, 1.0, 0.0);
	glRotatef(angle, 0.0, 0.0, 0.0);

	glScalef(0.25,0.25,0.25);

	{
		unsigned int i;
		glTranslatef(-8*50, 0.0, 0.0); /* set reference point */
		for(i=0;i<100;i++) {
			glTranslatef(8, 0.0, 0.0); /* set reference point */
			drawDude(dude);
		}
	}
	glFlush();

	{
		unsigned long long ms;
		static unsigned long long oldms = 0;
		struct timeval t;
		gettimeofday( &t, NULL );
		ms = t.tv_sec * 1000 + (t.tv_usec / 1000);
		if( oldms + 10 < ms )
		{
			angle += 1; /* update the angle of rotation */
			oldms = ms;
		}
	}
}

void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
#if !defined(USE_GTK)
	gluPerspective(60, (GLfloat)w / (GLfloat)h, 1.0, 100.0);
#endif
	glMatrixMode(GL_MODELVIEW);
}

#if defined(USE_GTK)
static gboolean rotate(gpointer user_data)
{
	GtkWidget *drawing_area = GTK_WIDGET(user_data);

	/* Could tweak angle from here? */
	/* g_print("rotate\n"); */
	gdk_window_invalidate_rect(drawing_area->window, &drawing_area->allocation, FALSE);
	gdk_window_process_updates(drawing_area->window, FALSE);
	return TRUE;
}

static gboolean expose(GtkWidget *drawing_area, GdkEventExpose *event, gpointer user_data)
{
	GdkGLContext *gl_ctx = gtk_widget_get_gl_context(drawing_area);
	GdkGLDrawable *gl_dbl = gtk_widget_get_gl_drawable(drawing_area);

	if (!gdk_gl_drawable_gl_begin(gl_dbl, gl_ctx)) {
		printf("Can't start drawable :(\n");
		exit(1);
	}


	/* Do drawing stuff */
	/* g_print("expose\n"); */
	display();
	
	/* Finish up */
	if (gdk_gl_drawable_is_double_buffered(gl_dbl))
		gdk_gl_drawable_swap_buffers(gl_dbl);
	else
		glFlush();

	gdk_gl_drawable_gl_end(gl_dbl);

	return TRUE;
}

/* This is the gtk way of the window changing size */	
static gboolean configure(GtkWidget *drawing_area, GdkEventConfigure *event,
                          gpointer user_data)
{
	GdkGLContext *gl_ctx = gtk_widget_get_gl_context(drawing_area);
	GdkGLDrawable *gl_dbl = gtk_widget_get_gl_drawable(drawing_area);

	if (!gdk_gl_drawable_gl_begin(gl_dbl, gl_ctx)) {
		printf("Can't start drawable :(\n");
		exit(1);
	}

	/* Get viewport size */
	glLoadIdentity();
	glViewport(0,0, drawing_area->allocation.width, drawing_area->allocation.height);

	/* I'm a little confused as gluPerspective does a glFrustrum based on a glOrtho
         * so I might be messing things up a bit here */		
	glOrtho(-10,10,-10,10,-20050,10000);
	glEnable(GL_BLEND);

	glMatrixMode(GL_PROJECTION);
	glMatrixMode(GL_MODELVIEW);
	
	gdk_gl_drawable_gl_end(gl_dbl);	

	return TRUE;
}
#endif

int main(int argc, char * argv[]) {
#if defined(USE_GTK)
	GtkWidget *window;
	GtkWidget *drawing_area;
	GdkGLConfig *gl_config;

	gtk_init(&argc, &argv);
#else
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_DEPTH);
#endif
	dude = createDude();


#if defined(USE_GTK)
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);
	drawing_area = gtk_drawing_area_new();

	gtk_container_add(GTK_CONTAINER(window), drawing_area);
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	gtk_widget_set_events(drawing_area, GDK_EXPOSURE_MASK);
	gtk_widget_show(window);

	gl_config = gdk_gl_config_new_by_mode(GDK_GL_MODE_RGB |
					      GDK_GL_MODE_DEPTH |
					      GDK_GL_MODE_DOUBLE);

	if (!gl_config) {
		printf("Messed up the config :(\n");
		exit(1);
	}
	
	if (!gtk_widget_set_gl_capability(drawing_area, gl_config, NULL, TRUE,
                                          GDK_GL_RGBA_TYPE)) {
		printf("Couldn't get capabilities we needed :(\n");
		exit(1);
	}
#else
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("loads of rotating dudes");
#endif

	glEnable( GL_DEPTH_TEST );
	glFrontFace(GL_CW);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

#if defined(USE_GTK)
	g_signal_connect(drawing_area, "configure-event",
                         G_CALLBACK(configure), NULL);
	g_signal_connect(drawing_area, "expose-event",
                         G_CALLBACK(expose), NULL);
	gtk_widget_show_all(window);

	g_timeout_add(1000/60, rotate, drawing_area);
	gtk_main();

#else
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(display);
	glutMainLoop();
#endif
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	free(dude);
	return 0;
}
