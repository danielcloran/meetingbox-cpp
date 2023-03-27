# if [ -d "./libs/vcpkg/" ]; then
#     echo "Install script already run."
# else
#     git clone https://github.com/Microsoft/vcpkg.git ./libs/vcpkg
#     ./libs/vcpkg/bootstrap-vcpkg.sh
# fi

# Cross Platform dependencies
# ./libs/vcpkg/vcpkg install "aws-sdk-cpp[core, lex]" --recurse
# ./libs/vcpkg/vcpkg install jsoncpp
# ./libs/vcpkg/vcpkg install eventpp

# ./libs/vcpkg/vcpkg install opengl
./libs/vcpkg/vcpkg install sdl2

# If on RaspberryPi, and folder doesn't exist install the RGB LED Matrix library
if grep -q "BCM" /proc/cpuinfo; then
    echo "This is a Raspberry Pi."

    if [ ! -d "./libs/rpi-rgb-led-matrix" ]; then
        git clone https://github.com/hzeller/rpi-rgb-led-matrix ./libs/rpi-rgb-led-matrix
        make -C libs/rpi-rgb-led-matrix/
    fi
    # sudo apt install -y libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev pkg-config
    # sudo apt-get install -y libx11-dev libxrandr-dev libxi-dev libudev-dev libgl1-mesa-dev ninja-build
    # export VCPKG_MAX_CONCURRENCY=4
    # export VCPKG_FORCE_SYSTEM_BINARIES=1
else
    echo "This is not a Raspberry Pi."
    # ./libs/vcpkg/vcpkg install glfw3
fi

./libs/vcpkg/vcpkg integrate install
