-- settings
-- maximal angular velocity, rad/sec (roll, pitch, yaw)
local max_vel = { 3.0, 3.0, 2.0 }

-- (for Acrobat)
--local kp = { 0.15, 0.25, 0.80 }
--local ki = { 0.06, 0.06, 0.15 }

-- (for Quad-IV)
local kp = { 0.25, 0.25, 2.0 }
local ki = { 0.06, 0.06, 0.25 }

-- static vars
local err = { 0.0, 0.0, 0.0 }
local iTerm = { 0.0, 0.0, 0.0 }

-- servo map
local servo = { 1, 2, 4 }

function angular_velocity_lock()
--[[
    -- use 5th channel for tuning
    local coef = (TX[5] + 1.0) / 2.0
    dbg("coef = %f", coef)
    kp[1] = coef
--]]

    err[1] = MX[servo[1]] * max_vel[1] - RAW.gyr.x
    err[2] = MX[servo[2]] * max_vel[2] + RAW.gyr.z
    err[3] = MX[servo[3]] * max_vel[3] + RAW.gyr.y
    dbg("1:% f, 2:% f, 3: % f", err[1], err[2], err[3])

    for i = 1, 3 do
        local pTerm = err[i] * kp[i]
        iTerm[i] = iTerm[i] + err[i] * ki[i]

        if iTerm[i] > 0.95 then
            iTerm[i] = 0.95
        elseif iTerm[i] < -0.95 then
            iTerm[i] = -0.95
        end

        MX[servo[i]] = pTerm + iTerm[i]

        if MX[servo[i]] > 1.0 then
            MX[servo[i]] = 1.0
        elseif MX[servo[i]] < -1.0 then
            MX[servo[i]] = -1.0
        end
    end
    dbg("1:% f, 2:% f, 3% f", MX[1], MX[2], MX[4])
end


-- maximal angles, rad (roll, pitch)
local max_angles = { 45.0, 45.0, 0.0 }

local kp_att = { 0.01, 0.01, 0.01 }
local ki_att = { 0.009, 0.009, 0.005 }

local err_att = { 0.0, 0.0, 0.0 }


function attitude_lock()
    err_att[1] = -FD.att.roll
    err_att[2] = 0.0 + FD.att.pitch
    local y = FD.att.yaw
    if y > 180.0 then
        y = 360.0 - y
        y = -y
    end
    err_att[3] = max_angles[3] - y
    dbg("1:% f, 2:% f, 3: % f", err_att[1], err_att[2], err_att[3])

    for i = 1, 2 do
        MX[servo[i]] = MX[servo[i]] + err_att[i] * kp_att[i]
    end

end

