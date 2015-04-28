/*!
 * Hello2D Game Engine
 * yoyo 2015 ShenZhen China
 * repo:https://github.com/play175/Hello2D
 * website:http://yoyo.play175.com
 * MIT Licensed
 */
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "blit.h"
#include "math2d.h"

#define max(a,b) ( ((a)>(b)) ? (a):(b) )
#define min(a,b) ( ((a)>(b)) ? (b):(a) )


//���ֻ��λ�ƣ�û�����ź���ת�������MMXָ���Ż�
#define TRANSFORM_MMX(BLEND_FUNCTION) \
	int x1,y1,x2,y2; \
	int x,y; \
	int ox,oy; \
	float ofx,ofy; \
	if(m->a == 1.0f && m->b == 0.0f && m->a == m->d && m->b == m->c) { \
		x1 = m->tx + dx;y1 = m->ty + dy; \
		if(x1 >=  dest_width || y1 >=  dest_height) return; \
		ox = 0;oy = 0; \
		x2 = src_width;y2 = src_height; \
		if(x1<0){ox -= x1;x2 += x1;x1 = 0;} \
		if(y1<0){oy -= y1;y2 += y1;y1 = 0;} \
		if(x1 + x2 >  dest_width) x2 = dest_width - x1; \
		if(y1 + y2 >  dest_height) y2 = dest_height - y1; \
		if(x2 <= 0 || y2 <= 0)return; \
		alphablend32(dest + y1 * dest_pitch + x1, dest_pitch \
		, src + oy * src_pitch + ox, src_pitch, x2, y2); \
		return; \
	} \
	float fx,fy; \
	struct mat mm = *m;\
	m = &mm; \
	mat_translate(m,dx,dy); \
	mat_transform_size(m,src_width, src_height, &x1, &y1, &x2, &y2); \
	if(x1<0)x1 = 0;if(x1 > dest_width)x1 = dest_width; \
	if(y1<0)y1 = 0;if(y1 > dest_height)y1 = dest_height; \
	if(x2<0)x2 = 0;if(x2 > dest_width)x2 = dest_width; \
	if(y2<0)y2 = 0;if(y2 > dest_height)y2 = dest_height; \
	if(!(x1<x2 && y1<y2))return; \
	mat_invert(m); \
	int dst_stride = dest_pitch - (x2 - x1); \
	uint32_t *dst_pix = dest + y1 * dest_pitch + x1; \
	mat_transform(m,x1,y1,&fx,&fy); \
	for(y = y1;y<y2;++y,fx += m->c,fy += m->d) { \
		ofx = fx;ofy = fy; \
		for(x = x1;x<x2;++x,ofx += m->a,ofy += m->b) { \
			ox = (int)ofx;\
			oy = (int)ofy;\
			if (oy >= 0 && oy < src_height && ox >= 0 && ox < src_width) { \
				BLEND_FUNCTION(dst_pix,src + oy * src_pitch + ox); \
			} \
			dst_pix++; \
		} \
		dst_pix += dst_stride; \
	}

#define TRANSFORM(BLEND_FUNCTION) \
	int x1,y1,x2,y2; \
	int x,y; \
	int ox,oy; \
	float ofx,ofy; \
	float fx,fy; \
	struct mat mm = *m;\
	m = &mm; \
	mat_translate(m,dx,dy); \
	mat_transform_size(m,src_width, src_height, &x1, &y1, &x2, &y2); \
	if(x1<0)x1 = 0;if(x1 > dest_width)x1 = dest_width; \
	if(y1<0)y1 = 0;if(y1 > dest_height)y1 = dest_height; \
	if(x2<0)x2 = 0;if(x2 > dest_width)x2 = dest_width; \
	if(y2<0)y2 = 0;if(y2 > dest_height)y2 = dest_height; \
	if(!(x1<x2 && y1<y2))return; \
	mat_invert(m); \
	int dst_stride = dest_pitch - (x2 - x1); \
	uint32_t *dst_pix = dest + y1 * dest_pitch + x1; \
	mat_transform(m,x1,y1,&fx,&fy); \
	for(y = y1;y<y2;++y,fx += m->c,fy += m->d) { \
		ofx = fx;ofy = fy; \
		for(x = x1;x<x2;++x,ofx += m->a,ofy += m->b) { \
			ox = (int)ofx;\
			oy = (int)ofy;\
			if (oy >= 0 && oy < src_height && ox >= 0 && ox < src_width) { \
				BLEND_FUNCTION(dst_pix,src + oy * src_pitch + ox); \
			} \
			dst_pix++; \
		} \
		dst_pix += dst_stride; \
	}

