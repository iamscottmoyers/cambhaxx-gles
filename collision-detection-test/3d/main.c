#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#if defined(__APPLE__) || defined(MACOSX)
# include <OpenGL/gl.h>
#else
# include <GL/gl.h>
#endif

#if USE_GTK == 1
# include <gtk/gtk.h>
# include <gtk/gtkgl.h>
#else
# if defined(__APPLE__) || defined(MACOSX)
#  include <GLUT/glut.h>
# else
#  include <GL/glut.h>
# endif
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

typedef struct bound_t {
  GLfloat max, min;
} bound_t;

typedef struct item_t {
	unsigned int count;
	pos_t *blocks;
  bound_t bounds[3];
} item_t;

typedef struct dude_t {
	item_t dude_item;
} dude_t;

typedef struct model_t {
  dude_t *fixed;
  pos_t fixed_pos;

  dude_t *moving;
  pos_t moving_pos;
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

	/* calculate bounds box */
	{
	  GLfloat x_min = d->dude_item.blocks[0].x;
	  GLfloat x_max = d->dude_item.blocks[0].x;
	  GLfloat y_min = d->dude_item.blocks[0].y;
	  GLfloat y_max = d->dude_item.blocks[0].y;
	  GLfloat z_min = d->dude_item.blocks[0].z;
	  GLfloat z_max = d->dude_item.blocks[0].z;
	  unsigned int i;
	  for(i=1;i<d->dude_item.count;++i) {
	    x_min = (d->dude_item.blocks[i].x < x_min) ? d->dude_item.blocks[i].x : x_min;
	    x_max = (d->dude_item.blocks[i].x > x_max) ? d->dude_item.blocks[i].x : x_max;
	    y_min = (d->dude_item.blocks[i].y < y_min) ? d->dude_item.blocks[i].y : y_min;
	    y_max = (d->dude_item.blocks[i].y > y_max) ? d->dude_item.blocks[i].y : y_max;
	    z_min = (d->dude_item.blocks[i].z < z_min) ? d->dude_item.blocks[i].z : z_min;
	    z_max = (d->dude_item.blocks[i].z > z_max) ? d->dude_item.blocks[i].z : z_max;
	  }
	  d->dude_item.bounds[0].min = x_min;
	  d->dude_item.bounds[0].max = x_max;
	  d->dude_item.bounds[1].min = y_min;
	  d->dude_item.bounds[1].max = y_max;
	  d->dude_item.bounds[2].min = z_min;
	  d->dude_item.bounds[2].max = z_max;
	}

	return d;
}

#include "data/b.cotton"
#include "data/b2.cotton"
#include "data/gir.cotton"
#include "data/test.cotton"
#include "data/doob.cotton"

static void destroyModel( model_t *m )
{
	if(m != NULL) {
		destroyDude(m->fixed);
		destroyDude(m->moving);
		free(m);
	}
}

