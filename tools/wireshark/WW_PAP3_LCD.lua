-- WW_MCP_LCD - WinWing PAP3 (MCP) 7-Segments Protocol post-dissector
-- Conditions to be activate :
--   header (0x00..0x01, LE) == 0x00f0
--   opcode (0x03)          == 0x38
local WW_MCP_LCD = Proto("WW_MCP_LCD", "WinWing PAP3 (MCP) 7-Segments Protocol")

-- Fields (for filtering/coloring)
local f_header = ProtoField.uint16("WW_MCP_LCD.header", "Header", base.HEX)
local f_pktnum =
    ProtoField.uint8("WW_MCP_LCD.packet", "Packet Number", base.HEX)
local f_opcode = ProtoField.uint8("WW_MCP_LCD.opcode", "Opcode", base.HEX)
local f_pid = ProtoField.uint16("WW_MCP_LCD.pid", "PID", base.HEX)
local f_unk_6_7 = ProtoField.uint16("WW_MCP_LCD.unk_6_7", "Unknown", base.HEX)
local f_cmd1 = ProtoField.uint8("WW_MCP_LCD.command1", "Unknown", base.HEX)
local f_always1 = ProtoField.uint8("WW_MCP_LCD.always_one", "Always One",
                                   base.HEX)
local f_unk_10_11 = ProtoField.uint16("WW_MCP_LCD.unk_0A_0B", "Unknown",
                                      base.HEX)
local f_checksum =
    ProtoField.uint16("WW_MCP_LCD.checksum", "Unknown", base.HEX)
local f_unk_15_29 = ProtoField.bytes("WW_MCP_LCD.unk_0E_1C", "Unknown",
                                     base.SPACE)
local f_payload = ProtoField.bytes("WW_MCP_LCD.payload", "Payload", base.SPACE)

-- Decode 7-seg (Speed + captain course + Heading + Altitude_Hi)
local f_seg_flags = ProtoField.uint8("WW_MCP_LCD.seg.unknown_bit_0x10",
                                     "Segment unknown flag (0x10)", base.HEX)
local f_speed_value = ProtoField.string("WW_MCP_LCD.speed", "Speed (IAS/Mach)")
local f_captain_course_value = ProtoField.string("WW_MCP_LCD.course",
                                                 "Captain Course")
local f_seg_bytes = ProtoField.bytes("WW_MCP_LCD.segbytes",
                                     "Raw segment bytes (A..G)")
local f_heading_value = ProtoField.string("WW_MCP_LCD.heading", "Heading")
local f_alt_hi_value = ProtoField.string("WW_MCP_LCD.altitude_hi",
                                         "Altitude (10k–1k–100)")
local f_alt_lo_value = ProtoField.string("WW_MCP_LCD.altitude_lo",
                                         "Altitude (10–1)")
local f_alt_value = ProtoField.string("WW_MCP_LCD.altitude", "Altitude")
local f_fo_course_value = ProtoField.string("WW_MCP_LCD.fo_course", "F/O Course")
local f_vspeed_value = ProtoField.string("WW_MCP_LCD.vspeed", "VSpeed")

local has_bit32, bit32lib = pcall(require, "bit32")
local has_bit, bitlib = pcall(require, "bit")

local function BAND(a, b)
    if has_bit32 and bit32lib and bit32lib.band then
        return bit32lib.band(a, b)
    end
    if has_bit and bitlib and bitlib.band then return bitlib.band(a, b) end
    local res, bit = 0, 1
    while a > 0 or b > 0 do
        local abit, bbit = a % 2, b % 2
        if abit == 1 and bbit == 1 then res = res + bit end
        a = math.floor(a / 2);
        b = math.floor(b / 2);
        bit = bit * 2
    end
    return res
end
WW_MCP_LCD.fields = {
    f_header, f_pktnum, f_opcode, f_pid, f_unk_6_7, f_cmd1, f_always1,
    f_unk_10_11, f_checksum, f_unk_15_29, f_payload, f_seg_flags, f_speed_value,
    f_captain_course_value, f_seg_bytes, f_heading_value, f_fo_course_value,
    f_vspeed_value, f_alt_value
}

-- Offsets -> segments for Speed And Captain Course
local SEG_OFFS = {
    A = 0x35, -- Top
    B = 0x31, -- Top-right
    C = 0x2D, -- Bottom-right
    D = 0x29, -- Bottom
    E = 0x25, -- Bottom-left
    F = 0x21, -- Top-left
    G = 0x1D -- Middle
}

