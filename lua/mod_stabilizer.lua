-- settings
-- maximal angular velocity, rad/sec (roll, pitch, yaw)
local max_vel = { 3.0, 1.5, 0.5 }

-- (for Acrobat)
local kp = { 0.15, 0.25, 0.80 }
local ki = { 0.06, 0.06, 0.15 }

-- static vars
local err = { 0.0, 0.0, 0.0 }
local iTerm = { 0.0, 0.0, 0.0 }

-- temp vars
local pTerm = 0

-- servo map
local servo = { 1, 2, 4 }

function angular_velocity_lock()
--[[
    -- use 5th channel for tuning
    local coef = (TX[5] + 1.0) / 2.0   -- -1.0...+1.0 -> 0.0...1.0
    dbg("coef = %f", coef)
    kp[1] = coef
]]

    err[1] = TX[servo[1]] * max_vel[1] - RAW.gyr.x
    err[2] = TX[servo[2]] * max_vel[2] + RAW.gyr.z
    err[3] = TX[servo[3]] * max_vel[3] + RAW.gyr.y

    for i = 1, 3 do
        pTerm = err[i] * kp[i]                  -- proportional
        iTerm[i] = iTerm[i] + err[i] * ki[i]    -- integral
        if iTerm[i] > 0.95 then                 -- check limits
            iTerm[i] = 0.95
        elseif iTerm[i] < -0.95 then
            iTerm[i] = -0.95
        end
        MX[servo[i]] = pTerm + iTerm[i]         -- summator

        if MX[servo[i]] > 1.0 then
            MX[servo[i]] = 1.0
        elseif MX[servo[i]] < -1.0 then
            MX[servo[i]] = -1.0
        end
    end
end

