#include "KeyOption.h"
#include <client/Minecraft.h>
#include <sstream>

std::string KeyOption::describeKey(Minecraft* minecraft, int keyIndex) {
    if (!minecraft || keyIndex < 0 || keyIndex >= 16 || !minecraft->options.keyMappings[keyIndex]) {
        return "Unbound";
    }

    std::ostringstream ss;
    ss << minecraft->options.keyMappings[keyIndex]->name << ": " << minecraft->options.keyMappings[keyIndex]->key;
    return ss.str();
}

KeyOption::KeyOption(Minecraft* minecraft, int keyIndex)
    : Touch::TButton(keyIndex, describeKey(minecraft, keyIndex)) {}

void KeyOption::mouseClicked(Minecraft* minecraft, int x, int y, int buttonNum) {
    selected = isInside(x, y);
    msg = selected ? "..." : describeKey(minecraft, id);
}

void KeyOption::keyPressed(Minecraft* minecraft, int key) {
    if (!selected || !minecraft) return;

    if (key != Keyboard::KEY_ESCAPE) {
        minecraft->options.setKey(id, key);
    }

    selected = false;
    msg = describeKey(minecraft, id);
}
