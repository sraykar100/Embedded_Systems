/*
 * Userspace program that communicates with the vga_ball device driver
 * through ioctls
 *
 * Stephen A. Edwards
 * Columbia University
 */

#include <stdio.h>
#include "vga_ball.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

int vga_ball_fd;

void set_background_color(const vga_ball_color_t *c)
{
  vga_ball_arg_t vla;
  vla.background = *c;
  if (ioctl(vga_ball_fd, VGA_BALL_WRITE_BACKGROUND, &vla)) {
      perror("ioctl(VGA_BALL_WRITE_BACKGROUND) failed");
      return;
  }
}

void set_ball_pos(unsigned short x, unsigned short y)
{
  vga_ball_arg_t vla;
  vla.ball_x = x;
  vla.ball_y = y;
  if (ioctl(vga_ball_fd, VGA_BALL_SET_POS, &vla)) {
      perror("ioctl(VGA_BALL_SET_POS) failed");
      return;
  }
}

int main()
{
  static const char filename[] = "/dev/vga_ball";
  vga_ball_color_t bg = { 0x00, 0x00, 0x40 };

  printf("VGA ball Userspace program started\n");

  if ((vga_ball_fd = open(filename, O_RDWR)) == -1) {
    fprintf(stderr, "could not open %s\n", filename);
    return -1;
  }

  set_background_color(&bg);
  unsigned short x = 320, y = 240;
  int dx = 2, dy = 2;
  for (int i = 0; i < 100; i++) {
    set_ball_pos(x, y);
    x += dx;
    y += dy;
    if (x <= 0 || x >= 640) dx = -dx;
    if (y <= 0 || y >= 480) dy = -dy;
    usleep(16000); // sleep for 16 ms (about 60 frames per second)
  }

  printf("Ball placed at (320, 240)\n");
  printf("VGA BALL Userspace program terminating\n");
  return 0;
}
