#pragma once
#include <concurrentqueue.h>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include <atomic>
#include <iostream>
#include <condition_variable>
#include <mutex>

namespace webview
{
    class webview;
}
namespace httplib
{
    class Server;
}
namespace mck
{
    struct Message
    {
        std::string section;
        std::string msgType;
        std::string data;
        Message() : section(""), msgType(""), data("") {}
    };
    void to_json(nlohmann::json &j, const Message &m);
    void from_json(const nlohmann::json &j, Message &m);
    
    class GuiBase
    {
    public:
        virtual void ReceiveMessage(Message &msg) = 0;
    };

    struct GuiSettings
    {
        std::string title = "";
        std::string path = "";
        unsigned port = 9000;
        unsigned height = 1024;
        unsigned width = 1280;
    };

    class GuiWindow
    {
    public:
        GuiWindow();
        ~GuiWindow();
        bool ShowMessageBox(std::string msg);
        bool ShowOpenFileDialog(std::string title, std::string mimeType, std::vector<std::string> &files, bool multi = false);
        bool Show(const GuiSettings &settings);
        bool ShowDebug(const GuiSettings &settings);
        void Close();

        template <typename T>
        bool SendMessage(std::string section, std::string msgType, T &data)
        {
            if (m_isInitialized == false)
            {
                return false;
            }

            auto outMsg = mck::Message();
            outMsg.section = section;
            outMsg.msgType = msgType;
            try
            {
                nlohmann::json j = outMsg;
                j["data"] = data;
                std::string out = "ReceiveMessage(" + j.dump() + ");";
                return Evaluate(out);
            }
            catch (std::exception &e)
            {
                std::cerr << "Failed to convert message to JSON: " << e.what() << std::endl;
                return false;
            }
            return true;
        }
        void ReceiveMessage(std::string msg);
        void SetBasePtr(mck::GuiBase *base);

    private:
        bool Evaluate(std::string msg);
        void SendThread();

        mck::GuiBase *m_base;

        bool m_isInitialized;
        std::atomic<bool> m_done;
        httplib::Server *m_server;
        webview::webview *m_window;
        std::thread m_serverThread;
        std::thread m_windowThread;
        std::thread m_sendThread;

        moodycamel::ConcurrentQueue<std::string> m_sendQueue;
        std::mutex m_sendMutex;
        std::condition_variable m_sendCond;
    };
}