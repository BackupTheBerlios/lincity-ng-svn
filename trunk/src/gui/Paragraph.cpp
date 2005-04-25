#include <config.h>

#include "Paragraph.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <ctype.h>
#include <float.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#include "FontManager.hpp"
#include "TextureManager.hpp"
#include "Painter.hpp"
#include "XmlReader.hpp"
#include "ComponentFactory.hpp"
#include "Document.hpp"
#include "TinyGetText/TinyGetText.hpp"

Paragraph::Paragraph()
    : texture(0)
{
}

Paragraph::~Paragraph()
{
    for(TextSpans::iterator i = textspans.begin(); i != textspans.end(); ++i)
        delete *i;
    delete texture;
}

void
Paragraph::parse(XmlReader& reader)
{
    parse(reader, style);
}

void
Paragraph::parse(XmlReader& reader, const Style& parentstyle)
{
    bool translatable = false;
    
    style = parentstyle;
    XmlReader::AttributeIterator iter(reader);
    while(iter.next()) {
        const char* attribute = (const char*) iter.getName();
        const char* value = (const char*) iter.getValue();

        if(parseAttribute(attribute, value)) {
            continue;
        } else if(style.parseAttribute(attribute, value)) {
            continue;
        } else if(strcmp(attribute, "translatable") == 0) {
            translatable = true;
        } else {
            std::cerr << "Skipping unknown attribut '" << attribute << "'.\n";
        }
    }

    std::auto_ptr<TinyGetText::DictionaryManager> dictionaryManager;
    TinyGetText::Dictionary* dictionary = 0;
    if(translatable) {
        dictionaryManager.reset(new TinyGetText::DictionaryManager());
        dictionaryManager->add_directory("locale/gui");
        dictionary = &(dictionaryManager->get_dictionary());
    }
        

    std::vector<Style> stylestack;
    stylestack.push_back(style);

    TextSpan* currentspan = 0;
  
    try {
        std::string currenthref;
        int depth = reader.getDepth();
        while(reader.read() && reader.getDepth() > depth) {
            if(reader.getNodeType() == XML_READER_TYPE_ELEMENT) {
                std::string node((const char*) reader.getName());
                if(node == "span" || node == "i" || node == "b" || node == "a") {
                    if(currentspan != 0) {
                        if(translatable) {
                            currentspan->text 
                                = dictionary->translate(currentspan->text);
                        }
                        textspans.push_back(currentspan);
                        currentspan = 0;
                    }
                    
                    Style style(stylestack.back());
                    if(node == "i")
                        style.italic = true;
                    if(node == "b")
                        style.bold = true;
                    currenthref = "";
                    XmlReader::AttributeIterator iter(reader);
                    while(iter.next()) {
                        const char* attribute = (const char*) iter.getName();
                        const char* value = (const char*) iter.getValue();
                        if(style.parseAttribute(attribute, value))
                            continue;
                        else if(strcmp(attribute, "href") == 0) {
                            currenthref = value;
                        } else {
                            std::cerr << "Unknown attribute '" << attribute
                                << "' in textspan node.\n";
                        }
                    }
                    style.parseAttributes(reader);
                    // TODO parse style attributes...
                    stylestack.push_back(style);       
                } else {
                    std::cerr << "Skipping unknown node '" << node << "'.\n";
                    reader.nextNode();
                }
            } else if(reader.getNodeType() == XML_READER_TYPE_TEXT) {
                if(currentspan == 0) {
                    currentspan = new TextSpan();
                    currentspan->style = stylestack.back();
                    currentspan->href = currenthref;
                }
                
                const char* p = (const char*) reader.getValue();
                // skip trailing spaces...
                while(*p != 0 && isspace(*p))
                    ++p;
            
                bool lastspace = false;
                for( ; *p != 0; ++p) {
                    if(isspace(*p)) {
                        if(!lastspace) {
                            lastspace = true;
                            currentspan->text += ' ';
                        }
                    } else {
                        lastspace = false;
                        currentspan->text += *p;
                    }
                }
            } else if(reader.getNodeType() == XML_READER_TYPE_END_ELEMENT) {
                std::string node((const char*) reader.getName());
                if(node == "span" || node == "b" || node == "i") {
                    if(currentspan != 0) {
                        if(translatable) {
                            currentspan->text 
                                = dictionary->translate(currentspan->text);
                        }
                        textspans.push_back(currentspan);
                        currentspan = 0;
                    }                                                              
                    stylestack.pop_back();
                } else {
                    std::cerr << "Internal error: unknown node end: '" <<
                        node << "'.\n";
                }
            }
        }

        if(currentspan != 0) {
            if(translatable) {
                currentspan->text 
                    = dictionary->translate(currentspan->text);
            }
            textspans.push_back(currentspan);
        }
    } catch(...) {
        if(currentspan != 0)
            delete currentspan;
        throw;
    }

    setFlags(FLAG_RESIZABLE);
}

