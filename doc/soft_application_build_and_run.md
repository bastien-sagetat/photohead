# Application's build and run

**Prerequisite**: Perform the steps describe in the [requirements.md](soft_requirements.md) file.

## Build

First, clone the photohead-linux-app repository:
```shell
git clone https://github.com/Photohead/photohead-linux-app.git
```

Then, create a build/ folder in the repository:
```shell
cd photohead-linux-app
mkdir build && cd build
```
Once in the build/ folder, you can perform the CMake configuration:
```shell
cmake ..
```

You can then perform the build itself (this will take a while):
```shell
make -j $(nproc)
```

You should now have a **Photohead** binary located in the build/ folder

🚧: **TODO**: Add install step once implemented

## Run

Simply execute the **Photohead** binary (located in the build/ folder):
```shell
./Photohead
```

🚧: **TODO**: For now, the binary must be executed from a GUI shell, otherwise it will fail, because we are still using the OpenCV GUI to display the image stream (will get rid of it once the RSTP/RTP server will be implemented)