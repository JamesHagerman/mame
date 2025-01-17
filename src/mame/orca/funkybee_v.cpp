// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "funkybee.h"

void funkybee_state::funkybee_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	// first, the character/sprite palette
	for (int i = 0; i < 32; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(*color_prom, 0);
		bit1 = BIT(*color_prom, 1);
		bit2 = BIT(*color_prom, 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// green component
		bit0 = BIT(*color_prom, 3);
		bit1 = BIT(*color_prom, 4);
		bit2 = BIT(*color_prom, 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// blue component
		bit0 = 0;
		bit1 = BIT(*color_prom, 6);
		bit2 = BIT(*color_prom, 7);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
		color_prom++;
	}
}

void funkybee_state::funkybee_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void funkybee_state::funkybee_colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE_LINE_MEMBER(funkybee_state::gfx_bank_w)
{
	m_gfx_bank = state;
	machine().tilemap().mark_all_dirty();
}

void funkybee_state::funkybee_scroll_w(uint8_t data)
{
	m_bg_tilemap->set_scrollx(0, flip_screen() ? -data : data);
}

WRITE_LINE_MEMBER(funkybee_state::flipscreen_w)
{
	flip_screen_set(state);
}

TILE_GET_INFO_MEMBER(funkybee_state::get_bg_tile_info)
{
	int code = m_videoram[tile_index] + ((m_colorram[tile_index] & 0x80) << 1);
	int color = m_colorram[tile_index] & 0x03;

	tileinfo.set(m_gfx_bank, code, color, 0);
}

TILEMAP_MAPPER_MEMBER(funkybee_state::funkybee_tilemap_scan)
{
	/* logical (col,row) -> memory offset */
	return 256 * row + col;
}

void funkybee_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(funkybee_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(funkybee_state::funkybee_tilemap_scan)), 8, 8, 32, 32);
}

void funkybee_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;

	for (offs = 0x0f; offs >= 0; offs--)
	{
		int offs2 = offs + 0x1e00;
		int attr = m_videoram[offs2];
		int code = (attr >> 2) | ((attr & 2) << 5);
		int color = m_colorram[offs2 + 0x10];
		int flipx = 0;
		int flipy = attr & 0x01;
		int sx = m_videoram[offs2 + 0x10];
		int sy = 224 - m_colorram[offs2];

		if (flip_screen())
		{
			sy += 32;
			flipx = !flipx;
		}

		m_gfxdecode->gfx(2 + m_gfx_bank)->transpen(bitmap,cliprect,
			code, color,
			flipx, flipy,
			sx, sy, 0);
	}
}

void funkybee_state::draw_columns( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;

	for (offs = 0x1f; offs >= 0; offs--)
	{
		int const flip = flip_screen();
		int code = m_videoram[0x1c00 + offs];
		int color = m_colorram[0x1f10] & 0x03;
		int sx = flip ? m_videoram[0x1f1f] : m_videoram[0x1f10];
		int sy = offs * 8;

		if (flip)
			sy = 248 - sy;

		m_gfxdecode->gfx(m_gfx_bank)->transpen(bitmap,cliprect,
				code, color,
				flip, flip,
				sx, sy,0);

		code = m_videoram[0x1d00 + offs];
		color = m_colorram[0x1f11] & 0x03;
		sx = flip ? m_videoram[0x1f1e] : m_videoram[0x1f11];
		sy = offs * 8;

		if (flip)
			sy = 248 - sy;

		m_gfxdecode->gfx(m_gfx_bank)->transpen(bitmap,cliprect,
				code, color,
				flip, flip,
				sx, sy,0);
	}
}

uint32_t funkybee_state::screen_update_funkybee(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	draw_columns(bitmap, cliprect);
	return 0;
}
