# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initalize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
#set(PICO_SDK_PATH "/usr/share/pico-sdk")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico-composite-video C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
add_executable(pico-composite-video)

pico_generate_pio_header(${PROJECT_NAME}  ${CMAKE_CURRENT_LIST_DIR}/cvideo.pio)
pico_generate_pio_header(${PROJECT_NAME}  ${CMAKE_CURRENT_LIST_DIR}/audio.pio)

target_sources(pico-composite-video PRIVATE cvideotest.c cvideo.c)


pico_set_program_name(pico-composite-video "pico-composite-video")
pico_set_program_version(pico-composite-video "0.1")

pico_enable_stdio_usb(pico-composite-video 1)
pico_enable_stdio_uart(pico-composite-video 0)

# Add the standard library to the build
target_link_libraries(pico-composite-video pico_stdlib)

# Add any user requested libraries
target_link_libraries(pico-composite-video
        hardware_dma
        hardware_pio
        hardware_timer
        hardware_clocks
        )
target_compile_options(pico-composite-video PUBLIC
  "-DUSE_DMA"
  )

target_include_directories(pico-composite-video PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/../inc
        ${CMAKE_CURRENT_LIST_DIR}/../cpu-z80
        ${CMAKE_CURRENT_LIST_DIR}/../galaksija)

pico_set_binary_type(pico-composite-video no_flash)


pico_add_extra_outputs(pico-composite-video)

##### pico galaksija
# Add executable. Default name is the project name, version 0.1
add_executable(galaksija)

pico_generate_pio_header(galaksija ${CMAKE_CURRENT_LIST_DIR}/cvideo.pio)
pico_generate_pio_header(galaksija ${CMAKE_CURRENT_LIST_DIR}/audio.pio)

target_sources(galaksija PRIVATE cvideo.c ../galaksija/galaksija.c hwpico.c ../cpu-z80/Z80.c ../menu/menu.c)


pico_set_program_name(galaksija "galaksija")
pico_set_program_version(galaksija "0.1")

pico_enable_stdio_usb(galaksija 1)
pico_enable_stdio_uart(galaksija 0)

target_include_directories(galaksija PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/../inc
        ${CMAKE_CURRENT_LIST_DIR}/../cpu-z80
        ${CMAKE_CURRENT_LIST_DIR}/../galaksija)


# Add the standard library to the build
target_link_libraries(galaksija pico_stdlib)

# Add any user requested libraries
target_link_libraries(galaksija
        hardware_dma
        hardware_pio
        hardware_timer
        hardware_clocks
        pico_multicore
        )
target_compile_options(galaksija PUBLIC
  "-DUSE_DMA"
  "-DLSB_FIRST"
  )

pico_set_binary_type(galaksija no_flash)


pico_add_extra_outputs(galaksija)


##### pico galaksija
# Add executable. Default name is the project name, version 0.1
add_executable(galaksijausb)

pico_generate_pio_header(galaksijausb ${CMAKE_CURRENT_LIST_DIR}/cvideo.pio)
pico_generate_pio_header(galaksijausb ${CMAKE_CURRENT_LIST_DIR}/audio.pio)

target_sources(galaksijausb PRIVATE 
	cvideo.c
	../galaksija/galaksija.c
	hwpico.c
	../menu/menu.c
	../cpu-z80/Z80.c
	../usb/hid_app.c)


pico_set_program_name(galaksijausb "galaksijausb")
pico_set_program_version(galaksijausb "0.1")

pico_enable_stdio_usb(galaksijausb 0)
pico_enable_stdio_uart(galaksijausb 0)

target_include_directories(galaksijausb PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/../inc
        ${CMAKE_CURRENT_LIST_DIR}/../cpu-z80
        ${CMAKE_CURRENT_LIST_DIR}/../galaksija
        ${CMAKE_CURRENT_LIST_DIR}/../usb)


# Add the standard library to the build
target_link_libraries(galaksijausb pico_stdlib)

# Add any user requested libraries
target_link_libraries(galaksijausb
        hardware_dma
        hardware_pio
        hardware_timer
        hardware_clocks
        pico_multicore
	tinyusb_host
	tinyusb_board
        )
target_compile_options(galaksijausb PUBLIC
  "-DUSE_DMA"
  "-DLSB_FIRST"
  "-DUSB"
  )

#pico_set_binary_type(galaksijausb no_flash)


pico_add_extra_outputs(galaksijausb)


