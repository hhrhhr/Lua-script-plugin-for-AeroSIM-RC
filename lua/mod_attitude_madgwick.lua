--[[
An effcient orientation ﬁlter for inertial and inertial/magnetic sensor arrays
Sebastian O.H. Madgwick
]]

-- gyroscope measurement error in rad/s (shown as 0.5 deg/s)
local gyroMeasError = 0.5 * DEG2RAD

-- compute beta
local beta_set = math.sqrt(3.0 / 4.0) * gyroMeasError
local beta = beta_set

local a = Vector3D() -- accelerometer measurements
local w = Vector3D() -- gyroscope measurements in rad/s

-- estimated orientation quaternion elements with initial conditions
local q = Quaternion()

-- quaternion derrivative from gyroscopes elements
local qDot = Quaternion()

-- objective function elements
local f1, f2, f3;

-- estimated direction of the gyroscope error
local qHat = Quaternion()

-- Axulirary variables to avoid reapeated calcualtions
local halfq = Quaternion()
local twoq = Quaternion()


function attitude_MadgwickAHRSupdateIMU()
    -- enable fast calibrate
    if FD.menu[3] then
        beta = 0.2
    else
        beta = beta_set
    end

    w = Vector3D(-RAW.gyr.x, -RAW.gyr.z, -RAW.gyr.y)
    a = Vector3D(-RAW.acc.x, -RAW.acc.z, -RAW.acc.y)
    a:norm()

    halfq = q:mul(0.5);
    twoq = q:mul(2.0);

    -- Compute the objective function and Jacobian
    f1 =       twoq.x * q.z - twoq.w * q.y - a.x;
    f2 =       twoq.w * q.x + twoq.y * q.z - a.y;
    f3 = 1.0 - twoq.x * q.x - twoq.y * q.y - a.z;

    -- Compute the gradient (matrix multiplication)
    qHat.x = twoq.z * f1 + twoq.w * f2 - 2.0 * twoq.x * f3
    qHat.y = twoq.z * f2 - twoq.w * f1 - 2.0 * twoq.y * f3
    qHat.z = twoq.x * f1 + twoq.y * f2
    qHat.w = twoq.x * f2 - twoq.y * f1
    qHat:norm()

    -- Compute the quaternion derrivative measured by gyroscopes
    qDot = halfq * Quaternion(w.x, w.y, w.z, 0.0)

    -- Compute then integrate the estimated quaternion derrivative
    q = q + (qDot - qHat:mul(beta)):mul(RAW.dt)
    q:norm()
--\\ copypasta end

    local q0, q1, q2, q3 = q.w, -q.x, -q.y, -q.z
    local tx, ty

    ty = 2*(q2*q3 - q0*q1)
    tx = 2*(q0*q0 + q3*q3) - 1.0
    r = -math.atan2(ty, tx)

    p = math.asin(2*(q1*q3 + q0*q2))

    ty = 2*(q1*q2 - q0*q3)
    tx = 2*(q0*q0 + q1*q1) - 1.0
    y = -math.atan2(ty, tx)


    y = -y
    if y < 0 then
        y = y + M_2_PI
    end

    FD.att.roll  = r * RAD2DEG
    FD.att.pitch = p * RAD2DEG
    FD.att.yaw   = y * RAD2DEG
end

