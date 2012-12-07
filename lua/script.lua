-- script.lua
for k, v in pairs({...}) do
    package.path = v .. "\\lua\\?.lua"
end

require("vector3d")
require("matrix3x3")
require("quaternion")

-- sensors data
local sensor = {
    dt = 0.0,
    gyro = Vector3D(),
    acc = Vector3D(),
    gps = { lat = 0.0, lon = 0.0, alt = 0.0, head = 0.0 }
}

-- transmitter (joystick)
local tx = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}

-- fligth data
local fd = {
    time = 0,
    mat = Matrix3x3(),
    quat = Quaternion(),
    pos = Vector3D(),
    att = {roll = 0.0, pitch = 0.0, yaw = 0.0}
}

-- reciever
local rx = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}

-- main loop, called every frame
function main(dt,
              gx, gy, gz,
              ax, ay, az,
              lat, lon, alt, head)

-- fill sensors
    sensor.dt = dt
    sensor.gyro:set(gx, gy, gz)
    sensor.acc:set(ax, ay, az)
    sensor.gps.lat = lat
    sensor.gps.lon = lon
    sensor.gps.alt = alt
    sensor.gps.head = head

-- calculate flight data
    fd.time = fd.time + dt

    return fd.time
end
