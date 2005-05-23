#ifndef __EconomyGraph_HPP
#define __EconomyGraph_HPP

#include "gui/Component.hpp"
#include "gui/XmlReader.hpp"

class EconomyGraph : public Component {
public:
    EconomyGraph();
    ~EconomyGraph();

    void parse(XmlReader& reader);
    void draw(Painter& painter);
    
private:

};
#endif
