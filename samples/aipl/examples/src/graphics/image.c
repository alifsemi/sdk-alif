/**
 * @file image.c
 */

/*********************
 *      INCLUDES
 *********************/
#include <stdlib.h>
#include "image.h"
#include "aipl_dave2d.h"
#include "display.h"
#include "aipl_arm_mve.h"
#include <zephyr/cache.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
	uint32_t x;
	uint32_t y;
	void *image;
	uint32_t pitch;
	uint32_t width;
	uint32_t height;
} graph_image_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void dave2d_image_draw(uint32_t format, const graph_image_t *image);
static void helium_image_draw(uint32_t format, const graph_image_t *image);
static void helium_image_draw_rgb888(const graph_image_t *image);
static void helium_image_draw_yv12(const graph_image_t *image);
static void helium_image_draw_i420(const graph_image_t *image);
static void helium_image_draw_i422(const graph_image_t *image);
static void helium_image_draw_i444(const graph_image_t *image);
static void helium_image_draw_i400(const graph_image_t *image);
static void helium_image_draw_nv21(const graph_image_t *image);
static void helium_image_draw_nv12(const graph_image_t *image);
static void helium_image_draw_yuy2(const graph_image_t *image);
static void helium_image_draw_uyvy(const graph_image_t *image);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void aipl_dave2d_prepare(void)
{
	d2_device *handle = aipl_dave2d_handle();

	/* Prepare frame buffer */
	d2_framebuffer(handle, display_inactive_buffer(), display_width(), display_width(),
		       display_height(), d2_mode_rgb565);
	/* Set background */
	d2_clear(handle, 0x00f0f0f0);
}

void aipl_dave2d_render(void)
{
	d2_device *handle = aipl_dave2d_handle();

	/* Close render buffer which has just captured all render commands */
	d2_endframe(handle);
	/* Start HW rendering of the closed frame */
	d2_startframe(handle);
	/* Wait until the render finishes */
	d2_endframe(handle);

	/* Switch to the next display buffer */
	display_next_frame();
}

