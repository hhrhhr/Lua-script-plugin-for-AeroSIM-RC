Matrix3x3 = Matrix3x3 or {}
Matrix3x3.__index = Matrix3x3

function Matrix3x3:new(xx, xy, xz, yx, yy, yz, zx, zy, zz)
    return setmetatable({
        { xx or 1.0, xy or 0.0, xz or 0.0 },
        { yx or 0.0, yy or 1.0, yz or 0.0 },
        { zx or 0.0, zy or 0.0, zz or 1.0 }
    }, Matrix3x3)
end

function Matrix3x3:set(xx, xy, xz, yx, yy, yz, zx, zy, zz)
    self[1][1] = xx; self[1][2] = xy; self[1][3] = xz
    self[2][1] = yx; self[2][2] = yy; self[2][3] = yz
    self[3][1] = zx; self[3][2] = zy; self[3][3] = zz
end

function Matrix3x3:map_vector(v)
    local x = v.x*self[1][1] + v.y*self[1][2] + v.z*self[1][3]
    local y = v.x*self[2][1] + v.y*self[2][2] + v.z*self[2][3]
    local z = v.x*self[3][1] + v.y*self[3][2] + v.z*self[3][3]
    return Vector3D:new(x, y, z)
end

function Matrix3x3:__tostring()
    return string.format("Matrix3x3((%f,\t%f,\t%f)\n\t  (%f,\t%f,\t%f)\n\t  (%f,\t%f,\t%f))",
            self[1][1], self[1][2], self[1][3],
            self[2][1], self[2][2], self[2][3],
            self[3][1], self[3][2], self[3][3]
        )
end

setmetatable(Matrix3x3, {__call = Matrix3x3.new})
