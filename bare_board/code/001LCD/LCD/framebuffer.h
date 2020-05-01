#ifndef _FRAMEBUFFER_H
#define _FRAMEBUFFER_H
unsigned short convert32bppto16bpp(unsigned int rgb);
void fb_put_pixel(int x,int y,unsigned int color);


#endif

