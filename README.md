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
    a. Due to the nature of the tests, admin is required for most testing situations - See [Running tests].
3. Right click on the solution and select "restore nuget packages"
4. Build the solution.

## Running tests

This entire code project is meant to be executed as a series of tests using a display device and a display capture device in tandem to verify that the expected display output is received.

## Deployment

Add additional notes about how to deploy this on a live system

## Built With

Documenting some of the main tools used to build this project, manage dependencies, etc will help users get more information if they are trying to understand or having difficulties getting the project up and running.

* Link to some dependency manager
* Link to some framework or build tool
* Link to some compiler, linting tool, bundler, etc

## Contributing

Please read our [CONTRIBUTING.md](CONTRIBUTING.md) which outlines all of our policies, procedures, and requirements for contributing to this project.

## Versioning and changelog

We use [SemVer](http://semver.org/) for versioning. For the versions available, see the [tags on this repository](link-to-tags-or-other-release-location).

It is a good practice to keep `CHANGELOG.md` file in repository that can be updated as part of a pull request.

## Authors

List main authors of this project with a couple of words about their contribution.

Also insert a link to the `owners.txt` file if it exists as well as any other dashboard or other resources that lists all contributors to the project.

## License

This project is licensed under the < INSERT LICENSE NAME > - see the [LICENSE](LICENSE) file for details

## Acknowledgments

* Hat tip to anyone whose code was used
* Inspiration
* etc


# Introduction 
This repository contains the source for a generic test framework for validating display adapter devices, such as GPUs and USB-to-HDMI dongles. It contains a plugin model for physical capture devices that can emulate monitors and receive pixel data, metadata, and other channels common to display protocols.

The goal for this project is to provide an industry-wide standard for validating display adapters. The framework is designed to be as generic and extensible as possible, to enable a very wide variety of hardware devices, protocols, and tests to be plugged in.

Specifications for the framework are available on the Microsoft-internal CGA-Internal Git repo [here](https://dev.azure.com/cga-internal/docs/_wiki/wikis/Docs/217/Overview).

# Overview
An overview of this project and how it functions can be found [here](https://dev.azure.com/cga-exchange/_git/docs?path=%2Fdisplay%2FHardwareHlk%2FTests.md).

# Getting Started
## Install Tooling
This testing framework makes use of the [Testing Authoring and Execution Framework (TAEF)](https://docs.microsoft.com/en-us/windows-hardware/drivers/taef/) used in much of Microsoft's internal and HLK testing. In order to run this project, you will need to have TAEF installed. Follow the steps (1, 1.5, and 2) from [this documentation](https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk) to get set up.

## Running a Test
Once the project is building locally - you can start running display loopback tests just by running the "Tests" project. This project will automatically attempt to discover the capture hardware you are using and start running an automated combinatorial test using that hardware. If 

# Contribute
We will happily consider any PRs made against this project. We don't have a firm process for 3rd party contributions set up yet, until then things will be considered on a case-by-case basis.
