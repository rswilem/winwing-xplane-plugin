#!/usr/bin/env python3

#Thanks to @schenlap for the original idea. Used in his Winwing FCU XPplane Plugin
import usb.core, usb.util, usb.backend.libusb1

VID = 0x4098
PID = 0xBF0F

# ---------- helpers ----------
def pad64(b):
    b = bytearray(b)
    if len(b) > 64:
        raise ValueError(
    f"packet too long: {len(b)} > 64 | data: {' '.join(f'{byte:02X}' for byte in b)}"
)
    b.extend(b'\x00' * (64 - len(b)))
    return bytes(b)

def find_backend():
    paths = [
        '/usr/lib/x86_64-linux-gnu/libusb-1.0.so.0',
        '/usr/lib/libusb-1.0.so.0',
        '/opt/homebrew/lib/libusb-1.0.0.dylib',
    ]
    for p in paths:
        be = usb.backend.libusb1.get_backend(find_library=lambda x: p)
        if be:
            return be
    return None

def open_pap3():
    be = find_backend()
    dev = usb.core.find(idVendor=VID, idProduct=PID, backend=be)
    if dev is None:
        raise RuntimeError("PAP3 (4098:BF0F) not found")

    dev.set_configuration()
    cfg = dev.get_active_configuration()
    intf = cfg[(0,0)]

    try:
        if dev.is_kernel_driver_active(intf.bInterfaceNumber):
            dev.detach_kernel_driver(intf.bInterfaceNumber)
    except Exception:
        pass

    ep_out = usb.util.find_descriptor(intf, custom_match=lambda e: usb.util.endpoint_direction(e.bEndpointAddress) == usb.util.ENDPOINT_OUT)
    ep_in  = usb.util.find_descriptor(intf, custom_match=lambda e: usb.util.endpoint_direction(e.bEndpointAddress) == usb.util.ENDPOINT_IN)
    if not ep_out:
        raise RuntimeError("OUT endpoint not found")
    return dev, ep_in, ep_out

# ---------- packets ----------
def send_lcd_init(ep_out):
    # EXACTEMENT ta trame (64 bytes)
    pkt = bytes.fromhex(
        "f0 00 03 12 0f bf 00 00 04 01 00 00 26 cc 00 00"
        "00 01 00 00 00 03 00 00"
        + "00 " * (64-24)
    )
    ep_out.write(pkt)

def send_38(ep_out, seq, payload_after_1d):
    head = [
        0xF0, 0x00, seq, 0x38, 0x0F, 0xBF, 0x00, 0x00,
        0x02, 0x01, 0x00, 0x00, 0xDF, 0xA2, 0x50, 0x00,
        0x00, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00
    ]

    buf = bytearray(head)
    buf.extend(payload_after_1d)
    ep_out.write(pad64(buf))

def send_38_empty(ep_out, seq):
    ep_out.write(pad64([0xF0, 0x00, (seq & 0xFF), 0x38]))

def send_2A_commit(ep_out, seq):
    buf = bytearray([0xF0, 0x00, (seq & 0xFF), 0x2A])
    if len(buf) < 0x1D:
        buf.extend(b'\x00' * (0x1D - len(buf)))
    tail = [0x0F,0xBF,0x00,0x00,0x03,0x01,0x00,0x00,0xDF,0xA2,0x50,0x00]
    buf.extend(tail)
    ep_out.write(pad64(buf))

def send_led(ep_out, led_id, status):
    head = [
        0x02, 0x0f, 0xbf, 0x00, 0x00, 0x03, 0x49, led_id,
        status, 0x00, 0x00, 0x00, 0x00, 0x00
    ]

    buf = bytearray(head)
    ep_out.write(buf)

def solenoid_off(ep_out):

    head = [
        0x02, 0x0f, 0xbf, 0x00, 0x00, 0x03, 0x49, 0x1E,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    ]

    buf = bytearray(head)
    ep_out.write(buf)   

# ---------- demo ----------
def main():
    _, _, ep_out = open_pap3()

    send_lcd_init(ep_out)

    seq = 0x40
    payload = bytes.fromhex(
        "00 00 10 "
        "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
        "00 00 00 00 00 00 00 00 00 00 00 00"
    )
    send_38(ep_out, seq, payload); seq += 1

    # 3) deux 0x38 vides
    send_38_empty(ep_out, seq); seq += 1
    send_38_empty(ep_out, seq); seq += 1

    # 4) commit 0x2A
    send_2A_commit(ep_out, seq); seq += 1

    print("Done: init + 0x38 + 0x38(empty)*2 + 0x2A(commit) envoyés.")

    for i in range(17):
        send_led(ep_out, led_id=(0x03+i), status=0x00)
        print(f"LED 0x{0x03+i:02X} OFF")
    

    send_led(ep_out, led_id=0x0F, status=0x01)
    print("LED 0x03 ON")

    solenoid_off(ep_out)
    print("AT solenoid OFF (will auto-ON after ~60ms)")
    

if __name__ == "__main__":
    main()
