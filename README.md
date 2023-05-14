---
ArtifactType: nupkg
Language: c++, csharp, markdown
Platform: windows
Tags: display,testing,taef,displays,graphics
---

# Microsoft Display Capture Tools

This repository contains the source for a generic test framework for validating display adapter devices, such as GPUs and USB-display dongles. It contains a plugin model for physical capture devices that can emulate monitors and receive pixel data, metadata, and other channels common to display protocols.

The goal for this project is to provide a standard for validating display adapters. The framework is designed to be as generic and extensible as possible, to enable a very wide variety of hardware devices, protocols, and tests to be plugged in.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

A test system running Windows 11 Pro or Enterprise.

You will need to install:
1. Visual Studio 2022 with the following components:
    a. .NET 7.0 Runtime
    b. Windows 11 SDK
    c. C++ Core Features
    d. C# and Visual Basic
    e. MSVC v143 - VS 2022 C++ x64/x86 build tools
    f. MSBuild
    g. C++ Universal Windows Platform support for v143 build tools
    h. NuGet package manager

>Note: Installing the Visual Studio 2022 "Workflows" for .Net desktop development, Universal Windows Platform development, and Desktop development with C++ will install all required individual components.

### Installing

A step by step series of examples that tell you how to get a development environment running

1. Clone the repository
2. Open the solution file (HardwareHLK.sln) from an admin Visual Studio 2022 instance.
    a. Due to the nature of the tests, admin is required for most testing situations - See [Starting a Test].
3. Right click on the solution and select "restore nuget packages"
4. Build the solution.

### Starting a Test

This entire code project is meant to be executed as a series of tests using a display device and a display capture device in tandem to verify that the expected display output is received. With the solution built, there are a few steps for running our default test pass:

1. Attach a capture device supporting one of the installed plugins.
    a. Two plugins are initially included
        - CapturePlugins\TanagerPlugin: Designed for a soon-to-be release capture device developed alongside this framework.
        - CapturePlugins\GenericCaptureCardPlugin: Designed for [DirectShow](https://learn.microsoft.com/en-us/windows/win32/directshow/directshow)-compatible USB capture cards.
            - The [Elgato Cam Link](https://www.elgato.com/en/cam-link-4k) is an example of such a device (although this is not an explicit endorsement). Generally... if the capture device appears as a Camera device to Windows, it should work.
            - These capture cards generally cannot emulate an arbitrary display, and as such have an additional setup step: you will need to go into Windows Display Settings and remove the target display from the desktop.
2. Run the Tests project in the solution
    a. By default - this will attempt to automatically determine which display sources correspond to available display sink devices, and will run all available tests on all source-to-sink combinations.

### Troubleshooting

Generally, the command line output of the tests will indicate what the problem is and next steps. Problems can be raised as Github issues or you can [contact the authors](#contact-us).

## Contributing

Please read our [CONTRIBUTING.md](CONTRIBUTING.md) which outlines all of our policies, procedures, and requirements for contributing to this project.

## Versioning and changelog

We use [SemVer](http://semver.org/) for versioning. For a PR to be accepted it will have to include appropriate versioning updates for the impacted components.

We maintain a [ChangeLog](CHANGELOG.md) to be updated alongside any notable changes.

## Authors

David Spruill (Microsoft) - principle author, framework
Katie Anderson (Microsoft) - principle author, Tanager plugin
Dave Hargrove (Microsoft) - hardware design, debugging
Zachary Northrup (Microsoft) - software design, CaptureCardViewer project
Larissa Umulinga (Microsoft) - author, CaptureCardViewer project

Note: Please add to this list as new components/contributors are added.

### Contact Us

Suggestions and feedback can be sent to the authors at hwhlk@microsoft.com

## License

This project is licensed under the MIT license - see the [LICENSE](LICENSE.txt) file for details

## Acknowledgments

* Hat tip to industry partners who helped inspire this project and provided early-stage feedback:
    Intel, Nvidia, AMD and Qualcomm