void aipl_image_draw(uint32_t x, uint32_t y, const aipl_image_t *image)
{
	graph_image_t img = {.x = x,
			     .y = y,
			     .image = image->data,
			     .pitch = image->pitch,
			     .width = image->width,
			     .height = image->height};

	if (aipl_dave2d_format_supported(image->format)) {
		dave2d_image_draw(aipl_dave2d_format_to_mode(image->format), &img);
	} else {
		helium_image_draw(image->format, &img);
	}
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void dave2d_image_draw(uint32_t mode, const graph_image_t *image)
{
	int32_t dsize = image->pitch * image->height * aipl_dave2d_mode_px_size(mode);
	sys_cache_data_flush_and_invd_range(image->image, dsize);

	d2_device *handle = aipl_dave2d_handle();

	d2_cliprect(handle, (d2_border)image->x, (d2_border)image->y,
		    (d2_border)image->x + image->width - 1,
		    (d2_border)image->y + image->height - 1);

	d2_u8 alpha_mode = aipl_dave2d_mode_has_alpha(mode) ? d2_to_copy : d2_to_one;
	d2_settextureoperation(handle, alpha_mode, d2_to_copy, d2_to_copy, d2_to_copy);

	d2_settexture(handle, image->image, image->pitch, image->width, image->height, mode);

	d2_settexturemode(handle, d2_tm_filter);
	d2_setfillmode(handle, d2_fm_texture);
	d2_setblendmode(handle, d2_bm_alpha, d2_bm_one_minus_alpha);
	d2_setalphablendmode(handle, d2_bm_one, d2_bm_one_minus_alpha);

	d2_settexturemapping(handle, D2_FIX4(image->x), D2_FIX4(image->y), D2_FIX16(0), D2_FIX16(0),
			     D2_FIX16(1), D2_FIX16(0), D2_FIX16(0), D2_FIX16(1));

	d2_renderquad(handle, D2_FIX4(image->x), D2_FIX4(image->y),
		      D2_FIX4(image->x + image->width - 1), D2_FIX4(image->y),
		      D2_FIX4(image->x + image->width - 1), D2_FIX4(image->y + image->height - 1),
		      D2_FIX4(image->x), D2_FIX4(image->y + image->height - 1), 0);
}

static void helium_image_draw(uint32_t format, const graph_image_t *image)
{
	/* Execute the current render buffer as we are about to write
	directly to framebuffer */
	d2_device *handle = aipl_dave2d_handle();
	d2_startframe(handle);
	d2_endframe(handle);

	switch (format) {
	case AIPL_COLOR_RGB888:
		helium_image_draw_rgb888(image);
		break;

	case AIPL_COLOR_YV12:
		helium_image_draw_yv12(image);
		break;

	case AIPL_COLOR_I420:
		helium_image_draw_i420(image);
		break;

	case AIPL_COLOR_I422:
		helium_image_draw_i422(image);
		break;

	case AIPL_COLOR_I444:
		helium_image_draw_i444(image);
		break;

	case AIPL_COLOR_I400:
		helium_image_draw_i400(image);
		break;

	case AIPL_COLOR_NV21:
		helium_image_draw_nv21(image);
		break;

	case AIPL_COLOR_NV12:
		helium_image_draw_nv12(image);
		break;

	case AIPL_COLOR_YUY2:
		helium_image_draw_yuy2(image);
		break;

	case AIPL_COLOR_UYVY:
		helium_image_draw_uyvy(image);
		break;

	default:
		/* Other color formats are unsupported for now */
		break;
	}
}

static void helium_image_draw_rgb888(const graph_image_t *image)
{
	const uint8_t *src_ptr = image->image;
	uint8_t *framebuffer = (uint8_t *)display_inactive_buffer();
	uint32_t frbuff_pitch = display_width();
	uint32_t frbuff_width = display_width();
	uint32_t frbuff_height = display_height();

	const uint8x16_t input_pixel_offsets = vmulq_n_u8(vidupq_n_u8(0, 1), 3);
	const uint8x16_t output_pixel_offsets = vmulq_n_u8(vidupq_n_u8(0, 1), 2);

	for (uint32_t i = 0; i < image->height && image->y + i < frbuff_height; ++i) {
		int32_t cnt = frbuff_width < image->x + image->width ? frbuff_width - image->x
								     : image->width;

		cnt *= 3;
		const uint8_t *src = src_ptr + i * image->pitch * 3;
		uint8_t *dst = framebuffer + ((i + image->y) * frbuff_pitch + image->x) * 2;

		while (cnt > 0) {
			mve_pred16_t tail_p = vctp8q(cnt / 3);

			uint8x16_t r = vldrbq_gather_offset_z(src + 2, input_pixel_offsets, tail_p);
			uint8x16_t g = vldrbq_gather_offset_z(src + 1, input_pixel_offsets, tail_p);
			uint8x16_t b = vldrbq_gather_offset_z(src, input_pixel_offsets, tail_p);

			uint8x16_t upper = vsriq(r, g, 5);
			uint8x16_t lower = vshlq_n(g, 3);
			lower = vsriq(lower, b, 3);

			vstrbq_scatter_offset_p(dst + 1, output_pixel_offsets, upper, tail_p);
			vstrbq_scatter_offset_p(dst, output_pixel_offsets, lower, tail_p);

			src += 48;
			dst += 32;
			cnt -= 48;
		}
	}
}

static void helium_image_draw_yv12(const graph_image_t *image)
{
	const uint8_t *src_ptr = image->image;
	uint16_t *framebuffer = (uint16_t *)display_inactive_buffer();
	uint32_t frbuff_pitch = display_width();
	uint32_t frbuff_width = display_width();
	uint32_t frbuff_height = display_height();

	const uint16x8_t y_pixel_offsets = vmulq_n_u16(vidupq_n_u16(0, 1), 2);
	const uint16x8_t output_pixel_offsets = vmulq_n_u16(vidupq_n_u16(0, 1), 4);

	for (uint32_t i = 0; i < image->height && image->y + i < frbuff_height; i += 2) {
		int32_t cnt = frbuff_width < image->x + image->width ? image->width
								     : frbuff_width - image->x;
		const uint32_t y_size = image->pitch * image->height;
		const uint8_t *y_src0 = src_ptr + i * image->pitch;
		const uint8_t *y_src1 = y_src0 + image->pitch;
		const uint8_t *v_src = src_ptr + y_size + i * image->pitch / 4;
		const uint8_t *u_src = v_src + y_size / 4;
		uint16_t *dst0 = framebuffer + ((i + image->y) * frbuff_pitch + image->x);
		uint16_t *dst1 = dst0 + frbuff_pitch;

		while (cnt > 0) {
			mve_pred16_t tail_p = vctp16q(cnt / 2);

			uint16x8_t y = vldrbq_gather_offset_z(y_src0, y_pixel_offsets, tail_p);
			uint16x8_t u = vldrbq_z_u16(u_src, tail_p);
			uint16x8_t v = vldrbq_z_u16(v_src, tail_p);

			int16x8_t c = vsubq_n_s16(vreinterpretq_s16(y), 16);
			int16x8_t d = vsubq_n_s16(vreinterpretq_s16(u), 128);
			int16x8_t e = vsubq_n_s16(vreinterpretq_s16(v), 128);

			int32x4_t c0 = vmovltq(c);
			int32x4_t c1 = vmovlbq(c);
			int32x4_t d0 = vmovltq(d);
			int32x4_t d1 = vmovlbq(d);
			int32x4_t e0 = vmovltq(e);
			int32x4_t e1 = vmovlbq(e);

			int32x4_t r0 = vmulq_n_s32(c0, 298);
			int32x4_t r1 = vmulq_n_s32(c1, 298);
			r0 = vmlaq(r0, e0, 409);
			r1 = vmlaq(r1, e1, 409);

			r0 = vshrq(vaddq(r0, 128), 8);
			r1 = vshrq(vaddq(r1, 128), 8);
			int16x8_t r = vqmovntq(vdupq_n_s16(0), r0);
			r = vqmovnbq(r, r1);
			uint16x8_t r_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), r));

			int32x4_t g0 = vmulq_n_s32(c0, 298);
			int32x4_t g1 = vmulq_n_s32(c1, 298);
			g0 = vmlaq(g0, d0, -100);
			g1 = vmlaq(g1, d1, -100);
			g0 = vmlaq(g0, e0, -208);
			g1 = vmlaq(g1, e1, -208);

			g0 = vshrq(vaddq(g0, 128), 8);
			g1 = vshrq(vaddq(g1, 128), 8);
			int16x8_t g = vqmovntq(vdupq_n_s16(0), g0);
			g = vqmovnbq(g, g1);
			uint16x8_t g_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), g));

			int32x4_t b0 = vmulq_n_s32(c0, 298);
			int32x4_t b1 = vmulq_n_s32(c1, 298);
			b0 = vmlaq(b0, d0, 516);
			b1 = vmlaq(b1, d1, 516);

			b0 = vshrq(vaddq(b0, 128), 8);
			b1 = vshrq(vaddq(b1, 128), 8);
			int16x8_t b = vqmovntq(vdupq_n_s16(0), b0);
			b = vqmovnbq(b, b1);
			uint16x8_t b_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), b));

			uint16x8_t rgb565 = vsliq(vshrq(g_out, 2), vshrq(r_out, 3), 6);
			rgb565 = vsliq(vshrq(b_out, 3), rgb565, 5);

			vstrhq_scatter_offset_p(dst0, output_pixel_offsets, rgb565, tail_p);

			y = vldrbq_gather_offset_z(y_src0 + 1, y_pixel_offsets, tail_p);

			c = vsubq_n_s16(vreinterpretq_s16(y), 16);
			c0 = vmovltq(c);
			c1 = vmovlbq(c);

			r0 = vmulq_n_s32(c0, 298);
			r1 = vmulq_n_s32(c1, 298);
			r0 = vmlaq(r0, e0, 409);
			r1 = vmlaq(r1, e1, 409);

			r0 = vshrq(vaddq(r0, 128), 8);
			r1 = vshrq(vaddq(r1, 128), 8);
			r = vqmovntq(vdupq_n_s16(0), r0);
			r = vqmovnbq(r, r1);
			r_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), r));

			g0 = vmulq_n_s32(c0, 298);
			g1 = vmulq_n_s32(c1, 298);
			g0 = vmlaq(g0, d0, -100);
			g1 = vmlaq(g1, d1, -100);
			g0 = vmlaq(g0, e0, -208);
			g1 = vmlaq(g1, e1, -208);

			g0 = vshrq(vaddq(g0, 128), 8);
			g1 = vshrq(vaddq(g1, 128), 8);
			g = vqmovntq(vdupq_n_s16(0), g0);
			g = vqmovnbq(g, g1);
			g_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), g));

			b0 = vmulq_n_s32(c0, 298);
			b1 = vmulq_n_s32(c1, 298);
			b0 = vmlaq(b0, d0, 516);
			b1 = vmlaq(b1, d1, 516);

			b0 = vshrq(vaddq(b0, 128), 8);
			b1 = vshrq(vaddq(b1, 128), 8);
			b = vqmovntq(vdupq_n_s16(0), b0);
			b = vqmovnbq(b, b1);
			b_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), b));

			rgb565 = vsliq(vshrq(g_out, 2), vshrq(r_out, 3), 6);
			rgb565 = vsliq(vshrq(b_out, 3), rgb565, 5);

			vstrhq_scatter_offset_p(dst0 + 1, output_pixel_offsets, rgb565, tail_p);

			y = vldrbq_gather_offset_z(y_src1, y_pixel_offsets, tail_p);

			c = vsubq_n_s16(vreinterpretq_s16(y), 16);
			c0 = vmovltq(c);
			c1 = vmovlbq(c);

			r0 = vmulq_n_s32(c0, 298);
			r1 = vmulq_n_s32(c1, 298);
			r0 = vmlaq(r0, e0, 409);
			r1 = vmlaq(r1, e1, 409);

			r0 = vshrq(vaddq(r0, 128), 8);
			r1 = vshrq(vaddq(r1, 128), 8);
			r = vqmovntq(vdupq_n_s16(0), r0);
			r = vqmovnbq(r, r1);
			r_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), r));

			g0 = vmulq_n_s32(c0, 298);
			g1 = vmulq_n_s32(c1, 298);
			g0 = vmlaq(g0, d0, -100);
			g1 = vmlaq(g1, d1, -100);
			g0 = vmlaq(g0, e0, -208);
			g1 = vmlaq(g1, e1, -208);

			g0 = vshrq(vaddq(g0, 128), 8);
			g1 = vshrq(vaddq(g1, 128), 8);
			g = vqmovntq(vdupq_n_s16(0), g0);
			g = vqmovnbq(g, g1);
			g_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), g));

			b0 = vmulq_n_s32(c0, 298);
			b1 = vmulq_n_s32(c1, 298);
			b0 = vmlaq(b0, d0, 516);
			b1 = vmlaq(b1, d1, 516);

			b0 = vshrq(vaddq(b0, 128), 8);
			b1 = vshrq(vaddq(b1, 128), 8);
			b = vqmovntq(vdupq_n_s16(0), b0);
			b = vqmovnbq(b, b1);
			b_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), b));

			rgb565 = vsliq(vshrq(g_out, 2), vshrq(r_out, 3), 6);
			rgb565 = vsliq(vshrq(b_out, 3), rgb565, 5);

			vstrhq_scatter_offset_p(dst1, output_pixel_offsets, rgb565, tail_p);

			y = vldrbq_gather_offset_z(y_src1 + 1, y_pixel_offsets, tail_p);

			c = vsubq_n_s16(vreinterpretq_s16(y), 16);
			c0 = vmovltq(c);
			c1 = vmovlbq(c);

			r0 = vmulq_n_s32(c0, 298);
			r1 = vmulq_n_s32(c1, 298);
			r0 = vmlaq(r0, e0, 409);
			r1 = vmlaq(r1, e1, 409);

			r0 = vshrq(vaddq(r0, 128), 8);
			r1 = vshrq(vaddq(r1, 128), 8);
			r = vqmovntq(vdupq_n_s16(0), r0);
			r = vqmovnbq(r, r1);
			r_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), r));

			g0 = vmulq_n_s32(c0, 298);
			g1 = vmulq_n_s32(c1, 298);
			g0 = vmlaq(g0, d0, -100);
			g1 = vmlaq(g1, d1, -100);
			g0 = vmlaq(g0, e0, -208);
			g1 = vmlaq(g1, e1, -208);

			g0 = vshrq(vaddq(g0, 128), 8);
			g1 = vshrq(vaddq(g1, 128), 8);
			g = vqmovntq(vdupq_n_s16(0), g0);
			g = vqmovnbq(g, g1);
			g_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), g));

			b0 = vmulq_n_s32(c0, 298);
			b1 = vmulq_n_s32(c1, 298);
			b0 = vmlaq(b0, d0, 516);
			b1 = vmlaq(b1, d1, 516);

			b0 = vshrq(vaddq(b0, 128), 8);
			b1 = vshrq(vaddq(b1, 128), 8);
			b = vqmovntq(vdupq_n_s16(0), b0);
			b = vqmovnbq(b, b1);
			b_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), b));

			rgb565 = vsliq(vshrq(g_out, 2), vshrq(r_out, 3), 6);
			rgb565 = vsliq(vshrq(b_out, 3), rgb565, 5);

			vstrhq_scatter_offset_p(dst1 + 1, output_pixel_offsets, rgb565, tail_p);

			y_src0 += 16;
			y_src1 += 16;
			u_src += 8;
			v_src += 8;
			dst0 += 16;
			dst1 += 16;
			cnt -= 16;
		}
	}
}

