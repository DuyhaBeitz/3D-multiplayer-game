FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG        12.1.0
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
    EasyNet
    GIT_REPOSITORY https://github.com/DuyhaBeitz/EasyNet.git
    GIT_TAG main
    UPDATE_COMMAND ""
)
FetchContent_MakeAvailable(EasyNet)