-- Offsets -> segments for Heading + Altitude(3 digits from left)
local SEG_OFFS_HA = {
    A = 0x36, -- Top
    B = 0x32, -- Top-right
    C = 0x2E, -- Bottom-right
    D = 0x2A, -- Bottom
    E = 0x26, -- Bottom-left
    F = 0x22, -- Top-left
    G = 0x1E -- Middle
}

-- Offsets F/O Course
local SEG_OFFS_FO = {
    G = 0x20,
    F = 0x24,
    E = 0x28,
    D = 0x2C,
    C = 0x30,
    B = 0x34,
    A = 0x38
}

-- Offsets VSPEED/Altitude
local SEG_OFFS_VA = {
    G = 0x1F,
    F = 0x23,
    E = 0x27,
    D = 0x2B,
    C = 0x2F,
    B = 0x33,
    A = 0x37
}

-- Positioning Bits (F/O Course)
local POSBITS_FO = {
    FoCU = 0x10, -- units F/O
    FoCD = 0x20, -- tens F/O
    FoCC = 0x40, -- hundreds F/O
    PlusBar = 0x80 -- special: upper/lower bars of the + sign (see below)
}
-- Positioning Bits (VSPEED + Alt low)
local POSBITS_VA = {
    VsU = 0x01,
    VsD = 0x02,
    VsC = 0x04,
    VsK = 0x08, -- 1,10,100,1000
    Vs10K = 0x10, -- 10k VSPEED: Only G segment (middle) is present -> serves as sign '-'
    AltU = 0x40, -- Units altitude
    AltD = 0x80 -- Tens altitude
}
-- Bits per digit position
local POSBITS = {
    U = 0x01, -- speed units
    D = 0x02, -- speed tens
    C = 0x04, -- speed hundreds
    M = 0x08, -- speed thousands
    PilCU = 0x20, -- captain course units
    PilCD = 0x40, -- captain course tens
    PilCC = 0x80 -- captain course hundreds
}

-- Bits per position (Heading + Alt left)
local POSBITS_HA = {
    AltC = 0x01, -- Altitude hundreds (3rd from left)
    AltK = 0x02, -- Altitude thousands
    AltT = 0x04, -- Altitude tens of thousands
    U08 = 0x08, -- Unknown
    HdU = 0x10, -- Heading units
    HdD = 0x20, -- Heading tens
    HdC = 0x40, -- Heading hundreds
    U80 = 0x80 -- Unknown
}

local BIT_UNKNOWN = 0x10 -- unknown (exposed as flag)

-- Patterns 7-seg standard (A..G) for 0..9 (true=segment ON)
-- Indexation: {A,B,C,D,E,F,G}
local DIGITS = {
    [0] = {true, true, true, true, true, true, false},
    [1] = {false, true, true, false, false, false, false},
    [2] = {true, true, false, true, true, false, true},
    [3] = {true, true, true, true, false, false, true},
    [4] = {false, true, true, false, false, true, true},
    [5] = {true, false, true, true, false, true, true},
    [6] = {true, false, true, true, true, true, true},
    [7] = {true, true, true, false, false, false, false},
    [8] = {true, true, true, true, true, true, true},
    [9] = {true, true, true, false, false, true, true}
}

-- Letters / symbols on 7 segments
-- Indexation: {A,B,C,D,E,F,G}
local EXTRAS = {
    A = {true, true, true, false, true, true, true}, -- "A"
    C = {true, false, false, true, true, true, false}, -- "C"
    E = {true, false, false, true, true, true, true}, -- "E"
    F = {true, false, false, false, true, true, true}, -- "F"
    H = {false, true, true, false, true, true, true}, -- "H"
    L = {false, false, false, true, true, true, false}, -- "L"
    P = {true, true, false, false, true, true, true}, -- "P"
    U = {false, true, true, true, true, true, false}, -- "U"
    ["-"] = {false, false, false, false, false, false, true} -- dash (G Segment)
}

-- Convert {A..G}=bool to "0".."9" char or letter/symbol ("A","C",...)
-- Returns nil if unknown pattern
local function segs_to_char(segs)
    -- 1) digits
    for d = 0, 9 do
        local pat = DIGITS[d]
        local ok = true
        for i, seg_on in ipairs(segs) do
            if pat[i] ~= seg_on then
                ok = false;
                break
            end
        end
        if ok then return tostring(d) end
    end
    -- 2) extras (letters/symbols)
    for ch, pat in pairs(EXTRAS) do
        local ok = true
        for i, seg_on in ipairs(segs) do
            if pat[i] ~= seg_on then
                ok = false;
                break
            end
        end
        if ok then return ch end
    end
    return nil
