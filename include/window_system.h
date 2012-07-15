#if !defined(WINDOW_SYSTEM_H)
#define WINDOW_SYSTEM_H


/** Default window width */
#define DEFAULT_WIN_WIDTH (800)

/** Default window height */
#define DEFAULT_WIN_HEIGHT (600)

/*
 * Called to setup the window system
 * Pass it argc and argv from main
 */
int window_system_initialise(int argc, char ***argv);

/*
 * Thin wrapper to call the main loop of the requisite window manager
 */
int window_system_start_main_loop(void);

/*
 * Called when the window manager is finished
 */
int window_system_cleanup(void);

/*
 * Setup the redraw/expose callback
 */
int window_system_set_redraw_callback(void (*redraw_callback)(void));

/*
 * Setup the resize callback
 */
int window_system_set_resize_callback(void (*resize_callback)(int,int));

/*
 * Setup the rotate callback
 * This may need to be more generic but is a WiP
 */
int window_system_set_rotate_callback(int rotation_speed, void (*rotation_callback)(void));

#endif

