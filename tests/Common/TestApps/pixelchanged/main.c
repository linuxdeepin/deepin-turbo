/*
*XGetImage(Display *display, Drawable d, int x, int y, unsigned
              int width, unsigned int height, unsigned long plane_mask, int
              format);
*/

#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xfuncs.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <X11/Xproto.h>
#include <X11/extensions/XTest.h>
#include <X11/extensions/record.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/time.h>

struct timeval prevstamp;
char output_filename[1024];

void timestamp(char *msg)
{
    struct timeval tim;
    FILE *output_file;
    char txtbuffer[80];

    gettimeofday(&tim, NULL);

    snprintf(txtbuffer, 80, "%d%03d ms %s\n", 
           (int)tim.tv_sec, (int)tim.tv_usec/1000, 
           msg);

    if (output_filename[0] == '\0') {
        printf("%s", txtbuffer);
    } else {
        if ((output_file = fopen(output_filename, "a+"))) {
            fprintf(output_file, "%s", txtbuffer);
            fclose(output_file);
        } else {
            fprintf(stderr, "pixelchanged: cannot open file '%s' for appending\n", output_filename);
            exit(2);
        }
    }
}

enum {
  SCHEDULER_EVENT_NONE = 0,
  SCHEDULER_EVENT_BUTTON,
  SCHEDULER_EVENT_KEY,
  SCHEDULER_EVENT_MOTION
};
XDevice* XPointerDevice       = NULL;
/**
 * Simulates user input event.
 *
 * @param dpy[in]     the display connection.
 * @param event[in]   the event to simulate.
 * @return
 */
void
scheduler_fake_event(Display* dpy, int event_type, int event_param1, int event_param2) {
  static int xPos = 0;
  static int yPos = 0;

  switch (event_type) {
    case SCHEDULER_EVENT_BUTTON: {
      int axis[2] = {xPos, yPos};
      XTestFakeDeviceButtonEvent(dpy, XPointerDevice, event_param1, event_param2, axis, 2, CurrentTime);
      break;
    }

    case SCHEDULER_EVENT_KEY:
      XTestFakeDeviceKeyEvent(dpy, XPointerDevice, event_param1, event_param2, NULL, 0, CurrentTime );
      break;

    case SCHEDULER_EVENT_MOTION:
      xPos = event_param1;
      yPos = event_param2;
    {
      int axis[2] = {xPos, yPos};
      XTestFakeDeviceMotionEvent(dpy, XPointerDevice, False, 0, axis, 2, CurrentTime);
      break;
    }
  }
}

/** 
 * 'Fakes' a mouse click, returning time sent.
 */
void
fake_event(Display *dpy, int x, int y)
{
  if (XPointerDevice) {
      scheduler_fake_event(dpy, SCHEDULER_EVENT_MOTION, x, y);
      scheduler_fake_event(dpy, SCHEDULER_EVENT_BUTTON, Button1, True);
      usleep(5000);
      scheduler_fake_event(dpy, SCHEDULER_EVENT_BUTTON, Button1, False);
      timestamp("Button1 released");
  }
}

int main(int argc, char **argv) {
    Display *dpy;
    int screen = 0;
    Window rootw;
    char *DISPLAY = NULL;
    XDeviceInfo *devInfo;
    char txtbuffer[80];

    int click_x = -1;
    int click_y = -1;
    unsigned long pixel_value = 0;
    int pixel_value_defined = False;
    int pixel_x = 423; /* track pixel in the middle of dali display by default */
    int pixel_y = 240;
    int quit_when_found = 0;

    int count = 0;
    int i = 0;

    int arg = 1;

    output_filename[0] = '\0';

    while (arg < argc) {
        if (0 == strcmp("-c", argv[arg])) {
            sscanf(argv[++arg], "%ux%u", &click_x, &click_y);
        } else if (0 == strcmp("-p", argv[arg])) {
            sscanf(argv[++arg], "%lx", &pixel_value);
            pixel_value_defined = True;
        } else if (0 == strcmp("-t", argv[arg])) { /* tracked pixel coordinates */
            sscanf(argv[++arg], "%ux%u", &pixel_x, &pixel_y);
        } else if (0 == strcmp("-f", argv[arg])) {
            sscanf(argv[++arg], "%s", output_filename);
        } else if (0 == strcmp("-q", argv[arg])) {
            quit_when_found = 1;
        }
        ++arg;
    }

    DISPLAY = (char*)getenv("DISPLAY");
    if (DISPLAY == NULL) {
        printf("Cannot open display. DISPLAY variable not set.\n");
        exit(1);
    }

    dpy = XOpenDisplay(DISPLAY);
    screen = XDefaultScreen(dpy);
    rootw = RootWindow(dpy, screen);

    /* find out where's the mouse */
    
    /* open input device required for XTestFakeDeviceXXX functions */
    if (!(devInfo = XListInputDevices(dpy, &count)) || !count) {
        fprintf(stderr, "Cannot input list devices\n");
        return 1;
    }

    for (i = 0; i < count; i++) {
        if ( devInfo[i].use == IsXExtensionPointer) {
            XPointerDevice = XOpenDevice(dpy, devInfo[i].id);
            break;
        }
    }

    
    XImage *image;
    unsigned long pixel = 0;
    unsigned long previous_pixel;

    image = XGetImage(dpy, rootw, pixel_x, pixel_y, 1, 1, AllPlanes, ZPixmap);
    previous_pixel = XGetPixel(image, 0, 0);

    if (click_x > -1)
        fake_event(dpy, click_x, click_y);

    while (1) {
        usleep(50000); /* sleep 50 ms */
        image = XGetImage(dpy, rootw, pixel_x, pixel_y, 1, 1, AllPlanes, ZPixmap);
        pixel = XGetPixel(image, 0, 0);
        if ( 
            (!pixel_value_defined && pixel != previous_pixel) 
            || (pixel_value_defined && pixel == pixel_value) ) {
            snprintf(txtbuffer, 80, "pixel changed to value 0x%lx", pixel);
            timestamp(txtbuffer);
            previous_pixel = pixel;
            if (quit_when_found) return 0;
        }
    }
}
