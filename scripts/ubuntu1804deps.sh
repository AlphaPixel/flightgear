set -e

packages=(
	# Build prereqs
	build-essential
	ninja-build

	# System libraries
	libx11-dev
	libgl1-mesa-dev

	# Library dependencies
	libcurl4-openssl-dev
	libboost-dev
	libopenal-dev
	libplib-dev
	zlib1g-dev
	liblzma-dev
	libopenscenegraph-3.4-dev
)

for package in "${packages[@]}";
do
	sudo apt-get -y install $package
done

# Remove any existing CMake package
command -v cmake >/dev/null 2>&1 && sudo apt-get -y remove cmake

# Install the latest CMake package
wget https://github.com/Kitware/CMake/releases/download/v3.21.5/cmake-3.21.5-linux-x86_64.sh -O /tmp/cmake_install.sh

sudo bash /tmp/cmake_install.sh --skip-license --prefix=/usr