end

-- Get all segments Bytes (A..G) for captain course/Speed
local function read_seg_bytes(hid)
    local function at(off)
        return (hid:len() > off) and hid(off, 1):uint() or 0
    end
    return {
        A = at(SEG_OFFS.A),
        B = at(SEG_OFFS.B),
        C = at(SEG_OFFS.C),
        D = at(SEG_OFFS.D),
        E = at(SEG_OFFS.E),
        F = at(SEG_OFFS.F),
        G = at(SEG_OFFS.G)
    }
end

-- Get segment bytes (A..G) for Heading/Altitude
local function read_seg_bytes_HA(hid)
    local function at(off)
        return (hid:len() > off) and hid(off, 1):uint() or 0
    end
    return {
        A = at(SEG_OFFS_HA.A),
        B = at(SEG_OFFS_HA.B),
        C = at(SEG_OFFS_HA.C),
        D = at(SEG_OFFS_HA.D),
        E = at(SEG_OFFS_HA.E),
        F = at(SEG_OFFS_HA.F),
        G = at(SEG_OFFS_HA.G)
    }
end

-- Reconstruct the 4 speed digits (M C D U) and 3 course digits (PilCC PilCD PilCU)
local function decode_speed_course(hid)
    local sb = read_seg_bytes(hid)

    -- for each position, build {A..G} = bool
    local function pos_to_segs(bitmask)
        return {
            BAND(sb.A, bitmask) ~= 0, BAND(sb.B, bitmask) ~= 0,
            BAND(sb.C, bitmask) ~= 0, BAND(sb.D, bitmask) ~= 0,
            BAND(sb.E, bitmask) ~= 0, BAND(sb.F, bitmask) ~= 0,
            BAND(sb.G, bitmask) ~= 0
        }
    end

    local speed = {
        M = segs_to_char(pos_to_segs(POSBITS.M)),
        C = segs_to_char(pos_to_segs(POSBITS.C)),
        D = segs_to_char(pos_to_segs(POSBITS.D)),
        U = segs_to_char(pos_to_segs(POSBITS.U))
    }
    local course = {
        PilCC = segs_to_char(pos_to_segs(POSBITS.PilCC)),
        PilCD = segs_to_char(pos_to_segs(POSBITS.PilCD)),
        PilCU = segs_to_char(pos_to_segs(POSBITS.PilCU))
    }

    -- unknown flags (0x10) present on one of the segments?
    local unknown_flag_present = BAND(sb.A, BIT_UNKNOWN) ~= 0 or
                                     BAND(sb.B, BIT_UNKNOWN) ~= 0 or
                                     BAND(sb.C, BIT_UNKNOWN) ~= 0 or
                                     BAND(sb.D, BIT_UNKNOWN) ~= 0 or
                                     BAND(sb.E, BIT_UNKNOWN) ~= 0 or
                                     BAND(sb.F, BIT_UNKNOWN) ~= 0 or
                                     BAND(sb.G, BIT_UNKNOWN) ~= 0

    return speed, course, sb, unknown_flag_present
end

local function decode_heading_alt_hi(hid, BAND, BIT_UNKNOWN)
    local sb = read_seg_bytes_HA(hid)

    local function pos_to_segs(bitmask)
        return {
            BAND(sb.A, bitmask) ~= 0, BAND(sb.B, bitmask) ~= 0,
            BAND(sb.C, bitmask) ~= 0, BAND(sb.D, bitmask) ~= 0,
            BAND(sb.E, bitmask) ~= 0, BAND(sb.F, bitmask) ~= 0,
            BAND(sb.G, bitmask) ~= 0
        }
    end

    -- Altitude (left → right) : T (10k), K (1k), C (100)
    local alt_hi = {
        T = segs_to_char(pos_to_segs(POSBITS_HA.AltT)),
        K = segs_to_char(pos_to_segs(POSBITS_HA.AltK)),
        C = segs_to_char(pos_to_segs(POSBITS_HA.AltC))
    }

    -- Heading : C (hundreds), D (tens), U (units)
    local heading = {
        C = segs_to_char(pos_to_segs(POSBITS_HA.HdC)),
        D = segs_to_char(pos_to_segs(POSBITS_HA.HdD)),
        U = segs_to_char(pos_to_segs(POSBITS_HA.HdU))
    }

    local unk_flag_08 = BAND(sb.A, 0x08) ~= 0 or BAND(sb.B, 0x08) ~= 0 or
                            BAND(sb.C, 0x08) ~= 0 or BAND(sb.D, 0x08) ~= 0 or
                            BAND(sb.E, 0x08) ~= 0 or BAND(sb.F, 0x08) ~= 0 or
                            BAND(sb.G, 0x08) ~= 0

    local unk_flag_80 = BAND(sb.A, 0x80) ~= 0 or BAND(sb.B, 0x80) ~= 0 or
                            BAND(sb.C, 0x80) ~= 0 or BAND(sb.D, 0x80) ~= 0 or
                            BAND(sb.E, 0x80) ~= 0 or BAND(sb.F, 0x80) ~= 0 or
                            BAND(sb.G, 0x80) ~= 0

    return heading, alt_hi, sb, (unk_flag_08 or unk_flag_80)
