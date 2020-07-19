#include "Key.hpp"

bool keyToChar(Key key, char* pChar)
{
    char ch = '\0';

    switch (key)
    {
    case Key::Backspace: ch = 0x08; break;
    case Key::Tab: ch = 0x09; break;
    case Key::Enter: ch = 0x0d; break;
    case Key::Esc: ch = 0x1b; break;
    case Key::Space: ch = ' '; break;

    case Key::Key0: ch = '0'; break;
    case Key::Key1: ch = '1'; break;
    case Key::Key2: ch = '2'; break;
    case Key::Key3: ch = '3'; break;
    case Key::Key4: ch = '4'; break;
    case Key::Key5: ch = '5'; break;
    case Key::Key6: ch = '6'; break;
    case Key::Key7: ch = '7'; break;
    case Key::Key8: ch = '8'; break;
    case Key::Key9: ch = '9'; break;

    case Key::A: ch = 'a'; break;
    case Key::B: ch = 'b'; break;
    case Key::C: ch = 'c'; break;
    case Key::D: ch = 'd'; break;
    case Key::E: ch = 'e'; break;
    case Key::F: ch = 'f'; break;
    case Key::G: ch = 'g'; break;
    case Key::H: ch = 'h'; break;
    case Key::I: ch = 'i'; break;
    case Key::J: ch = 'j'; break;
    case Key::K: ch = 'k'; break;
    case Key::L: ch = 'l'; break;
    case Key::M: ch = 'm'; break;
    case Key::N: ch = 'n'; break;
    case Key::O: ch = 'o'; break;
    case Key::P: ch = 'p'; break;
    case Key::Q: ch = 'q'; break;
    case Key::R: ch = 'r'; break;
    case Key::S: ch = 's'; break;
    case Key::T: ch = 't'; break;
    case Key::U: ch = 'u'; break;
    case Key::V: ch = 'v'; break;
    case Key::W: ch = 'w'; break;
    case Key::X: ch = 'x'; break;
    case Key::Y: ch = 'y'; break;
    case Key::Z: ch = 'z'; break;

    case Key::Semicolon: ch = ';'; break;
    case Key::Equals: ch = '='; break;
    case Key::Comma: ch = ','; break;
    case Key::Minus: ch = '-'; break;
    case Key::Period: ch = '.'; break;
    case Key::Slash: ch = '/'; break;
    case Key::Backtick: ch = '`'; break;
    case Key::LeftBracket: ch = '['; break;
    case Key::Backslash: ch = '\\'; break;
    case Key::RightBracket: ch = ']'; break;
    case Key::Apostrophe: ch = '\''; break;

    case Key::Exclamation: ch = '!'; break;
    case Key::At: ch = '@'; break;
    case Key::Pound: ch = '#'; break;
    case Key::Dollar: ch = '$'; break;
    case Key::Percent: ch = '%'; break;
    case Key::Caret: ch = '^'; break;
    case Key::Ampersand: ch = '&'; break;
    case Key::Asterisk: ch = '*'; break;
    case Key::LeftParen: ch = '('; break;
    case Key::RightParen: ch = ')'; break;

    case Key::ShiftA: ch = 'A'; break;
    case Key::ShiftB: ch = 'B'; break;
    case Key::ShiftC: ch = 'C'; break;
    case Key::ShiftD: ch = 'D'; break;
    case Key::ShiftE: ch = 'E'; break;
    case Key::ShiftF: ch = 'F'; break;
    case Key::ShiftG: ch = 'G'; break;
    case Key::ShiftH: ch = 'H'; break;
    case Key::ShiftI: ch = 'I'; break;
    case Key::ShiftJ: ch = 'J'; break;
    case Key::ShiftK: ch = 'K'; break;
    case Key::ShiftL: ch = 'L'; break;
    case Key::ShiftM: ch = 'M'; break;
    case Key::ShiftN: ch = 'N'; break;
    case Key::ShiftO: ch = 'O'; break;
    case Key::ShiftP: ch = 'P'; break;
    case Key::ShiftQ: ch = 'Q'; break;
    case Key::ShiftR: ch = 'R'; break;
    case Key::ShiftS: ch = 'S'; break;
    case Key::ShiftT: ch = 'T'; break;
    case Key::ShiftU: ch = 'U'; break;
    case Key::ShiftV: ch = 'V'; break;
    case Key::ShiftW: ch = 'W'; break;
    case Key::ShiftX: ch = 'X'; break;
    case Key::ShiftY: ch = 'Y'; break;
    case Key::ShiftZ: ch = 'Z'; break;

    case Key::Colon: ch = ':'; break;
    case Key::Plus: ch = '+'; break;
    case Key::Less: ch = '<'; break;
    case Key::Underscore: ch = '_'; break;
    case Key::Greater: ch = '>'; break;
    case Key::Question: ch = '?'; break;
    case Key::Tilde: ch = '~'; break;
    case Key::LeftBrace: ch = '{'; break;
    case Key::VerticalBar: ch = '|'; break;
    case Key::RightBrace: ch = '}'; break;
    case Key::Quote: ch = '"'; break;

    case Key::CtrlBackspace: ch = 0x7f; break;
    case Key::CtrlEnter: ch = 0x0a; break;
    case Key::CtrlSpace: ch = 0x00; break;

    case Key::Ctrl2: ch = 0x00; break;
    case Key::Ctrl6: ch = 0x1e; break;

    case Key::CtrlA: ch = 0x01; break;
    case Key::CtrlB: ch = 0x02; break;
    case Key::CtrlC: ch = 0x03; break;
    case Key::CtrlD: ch = 0x04; break;
    case Key::CtrlE: ch = 0x05; break;
    case Key::CtrlF: ch = 0x06; break;
    case Key::CtrlG: ch = 0x07; break;
    case Key::CtrlH: ch = 0x08; break;
    case Key::CtrlI: ch = 0x09; break;
    case Key::CtrlJ: ch = 0x0a; break;
    case Key::CtrlK: ch = 0x0b; break;
    case Key::CtrlL: ch = 0x0c; break;
    case Key::CtrlM: ch = 0x0d; break;
    case Key::CtrlN: ch = 0x0e; break;
    case Key::CtrlO: ch = 0x0f; break;
    case Key::CtrlP: ch = 0x10; break;
    case Key::CtrlQ: ch = 0x11; break;
    case Key::CtrlR: ch = 0x12; break;
    case Key::CtrlS: ch = 0x13; break;
    case Key::CtrlT: ch = 0x14; break;
    case Key::CtrlU: ch = 0x15; break;
    case Key::CtrlV: ch = 0x16; break;
    case Key::CtrlW: ch = 0x17; break;
    case Key::CtrlX: ch = 0x18; break;
    case Key::CtrlY: ch = 0x19; break;
    case Key::CtrlZ: ch = 0x1a; break;

    case Key::CtrlMinus: ch = 0x1f; break;
    case Key::CtrlLeftBracket: ch = 0x1b; break;
    case Key::CtrlBackslash: ch = 0x1c; break;
    case Key::CtrlRightBracket: ch = 0x1d; break;

    case Key::CtrlAt: ch = 0x00; break;
    case Key::CtrlCaret: ch = 0x1e; break;

    case Key::CtrlShiftA: ch = 0x01; break;
    case Key::CtrlShiftB: ch = 0x02; break;
    case Key::CtrlShiftC: ch = 0x03; break;
    case Key::CtrlShiftD: ch = 0x04; break;
    case Key::CtrlShiftE: ch = 0x05; break;
    case Key::CtrlShiftF: ch = 0x06; break;
    case Key::CtrlShiftG: ch = 0x07; break;
    case Key::CtrlShiftH: ch = 0x08; break;
    case Key::CtrlShiftI: ch = 0x09; break;
    case Key::CtrlShiftJ: ch = 0x0a; break;
    case Key::CtrlShiftK: ch = 0x0b; break;
    case Key::CtrlShiftL: ch = 0x0c; break;
    case Key::CtrlShiftM: ch = 0x0d; break;
    case Key::CtrlShiftN: ch = 0x0e; break;
    case Key::CtrlShiftO: ch = 0x0f; break;
    case Key::CtrlShiftP: ch = 0x10; break;
    case Key::CtrlShiftQ: ch = 0x11; break;
    case Key::CtrlShiftR: ch = 0x12; break;
    case Key::CtrlShiftS: ch = 0x13; break;
    case Key::CtrlShiftT: ch = 0x14; break;
    case Key::CtrlShiftU: ch = 0x15; break;
    case Key::CtrlShiftV: ch = 0x16; break;
    case Key::CtrlShiftW: ch = 0x17; break;
    case Key::CtrlShiftX: ch = 0x18; break;
    case Key::CtrlShiftY: ch = 0x19; break;
    case Key::CtrlShiftZ: ch = 0x1a; break;

    case Key::CtrlUnderscore: ch = 0x1f; break;

    default:
        return false;
    }

    if (pChar)
    {
        *pChar = ch;
    }
    return true;
}