static void helium_image_draw_i420(const graph_image_t *image)
{
	const uint8_t *src_ptr = image->image;
	uint16_t *framebuffer = (uint16_t *)display_inactive_buffer();
	uint32_t frbuff_pitch = display_width();
	uint32_t frbuff_width = display_width();
	uint32_t frbuff_height = display_height();

	const uint16x8_t y_pixel_offsets = vmulq_n_u16(vidupq_n_u16(0, 1), 2);
	const uint16x8_t output_pixel_offsets = vmulq_n_u16(vidupq_n_u16(0, 1), 4);

	for (uint32_t i = 0; i < image->height && image->y + i < frbuff_height; i += 2) {
		int32_t cnt = frbuff_width < image->x + image->width ? image->width
								     : frbuff_width - image->x;
		const uint32_t y_size = image->pitch * image->height;
		const uint8_t *y_src0 = src_ptr + i * image->pitch;
		const uint8_t *y_src1 = y_src0 + image->pitch;
		const uint8_t *u_src = src_ptr + y_size + i * image->pitch / 4;
		const uint8_t *v_src = u_src + y_size / 4;
		uint16_t *dst0 = framebuffer + ((i + image->y) * frbuff_pitch + image->x);
		uint16_t *dst1 = dst0 + frbuff_pitch;

		while (cnt > 0) {
			mve_pred16_t tail_p = vctp16q(cnt / 2);

			uint16x8_t y = vldrbq_gather_offset_z(y_src0, y_pixel_offsets, tail_p);
			uint16x8_t u = vldrbq_z_u16(u_src, tail_p);
			uint16x8_t v = vldrbq_z_u16(v_src, tail_p);

			int16x8_t c = vsubq_n_s16(vreinterpretq_s16(y), 16);
			int16x8_t d = vsubq_n_s16(vreinterpretq_s16(u), 128);
			int16x8_t e = vsubq_n_s16(vreinterpretq_s16(v), 128);

			int32x4_t c0 = vmovltq(c);
			int32x4_t c1 = vmovlbq(c);
			int32x4_t d0 = vmovltq(d);
			int32x4_t d1 = vmovlbq(d);
			int32x4_t e0 = vmovltq(e);
			int32x4_t e1 = vmovlbq(e);

			int32x4_t r0 = vmulq_n_s32(c0, 298);
			int32x4_t r1 = vmulq_n_s32(c1, 298);
			r0 = vmlaq(r0, e0, 409);
			r1 = vmlaq(r1, e1, 409);

			r0 = vshrq(vaddq(r0, 128), 8);
			r1 = vshrq(vaddq(r1, 128), 8);
			int16x8_t r = vqmovntq(vdupq_n_s16(0), r0);
			r = vqmovnbq(r, r1);
			uint16x8_t r_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), r));

			int32x4_t g0 = vmulq_n_s32(c0, 298);
			int32x4_t g1 = vmulq_n_s32(c1, 298);
			g0 = vmlaq(g0, d0, -100);
			g1 = vmlaq(g1, d1, -100);
			g0 = vmlaq(g0, e0, -208);
			g1 = vmlaq(g1, e1, -208);

			g0 = vshrq(vaddq(g0, 128), 8);
			g1 = vshrq(vaddq(g1, 128), 8);
			int16x8_t g = vqmovntq(vdupq_n_s16(0), g0);
			g = vqmovnbq(g, g1);
			uint16x8_t g_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), g));

			int32x4_t b0 = vmulq_n_s32(c0, 298);
			int32x4_t b1 = vmulq_n_s32(c1, 298);
			b0 = vmlaq(b0, d0, 516);
			b1 = vmlaq(b1, d1, 516);

			b0 = vshrq(vaddq(b0, 128), 8);
			b1 = vshrq(vaddq(b1, 128), 8);
			int16x8_t b = vqmovntq(vdupq_n_s16(0), b0);
			b = vqmovnbq(b, b1);
			uint16x8_t b_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), b));

			uint16x8_t rgb565 = vsliq(vshrq(g_out, 2), vshrq(r_out, 3), 6);
			rgb565 = vsliq(vshrq(b_out, 3), rgb565, 5);

			vstrhq_scatter_offset_p(dst0, output_pixel_offsets, rgb565, tail_p);

			y = vldrbq_gather_offset_z(y_src0 + 1, y_pixel_offsets, tail_p);

			c = vsubq_n_s16(vreinterpretq_s16(y), 16);
			c0 = vmovltq(c);
			c1 = vmovlbq(c);

			r0 = vmulq_n_s32(c0, 298);
			r1 = vmulq_n_s32(c1, 298);
			r0 = vmlaq(r0, e0, 409);
			r1 = vmlaq(r1, e1, 409);

			r0 = vshrq(vaddq(r0, 128), 8);
			r1 = vshrq(vaddq(r1, 128), 8);
			r = vqmovntq(vdupq_n_s16(0), r0);
			r = vqmovnbq(r, r1);
			r_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), r));

			g0 = vmulq_n_s32(c0, 298);
			g1 = vmulq_n_s32(c1, 298);
			g0 = vmlaq(g0, d0, -100);
			g1 = vmlaq(g1, d1, -100);
			g0 = vmlaq(g0, e0, -208);
			g1 = vmlaq(g1, e1, -208);

			g0 = vshrq(vaddq(g0, 128), 8);
			g1 = vshrq(vaddq(g1, 128), 8);
			g = vqmovntq(vdupq_n_s16(0), g0);
			g = vqmovnbq(g, g1);
			g_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), g));

			b0 = vmulq_n_s32(c0, 298);
			b1 = vmulq_n_s32(c1, 298);
			b0 = vmlaq(b0, d0, 516);
			b1 = vmlaq(b1, d1, 516);

			b0 = vshrq(vaddq(b0, 128), 8);
			b1 = vshrq(vaddq(b1, 128), 8);
			b = vqmovntq(vdupq_n_s16(0), b0);
			b = vqmovnbq(b, b1);
			b_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), b));

			rgb565 = vsliq(vshrq(g_out, 2), vshrq(r_out, 3), 6);
			rgb565 = vsliq(vshrq(b_out, 3), rgb565, 5);

			vstrhq_scatter_offset_p(dst0 + 1, output_pixel_offsets, rgb565, tail_p);

			y = vldrbq_gather_offset_z(y_src1, y_pixel_offsets, tail_p);

			c = vsubq_n_s16(vreinterpretq_s16(y), 16);
			c0 = vmovltq(c);
			c1 = vmovlbq(c);

			r0 = vmulq_n_s32(c0, 298);
			r1 = vmulq_n_s32(c1, 298);
			r0 = vmlaq(r0, e0, 409);
			r1 = vmlaq(r1, e1, 409);

			r0 = vshrq(vaddq(r0, 128), 8);
			r1 = vshrq(vaddq(r1, 128), 8);
			r = vqmovntq(vdupq_n_s16(0), r0);
			r = vqmovnbq(r, r1);
			r_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), r));

			g0 = vmulq_n_s32(c0, 298);
			g1 = vmulq_n_s32(c1, 298);
			g0 = vmlaq(g0, d0, -100);
			g1 = vmlaq(g1, d1, -100);
			g0 = vmlaq(g0, e0, -208);
			g1 = vmlaq(g1, e1, -208);

			g0 = vshrq(vaddq(g0, 128), 8);
			g1 = vshrq(vaddq(g1, 128), 8);
			g = vqmovntq(vdupq_n_s16(0), g0);
			g = vqmovnbq(g, g1);
			g_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), g));

			b0 = vmulq_n_s32(c0, 298);
			b1 = vmulq_n_s32(c1, 298);
			b0 = vmlaq(b0, d0, 516);
			b1 = vmlaq(b1, d1, 516);

			b0 = vshrq(vaddq(b0, 128), 8);
			b1 = vshrq(vaddq(b1, 128), 8);
			b = vqmovntq(vdupq_n_s16(0), b0);
			b = vqmovnbq(b, b1);
			b_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), b));

			rgb565 = vsliq(vshrq(g_out, 2), vshrq(r_out, 3), 6);
			rgb565 = vsliq(vshrq(b_out, 3), rgb565, 5);

			vstrhq_scatter_offset_p(dst1, output_pixel_offsets, rgb565, tail_p);

			y = vldrbq_gather_offset_z(y_src1 + 1, y_pixel_offsets, tail_p);

			c = vsubq_n_s16(vreinterpretq_s16(y), 16);
			c0 = vmovltq(c);
			c1 = vmovlbq(c);

			r0 = vmulq_n_s32(c0, 298);
			r1 = vmulq_n_s32(c1, 298);
			r0 = vmlaq(r0, e0, 409);
			r1 = vmlaq(r1, e1, 409);

			r0 = vshrq(vaddq(r0, 128), 8);
			r1 = vshrq(vaddq(r1, 128), 8);
			r = vqmovntq(vdupq_n_s16(0), r0);
			r = vqmovnbq(r, r1);
			r_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), r));

			g0 = vmulq_n_s32(c0, 298);
			g1 = vmulq_n_s32(c1, 298);
			g0 = vmlaq(g0, d0, -100);
			g1 = vmlaq(g1, d1, -100);
			g0 = vmlaq(g0, e0, -208);
			g1 = vmlaq(g1, e1, -208);

			g0 = vshrq(vaddq(g0, 128), 8);
			g1 = vshrq(vaddq(g1, 128), 8);
			g = vqmovntq(vdupq_n_s16(0), g0);
			g = vqmovnbq(g, g1);
			g_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), g));

			b0 = vmulq_n_s32(c0, 298);
			b1 = vmulq_n_s32(c1, 298);
			b0 = vmlaq(b0, d0, 516);
			b1 = vmlaq(b1, d1, 516);

			b0 = vshrq(vaddq(b0, 128), 8);
			b1 = vshrq(vaddq(b1, 128), 8);
			b = vqmovntq(vdupq_n_s16(0), b0);
			b = vqmovnbq(b, b1);
			b_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), b));

			rgb565 = vsliq(vshrq(g_out, 2), vshrq(r_out, 3), 6);
			rgb565 = vsliq(vshrq(b_out, 3), rgb565, 5);

			vstrhq_scatter_offset_p(dst1 + 1, output_pixel_offsets, rgb565, tail_p);

			y_src0 += 16;
			y_src1 += 16;
			u_src += 8;
			v_src += 8;
			dst0 += 16;
			dst1 += 16;
			cnt -= 16;
		}
	}
}

