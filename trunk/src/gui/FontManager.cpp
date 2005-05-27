/*
Copyright (C) 2005 Matthias Braun <matze@braunis.de>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include <config.h>

#include "FontManager.hpp"

#include <SDL_ttf.h>
#include <stdexcept>
#include <sstream>

#include "PhysfsStream/PhysfsSDL.hpp"

FontManager* fontManager = 0;

FontManager::FontManager()
{
}

FontManager::~FontManager()
{
    for(Fonts::iterator i = fonts.begin(); i != fonts.end(); ++i)
        TTF_CloseFont(i->second);
}

TTF_Font*
FontManager::getFont(Style style)
{
    FontInfo info;
    info.name = style.font_family;
    info.fontsize = (int) style.font_size;
    info.fontstyle = 0;
    if(style.italic)
        info.fontstyle |= TTF_STYLE_ITALIC;
    if(style.bold)
        info.fontstyle |= TTF_STYLE_BOLD;

    Fonts::iterator i = fonts.find(info);
    if(i != fonts.end())
        return i->second;
    
    std::string fontfile = "fonts/" + info.name + ".ttf";
    TTF_Font* font = TTF_OpenFontRW(getPhysfsSDLRWops(fontfile), 1,
            info.fontsize);
    if(!font) {
        std::stringstream msg;
        msg << "Error opening font '" << fontfile 
            << "': " << SDL_GetError();
        throw std::runtime_error(msg.str());
    }
    if(info.fontstyle != 0)
        TTF_SetFontStyle(font, info.fontstyle);

    fonts.insert(std::make_pair(info, font));
    return font;
}

