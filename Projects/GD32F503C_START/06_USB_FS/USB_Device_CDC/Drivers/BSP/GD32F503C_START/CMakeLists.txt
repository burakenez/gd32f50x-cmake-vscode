project(GD32F503C_START LANGUAGES C CXX ASM)

add_library(GD32F503C_START OBJECT
    ${DRIVERS_DIR}/BSP/GD32F503C_START/gd32f503c_start.c
    )

target_include_directories(GD32F503C_START PUBLIC
    ${DRIVERS_DIR}/BSP/GD32F503C_START
    )

target_link_libraries(GD32F503C_START PUBLIC GD32F50x_standard_peripheral)
