


set(CMAKE_C_FLAGS "-Werror -Wall -Wextra -pedantic -Wcast-align \
                   -Wdisabled-optimization -Wformat=2 \
                   -Winit-self -Wlogical-op -Wmissing-include-dirs \
                   -Wredundant-decls -Wshadow -Wstrict-overflow=4 \
                   -Wundef -Wno-unused -Wno-variadic-macros \
                   -Wno-parentheses -fdiagnostics-show-option")

add_definitions( -DLOG_USE_COLOR )

add_library(logc log.c log.h)