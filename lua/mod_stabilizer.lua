--[[ settings ]]
-- maximal angular velocity (rad/sec)
local max_vel = { 5.0, 1.5, 0.5 }

-- coefficients
--[[
-- Acrobat
local kp = { 0.10, 0.20, 0.40 }
local ki = { 0.04, 0.06, 0.08 }
local kd = { 0.60, 0.60, 0.60 }
--]]

-- Extra
local kp = { 0.20, 0.15, 0.30 }
local ki = { 0.00, 0.00, 0.00 }
local kd = { 0.60, 0.70, 0.60 }


--[[ static variables ]]
-- states
local ki_state = { 0.0, 0.0, 0.0 }
local kd_state = { 0.0, 0.0, 0.0 }

-- internals
local abs = math.abs
local servo = { 1, 2, 4 }

function angular_velocity_lock()
    local err = { TX[1] * max_vel[1] - RAW.gyro.y,
                  TX[2] * max_vel[2] + RAW.gyro.x,
                  TX[4] * max_vel[3] + RAW.gyro.z}

    coef = (TX[5] + 1) / 2.0   -- -1.0...+1.0 -> 0.0...1.0
    table.insert(DBG, "coef = " .. coef)
    --kd[3] = coef

    for i = 1, 3 do
        -- proportional
        local pTerm = err[i] * kp[i]

        -- integral
        ki_state[i] = ki_state[i] + err[i]
        if ki_state[i] > 1.0 then
            ki_state[i] = 1.0
        elseif ki_state[i] < -1.0 then
            ki_state[i] = -1.0
        end
        local iTerm = ki_state[i] * ki[i]

        -- derivative
        local dTerm = (MX[servo[i]] - kd_state[i]) * kd[i]
        kd_state[i] = MX[servo[i]]

        -- summator
        MX[servo[i]] = MX[servo[i]] + pTerm + iTerm - dTerm

        -- check limits
        if MX[servo[i]] > 1.0 then
            MX[servo[i]] = 1.0
        elseif MX[servo[i]] < -1.0 then
            MX[servo[i]] = -1.0
        end

    end
end