static model_t *createModel(void) {
	model_t *m = malloc( sizeof( model_t ) );

	if(m != NULL) {
		m->fixed = createDude(dude_gir, sizeof(dude_gir));
		m->fixed_pos.x = 15.0f; m->fixed_pos.y = 0.0f; m->fixed_pos.z = 0.0f;

		m->moving = createDude(dude_doob, sizeof(dude_doob));
		m->moving_pos.x = 0.0f; m->moving_pos.y = 0.0f; m->moving_pos.z = 0.0f;

		if((m->fixed == NULL) || (m->moving == NULL)) {
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
	/* 3 vertices for every triangle in a cube. We need this amount as each vertex
	   needs a unique normal associated with it for lighting to work correctly. */
	const GLfloat vertices[] = {
		/* Front */
		M, M, P,
		P, M, P,
		P, P, P,

		M, M, P,
		P, P, P,
		M, P, P,

		/* Back */
		P, M, M,
		M, M, M,
		M, P, M,

		P, M, M,
		M, P, M,
		P, P, M,

		/* Right */
		P, M, P,
		P, M, M,
		P, P, M,

		P, M, P,
		P, P, M,
		P, P, P,

		/* Left */
		M, M, M,
		M, M, P,
		M, P, P,

		M, M, M,
		M, P, P,
		M, P, M,

		/* Top */
		M, P, P,
		P, P, P,
		P, P, M,

		M, P, P,
		P, P, M,
		M, P, M,

		/* Bottom */
		P, M, P,
		M, M, P,
		M, M, M,

		P, M, P,
		M, M, M,
		P, M, M
	};

	const GLfloat normals[] = {
		/* Front - All normals facing forwards */
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		/* Back - All normals facing backwards */
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,

		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,

		/* Right - All normals facing right */
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		/* Left - All normals facing left */
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,

		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,

		/* Top - All normals facing up */
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		/* Bottom - All normals facing down */
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,

		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f
	};

	const GLubyte indices[] = {
		0, 1, 2, 3, 4, 5,       /* FRONT */
		6, 7, 8, 9, 10, 11,     /* BACK */
		12, 13, 14, 15, 16, 17, /* RIGHT */
		18, 19, 20, 21, 22, 23, /* LEFT */
		24, 25, 26, 27, 28, 29, /* TOP */
		30, 31, 32, 33, 34, 35  /* BOTTOM */
	};

	glNormalPointer(GL_FLOAT, 0, normals);
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glDrawElements(GL_TRIANGLES, NELEMS(indices), GL_UNSIGNED_BYTE, indices);
}

/* i can't remember what we said about glBegin and glEnd? were we not supposed to use them?
   this function is only really used to display the collisions a bit easier
 */
static void drawCubeVariable(GLfloat x_min, GLfloat x_max, GLfloat y_min, GLfloat y_max, GLfloat z_min, GLfloat z_max)
{
  glBegin(GL_QUADS);
  glVertex3f(x_max,y_max,z_max);
  glVertex3f(x_min,y_max,z_max);
  glVertex3f(x_min,y_min,z_max);
  glVertex3f(x_max,y_min,z_max);
 
  glVertex3f(x_max,y_max,z_min);
  glVertex3f(x_min,y_max,z_min);
  glVertex3f(x_min,y_min,z_min);
  glVertex3f(x_max,y_min,z_min);
 
  glVertex3f(x_max,y_max,z_max);
  glVertex3f(x_max,y_min,z_max);
  glVertex3f(x_max,y_min,z_min);
  glVertex3f(x_max,y_max,z_min);
 
  glVertex3f(x_min,y_max,z_max);
  glVertex3f(x_min,y_min,z_max);
  glVertex3f(x_min,y_min,z_min);
  glVertex3f(x_min,y_max,z_min);
 
  glVertex3f(x_max,y_max,z_max);
  glVertex3f(x_min,y_max,z_max);
  glVertex3f(x_min,y_max,z_min);
  glVertex3f(x_max,y_max,z_min);
 
  glVertex3f(x_max,y_min,z_max);
  glVertex3f(x_min,y_min,z_max);
  glVertex3f(x_min,y_min,z_min);
  glVertex3f(x_max,y_min,z_min);
  glEnd();
}

static void drawDude(dude_t *d)
{
	unsigned int i, j;

	/* TODO:
	   figure out a better way of doing this color setting.
	   I think that using GLubyte rather than the GLfloat is fine for the colors?
	*/
	GLubyte colors[6*6*4];
	for(i=0;i<d->dude_item.count;++i) {
		for(j=0;j<6*6;j++) {
			memcpy(colors+(j * 4), &(d->dude_item.blocks[i].c), 4);
		}

		glPushMatrix();
		glTranslatef(d->dude_item.blocks[i].x, d->dude_item.blocks[i].y, d->dude_item.blocks[i].z);
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
		drawCube();
		glPopMatrix();
	}
}

static void drawModel(model_t *m)
{
	glPushMatrix();
	glTranslatef(m->fixed_pos.x, m->fixed_pos.y, m->fixed_pos.z);
	drawDude(m->fixed);
	/* draw a wireframe around m->fixed */
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); /* line mode */
	{
	  /* use bounds[] to create a wireframe around the object */
	  dude_t *d = m->fixed;
	  drawCubeVariable(d->dude_item.bounds[0].min-P,
			   d->dude_item.bounds[0].max+P,
			   d->dude_item.bounds[1].min-P,
			   d->dude_item.bounds[1].max+P,
			   d->dude_item.bounds[2].min-P,
			   d->dude_item.bounds[2].max+P);
	}
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); /* fill mode */
	glPopMatrix();


	glPushMatrix();
	glTranslatef(m->moving_pos.x, m->moving_pos.y, m->moving_pos.z);
	drawDude(m->moving);
	/* draw a wireframe around m->moving */
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); /* line mode */
	{
	  /* use bounds[] to create a wireframe around the object */
	  dude_t *d = m->moving;
	  drawCubeVariable(d->dude_item.bounds[0].min-P,
			   d->dude_item.bounds[0].max+P,
			   d->dude_item.bounds[1].min-P,
			   d->dude_item.bounds[1].max+P,
			   d->dude_item.bounds[2].min-P,
			   d->dude_item.bounds[2].max+P);
	}
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); /* fill mode */
	glPopMatrix();
}

