#ifndef FPVUE_UDPRECEIVER_H
#define FPVUE_UDPRECEIVER_H

#include <jni.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <atomic>
#include <cstdio>
#include <iostream>
#include <thread>
#include <mutex>

class UDPReceiver
{
  public:
    typedef std::function<void(const uint8_t[], size_t)> DATA_CALLBACK;
    typedef std::function<void(const std::string)>       SOURCE_IP_CALLBACK;

  public:
    /**
     * @param bindAddr : Interface address to bind to. "" or "0.0.0.0" means
     *                   any interface (INADDR_ANY). Otherwise a dotted IPv4
     *                   string, e.g. "192.168.1.10".
     */
    UDPReceiver(
        JavaVM*       javaVm,
        std::string   bindAddr,
        int           port,
        std::string   name,
        int           CPUPriority,
        DATA_CALLBACK onDataReceivedCallback,
        size_t        WANTED_RCVBUF_SIZE = 0);

    void registerOnSourceIPFound(SOURCE_IP_CALLBACK onSourceIP1);
    void startReceiving();
    void stopReceiving();

    long        getNReceivedBytes() const;
    std::string getSourceIPAddress() const;
    int         getPort() const;
    std::string getBindAddr() const { return mBindAddr; }

    void setForwarding(const std::string& ip, int port, bool enabled);

  private:
    void receiveFromUDPLoop();

    const DATA_CALLBACK onDataReceivedCallback = nullptr;
    SOURCE_IP_CALLBACK  onSourceIP             = nullptr;
    const std::string   mBindAddr;
    const int           mPort;
    const int           mCPUPriority;
    const size_t        WANTED_RCVBUF_SIZE;
    const std::string   mName;

    int                          mSocket        = 0;
    std::string                  senderIP       = "0.0.0.0";
    std::atomic<bool>            receiving      = false;
    std::atomic<long>            nReceivedBytes = 0;
    std::unique_ptr<std::thread> mUDPReceiverThread;
    // https://en.wikipedia.org/wiki/User_Datagram_Protocol
    // 65,507 bytes (65,535 − 8 byte UDP header − 20 byte IP header).

    static constexpr const size_t UDP_PACKET_MAX_SIZE = 65507;
    JavaVM*                       javaVm;

    std::mutex                    mForwardMutex;
    std::string                   mForwardIP      = "";
    int                           mForwardPort    = 0;
    bool                          mForwardEnabled = false;
    struct sockaddr_in            mDestAddr;
};

#endif  // FPVUE_UDPRECEIVER_H