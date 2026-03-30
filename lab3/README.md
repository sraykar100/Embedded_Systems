Useful macros:

If editing the hardware interface, make sure to insert these in the .tcl file
set_module_assignment embeddedsw.dts.vendor "csee4840"
set_module_assignment embeddedsw.dts.name "vga_ball"
set_module_assignment embeddedsw.dts.vendor "vga"

If editing just the .sv file, we still have to run make qsys-clean and make qsys

For both edits, we then must move the rbf (and dtb to be safe) file onto the mount of the SD card.

Our call chain:
USERSPACE KERNEL HARDWARE
───────── ────── ────────

ioctl(fd, VGA_BALL_SET_POS, &vla)  
 │  
 ▼ (system call trap: CPU switches from user mode to kernel mode)

                    vga_ball_ioctl(f, cmd, arg)
                        │
                        │ arg is a raw number -- it's the
                        │ userspace pointer &vla, but we
                        │ can't dereference it directly
                        ▼
                    copy_from_user(&kernel_vla, arg, size)
                        │
                        │ Safely copies bytes from user
                        │ address space to kernel stack
                        ▼
                    iowrite32(x, dev.virtbase + 4)
                        │
                        ▼ (memory-mapped I/O write on AXI bus)

                                                    FPGA latches
                                                    ball_x = x
