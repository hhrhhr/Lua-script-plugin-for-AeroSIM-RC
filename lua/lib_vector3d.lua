Vector3D = Vector3D or {}
Vector3D.__index = Vector3D

function Vector3D:new(x, y, z)
    return setmetatable({x = x or 0.0, y = y or 0.0, z = z or 0.0}, Vector3D)
end

--setmetatable(Vector3D, { __call = function(_, ...) return Vector3D.new(...) end })
setmetatable(Vector3D, { __call = Vector3D.new })

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

function Vector3D.__mul(a, b)   -- cross product
    local x = a.y * b.z - a.z * b.y
    local y = a.z * b.x - a.x * b.z
    local z = a.x * b.y - a.y * b.x
    return Vector3D:new(x, y, z)
end

function Vector3D.__unm(v)      -- unary minus
    return Vector3D:new(-v.x, -v.y, -v.z)
end

function Vector3D.__tostring(v)
    return string.format("Vector3D(%f, %f, %f)", v.x, v.y, v.z)
end

function Vector3D:set(x, y, z)
    self.x = x
    self.y = y
    self.z = z
end

function Vector3D:dot(v)
    return self.x * v.x + self.y * v.y + self.z * v.z
end

function Vector3D:lenSq()
    return self.x * self.x + self.y * self.y + self.z * self.z
end

function Vector3D:len()
    return math.sqrt(self:lenSq())
end

function Vector3D:scale(s)
    self.x = self.x * s
    self.y = self.y * s
    self.z = self.z * s
end

function Vector3D:mul(s)
    local x = self.x * s
    local y = self.y * s
    local z = self.z * s
    return Vector3D:new(x, y, z)
end

function Vector3D:norm()
    self:scale(1 / self:len())
end

function Vector3D:angle_to(v)
    local dot = self:dot(v)
    local scalar = self:len() * v:len()
    return math.acos(dot / scalar)
end

