# If on RaspberryPi, and folder doesn't exist install the RGB LED Matrix library
if grep -q "BCM" /proc/cpuinfo; then
    echo "This is a Raspberry Pi."
    export VCPKG_FORCE_SYSTEM_BINARIES=1
    sudo apt-get remove libsdl2-dev libsdl2-2.0-0
    if [ ! -d "./libs/rpi-rgb-led-matrix" ]; then
        git clone https://github.com/hzeller/rpi-rgb-led-matrix ./libs/rpi-rgb-led-matrix
        make -C libs/rpi-rgb-led-matrix/
    fi
    # sudo apt install -y libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev pkg-config
    # sudo apt-get install -y libx11-dev libxrandr-dev libxi-dev libudev-dev libgl1-mesa-dev ninja-build
else
    echo "This is not a Raspberry Pi."
fi

if [ -d "./libs/vcpkg/" ]; then
    echo "Install script already run."
else
    git clone https://github.com/Microsoft/vcpkg.git ./libs/vcpkg
    ./libs/vcpkg/bootstrap-vcpkg.sh

    git clone --recurse-submodules https://github.com/Picovoice/porcupine.git ./libs/porcupine
fi

./libs/vcpkg/vcpkg install --x-install-root=libs


# wget https://github.com/libsdl-org/SDL/archive/refs/tags/release-2.26.4.zip
# unzip release-2.26.4.zip
# cd SDL-release-2.26.4
# mkdir build
# cd build
# ../configure
# make
# sudo make install