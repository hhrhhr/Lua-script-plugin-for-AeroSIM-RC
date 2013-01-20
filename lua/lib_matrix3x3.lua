--[[
порядок вращения: Y, Z, X
psi     ψ   roll    bank        X    front
phi     φ   pitch   attitude    Z    right
theta   θ   yaw     heading     Y    up
--]]

Matrix3x3 = Matrix3x3 or {}
Matrix3x3.__index = Matrix3x3

function Matrix3x3:new(xx, xy, xz, yx, yy, yz, zx, zy, zz)
    return setmetatable({
        { xx or 1.0, xy or 0.0, xz or 0.0 },
        { yx or 0.0, yy or 1.0, yz or 0.0 },
        { zx or 0.0, zy or 0.0, zz or 1.0 }
    }, Matrix3x3)
end

setmetatable(Matrix3x3, {__call = Matrix3x3.new})

function Matrix3x3.__add(a, b)
    local m = Matrix3x3()
    for r = 1, 3 do
        for c = 1, 3 do
            m[r][c] = a[r][c] + b[r][c]
        end
    end
    return m
end

function Matrix3x3.__sub(a, b)
    local m = Matrix3x3()
    for r = 1, 3 do
        for c = 1, 3 do
            m[r][c] = a[r][c] - b[r][c]
        end
    end
    return m
end

function Matrix3x3.__mul(a, b)
    local m = Matrix3x3()
    local sum = 0.0
    for r = 1, 3 do
        for c = 1, 3 do
            sum = 0.0
            for i = 1, 3 do
                sum = sum + a[r][i] * b[i][c]
            end
            m[r][c] = sum
        end
    end
    return m
end

function Matrix3x3:__tostring()
    return string.format("Matrix3x3((% f, % f, % f)\n          (% f, % f, % f)\n          (% f, % f, % f))",
            self[1][1], self[1][2], self[1][3],
            self[2][1], self[2][2], self[2][3],
            self[3][1], self[3][2], self[3][3]
        )
end

function Matrix3x3:set(xx, xy, xz, yx, yy, yz, zx, zy, zz)
    self[1][1] = xx; self[1][2] = xy; self[1][3] = xz
    self[2][1] = yx; self[2][2] = yy; self[2][3] = yz
    self[3][1] = zx; self[3][2] = zy; self[3][3] = zz
end

function Matrix3x3:transpose()
    local m = Matrix3x3()
    for r = 1, 3 do
        for c = 1, 3 do
            m[c][r] = self[r][c]
        end
    end
    return m
end

function Matrix3x3:map_vector(v)
    local x = v.x*self[1][1] + v.y*self[1][2] + v.z*self[1][3]
    local y = v.x*self[2][1] + v.y*self[2][2] + v.z*self[2][3]
    local z = v.x*self[3][1] + v.y*self[3][2] + v.z*self[3][3]
    return Vector3D:new(x, y, z)
end

--[[
       1 X           2 Y               3 Z
    +--------+----------------+-----------------+
1 X !  ch*ca !    -ch*sa*cb   !  ch*sa*sb+sh*cb !
    +--------+----------------+-----------------+
2 Y !   sa   !      ca*cb     !      -ca*sb     !
    +--------+----------------+-----------------+
3 Z ! -sh*ca ! sh*sa*cb+ch*sb ! -sh*sa*sb+ch*cb !
    +--------+----------------+-----------------+
--]]

function Matrix3x3:from_euler(roll, pitch, yaw)
    local ch = math.cos(yaw)
    local sh = math.sin(yaw)
    local ca = math.cos(pitch)
    local sa = math.sin(pitch)
    local cb = math.cos(roll)
    local sb = math.sin(roll)
    local chsa = ch*sa
    local shsa = sh*sa

    self[1][1] =  ch*ca
    self[1][2] = -chsa*cb + sh*sb
    self[1][3] =  chsa*sb + sh*cb
    self[2][1] =  sa
    self[2][2] =  ca*cb
    self[2][3] = -ca*sb
    self[3][1] = -sh*ca
    self[3][2] =  shsa*cb + ch*sb
    self[3][3] = -shsa*sb + ch*cb
end

function Matrix3x3:rotate(roll, pitch, yaw)
    local m = Matrix3x3()
    m:from_euler(roll, pitch, yaw)
    return self * m
end

function Matrix3x3:to_euler()
    local roll, pitch, yaw = 0.0, 0.0, 0.0

    -- check for gimbal lock (~86.3°)
    if self[2][1] > 0.998 then      -- north pole
        roll  = 0.0
        pitch = M_PI_2
        yaw   = math.atan2(self[1][3], self[3][3])
    elseif self[2][1] < -0.998 then -- south pole
        roll  = 0.0
        pitch = -M_PI_2
        yaw   = math.atan2(self[1][3], self[3][3])
    else
        roll  = math.atan2(-self[2][3], self[2][2])
        pitch = math.asin(self[2][1])
        yaw   = math.atan2(-self[3][1], self[1][1])
    end

    return roll, pitch, yaw
end

