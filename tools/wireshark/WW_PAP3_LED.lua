-- ww_mcp_led.lua
-- WW_MCP_LED - WinWing PAP3 (MCP) LED Control post-dissector
--  02 0F BF 00 00 03 49 XX YY 00 00 00 00 ...
--  ^  ^  ^           ^  ^  ^
--  0  1..2           6  7  8 (offsets)
--  0x02, PID LE=0xBF0F, opcode=0x49, LED_ID=XX, LED_VAL=YY (00=Off, 01=On)
local WW_MCP_LED = Proto("WW_MCP_LED", "WinWing PAP3 (MCP) LED Control")

-- Map ID -> LED Name
local LED_NAMES = {
    [0x03] = "N1",
    [0x04] = "SPEED",
    [0x05] = "VNAV",
    [0x06] = "LVL CHG",
    [0x07] = "HDG SEL",
    [0x08] = "LNAV",
    [0x09] = "VOR LOC",
    [0x0A] = "APP",
    [0x0B] = "ALT HLD",
    [0x0C] = "V/S",
    [0x0D] = "CMD/A",
    [0x0E] = "CWS/A",
    [0x0F] = "CMD/B",
    [0x10] = "CWS/B",
    [0x11] = "A/T ARM",
    [0x12] = "MA CAPT",
    [0x13] = "MA F/O"
}

-- Fields
local f_led_pid = ProtoField.uint16("WW_MCP_LED.pid", "PID (LE)", base.HEX)
local f_led_opcode = ProtoField.uint8("WW_MCP_LED.opcode", "Opcode", base.HEX)
local f_led_id = ProtoField.uint8("WW_MCP_LED.id", "LED ID", base.DEC)
local f_led_state = ProtoField.string("WW_MCP_LED.on", "Etat LED")
local f_led_name = ProtoField.string("WW_MCP_LED.name", "LED Name")

WW_MCP_LED.fields = {f_led_pid, f_led_opcode, f_led_id, f_led_state, f_led_name}

local f_usbhid_data = Field.new("usbhid.data")
-- Helper to obtain TvbRange from usbhid.data
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

function WW_MCP_LED.dissector(_, pinfo, tree)
    local hid = get_hid_tvbr()
    if not hid then return end

    local len = hid:len()
    -- We need at least to YY (offset 8)
    if len < 9 then return end

    --  - PID LE 0xBF0F offsets 1..2
    --  - opcode 0x49 offset 6
    local pid_le = hid(1, 2):le_uint()
    local opcode = hid(6, 1):uint()
    if pid_le ~= 0xBF0F or opcode ~= 0x49 then return end

    -- It's a LED frame : we parse
    local led_id = hid(7, 1):uint()
    local led_v = hid(8, 1):uint()

    local name = LED_NAMES[led_id] or string.format("ID_0x%02X", led_id)

    pinfo.cols.protocol = "WW_MCP_LED"
    local subtree = tree:add(WW_MCP_LED, hid:tvb(), "LED Control (0x49)")
    subtree:add(f_led_id, hid(7, 1))
    subtree:add(f_led_state, (led_v == 1) and "On" or "Off")
    subtree:add(f_led_name, name)

    -- Info collumn push : ie: "LED[HDG_SEL]=On"
    pinfo.cols.info:append(string.format(" LED[%s]=%s", name,
                                         (led_v == 1) and "On" or "Off"))
end

register_postdissector(WW_MCP_LED)
