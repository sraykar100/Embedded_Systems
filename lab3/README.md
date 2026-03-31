Useful macros:

If editing the hardware interface, make sure to insert these in the .tcl file
set_module_assignment embeddedsw.dts.vendor "csee4840"
set_module_assignment embeddedsw.dts.name "vga_ball"
set_module_assignment embeddedsw.dts.vendor "vga"

If editing just the .sv file, we still have to run

1. make qsys-clean
2. make qsys
3. make rbf
4. ./embedded_command_shell.sh and make dtb

For both edits, we then must move the rbf (and dtb to be safe) file onto the mount of the SD card. Remount with mount /dev/mmcblk0p1 /mnt on the FPGA.

Also, if working from mac (due to file name sensitivity), use this command to combat non-existent changes in git.
cd /Users/marcuslam/Desktop/School/Y3/S2/ES/Embedded_Systems && git update-index --assume-unchanged lab3/linux-headers-4.19.0/ip6t_HL.h lab3/linux-headers-4.19.0/ipt_ECN.h lab3/linux-headers-4.19.0/ipt_TTL.h lab3/linux-headers-4.19.0/xt_CONNMARK.h lab3/linux-headers-4.19.0/xt_DSCP.h lab3/linux-headers-4.19.0/xt_MARK.h lab3/linux-headers-4.19.0/xt_RATEEST.h lab3/linux-headers-4.19.0/xt_TCPMSS.h

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
