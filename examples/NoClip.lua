script_name("No Clip")
script_author("Junior_Djjr (Converted to MoonLoader III)")
script_description("Fly through Liberty City. Type NOC to toggle.")
script_version("1.0")

-- Config
local SPEED = 0.75
local SPEED_ADD = 0.15
local CHEAT_CODE = "NOC"
local SHOW_INFOS = true

-- Key codes
local VK_W = 87
local VK_S = 83
local VK_A = 65
local VK_D = 68
local VK_Q = 81
local VK_E = 69
local VK_SHIFT = 16
local VK_PRIOR = 33  -- PageUp
local VK_NEXT = 34   -- PageDown

local noClipActive = false
local playerPed = 0
local playerCar = 0
local posX, posY, posZ = 0, 0, 0

while true do
    wait(0)
    playerCar = 0

    if test_cheat(CHEAT_CODE) then
        if noClipActive then
            -- Deactivate
            noClipActive = false

            if playerCar ~= 0 then
                if does_car_exist(playerCar) then
                    set_car_collision(playerCar, true)
                end
            else
                if playerPed ~= 0 then
                    set_ped_collision(playerPed, true)
                end
            end

            set_player_can_enter_exit_vehicles(0, true)
            print_string("No Clip: OFF", 2000)
        else
            -- Activate
            if not is_player_playing(0) then
                print_string("No Clip: Player not found", 2000)
            else
                playerPed = get_player_ped(0)

                if is_char_in_any_car(playerPed) then
                    playerCar = get_player_car(0)
                    if playerCar ~= 0 then
                        set_car_collision(playerCar, false)
                    end
                else
                    playerCar = 0
                    set_ped_collision(playerPed, false)
                end

                posX, posY, posZ = get_player_coords(0)
                set_player_can_enter_exit_vehicles(0, false)

                noClipActive = true
                print_string("No Clip: ON", 2000)
            end
        end
    end

    if not noClipActive then
        -- Reset car reference when not in no-clip
        playerCar = 0
    else
        -- No-Clip loop
        local dt = get_frame_delta_time()
        local speedDt = SPEED * dt

        -- Shift = 5x speed
        if is_key_pressed(VK_SHIFT) then
            speedDt = speedDt * 5.0
        end

        -- Get camera direction
        local camX, camY, camZ = get_camera_position()
        local targetX, targetY, targetZ = get_camera_target()

                -- Get camera basis vectors
        local dx = targetX - camX
        local dy = targetY - camY
        local dz = targetZ - camZ
        
        -- Normalize forward
        local len = math.sqrt(dx*dx + dy*dy + dz*dz)
        if len > 0.001 then
            dx = dx / len
            dy = dy / len
            dz = dz / len
        end
        
        -- Right vector = cross(front, world_up)
        local rx = dy
        local ry = -dx
        local rz = 0
        local rlen = math.sqrt(rx*rx + ry*ry)
        if rlen > 0.001 then
            rx = rx / rlen
            ry = ry / rlen
        end

        -- Movement
        if is_key_pressed(VK_W) then
            posX = posX + dx * speedDt
            posY = posY + dy * speedDt
            posZ = posZ + dz * speedDt
        end
        if is_key_pressed(VK_S) then
            posX = posX - dx * speedDt
            posY = posY - dy * speedDt
            posZ = posZ - dz * speedDt
        end
        if is_key_pressed(VK_A) then
            posX = posX - rx * speedDt
            posY = posY - ry * speedDt
        end
        if is_key_pressed(VK_D) then
            posX = posX + rx * speedDt
            posY = posY + ry * speedDt
        end
        if is_key_pressed(VK_Q) then
            posZ = posZ - speedDt
        end
        if is_key_pressed(VK_E) then
            posZ = posZ + speedDt
        end

        -- Speed adjustment (PageUp / PageDown)
        if is_key_pressed(VK_PRIOR) then
            SPEED = SPEED + SPEED_ADD
            if is_key_pressed(VK_SHIFT) then
                SPEED = SPEED + SPEED_ADD * 4.0
            end
        end

        if is_key_pressed(VK_NEXT) then
            local dec = SPEED_ADD
            if is_key_pressed(VK_SHIFT) then
                dec = dec * 5.0
            end
            SPEED = SPEED - dec
            if SPEED < SPEED_ADD then
                SPEED = SPEED_ADD
            end
        end

        -- Apply position
        if playerCar ~= 0 then
            if does_car_exist(playerCar) then
                set_car_coords(playerCar, posX, posY, posZ)
            else
                -- Car was destroyed, switch to on-foot mode
                playerCar = 0
                set_ped_collision(playerPed, false)
            end
        else
            set_player_coords(0, posX, posY, posZ)
        end

        -- Show info
        if SHOW_INFOS then
            print_string(string.format("No Clip | Speed: %.2f | %.2f %.2f %.2f", SPEED, posX, posY, posZ), 100)
        end
    end
end