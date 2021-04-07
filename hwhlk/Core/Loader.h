#pragma once
#include <mutex>
#include <memory>
#include <string>

template<typename TSingleton>
class Singleton abstract
{
public:
    static std::shared_ptr<TSingleton> Instance()
    {
        std::unique_lock<std::mutex> lock(s_lock);
        auto instance = s_instance.lock();
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

class BinaryLoader : Singleton<BinaryLoader>
{
private:
    std::wstring m_path;

public:
    BinaryLoader() : m_path(L"") {}
    static std::shared_ptr<BinaryLoader> GetOrCreate() { return BinaryLoader::Instance();  }

    void SetPath(std::wstring path) { m_path = path; }
    std::wstring GetPath() { return m_path; }
};