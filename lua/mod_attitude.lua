-- settings
local head_kP = 0.0001

-- local vars
local err = 0.0
local gyr = Vector3D()
local m = Matrix3x3()
local q = Quaternion()
local r, p, y = 0.0, 0.0, 0.0

function calculate_attitude_matrix()
--[[
    err = RAW.gps.head - FD.att.yaw
    if err > 180.0 then err = err - 360 end
    if err < -180.0 then err = 360 - err end
    err = err * head_kP
]]
    gyr.x = RAW.gyr.x * RAW.dt
    gyr.y = RAW.gyr.y * RAW.dt
    gyr.z = RAW.gyr.z * RAW.dt

    m = m:rotate(gyr.x, gyr.z, gyr.y)

    r, p, y = m:to_euler()
    y = -y
    if y < 0 then
        y = y + M_2_PI
    end

    FD.att.roll  = r * RAD2DEG
    FD.att.pitch = p * RAD2DEG
    FD.att.yaw   = y * RAD2DEG
end

function calculate_attitude_quat()
    gyr.x = RAW.gyr.x * RAW.dt
    gyr.y = RAW.gyr.y * RAW.dt
    gyr.z = RAW.gyr.z * RAW.dt

    q = q:rotate(gyr.x, gyr.z, gyr.y)

    r, p, y = q:to_euler()
    y = -y
    if y < 0 then
        y = y + M_2_PI
    end

    FD.att.roll  = r * RAD2DEG
    FD.att.pitch = p * RAD2DEG
    FD.att.yaw   = y * RAD2DEG
end

function calculate_attitude_raw()
    r, p, y = FD.mat:to_euler()
    y = -y
    if y < 0 then
        y = y + M_2_PI
    end

    FD.att.roll  = r * RAD2DEG
    FD.att.pitch = p * RAD2DEG
    FD.att.yaw   = y * RAD2DEG
end


-- System constants
local is_acc = true

-- sampling period in seconds (shown as 1 ms)
--local deltat 1 / 60.0

-- gyroscope measurement error in rad/s (shown as 5 deg/s)
local gyroMeasError = 0.4 * DEG2RAD

-- compute beta
local beta_set = math.sqrt(3.0 / 4.0) * gyroMeasError
local beta = beta_set

-- Global system variables
local a_x, a_y, a_z; -- accelerometer measurements
local w_x, w_y, w_z; -- gyroscope measurements in rad/s

-- estimated orientation quaternion elements with initial conditions
local SEq_1, SEq_2, SEq_3, SEq_4 = 1.0, 0.0, 0.0, 0.0;

function MadgwickAHRSupdateIMU()
    -- enable fast calibrate
    if FD.menu[3] then
        beta = 0.2
    else
        beta = beta_set
    end

    --[[ X -front (back)
         Y -right (left)
         Z -up    (down) ]]
    local w_x, w_y, w_z = -RAW.gyr.x, -RAW.gyr.z, -RAW.gyr.y
    local a_x, a_y, a_z = -RAW.acc.x, -RAW.acc.z, -RAW.acc.y
    local deltat = RAW.dt

