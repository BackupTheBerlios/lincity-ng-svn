#include <config.h>

#include "ComponentFactory.hpp"
#include "XmlReader.hpp"
#include "Desktop.hpp"

#include <sstream>
#include <stdexcept>
#include <iostream>

void initFactories();

ComponentFactories* component_factories = 0;

Component* createComponent(const std::string& type, Component* parent,
        XmlReader& reader)
{
    initFactories();
    
    if(component_factories == 0)
        throw std::runtime_error("No component factories registered");
    
    ComponentFactories::iterator i = component_factories->find(type);
    if(i == component_factories->end()) {
        std::stringstream msg;
        msg << "Couldn't find a component factory for '" << type << "'";
        throw std::runtime_error(msg.str());
    }
   
    try {
        return i->second->createComponent(parent, reader);
    } catch(std::exception& e) {
        std::stringstream msg;
        msg << "Error while parsing component '" << type << "': " << e.what();
        throw std::runtime_error(msg.str());
    } catch(...) {
        throw;
    }
}

Component* loadGUIFile(const std::string& filename)
{
    XmlReader reader(filename);
    
    if(strcmp((const char*) reader.getName(), "gui") != 0) {
        std::stringstream msg;
        msg << "XML File '" << filename << "' is not a gui file.";
        throw std::runtime_error(msg.str());
    }

    Desktop* desktop = new Desktop(0, reader);
    return desktop;
}

Component* parseEmbeddedComponent(Component* parent, XmlReader& reader)
{
    Component* component = 0;
    try {
        int depth = reader.getDepth();
        while(reader.read() && reader.getDepth() > depth) {
            if(reader.getNodeType() == XML_READER_TYPE_ELEMENT) {
                const char* name = (const char*) reader.getName();
                if(component == 0) {
                    component = createComponent(name, parent, reader);
                } else {
                    std::cerr << "Multiple components specified."
                        << " Skipping '" << name << "'.\n";
                    continue;
                }
            }
        }
        if(component == 0) {
            std::cerr << "Warning: "
                 << "No Component specified window title/buttons or contents\n";        }
    } catch(...) {
        delete component;
        throw;
    }

    return component;
}

// import factory
class ImportFactory : public Factory
{
public:
    ImportFactory()
    {
        if(component_factories == 0)
            component_factories = new ComponentFactories;

        component_factories->insert(std::make_pair("Import", this));
    }
    
    Component* createComponent(Component* parent, XmlReader& reader);
};
//static ImportFactory factory_Import;

Component*
ImportFactory::createComponent(Component* parent, XmlReader& reader)
{
    std::string importfile;
    
    XmlReader::AttributeIterator iter(reader);
    while(iter.next()) {
        const char* attribute = (const char*) iter.getName();
        const char* value = (const char*) iter.getValue();

        if(strcmp(attribute, "src") == 0) {
            importfile = value;
        } else {
            std::cerr << "Skipping unknown attribute '" << attribute << "'.\n";
        }
    }

    if(importfile == "")
        throw std::runtime_error("No src attribute specified.");

    XmlReader nreader(importfile);
    return ::createComponent((const char*) nreader.getName(), parent, nreader);
}

//---------------------------------------------------------------------------

/** It seems ar or g++ strip out unused functions in static libraries. For some
 * reasons also global constructor seem to be considered unused if noone uses
 * the global object. So can't use our slick component factory registration
 * tricks :-/ And have to fill in the list manually here
 */

#include "Button.hpp"
#include "Desktop.hpp"
#include "Document.hpp"
#include "Image.hpp"
#include "Panel.hpp"
#include "Paragraph.hpp"
#include "ScrollBar.hpp"
#include "ScrollView.hpp"
#include "TableLayout.hpp"
#include "Window.hpp"

IMPLEMENT_COMPONENT_FACTORY(Button)
IMPLEMENT_COMPONENT_FACTORY(Desktop)
IMPLEMENT_COMPONENT_FACTORY(Document)
IMPLEMENT_COMPONENT_FACTORY(Image)
IMPLEMENT_COMPONENT_FACTORY(Panel)
IMPLEMENT_COMPONENT_FACTORY(Paragraph)
IMPLEMENT_COMPONENT_FACTORY(ScrollBar)
IMPLEMENT_COMPONENT_FACTORY(ScrollView)
IMPLEMENT_COMPONENT_FACTORY(TableLayout)
IMPLEMENT_COMPONENT_FACTORY(Window)

void initFactories()
{
    static bool initialized = false;
    if(!initialized) {
        new INTERN_ButtonFactory();
        new INTERN_DesktopFactory();
        new INTERN_DocumentFactory();
        new INTERN_ImageFactory();
        new INTERN_PanelFactory();
        new INTERN_ParagraphFactory();
        new INTERN_ScrollBarFactory();
        new INTERN_ScrollViewFactory();
        new INTERN_TableLayoutFactory();
        new INTERN_WindowFactory();
        new ImportFactory();
        initialized = true;
    }
}

