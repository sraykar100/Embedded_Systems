Useful macros:

If editing the hardware interface, make sure to insert these in the .tcl file
set_module_assignment embeddedsw.dts.vendor "csee4840"
set_module_assignment embeddedsw.dts.name "vga_ball"
set_module_assignment embeddedsw.dts.vendor "vga"

If editing just the .sv file, we still have to run make qsys-clean and make qsys

For both edits, we then must move the rbf (and dtb to be safe)  file onto the mount of the SD card. 
