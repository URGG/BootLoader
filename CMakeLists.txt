cmake_minimum_required(VERSION 3.20)
project(simple_os ASM_NASM C)

# Enable NASM
enable_language(ASM_NASM)

# Set C compiler flags for freestanding environment
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -ffreestanding -nostdlib -fno-pie -fno-stack-protector -O2 -Wall")

# Set NASM flags
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf32)
set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> <FLAGS> -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} -o <OBJECT> <SOURCE>")

# Bootloader (raw binary)
add_custom_command(
        OUTPUT boot.bin
        COMMAND nasm -f bin ${CMAKE_SOURCE_DIR}/boot.asm -o boot.bin
        DEPENDS BL/boot.asm
        COMMENT "Building bootloader..."
)

# Kernel entry assembly
add_library(kernel_entry OBJECT kernel_entry.asm
        BL/kernel.c)

# Kernel C code
add_library(kernel_lib OBJECT BL/kernel.c)

# Link kernel
add_custom_command(
        OUTPUT kernel.bin
        COMMAND ld -m elf_i386 -T ${CMAKE_SOURCE_DIR}/linker.ld
        $<TARGET_OBJECTS:kernel_entry>
        $<TARGET_OBJECTS:kernel_lib>
        -o kernel.bin
        DEPENDS kernel_entry kernel_lib linker.ld
        COMMENT "Linking kernel..."
)

# Create OS image
add_custom_command(
        OUTPUT os.img
        COMMAND cat boot.bin kernel.bin > os.img
        COMMAND truncate -s 1440K os.img
        DEPENDS boot.bin kernel.bin
        COMMENT "Creating OS image..."
)

# Main target
add_custom_target(os_image ALL DEPENDS os.img)

# Run target
add_custom_target(run
        COMMAND qemu-system-i386 -fda os.img
        DEPENDS os_image
        COMMENT "Running OS in QEMU..."
)

# Clean extra files
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES
        "boot.bin;kernel.bin;os.img")