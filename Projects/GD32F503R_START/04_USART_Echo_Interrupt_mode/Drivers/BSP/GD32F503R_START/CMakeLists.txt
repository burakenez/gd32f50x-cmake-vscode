project(GD32F503R_START LANGUAGES C CXX ASM)

add_library(GD32F503R_START OBJECT
    ${DRIVERS_DIR}/BSP/GD32F503R_START/gd32f503r_start.c
    )

target_include_directories(GD32F503R_START PUBLIC
    ${DRIVERS_DIR}/BSP/GD32F503R_START
    )

target_link_libraries(GD32F503R_START PUBLIC GD32F50x_standard_peripheral)
