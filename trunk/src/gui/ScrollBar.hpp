#ifndef __SCROLLBAR_HPP__
#define __SCROLLBAR_HPP__

#include "Component.hpp"
#include "ComponentHolder.hpp"
#include <sigc++/signal.h>

class XmlReader;
class Button;

class ScrollBar : public Component
{
public:
    ScrollBar(Component* parent, XmlReader& reader);
    virtual ~ScrollBar();

    void resize(float width, float height);
    void draw(Painter& painter);
    void event(const Event& event);

    void setRange(float min, float max);

    sigc::signal<void, ScrollBar*, float> signalValueChanged;

private:
    Child& button1()
    { return childs[0]; }
    Child& button2()
    { return childs[1]; }
    Child& scroller()
    { return childs[2]; }
    
    float minVal;
    float maxVal;
    float currentVal;

    bool scrolling;
    float scrollOffset;
};

#endif

