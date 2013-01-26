function attitude_calculate_raw()
    local r, p, y = FD.mat:to_euler()

    y = -y
    if y < 0 then
        y = y + M_2_PI
    end

    FD.att.roll  = r * RAD2DEG
    FD.att.pitch = p * RAD2DEG
    FD.att.yaw   = y * RAD2DEG
end

