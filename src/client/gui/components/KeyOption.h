#pragma once
#include "Button.h"
#include <client/Options.h>

class KeyOption : public Touch::TButton {
public:
    KeyOption(Minecraft* minecraft, int keyIndex);

    virtual void mouseClicked(Minecraft* minecraft, int x, int y, int buttonNum);
    virtual void released(int mx, int my) {}
    virtual void keyPressed(Minecraft* minecraft, int key);
private:
    static std::string describeKey(Minecraft* minecraft, int keyIndex);
};
