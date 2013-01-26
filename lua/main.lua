require("plugin_config")
require("libraries")

-- data from simulator
RAW = {
    menu = 0,
    dt = 0.0,
    gyr = Vector3D(),
    acc = Vector3D(),
    gps = { lat = 0.0, lon = 0.0, alt = 0.0 }
}
TX = {}
M = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }

-- output data
RX = {}
DBGSTR = ""

-- work data
MX = {}     -- mixer
DBG = {}    -- debug strings buffer

-- init tables
for i = 1, 39 do
    table.insert(TX, 0.0)
    table.insert(MX, 0.0)
    table.insert(RX, 0.0)
end

require("modules")

-- put your structs here -------------------------------------------------\

FD = {
    time = 0.0,
    menu = { false, false, false, false },
    mat = Matrix3x3(),
    quat = Quaternion(),
    pos = Vector3D(),
    vel = Vector3D(),
    att = { roll = 0.0, pitch = 0.0, yaw = 0.0 }
}

-- end of user code ------------------------------------------------------/

function main()     -- main loop, called every frame
    DBG = {}
    parse_menu()
    correct_sensors()

-- put your code here ----------------------------------------------------\

    dbg("menu: %4d (%5s, %5s, %5s, %5s)",
        RAW.menu, FD.menu[1], FD.menu[2], FD.menu[3], FD.menu[4])

    FD.time = FD.time + RAW.dt
    dbg("time: %f", FD.time)

    dbg("gyr x:% 10.6f, y:% 10.6f, z:% 10.6f", RAW.gyr.x, RAW.gyr.y, RAW.gyr.z)
    dbg("acc x:% 10.6f, y:% 10.6f, z:% 10.6f", RAW.acc.x, RAW.acc.y, RAW.acc.z)
    dbg("lat: % f, lon: % f", RAW.gps.lat, RAW.gps.lon)
    dbg("alt: %.2f head: %.2f\n", RAW.gps.alt, RAW.gps.head)

    attitude_calculate_raw()
    dbg("   raw RPY: % 7.2f, % 7.2f, % 7.2f", FD.att.roll, FD.att.pitch, FD.att.yaw)

    attitude_MadgwickAHRSupdateIMU()
    dbg("!!AHRS RPY: % 7.2f, % 7.2f, % 7.2f", FD.att.roll, FD.att.pitch, FD.att.yaw)


    actuator_pre_mixer()

    -- Plugin channel 1
    if TX[23] > 0.333 then          -- function #1
        dbg("MODE : angular velocity locked")
        angular_velocity_lock()
    elseif TX[23] > -0.333 then     -- function #2
        dbg("MODE : direct control")
    else                            -- function #3
        dbg("MODE : attitude lock")
        attitude_lock()
        angular_velocity_lock()
    end

    -- Plugin channel 2
    if TX[24] > 0.333 then          -- function #4
        --
    elseif TX[24] > -0.333 then     -- function #5
        --
    else                            -- function #6
        --
    end

    actuator_mixer()

    actuator_post_mixer()


-- end of user code ------------------------------------------------------/

    DBGSTR = table.concat(DBG)
end

