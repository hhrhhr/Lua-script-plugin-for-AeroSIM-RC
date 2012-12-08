Vector3D = Vector3D or {}
Vector3D.__index = Vector3D

function Vector3D:new(x, y, z)
    return setmetatable({x = x or 0.0, y = y or 0.0, z = z or 0.0}, Vector3D)
end

function Vector3D:set(x, y, z)
    self.x = x
    self.y = y
    self.z = z
end

function Vector3D.__add(a, b)
    local x = a.x + b.x
    local y = a.y + b.y
    local z = a.z + b.z
    return Vector3D:new(x, y, z)
end

function Vector3D.__sub(a, b)
    local x = a.x - b.x
    local y = a.y - b.y
    local z = a.z - b.z
    return Vector3D:new(x, y, z)
end

function Vector3D.__mul(a, b)
    local x = a.y * b.z + a.z * b.y
    local y = a.z * b.x + a.x * b.z
    local z = a.x * b.y + a.y * b.x
    return Vector3D:new(x, y, z)
end

function Vector3D:__tostring()
    return string.format("Vector3D(%f,\t%f,\t%f)", self.x, self.y, self.z)
end

function Vector3D:dot(v)
    return self.x * v.x + self.y * v.y + self.z * v.z
end

function Vector3D:len()
    return math.sqrt(self.x * self.x + self.y * self.y + self.z * self.z)
end

function Vector3D:lenSq()
    return self.x * self.x + self.y * self.y + self.z * self.z
end

function Vector3D:norm()
    local len = self:len()
    self.x = self.x / len
    self.y = self.y / len
    self.z = self.z / len
end

function Vector3D:scale(s)
    self.x = self.x * s
    self.y = self.y * s
    self.z = self.z * s
end

setmetatable(Vector3D, {__call = Vector3D.new})

