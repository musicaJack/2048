idf_component_register(
    SRCS
        "${CMAKE_CURRENT_LIST_DIR}/../../../src/game/game_logic.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/../../../src/game/game_state.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/../../../src/game/game_2048.cpp"
    INCLUDE_DIRS
        "${CMAKE_CURRENT_LIST_DIR}/../../../include"
    REQUIRES esp_timer
)