--[[
An effcient orientation ﬁlter for inertial and inertial/magnetic sensor arrays
Sebastian O.H. Madgwick
April 30, 2010
...
IMU ﬁlter implementation optimised in C
]]
--// copypasta begin
    -- Local system variables
    -- vector norm
    local norm;
    -- quaternion derrivative from gyroscopes elements
    local SEqDot_omega_1, SEqDot_omega_2, SEqDot_omega_3, SEqDot_omega_4;
    -- objective function elements
    local f_1, f_2, f_3;
    -- objective function Jacobian elements
    local J_11or24, J_12or23, J_13or22, J_14or21, J_32, J_33;
    -- estimated direction of the gyroscope error
    local SEqHatDot_1, SEqHatDot_2, SEqHatDot_3, SEqHatDot_4;

    -- Axulirary variables to avoid reapeated calcualtions
    local halfSEq_1 = 0.5 * SEq_1;
    local halfSEq_2 = 0.5 * SEq_2;
    local halfSEq_3 = 0.5 * SEq_3;
    local halfSEq_4 = 0.5 * SEq_4;
    local twoSEq_1 = 2.0 * SEq_1;
    local twoSEq_2 = 2.0 * SEq_2;
    local twoSEq_3 = 2.0 * SEq_3;

    -- Normalise the accelerometer measurement
    norm = math.sqrt(a_x * a_x + a_y * a_y + a_z * a_z);
    a_x = a_x / norm;
    a_y = a_y / norm;
    a_z = a_z / norm;

    -- Compute the objective function and Jacobian
    f_1      =       twoSEq_2 * SEq_4 - twoSEq_1 * SEq_3 - a_x;
    f_2      =       twoSEq_1 * SEq_2 + twoSEq_3 * SEq_4 - a_y;
    f_3      = 1.0 - twoSEq_2 * SEq_2 - twoSEq_3 * SEq_3 - a_z;
    J_11or24 = twoSEq_3;        -- J_11 negated in matrix multiplication
    J_12or23 = 2.0 * SEq_4;
    J_13or22 = twoSEq_1;        -- J_12 negated in matrix multiplication
    J_14or21 = twoSEq_2;
    J_32     = 2.0 * J_14or21;  -- negated in matrix multiplication
    J_33     = 2.0 * J_11or24;  -- negated in matrix multiplication

    -- Compute the gradient (matrix multiplication)
    SEqHatDot_1 = J_14or21 * f_2 - J_11or24 * f_1;
    SEqHatDot_2 = J_12or23 * f_1 + J_13or22 * f_2 - J_32     * f_3;
    SEqHatDot_3 = J_12or23 * f_2 - J_33     * f_3 - J_13or22 * f_1;
    SEqHatDot_4 = J_14or21 * f_1 + J_11or24 * f_2;

    -- Normalise the gradient
    norm = math.sqrt(SEqHatDot_1 * SEqHatDot_1 + SEqHatDot_2 * SEqHatDot_2 + SEqHatDot_3 * SEqHatDot_3 + SEqHatDot_4 * SEqHatDot_4);
    SEqHatDot_1 = SEqHatDot_1 / norm;
    SEqHatDot_2 = SEqHatDot_2 / norm;
    SEqHatDot_3 = SEqHatDot_3 / norm;
    SEqHatDot_4 = SEqHatDot_4 / norm;

    -- Compute the quaternion derrivative measured by gyroscopes
    SEqDot_omega_1 = -halfSEq_2 * w_x - halfSEq_3 * w_y - halfSEq_4 * w_z;
    SEqDot_omega_2 =  halfSEq_1 * w_x + halfSEq_3 * w_z - halfSEq_4 * w_y;
    SEqDot_omega_3 =  halfSEq_1 * w_y - halfSEq_2 * w_z + halfSEq_4 * w_x;
    SEqDot_omega_4 =  halfSEq_1 * w_z + halfSEq_2 * w_y - halfSEq_3 * w_x;

    -- Compute then integrate the estimated quaternion derrivative
    SEq_1 = SEq_1 + (SEqDot_omega_1 - (beta * SEqHatDot_1)) * deltat;
    SEq_2 = SEq_2 + (SEqDot_omega_2 - (beta * SEqHatDot_2)) * deltat;
    SEq_3 = SEq_3 + (SEqDot_omega_3 - (beta * SEqHatDot_3)) * deltat;
    SEq_4 = SEq_4 + (SEqDot_omega_4 - (beta * SEqHatDot_4)) * deltat;

    -- Normalise quaternion
    norm = math.sqrt(SEq_1 * SEq_1 + SEq_2 * SEq_2 + SEq_3 * SEq_3 + SEq_4 * SEq_4);
    SEq_1 = SEq_1 / norm;
    SEq_2 = SEq_2 / norm;
    SEq_3 = SEq_3 / norm;
    SEq_4 = SEq_4 / norm;
--\\ copypasta end

    local q0, q1, q2, q3 = SEq_1, -SEq_2, -SEq_3, -SEq_4
    local tx, ty

    ty = 2*(q2*q3 - q0*q1)
    tx = 2*q0*q0 + 2*q3*q3 - 1.0
    r = -math.atan2(ty, tx)

    p = math.asin(2*q1*q3 + 2*q0*q2)

    ty = 2*(q1*q2 - q0*q3)
    tx = 2*q0*q0 + 2*q1*q1 - 1.0
    y = -math.atan2(ty, tx)

    y = -y
    if y < 0 then
        y = y + M_2_PI
    end

    FD.att.roll  = r * RAD2DEG
    FD.att.pitch = p * RAD2DEG
    FD.att.yaw   = y * RAD2DEG
end