void alphablend32(uint32_t *dest_bmp, int dest_pitch, uint32_t *src_bmp, int src_pitch, int blend_width, int blend_height)
{
#ifdef __MINGW32__
        int nextLineOffset_src = (src_pitch - blend_width) * 4;	// �����һ�����غ�ͨ�����ϸ�ֵ�����ֱ�Ӷ�λ��������ʼ����
        int nextLineOffset_dst = (dest_pitch - blend_width) * 4;

        asm volatile(
                "xorl %%edx, %%edx;"//�ѻ�ϵĸ߶�
                "movl %2, %%ecx;"//Ҫ��ϵĿ��

                "BLEND_BEGIN:;"
                "cmpl $0x00FFFFFF, (%%esi);"//; ���alphaΪ0,��������ϲ���
                "jna BLEND_END;"//С�ڵ���

                "movd (%%edi), %%mm0;"//		; ��Ŀ������ֵ����mm0�Ĵ����ĵ�32λ
                "movd (%%esi), %%mm1;"//; ��Դ����ֵ����mm1�Ĵ����ĵ�32λ

                //Core Begin
                "pxor %%mm2, %%mm2;" //; ��MM2��0
                "punpcklbw %%mm2, %%mm0;"  //			; src:8 bit��16 bit�����ɽ����32bit expand to 64 bit
                "punpcklbw %%mm2, %%mm1;"  //			; dst:8 bit��16 bit�����ɽ��.32bit expand to 64 bit
                "movq %%mm1, %%mm3;"  //			; ��ΪҪ��dst��Alphaֵ
                "punpckhwd	 %%mm3, %%mm3;"  //			; �����ƶ���˫��
                "punpckhdq	%%mm3, %%mm3;"  //			; ˫���ƶ������֣������а˸����ص�Alpha��!
                "movq %%mm0, %%mm4;"  //			; mm4 = dst
                "movq %%mm1, %% mm5;"  //			; mm5 = src
                "psubusw %%mm1, %%mm4;"  //			; dst-src�����ͼ���С��0Ϊ0
                "psubusw %%mm0, %%mm5;"  //			; src-dst�����ͼ���С��0Ϊ0
                "pmullw %%mm3, %% mm4;"  //			; Alpha * (src-dst)
                "pmullw	 %%mm3, %%mm5;"  //			; Alpha * (dst-src)
                "psrlw $8, %%mm4;"  //				; ����256��now mm4 get the result��(src-dst)<0 ����
                "psrlw $8, %%mm5;"  //				; ����256��now mm5 get the result��(dst-src)>0 ����
                "paddusw %%mm5, %%mm0;"  //			; ���ͼӵ�ԭͼ��:D=Alpha*(O-S)+S��(src-dst)<0 ����
                "psubusw %%mm4, %% mm0;"  //			; ���ͼӵ�ԭͼ��D=S-Alpha*(S-O)��(dst-src)>0 ����
                "packuswb %%mm0, %%mm0;"  //			; ��������32bit
                //Core End

                "movd %%mm0, (%%edi) ;"  //			; ��Ͻ��д��Ŀ������

                "BLEND_END:;"
                "addl $4, %%edi;"
                "addl $4, %%esi;"
                "loop BLEND_BEGIN;"//; ѭ��

                "addl %4, %%esi;"//; ����ƫ������ʹ��λ��������ʼ��
                "addl %5, %%edi;"

                "incl %%edx;"
                "movl %2, %%ecx;"

                "cmpl %3, %%edx;"//��edxС��blend_height,��ת�Ƶ�����������
                "jb BLEND_BEGIN;"

                "EMMS;" //��Ϊ��mm0��mm7,��Щ�Ĵ����ǡ����á�����Ĵ����ĵ�64λ,����ÿ��������MMXָ���һ��Ҫ��EMMSָ��Ĵ������

        :
        :"S"(src_bmp), "D"(dest_bmp), "m"(blend_width), "m"(blend_height), "m"(nextLineOffset_src), "m"(nextLineOffset_dst)
        );
#else

        // Cʵ��
        int nextLineOffset_src = (src_pitch - blend_width);
        int nextLineOffset_dst = (dest_pitch - blend_width);
        int h,w;

        uint8_t a1,r1,g1,b1;
        uint8_t r2,g2,b2;

        for (h=0, w=0; h<blend_height; h++) {
                for (w=0; w<blend_width; w++) {
                        {
                                b1 = (*src_bmp) & 0x000000ff;
                                g1 = ((*src_bmp) & 0x0000ff00)>>8;
                                r1 = ((*src_bmp) & 0x00ff0000)>>16;
                                a1 = ((*src_bmp) & 0xff000000)>>24;

                                b2 = (*dest_bmp) & 0x000000ff;
                                g2 = ((*dest_bmp) & 0x0000ff00)>>8;
                                r2 = ((*dest_bmp) & 0x00ff0000)>>16;
                                //>>8�ǽ���ֵ��ʵ��Ӧ���ǳ���255
                                *dest_bmp = ((a1 * (r1 - r2) >> 8) + r2) << 16 | ((a1 * (g1 - g2) >> 8) + g2) << 8 | ((a1 * (b1 - b2) >> 8) + b2) << 0 ;
                        }

                        src_bmp++;
                        dest_bmp++;
                }

                src_bmp += nextLineOffset_src;
                dest_bmp += nextLineOffset_dst;
        }
#endif
}