end

local function read_seg_bytes_map(hid, MAP)
    local function at(off)
        return (hid:len() > off) and hid(off, 1):uint() or 0
    end
    return {
        A = at(MAP.A),
        B = at(MAP.B),
        C = at(MAP.C),
        D = at(MAP.D),
        E = at(MAP.E),
        F = at(MAP.F),
        G = at(MAP.G)
    }
end

local function decode_fo_course(hid)
    local sb = read_seg_bytes_map(hid, SEG_OFFS_FO)

    local function pos_to_segs(bitmask)
        return {
            BAND(sb.A, bitmask) ~= 0, BAND(sb.B, bitmask) ~= 0,
            BAND(sb.C, bitmask) ~= 0, BAND(sb.D, bitmask) ~= 0,
            BAND(sb.E, bitmask) ~= 0, BAND(sb.F, bitmask) ~= 0,
            BAND(sb.G, bitmask) ~= 0
        }
    end

    local fo = {
        C = segs_to_char(pos_to_segs(POSBITS_FO.FoCC)),
        D = segs_to_char(pos_to_segs(POSBITS_FO.FoCD)),
        U = segs_to_char(pos_to_segs(POSBITS_FO.FoCU))
    }

    -- The + sign bars (info) : 0x80 on offsets 0x2C (D) and/or 0x28 (E)
    local plus_top = BAND(sb.D, POSBITS_FO.PlusBar) ~= 0 -- above the '-' ??
    local plus_bot = BAND(sb.E, POSBITS_FO.PlusBar) ~= 0 -- below the '-' ??

    return fo, plus_top, plus_bot
end

local function decode_vspeed_alt_lo(hid)
    local sb = read_seg_bytes_map(hid, SEG_OFFS_VA)

    local function pos_to_segs(bitmask)
        return {
            BAND(sb.A, bitmask) ~= 0, BAND(sb.B, bitmask) ~= 0,
            BAND(sb.C, bitmask) ~= 0, BAND(sb.D, bitmask) ~= 0,
            BAND(sb.E, bitmask) ~= 0, BAND(sb.F, bitmask) ~= 0,
            BAND(sb.G, bitmask) ~= 0
        }
    end

    local function blank_or_char(bitmask)
        -- All segment OFF for this position ?
        if BAND(sb.A, bitmask) == 0 and BAND(sb.B, bitmask) == 0 and
            BAND(sb.C, bitmask) == 0 and BAND(sb.D, bitmask) == 0 and
            BAND(sb.E, bitmask) == 0 and BAND(sb.F, bitmask) == 0 and
            BAND(sb.G, bitmask) == 0 then return "" end
        local ch = segs_to_char(pos_to_segs(bitmask))
        return (ch == nil) and "?" or ch
    end

    -- VS digits (no sign)
    local vsK = blank_or_char(POSBITS_VA.VsK)
    local vsC = blank_or_char(POSBITS_VA.VsC)
    local vsD = blank_or_char(POSBITS_VA.VsD)
    local vsU = blank_or_char(POSBITS_VA.VsU)
    local vs_digits = vsK .. vsC .. vsD .. vsU

    -- "-" sign if G segment is lit for Vs10K
    local minus_flag = (BAND(sb.G, POSBITS_VA.Vs10K) ~= 0)

    -- Altitude (10–1)
    local alt_lo = blank_or_char(POSBITS_VA.AltD) ..
                       blank_or_char(POSBITS_VA.AltU)

    return vs_digits, minus_flag, alt_lo
