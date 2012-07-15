#include <stdlib.h>
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include "window_system.h"

static GtkWidget *drawing_area;
static GtkWidget *window;
static GdkGLConfig *gl_config;

int window_system_initialise(int argc, char ***argv)
{
	gtk_init(&argc, argv);
	gtk_gl_init(&argc, argv);
	
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW (window), DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT);
	drawing_area = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER(window), drawing_area);
	g_signal_connect_swapped(window, "destroy",
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
	
        return 0;
}

int window_system_cleanup()
{
        /* TODO: Clear it all up */
        return 0;
}

int window_system_start_main_loop()
{
	gtk_widget_show_all(window);
        gtk_main();
        return 0;
}

static void (*redraw_func)(void);
static gboolean redraw(GtkWidget *drawing_area, GdkEventExpose *event, 
                       gpointer user_data)
{
	GdkGLContext *gl_ctx = gtk_widget_get_gl_context(drawing_area);
	GdkGLDrawable *gl_dbl = gtk_widget_get_gl_drawable(drawing_area);

	if (!gdk_gl_drawable_gl_begin(gl_dbl, gl_ctx)) {
		printf("Can't start drawable :(\n");
		exit(1);
	}

	/* Do drawing stuff */
	
	if (redraw_func)
	        redraw_func();

	/* Finish up */
	if (gdk_gl_drawable_is_double_buffered(gl_dbl))
		gdk_gl_drawable_swap_buffers(gl_dbl);
	else
		glFlush();

	gdk_gl_drawable_gl_end(gl_dbl);

	return TRUE;
}

/* Takes a function to call, hooks it up and that's called */
int window_system_set_redraw_callback(void (*redraw_callback)(void))
{
        redraw_func = redraw_callback;
	g_signal_connect(drawing_area, "expose-event",
                         G_CALLBACK(redraw), NULL);

        return 0;
}

static void (*resize_func)(int,int);
static gboolean resize(GtkWidget *drawing_area, GdkEventConfigure *event,
                       gpointer user_data)
{
	GdkGLContext *gl_ctx = gtk_widget_get_gl_context(drawing_area);
	GdkGLDrawable *gl_dbl = gtk_widget_get_gl_drawable(drawing_area);

	if (!gdk_gl_drawable_gl_begin(gl_dbl, gl_ctx)) {
		printf("Can't start drawable :(\n");
		exit(1);
	}

        if (resize_func)
                resize_func(drawing_area->allocation.width, drawing_area->allocation.height);

	gdk_gl_drawable_gl_end(gl_dbl);
	return TRUE;
}

int window_system_set_resize_callback(void (*resize_callback)(int,int))
{
        resize_func = resize_callback;
	g_signal_connect(drawing_area, "configure-event",
                         G_CALLBACK(resize), NULL);
	gtk_widget_show_all(drawing_area);
        return 0;
}

static void (*rotate_func)(void);
static gboolean rotate(gpointer user_data)
{
	GtkWidget *drawing_area = GTK_WIDGET(user_data);

	if (rotate_func)
	        rotate_func();

	gdk_window_invalidate_rect(drawing_area->window, &drawing_area->allocation, FALSE);
	gdk_window_process_updates(drawing_area->window, FALSE);
	return TRUE;
}

int window_system_set_rotate_callback(int rotation_speed, void (*rotation_callback)(void))
{
        /* Setup the function pointer to call here */
        rotate_func = rotation_callback;
	g_timeout_add(rotation_speed, rotate, drawing_area);
        gtk_widget_show_all(window);
        return 0;
}