static void helium_image_draw_i422(const graph_image_t *image)
{
	const uint8_t *src_ptr = image->image;
	uint16_t *framebuffer = (uint16_t *)display_inactive_buffer();
	uint32_t frbuff_pitch = display_width();
	uint32_t frbuff_width = display_width();
	uint32_t frbuff_height = display_height();

	const uint16x8_t y_pixel_offsets = vmulq_n_u16(vidupq_n_u16(0, 1), 2);
	const uint16x8_t output_pixel_offsets = vmulq_n_u16(vidupq_n_u16(0, 1), 4);

	for (uint32_t i = 0; i < image->height && image->y + i < frbuff_height; ++i) {
		int32_t cnt = frbuff_width < image->x + image->width ? image->width
								     : frbuff_width - image->x;
		const uint32_t y_size = image->pitch * image->height;
		const uint8_t *y_src = src_ptr + i * image->pitch;
		const uint8_t *u_src = src_ptr + y_size + i * image->pitch / 2;
		const uint8_t *v_src = u_src + y_size / 2;
		uint16_t *dst = framebuffer + ((i + image->y) * frbuff_pitch + image->x);

		while (cnt > 0) {
			mve_pred16_t tail_p = vctp16q(cnt / 2);

			uint16x8_t y = vldrbq_gather_offset_z(y_src, y_pixel_offsets, tail_p);
			uint16x8_t u = vldrbq_z_u16(u_src, tail_p);
			uint16x8_t v = vldrbq_z_u16(v_src, tail_p);

			int16x8_t c = vsubq_n_s16(vreinterpretq_s16(y), 16);
			int16x8_t d = vsubq_n_s16(vreinterpretq_s16(u), 128);
			int16x8_t e = vsubq_n_s16(vreinterpretq_s16(v), 128);

			int32x4_t c0 = vmovltq(c);
			int32x4_t c1 = vmovlbq(c);
			int32x4_t d0 = vmovltq(d);
			int32x4_t d1 = vmovlbq(d);
			int32x4_t e0 = vmovltq(e);
			int32x4_t e1 = vmovlbq(e);

			int32x4_t r0 = vmulq_n_s32(c0, 298);
			int32x4_t r1 = vmulq_n_s32(c1, 298);
			r0 = vmlaq(r0, e0, 409);
			r1 = vmlaq(r1, e1, 409);

			r0 = vshrq(vaddq(r0, 128), 8);
			r1 = vshrq(vaddq(r1, 128), 8);
			int16x8_t r = vqmovntq(vdupq_n_s16(0), r0);
			r = vqmovnbq(r, r1);
			uint16x8_t r_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), r));

			int32x4_t g0 = vmulq_n_s32(c0, 298);
			int32x4_t g1 = vmulq_n_s32(c1, 298);
			g0 = vmlaq(g0, d0, -100);
			g1 = vmlaq(g1, d1, -100);
			g0 = vmlaq(g0, e0, -208);
			g1 = vmlaq(g1, e1, -208);

			g0 = vshrq(vaddq(g0, 128), 8);
			g1 = vshrq(vaddq(g1, 128), 8);
			int16x8_t g = vqmovntq(vdupq_n_s16(0), g0);
			g = vqmovnbq(g, g1);
			uint16x8_t g_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), g));

			int32x4_t b0 = vmulq_n_s32(c0, 298);
			int32x4_t b1 = vmulq_n_s32(c1, 298);
			b0 = vmlaq(b0, d0, 516);
			b1 = vmlaq(b1, d1, 516);

			b0 = vshrq(vaddq(b0, 128), 8);
			b1 = vshrq(vaddq(b1, 128), 8);
			int16x8_t b = vqmovntq(vdupq_n_s16(0), b0);
			b = vqmovnbq(b, b1);
			uint16x8_t b_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), b));

			uint16x8_t rgb565 = vsliq(vshrq(g_out, 2), vshrq(r_out, 3), 6);
			rgb565 = vsliq(vshrq(b_out, 3), rgb565, 5);

			vstrhq_scatter_offset_p(dst, output_pixel_offsets, rgb565, tail_p);

			y = vldrbq_gather_offset_z(y_src + 1, y_pixel_offsets, tail_p);

			c = vsubq_n_s16(vreinterpretq_s16(y), 16);
			c0 = vmovltq(c);
			c1 = vmovlbq(c);

			r0 = vmulq_n_s32(c0, 298);
			r1 = vmulq_n_s32(c1, 298);
			r0 = vmlaq(r0, e0, 409);
			r1 = vmlaq(r1, e1, 409);

			r0 = vshrq(vaddq(r0, 128), 8);
			r1 = vshrq(vaddq(r1, 128), 8);
			r = vqmovntq(vdupq_n_s16(0), r0);
			r = vqmovnbq(r, r1);
			r_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), r));

			g0 = vmulq_n_s32(c0, 298);
			g1 = vmulq_n_s32(c1, 298);
			g0 = vmlaq(g0, d0, -100);
			g1 = vmlaq(g1, d1, -100);
			g0 = vmlaq(g0, e0, -208);
			g1 = vmlaq(g1, e1, -208);

			g0 = vshrq(vaddq(g0, 128), 8);
			g1 = vshrq(vaddq(g1, 128), 8);
			g = vqmovntq(vdupq_n_s16(0), g0);
			g = vqmovnbq(g, g1);
			g_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), g));

			b0 = vmulq_n_s32(c0, 298);
			b1 = vmulq_n_s32(c1, 298);
			b0 = vmlaq(b0, d0, 516);
			b1 = vmlaq(b1, d1, 516);

			b0 = vshrq(vaddq(b0, 128), 8);
			b1 = vshrq(vaddq(b1, 128), 8);
			b = vqmovntq(vdupq_n_s16(0), b0);
			b = vqmovnbq(b, b1);
			b_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), b));

			rgb565 = vsliq(vshrq(g_out, 2), vshrq(r_out, 3), 6);
			rgb565 = vsliq(vshrq(b_out, 3), rgb565, 5);

			vstrhq_scatter_offset_p(dst + 1, output_pixel_offsets, rgb565, tail_p);

			y_src += 16;
			u_src += 8;
			v_src += 8;
			dst += 16;
			cnt -= 16;
		}
	}
}

static void helium_image_draw_i444(const graph_image_t *image)
{
	const uint8_t *src_ptr = image->image;
	uint16_t *framebuffer = (uint16_t *)display_inactive_buffer();
	uint32_t frbuff_pitch = display_width();
	uint32_t frbuff_width = display_width();
	uint32_t frbuff_height = display_height();

	for (uint32_t i = 0; i < image->height && image->y + i < frbuff_height; ++i) {
		int32_t cnt = frbuff_width < image->x + image->width ? image->width
								     : frbuff_width - image->x;
		const uint32_t y_size = image->pitch * image->height;
		const uint8_t *y_src = src_ptr + i * image->pitch;
		const uint8_t *u_src = src_ptr + y_size + i * image->pitch;
		const uint8_t *v_src = u_src + y_size;
		uint16_t *dst = framebuffer + ((i + image->y) * frbuff_pitch + image->x);

		while (cnt > 0) {
			mve_pred16_t tail_p = vctp16q(cnt);

			uint16x8_t y = vldrbq_z_u16(y_src, tail_p);
			uint16x8_t u = vldrbq_z_u16(u_src, tail_p);
			uint16x8_t v = vldrbq_z_u16(v_src, tail_p);

			int16x8_t c = vsubq_n_s16(vreinterpretq_s16(y), 16);
			int16x8_t d = vsubq_n_s16(vreinterpretq_s16(u), 128);
			int16x8_t e = vsubq_n_s16(vreinterpretq_s16(v), 128);

			int32x4_t c0 = vmovltq(c);
			int32x4_t c1 = vmovlbq(c);
			int32x4_t d0 = vmovltq(d);
			int32x4_t d1 = vmovlbq(d);
			int32x4_t e0 = vmovltq(e);
			int32x4_t e1 = vmovlbq(e);

			int32x4_t r0 = vmulq_n_s32(c0, 298);
			int32x4_t r1 = vmulq_n_s32(c1, 298);
			r0 = vmlaq(r0, e0, 409);
			r1 = vmlaq(r1, e1, 409);

			r0 = vshrq(vaddq(r0, 128), 8);
			r1 = vshrq(vaddq(r1, 128), 8);
			int16x8_t r = vqmovntq(vdupq_n_s16(0), r0);
			r = vqmovnbq(r, r1);
			uint16x8_t r_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), r));

			int32x4_t g0 = vmulq_n_s32(c0, 298);
			int32x4_t g1 = vmulq_n_s32(c1, 298);
			g0 = vmlaq(g0, d0, -100);
			g1 = vmlaq(g1, d1, -100);
			g0 = vmlaq(g0, e0, -208);
			g1 = vmlaq(g1, e1, -208);

			g0 = vshrq(vaddq(g0, 128), 8);
			g1 = vshrq(vaddq(g1, 128), 8);
			int16x8_t g = vqmovntq(vdupq_n_s16(0), g0);
			g = vqmovnbq(g, g1);
			uint16x8_t g_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), g));

			int32x4_t b0 = vmulq_n_s32(c0, 298);
			int32x4_t b1 = vmulq_n_s32(c1, 298);
			b0 = vmlaq(b0, d0, 516);
			b1 = vmlaq(b1, d1, 516);

			b0 = vshrq(vaddq(b0, 128), 8);
			b1 = vshrq(vaddq(b1, 128), 8);
			int16x8_t b = vqmovntq(vdupq_n_s16(0), b0);
			b = vqmovnbq(b, b1);
			uint16x8_t b_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), b));

			uint16x8_t rgb565 = vsliq(vshrq(g_out, 2), vshrq(r_out, 3), 6);
			rgb565 = vsliq(vshrq(b_out, 3), rgb565, 5);

			vst1q_p(dst, rgb565, tail_p);

			y_src += 8;
			u_src += 8;
			v_src += 8;
			dst += 8;
			cnt -= 8;
		}
	}
}

