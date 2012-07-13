#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* TODO: Our code shouldn't use the OpenGL Utility library as OpenGLES does not support it. */
#if defined(__APPLE__) || defined(MACOSX)
# include <OpenGL/gl.h>
# include <OpenGL/glu.h>
#else
# include <GL/gl.h>
# include <GL/glu.h>
#endif

#if USE_GTK == 1
# include <gtk/gtk.h>
# include <gtk/gtkgl.h>
#else
# include <GLUT/glut.h>
#endif

/** Removes unused variable warnings. */
#define UNUSED(_var) ((void)(_var))

/** Returns the number of elements in an array. */
#define NELEMS(_arr) (sizeof((_arr))/sizeof((_arr)[0]))

/** Default window width */
#define DEFAULT_WIN_WIDTH (800)
/** Default window height */
#define DEFAULT_WIN_HEIGHT (600)
/** The rotation speed */
#define ROTATION_SPEED_MS (1000/60)

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

typedef struct model_t {
	dude_t *b;
	pos_t b_pos;

	dude_t *b2;
	pos_t b2_pos;

	dude_t *gir;
	pos_t gir_pos;

	dude_t *test;
	pos_t test_pos;
} model_t;


static void destroyDude( dude_t *d )
{
	if(d != NULL) {
		free(d->dude_item.blocks);
		free(d);
	}
}

static dude_t *createDude(const pos_t dude[], size_t size) {

	dude_t *d = (dude_t *)calloc(1, sizeof(dude_t));
	if(d == NULL) {
		return NULL;
	}
	d->dude_item.blocks = malloc(size);
	if(d->dude_item.blocks == NULL) {
		free(d);
		return NULL;
	}
	d->dude_item.count = size / sizeof(pos_t);
	memcpy(d->dude_item.blocks, dude, size);

	return d;
}

#include "data/b.cotton"
#include "data/b2.cotton"
#include "data/gir.cotton"
#include "data/test.cotton"

static void destroyModel( model_t *m )
{
	if(m != NULL) {
		destroyDude(m->b);
		destroyDude(m->b2);
		destroyDude(m->gir);
		destroyDude(m->test);
		free(m);
	}
}

static model_t *createModel(void) {
	model_t *m = malloc( sizeof( model_t ) );

	if(m != NULL) {
		m->b = createDude(dude_b, sizeof(dude_b));
		m->b_pos.x = 0.0f; m->b_pos.y = 0.0f; m->b_pos.z = 0.0f;

		m->b2 = createDude(dude_b2, sizeof(dude_b2));
		m->b2_pos.x = 0.0f; m->b2_pos.y = 10.0f; m->b2_pos.z = 0.0f;

		m->gir = createDude(dude_gir, sizeof(dude_gir));
		m->gir_pos.x = 0.0f; m->gir_pos.y = 0.0f; m->gir_pos.z = 10.0f;

		m->test = createDude(dude_test, sizeof(dude_test));
		m->test_pos.x = 10.0f; m->test_pos.y = 0.0f; m->test_pos.z = 0.0f;

		if((m->b == NULL) || (m->b2 == NULL) ||
		   (m->gir == NULL) || (m->test == NULL)) {
			destroyModel(m);
			m = NULL;
		}
	}

	return m;
}

static model_t *model;
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

	static GLubyte indices[] = {
		0, 1, 2, 0, 2, 3, /* FRONT */
		7, 6, 5, 7, 5, 4, /* BACK */
		3, 2, 6, 3, 6, 7, /* RIGHT */
		4, 5, 1, 4, 1, 0, /* LEFT */
		1, 5, 6, 1, 6, 2, /* TOP */
		4, 0, 3, 4, 3, 7  /* BOTTOM */
	};

	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glDrawElements(GL_TRIANGLES, NELEMS(indices), GL_UNSIGNED_BYTE, indices);
}

static void drawDude(dude_t *d)
{
	unsigned int i, j;

	/* TODO:
	   figure out a better way of doing this color setting.
	   I think that using GLubyte rather than the GLfloat is fine for the colors?
	*/
	GLubyte colors[8*4];
	for(i=0;i<d->dude_item.count;++i) {
		for(j=0;j<8;j++) {
			memcpy(colors+(j * 4), &(d->dude_item.blocks[i].c), 4);
		}

		glPushMatrix();
		glTranslatef(d->dude_item.blocks[i].x, d->dude_item.blocks[i].y, d->dude_item.blocks[i].z);
		glColorPointer(4, GL_UNSIGNED_BYTE, 0 , colors);
		drawCube();
		glPopMatrix();
	}
}

