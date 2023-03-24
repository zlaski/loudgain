	
struct winsize
{
  unsigned short ws_row;	/* rows, in characters */
  unsigned short ws_col;	/* columns, in characters */
  unsigned short ws_xpixel;	/* horizontal size, pixels */
  unsigned short ws_ypixel;	/* vertical size, pixels */
};

#define TIOCGWINSZ 0x5413
#define ioctl(fd, cmd, ...) (-1)
