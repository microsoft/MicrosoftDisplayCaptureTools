# Contributing

This project welcomes contributions and suggestions. Most contributions require you to
agree to a Contributor License Agreement (CLA) declaring that you have the right to,
and actually do, grant us the rights to use your contribution. For details, visit
https://cla.microsoft.com.

When you submit a pull request, a CLA-bot will automatically determine whether you need
to provide a CLA and decorate the PR appropriately (e.g., label, comment). Simply follow the
instructions provided by the bot. You will only need to do this once across all repositories using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/)
or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

## Licensing

All code in this repo is licensed with a standard MIT license, the text of which can be found [here](LICENSE.txt). No changes will be considered which cannot be included under this license.

## Policies

In order to maintain clarity and direction - we will only accept pull requests that include appropriate updates to the [changelog](CHANGELOG.md) and stick to our semantic versioning.

### Formatting

A [clang-format](https://clang.llvm.org/docs/ClangFormat.html) [file](.clang-format) is included in this project and all contributed code should conform to that formatting.

## Issues

Issues must be raised within this github project to be considered.

### Issues we will consider

* Bugs identified in the framework, such as rendering an incorrect image for a particular parameter combination.
    * Please include details of the bug repro setup, including test machine configuration, test details and both the predicted & captured output.
* Suggestions of new "tools"/parameters to be added to the combinatoric framework.
* Suggestions of APIs/DDIs to be tested that are either not tested, or tested in a different way.
* Suggestions of new APIs in the test framework to enable new tools or enable framework use in a new context.

### Issues we won't consider

* Test failures that could be due to the hardware legitimately failing the test, without a detailed investigation that identified an actual framework bug.
* Requests for HLK waivers (please contact your Microsoft partner representative for requesting waivers)