#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>

int main(int argc, char *argv[]) {
	if (argc < 3) {
		fprintf(stderr, "usage: %s <pointer> <screen>\n", argv[0]);
		return 1;
	}

	int deviceid = atoi(argv[1]);
	int screen = atoi(argv[2]);
	char *display_name = getenv("DISPLAY");

	Display *display = XOpenDisplay(display_name);
	if (!display) {
		fprintf(stderr, "Unable to connect to X server %s\n", display_name);
		return 2;
	}

	if (deviceid == 0) {
		int device_count;
		XIDeviceInfo *devices = XIQueryDevice(display, XIAllDevices, &device_count);
		for (int i = 0; i < device_count; i++)
			if (devices[i].use == XIMasterPointer && !strcmp(devices[i].name, argv[1]))
				deviceid = devices[i].deviceid;
		XIFreeDeviceInfo(devices);
	}

	Window screen_root = RootWindow(display, screen);
	Bool ret = XIWarpPointer(display, deviceid, None, screen_root, 0, 0, 0, 0, 0, 0);
	if (ret) {
		fprintf(stderr, "XIWarpPointer failed\n");
		return 3;
	}

	XCloseDisplay(display);
	return 0;
}