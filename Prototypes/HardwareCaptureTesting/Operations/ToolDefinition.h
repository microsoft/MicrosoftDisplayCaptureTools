#pragma once

namespace winrt::HardwareCaptureTesting::Operations
{
    class Tool
    {
    public:

        // Create
        // -------------------------------------------------------------------------------------------------
        // Instantiates a tool
        // -------------------------------------------------------------------------------------------------
        // Parameters:
        //   name - The name of the tool to create.
        //
        static Tool* Create(winrt::hstring name);

        virtual ~Tool() {}

        virtual winrt::hstring GetName() = 0;
        virtual ToolCategory GetCategory() = 0;

        // ValidateParameters
        // -------------------------------------------------------------------------------------------------
        // Validates that the tool can be used right now
        // -------------------------------------------------------------------------------------------------
        // Parameters:
        //   params - the PICT-provided string indicating the desired state for this tool
        //
        virtual HRESULT ValidateParameters(winrt::hstring params) = 0;

        // Run
        // -------------------------------------------------------------------------------------------------
        // Exercises the tool
        // -------------------------------------------------------------------------------------------------
        // Parameters:
        //   toolName   - The name of the tool to create.
        //   toolParams - The PICT-specificed parameter to supply to this tool
        //
        virtual HRESULT Run(winrt::hstring params) = 0;

    };
}