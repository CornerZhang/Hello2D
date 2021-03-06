/*!
 * Hello2D Game Engine
 * yoyo 2015 ShenZhen China
 * repo:https://github.com/play175/Hello2D
 * website:http://yoyo.play175.com
 * MIT Licensed
 */
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include "h2d.h"
#include "blit.h"
#include "dirty.h"

#ifndef _GAME_H
#define _GAME_H

#define BLEND_NONE 0
#define BLEND_NORMAL 1
#define BLEND_ADD 2
#define BLEND_LIGHTEN 3
#define BLEND_OVERLY 4

struct game {
	void *bits;
	int w,h;
	bool running;
	bool singleton;//是否只允许运行一个实例
	bool alpha;//是否透明窗口
	bool noborder;//是否无窗口边框
	bool mousedown;//鼠标是否按下
	uint32_t now;//当前毫秒数（启动后）
	float delta;//update实际间隔：秒
	float fixeddelta;//update固定间隔：秒
	uint32_t frame;//总帧数
	uint32_t bgcolor;
	int mousex,mousey;
};

struct game *G;

void game_init();
void game_free();
bool game_create(int argc, char **argv);
void game_destroy();

void game_update();
void game_frame();
void game_touch(int x, int y,int touch);
void game_wheel(int delta,int x,int y);
void game_resizing(int neww, int newh, int oldw, int oldh);
void game_resized(int w,int h);
void game_dropfile(char *file,int index,int total);
void game_keydown(int key, bool ctrl, bool alt, bool shift,short repeat);
void game_keyup(int key, bool ctrl, bool alt, bool shift,short repeat);


static inline void game_draw(void *bits,int bits_width,int bits_height \
               ,int sx,int sy,int width,int height,int tx,int ty) {

	if (bits_width <= 0)return;

	uint32_t *src = (uint32_t *)(bits);
	uint32_t *dest = (uint32_t *)G->bits;
	int blend_width = width,blend_height = height;
	if (tx<0) {
		blend_width += tx;
		sx += -tx;
		tx = 0;
	}
	if (ty<0) {
		blend_height += ty;
		sy += -ty;
		ty = 0;
	}
	if (blend_width + tx > G->w)blend_width = G->w - tx;
	if (blend_height + ty > G->h)blend_height = G->h - ty;

	src += (sx + sy * bits_width);
	dest += (tx + ty * G->w);

	if (blend_width<=0 || blend_height<=0)return;
	alphablend32(dest, G->w, src, bits_width, blend_width, blend_height);
}


static inline void game_fill(uint32_t color) {
	int count = G->h * G->w;
	memset32(G->bits,color,count);//读取color的透明度
}

static inline void game_fillcolor(uint32_t color) {
	int count = G->h * G->w;
	memset32(G->bits,0xff<<24 | color,count);
}

static inline void game_fillrect(void *dest, int pitch, int width, int height, uint32_t data) {
	pitch *= 4;
	while(height--) {
		asm volatile(
			"cld;"
			"rep stosl;"//从eax保存四字节到edi,直到ecx为0
			:
			:"m"(dest), "m"(dest), "D"(dest), "a"(data), "c"(width)
		);
		dest += pitch;
	}
}

static inline void game_blend(uint8_t blendmode,int dx,int dy,uint32_t *dest,int dest_pitch,int dest_width,int dest_height
                              , uint32_t *src,int src_pitch,int src_width,int src_height, struct mat *mat) {
	switch (blendmode) {
	case BLEND_ADD:
		transform_blend_add(dx,dy,dest,dest_pitch,dest_width,dest_height,src,src_pitch,src_width,src_height, mat);
		break;
	case BLEND_OVERLY:
		transform_blend_overly(dx,dy,dest,dest_pitch,dest_width,dest_height,src,src_pitch,src_width,src_height, mat);
		break;
	case BLEND_LIGHTEN:
		transform_blend_lighten(dx,dy,dest,dest_pitch,dest_width,dest_height,src,src_pitch,src_width,src_height, mat);
		break;
	case BLEND_NONE:
		transform_blend_replace(dx,dy,dest,dest_pitch,dest_width,dest_height,src,src_pitch,src_width,src_height, mat);
		break;
	case BLEND_NORMAL:
	default:
		transform_blend(dx,dy,dest,dest_pitch,dest_width,dest_height,src,src_pitch,src_width,src_height, mat);
		break;
	}
}

#define _GAME_FPS

#ifdef _GAME_FPS

// 64×8的数字符号图像
// ·每个数字的尺寸为6×8
// ·像素存储顺序为自上而下
// ·带alpha通道
static uint32_t _bmp_num[64*8] = {
	0x00ffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
	0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
	0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
	0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
	0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
	0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
	0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
	0x00ffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
};

static uint32_t _bmp_fps[18*8] = {
	0xFFffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff,0xFFffffff,0x00ffffff,
	0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,0x00ffffff,0xFFffffff,
	0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,0x00ffffff,0x00ffffff,
	0xFFffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff,0x00ffffff,0x00ffffff,
	0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff,0xFFffffff,0x00ffffff,
	0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,0x00ffffff,0xFFffffff,
	0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,0x00ffffff,0xFFffffff,
	0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xFFffffff, 0xFFffffff, 0xFFffffff,0xFFffffff,0x00ffffff,
};

static inline void game_drawnum(int num,int x,int y) {
	int val = num;
	int offset_x = x;

	do {
		offset_x += 6;
		val /= 10;
	} while (val > 0);

	do {
		val = num % 10;
		num /= 10;

		game_draw(_bmp_num,64,8,val*6,0,6,8,offset_x,y);

		offset_x -= 6;
	} while (num > 0);
}

static inline void game_showfps() {
	static uint32_t last_time = 0;
	static uint32_t frames = 0;
	static uint32_t fps = 60;

	uint32_t now = clock();
	if (now - last_time >= 500) {
		fps = (2 * frames * (now - last_time)) / 500;
		frames = 0;
		last_time += 500;
	} else {
		frames++;
	}

	int x = 5,y = 7;
	game_draw(_bmp_fps,18,8,6 * 0,0,6,8,x + 8 *0,y);
	game_draw(_bmp_fps,18,8,6 * 1,0,6,8,x + 8 *1,y);
	game_draw(_bmp_fps,18,8,6 * 2,0,6,8,x + 8 *2,y);
	game_drawnum(fps,x + 8 * 3 - 3,y);

	dirty_mark(x,y,60,18);
}

#endif //_GAME_FPS

#endif
