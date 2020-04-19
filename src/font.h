// Original font class was replaced by an SDL_ttf based one by Paul Cercueil.
// License: GPL version 2 or later.

#ifndef FONT_H
#define FONT_H

#include <memory>
#include <string>

#include <SDL_ttf.h>

#include "font_spec.h"

class OffscreenSurface;
class Surface;

/**
 * Wrapper around a TrueType or other FreeType-supported font.
 * The wrapper is valid even if the font couldn't be loaded, but in that case
 * nothing will be drawn.
 */
class Font {
public:
	enum HAlign { HAlignLeft, HAlignRight,  HAlignCenter };
	enum VAlign { VAlignTop,  VAlignBottom, VAlignMiddle };

	explicit Font(FontSpec spec);
	~Font();

	int getTextWidth(const std::string& text) const;
	int getTextHeight(const std::string& text) const;

	int getLineSpacing() const
	{
		return lineSpacing;
	}

	/**
	 * Draws a text on a surface in this font.
	 * @return The width of the text in pixels.
	 */
	int write(Surface& surface,
				const std::string &text, int x, int y,
				HAlign halign = HAlignLeft, VAlign valign = VAlignTop) const;

	std::unique_ptr<OffscreenSurface> render(const std::string& text) const;

	const FontSpec& spec() const { return spec_; }

private:
	Font(TTF_Font *font);

	/**
	 * Draws a single line of text on a surface in this font.
	 * @return The width of the text in pixels.
	 */
	int writeLine(Surface& surface, std::string const& text,
				int x, int y, HAlign halign, VAlign valign) const;

	TTF_Font *font;
	int lineSpacing;
	FontSpec spec_;
};

#endif /* FONT_H */