static inline void alpha_blend_replace(uint32_t *dest_bmp,uint32_t *src_bmp)
{
        *dest_bmp = *src_bmp;
}

//#define RMask 0x00ff0000
//#define GMask 0x0000ff00
//#define BMask 0x000000ff

static inline void alpha_blend(uint32_t *dest_bmp,uint32_t *src_bmp)
{
	uint8_t a1,r1,g1,b1;
	uint8_t r2,g2,b2;
	a1 = ((*src_bmp) & 0xff000000)>>24;
	if (a1 == 255) {
			*dest_bmp = *src_bmp;
			return;
	}
	if (a1 == 0) {
			return;
	}
	b1 = (*src_bmp) & 0x000000ff;
	g1 = ((*src_bmp) & 0x0000ff00)>>8;
	r1 = ((*src_bmp) & 0x00ff0000)>>16;

	b2 = (*dest_bmp) & 0x000000ff;
	g2 = ((*dest_bmp) & 0x0000ff00)>>8;
	r2 = ((*dest_bmp) & 0x00ff0000)>>16;

	*dest_bmp = a1 << 24 | ((a1 * (r1 - r2) >> 8) + r2) << 16 | ((a1 * (g1 - g2) >> 8) + g2) << 8 | ((a1 * (b1 - b2) >> 8) + b2);

}

static inline void alpha_blend_add(uint32_t *dest_bmp,uint32_t *src_bmp)
{
	uint8_t a1,r1,g1,b1;
	uint8_t r2,g2,b2;
	b1 = (*src_bmp) & 0x000000ff;
	g1 = ((*src_bmp) & 0x0000ff00)>>8;
	r1 = ((*src_bmp) & 0x00ff0000)>>16;
	a1 = ((*src_bmp) & 0xff000000)>>24;

	b2 = (*dest_bmp) & 0x000000ff;
	g2 = ((*dest_bmp) & 0x0000ff00)>>8;
	r2 = ((*dest_bmp) & 0x00ff0000)>>16;

	b2 = min(255, max(0, (b2 + 2 * b1) - 1));
	g2 = min(255, max(0, (g2 + 2 * g1) - 1));
	r2 = min(255, max(0, (r2 + 2 * r1) - 1));

	*dest_bmp = a1 << 24 | r2 << 16 | g2 << 8 | b2;
}

