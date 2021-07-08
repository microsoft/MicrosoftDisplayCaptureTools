#pragma once
#include <mutex>
#include <memory>
#include <string>
#include "Singleton.h"
#include "winrt/CaptureCard.h"
#include "SampleDisplayCapture.h"
#include "CaptureCard.Controller.h"
#include "CaptureCard.Controller.g.h"


class MethodAccess :
    public Controller,
    public std:: enable_shared_from_this<MethodAccess>,
    public Singleton <MethodAccess>

{
private:
    //MethodAccess();
    //winrt::CaptureCard::implementation::Controller MethodAccess::CardController;

public:

    std::shared_ptr<MethodAccess> getMethodInstance(std::weak_ptr<winrt::CaptureCard::implementation::Controller> Ctrl_ptr)
    {
        return MethodAccess::Instance(Ctrl_ptr);
    }

    winrt::Windows::Storage::Streams::Buffer FpgaRead()
    {
        return Ctrl_ptr.FpgaRead();
    }
    
};


