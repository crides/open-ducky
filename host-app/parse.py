#!/usr/bin/python3

import sys, re, usb

STR = 0x00
RELEASE = 0x10
DELAY = 0x80
MOUSE = 0x90
PRESS = 0xA0
HOLD = 0xB0
FREQ = 0xC0
COLOR = 0xD0
CLICK = 0xE0
STOP = 0xFF

cmds = ["STR", "RELEASE", "DELAY", "HOLD",
        "PRESS", "FREQ", "COLOR"]
str_allowed = "^[-\\w\\d!-)`~+=_'\":;<>,.?/\\\\|]+$"
# These keys can be stand alone cmds
modifiers = ["CTRL", "SHIFT", "ALT", "SUPER"]
# These keys cannot be standalone cmds, but can be combined with modifiers
other_keys = [str(i) for i in range(10)] + list("`-=[]\\';,./") + [chr(i) for i in range(0x41, 0x5b)]
str_lookup, act_key_lookup = {}, {}
with open("str_lookup_tab", "r") as f:
    str_lookup = {line[0]: eval(line[2:-1]) for line in f.readlines()}

with open("act_key_lookup_tab", "r") as f:
    for line in f.readlines():
        kv = line.split("=")
        act_key_lookup[kv[0].strip()] = eval(kv[1].strip())

all_keys_lookup = act_key_lookup.copy()
all_keys_lookup.update(str_lookup)

action_keys = list(act_key_lookup.keys())
all_keys = action_keys + list(str_lookup.keys())

def err(msg, *args, **kwargs):
    print(msg, *args, file=sys.stderr, **kwargs)

def errloc(fname, ln, msg, *args, **kwargs):
    print("In file {} on line {}: ".format(fname, ln), msg, *args, file=sys.stderr, **kwargs)

def send_data(data):
    #print([hex(i) for i in data])
    #data = ADDRS + COLORS + data
    dev = usb.core.find(idVendor=0x0451, idProduct=0xe012)
    dev.detach_kernel_driver(0)
    dev.set_configuration(1)

    data_size = len(data)
    buf = [0, 0xCA, (data_size >> 8) & 0xFF, data_size & 0xFF] + [0 for i in range(16)]
    dev.ctrl_transfer(0x21, 0x09, 0x0200, 0, buf, 5000)
    for i in range(data_size // 16 + 1):
        dev.ctrl_transfer(0x21, 0x09, 0x0200, 0, [0, 0xCA, 0, 0] + data[i * 16:(i + 1) * 16], 5000)

    usb.util.dispose_resources(dev)
    dev.attach_kernel_driver(0)

def parse_inst(lines):                  # TODO Remove Color section in MCU
    addrs = [0x1808 for i in range(4)]  # Need to convert it later
    data = []
    def add(data, val):
        if isinstance(val, list):
            data += val
        else:
            data.append(val)

    for lnr, line in enumerate(lines):
        line = line[:-1]                # Get rid of the line ending
        if line == "":
            continue
        #cmds = [s.upper() for s in line.split()]
        cmds = line.split()
        cmd, *operands = cmds
        match = re.match("^BTN([1234]):$", line)
        if match:                       # A label
            addrs[int(match.group(1)) - 1] += len(data) # With addrs
        elif cmd in action_keys:
            if not operands:             # Only one key
                add(data, PRESS)
                add(data, act_key_lookup[cmd])
            elif cmd in modifiers:
                if cmds[-1] not in all_keys:
                    errloc(fname, lnr, "Last key should not be a modifier!")
                    exit(1)
                head = PRESS
                for i in range(1, len(cmds) - 1):
                    if keys[i] not in modifiers:
                        errloc(fname, lnr, "Every key except the last should be a modifier!")
                        exit(1)
                    head |= eval(cmds[i])
                add(data, head)
                add(data, all_keys_lookup[cmds[-1]])
            else:
                errloc(fname, lnr, "`{}' not allowed here!".format(cmd))
                exit(1)

        elif cmd == "STR":
            if not re.match(str_allowed, operands[0]):
                errloc(fname, lnr, "STR operand contains invalid characters!")
                exit(1)
            add(data, STR)
            add(data, [str_lookup[c] for c in operands[0]])
            add(data, 0)
        elif cmd == "RELEASE":
            add(data, RELEASE)
        elif cmd == "MOVE":
            if len(operands) != 2:
                errloc(fname, lnr, "Only 2 operands allowed for MOVE!")
                exit(1)
            try:
                for i in range(2):
                    operands[i] = int(operands[i])
            except:
                errloc(fname, lnr, "Operands of MOVE must be all integers " \
                        + "or a specified color!")
                exit(1)
            add(data, MOUSE)
            add(data, operands)
        elif cmd == "DELAY":
            add(data, DELAY)
            add(data, int(operands[0]))
        elif cmd == "LEFTCLICK":
            add(data, CLICK | 0x01)
        elif cmd == "RIGHTCLICK":
            add(data, CLICK | 0x02)
        elif cmd == "MIDDLECLICK":
            add(data, CLICK | 0x04)
        elif cmd in ["PRESS", "HOLD"]:
            head = 0
            if cmd == "PRESS":
                head = PRESS
            else:
                head = HOLD
            for i in range(len(operands) - 1):
                if operands[i] not in modifiers:
                    errloc(fname, lnr, "Every key except the last should be a modifier!")
                    exit(1)
                head |= eval(operands[i])
            add(data, head)
            add(data, eval(operands[-1]))
        elif cmd == "COLOR":
            if len(operands) == 3:    # Probably real colors
                try:
                    for i in range(3):
                        operands[i] = int(operands[i])
                except:
                    errloc(fname, lnr, "Operands of COLOR must be all integers " \
                            + "or a specified color!")
                    exit(1)
                add(data, COLOR)
                add(data, operands)
            elif len(operands) == 1:
                color = 0
                color_s = operands[0]
                if color_s == "NONE":
                    color = 0
                elif color_s == "RED":
                    color = 0x04
                elif color_s == "GREEN":
                    color = 0x02
                elif color_s == "BLUE":
                    color = 0x01
                elif color_s == "MEGENTA":
                    color = 0x05
                elif color_s == "YELLOW":
                    color = 0x06
                elif color_s == "CYAN":
                    color = 0x03
                elif color_s == "WHITE":
                    color = 0x07
                else:
                    errloc(fname, lnr, "Unknown color {}!".format(color_s))
                    exit(1)
                add(data, COLOR | 0x08 | color)
                # TODO Edit code in MCU to interp short codes
            else:
                errloc(fname, lnr, "Unknown operands to COLOR!")
                exit(1)
        elif cmd == "STOP":
            add(data, STOP)
        else:
            errloc(fname, lnr, "No such command `{}'!".format(cmd))
            exit(1)
    #add(data, STOP)
    if len(data) + 0x1808 >= 0x3C00:
        err("Too many instructions! Please reduce the size by {}".format(len(data) + 0x1808 - 0x3C00))
        exit(1)
    # Seperate addrs
    real_addrs = []
    for i, addr in enumerate(addrs):
        real_addrs.append((addr & 0xFF00) >> 8)
        real_addrs.append(addr & 0xFF)
    data = real_addrs + data
    return data

if __name__ == '__main__' and len(sys.argv) == 2:
    # TODO Add error log
    fname = sys.argv[1]
    lines = None
    try:
        with open(fname, "r") as f:
            lines = f.readlines()
    except:
        err("No such file: " + fname)
    send_data(parse_inst(lines))
