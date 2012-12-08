Quaternion = Quaternion or {}
Quaternion.__index = Quaternion

function Quaternion:new(x, y, z, w)
    return setmetatable({x = x or 0.0, y = y or 0.0, z = z or 0.0, w = w or 1.0}, Quaternion)
end

function Quaternion:set(x, y, z, w)
    self.x = x
    self.y = y
    self.z = z
    self.w = w
end

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

    local x = a.w*b.x + a.x*b.w + a.y*b.z + a.z*b.y
    local y = a.w*b.y + a.y*b.w + a.z*b.x - a.x*b.z
    local z = a.w*b.z + a.z*b.w + a.x*b.y - a.y*b.x
    local w = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z

--[[
    -- from Qt
    local ww = (a.z+a.x)*(b.x+b.y)
    local yy = (a.w-a.y)*(b.w+b.z)
    local zz = (a.w+a.y)*(b.w-b.z)
    local xx = ww + yy + zz
    local qq = 0.5* (xx + (a.z-a.x)*(b.x-b.y))

    local w = qq-ww+(a.z-a.y)*(b.y-b.z)
    local x = qq-xx+(a.x+a.w)*(b.x+b.w)
    local y = qq-yy+(a.w-a.x)*(b.y+b.z)
    local z = qq-zz+(a.z+a.y)*(b.w-b.x)
--]]

    return Quaternion:new(x, y, z, w)
end

function Quaternion:__tostring()
    return string.format("Quaternion(%f,\t%f,\t%f,\t%f)", self.x, self.y, self.z, self.w)
end

function Quaternion:conjugate()
    local x = -self.x
    local y = -self.y
    local z = -self.z
    local w = self.w
    return Quaternion:new(x, y, z, w)
end

function Quaternion:mag()
    return math.sqrt(self.x*self.x + self.y*self.y + self.z*self.z + self.w*self.w)
end

function Quaternion:magSq()
    return self.x*self.x + self.y*self.y + self.z*self.z + self.w*self.w
end

function Quaternion:norm()
    local mag = math.sqrt(self.x*self.x + self.y*self.y + self.z*self.z + self.w*self.w)
    self.x = self.x / mag
    self.y = self.y / mag
    self.z = self.z / mag
    self.w = self.w / mag
end

function Quaternion:scale(s)
    self.x = self.x*s
    self.y = self.y*s
    self.z = self.z*s
    self.w = self.w*s
end

setmetatable(Quaternion, {__call = Quaternion.new})

