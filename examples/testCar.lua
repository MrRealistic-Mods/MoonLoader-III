while true do
    wait(0)

    if test_cheat("TESTLUA") then

        local px, py, pz = get_player_coords(0)
        local carModel = 101

        request_model(carModel)

        --this returns false for now
        --if has_model_loaded(carModel) then 
            local car = create_car(carModel, px, py + 4.0, pz)
            mark_model_as_no_longer_needed(carModel)
        --else
            --print("not loaded")
        --end
    end
end