end

-- Globale Declaration
local f_usbhid_data = Field.new("usbhid.data")

-- helper function to get a TvbRange from usbhid.data
local function get_hid_tvbr()
    local fi = f_usbhid_data()
    if not fi then return nil end
    if type(fi) == "table" then
        if #fi == 0 then
            return nil
        else
            fi = fi[1]
        end
    end

    -- CASE A : It's a FieldInfo (got a .value) -> FieldInfo:range()
    local is_fieldinfo = pcall(function() return fi.value end)
    if is_fieldinfo then
        -- 1) Direct try with FieldInfo:range()
        local ok_r, tvbr = pcall(function() return fi:range() end)
        if ok_r and tvbr and pcall(function() return tvbr:len() end) then
            return tvbr
        end
        -- 2) Fallback : Rebuild a Tvb from the ByteArray of FieldInfo.value
        local ok_ba, ba = pcall(function() return fi.value end)
        if ok_ba and ba then
            local tvb = ByteArray.tvb(ba, "usbhid.data")
            return tvb:range(0, tvb:len())
        end
    end

    -- CASE B : Already a TvbRange (has :len() and :tvb())
    if type(fi) == "userdata" and pcall(function() return fi:len() end) and
        pcall(function() return fi:tvb() end) then return fi end

    -- CASE C : It's a Tvb (has :reported_length())
    if type(fi) == "userdata" and
        pcall(function() return fi:reported_length() end) then
        local tvb = fi
        return tvb:range(0, tvb:len())
    end

    return nil
end

local function first_tvb(field)
    if not field then return nil end
    local v = field()
    if type(v) == "table" then
        if #v == 0 then
            return nil
        else
            return v[1]
        end
    end
    return v
end

