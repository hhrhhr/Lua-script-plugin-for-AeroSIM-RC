-- main.lua
require("lib_vector3d")
require("lib_matrix3x3")
require("lib_quaternion")

-- sensors data
RAW = {
    dt = 0.0,
    gyro = Vector3D(),
    acc = Vector3D(),
    gps = { lat = 0.0, lon = 0.0, alt = 0.0, head = 0.0 }
}

-- transmitter (joystick)
TX = {-0.11, -0.22, -0.33, -0.44, -0.55, -0.66, -0.77, -0.88, -0.99}

-- fligth data
FD = {
    time = 0,
    mat = Matrix3x3(),
    quat = Quaternion(),
    pos = Vector3D(),
    att = { roll = 0.0, pitch = 0.0, yaw = 0.0 }
}

-- reciever
RX = {0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09, 0.10}

require("mod_stabilizer")

-- main loop, called every frame
function main(dt,
              gx, gy, gz,
              ax, ay, az,
              lat, lon, alt, head,
              ch1, ch2, ch3, ch4, ch5, ch6, ch7, ch8, ch9)

-- fill sensors
    RAW.dt = dt
    RAW.gyro:set(gx, gy, gz)
    RAW.acc:set(ax, ay, az)
    RAW.gps.lat = lat
    RAW.gps.lon = lon
    RAW.gps.alt = alt
    RAW.gps.head = head
    TX = { ch1, ch2, ch3, ch4, ch5, ch6, ch7, ch8, ch9 }

-- calculate flight data
    FD.time = FD.time + dt

    angle_velocity_lock()

-- back to C
    return FD.time, RX
end

