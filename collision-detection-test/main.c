#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <assert.h>

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
/** The maximum voxel per second speed **/
#define MAX_MOVEMENT_SPEED 1

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

typedef struct dir_t {
	GLfloat xy; /* Angle between x and y planes */
	GLfloat xz; /* Angle between x and z planes */
} dir_t;

typedef struct anim_t {
	dir_t direction;
	GLfloat speed;
} anim_t;

typedef struct dude_t {
	item_t dude_item;
	pos_t position;
	anim_t animation;
} dude_t;

typedef struct dudes_t {
	dude_t* dude;
	struct dudes_t *next;
} dudes_t;

typedef struct model_t {
	dudes_t *dudes;
} model_t;

/* Simple Linked List to Add and Remove dudes_t */
dudes_t *addDude(dudes_t *l, dude_t *d)
{
	dudes_t *n = (dudes_t *)calloc(1, sizeof(dudes_t));
	n->dude = d;
	n->next = NULL;
	if(l == NULL) {
		return n;
	}
	else {
		dudes_t *u = l;
		while(u->next != NULL) {
			u = u->next;
		}
		u->next = n;
		return l;
	}
	return NULL;
}

dudes_t *removeDude(dudes_t *l, dude_t *d)
{
	if(l == NULL) {
		return NULL;
	}
	else {
		dudes_t *n = l;
		dudes_t *p = NULL;
		do {
			if(n->dude == d) {
				if(n != l) {
					p->next = n->next;
					free(n);
					return l;
				}
				else {
					l = n->next;
					free(n);
					return l;
				}
			}
			p = n;
			n = n->next;
		} while(n != NULL);
			
	}
	assert(0 && "Didn't find dude to remove from linked list...");
	return l;
}

static void destroyDude( dude_t *d )
{
	if(d != NULL) {
		free(d->dude_item.blocks);
		free(d);
	}
}

