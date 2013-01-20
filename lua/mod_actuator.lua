-- settings
local resolution = 100      -- half

-- DR & reverse
local mixer = { [1] = 1.0, [3] = 1.0 }


function process_actuator()
    for k, v in ipairs(mixer) do
        MX[k] = MX[k] * v
    end

    for i = 1, 39 do
        local s = MX[i]
        s = math.floor(s * resolution ) / resolution
        RX[i] = s
    end
end

