---
ArtifactType: nupkg, executable, azure-web-app, azure-cloud-service, etc. More requirements for artifact type standardization may come later.
Documentation: URL
Language: typescript, csharp, java, js, python, golang, powershell, markdown, etc. More requirements for language names standardization may come later.
Platform: windows, node, linux, ubuntu16, azure-function, etc. More requirements for platform standardization may come later.
Stackoverflow: URL
Tags: comma,separated,list,of,tags
---

# Microsoft Display Capture Tools

This repository contains the source for a generic test framework for validating display adapter devices, such as GPUs and USB-to-HDMI dongles. It contains a plugin model for physical capture devices that can emulate monitors and receive pixel data, metadata, and other channels common to display protocols.

The goal for this project is to provide an industry-wide standard for validating display adapters. The framework is designed to be as generic and extensible as possible, to enable a very wide variety of hardware devices, protocols, and tests to be plugged in.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.
1. Clone the repo
2. Restore Nuget Dependencies
3. Do a complete solution build.

### Prerequisites

You will need to install:
1. Visual Studio 2022

### Installing

A step by step series of examples that tell you how to get a development environment running

1. Describe what needs to be done first

    ``` batch
    Give an example of performing step 1
    ```

2. And then repeat for each step

    ``` sh
    Another example, this time for step 2
    ```

## Running the tests

Explain how to run the tests for this project that are relevant to users. You can also link to the testing portion of [CONTRIBUTING.md](CONTRIBUTING.md) for tests relevant to contributors.

### End-to-end tests

Explain what these tests test and why

```
Give an example
```

### Unit tests

Explain what these test and why

```
Give examples
```

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
Once the tooling is installed, just hitting F5 should start the "basic" test run. This test run is intended for illustrative purposes in seeing how the framework functions and how the various components fit together. The AddressSanitizer for Windows will be required to run the project for memory detector issues. These are shown by Tl.exe having different breakpoints, heap errors, or exceptions thrown at different lines of the code. In order to install it, follow the instruction under the title “Turning on Asan for Windows MSBuild projects” from [this documentation] AddressSanitizer (ASan) for Windows with MSVC | C++ Team Blog (microsoft.com). Once installed, the incremental linking needs to be disabled through Solutions Explorer menu > right click on Core(Desktop) > Properties > Linker (under General) > choose No(/INCREMENTAL:NO) on the pop up menu next to Enable Incremental Linking > and click on Apply. On the same Core Property Pages menu, select C/C++ (under General), select None next to Debug Information Format. As for the Warning Level, select Level3 (/W3).

# Contribute
We will happily consider any PRs made against this project. We don't have a firm process for 3rd party contributions set up yet, until then things will be considered on a case-by-case basis.
