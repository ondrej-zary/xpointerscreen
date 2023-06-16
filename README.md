xpointerscreen
==============

Move a X11 pointer to another screen

This simple tool can move one of the (mouse) pointers (as supported by MPX -
Multi-Pointer X) to any of existing X11 screens. Useful when the screens are
not adjacent so there's no other way to move a pointer away from the primary
screen.

Compiling from source
---------------------
Requirements: make, gcc, libx11-dev, libxi-dev

To compile, simply run "make":

    $ make

Example
=======

A POS system with two displays, a big one facing the cashier and a small one
facing the customer.

The big internal display is 1920x1080 and the small external is 1280x800.
They're configured as separate X11 screens (using ZaphodHeads) so they can
run different applications that will not mix.
The external display is placed 2000 pixels to the right so there's a gap
between the displays. Mouse pointer will not cross this gap - we'll use two
separate pointers (see below).

Example /etc/X11/xorg.conf.d/zaphod.conf:

    Section "Device"
      Identifier    "Internal LCD"
      Driver        "modesetting"
      Option        "ZaphodHeads" "eDP-1"
      Screen        0
    EndSection

    Section "Device"
      Identifier    "Rear LCD"
      Driver        "modesetting"
      Option        "ZaphodHeads" "DP-1"
      Screen        1
    EndSection

    Section "Screen"
      Identifier    "Internal screen"
      Device        "Internal LCD"
    EndSection

    Section "Screen"
      Identifier    "Rear screen"
      Device        "Rear LCD"
    EndSection

    Section "ServerLayout"
      Identifier    "Double Layout"
      Screen        "Internal screen" 0 0
      Screen        "Rear screen" 2000 0
    EndSection

As both displays have touchscreens, they must first be configured to control
the correct part of the ServerLayout. By default, each touchscreen spans the
entire area of the layout (from the upper left corner of the primary display
to the lower right corner of the secondary one in our case). Setting the
"Coordinate Transformation Matrix" using xinput will do it.

Create a secondary (mouse) pointer and attach the small touchscreen device
to it. Other devices (the primary touschscreen and any mice connected) will
control the primary pointer.

But both pointers are still on the primary screen and there's no way to move
them to the secondary one. xdotool does not support XInput2 so it can only
move the primary pointer. Here comes the xpointerscreen tool which can move
any of the pointers to and X11 screen.
Now both touchscreens work, each using its own pointer.

Example /etc/X11/Xsession.d/80dualscreen:

    PRI_TOUCH="Elo Touch WLIDS 21.5"
    SEC_TOUCH="Elo Touch Solutions Elo Touch Solutions Pcap USB Interface"
    PRI_X_OFFSET=0
    PRI_Y_OFFSET=0
    SEC_X_OFFSET=2000
    SEC_Y_OFFSET=0

    PRI_RES=$(xrandr --screen 0| grep ^Screen | sed -e 's/^.*\(current \)\([[:digit:]]* x [[:digit:]]*\),.*/\2/')
    PRI_X=$(echo $PRI_RES | sed -e 's/\([[:digit:]]*\) x \([[:digit:]]*\)/\1/')
    PRI_Y=$(echo $PRI_RES | sed -e 's/\([[:digit:]]*\) x \([[:digit:]]*\)/\2/')
    SEC_RES=$(xrandr --screen 1| grep ^Screen | sed -e 's/^.*\(current \)\([[:digit:]]* x [[:digit:]]*\),.*/\2/')
    SEC_X=$(echo $SEC_RES | sed -e 's/\([[:digit:]]*\) x \([[:digit:]]*\)/\1/')
    SEC_Y=$(echo $SEC_RES | sed -e 's/\([[:digit:]]*\) x \([[:digit:]]*\)/\2/')

    TOTAL_HEIGHT=$(($PRI_Y > $SEC_Y ? $PRI_Y : $SEC_Y))
    TOTAL_WIDTH=$(($SEC_X_OFFSET+$SEC_X))

    PRI_C0=$(echo "$PRI_X/$TOTAL_WIDTH" | bc -l)
    PRI_C1=$(echo "$PRI_X_OFFSET/$TOTAL_WIDTH" | bc -l)
    PRI_C2=$(echo $PRI_Y/$TOTAL_HEIGHT | bc -l)
    PRI_C3=$(echo "$PRI_Y_OFFSET/$TOTAL_HEIGHT" | bc -l)
    SEC_C0=$(echo "$SEC_X/$TOTAL_WIDTH" | bc -l)
    SEC_C1=$(echo "$SEC_X_OFFSET/$TOTAL_WIDTH" | bc -l)
    SEC_C2=$(echo $SEC_Y/$TOTAL_HEIGHT | bc -l)
    SEC_C3=$(echo "$SEC_Y_OFFSET/$TOTAL_HEIGHT" | bc -l)

    xinput set-prop "$PRI_TOUCH" --type=float "Coordinate Transformation Matrix" "$PRI_C0", 0, "$PRI_C1", 0, "$PRI_C2", "$PRI_C3", 0, 0, 1
    xinput set-prop "$SEC_TOUCH" --type=float "Coordinate Transformation Matrix" "$SEC_C0", 0, "$SEC_C1", 0, "$SEC_C2", "$SEC_C3", 0, 0, 1

    xinput create-master "Secondary"
    xinput reattach "$SEC_TOUCH" "Secondary pointer"
    /usr/local/bin/xpointerscreen "Secondary pointer" 1
