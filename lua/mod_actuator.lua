-- settings
local resolution = 100      -- half

-- input channels
--           THR   AIL   PIT   RUD
local tx = {  3,    1,    2,    4 }

-- output channels
local rx = { 31, 32, 33, 34, 12, 13, 27}

-- mixer matrix [#rx][#tx]
local mx = {
           { 1.0,  0.0, -0.4, -0.5 },
           { 1.0, -0.4,  0.0,  0.5 },
           { 1.0,  0.0,  0.4, -0.5 },
           { 1.0,  0.4,  0.0,  0.5 },
           { 0.0,  0.0,  0.0, -0.5 },
           { 0.0,  0.0,  0.5,  0.0 },
           { 0.0,  0.5,  0.0,  0.0 }
}
--[[
^ front
!       31
!       O
!  34 O   O 32
!       O
!       33     right
+------------------>
]]

function actuator_pre_mixer()
    for k, v in ipairs(tx) do
        MX[v] = TX[v]
    end
end

function actuator_mixer()
    for k, mxs in ipairs(mx) do
        local ch = 0.0

        for n, m in ipairs(mxs) do
            ch = ch + MX[tx[n]] * m
        end

        RX[rx[k]] = ch

        if RX[rx[k]] > 1.0 then
            RX[rx[k]] = 1.0
        elseif RX[rx[k]] < -1.0 then
            RX[rx[k]] = -1.0
        end
    end
end


function actuator_post_mixer()
    for _, v in ipairs(rx) do
        local s = RX[v]
        s = (math.floor(s * resolution)) / resolution
        RX[v] = s
    end
end

