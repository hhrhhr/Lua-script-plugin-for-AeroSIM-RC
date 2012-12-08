local max_roll_vel = 3.141596 * 2.0
local max_pitch_vel = 3.141596
local max_yaw_vel = 3.141596 / 2.0

local abs = math.abs

function angle_velocity_lock()
    -- just bypass
    if TX[8] < 0.75 then
        RX[1] = TX[1]
        RX[2] = TX[2]
        RX[4] = TX[4]
        return
    end

    local need = { TX[1] * max_roll_vel, TX[2] * max_pitch_vel, TX[4] * max_yaw_vel }
    local curr = { RAW.gyro.y, -RAW.gyro.x, -RAW.gyro.z }
    local servo = { 1, 2, 4 }

    for i = 1, 3 do
        local s = RX[servo[i]]

        local err = curr[i] >= need[i]
        local adj = abs(curr[i] - need[i]) * 0.1

        if err then
            s = s - adj
        else
            s = s + adj
        end

        if s > 1.0 then
            s = 1.0
        elseif s < -1.0 then
            s = -1.0
        end

        RX[servo[i]] = s
    end
end
