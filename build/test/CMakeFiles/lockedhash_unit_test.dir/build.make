# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/secui/git/LockedHash

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/secui/git/LockedHash/build

# Include any dependencies generated for this target.
include test/CMakeFiles/lockedhash_unit_test.dir/depend.make

# Include the progress variables for this target.
include test/CMakeFiles/lockedhash_unit_test.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/lockedhash_unit_test.dir/flags.make

test/CMakeFiles/lockedhash_unit_test.dir/test_lockedhash.cpp.o: test/CMakeFiles/lockedhash_unit_test.dir/flags.make
test/CMakeFiles/lockedhash_unit_test.dir/test_lockedhash.cpp.o: ../test/test_lockedhash.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/secui/git/LockedHash/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/CMakeFiles/lockedhash_unit_test.dir/test_lockedhash.cpp.o"
	cd /home/secui/git/LockedHash/build/test && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/lockedhash_unit_test.dir/test_lockedhash.cpp.o -c /home/secui/git/LockedHash/test/test_lockedhash.cpp

test/CMakeFiles/lockedhash_unit_test.dir/test_lockedhash.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/lockedhash_unit_test.dir/test_lockedhash.cpp.i"
	cd /home/secui/git/LockedHash/build/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/secui/git/LockedHash/test/test_lockedhash.cpp > CMakeFiles/lockedhash_unit_test.dir/test_lockedhash.cpp.i

test/CMakeFiles/lockedhash_unit_test.dir/test_lockedhash.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/lockedhash_unit_test.dir/test_lockedhash.cpp.s"
	cd /home/secui/git/LockedHash/build/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/secui/git/LockedHash/test/test_lockedhash.cpp -o CMakeFiles/lockedhash_unit_test.dir/test_lockedhash.cpp.s

test/CMakeFiles/lockedhash_unit_test.dir/test_lockedhash_keypair.cpp.o: test/CMakeFiles/lockedhash_unit_test.dir/flags.make
test/CMakeFiles/lockedhash_unit_test.dir/test_lockedhash_keypair.cpp.o: ../test/test_lockedhash_keypair.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/secui/git/LockedHash/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object test/CMakeFiles/lockedhash_unit_test.dir/test_lockedhash_keypair.cpp.o"
	cd /home/secui/git/LockedHash/build/test && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/lockedhash_unit_test.dir/test_lockedhash_keypair.cpp.o -c /home/secui/git/LockedHash/test/test_lockedhash_keypair.cpp

test/CMakeFiles/lockedhash_unit_test.dir/test_lockedhash_keypair.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/lockedhash_unit_test.dir/test_lockedhash_keypair.cpp.i"
	cd /home/secui/git/LockedHash/build/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/secui/git/LockedHash/test/test_lockedhash_keypair.cpp > CMakeFiles/lockedhash_unit_test.dir/test_lockedhash_keypair.cpp.i

test/CMakeFiles/lockedhash_unit_test.dir/test_lockedhash_keypair.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/lockedhash_unit_test.dir/test_lockedhash_keypair.cpp.s"
	cd /home/secui/git/LockedHash/build/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/secui/git/LockedHash/test/test_lockedhash_keypair.cpp -o CMakeFiles/lockedhash_unit_test.dir/test_lockedhash_keypair.cpp.s

# Object files for target lockedhash_unit_test
lockedhash_unit_test_OBJECTS = \
"CMakeFiles/lockedhash_unit_test.dir/test_lockedhash.cpp.o" \
"CMakeFiles/lockedhash_unit_test.dir/test_lockedhash_keypair.cpp.o"

# External object files for target lockedhash_unit_test
lockedhash_unit_test_EXTERNAL_OBJECTS =

test/lockedhash_unit_test: test/CMakeFiles/lockedhash_unit_test.dir/test_lockedhash.cpp.o
test/lockedhash_unit_test: test/CMakeFiles/lockedhash_unit_test.dir/test_lockedhash_keypair.cpp.o
test/lockedhash_unit_test: test/CMakeFiles/lockedhash_unit_test.dir/build.make
test/lockedhash_unit_test: lib/libgtest_main.a
test/lockedhash_unit_test: lib/libgtest.a
test/lockedhash_unit_test: test/CMakeFiles/lockedhash_unit_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/secui/git/LockedHash/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable lockedhash_unit_test"
	cd /home/secui/git/LockedHash/build/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/lockedhash_unit_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/lockedhash_unit_test.dir/build: test/lockedhash_unit_test

.PHONY : test/CMakeFiles/lockedhash_unit_test.dir/build

test/CMakeFiles/lockedhash_unit_test.dir/clean:
	cd /home/secui/git/LockedHash/build/test && $(CMAKE_COMMAND) -P CMakeFiles/lockedhash_unit_test.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/lockedhash_unit_test.dir/clean

test/CMakeFiles/lockedhash_unit_test.dir/depend:
	cd /home/secui/git/LockedHash/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/secui/git/LockedHash /home/secui/git/LockedHash/test /home/secui/git/LockedHash/build /home/secui/git/LockedHash/build/test /home/secui/git/LockedHash/build/test/CMakeFiles/lockedhash_unit_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/lockedhash_unit_test.dir/depend

