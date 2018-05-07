/*****************************************************************************
 * @file
 * @author   Sergey Khabarov
 * @brief    Main entry function for real Firmware. 
 * @details  This file matches to linker symbol '.text.startup' and will be
*            assigned to default entry point 0x10000000. See linker script.
 * @warning  DO NOT ADD NEW METHODS INTO THIS FILE
 ****************************************************************************/

extern void helloWorld();

int main() {
    helloWorld();
    return 0;
}