static void helium_image_draw_i400(const graph_image_t *image)
{
	const uint8_t *src_ptr = image->image;
	uint16_t *framebuffer = (uint16_t *)display_inactive_buffer();
	uint32_t frbuff_pitch = display_width();
	uint32_t frbuff_width = display_width();
	uint32_t frbuff_height = display_height();

	for (uint32_t i = 0; i < image->height && image->y + i < frbuff_height; ++i) {
		int32_t cnt = frbuff_width < image->x + image->width ? image->width
								     : frbuff_width - image->x;
		const uint8_t *y_src = src_ptr + i * image->pitch;
		uint16_t *dst = framebuffer + ((i + image->y) * frbuff_pitch + image->x);

		while (cnt > 0) {
			mve_pred16_t tail_p = vctp16q(cnt);

			uint16x8_t y = vldrbq_z_u16(y_src, tail_p);

			int16x8_t c = vsubq_n_s16(vreinterpretq_s16(y), 16);

			int32x4_t c0 = vmovltq(c);
			int32x4_t c1 = vmovlbq(c);

			int32x4_t r0 = vmulq_n_s32(c0, 298);
			int32x4_t r1 = vmulq_n_s32(c1, 298);

			r0 = vshrq(vaddq(r0, 128), 8);
			r1 = vshrq(vaddq(r1, 128), 8);
			int16x8_t r = vqmovntq(vdupq_n_s16(0), r0);
			r = vqmovnbq(r, r1);
			uint16x8_t r_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), r));

			int32x4_t g0 = vmulq_n_s32(c0, 298);
			int32x4_t g1 = vmulq_n_s32(c1, 298);

			g0 = vshrq(vaddq(g0, 128), 8);
			g1 = vshrq(vaddq(g1, 128), 8);
			int16x8_t g = vqmovntq(vdupq_n_s16(0), g0);
			g = vqmovnbq(g, g1);
			uint16x8_t g_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), g));

			int32x4_t b0 = vmulq_n_s32(c0, 298);
			int32x4_t b1 = vmulq_n_s32(c1, 298);

			b0 = vshrq(vaddq(b0, 128), 8);
			b1 = vshrq(vaddq(b1, 128), 8);
			int16x8_t b = vqmovntq(vdupq_n_s16(0), b0);
			b = vqmovnbq(b, b1);
			uint16x8_t b_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), b));

			uint16x8_t rgb565 = vsliq(vshrq(g_out, 2), vshrq(r_out, 3), 6);
			rgb565 = vsliq(vshrq(b_out, 3), rgb565, 5);

			vst1q_p(dst, rgb565, tail_p);

			y_src += 8;
			dst += 8;
			cnt -= 8;
		}
	}
}

static void helium_image_draw_nv21(const graph_image_t *image)
{
	const uint8_t *src_ptr = image->image;
	uint16_t *framebuffer = (uint16_t *)display_inactive_buffer();
	uint32_t frbuff_pitch = display_width();
	uint32_t frbuff_width = display_width();
	uint32_t frbuff_height = display_height();

	const uint16x8_t input_pixel_offsets = vmulq_n_u16(vidupq_n_u16(0, 1), 2);
	const uint16x8_t output_pixel_offsets = vmulq_n_u16(vidupq_n_u16(0, 1), 4);

	for (uint32_t i = 0; i < image->height && image->y + i < frbuff_height; i += 2) {
		int32_t cnt = frbuff_width < image->x + image->width ? image->width
								     : frbuff_width - image->x;
		const uint32_t y_size = image->pitch * image->height;
		const uint8_t *y_src0 = src_ptr + i * image->pitch;
		const uint8_t *y_src1 = y_src0 + image->pitch;
		const uint8_t *v_src = src_ptr + y_size + i * image->pitch / 2;
		const uint8_t *u_src = v_src + 1;
		uint16_t *dst0 = framebuffer + ((i + image->y) * frbuff_pitch + image->x);
		uint16_t *dst1 = dst0 + frbuff_pitch;

		while (cnt > 0) {
			mve_pred16_t tail_p = vctp16q(cnt / 2);

			uint16x8_t y = vldrbq_gather_offset_z(y_src0, input_pixel_offsets, tail_p);
			uint16x8_t u = vldrbq_gather_offset_z(u_src, input_pixel_offsets, tail_p);
			uint16x8_t v = vldrbq_gather_offset_z(v_src, input_pixel_offsets, tail_p);

			int16x8_t c = vsubq_n_s16(vreinterpretq_s16(y), 16);
			int16x8_t d = vsubq_n_s16(vreinterpretq_s16(u), 128);
			int16x8_t e = vsubq_n_s16(vreinterpretq_s16(v), 128);

			int32x4_t c0 = vmovltq(c);
			int32x4_t c1 = vmovlbq(c);
			int32x4_t d0 = vmovltq(d);
			int32x4_t d1 = vmovlbq(d);
			int32x4_t e0 = vmovltq(e);
			int32x4_t e1 = vmovlbq(e);

			int32x4_t r0 = vmulq_n_s32(c0, 298);
			int32x4_t r1 = vmulq_n_s32(c1, 298);
			r0 = vmlaq(r0, e0, 409);
			r1 = vmlaq(r1, e1, 409);

			r0 = vshrq(vaddq(r0, 128), 8);
			r1 = vshrq(vaddq(r1, 128), 8);
			int16x8_t r = vqmovntq(vdupq_n_s16(0), r0);
			r = vqmovnbq(r, r1);
			uint16x8_t r_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), r));

			int32x4_t g0 = vmulq_n_s32(c0, 298);
			int32x4_t g1 = vmulq_n_s32(c1, 298);
			g0 = vmlaq(g0, d0, -100);
			g1 = vmlaq(g1, d1, -100);
			g0 = vmlaq(g0, e0, -208);
			g1 = vmlaq(g1, e1, -208);

			g0 = vshrq(vaddq(g0, 128), 8);
			g1 = vshrq(vaddq(g1, 128), 8);
			int16x8_t g = vqmovntq(vdupq_n_s16(0), g0);
			g = vqmovnbq(g, g1);
			uint16x8_t g_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), g));

			int32x4_t b0 = vmulq_n_s32(c0, 298);
			int32x4_t b1 = vmulq_n_s32(c1, 298);
			b0 = vmlaq(b0, d0, 516);
			b1 = vmlaq(b1, d1, 516);

			b0 = vshrq(vaddq(b0, 128), 8);
			b1 = vshrq(vaddq(b1, 128), 8);
			int16x8_t b = vqmovntq(vdupq_n_s16(0), b0);
			b = vqmovnbq(b, b1);
			uint16x8_t b_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), b));

			uint16x8_t rgb565 = vsliq(vshrq(g_out, 2), vshrq(r_out, 3), 6);
			rgb565 = vsliq(vshrq(b_out, 3), rgb565, 5);

			vstrhq_scatter_offset_p(dst0, output_pixel_offsets, rgb565, tail_p);

			y = vldrbq_gather_offset_z(y_src0 + 1, input_pixel_offsets, tail_p);

			c = vsubq_n_s16(vreinterpretq_s16(y), 16);
			c0 = vmovltq(c);
			c1 = vmovlbq(c);

			r0 = vmulq_n_s32(c0, 298);
			r1 = vmulq_n_s32(c1, 298);
			r0 = vmlaq(r0, e0, 409);
			r1 = vmlaq(r1, e1, 409);

			r0 = vshrq(vaddq(r0, 128), 8);
			r1 = vshrq(vaddq(r1, 128), 8);
			r = vqmovntq(vdupq_n_s16(0), r0);
			r = vqmovnbq(r, r1);
			r_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), r));

			g0 = vmulq_n_s32(c0, 298);
			g1 = vmulq_n_s32(c1, 298);
			g0 = vmlaq(g0, d0, -100);
			g1 = vmlaq(g1, d1, -100);
			g0 = vmlaq(g0, e0, -208);
			g1 = vmlaq(g1, e1, -208);

			g0 = vshrq(vaddq(g0, 128), 8);
			g1 = vshrq(vaddq(g1, 128), 8);
			g = vqmovntq(vdupq_n_s16(0), g0);
			g = vqmovnbq(g, g1);
			g_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), g));

			b0 = vmulq_n_s32(c0, 298);
			b1 = vmulq_n_s32(c1, 298);
			b0 = vmlaq(b0, d0, 516);
			b1 = vmlaq(b1, d1, 516);

			b0 = vshrq(vaddq(b0, 128), 8);
			b1 = vshrq(vaddq(b1, 128), 8);
			b = vqmovntq(vdupq_n_s16(0), b0);
			b = vqmovnbq(b, b1);
			b_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), b));

			rgb565 = vsliq(vshrq(g_out, 2), vshrq(r_out, 3), 6);
			rgb565 = vsliq(vshrq(b_out, 3), rgb565, 5);

			vstrhq_scatter_offset_p(dst0 + 1, output_pixel_offsets, rgb565, tail_p);

			y = vldrbq_gather_offset_z(y_src1, input_pixel_offsets, tail_p);

			c = vsubq_n_s16(vreinterpretq_s16(y), 16);
			c0 = vmovltq(c);
			c1 = vmovlbq(c);

			r0 = vmulq_n_s32(c0, 298);
			r1 = vmulq_n_s32(c1, 298);
			r0 = vmlaq(r0, e0, 409);
			r1 = vmlaq(r1, e1, 409);

			r0 = vshrq(vaddq(r0, 128), 8);
			r1 = vshrq(vaddq(r1, 128), 8);
			r = vqmovntq(vdupq_n_s16(0), r0);
			r = vqmovnbq(r, r1);
			r_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), r));

			g0 = vmulq_n_s32(c0, 298);
			g1 = vmulq_n_s32(c1, 298);
			g0 = vmlaq(g0, d0, -100);
			g1 = vmlaq(g1, d1, -100);
			g0 = vmlaq(g0, e0, -208);
			g1 = vmlaq(g1, e1, -208);

			g0 = vshrq(vaddq(g0, 128), 8);
			g1 = vshrq(vaddq(g1, 128), 8);
			g = vqmovntq(vdupq_n_s16(0), g0);
			g = vqmovnbq(g, g1);
			g_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), g));

			b0 = vmulq_n_s32(c0, 298);
			b1 = vmulq_n_s32(c1, 298);
			b0 = vmlaq(b0, d0, 516);
			b1 = vmlaq(b1, d1, 516);

			b0 = vshrq(vaddq(b0, 128), 8);
			b1 = vshrq(vaddq(b1, 128), 8);
			b = vqmovntq(vdupq_n_s16(0), b0);
			b = vqmovnbq(b, b1);
			b_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), b));

			rgb565 = vsliq(vshrq(g_out, 2), vshrq(r_out, 3), 6);
			rgb565 = vsliq(vshrq(b_out, 3), rgb565, 5);

			vstrhq_scatter_offset_p(dst1, output_pixel_offsets, rgb565, tail_p);

			y = vldrbq_gather_offset_z(y_src1 + 1, input_pixel_offsets, tail_p);

			c = vsubq_n_s16(vreinterpretq_s16(y), 16);
			c0 = vmovltq(c);
			c1 = vmovlbq(c);

			r0 = vmulq_n_s32(c0, 298);
			r1 = vmulq_n_s32(c1, 298);
			r0 = vmlaq(r0, e0, 409);
			r1 = vmlaq(r1, e1, 409);

			r0 = vshrq(vaddq(r0, 128), 8);
			r1 = vshrq(vaddq(r1, 128), 8);
			r = vqmovntq(vdupq_n_s16(0), r0);
			r = vqmovnbq(r, r1);
			r_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), r));

			g0 = vmulq_n_s32(c0, 298);
			g1 = vmulq_n_s32(c1, 298);
			g0 = vmlaq(g0, d0, -100);
			g1 = vmlaq(g1, d1, -100);
			g0 = vmlaq(g0, e0, -208);
			g1 = vmlaq(g1, e1, -208);

			g0 = vshrq(vaddq(g0, 128), 8);
			g1 = vshrq(vaddq(g1, 128), 8);
			g = vqmovntq(vdupq_n_s16(0), g0);
			g = vqmovnbq(g, g1);
			g_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), g));

			b0 = vmulq_n_s32(c0, 298);
			b1 = vmulq_n_s32(c1, 298);
			b0 = vmlaq(b0, d0, 516);
			b1 = vmlaq(b1, d1, 516);

			b0 = vshrq(vaddq(b0, 128), 8);
			b1 = vshrq(vaddq(b1, 128), 8);
			b = vqmovntq(vdupq_n_s16(0), b0);
			b = vqmovnbq(b, b1);
			b_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), b));

			rgb565 = vsliq(vshrq(g_out, 2), vshrq(r_out, 3), 6);
			rgb565 = vsliq(vshrq(b_out, 3), rgb565, 5);

			vstrhq_scatter_offset_p(dst1 + 1, output_pixel_offsets, rgb565, tail_p);

			y_src0 += 16;
			y_src1 += 16;
			u_src += 16;
			v_src += 16;
			dst0 += 16;
			dst1 += 16;
			cnt -= 16;
		}
	}
}

