#include <GuiWindow.hpp>
#include <cstdio>
#include <filesystem>

namespace fs = std::filesystem;

class GuiReceiver : public mck::GuiBase
{
public:
    void ReceiveMessage(mck::Message &msg) override
    {
        if (msg.msgType == "log")
        {
            std::printf("New log message: %s\n", msg.data.c_str());
        }
        else if (msg.msgType == "cmd" && msg.section == "files" && msg.data == "getCurrentDir")
        {
            if (guiPtr != nullptr)
            {
                std::string curPath = fs::current_path().string();
                guiPtr->SendMessage("files", "currentDir", curPath);
            }
        }
    }

    void SetGuiPtr(mck::GuiWindow *gui)
    {
        guiPtr = gui;
    }

private:
    mck::GuiWindow *guiPtr{nullptr};
};

int main(int argc, char **argcv)
{
    GuiReceiver receiver;

    mck::GuiWindow guiWin;
    mck::GuiSettings settings;
    settings.title = "Hello World";
    settings.path = "./www";
    settings.height = 600;
    settings.width = 800;

    receiver.SetGuiPtr(&guiWin);
    guiWin.SetBasePtr(&receiver);
    guiWin.Show(settings);

    return 0;
}