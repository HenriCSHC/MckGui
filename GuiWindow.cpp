#include "GuiWindow.hpp"
#include "webview/webview.h"
#include "cpp-httplib/httplib.h"

void MsgFromGui(std::string idx, std::string msg, void *arg)
{
    mck::GuiWindow *win = (mck::GuiWindow *)arg;
    #ifdef DEBUG
        std::cout << "Msg from GUI: " << idx << " : " << msg << std::endl;
    #endif

    win->ReceiveMessage(msg);

    return;
}


void mck::to_json(nlohmann::json &j, const Message &m)
{
    j["section"] = m.section;
    j["msgType"] = m.msgType;
    j["data"] = m.data;
}
void mck::from_json(const nlohmann::json &j, Message &m)
{
    m.section = j.at("section").get<std::string>();
    m.msgType = j.at("msgType").get<std::string>();
    m.data = j.at("data").get<std::string>();
}

mck::GuiWindow::GuiWindow()
    : m_isInitialized(false),m_done(false)
{
    m_server = new httplib::Server();
    m_window = new webview::webview(true, nullptr);
    m_sendQueue = moodycamel::ConcurrentQueue<std::string>(512);
}

mck::GuiWindow::~GuiWindow()
{
    if (m_isInitialized)
    {
        Close();
    }
    delete m_server;
    delete m_window;
}

bool mck::GuiWindow::Show(std::string title, std::string path, unsigned port)
{
    if (m_isInitialized == true)
    {
        std::cerr << "GuiWindow is already shown" << std::endl;
        return false;
    }

    // HTML Server
    bool ret = m_server->set_mount_point("/", path.c_str());
    if (ret == false)
    {
        std::cerr << "Failed to set mount point to path " << path << "! Does the path exist?" << std::endl;
        return false;
    }
    m_serverThread = std::thread([this, &port]() {
        m_server->listen("localhost", port);
    });

    // Send Thread
    //m_sendThread = std::thread(&mck::GuiWindow::SendThread, this);

    m_window->set_title(title);
    m_window->set_size(1280, 720, WEBVIEW_HINT_NONE);
    m_window->navigate("http://localhost:" + std::to_string(port));
    m_window->bind("SendMessage", MsgFromGui, (void *)this);

    m_isInitialized = true;
    m_window->run();

    return true;
}

bool mck::GuiWindow::ShowDebug(std::string title, unsigned port)
{
    if (m_isInitialized == true)
    {
        std::cerr << "GuiWindow is already shown" << std::endl;
        return false;
    }

    m_window->set_title(title);
    m_window->set_size(1280, 720, WEBVIEW_HINT_NONE);
    m_window->navigate("http://localhost:" + std::to_string(port));
    m_window->bind("SendMessage", MsgFromGui, (void *)this);

    m_isInitialized = true;
    m_window->run();

    return true;
}
void mck::GuiWindow::Close()
{
    m_done = true;
    if (m_windowThread.joinable())
    {
        m_window->terminate();
        m_windowThread.join();
    }
    if (m_serverThread.joinable())
    {
        m_server->stop();
        m_serverThread.join();
    }
    if (m_sendThread.joinable())
    {
        m_sendThread.join();
    }
    m_isInitialized = false;
}
void mck::GuiWindow::ReceiveMessage(std::string msg)
{
    std::vector<mck::Message> mckMsgs;
    try
    {
        nlohmann::json j = nlohmann::json::parse(msg);
        mckMsgs = j.get<std::vector<mck::Message>>();
    }
    catch (std::exception &e)
    {
        return;
    }
    if (m_base != nullptr)
    {
        for(auto &m : mckMsgs) {
            m_base->ReceiveMessage(m);
        }
    }
}
void mck::GuiWindow::SetBasePtr(mck::GuiBase *base)
{
    m_base = base;
}

bool mck::GuiWindow::Evaluate(std::string msg)
{
    if (m_isInitialized == false)
    {
        return false;
    }

    m_window->dispatch([&, msg]() {
        m_window->eval(msg);
    });

    return true;

    //return m_sendQueue.try_enqueue(msg);
}

void mck::GuiWindow::SendThread()
{
    while (true)
    {
        if (m_done.load())
        {
            break;
        }
        std::string msg = "";
        while (m_sendQueue.try_dequeue(msg))
        {
            m_window->dispatch([&]() {
                m_window->eval(msg);
            });
            //m_window->eval(msg);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}