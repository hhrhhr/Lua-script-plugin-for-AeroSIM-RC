-- settings
local gps_update = 0.2  -- 5Hz


local function process_sensors()
    local x = RAW.acc.y
    local y = RAW.acc.z
    local z = RAW.acc.x

    RAW.acc.x = x
    RAW.acc.y = y
    RAW.acc.z = z

    x = RAW.gyr.y
    y = RAW.gyr.z
    z = RAW.gyr.x

    RAW.gyr.x = x
    RAW.gyr.y = y
    RAW.gyr.z = z
end

local gps = { dt = 0.0, lat = 0.0, lon = 0.0, alt = 0.0, head = 0.0 }

local function process_gps()
    if gps.dt < gps_update then
        gps.dt = gps.dt + RAW.dt
        RAW.gps.lat = gps.lat
        RAW.gps.lon = gps.lon
        RAW.gps.alt = gps.alt
        RAW.gps.head = gps.head
    else
        gps.dt = 0.0
        local lat_dt = math.abs(RAW.gps.lat - gps.lat)
        local lon_dt = math.abs(RAW.gps.lon - gps.lon)
        if lat_dt > 0.000001 or lon_dt > 0.000001 then
            local lat1 = gps.lat     * DEG2RAD
            local lat2 = RAW.gps.lat * DEG2RAD
            local lon1 = gps.lon     * DEG2RAD
            local lon2 = RAW.gps.lon * DEG2RAD
            lon_dt = lon2 - lon1
            local y = math.sin(lon_dt) * math.cos(lat2)
            local x = math.cos(lat1) * math.sin(lat2)
                    - math.sin(lat1) * math.cos(lat2) * math.cos(lon_dt)

            gps.head = math.atan2(y, x) * RAD2DEG
            if gps.head < 0.0 then
                gps.head = gps.head + 360
            end
            gps.lat = RAW.gps.lat
            gps.lon = RAW.gps.lon
            gps.alt = RAW.gps.alt
        end
    end
end

local function correct_matrix()
    FD.mat:set( M[5], M[8], M[2],
                M[6], M[9], M[3],
                M[4], M[7], M[1] )
end

function correct_sensors()
    process_sensors()
    process_gps()
    correct_matrix()
end
