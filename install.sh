if [ -d "./vcpkg" ]; then
    echo "Install script already run."
else
    sudo apt install -y ninja-build
    git clone https://github.com/Microsoft/vcpkg.git
    ./vcpkg/bootstrap-vcpkg.sh

    # If on RaspberryPi, and folder doesn't exist install the RGB LED Matrix library
    if grep -q "BCM" /proc/cpuinfo && [ ! -d "./libs/rpi-rgb-led-matrix" ]; then
        echo "This is a Raspberry Pi."
        git clone https://github.com/hzeller/rpi-rgb-led-matrix ./libs/rpi-rgb-led-matrix
        make -C libs/rpi-rgb-led-matrix/
    fi

fi

# Install dependencies
./vcpkg/vcpkg install "aws-sdk-cpp[lex]" --recurse
./vcpkg/vcpkg install jsoncpp
./vcpkg/vcpkg install sfml
./vcpkg/vcpkg install eventpp

./vcpkg/vcpkg integrate install