static void helium_image_draw_nv12(const graph_image_t *image)
{
	const uint8_t *src_ptr = image->image;
	uint16_t *framebuffer = (uint16_t *)display_inactive_buffer();
	uint32_t frbuff_pitch = display_width();
	uint32_t frbuff_width = display_width();
	uint32_t frbuff_height = display_height();

	const uint16x8_t input_pixel_offsets = vmulq_n_u16(vidupq_n_u16(0, 1), 2);
	const uint16x8_t output_pixel_offsets = vmulq_n_u16(vidupq_n_u16(0, 1), 4);

	for (uint32_t i = 0; i < image->height && image->y + i < frbuff_height; i += 2) {
		int32_t cnt = frbuff_width < image->x + image->width ? image->width
								     : frbuff_width - image->x;
		const uint32_t y_size = image->pitch * image->height;
		const uint8_t *y_src0 = src_ptr + i * image->pitch;
		const uint8_t *y_src1 = y_src0 + image->pitch;
		const uint8_t *u_src = src_ptr + y_size + i * image->pitch / 2;
		const uint8_t *v_src = u_src + 1;
		uint16_t *dst0 = framebuffer + ((i + image->y) * frbuff_pitch + image->x);
		uint16_t *dst1 = dst0 + frbuff_pitch;

		while (cnt > 0) {
			mve_pred16_t tail_p = vctp16q(cnt / 2);

			uint16x8_t y = vldrbq_gather_offset_z(y_src0, input_pixel_offsets, tail_p);
			uint16x8_t u = vldrbq_gather_offset_z(u_src, input_pixel_offsets, tail_p);
			uint16x8_t v = vldrbq_gather_offset_z(v_src, input_pixel_offsets, tail_p);

			int16x8_t c = vsubq_n_s16(vreinterpretq_s16(y), 16);
			int16x8_t d = vsubq_n_s16(vreinterpretq_s16(u), 128);
			int16x8_t e = vsubq_n_s16(vreinterpretq_s16(v), 128);

			int32x4_t c0 = vmovltq(c);
			int32x4_t c1 = vmovlbq(c);
			int32x4_t d0 = vmovltq(d);
			int32x4_t d1 = vmovlbq(d);
			int32x4_t e0 = vmovltq(e);
			int32x4_t e1 = vmovlbq(e);

			int32x4_t r0 = vmulq_n_s32(c0, 298);
			int32x4_t r1 = vmulq_n_s32(c1, 298);
			r0 = vmlaq(r0, e0, 409);
			r1 = vmlaq(r1, e1, 409);

			r0 = vshrq(vaddq(r0, 128), 8);
			r1 = vshrq(vaddq(r1, 128), 8);
			int16x8_t r = vqmovntq(vdupq_n_s16(0), r0);
			r = vqmovnbq(r, r1);
			uint16x8_t r_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), r));

			int32x4_t g0 = vmulq_n_s32(c0, 298);
			int32x4_t g1 = vmulq_n_s32(c1, 298);
			g0 = vmlaq(g0, d0, -100);
			g1 = vmlaq(g1, d1, -100);
			g0 = vmlaq(g0, e0, -208);
			g1 = vmlaq(g1, e1, -208);

			g0 = vshrq(vaddq(g0, 128), 8);
			g1 = vshrq(vaddq(g1, 128), 8);
			int16x8_t g = vqmovntq(vdupq_n_s16(0), g0);
			g = vqmovnbq(g, g1);
			uint16x8_t g_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), g));

			int32x4_t b0 = vmulq_n_s32(c0, 298);
			int32x4_t b1 = vmulq_n_s32(c1, 298);
			b0 = vmlaq(b0, d0, 516);
			b1 = vmlaq(b1, d1, 516);

			b0 = vshrq(vaddq(b0, 128), 8);
			b1 = vshrq(vaddq(b1, 128), 8);
			int16x8_t b = vqmovntq(vdupq_n_s16(0), b0);
			b = vqmovnbq(b, b1);
			uint16x8_t b_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), b));

			uint16x8_t rgb565 = vsliq(vshrq(g_out, 2), vshrq(r_out, 3), 6);
			rgb565 = vsliq(vshrq(b_out, 3), rgb565, 5);

			vstrhq_scatter_offset_p(dst0, output_pixel_offsets, rgb565, tail_p);

			y = vldrbq_gather_offset_z(y_src0 + 1, input_pixel_offsets, tail_p);

			c = vsubq_n_s16(vreinterpretq_s16(y), 16);
			c0 = vmovltq(c);
			c1 = vmovlbq(c);

			r0 = vmulq_n_s32(c0, 298);
			r1 = vmulq_n_s32(c1, 298);
			r0 = vmlaq(r0, e0, 409);
			r1 = vmlaq(r1, e1, 409);

			r0 = vshrq(vaddq(r0, 128), 8);
			r1 = vshrq(vaddq(r1, 128), 8);
			r = vqmovntq(vdupq_n_s16(0), r0);
			r = vqmovnbq(r, r1);
			r_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), r));

			g0 = vmulq_n_s32(c0, 298);
			g1 = vmulq_n_s32(c1, 298);
			g0 = vmlaq(g0, d0, -100);
			g1 = vmlaq(g1, d1, -100);
			g0 = vmlaq(g0, e0, -208);
			g1 = vmlaq(g1, e1, -208);

			g0 = vshrq(vaddq(g0, 128), 8);
			g1 = vshrq(vaddq(g1, 128), 8);
			g = vqmovntq(vdupq_n_s16(0), g0);
			g = vqmovnbq(g, g1);
			g_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), g));

			b0 = vmulq_n_s32(c0, 298);
			b1 = vmulq_n_s32(c1, 298);
			b0 = vmlaq(b0, d0, 516);
			b1 = vmlaq(b1, d1, 516);

			b0 = vshrq(vaddq(b0, 128), 8);
			b1 = vshrq(vaddq(b1, 128), 8);
			b = vqmovntq(vdupq_n_s16(0), b0);
			b = vqmovnbq(b, b1);
			b_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), b));

			rgb565 = vsliq(vshrq(g_out, 2), vshrq(r_out, 3), 6);
			rgb565 = vsliq(vshrq(b_out, 3), rgb565, 5);

			vstrhq_scatter_offset_p(dst0 + 1, output_pixel_offsets, rgb565, tail_p);

			y = vldrbq_gather_offset_z(y_src1, input_pixel_offsets, tail_p);

			c = vsubq_n_s16(vreinterpretq_s16(y), 16);
			c0 = vmovltq(c);
			c1 = vmovlbq(c);

			r0 = vmulq_n_s32(c0, 298);
			r1 = vmulq_n_s32(c1, 298);
			r0 = vmlaq(r0, e0, 409);
			r1 = vmlaq(r1, e1, 409);

			r0 = vshrq(vaddq(r0, 128), 8);
			r1 = vshrq(vaddq(r1, 128), 8);
			r = vqmovntq(vdupq_n_s16(0), r0);
			r = vqmovnbq(r, r1);
			r_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), r));

			g0 = vmulq_n_s32(c0, 298);
			g1 = vmulq_n_s32(c1, 298);
			g0 = vmlaq(g0, d0, -100);
			g1 = vmlaq(g1, d1, -100);
			g0 = vmlaq(g0, e0, -208);
			g1 = vmlaq(g1, e1, -208);

			g0 = vshrq(vaddq(g0, 128), 8);
			g1 = vshrq(vaddq(g1, 128), 8);
			g = vqmovntq(vdupq_n_s16(0), g0);
			g = vqmovnbq(g, g1);
			g_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), g));

			b0 = vmulq_n_s32(c0, 298);
			b1 = vmulq_n_s32(c1, 298);
			b0 = vmlaq(b0, d0, 516);
			b1 = vmlaq(b1, d1, 516);

			b0 = vshrq(vaddq(b0, 128), 8);
			b1 = vshrq(vaddq(b1, 128), 8);
			b = vqmovntq(vdupq_n_s16(0), b0);
			b = vqmovnbq(b, b1);
			b_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), b));

			rgb565 = vsliq(vshrq(g_out, 2), vshrq(r_out, 3), 6);
			rgb565 = vsliq(vshrq(b_out, 3), rgb565, 5);

			vstrhq_scatter_offset_p(dst1, output_pixel_offsets, rgb565, tail_p);

			y = vldrbq_gather_offset_z(y_src1 + 1, input_pixel_offsets, tail_p);

			c = vsubq_n_s16(vreinterpretq_s16(y), 16);
			c0 = vmovltq(c);
			c1 = vmovlbq(c);

			r0 = vmulq_n_s32(c0, 298);
			r1 = vmulq_n_s32(c1, 298);
			r0 = vmlaq(r0, e0, 409);
			r1 = vmlaq(r1, e1, 409);

			r0 = vshrq(vaddq(r0, 128), 8);
			r1 = vshrq(vaddq(r1, 128), 8);
			r = vqmovntq(vdupq_n_s16(0), r0);
			r = vqmovnbq(r, r1);
			r_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), r));

			g0 = vmulq_n_s32(c0, 298);
			g1 = vmulq_n_s32(c1, 298);
			g0 = vmlaq(g0, d0, -100);
			g1 = vmlaq(g1, d1, -100);
			g0 = vmlaq(g0, e0, -208);
			g1 = vmlaq(g1, e1, -208);

			g0 = vshrq(vaddq(g0, 128), 8);
			g1 = vshrq(vaddq(g1, 128), 8);
			g = vqmovntq(vdupq_n_s16(0), g0);
			g = vqmovnbq(g, g1);
			g_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), g));

			b0 = vmulq_n_s32(c0, 298);
			b1 = vmulq_n_s32(c1, 298);
			b0 = vmlaq(b0, d0, 516);
			b1 = vmlaq(b1, d1, 516);

			b0 = vshrq(vaddq(b0, 128), 8);
			b1 = vshrq(vaddq(b1, 128), 8);
			b = vqmovntq(vdupq_n_s16(0), b0);
			b = vqmovnbq(b, b1);
			b_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), b));

			rgb565 = vsliq(vshrq(g_out, 2), vshrq(r_out, 3), 6);
			rgb565 = vsliq(vshrq(b_out, 3), rgb565, 5);

			vstrhq_scatter_offset_p(dst1 + 1, output_pixel_offsets, rgb565, tail_p);

			y_src0 += 16;
			y_src1 += 16;
			u_src += 16;
			v_src += 16;
			dst0 += 16;
			dst1 += 16;
			cnt -= 16;
		}
	}
}

