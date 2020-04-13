#include "font.h"

#include "debug.h"
#include "surface.h"
#include "utilities.h"

#include <SDL.h>
#include <SDL_ttf.h>

#include <algorithm>
#include <cassert>
#include <vector>

using namespace std;

Font::Font(std::string path, unsigned int size) :
path_(std::move(path)), size_(size)
{
	font = nullptr;
	lineSpacing = 1;

	/* Note: TTF_Init and TTF_Quit perform reference counting, so call them
	 * both unconditionally for each font. */
	if (TTF_Init() < 0) {
		ERROR("Unable to init SDL_ttf library\n");
		return;
	}

	font = TTF_OpenFont(path_.c_str(), size);
	if (!font) {
		ERROR("Unable to open font '%s'\n", path.c_str());
		TTF_Quit();
		return;
	}

	lineSpacing = TTF_FontLineSkip(font);
}

Font::~Font()
{
	if (font) {
		TTF_CloseFont(font);
		TTF_Quit();
	}
}

int Font::getTextWidth(const string &text) const
{
	if (!font) {
		return 1;
	}

	int w;
	size_t pos = text.find('\n', 0);
	if (pos == string::npos) {
		TTF_SizeUTF8(font, text.c_str(), &w, nullptr);
		return w;
	} else {
		int maxWidth = 1;
		size_t prev = 0;
		do {
			TTF_SizeUTF8(font, text.substr(prev, pos - prev).c_str(), &w, nullptr);
			maxWidth = max(w, maxWidth);
			prev = pos + 1;
			pos = text.find('\n', prev);
		} while (pos != string::npos);
		TTF_SizeUTF8(font, text.substr(prev).c_str(), &w, nullptr);
		return max(w, maxWidth);
	}
}

int Font::getTextHeight(const string &text) const
{
	int nLines = 1 + std::count(text.begin(), text.end(), '\n');
	return nLines * getLineSpacing();
}

int Font::write(Surface& surface, const string &text,
			int x, int y, HAlign halign, VAlign valign) const
{
	if (!font) {
		return 0;
	}

	size_t pos = text.find('\n', 0);
	if (pos == string::npos) {
		return writeLine(surface, text, x, y, halign, valign);
	} else {
		int maxWidth = 0;
		size_t prev = 0;
		do {
			maxWidth = max(maxWidth,
					writeLine(surface, text.substr(prev, pos - prev),
						x, y, halign, valign));
			y += lineSpacing;
			prev = pos + 1;
			pos = text.find('\n', prev);
		} while (pos != string::npos);
		return max(maxWidth,
				writeLine(surface, text.substr(prev), x, y, halign, valign));
	}
}

int Font::writeLine(Surface& surface, std::string const& text,
				int x, int y, HAlign halign, VAlign valign) const
{
	if (text.empty()) {
		// SDL_ttf will return a nullptr when rendering the empty string.
		return 0;
	}

	switch (valign) {
	case VAlignTop:
		break;
	case VAlignMiddle:
		y -= lineSpacing / 2;
		break;
	case VAlignBottom:
		y -= lineSpacing;
		break;
	}

	SDL_Color color = { 0, 0, 0, 0 };
	SDL_Surface *s = TTF_RenderUTF8_Blended(font, text.c_str(), color);
	if (!s) {
		ERROR("Font rendering failed for text \"%s\"\n", text.c_str());
		return 0;
	}
	const int width = s->w;

	switch (halign) {
	case HAlignLeft:
		break;
	case HAlignCenter:
		x -= width / 2;
		break;
	case HAlignRight:
		x -= width;
		break;
	}

	SDL_Rect rect = { (Sint16) x, (Sint16) (y - 1), 0, 0 };
	SDL_BlitSurface(s, NULL, surface.raw, &rect);

	/* Note: rect.x / rect.y are reset everytime because SDL_BlitSurface
	 * will modify them if negative */
	rect.x = x;
	rect.y = y + 1;
	SDL_BlitSurface(s, NULL, surface.raw, &rect);

	rect.x = x - 1;
	rect.y = y;
	SDL_BlitSurface(s, NULL, surface.raw, &rect);

	rect.x = x + 1;
	rect.y = y;
	SDL_BlitSurface(s, NULL, surface.raw, &rect);
	SDL_FreeSurface(s);

	rect.x = x;
	rect.y = y;
	color.r = 0xff;
	color.g = 0xff;
	color.b = 0xff;

	s = TTF_RenderUTF8_Blended(font, text.c_str(), color);
	if (!s) {
		ERROR("Font rendering failed for text \"%s\"\n", text.c_str());
		return width;
	}
	SDL_BlitSurface(s, NULL, surface.raw, &rect);
	SDL_FreeSurface(s);

	return width;
}

static inline uint8_t *get_pixel8(SDL_Surface *s, int row, int col)
{
	uintptr_t row_addr = (uintptr_t)s->pixels + row * s->pitch;

	assert(row < s->h);
	assert(col < s->w);

	return (uint8_t *)row_addr + col;
}

static inline uint32_t *get_pixel32(SDL_Surface *s, int row, int col)
{
	uintptr_t row_addr = (uintptr_t)s->pixels + row * s->pitch;

	assert(row < s->h);
	assert(col < s->w);

	return (uint32_t *)row_addr + col;
}

std::unique_ptr<OffscreenSurface> Font::render(const std::string& line) const
{
	SDL_Color color = { 0, 0, 0 };
	SDL_Surface *s, *raw;
	unsigned int i, j;

	s = TTF_RenderUTF8_Shaded(font, line.c_str(), color, color);
	if (!s)
		return std::unique_ptr<OffscreenSurface>();

	raw = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA,
				   s->w + 2, s->h + 2, 32,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
				   0xff << 8, 0xff << 16, 0xff << 24, 0xff
#else
				   0xff << 16, 0xff << 8, 0xff, 0xff << 24
#endif
				   );

	for (unsigned int row = 0; row < raw->h; row++) {
		for (unsigned int col = 0; col < raw->w; col++) {
			uint8_t center_a, north_a, south_a,
				east_a, west_a, outline_a;

			if (row > 0 && col > 0 && row <= s->h && col <= s->w)
				center_a = *get_pixel8(s, row - 1, col - 1);
			else
				center_a = 0;

			outline_a = center_a;

			if (row >= 2 && col >= 1 && col <= s->w) {
				north_a = *get_pixel8(s, row - 2, col - 1);
				outline_a = std::max(outline_a, north_a);
			}

			if (row <= s->h - 1 && col >= 1 && col <= s->w) {
				south_a = *get_pixel8(s, row, col - 1);
				outline_a = std::max(outline_a, south_a);
			}

			if (row >= 1 && row <= s->h && col <= s->w - 1) {
				east_a = *get_pixel8(s, row - 1, col);
				outline_a = std::max(outline_a, east_a);
			}

			if (row >= 1 && row <= s->h && col >= 2) {
				west_a = *get_pixel8(s, row - 1, col - 2);
				outline_a = std::max(outline_a, west_a);
			}

			*get_pixel32(raw, row, col) =
				(center_a << 16) | (center_a << 8) |
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
				(center_a << 24) | outline_a;
#else
				(outline_a << 24) | center_a;
#endif
		}
	}

	SDL_FreeSurface(s);

	return std::unique_ptr<OffscreenSurface>(new OffscreenSurface(raw));
}