static void drawModel(model_t *m)
{
	glPushMatrix();
	glTranslatef(m->b_pos.x, m->b_pos.y, m->b_pos.z);
	drawDude(m->b);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(m->b2_pos.x, m->b2_pos.y, m->b2_pos.z);
	drawDude(m->b2);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(m->gir_pos.x, m->gir_pos.y, m->gir_pos.z);
	drawDude(m->gir);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(m->test_pos.x, m->test_pos.y, m->test_pos.z);
	drawDude(m->test);
	glPopMatrix();
}

static void display(void)
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef( 0, 0, -5 );

	glRotatef(angle, 1.0, 0.0, 0.0);
	glRotatef(angle, 0.0, 1.0, 0.0);
	glRotatef(angle, 0.0, 0.0, 0.0);

	glScalef(0.25,0.25,0.25);

	drawModel(model);
	glFlush();
}

static void make_frustrum( double fovy, double aspect_ratio, double front, double back )
{
	const double DEG2RAD = 3.14159265 / 180;
	double tangent = tan(fovy/2 * DEG2RAD);
	double height = front * tangent;
	double width = height * aspect_ratio;
	glFrustum( -width /* left */, width /* right */,
	           -height /* bottom */, height /* top */,
	           front /* near */, back /* far */ );
}

static void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	make_frustrum(60, w / (GLfloat)h, 1.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
}

static void rotate()
{
	angle += 1.0;

	/* If angle gets too large the float will not be able to represent
	   the increments and the rotation will stop. */
	if( angle > 360.0f ) {
		angle = 0.0f;
	}
}

#if USE_GTK == 0
static void rotate_callback(int value)
{
	UNUSED( value );
	rotate();
	glutTimerFunc(ROTATION_SPEED_MS, rotate_callback, 0 );
}

#endif

#if USE_GTK == 1
static gboolean rotate_callback(gpointer user_data)
{
	GtkWidget *drawing_area = GTK_WIDGET(user_data);

	/* g_print("rotate\n"); */
	rotate();

	gdk_window_invalidate_rect(drawing_area->window, &drawing_area->allocation, FALSE);
	gdk_window_process_updates(drawing_area->window, FALSE);
	return TRUE;
}

static gboolean expose(GtkWidget *drawing_area, GdkEventExpose *event, gpointer user_data)
{
	GdkGLContext *gl_ctx = gtk_widget_get_gl_context(drawing_area);
	GdkGLDrawable *gl_dbl = gtk_widget_get_gl_drawable(drawing_area);

	UNUSED( event );
	UNUSED( user_data );

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

	UNUSED( event );
	UNUSED( user_data );

	if (!gdk_gl_drawable_gl_begin(gl_dbl, gl_ctx)) {
		printf("Can't start drawable :(\n");
		exit(1);
	}

	reshape( drawing_area->allocation.width, drawing_area->allocation.height );

	gdk_gl_drawable_gl_end(gl_dbl);

	return TRUE;
}
#endif

int main(int argc, char * argv[]) {
#if USE_GTK == 1
	GtkWidget *window;
	GtkWidget *drawing_area;
	GdkGLConfig *gl_config;

	gtk_init(&argc, &argv);
	gtk_gl_init(&argc, &argv);
#else
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_DEPTH);
#endif
	model = createModel();

#if USE_GTK == 1
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW (window), DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT);
	drawing_area = gtk_drawing_area_new();
	gtk_container_add( GTK_CONTAINER( window ), drawing_area );
	g_signal_connect_swapped (window, "destroy",
	                          G_CALLBACK (gtk_main_quit), NULL);
	gtk_widget_set_events(drawing_area, GDK_EXPOSURE_MASK);

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

	g_signal_connect(drawing_area, "configure-event",
                         G_CALLBACK(configure), NULL);
	g_signal_connect(drawing_area, "expose-event",
                         G_CALLBACK(expose), NULL);
	gtk_widget_show_all(window);

	g_timeout_add(ROTATION_SPEED_MS, rotate_callback, drawing_area);
#else
	glutInitWindowSize(DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("loads of rotating dudes");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(display);
	glutTimerFunc(ROTATION_SPEED_MS, rotate_callback, 0 );
#endif

	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CW);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

#if USE_GTK == 1
	gtk_main();
#else
	glutMainLoop();
#endif
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	destroyModel(model);
	return 0;
}
