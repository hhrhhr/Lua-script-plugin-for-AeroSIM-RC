DEG2RAD = math.pi / 180.0
RAD2DEG = 180.0 / math.pi
M_2_PI     = math.pi * 2.0
M_PI_2    = math.pi / 2.0
M_PI_4    = math.pi / 4.0

function copysign(x, y)
    if ((x < 0 and y > 0) or (x > 0 and y < 0)) then
        return -x;
    else
        return x;
    end
end

function null_function()
    local value = nil
end


function dbg(fmt, ...)
    table.insert(DBG, string.format(fmt .. "\n", ...))
end


local MENU = {
--    bit.lshift(1, 0), -- reload scripts
--    bit.lshift(1, 1), -- change window size and position
    bit.lshift(1, 2),
    bit.lshift(1, 3),
    bit.lshift(1, 4),
    bit.lshift(1, 5)
}
function parse_menu()
    for k, v in ipairs(MENU) do
        FD.menu[k] = bit.band(RAW.menu, v) ~= 0
    end
end


local idx = 1
function next_resolution()
    local res = SETTINGS.resolutions
    local x, y, w, h = res[idx][1], res[idx][2], res[idx][3], res[idx][4]
    idx = idx + 1
    if idx > #res then
        idx = 1
    end
    return x, y, w, h
end

