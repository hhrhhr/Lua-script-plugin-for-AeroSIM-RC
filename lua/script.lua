-- script.lua
for k, v in pairs({...}) do
    package.path = v .. "\\lua\\?.lua"
end

require("vector3d")
require("matrix3x3")

local fligthTime = 0.0

function main(dt,
              xx, xy, xz, yx, yy, yz, zx, zy, zz,
              ax, ay, az,
              gx, gy, gz)

    fligthTime = fligthTime + dt

    -- add gravity to model acceleration
    local mat = Matrix3x3:new(xx, xy, xz, yx, yy, yz, zx, zy, zz)
    local acc = Vector3D:new(ax, ay, az)
    local gee = mat:map_vector(Vector3D:new(0.0, 0.0, -9.81))
    acc = acc + gee

    return fligthTime, gee.x, gee.y, gee.z
end