/**
 * Reflows the text and renders it onto a texture 
 * Cleaning this big code up a bit more is always nice. However be very careful
 * when doing so and test it alot, as the code very easily breaks...
 */
void
Paragraph::resize(float width, float height)
{
    // free old texture
    if(texture) {
        delete texture;
        texture = 0;
    }

    if(width == 0) {
        this->width = this->height = 0;
        return;
    }
    std::auto_ptr<FontManager> fontManager (new FontManager());

    std::vector<int> ycoords;
    std::vector<SDL_Surface*> lineimages;
    std::vector<SDL_Surface*> spanimages;
    std::vector<int> spanbaselines;
    int lineheight = 0;
    int baseline = 0;
    int boxheight = 0;

    TextSpans::iterator i = textspans.begin();
    if(i == textspans.end()) {
        // no need to render anything if there are no spans
        this->width = 0;
        this->height = 0;
        texture = 0;
        return;
    }

    const TextSpan* span = *i;
    const std::string* text = &(span->text);
    TTF_Font* font = fontManager->getFont(span->style);
    std::string::size_type p = 0;
    std::string::size_type linestart = 0;
    lineheight = TTF_FontHeight(font);
    std::string line;
    float linepos = 0;

    while(1) {
        std::string::size_type lastp = p;
        if( (*text) [p] == ' ') {
            // we don't need the space at the beginning of the line
            if(p-linestart != 0 || linepos != 0)
                line += ' ';
            else {
                lastp++;
                linestart++;
            }
            ++p;
        }
            
        // take a word
        for( ; p < text->size() && (*text) [p] != ' '; ++p) {
            line += (*text) [p]; 
        }

        // check line size...
        int render_width, render_height;
        TTF_SizeText(font, line.c_str(), &render_width, &render_height);

        bool render = false;
        bool linefeed = false;
        // we need a linefeed if width isn't enough for current span
        if(width > 0 && linepos + render_width >= width) {
            render = true;
            linefeed = true;
           
            // we have to leave out the last word (which made it too width)
            if(lastp-linestart > 0 || linepos != 0) {
                line = std::string(*text, linestart, lastp-linestart);
                // set new linestart and set p back
                p = lastp;
            }
            linestart = p;
        }

        /* span is over, so we need to render now (and if it was the last span,
         * we need a linefeed too)
         */
        if(p >= text->size()) {
            render = true;
            ++i;
            if(i == textspans.end())
                linefeed = true;
        }
        
        if(render && line != "") {
            if(TTF_FontHeight(font) > lineheight) {
                lineheight = TTF_FontHeight(font);
                baseline = TTF_FontAscent(font);
            }
            
            // render span
            //printf("Rendering: '%s'.\n", line.c_str());
            SDL_Surface* spansurface = TTF_RenderText_Blended(font,
                    line.c_str(), span->style.text_color.getSDLColor());
            if(spansurface == 0) {
                std::stringstream msg;
                msg << "Error rendering text: " << SDL_GetError();
                throw std::runtime_error(msg.str());
            }
            SDL_SetAlpha(spansurface, 0, 0);
            spanimages.push_back(spansurface);
            spanbaselines.push_back(TTF_FontAscent(font));
            
            linepos += spansurface->w;
            line = "";
        }

        // linefeed: compose all span images to line image
        if(linefeed) {
            // compose all spanimages into a line surface
            if(spanimages.size() == 1) {
                lineimages.push_back(spanimages.back());
            } else {
                SDL_Surface* lineimage = SDL_CreateRGBSurface(0, (int) linepos,
                        (int) lineheight, 32,
                        0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
                if(lineimage == 0) {
                    throw std::runtime_error(
                            "Out of memory when composing line image");
                }
                SDL_SetAlpha(lineimage, 0, 0);

                Sint16 x = 0;
                SDL_Rect rect;                
                for(size_t i = 0; i < spanimages.size(); ++i) {
                    rect.x = x;
                    rect.y = baseline - spanbaselines[i];
                    if(rect.y < 0) {
                        printf("smaller... %d %d\n",
                                baseline, spanbaselines[i]);
                        rect.y = 0;
                    }

                    SDL_BlitSurface(spanimages[i], 0, lineimage, &rect);
                    x += spanimages[i]->w;

                    SDL_FreeSurface(spanimages[i]);
                }
                lineimages.push_back(lineimage);
            }
            spanbaselines.clear();
            spanimages.clear();
                            
            ycoords.push_back(boxheight);
            boxheight += lineheight;

            lineheight = TTF_FontHeight(font);
            baseline = TTF_FontAscent(font);
            
            line = "";
            linepos = 0;
        }

        // advance to next span if necessary
        if(p >= text->size()) {
            TTF_CloseFont(font);
            if(i == textspans.end())
                break;
            span = *i;
            text = &(span->text);
            font = fontManager->getFont(span->style);
            linestart = p = 0;
            line = "";
        }
    }

    if(height < 0) {
        height = boxheight;
        if(height < style.min_height) {
            height = style.min_height;
        }
    }
    // check height defined in style
    if(height == 0) {
        this->width = this->height = 0;
        for(std::vector<SDL_Surface*>::iterator i = lineimages.begin();
                i != lineimages.end(); ++i)
            SDL_FreeSurface(*i);
        return;
    }

    /* Step2: compose all lines to the final image */
    if(width < 0) {
        width = lineimages[0]->w;
    }
    SDL_Surface* result = SDL_CreateRGBSurface(0, (int) width, (int) height,
            32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
    if(result == 0) {
        throw std::runtime_error("Out of memory when creating text image");
    }
    for(size_t i = 0; i < lineimages.size(); ++i) {
        SDL_Rect rect;
        if(style.alignment == Style::ALIGN_LEFT) {
            rect.x = (Sint16) 0;
        } else if(style.alignment == Style::ALIGN_CENTER) {
            rect.x = (Sint16) (width - lineimages[i]->w) / 2;
        } else {
            rect.x = (Sint16) (width - lineimages[i]->w);
        }
        rect.y = (Sint16) ycoords[i];
        SDL_BlitSurface(lineimages[i], 0, result, &rect);
        SDL_FreeSurface(lineimages[i]);
    }
    SDL_Surface* surface = SDL_DisplayFormatAlpha(result);
    SDL_FreeSurface(result);
    if(surface == 0) {
        throw std::runtime_error("Out of memory when creating text image(d)");
    }
    
    texture = texture_manager->create(surface);

    this->width = width;
    this->height = height;

    setDirty();
}

void
Paragraph::draw(Painter& painter)
{
    if(!texture)
        return;

    painter.drawTexture(texture, Vector2(0, 0));
}

void
Paragraph::setText(const std::string& newtext)
{
    setText(newtext, style);
}

std::string Paragraph::getText() const
{
  std::string t;
  for(TextSpans::const_iterator i = textspans.begin(); i != textspans.end(); ++i)
    t+=(*i)->text;
  return t;
}


void
Paragraph::setText(const std::string& newtext, const Style& style)
{
    for(TextSpans::iterator i = textspans.begin(); i != textspans.end(); ++i)
        delete *i;
    textspans.clear();
    TextSpan* span = new TextSpan();
    span->style = style;
    span->text = newtext;
    textspans.push_back(span);

    // rerender text
    resize(width, height);

    // eventually trigger resize of parent document
    Document* doc = dynamic_cast<Document*> (getParent());
    if(doc) {
        doc->resize(doc->getWidth(), doc->getHeight());
    }
}

IMPLEMENT_COMPONENT_FACTORY(Paragraph)
