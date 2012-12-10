--[[ settings ]]
-- maximal angular velocity (rad/sec)
local max_vel = { 5.0, 1.5, 0.5 }

-- coefficients
--[[
-- Acrobat
local kp = { 0.10, 0.20, 0.40 }
local ki = { 0.04, 0.06, 0.08 }
local kd = { 0.60, 0.60, 0.60 }

-- Extra
local kp = { 0.20, 0.15, 0.30 }
local ki = { 0.00, 0.00, 0.00 }
local kd = { 0.60, 0.70, 0.60 }
--]]

-- dt for Acrobat
local kp = { 0.11, 0.25, 0.80 }
local ki = { 4.00, 4.00, 8.00 }
local kd = { 0.0015, 0.0030, 0.0050 }


--[[ static variables ]]
-- states
local err_state = { 0.0, 0.0, 0.0 }
local ki_state = { 0.0, 0.0, 0.0 }
local kd_state = { 0.0, 0.0, 0.0 }

-- const internals
local servo = { 1, 2, 4 }

function angular_velocity_dt_lock()
    local err = { TX[1] * max_vel[1] - RAW.gyro.y,
                  TX[2] * max_vel[2] + RAW.gyro.x,
                  TX[4] * max_vel[3] + RAW.gyro.z}

    coef = (TX[5] + 1) / 2.0   -- -1.0...+1.0 -> 0.0...1.0
    table.insert(DBG, "coef = " .. coef)
    --kd[3] = coef / 100.0

    for i = 1, 3 do
        local pTerm = err[i] * kp[i]

        ki_state[i] = ki_state[i] + err[i] * RAW.dt

        if ki_state[i] > 0.1 then
            ki_state[i] = 0.1
        elseif ki_state[i] < -0.1 then
            ki_state[i] = -0.1
        end

        local iTerm = ki_state[i] * ki[i]

        local dTerm = (err_state[i] - err[i]) / RAW.dt * kd[i]
        err_state[i] = err[i]

        MX[servo[i]] = pTerm + iTerm + dTerm

        if MX[servo[i]] > 1.0 then
            MX[servo[i]] = 1.0
        elseif MX[servo[i]] < -1.0 then
            MX[servo[i]] = -1.0
        end
    end
end

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