function WW_MCP_LCD.dissector(_, pinfo, tree)
    local hid = get_hid_tvbr()
    if not hid then return end

    -- From here 'hid' MUST be a TvbRange
    -- Always use the object syntax with ':' for TvbRange methods
    local len = hid:len()
    if len < 5 then return end

    local header_le = hid(0, 2):le_uint()
    local opcode = hid(3, 1):uint()
    if header_le ~= 0x00f0 or opcode ~= 0x38 then return end

    pinfo.cols.protocol = "WW_MCP_LCD"
    local subtree = tree:add(WW_MCP_LCD, hid:tvb(), "Datas")

    -- Fields
    subtree:add_le(f_header, hid(0, 2))
    subtree:add(f_pktnum, hid(2, 1))
    subtree:add(f_opcode, hid(3, 1))
    subtree:add_le(f_pid, hid(4, 2))
    subtree:add_le(f_unk_6_7, hid(6, 2))
    subtree:add(f_cmd1, hid(8, 1))
    local always1 = subtree:add(f_always1, hid(9, 1))
    if hid(9, 1):uint() ~= 1 then
        always1:add_expert_info(PI_MALFORMED, PI_WARN, "Expected value 1")
    end
    subtree:add_le(f_unk_10_11, hid(10, 2))
    subtree:add_le(f_checksum, hid(12, 2))

    if len >= 0x1D then
        subtree:add(f_unk_15_29, hid(0x0E, 0x1D - 0x0E))
        if len > 0x1D then
            subtree:add(f_payload, hid(0x1D, len - 0x1D))
            -- Common sub-tree for all 7-seg decodings
            local dec_tree = subtree:add(WW_MCP_LCD, hid:tvb())
            dec_tree:set_text("Decoded 7-segments Display Data")

            -- Shared variables for Info line & unknown flag
            local speed_str, course_str, heading_str, alt_hi_str, fo_course_str,
                  vspeed_str, alt_lo_str, vsign_str
            local any_unknown = false
            -- === 7-segments decoding: Speed (M C D U) and Pilot Course (PilCC PilCD PilCU) ===
            do
                local speed, course, segbytes, unknown_flag =
                    decode_speed_course(hid)

                -- Blank if all segments (A..G) are OFF for a given position
                local function is_blank(bitmask)
                    return BAND(segbytes.A, bitmask) == 0 and
                               BAND(segbytes.B, bitmask) == 0 and
                               BAND(segbytes.C, bitmask) == 0 and
                               BAND(segbytes.D, bitmask) == 0 and
                               BAND(segbytes.E, bitmask) == 0 and
                               BAND(segbytes.F, bitmask) == 0 and
                               BAND(segbytes.G, bitmask) == 0
                end

                -- If blank => "", else show digit; if unknown pattern => "?"
                local function digit_or_blank(digit, bitmask)
                    if is_blank(bitmask) then return "" end
                    return (digit == nil) and "?" or tostring(digit)
                end
                speed_str = digit_or_blank(speed.M, POSBITS.M) ..
                                digit_or_blank(speed.C, POSBITS.C) ..
                                digit_or_blank(speed.D, POSBITS.D) ..
                                digit_or_blank(speed.U, POSBITS.U)

                course_str = digit_or_blank(course.PilCC, POSBITS.PilCC) ..
                                 digit_or_blank(course.PilCD, POSBITS.PilCD) ..
                                 digit_or_blank(course.PilCU, POSBITS.PilCU)

                -- Cumulative Unknown Flag
                any_unknown = any_unknown or unknown_flag
            end

            -- === 7-seg decode (F/O Course) ===
            do
                local fo, plus_top, plus_bot = decode_fo_course(hid)
                local function c(v)
                    return (v == nil) and "?" or v
                end
                fo_course_str = c(fo.C) .. c(fo.D) .. c(fo.U)

                -- Keep info for VSPEED sign
                _fo_plus_top = plus_top
                _fo_plus_bot = plus_bot

            end

            -- === 7-seg decode (VSpeed / Altitude-lo) ===
            do
                local vs_digits, minus_flag, altlo = decode_vspeed_alt_lo(hid)

                -- Requested sign rule
                local sign_char = ""
                if minus_flag then
                    if (_fo_plus_top and _fo_plus_bot) then
                        sign_char = "+" -- '-' + 'top+bot' => positive
                    else
                        sign_char = "-" -- '-' alone => negative
                    end
                end

                vspeed_str = sign_char .. vs_digits
                alt_lo_str = altlo

            end

            -- === 7-seg decode (Heading / Altitude-hi) ===
            do
                local heading, alt_hi, segbytes2, unk2 =
                    decode_heading_alt_hi(hid, BAND, BIT_UNKNOWN)

                -- blank if all segments OFF for the given position
                local function is_blank_ha(bitmask)
                    return BAND(segbytes2.A, bitmask) == 0 and
                               BAND(segbytes2.B, bitmask) == 0 and
                               BAND(segbytes2.C, bitmask) == 0 and
                               BAND(segbytes2.D, bitmask) == 0 and
                               BAND(segbytes2.E, bitmask) == 0 and
                               BAND(segbytes2.F, bitmask) == 0 and
                               BAND(segbytes2.G, bitmask) == 0
                end

                local function c_or_blank(ch, bitmask)
                    if is_blank_ha(bitmask) then return "" end
                    return (ch == nil) and "?" or tostring(ch)
                end

                heading_str = c_or_blank(heading.C, POSBITS_HA.HdC) ..
                                  c_or_blank(heading.D, POSBITS_HA.HdD) ..
                                  c_or_blank(heading.U, POSBITS_HA.HdU)

                alt_hi_str = c_or_blank(alt_hi.T, POSBITS_HA.AltT) ..
                                 c_or_blank(alt_hi.K, POSBITS_HA.AltK) ..
                                 c_or_blank(alt_hi.C, POSBITS_HA.AltC)


                -- Cumulative Unknown Flags
                any_unknown = any_unknown or unk2
            end

            local altitude_full = (alt_hi_str or "") .. (alt_lo_str or "")
            dec_tree:add(f_captain_course_value, course_str)
            dec_tree:add(f_speed_value, speed_str)
            dec_tree:add(f_heading_value, heading_str)
            dec_tree:add(f_alt_value, altitude_full)
            dec_tree:add(f_vspeed_value, vspeed_str)
            dec_tree:add(f_fo_course_value, fo_course_str)
            -- Add 0x10 flag if at least one of the two decodings saw it
            if any_unknown then
                dec_tree:add(f_seg_flags, BIT_UNKNOWN)
            end

            -- Consolidated Info Column
            pinfo.cols.info:append(string.format(
                                       " CPT-CRS=%s/ SPD=%s/  FO-CRS=%s  VS=%s  ALT[%s]",
                                       course_str or "", speed_str or "",
                                       fo_course_str or "", vspeed_str or "",
                                       ((alt_hi_str or "") .. (alt_lo_str or ""))))
        end
    elseif len > 0x0E then
        subtree:add(f_unk_15_29, hid(0x0E, len - 0x0E))
    end
end

register_postdissector(WW_MCP_LCD)
