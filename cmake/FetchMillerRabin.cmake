include(FetchContent)

cmake_policy(SET CMP0169 NEW)  # Suppress deprecated usage warning

FetchContent_Declare(
    miller_rabin
    GIT_REPOSITORY https://github.com/Fudmottin/Miller_Rabin.git
    GIT_TAG main
)

FetchContent_MakeAvailable(miller_rabin)

set(MILLER_RABIN_SOURCE_DIR "${miller_rabin_SOURCE_DIR}")

