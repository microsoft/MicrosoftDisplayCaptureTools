#pragma once
#include <mutex>
#include <memory>
#include <string>
#include "winrt/CaptureCard.h"
#include "SampleDisplayCapture.h"
#include "CaptureCard.Controller.h"
#include "CaptureCard.Controller.g.h"

template<typename TSingleton>
class Singleton abstract
{
public:
    static std::shared_ptr<TSingleton> Instance(std::weak_ptr Ctrl_ptr)
    {
        std::unique_lock<std::mutex> lock(s_lock);
        //auto instance = s_instance.lock();
        auto instance = Ctrl_ptr.lock();
        if (nullptr == instance)
        {
            instance = std::make_shared<TSingleton>();
            s_instance = instance;
        }
        return instance;
    }

    Singleton(const Singleton&) = delete;
    const Singleton& operator=(const Singleton&) = delete;

protected:
    Singleton() = default;
    virtual ~Singleton() {};

private:
    static std::mutex s_lock;
    static std::weak_ptr<TSingleton> s_instance;
};

template<typename TSingleton>
std::mutex Singleton<TSingleton>::s_lock;

template<typename TSingleton>
std::weak_ptr<TSingleton> Singleton<TSingleton>::s_instance;


class MethodAccess : Singleton <MethodAccess>
{
private:
    //MethodAccess();
    //winrt::CaptureCard::implementation::Controller MethodAccess::CardController;

public:

    std::shared_ptr<MethodAccess> getMethodInstance(std::weak_ptr<int> Ctrl_ptr) 
    {
        return MethodAccess::Instance(std::weak_ptr Ctrl_ptr);
    }
    //std::shared_ptr<SampleDisplayCapture> capCard_ptr;

    winrt::Windows::Storage::Streams::Buffer FpgaRead()
    {
        return Ctrl_ptr.FpgaRead();
    }
    
};


