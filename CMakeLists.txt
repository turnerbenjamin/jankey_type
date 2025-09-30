cmake_minimum_required(VERSION 3.16)
project(jankey_type C)

# Set C standard
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Set build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Find ncurses library
find_package(PkgConfig REQUIRED)
pkg_check_modules(NCURSES REQUIRED ncurses)

# Collect all .c files from src directory
set(SOURCES 
    "src/main.c"
    "src/typing_test.c"
    "src/word_store.c"
    "src/err.c"
    "src/helpers.c"
)

set(HEADERS
    "src/constants.h"
    "src/typing_test.h"
    "src/word_store.h"
    "src/helpers.h"
    "src/err.h"
)

# Create executable
add_executable(out ${SOURCES} ${HEADERS})

# Set output directory
set_target_properties(out PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Strict compilation flags
target_compile_options(out PRIVATE
    -g
    -Werror
    -Wall
    -Wextra                    # Extra warnings
    -Wpedantic                 # Strict ISO C compliance
    -Wformat=2                 # Format string security
    -Wformat-security          # Format string security
    -Wnull-dereference         # Null pointer dereference
    -Wstack-protector          # Stack protection
    -Wtrampolines              # Trampolines usage
    -Walloca                   # Alloca usage
    -Wvla                      # Variable length arrays
    -Warray-bounds=2           # Array bounds checking
    -Wimplicit-fallthrough=3   # Switch fallthrough
    -Wtraditional-conversion   # Traditional C conversions
    -Wstrict-prototypes        # Strict function prototypes
    -Wold-style-definition     # Old style function definitions
    -Wmissing-prototypes       # Missing function prototypes
    -Wmissing-declarations     # Missing declarations
    -Wredundant-decls          # Redundant declarations
    -Wnested-externs           # Nested extern declarations
    -Winline                   # Inline function warnings
    -Wcast-qual                # Cast away const/volatile
    -Wcast-align=strict        # Cast alignment
    -Wwrite-strings            # String literal to char*
    -Wconversion               # Type conversions
    -Wsign-conversion          # Sign conversions
    -Wfloat-conversion         # Float conversions
    -Wlogical-op               # Logical operations
    -Waggregate-return         # Aggregate return
    -Wstrict-overflow=5        # Strict overflow checking
    -Wswitch-default           # Switch without default
    -Wswitch-enum              # Switch enum handling
    -Wunreachable-code         # Unreachable code
    -Wundef                    # Undefined macros
    -Wshadow                   # Variable shadowing
    -Wpointer-arith            # Pointer arithmetic
    -Wbad-function-cast        # Bad function casts
    -Wjump-misses-init         # Jump misses initialization
    -Wunsuffixed-float-constants # Unsuffixed float constants
)

# Link libraries
target_link_libraries(out ${NCURSES_LIBRARIES})

# Include directories
target_include_directories(out PRIVATE ${NCURSES_INCLUDE_DIRS})

# Compiler-specific flags
target_compile_definitions(out PRIVATE ${NCURSES_CFLAGS_OTHER})
