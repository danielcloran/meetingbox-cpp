# If on RaspberryPi, and folder doesn't exist install the RGB LED Matrix library
if grep -q "BCM" /proc/cpuinfo; then
    echo "This is a Raspberry Pi."
    export VCPKG_FORCE_SYSTEM_BINARIES=1
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
fi

./libs/vcpkg/vcpkg install --x-install-root=libs
./libs/vcpkg/vcpkg integration install
