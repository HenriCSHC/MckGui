# MckGui
Wrapper class for webview / gtk2webkit and cpp-httplib

## Dependencies

### Debian / Ubuntu
```
sudo apt install libgtk-3-dev libwebkit2gtk-4.0-dev
```

### Fedora
```
sudo dnf install gtk3-devel webkit2gtk3-devel
```

## Usage

Add GuiWindow.hpp to your program with ```#include <GuiWindow.hpp>```

Add following compiler flags ```-IMckGui -IMckGui/json/include `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0` ```

