if [ -d "./vcpkg" ]; then
    echo "Install script already run."
else
    # If on RaspberryPi, and folder doesn't exist install the RGB LED Matrix library
    if grep -q "BCM" /proc/cpuinfo && [ ! -d "./libs/rpi-rgb-led-matrix" ]; then
        echo "This is a Raspberry Pi."
        sudo apt install -y ninja-build
        git clone https://github.com/hzeller/rpi-rgb-led-matrix ./libs/rpi-rgb-led-matrix
        make -C libs/rpi-rgb-led-matrix/
    fi

    git clone https://github.com/Microsoft/vcpkg.git ./libs/vcpkg
    ./libs/vcpkg/bootstrap-vcpkg.sh

fi

# Install dependencies
./libs/vcpkg/vcpkg install "aws-sdk-cpp[lex]" --recurse
./libs/vcpkg/vcpkg install jsoncpp
./libs/vcpkg/vcpkg install sfml
./libs/vcpkg/vcpkg install eventpp

./libs/vcpkg/vcpkg integrate install
