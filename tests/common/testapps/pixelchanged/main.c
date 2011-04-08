/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (directui@nokia.com)
**
** This file is part of applauncherd
**
** If you have questions regarding the use of this file, please contact
** Nokia at directui@nokia.com.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/

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

struct timeval g_prev_stamp;
char g_output_filename[1024];

void timestamp(char *msg)
{
    struct timeval tim;
    FILE *output_file;
    char txtbuffer[80];

    gettimeofday(&tim, NULL);

    snprintf(txtbuffer, 80, "%d%03d ms %s\n", 
           (int)tim.tv_sec, (int)tim.tv_usec/1000, 
           msg);

    if (g_output_filename[0] == '\0') {
        printf("%s", txtbuffer);
    } else {
        if ((output_file = fopen(g_output_filename, "a+"))) {
            fprintf(output_file, "%s", txtbuffer);
            fclose(output_file);
        } else {
            fprintf(stderr, "pixelchanged: cannot open file '%s' for appending\n", g_output_filename);
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

XDevice* XPointerDevice = NULL;

/*!
 * Simulates user input event.
 *
 * \param dpy[in]     the display connection.
 * \param event[in]   the event to simulate.
 */
void scheduler_fake_event(Display* dpy, int event_type, int event_param1, int event_param2) 
{
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

//! 'Fakes' a mouse click, returning time sent.
void fake_event(Display *dpy, int x, int y)
{
  if (XPointerDevice) {
      scheduler_fake_event(dpy, SCHEDULER_EVENT_MOTION, x, y);
      scheduler_fake_event(dpy, SCHEDULER_EVENT_BUTTON, Button1, True);
      XSync(dpy, False);
      usleep(200000);
      scheduler_fake_event(dpy, SCHEDULER_EVENT_BUTTON, Button1, False);
      timestamp("Button1 released");
  }
}

//! This code is from "wmctrl" by Tomas Styblo <tripie@cpan.org>
static int client_msg(Display *disp, Window win, char *msg, /* {{{ */
        unsigned long data0, unsigned long data1, 
        unsigned long data2, unsigned long data3,
        unsigned long data4) {
        
    XEvent event;
    long mask = SubstructureRedirectMask | SubstructureNotifyMask;

    event.xclient.type = ClientMessage;
    event.xclient.serial = 0;
    event.xclient.send_event = True;
    event.xclient.message_type = XInternAtom(disp, msg, False);
    event.xclient.window = win;
    event.xclient.format = 32;
    event.xclient.data.l[0] = data0;
    event.xclient.data.l[1] = data1;
    event.xclient.data.l[2] = data2;
    event.xclient.data.l[3] = data3;
    event.xclient.data.l[4] = data4;
    
    if (XSendEvent(disp, DefaultRootWindow(disp), False, mask, &event)) {
        return EXIT_SUCCESS;
    }
    else {
        fprintf(stderr, "Cannot send %s event.\n", msg);
        return EXIT_FAILURE;
    }
}/*}}}*/

//! Raises the given window of given display.
void raise_window(Display *dpy, Window win)
{
    client_msg(dpy, win, "_NET_ACTIVE_WINDOW", 0, 0, 0, 0, 0);
    XSync(dpy, False);
}

//! Main function.
int main(int argc, char **argv) 
{
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
    int pixel_x = 423;
    int pixel_y = 240;
    int pixel2_x = -1;
    int pixel2_y = -1;
    int quit_when_found = 0;
    int window_id = 0;
    int count = 0;
    int i = 0;

    int arg = 1;

    g_output_filename[0] = '\0';

    DISPLAY = (char*)getenv("DISPLAY");
    if (DISPLAY == NULL) {
        printf("Cannot open display. DISPLAY variable not set.\n");
        exit(1);
    }

    dpy = XOpenDisplay(DISPLAY);
    screen = XDefaultScreen(dpy);
    rootw = RootWindow(dpy, screen);

    i = 0;

    while (arg < argc) {
        if (0 == strcmp("-c", argv[arg])) {
            sscanf(argv[++arg], "%ux%u", &click_x, &click_y);
        } else if (0 == strcmp("-p", argv[arg])) {
            sscanf(argv[++arg], "%lx", &pixel_value);
            pixel_value_defined = True;
        } else if (0 == strcmp("-r", argv[arg])) {
            sscanf(argv[++arg], "%x", &window_id);
            raise_window(dpy, window_id);
            return EXIT_SUCCESS;
        } else if (0 == strcmp("-t", argv[arg])) { /* tracked pixel coordinates */
            if (i == 0) {
                sscanf(argv[++arg], "%ux%u", &pixel_x, &pixel_y);
            } else {
                sscanf(argv[++arg], "%ux%u", &pixel2_x, &pixel2_y);
            }
            i++;
        } else if (0 == strcmp("-f", argv[arg])) {
            sscanf(argv[++arg], "%s", g_output_filename);
        } else if (0 == strcmp("-q", argv[arg])) {
            quit_when_found = 1;
        } else {
            fprintf(stderr, "Unrecognized option %s\n", argv[arg]);
            return EXIT_FAILURE;
        }
        ++arg;
    }

    
    // open input device required for XTestFakeDeviceXXX functions
    if (!(devInfo = XListInputDevices(dpy, &count)) || !count) {
        fprintf(stderr, "Cannot input list devices\n");
        return EXIT_FAILURE;
    }

    // find out where's the mouse
    for (i = 0; i < count; i++) {
        if ( devInfo[i].use == IsXExtensionPointer) {
            XPointerDevice = XOpenDevice(dpy, devInfo[i].id);
            break;
        }
    }
    
    XImage *image;
    unsigned long pixel = 0, pixel2 = 0;
    unsigned long previous_pixel = 0, previous_pixel2 = 0;

    image = XGetImage(dpy, rootw, pixel_x, pixel_y, 1, 1, AllPlanes, ZPixmap);
    previous_pixel = XGetPixel(image, 0, 0);

    if (pixel2_x != -1) {
        image = XGetImage(dpy, rootw, pixel2_x, pixel2_y, 1, 1, AllPlanes, ZPixmap);
        previous_pixel2 = XGetPixel(image, 0, 0);
    }

    if (click_x > -1) fake_event(dpy, click_x, click_y);

    while (1) {
        usleep(50000);

        image = XGetImage(dpy, rootw, pixel_x, pixel_y, 1, 1, AllPlanes, ZPixmap);
        pixel = XGetPixel(image, 0, 0);

        if (pixel2_x != -1) {
            image = XGetImage(dpy, rootw, pixel2_x, pixel2_y, 1, 1, AllPlanes, ZPixmap);
            pixel2 = XGetPixel(image, 0, 0);            
        }

        if ( 
            (!pixel_value_defined && pixel != previous_pixel) 
            || (pixel_value_defined && pixel == pixel_value) ) {
            snprintf(txtbuffer, 80, "pixel changed to value 0x%lx", pixel);
            timestamp(txtbuffer);
            previous_pixel = pixel;
            
            if (quit_when_found) return EXIT_SUCCESS;
        }

        if ( 
            pixel2_x != -1 &&
            ((!pixel_value_defined && pixel2 != previous_pixel2) 
                || (pixel_value_defined && pixel2 == pixel_value)) ) {
            snprintf(txtbuffer, 80, "pixel2 changed to value 0x%lx", pixel2);
            timestamp(txtbuffer);
            previous_pixel2 = pixel2;
            
            if (quit_when_found) return EXIT_SUCCESS;
        }

    }
    
    return EXIT_SUCCESS;
}