static dude_t *createDude(const pos_t dude[], size_t size, pos_t *position, anim_t *animation) {

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

	memcpy(&(d->position), position, sizeof(pos_t));
	memcpy(&(d->animation), animation, sizeof(anim_t));

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

#include "data/collision_fixed.cotton"
#include "data/collision_moving.cotton"

static void destroyModel( model_t *m )
{
	dudes_t *ds = m->dudes;
	while(ds) {
		dudes_t *dn = ds->next;
		destroyDude(ds->dude);
		ds = dn;
	}
	free(m);
}

static model_t *createModel(void) {
	model_t *m = calloc(1,  sizeof( model_t ) );
	if(m != NULL) {
		{
			pos_t pos = {0.0f, 0.0f, 0.0f, {255, 0, 0, 1}};
			anim_t anim = {{0.0f, 0.0f}, 1.0f};
			dude_t *d = createDude(collision_fixed, sizeof(collision_fixed), &pos, &anim);
			if(d != NULL) {
				m->dudes = addDude(m->dudes, d);
			}
			else {
				fprintf(stderr, "Error creating dude (fixed)\n");
			}
		}
		{
			pos_t pos = {-7.0f, 0.0f, 10.0f, {255, 0, 0, 1}};
			anim_t anim = {{0.0f, 0.0f}, 2.0f};
			dude_t *d = createDude(collision_moving, sizeof(collision_moving), &pos, &anim);
			if(d != NULL) {
				m->dudes = addDude(m->dudes, d);
			}
			else {
				fprintf(stderr, "Error creating dude (moving)\n");
			}
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
	/* draw each dude from dudes */
	dudes_t *ds = m->dudes;
	while(ds) {
		glPushMatrix();
		glTranslatef(ds->dude->position.x, ds->dude->position.y, ds->dude->position.z);
		drawDude(ds->dude);
		/* draw a wireframe around m->fixed */
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); /* line mode */
		{
			/* use bounds[] to create a wireframe around the object */
			drawCubeVariable(ds->dude->dude_item.bounds[0].min-P,
			                 ds->dude->dude_item.bounds[0].max+P,
			                 ds->dude->dude_item.bounds[1].min-P,
			                 ds->dude->dude_item.bounds[1].max+P,
			                 ds->dude->dude_item.bounds[2].min-P,
			                 ds->dude->dude_item.bounds[2].max+P);
		}
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); /* fill mode */
		glPopMatrix();

		ds = ds->next;
	}
}

/* Basic alteration of view of dudes
   I wanted to be able to move about in the model to check collisions
*/

/* Initial viewpoint */
static GLfloat _x = 1.0f;
static GLfloat _y = 2.0f;
static GLfloat _z = -16.0f;
static GLfloat _r = 45.0f;
static GLfloat _rx = 1.0f;
static GLfloat _ry = 0.0f;
static GLfloat _rz = 0.0f;

/* a + d: x plane
   w + s: z plane
   r + f: y plane
   i + k: rotation (x)
   j + l: rotation (y);
*/
static void keyboard(unsigned char key, int x, int y)
{
  UNUSED( x );
  UNUSED( y );
  _rx = _ry = _rz = 0.0f;
  switch(key)
    {
    case 'A':
    case 'a':
      _x += 1.0f;
      break;
    case 'D':
    case 'd':
      _x -= 1.0f;
      break;
    case 'W':
    case 'w':
      _z += 1.0f;
      break;
    case 'S':
    case 's':
      _z -= 1.0f;
      break;
    case 'R':
    case 'r':
      _y -= 1.0f;
      break;
    case 'F':
    case 'f':
      _y += 1.0f;
      break;
    case 'I':
    case 'i':
      _rx = 1.0f;
      _ry = _rz = 0.0f;
      _r += 45.0f;
      if(_r >= 360.0f) {
	_r = 0.0f;
      }
      break;
    case 'K':
    case 'k':
      _rx = 1.0f;
      _ry = _rz = 0.0f;
      _r -= 45.0f;
      if(_r < 0.0f) {
	_r = 360.f + _r;
      }
      break;
      break;
    case 'J':
    case 'j':
      _ry = 1.0f;
      _rx = _rz = 0.0f;
      _r += 45.0f;
      if(_r >= 360.0f) {
	_r = 0.0f;
      }
      break;
      break;
    case 'L':
    case 'l':
      _ry = 1.0f;
      _r-= 45.0f;
      _rx = _rz = 0.0f;
      if(_r < 0.0f) {
	_r = 360.0f + _r;
      }
      break;
    default:
      break;
    }
  /* Redraw the screen */
  glutPostRedisplay();
}

static void display(void)
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	/* Set inital view point and rotation */
	glTranslatef(_x, _y, _z);
	glRotatef(_r, _rx, _ry, _rz);

	/* Initialise glut keyboard input */
	glutKeyboardFunc(keyboard);
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

#define getMIN(dude, plane, bound) dude->position.plane - (GLfloat)abs((int)dude->dude_item.bounds[bound].min)
#define getMAX(dude, plane, bound) dude->position.plane + dude->dude_item.bounds[bound].max
#define DEBUG 1

static int check_collision(dude_t *a, dude_t *b)
{
	/* Use pos_t and bounds_t to calculate where in space dudes are */
	/* Get the x,y,z max and min bounds of dudes */
	GLfloat a_x_min = getMIN(a, x, 0);
	GLfloat a_x_max = getMAX(a, x, 0);

	GLfloat a_y_min = getMIN(a, y, 1);
	GLfloat a_y_max = getMAX(a, y, 1);

	GLfloat a_z_min = getMIN(a, z, 2);
	GLfloat a_z_max = getMAX(a, z, 2);

	GLfloat b_x_min = getMIN(b, x, 0);
	GLfloat b_x_max = getMAX(b, x, 0);

	GLfloat b_y_min = getMIN(b, y, 1);
	GLfloat b_y_max = getMAX(b, y, 1);

	GLfloat b_z_min = getMIN(b, z, 2);
	GLfloat b_z_max = getMAX(b, z, 2);

	/* Calculate collisions in all planes */
	unsigned char x_collision = (a_x_max >= b_x_min) && (a_x_min <= b_x_max);
	unsigned char y_collision = (a_y_max >= b_y_min) && (a_y_min <= b_y_max);
	unsigned char z_collision = (a_z_max >= b_z_min) && (a_z_min <= b_z_max);

#ifdef DEBUG
	printf("a:\tx:(%d, %d)\ty:(%d, %d)\tz:(%d, %d)\n",
	       (int)a_x_min, (int)a_x_max,
	       (int)a_y_min, (int)a_y_max,
	       (int)a_z_min, (int)a_z_max);
	printf("b:\tx:(%d, %d)\ty:(%d, %d)\tz:(%d, %d)\n",
	       (int)b_x_min, (int)b_x_max,
	       (int)b_y_min, (int)b_y_max,
	       (int)b_z_min, (int)b_z_max);

	printf("x:%d y:%d z:%d\n", x_collision, y_collision, z_collision);
#endif

	/* Collision only if all planes show collisions,
	   otherwise just in line
	*/
	if(x_collision && y_collision && z_collision) {
		printf("Bounding box collision\n");
		/* Check at finer granualrity */
#define OPTION2

#ifdef OPTION1
		/* Option 1: blanket check - compare all voxels in a and b */
		{
			pos_t *_a = NULL, *_b = NULL;
			unsigned int _aI = 0, _bI = 0;
			/* For each voxel in a */
			for(_a = &(a->dude_item.blocks[_aI++]); _aI <= a->dude_item.count; _a = &(a->dude_item.blocks[_aI++]) ) {

				/* For each voxel in b */
				for(_bI = 0, _b = &(b->dude_item.blocks[_bI++]); _bI <= b->dude_item.count; _b = &(b->dude_item.blocks[_bI++]) ) {
					
					/* Check for collisions */
					/* Current voxel plus the current dude position */
					if(((_a->x + a->position.x) == (_b->x + b->position.x))
					   &&
					   ((_a->y + a->position.y) == (_b->y + b->position.y))
					   &&
					   ((_a->z + a->position.z) == (_b->z + b->position.z))) {
						/* Collision */
						printf("Voxel collision\n");
						return 1;
					}
				}
			}
		}
#elif defined OPTION2
		/* Option 2: */
		{
			unsigned char *dude_a_collision = NULL;
			
			/* work out size of dude a */
			int x_size = (int)(a_x_max - a_x_min) + 1;
			int y_size = (int)(a_y_max - a_y_min) + 1;
			int z_size = (int)(a_z_max - a_z_min) + 1;
			
			pos_t *p = NULL;
			unsigned int i = 0;
			
			/* Allocate space */
			if((dude_a_collision = (unsigned char *)calloc((x_size * y_size * z_size), 1)) == NULL) {
				fprintf(stderr, "Unable to allocate memory: dude_a_collision\n");
				exit(-1);
			}
			/* dude_a_collision being used as [z][y][x] */
			
			/* Fill in map of voxels in dude a */
			/* This required normalising the positions relative to a point where all values
			   end up starting from zero. Using the min bounds for this.
			*/
			for(p = &(a->dude_item.blocks[i++]); i <= a->dude_item.count; p = &(a->dude_item.blocks[i++]) ) {
				dude_a_collision[(int)((p->x - a->dude_item.bounds[0].min) +
				                       ((p->y - a->dude_item.bounds[1].min) * x_size) +
				                       ((p->z - a->dude_item.bounds[2].min) * y_size * x_size))] = 1;
			}
			
			/* Check against dude b */
			/* Need to translate dude b voxels in relation to the point used in above step
			   (voxel offset + center point(b)) - (center point(a) + point of reference)
			*/
			
			for(i = 0, p = &(b->dude_item.blocks[i++]); i <= b->dude_item.count; p = &(b->dude_item.blocks[i++]) ) {
				int x = (int)((p->x + b->position.x) - (a->position.x + a->dude_item.bounds[0].min));
				int y = (int)((p->y + b->position.y) - (a->position.y + a->dude_item.bounds[1].min));
				int z = (int)((p->z + b->position.z) - (a->position.z + a->dude_item.bounds[2].min));
				if( ((x >= 0) && (x < x_size)) && ((y >= 0) && (y < y_size)) && ((z >= 0) && (z < z_size)) ) {
					if(dude_a_collision[(x + (y * x_size) + (z * y_size * x_size))]) {
						printf("Voxel collision\n");
						free(dude_a_collision);
						return 1;
					}
				}
			}
			free(dude_a_collision);
		}
#else
#error "You need to specify a collision check"
#endif
		return 0;
	}
	return 0;
}

/* Move each dude depending on their speed and direction attributes */
static void move_dude_callback(int value)
{
	/* Used to determine if a dude should move this iteration */
	static int timestep = 0;
	/* Only update the screen if any item has moved */
	int moved = 0;

	/* Loop through dudes */
	dudes_t *ds = model->dudes;

	UNUSED( value );

	/* After writing this i realised that it shouldn't quite work this way
	   Each dude should be moved, and then a collision check should be made
	   In the current implementation if two items are moving at the same speed in the same
	   direction 1 voxel apart, depeding on the order they are draw there would be different outcome
	 */

	/* TODO: figure out how to animate different speeds */
	while(ds) {
		if(ds->dude->animation.speed != 0.0f) {
			dudes_t *coll = NULL;

			/* Create a copy of the current position */
			pos_t curr;
			memcpy(&curr, &(ds->dude->position), sizeof(pos_t));

			/* Move in the direction of travel */
			/* TODO: workout direction of travel using animation.direction */
			/* TODO: workout smothly moving the items */
			ds->dude->position.z -= (2*P * ds->dude->animation.speed);

			/* Then check collisions with each other dude */
			coll = model->dudes;
			while(coll) {
				if(coll->dude != ds->dude) {
					if(check_collision(ds->dude, coll->dude)) {
						/* Can't move because of a collision */
						memcpy(&(ds->dude->position), &curr, sizeof(pos_t));
						/* Stop movement */
						ds->dude->animation.speed = 0.0f;
						break;
					}
					else {
						moved = 1;
					}
				}
				coll = coll->next;
			}
		}
		ds = ds->next;
	}

	if(++timestep >= MAX_MOVEMENT_SPEED) {
		timestep = 0;
	}
	if(moved) {
		glutPostRedisplay();
	}
	glutTimerFunc(1000/MAX_MOVEMENT_SPEED, move_dude_callback, 0);
#if 0
  static int collision = 0;
  pos_t before_move = model->moving_pos;
  UNUSED( value );
  if(!collision) {
#if 0
    /* tried to make it a bit more interesting! */
    switch(rand() % 6) {
    case 0:
      model->moving_pos.x += 2*P;
      break;
    case 1:
      model->moving_pos.y += 2*P;
      break;
    case 2:
      model->moving_pos.z += 2*P;
      break;
    case 3:
      model->moving_pos.x += 2*P;
      break;
    case 4:
      model->moving_pos.y -= 2*P;
      break;
    case 5:
      model->moving_pos.z -= 2*P;
      break;
    default:
      break;
    }
#else
    model->moving_pos.z -= 2*P;
#endif
    if(check_collision(&(model->moving_pos),
		       &(model->moving->dude_item),
		       &(model->fixed_pos),
		       &(model->fixed->dude_item))) {
      /* Copy previous position back to model
	 This will be dependent on which way the dudes are being checked
       */
      memcpy((void *)&(model->moving_pos), (void *)&(before_move), sizeof(pos_t));
      /*collision = 1;*/
      /* In this example move moving to another place to test */
      if((model->moving_pos.x + 3.0f) > 8.0f) {
	collision = 1;
      }
      else {
	model->moving_pos.y = 0.0f; model->moving_pos.z = 10.0f;
	model->moving_pos.x += 3.0f;
      }
    }
    glutPostRedisplay();
    glutTimerFunc(1000/MAX_MOVEMENT_SPEED, move_dude_callback, 0);
  }
#endif
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
	glutTimerFunc(1000/MAX_MOVEMENT_SPEED, move_dude_callback, 0);
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
