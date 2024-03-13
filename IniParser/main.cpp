#include "Ini.h"

int main() {
    string str("test.ini");
    Ini ini(str);
    ini.Clear();
    ini.Save("save");
}