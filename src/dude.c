#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#if defined(__APPLE__) || defined(MACOSX)
# include <OpenGL/gl.h>
#else
# include <GL/gl.h>
#endif

#if !defined(USE_GTK)
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

	dude_t *doob;
	pos_t doob_pos;
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
#include "data/doob.cotton"

static void destroyModel( model_t *m )
{
	if(m != NULL) {
		destroyDude(m->b);
		destroyDude(m->b2);
		destroyDude(m->gir);
		destroyDude(m->test);
		destroyDude(m->doob);
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

		m->doob = createDude(dude_doob, sizeof(dude_doob));
		m->doob_pos.x = 0.0f; m->doob_pos.y = 0.0f; m->doob_pos.z = 20.0f;

		if((m->b == NULL) || (m->b2 == NULL) ||
		   (m->gir == NULL) || (m->test == NULL) ||
		   (m->doob == NULL)) {
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

	glPushMatrix();
	glTranslatef(m->doob_pos.x, m->doob_pos.y, m->doob_pos.z);
	drawDude(m->doob);
	glPopMatrix();
}

static void display(void)
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef( 0, -2, -7 );
	glRotatef(angle, 0.0, 1.0, 0.0);
	glScalef(0.25,0.25,0.25);
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
	glutPostRedisplay();
	glutTimerFunc(ROTATION_SPEED_MS, rotate_callback, 0 );
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
	window_system_initialise(argc, &argv);
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

        window_system_set_redraw_callback(display);
        window_system_set_resize_callback(reshape);
        window_system_set_rotate_callback(ROTATION_SPEED_MS, rotate);

#else
	glutInitWindowSize(DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("loads of rotating dudes");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutTimerFunc(ROTATION_SPEED_MS, rotate_callback, 0 );
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
	window_system_start_main_loop();
#else
	glutMainLoop();
#endif
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	destroyModel(model);

	return 0;
}
