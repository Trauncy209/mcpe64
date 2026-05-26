#include "TextOption.h"
#include <client/Minecraft.h>

TextOption::TextOption(Minecraft* minecraft, int optId)
    : TextBox(optId, "Option")
{
    (void)minecraft;
}

bool TextOption::loseFocus(Minecraft* minecraft) {
    (void)minecraft;
    return TextBox::loseFocus(minecraft);
}
