local resolution = 200
local servo = { 1, 2, 4 }

function process_actuator()
    for i = 1, 3 do
        local s = MX[servo[i]]
        s = math.floor(s * resolution)
        s = s / resolution
        RX[servo[i]] = s
    end
end

