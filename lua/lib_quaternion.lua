Quaternion = Quaternion or {}
Quaternion.__index = Quaternion

function Quaternion:new(x, y, z, w)
    return setmetatable({x = x or 0.0, y = y or 0.0, z = z or 0.0, w = w or 1.0}, Quaternion)
end

setmetatable(Quaternion, {__call = Quaternion.new})

function Quaternion.__add(a, b)
    local x = a.x + b.x
    local y = a.y + b.y
    local z = a.z + b.z
    local w = a.w + b.w
    return Quaternion:new(x, y, z)
end

function Quaternion.__sub(a, b)
    local x = a.x - b.x
    local y = a.y - b.y
    local z = a.z - b.z
    local w = a.w - b.w
    return Quaternion:new(x, y, z)
end

function Quaternion.__mul(a, b)
    local x = a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y
    local y = a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x
    local z = a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w
    local w = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z
--[[
    -- from Qt
    local ww = (a.z+a.x)*(b.x+b.y)
    local yy = (a.w-a.y)*(b.w+b.z)
    local zz = (a.w+a.y)*(b.w-b.z)
    local xx = ww + yy + zz
    local qq = 0.5 * (xx + (a.z-a.x)*(b.x-b.y))

    local w = qq-ww+(a.z-a.y)*(b.y-b.z)
    local x = qq-xx+(a.x+a.w)*(b.x+b.w)
    local y = qq-yy+(a.w-a.x)*(b.y+b.z)
    local z = qq-zz+(a.z+a.y)*(b.w-b.x)
--]]
    return Quaternion:new(x, y, z, w)
end

function Quaternion.__unm(q)
    return Quaternion:new(-q.x, -q.y, -q.z, q.w)
end

function Quaternion:__tostring()
    return string.format("Quaternion(%f, %f, %f, %f)", self.x, self.y, self.z, self.w)
end

function Quaternion:set(x, y, z, w)
    self.x = x
    self.y = y
    self.z = z
    self.w = w
end

function Quaternion:conjugate()
    return Quaternion:new(-self.x, -self.y, -self.z, self.w)
end

function Quaternion:magSq()
    return self.x*self.x + self.y*self.y + self.z*self.z + self.w*self.w
end

function Quaternion:mag()
    return math.sqrt(self:magSq())
end

function Quaternion:scale(s)
    self.x = self.x * s
    self.y = self.y * s
    self.z = self.z * s
    self.w = self.w * s
end

function Quaternion:norm()
    self:scale(1 / self:mag())
end

function Quaternion:map_vector(v)
    local q = self * Quaternion(v.x, v.y, v.z, 0.0) * -self
    return Vector3D(q.x, q.y, q.z)
end

function Quaternion:from_euler(roll, pitch, yaw)
    local ch = math.cos(yaw / 2.0)
    local sh = math.sin(yaw / 2.0)
    local ca = math.cos(pitch / 2.0)
    local sa = math.sin(pitch / 2.0)
    local cb = math.cos(roll / 2.0)
    local sb = math.sin(roll / 2.0)
    local chca = ch * ca
    local shsa = sh * sa
    local shca = sh * ca
    local chsa = ch * sa

    self.x = shsa * cb + chca * sb;
    self.y = shca * cb + chsa * sb;
    self.z = chsa * cb - shca * sb;
    self.w = chca * cb - shsa * sb;
end

function Quaternion:rotate(roll, pitch, yaw)
    local q = Quaternion()
    q:from_euler(roll, pitch, yaw)
    return self * q
end

function Quaternion:to_euler()
    -- q can be non-normalized
    local roll, pitch, yaw = 0.0, 0.0, 0.0
    local q = self

    local xx = q.x * q.x
    local yy = q.y * q.y
    local zz = q.z * q.z
    local ww = q.w * q.w

    local unit = xx + yy + zz + ww
    local test = q.x * q.y + q.z * q.w

    -- check for gimbal lock (~86.3°)
    if test > (0.499 * unit) then         -- north pole
        yaw   = 2 * math.atan2(q.x, q.w)
        pitch = M_PI_2
        roll  = 0.0
    elseif test < (-0.499 * unit) then    -- south pole
        yaw   = -2 * math.atan2(q.x, q.w)
        pitch = -M_PI_2
        roll  = 0.0
    else
        local x, y

        y = 2 * (q.x * q.w - q.y * q.z)
        x = -xx + yy - zz + ww
        roll = math.atan2(y, x)
        pitch = math.asin(2 * test / unit)
        y = 2 * (q.y * q.w - q.x * q.z)
        x = xx - yy - zz + ww
        yaw = math.atan2(y, x)
    end

    return roll, pitch, yaw
end

function Quaternion:from_matrix3x3(m)
    local x = math.sqrt(math.max(0, 1 + m[1][1] - m[2][2] - m[3][3])) * 0.5
    local y = math.sqrt(math.max(0, 1 - m[1][1] + m[2][2] - m[3][3])) * 0.5
    local z = math.sqrt(math.max(0, 1 - m[1][1] - m[2][2] + m[3][3])) * 0.5
    local w = math.sqrt(math.max(0, 1 + m[1][1] + m[2][2] + m[3][3])) * 0.5

    self.x = copysign(x, m[3][2] - m[2][3])
    self.y = copysign(y, m[1][3] - m[3][1])
    self.z = copysign(z, m[2][1] - m[1][2])
    self.w = w
end

function Quaternion:to_matrix3x3()
    local m = Matrix3x3()

    local xx = self.x * self.x;
    local xy = self.x * self.y;
    local xz = self.x * self.z;
    local xw = self.x * self.w;
    local yy = self.y * self.y;
    local yz = self.y * self.z;
    local yw = self.y * self.w;
    local zz = self.z * self.z;
    local zw = self.z * self.w;

    m[1][1] = 1 - 2 * ( yy + zz );
    m[1][2] =     2 * ( xy - zw );
    m[1][3] =     2 * ( xz + yw );
    m[2][1] =     2 * ( xy + zw );
    m[2][2] = 1 - 2 * ( xx + zz );
    m[2][3] =     2 * ( yz - xw );
    m[3][1] =     2 * ( xz - yw );
    m[3][2] =     2 * ( yz + xw );
    m[3][3] = 1 - 2 * ( xx + yy );

    return m
end
