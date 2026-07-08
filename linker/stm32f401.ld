MEMORY 
{
    flash   (RX) : ORIGIN = 0x08000000, LENGTH = 64K
    sram    (RW) : ORIGIN = 0x20000000, LENGTH = 64K
}

ENTRY(Reset_Handler)

_min_stack_size = 0x800;

SECTIONS
{
    .isr_vector : ALIGN(4) 
    {
        _vector_start = .;
        KEEP(*(.isr_vector))
        . = ALIGN(4);
        _vector_end = .;
    } > flash

    .text : ALIGN(4)
    {
        _text_start = .;
        *(.text)
        *(.text*)
        _text_ro_start = .;
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
        _text_end = .;
    } > flash

    .data : ALIGN(4)
    {
        _data_start = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _data_end = .;
    } > sram AT > flash

    _data_flash_start = LOADADDR(.data);

    .bss : ALIGN(4) 
    {
        _bss_start = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _bss_end = .;
    } > sram

    end = .;

    ._stack : ALIGN(8) 
    {
        _stack_start = .;
        . = . + _min_stack_size;
        . = ALIGN(8);
        _stack_top = .;
    } > sram
}