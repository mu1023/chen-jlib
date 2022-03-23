// TestServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <IocpNetMgr.h>
#include <Logging.h>
using namespace cj;
bool m_Run = true;
class TestClientNetCallBack : public NetCallBack
{
public:
    virtual bool OnNetConneted(std::shared_ptr<ConnSock> pNewConnSock)
    {
        std::cout << "new ConnSock " << pNewConnSock->GetAllocID() << " " << pNewConnSock->GetFd() << std::endl;
        m_Conn = pNewConnSock;

        return true;
    }
    virtual bool OnNetRecv(UInt32 iAllodID, const NetBuffer& rNetBuffer)
    {
        std::string str(rNetBuffer.Peek(), rNetBuffer.ReadableBytes());
        std::cout << str << std::endl;
        return true;
    };
    virtual bool OnNetDisConnected(UInt32 allocID)
    {
        if (m_Conn == nullptr)
        {
            std::cout << "ConnSock DisConnected. not find " << allocID << std::endl;
        }
        else
        {
            std::cout << "ConnSock DisConnected " << allocID << std::endl;

        }
        m_Conn = nullptr;
        return true;
    }
    void Update()
    {
        if (m_Conn)
        {
            UInt32 now = GetTickCount();
            if (now - m_LastTime > 1000)
            {
                m_NetMgr->SendData(m_Conn, "wtf", 3);
                m_LastTime = now;
                m_TryCnt--;
            }
            if (m_TryCnt <= 0)
            {
                m_NetMgr->DisconnectConn(m_Conn->GetAllocID());
                m_Conn = nullptr;
                m_Run = false;
            }
        }
    }
    std::shared_ptr<ConnSock>  m_Conn;
    IocpNetMgr* m_NetMgr;
    int m_TryCnt;
    UInt64 m_LastTime;
};
int main()
{
    std::shared_ptr<cj::AppenderBase> consoleAppender = std::make_shared<cj::ConsoleAppender<cj::NullMutex>>();
    cj::Logger::Instance()->InsertAppenderPtr(consoleAppender);
    LOGGER_DEBUG("{0},{1}", "das", 1);

    SOCKET_STARTUP
    IocpNetMgr client;
    TestClientNetCallBack   cb;
    client.Initialize(&cb, 4);
    client.Connect("127.0.0.1",10088);

    cb.m_NetMgr = &client;
    cb.m_TryCnt = 100;
    while (m_Run)
    {
        client.Upadete();
        cb.Update();
        Sleep(1);
    }
    SOCKET_CLEANUP
    return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