static void helium_image_draw_yuy2(const graph_image_t *image)
{
	const uint8_t *src_ptr = image->image;
	uint16_t *framebuffer = (uint16_t *)display_inactive_buffer();
	uint32_t frbuff_pitch = display_width();
	uint32_t frbuff_width = display_width();
	uint32_t frbuff_height = display_height();

	const uint16x8_t input_pixel_offsets = vmulq_n_u16(vidupq_n_u16(0, 1), 4);
	const uint16x8_t output_pixel_offsets = vmulq_n_u16(vidupq_n_u16(0, 1), 4);

	for (uint32_t i = 0; i < image->height && image->y + i < frbuff_height; ++i) {
		int32_t cnt = frbuff_width < image->x + image->width ? image->width
								     : frbuff_width - image->x;
		const uint8_t *y_src = src_ptr + i * image->pitch * 2;
		const uint8_t *u_src = y_src + 1;
		const uint8_t *v_src = u_src + 2;
		uint16_t *dst = framebuffer + ((i + image->y) * frbuff_pitch + image->x);

		while (cnt > 0) {
			mve_pred16_t tail_p = vctp16q(cnt / 2);

			uint16x8_t y = vldrbq_gather_offset_z(y_src, input_pixel_offsets, tail_p);
			uint16x8_t u = vldrbq_gather_offset_z(u_src, input_pixel_offsets, tail_p);
			uint16x8_t v = vldrbq_gather_offset_z(v_src, input_pixel_offsets, tail_p);

			int16x8_t c = vsubq_n_s16(vreinterpretq_s16(y), 16);
			int16x8_t d = vsubq_n_s16(vreinterpretq_s16(u), 128);
			int16x8_t e = vsubq_n_s16(vreinterpretq_s16(v), 128);

			int32x4_t c0 = vmovltq(c);
			int32x4_t c1 = vmovlbq(c);
			int32x4_t d0 = vmovltq(d);
			int32x4_t d1 = vmovlbq(d);
			int32x4_t e0 = vmovltq(e);
			int32x4_t e1 = vmovlbq(e);

			int32x4_t r0 = vmulq_n_s32(c0, 298);
			int32x4_t r1 = vmulq_n_s32(c1, 298);
			r0 = vmlaq(r0, e0, 409);
			r1 = vmlaq(r1, e1, 409);

			r0 = vshrq(vaddq(r0, 128), 8);
			r1 = vshrq(vaddq(r1, 128), 8);
			int16x8_t r = vqmovntq(vdupq_n_s16(0), r0);
			r = vqmovnbq(r, r1);
			uint16x8_t r_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), r));

			int32x4_t g0 = vmulq_n_s32(c0, 298);
			int32x4_t g1 = vmulq_n_s32(c1, 298);
			g0 = vmlaq(g0, d0, -100);
			g1 = vmlaq(g1, d1, -100);
			g0 = vmlaq(g0, e0, -208);
			g1 = vmlaq(g1, e1, -208);

			g0 = vshrq(vaddq(g0, 128), 8);
			g1 = vshrq(vaddq(g1, 128), 8);
			int16x8_t g = vqmovntq(vdupq_n_s16(0), g0);
			g = vqmovnbq(g, g1);
			uint16x8_t g_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), g));

			int32x4_t b0 = vmulq_n_s32(c0, 298);
			int32x4_t b1 = vmulq_n_s32(c1, 298);
			b0 = vmlaq(b0, d0, 516);
			b1 = vmlaq(b1, d1, 516);

			b0 = vshrq(vaddq(b0, 128), 8);
			b1 = vshrq(vaddq(b1, 128), 8);
			int16x8_t b = vqmovntq(vdupq_n_s16(0), b0);
			b = vqmovnbq(b, b1);
			uint16x8_t b_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), b));

			uint16x8_t rgb565 = vsliq(vshrq(g_out, 2), vshrq(r_out, 3), 6);
			rgb565 = vsliq(vshrq(b_out, 3), rgb565, 5);

			vstrhq_scatter_offset_p(dst, output_pixel_offsets, rgb565, tail_p);

			y = vldrbq_gather_offset_z(y_src + 2, input_pixel_offsets, tail_p);

			c = vsubq_n_s16(vreinterpretq_s16(y), 16);
			c0 = vmovltq(c);
			c1 = vmovlbq(c);

			r0 = vmulq_n_s32(c0, 298);
			r1 = vmulq_n_s32(c1, 298);
			r0 = vmlaq(r0, e0, 409);
			r1 = vmlaq(r1, e1, 409);

			r0 = vshrq(vaddq(r0, 128), 8);
			r1 = vshrq(vaddq(r1, 128), 8);
			r = vqmovntq(vdupq_n_s16(0), r0);
			r = vqmovnbq(r, r1);
			r_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), r));

			g0 = vmulq_n_s32(c0, 298);
			g1 = vmulq_n_s32(c1, 298);
			g0 = vmlaq(g0, d0, -100);
			g1 = vmlaq(g1, d1, -100);
			g0 = vmlaq(g0, e0, -208);
			g1 = vmlaq(g1, e1, -208);

			g0 = vshrq(vaddq(g0, 128), 8);
			g1 = vshrq(vaddq(g1, 128), 8);
			g = vqmovntq(vdupq_n_s16(0), g0);
			g = vqmovnbq(g, g1);
			g_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), g));

			b0 = vmulq_n_s32(c0, 298);
			b1 = vmulq_n_s32(c1, 298);
			b0 = vmlaq(b0, d0, 516);
			b1 = vmlaq(b1, d1, 516);

			b0 = vshrq(vaddq(b0, 128), 8);
			b1 = vshrq(vaddq(b1, 128), 8);
			b = vqmovntq(vdupq_n_s16(0), b0);
			b = vqmovnbq(b, b1);
			b_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), b));

			rgb565 = vsliq(vshrq(g_out, 2), vshrq(r_out, 3), 6);
			rgb565 = vsliq(vshrq(b_out, 3), rgb565, 5);

			vstrhq_scatter_offset_p(dst + 1, output_pixel_offsets, rgb565, tail_p);

			y_src += 32;
			u_src += 32;
			v_src += 32;
			dst += 16;
			cnt -= 16;
		}
	}
}

