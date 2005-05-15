#include <config.h>

#include "PainterSDL.hpp"

#include <iostream>
#include <assert.h>
#include <math.h>
#include <typeinfo>
#include <SDL_opengl.h>
#include <SDL_gfxPrimitives.h>
#include <SDL_rotozoom.h>

#include "TextureSDL.hpp"

#ifdef _MSC_VER
#define lrint(x) (long int)x
#define alloca _alloca
#endif

PainterSDL::PainterSDL(SDL_Surface* _target)
    : target(_target)
{
}

PainterSDL::PainterSDL(TextureSDL* texture)
    : target(texture->surface)
{
}

PainterSDL::~PainterSDL()
{
}

void
PainterSDL::drawTexture(const Texture* texture, const Vector2& pos)
{
    assert(typeid(*texture) == typeid(TextureSDL));
    const TextureSDL* textureSDL = static_cast<const TextureSDL*> (texture);
    
    if(texture == 0) {
        std::cerr << "Trying to render 0 texture.";
#ifdef DEBUG
        assert(false);
#endif
        return;
    }

    Vector2 screenpos = transform.apply(pos);
    
    SDL_Rect drect;
    drect.x = lrint(screenpos.x);
    drect.y = lrint(screenpos.y);
    SDL_BlitSurface(textureSDL->surface, 0, target, &drect);
}

void
PainterSDL::drawStretchTexture(const Texture* texture, const Rect2D& rect)
{
    assert(typeid(*texture) == typeid(TextureSDL));
    const TextureSDL* textureSDL = static_cast<const TextureSDL*> (texture);
    
    if(texture == 0 || texture->getWidth() == 0 || texture->getHeight() == 0) {
        std::cerr << "Trying to render 0 texture.";
#ifdef DEBUG
        assert(false);
#endif
        return;
    }

    Vector2 screenpos = transform.apply(rect.p1);
    
    SDL_Rect drect;
    drect.x = lrint(screenpos.x);
    drect.y = lrint(screenpos.y);
    // kinda hacky... but zoomSurface sometimes produces 1 pixel too small
    // images
    drect.w = lrint(rect.getWidth()) + 1;
    drect.h = lrint(rect.getHeight()) + 1;

    double zoomx = drect.w / textureSDL->getWidth();
    double zoomy = drect.h / textureSDL->getHeight(); 
    SDL_Surface *tmp 
        = zoomSurface(textureSDL->surface, zoomx, zoomy, SMOOTHING_OFF);
    SDL_BlitSurface(tmp, 0, target, &drect);
    SDL_FreeSurface(tmp);
}


void 
PainterSDL::fillPolygon(int numberPoints, const Vector2* points)
{
    Vector2 screenpos;
    Sint16* vx = (Sint16*) alloca(numberPoints * sizeof(*vx));
    Sint16* vy = (Sint16*) alloca(numberPoints * sizeof(*vy));
    for(int i = 0; i < numberPoints; i++ ) {
         screenpos = transform.apply( points[ i ] );
         vx[ i ] = (int) screenpos.x;
         vy[ i ] = (int) screenpos.y;
    }
    filledPolygonRGBA( target, vx, vy, numberPoints,
            fillColor.r, fillColor.g, fillColor.b, fillColor.a);
}

void
PainterSDL::drawPolygon(int numberPoints, const Vector2* points)
{
    Vector2 screenpos;
    Sint16* vx = (Sint16*) alloca(numberPoints * sizeof(*vx));
    Sint16* vy = (Sint16*) alloca(numberPoints * sizeof(*vy));
    for(int i = 0; i < numberPoints; i++ ) {
         screenpos = transform.apply( points[ i ] );
         vx[ i ] = (int) screenpos.x;
         vy[ i ] = (int) screenpos.y;
    }
    aapolygonRGBA( target, vx, vy, numberPoints,
            lineColor.r, lineColor.g, lineColor.b, lineColor.a);
}


void
PainterSDL::fillRectangle(const Rect2D& rect)
{
    Vector2 screenpos = transform.apply(rect.p1);
    Vector2 screenpos2 = transform.apply(rect.p2);
    boxRGBA(target, (int) screenpos.x, (int) screenpos.y,
            (int) screenpos2.x, (int) screenpos2.y,
            fillColor.r, fillColor.g, fillColor.b, fillColor.a);
}

void
PainterSDL::drawRectangle(const Rect2D& rect)
{
    Vector2 screenpos = transform.apply(rect.p1);
    Vector2 screenpos2 = transform.apply(rect.p2);
    rectangleRGBA(target, (int) screenpos.x, (int) screenpos.y,
            (int) screenpos2.x, (int) screenpos2.y,
            lineColor.r, lineColor.g, lineColor.b, lineColor.a);
}

void
PainterSDL::setFillColor(Color color)
{
    fillColor = color;
}

void
PainterSDL::setLineColor(Color color)
{
    lineColor = color;
}

void
PainterSDL::translate(const Vector2& vec)
{
    transform.translation -= vec;
}

void
PainterSDL::pushTransform()
{
    transformStack.push_back(transform);
}

void
PainterSDL::popTransform()
{
    transform = transformStack.back();
    transformStack.pop_back();
}

void
PainterSDL::setClipRectangle(const Rect2D& rect)
{
    Vector2 screenpos = transform.apply(rect.p1);
    SDL_Rect cliprect;
    cliprect.x = (int) screenpos.x;
    cliprect.y = (int) screenpos.y;
    cliprect.w = (int) rect.getWidth();
    cliprect.h = (int) rect.getHeight();
    SDL_SetClipRect(target, &cliprect);
}

void
PainterSDL::clearClipRectangle()
{
    SDL_SetClipRect(target, 0);
}

Painter*
PainterSDL::createTexturePainter(Texture* texture)
{
    assert(typeid(*texture) == typeid(TextureSDL));
    TextureSDL* textureSDL = static_cast<TextureSDL*> (texture);    
    
    return new PainterSDL(textureSDL);
}