static void display(void)
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef( -1, -1, -2);
	glRotatef(45.0, 0.0, 0.0, 0.0);
	glScalef(0.1,0.1,0.1);
	glTranslatef( 0, 0, -10 );

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
	angle += 0.5;

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
/* just move the model->moving object in the x plane */
static void move_dude_callback(int value) {
  static int collision = 0;
  UNUSED( value );
  if(!collision) {
    /* tried to make it a bit more interesting! */
    switch(rand() % 6) {
    case 0:
      model->moving_pos.x += 1.0;
      break;
    case 1:
      model->moving_pos.y += 1.0;
      break;
    case 2:
      model->moving_pos.z += 1.0;
      break;
    case 3:
      model->moving_pos.x += 1.0;
      break;
    case 4:
      model->moving_pos.y -= 1.0;
      break;
    case 5:
      model->moving_pos.z -= 1.0;
      break;
    default:
      break;
    }
    /* check for collisions */
    printf("move_dude\n");
    /* need to use bounds[] and the pos_t to figure out where in space objects are */
    /* loop through model->* if not self check for collision */
    /* compare model->moving with model->fixed */
    {      
      /* moving bounds, relative to object position */
      GLfloat m_x_min = model->moving_pos.x - (GLfloat)abs((int)model->moving->dude_item.bounds[0].min);
      GLfloat m_x_max = model->moving_pos.x + model->moving->dude_item.bounds[0].max;

      GLfloat m_y_min = model->moving_pos.y - (GLfloat)abs((int)model->moving->dude_item.bounds[1].min);
      GLfloat m_y_max = model->moving_pos.y + model->moving->dude_item.bounds[1].max;

      GLfloat m_z_min = model->moving_pos.z - (GLfloat)abs((int)model->moving->dude_item.bounds[2].min);
      GLfloat m_z_max = model->moving_pos.z + model->moving->dude_item.bounds[2].max;

      /* fixed bounds, relative to object position */
      GLfloat f_x_min = model->fixed_pos.x - (GLfloat)abs((int)model->fixed->dude_item.bounds[0].min);
      GLfloat f_x_max = model->fixed_pos.x + model->fixed->dude_item.bounds[0].max;

      GLfloat f_y_min = model->fixed_pos.y - (GLfloat)abs((int)model->fixed->dude_item.bounds[1].min);
      GLfloat f_y_max = model->fixed_pos.y + model->fixed->dude_item.bounds[1].max;

      GLfloat f_z_min = model->fixed_pos.z - (GLfloat)abs((int)model->fixed->dude_item.bounds[2].min);
      GLfloat f_z_max = model->fixed_pos.z + model->fixed->dude_item.bounds[2].max;

      printf("fixed:\t(%d, %d)\n\t(%d, %d)\n\t(%d, %d)\n",
	     (int)f_x_min, (int)f_x_max,
	     (int)f_y_min, (int)f_y_max,
	     (int)f_z_min, (int)f_z_max);
      printf("moving:\t(%d, %d)\n\t(%d, %d)\n\t(%d, %d)\n",
	     (int)m_x_min, (int)m_x_max,
	     (int)m_y_min, (int)m_y_max,
	     (int)m_z_min, (int)m_z_max);

      if(((m_x_max > f_x_min) && (m_x_min < f_x_max)) /* x */
	 &&
	 ((m_y_max > f_y_min) && (m_y_min < f_y_max)) /* y */
	 &&
	 ((m_z_max > f_z_min) && (m_z_min < f_z_max)) /* z */
	 ) {
	printf("collision!\n");
	collision = 1;
	/* this is a collision of th ebounding boxes
	   it would then require checking dude_item->pos_t* for collisions
	 */
      }
    }
    glutPostRedisplay();
    glutTimerFunc(1000/2, move_dude_callback, 0);
  }
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

static void light_0_enable(void)
{
	const GLfloat ambient[] = {0.6, 0.6, 0.6, 1.0};
	const GLfloat diffuse[] = {0.2, 0.2, 0.2, 1.0};
	const GLfloat specular[] = {0.0, 0.0, 0.0, 1.0};
	const GLfloat position[] = {0.0, 10.0, 10.0, 0.0};
	const GLfloat direction[] = {0.0, -1.0, -1.0};

	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, direction);
	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 45.0);
}

int main(int argc, char * argv[])
{
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
	if(model == NULL) {
		printf("Failed to create model :(\n");
		exit(1);
	}

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

	/* turned rotation off */
	/*g_timeout_add(ROTATION_SPEED_MS, rotate_callback, drawing_area);*/
#else
	glutInitWindowSize(DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Simple 3D Collision Detection");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	/* turned rotation off */
	/*glutTimerFunc(ROTATION_SPEED_MS, rotate_callback, 0 );*/
	/* move m->moving */
	srand(2);
	glutTimerFunc(1000/2, move_dude_callback, 0);
#endif

	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel(GL_SMOOTH);

	light_0_enable();

#if USE_GTK == 1
	gtk_main();
#else
	glutMainLoop();
#endif
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	destroyModel(model);

	return 0;
}