static void helium_image_draw_uyvy(const graph_image_t *image)
{
	const uint8_t *src_ptr = image->image;
	uint16_t *framebuffer = (uint16_t *)display_inactive_buffer();
	uint32_t frbuff_pitch = display_width();
	uint32_t frbuff_width = display_width();
	uint32_t frbuff_height = display_height();

	const uint16x8_t input_pixel_offsets = vmulq_n_u16(vidupq_n_u16(0, 1), 4);
	const uint16x8_t output_pixel_offsets = vmulq_n_u16(vidupq_n_u16(0, 1), 4);

	for (uint32_t i = 0; i < image->height && image->y + i < frbuff_height; ++i) {
		int32_t cnt = frbuff_width < image->x + image->width ? image->width
								     : frbuff_width - image->x;
		const uint8_t *u_src = src_ptr + i * image->pitch * 2;
		const uint8_t *y_src = u_src + 1;
		const uint8_t *v_src = u_src + 2;
		uint16_t *dst = framebuffer + ((i + image->y) * frbuff_pitch + image->x);

		while (cnt > 0) {
			mve_pred16_t tail_p = vctp16q(cnt / 2);

			uint16x8_t y = vldrbq_gather_offset_z(y_src, input_pixel_offsets, tail_p);
			uint16x8_t u = vldrbq_gather_offset_z(u_src, input_pixel_offsets, tail_p);
			uint16x8_t v = vldrbq_gather_offset_z(v_src, input_pixel_offsets, tail_p);

			int16x8_t c = vsubq_n_s16(vreinterpretq_s16(y), 16);
			int16x8_t d = vsubq_n_s16(vreinterpretq_s16(u), 128);
			int16x8_t e = vsubq_n_s16(vreinterpretq_s16(v), 128);

			int32x4_t c0 = vmovltq(c);
			int32x4_t c1 = vmovlbq(c);
			int32x4_t d0 = vmovltq(d);
			int32x4_t d1 = vmovlbq(d);
			int32x4_t e0 = vmovltq(e);
			int32x4_t e1 = vmovlbq(e);

			int32x4_t r0 = vmulq_n_s32(c0, 298);
			int32x4_t r1 = vmulq_n_s32(c1, 298);
			r0 = vmlaq(r0, e0, 409);
			r1 = vmlaq(r1, e1, 409);

			r0 = vshrq(vaddq(r0, 128), 8);
			r1 = vshrq(vaddq(r1, 128), 8);
			int16x8_t r = vqmovntq(vdupq_n_s16(0), r0);
			r = vqmovnbq(r, r1);
			uint16x8_t r_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), r));

			int32x4_t g0 = vmulq_n_s32(c0, 298);
			int32x4_t g1 = vmulq_n_s32(c1, 298);
			g0 = vmlaq(g0, d0, -100);
			g1 = vmlaq(g1, d1, -100);
			g0 = vmlaq(g0, e0, -208);
			g1 = vmlaq(g1, e1, -208);

			g0 = vshrq(vaddq(g0, 128), 8);
			g1 = vshrq(vaddq(g1, 128), 8);
			int16x8_t g = vqmovntq(vdupq_n_s16(0), g0);
			g = vqmovnbq(g, g1);
			uint16x8_t g_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), g));

			int32x4_t b0 = vmulq_n_s32(c0, 298);
			int32x4_t b1 = vmulq_n_s32(c1, 298);
			b0 = vmlaq(b0, d0, 516);
			b1 = vmlaq(b1, d1, 516);

			b0 = vshrq(vaddq(b0, 128), 8);
			b1 = vshrq(vaddq(b1, 128), 8);
			int16x8_t b = vqmovntq(vdupq_n_s16(0), b0);
			b = vqmovnbq(b, b1);
			uint16x8_t b_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), b));

			uint16x8_t rgb565 = vsliq(vshrq(g_out, 2), vshrq(r_out, 3), 6);
			rgb565 = vsliq(vshrq(b_out, 3), rgb565, 5);

			vstrhq_scatter_offset_p(dst, output_pixel_offsets, rgb565, tail_p);

			y = vldrbq_gather_offset_z(y_src + 2, input_pixel_offsets, tail_p);

			c = vsubq_n_s16(vreinterpretq_s16(y), 16);
			c0 = vmovltq(c);
			c1 = vmovlbq(c);

			r0 = vmulq_n_s32(c0, 298);
			r1 = vmulq_n_s32(c1, 298);
			r0 = vmlaq(r0, e0, 409);
			r1 = vmlaq(r1, e1, 409);

			r0 = vshrq(vaddq(r0, 128), 8);
			r1 = vshrq(vaddq(r1, 128), 8);
			r = vqmovntq(vdupq_n_s16(0), r0);
			r = vqmovnbq(r, r1);
			r_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), r));

			g0 = vmulq_n_s32(c0, 298);
			g1 = vmulq_n_s32(c1, 298);
			g0 = vmlaq(g0, d0, -100);
			g1 = vmlaq(g1, d1, -100);
			g0 = vmlaq(g0, e0, -208);
			g1 = vmlaq(g1, e1, -208);

			g0 = vshrq(vaddq(g0, 128), 8);
			g1 = vshrq(vaddq(g1, 128), 8);
			g = vqmovntq(vdupq_n_s16(0), g0);
			g = vqmovnbq(g, g1);
			g_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), g));

			b0 = vmulq_n_s32(c0, 298);
			b1 = vmulq_n_s32(c1, 298);
			b0 = vmlaq(b0, d0, 516);
			b1 = vmlaq(b1, d1, 516);

			b0 = vshrq(vaddq(b0, 128), 8);
			b1 = vshrq(vaddq(b1, 128), 8);
			b = vqmovntq(vdupq_n_s16(0), b0);
			b = vqmovnbq(b, b1);
			b_out = vreinterpretq_u16(vqmovunbq(vdupq_n_u8(0), b));

			rgb565 = vsliq(vshrq(g_out, 2), vshrq(r_out, 3), 6);
			rgb565 = vsliq(vshrq(b_out, 3), rgb565, 5);

			vstrhq_scatter_offset_p(dst + 1, output_pixel_offsets, rgb565, tail_p);

			y_src += 32;
			u_src += 32;
			v_src += 32;
			dst += 16;
			cnt -= 16;
		}
	}
}
