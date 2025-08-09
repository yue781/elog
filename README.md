
`elog4cpp` : means that this is a very `easy` to use and very `efficient` in performance c++ logging library. It supports c++11 and above, and is fully cross-platform.

The use of `easy` is reflected in:
* Simple api, you only need to focus on an `elog::Log` class, or static method `Log::<LEVEL>`, or macro definition `ELG_<LEVEL>`.
* Formatting output is simple because the [fmt](https://github.com/fmtlib/fmt) library is used for formatting output.
* Custom formatting is simple because custom `formatter` is supported, and four `formatters` are preconfigured, including defaultFormatter, colorfulFormatter, jsonFormatter, and customFormatter.
* Easy to configure, supports reading configuration items via `json` file with one click.
* Easy to introduce, support `cmake` command to introduce and use the project with one click.

Performance `efficiency` is reflected in:

* The latency of outputting a log is only `180ns` synchronously and `120ns` asynchronously, which is at least 4 times the performance of spdlog.


## Quick Start

### Requirements

* C++11 and above, which is cross-platform

### Installation and Introduction


    1. download the project source code via the git command
        

    2. Add the project to a subproject.

       ```cmake
       add_subdirectory(elog4cpp)
       ```

    3. Link `elog` in the target that needs to use the library.

       ```cmake
       target_link_libraries(target elog)
       ```

