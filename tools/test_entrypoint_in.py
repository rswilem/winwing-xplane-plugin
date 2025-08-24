#!/bin/env python3

#Thanks to @schenlap for the original idea. Used in his Winwing FCU XPplane Plugin

# -*- coding: utf-8 -*-

import time
import usb.core
import usb.backend.libusb1
import usb.util
import curses

# ========== USB helper ==========
def find_usblib():
    path = [
        '/opt/homebrew/lib/libusb-1.0.0.dylib',
        '/usr/lib/x86_64-linux-gnu/libusb-1.0.so.0',
        '/usr/lib/libusb-1.0.so.0'
    ]
    for p in path:
        backend = usb.backend.libusb1.get_backend(find_library=lambda _: p)
        if backend:
            print(f"using {p}")
            return backend

    print(f"*** No usblib found. Install it with:")
    print(f"***   debian: apt install libusb-1.0-0")
    print(f"***   mac:    brew install libusb")
    print(f"***   If you get this warning and fcu is working, please open an issue at")
    print(f"***   https://github.com/schenlap/winwing_fcu")
    return None

# ========== UI helper (curses) ==========
COLUMNS_PER_ROW = 8
CELL_WIDTH = 14 

def draw_grid(stdscr, values, last_values):
    """
    Affiche:
      ligne offsets: 0x00 ... (centré au-dessus)
      ligne valeurs: (0xNN - ddd)
    Met en surbrillance les cellules qui ont changé (vs last_values).
    """
    stdscr.erase()
    max_y, max_x = stdscr.getmaxyx()

    stdscr.addstr(0, 0, "HID report viewer — offsets (top) / values (bottom). 'q' to quit.")

    if values is None:
        stdscr.refresh()
        return

    n = len(values)
    rows = (n + COLUMNS_PER_ROW - 1) // COLUMNS_PER_ROW

    try:
        curses.start_color()
        curses.use_default_colors()
        curses.init_pair(1, curses.COLOR_YELLOW, -1)
        changed_attr = curses.color_pair(1) | curses.A_BOLD
    except curses.error:
        changed_attr = curses.A_BOLD 

    base_y = 2

    for idx in range(n):
        row = idx // COLUMNS_PER_ROW
        col = idx % COLUMNS_PER_ROW

        header = f"0x{idx:02X}"
        val    = values[idx]
        value_str = f"(0x{val:02X} - {val:3d})"

        y_header = base_y + row * 2
        y_value  = y_header + 1
        x_cell   = col * CELL_WIDTH

        x_header = x_cell + (CELL_WIDTH - len(header)) // 2
        x_value  = x_cell


        attr = curses.A_NORMAL
        if last_values is not None and idx < len(last_values):
            if last_values[idx] != values[idx]:
                attr = changed_attr


        try:
            stdscr.addstr(y_header, x_header, header, curses.A_DIM)
            stdscr.addstr(y_value,  x_value,  value_str.ljust(CELL_WIDTH - 1), attr)
        except curses.error:
            pass

    stdscr.refresh()


def run_ui(stdscr, endpoint_in):
    stdscr.nodelay(True)
    curses.curs_set(0)

    num_bytes = None 
    buf_bytes = None

    last_draw = 0.0

    while True:
        try:
            buf_bytes = endpoint_in.read(0x81, 7)
        except usb.core.USBTimeoutError:
            pass
        except usb.core.USBError:
            pass


        if buf_bytes is not None:
            if num_bytes is None:

                num_bytes = buf_bytes
            else:
                if num_bytes != buf_bytes:

                    draw_grid(stdscr, list(buf_bytes), list(num_bytes))

                    num_bytes = buf_bytes

        now = time.time()
        if now - last_draw > 0.2:
            draw_grid(stdscr,
                      list(num_bytes) if num_bytes is not None else None,
                      None)
            last_draw = now

        # Quit avec 'q'
        try:
            ch = stdscr.getch()
            if ch in (ord('q'), ord('Q')):
                return
        except curses.error:
            pass

        time.sleep(0.01)


def main():
    backend = find_usblib()

    device = usb.core.find(idVendor=0x4098, idProduct=0xBF0F, backend=backend)
    if device is None:
        print("searching for MCP ... fails")
        return
    else:
        print("searching for MCP ... found")

    # Prépare la config / interface
    device.set_configuration()
    cfg = device.get_active_configuration()
    interface = cfg[(0, 0)]

    # macOS : is_kernel_driver_active
    try:
        if device.is_kernel_driver_active(interface.bInterfaceNumber):
            device.detach_kernel_driver(interface.bInterfaceNumber)
    except NotImplementedError:
        pass

    endpoints = interface.endpoints()
    endpoint_in = endpoints[0]
    print(endpoint_in)

    curses.wrapper(run_ui, endpoint_in)


if __name__ == "__main__":
    main()
