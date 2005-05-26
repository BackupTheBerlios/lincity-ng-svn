#include <config.h>

#include "TextureManagerGL.hpp"

#include <SDL_opengl.h>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "TextureGL.hpp"

TextureManagerGL::TextureManagerGL()
{
}

TextureManagerGL::~TextureManagerGL()
{
}

static int powerOfTwo(int val) {
    int result = 1;
    while(result < val)
        result *= 2;
    return result;
}

Texture*
TextureManagerGL::create(SDL_Surface* image)
{
    int texture_w = powerOfTwo(image->w);
    int texture_h = powerOfTwo(image->h);

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    SDL_Surface* convert = SDL_CreateRGBSurface(SDL_SWSURFACE,
            texture_w, texture_h, 32,
            0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
#else
    SDL_Surface* convert = SDL_CreateRGBSurface(SDL_SWSURFACE,
        texture_w, texture_h, 32,
        0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
#endif
    if(convert == 0)
        throw std::runtime_error("Couldn't create texture: out of memory");
    SDL_SetAlpha(image, 0, 0);
    SDL_BlitSurface(image, 0, convert, 0);

    unsigned int handle;
    glGenTextures(1, &handle);

    SDL_PixelFormat* format = convert->format;

    glBindTexture(GL_TEXTURE_2D, handle);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, convert->pitch/format->BytesPerPixel);
    glTexImage2D(GL_TEXTURE_2D, 0, format->BytesPerPixel,
            convert->w, convert->h, 0, GL_RGBA,
            GL_UNSIGNED_BYTE, convert->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP);

    TextureGL* texture = new TextureGL(handle);
    texture->rect = Rect2D(0, 0,
            (float) image->w/(float) texture_w,
            (float) image->h/(float) texture_h);
    texture->width = image->w;
    texture->height = image->h;

    SDL_FreeSurface(image);
    SDL_FreeSurface(convert);
    return texture;
}

