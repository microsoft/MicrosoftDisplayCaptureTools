========================================================================
              GenericCaptureCardPlugin Project Overview
========================================================================

This project provides an implementation of the capture card interfaces
for the Microsoft Display Capture Tests, specifically for generic off-
the-shelf USB display capture cards which present as camera devices to
Windows. This project will allow the testing framework to discover such
devices and expose them to tests/utilities consuming the framework. 

Note that these devices typically cannot capture full-range, full-
fidelity screen captures - and so are limited in which tests they can
successfully run. As such, this implementation is provided chiefly as
a demonstration and example.