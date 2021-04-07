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
Once the tooling is installed, just hitting F5 should start the "basic" test run. This test run is intended for illustrative purposes in seeing how the framework functions and how the various components fit together.

# Contribute
We will happily consider any PRs made against this project. We don't have a firm process for 3rd party contributions set up yet, until then things will be considered on a case-by-case basis.