static inline void alpha_blend_overly(uint32_t *dest_bmp,uint32_t *src_bmp)
{
	uint8_t a1,r1,g1,b1;
	uint8_t r2,g2,b2;
	b1 = (*src_bmp) & 0x000000ff;
	g1 = ((*src_bmp) & 0x0000ff00)>>8;
	r1 = ((*src_bmp) & 0x00ff0000)>>16;
	a1 = ((*src_bmp) & 0xff000000)>>24;

	b2 = (*dest_bmp) & 0x000000ff;
	g2 = ((*dest_bmp) & 0x0000ff00)>>8;
	r2 = ((*dest_bmp) & 0x00ff0000)>>16;


	b2 = (b2 < 128) ? (2 * b1 * b2 / 255) : (255 - 2 * (255 - b1) * (255 - b2) / 255);
	g2 = (g2 < 128) ? (2 * g1 * g2 / 255) : (255 - 2 * (255 - g1) * (255 - g2) / 255);
	r2 = (r2 < 128) ? (2 * r1 * r2 / 255) : (255 - 2 * (255 - r1) * (255 - r2) / 255);

	*dest_bmp = a1 << 24 | r2 << 16 | g2 << 8 | b2;
}

static inline void alpha_blend_lighten(uint32_t *dest_bmp,uint32_t *src_bmp)
{
	uint8_t a1,r1,g1,b1;
	uint8_t r2,g2,b2;
	b1 = (*src_bmp) & 0x000000ff;
	g1 = ((*src_bmp) & 0x0000ff00)>>8;
	r1 = ((*src_bmp) & 0x00ff0000)>>16;
	a1 = ((*src_bmp) & 0xff000000)>>24;

	b2 = (*dest_bmp) & 0x000000ff;
	g2 = ((*dest_bmp) & 0x0000ff00)>>8;
	r2 = ((*dest_bmp) & 0x00ff0000)>>16;

	b2 = b2 > b1 ? b2 : b1;
	g2 = g2 > g1 ? g2 : g1;
	r2 = r2 > r1 ? r2 : r1;

	*dest_bmp = a1 << 24 | r2 << 16 | g2 << 8 | b2;
}

void transform_blend_replace(int dx,int dy,uint32_t *dest,int dest_pitch,int dest_width,int dest_height
               , uint32_t *src,int src_pitch,int src_width,int src_height, struct mat *m)
{
        TRANSFORM(alpha_blend_replace)
}

void transform_blend(int dx,int dy,uint32_t *dest,int dest_pitch,int dest_width,int dest_height
                     , uint32_t *src,int src_pitch,int src_width,int src_height, struct mat *m)
{
        TRANSFORM_MMX(alpha_blend)
}

void transform_blend_add(int dx,int dy,uint32_t *dest,int dest_pitch,int dest_width,int dest_height
                         , uint32_t *src,int src_pitch,int src_width,int src_height, struct mat *m)
{

        TRANSFORM(alpha_blend_add)
}

void transform_blend_lighten(int dx,int dy,uint32_t *dest,int dest_pitch,int dest_width,int dest_height
                             , uint32_t *src,int src_pitch,int src_width,int src_height, struct mat *m)
{

        TRANSFORM(alpha_blend_lighten)
}

void transform_blend_overly(int dx,int dy,uint32_t *dest,int dest_pitch,int dest_width,int dest_height
                            , uint32_t *src,int src_pitch,int src_width,int src_height, struct mat *m)
{

        TRANSFORM(alpha_blend_overly)
}