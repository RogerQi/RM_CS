# Installation

*Disclaimer:*
This installation guide is written based on a clean Ubuntu 16.04 build with a Nvidia Graphic Card. Note that we don't guarantee it would work on any system. Also, we are not responsible for any potential damage to your data. Please proceed with caution.

The system image we used in this tutorial can be downloaded [here](http://mirrors.lug.mtu.edu/ubuntu-releases/16.04/ubuntu-16.04.5-desktop-amd64.iso)

There are some newer releases of libraries we used in this project. However, in this tutorial, we will use the older release for the sake of compatibility.

**1. Before we start...**

Install essential building tools,
```
sudo apt-get update
sudo apt-get install build-essential cmake git
```
Create a dep directory. We will do all the dirty jobs in it,
```
mkdir dep && cd dep
```

**2. Install CUDA 9.0**

A thing to note here is that we are going to use dpkg to install **INSTEAD OF** .tar/.run throughout the tutorial. Also, this is a **CUDA 9.0** but not a more recent version of CUDA. The reason why we want this is that it will provide us with a better management of the packages and may avoid potential dependency/compatibility issue(s).

Get the relevant dpkg through whatever means you like (browser/terminal/or copy/pasting...). The CUDA package we used here is downloaded from [here](https://developer.nvidia.com/compute/cuda/9.0/Prod/local_installers/cuda-repo-ubuntu1604-9-0-local_9.0.176-1_amd64-deb). After finishing, go into the directory in which it is stored, and run,
```
sudo dpkg -i cuda-repo-ubuntu1604-9-0-local_9.0.176-1_amd64-deb (this is the filename I got on my machine.)
```

(Optional:) In some cases, it may give you a warning that the GPG key is not installed.
```
...
The public CUDA GPG key does not appear to be installed.
...
```
In this case, run the suggested command. In my case, it was,
```
sudo apt-key add /var/cuda-repo-9-0-local/7fa2af80.pub
```

Now we have finished setting up the repo. Run,
```
sudo apt-get update
sudo apt-get install cuda
```

You should be all set with CUDA. (In rare occasion or newer releases, it may give you a warning about CUDA not being able to disable the nouveau driver and therefore driver is not installed. See FAQ you have this problem.

**3. Install cuDNN**

Get cuDNN 7.3 for **CUDA 9.0**. Just google "cuDNN download" and it should be the first result. In this tutorial, we are going to use RUNTIME library of cuDNN 7.30 for CUDA9.0 on Ubuntu 16.04 (.deb). Note that it is unnecessary to get the developer version here - runtime library is sufficient for out development purpose. In my case, I download the relevant package from [here](https://developer.nvidia.com/compute/machine-learning/cudnn/secure/v7.3.0/prod/9.0_2018920/Ubuntu16_04-x64/libcudnn7_7.3.0.29-1-cuda9.0_amd64).

After downloading, run,
```
sudo dpkg -i libcudnn7_7.3.0.29-1+cuda9.0_amd64.deb (or whatever filename your downloaded file is)
```
and you should be all set with cuDNN.

**4. Install PyCUDA**

Next, we are going to install PyCUDA. This is an unncessary step and you can skip this step if you are planning to do all the development work with C++. However since we do use Python for PoC and iterations, it's recommended to have PyCUDA installed.

First, setup the python environment,
```
sudo apt install python2.7-dev
wget https://bootstrap.pypa.io/get-pip.py
sudo python get-pip.py
```
Then,
```
sudo pip install pycuda
```

**5. Install TensorRT**

Now, we are going to get TensorRT. We want to use TensorRT 3 for Ubuntu 1604 and CUDA 9.0 DEB local repo packages here. Again, google TensorRT and follow the download page. Choose TensorRT 3 **INSTEAD OF TensorRT 5**. In my case, the link I used to get the file was [this](https://developer.nvidia.com/compute/machine-learning/tensorrt/3.0/ga/nv-tensorrt-repo-ubuntu1604-ga-cuda9.0-trt3.0.4-20180208_1-1_amd64-deb).

After finishing download. Run,
```
sudo dpkg -i nv-tensorrt-repo-ubuntu1604-ga-cuda9.0-trt3.0.4-20180208_1-1_amd64.deb
sudo apt-get update
sudo apt-get install tensorrt
```
(Optional:) If you installed PyCUDA, you may want to also get TRT support for python here,
```
sudo pip install uff-converter-tf python3-libnvinfer
```
To test it,
```
Python 3.6.4 (default, Mar  9 2018, 23:15:03)
[GCC 4.2.1 Compatible Apple LLVM 9.0.0 (clang-900.0.39.2)] on darwin
Type "help", "copyright", "credits" or "license" for more information.
>>> import tensorrt
>>>
```

If the module is imported without throwing any exception, then it is properly installed.

**6. Install OpenCV**

This is probably the longest and most painful step. Before following the instruction below, make sure you have all packages from the previous sections properly installed. First, install some relevant packages,
```
sudo apt-get install \
    libglew-dev \
    libtiff5-dev \
    zlib1g-dev \
    libjpeg-dev \
    libpng12-dev \
    libjasper-dev \
    libavcodec-dev \
    libavformat-dev \
    libavutil-dev \
    libpostproc-dev \
    libswscale-dev \
    libeigen3-dev \
    libtbb-dev \
    libgtk2.0-dev \
    pkg-config
```
To enable gstreamer support for camera, you also need to run,
```
sudo apt-get install libgstreamer1.0-0 gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-doc gstreamer1.0-tools libgstreamer-plugins-base1.0-dev libgstreamer1.0-dev
```
Now, get OpenCV from [here](https://github.com/opencv/opencv/archive/3.4.2.zip). After downloading, run,
```
unzip 3.4.2.zip
cd opencv-3.4.2
mkdir build && cd build
```
Configure our build,
```
cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DBUILD_PNG=OFF \
    -DBUILD_TIFF=OFF \
    -DBUILD_TBB=OFF \
    -DBUILD_JPEG=OFF \
    -DBUILD_JASPER=OFF \
    -DBUILD_ZLIB=OFF \
    -DBUILD_EXAMPLES=ON \
    -DBUILD_opencv_java=OFF \
    -DBUILD_opencv_python2=ON \
    -DBUILD_opencv_python3=OFF \
    -DENABLE_NEON=ON \
    -DWITH_OPENCL=OFF \
    -DWITH_OPENMP=OFF \
    -DWITH_FFMPEG=ON \
    -DWITH_GSTREAMER=ON \
    -DWITH_GSTREAMER_0_10=ON \
    -DWITH_CUDA=ON \
    -DWITH_GTK=ON \
    -DWITH_VTK=OFF \
    -DWITH_TBB=OFF \
    -DWITH_1394=OFF \
    -DWITH_OPENEXR=OFF \
    -DCUDA_TOOLKIT_ROOT_DIR=/usr/local/cuda-9.0 \
    -DINSTALL_C_EXAMPLES=ON \
    -DINSTALL_TESTS=OFF \
    -DOPENCV_TEST_DATA_PATH=../opencv_extra/testdata \
    ../
```
Now, compile it,
```
make -j6
```
This is going to take a while. Depending on your PC's performance, it may take from 5min to 2hrs. After finishing, run,
```
sudo make install
```
To see if it have been properly installed, you can try,
```
Python 3.6.4 (default, Mar  9 2018, 23:15:03)
[GCC 4.2.1 Compatible Apple LLVM 9.0.0 (clang-900.0.39.2)] on darwin
Type "help", "copyright", "credits" or "license" for more information.
>>> import cv2
>>> print(cv2.__version__)
3.4.2
>>>
```

**7. Try and if you can compile the repo!**

Get this repo from github by running,

```
git clone https://github.com/RogerQi/RM_CS.git
```

Before we start compiling, we need to let the program know the Architecture and Compute Capability of your GPU.
To do this, refer to the page [Nvidia CUDA GPUs](https://developer.nvidia.com/cuda-gpus). For example, I got a
GeForce GTX 1060 on my PC, so I looked for it under CUDA-Enabled GeForce Products and found out it had
a compute capability of 6.1.

Now go to RM_CS/tensorrt. You should see a CMakeLists.txt. Around Line 21 you should see something like this
```
set(
	CUDA_NVCC_FLAGS
	${CUDA_NVCC_FLAGS};
    -O3
	-gencode arch=compute_53,code=sm_53
	-gencode arch=compute_62,code=sm_62
)
```

Change it to be like this
```
set(
	CUDA_NVCC_FLAGS
	${CUDA_NVCC_FLAGS};
    -O3
	-gencode arch=compute_61,code=sm_61
)
```

Note that I use

```
-gencode arch=compute_61,code=sm_61
```

because the compute capability of my GPU is 6.1. You are likely to have a different GPU!

Now, build it with default configuration,
```
cd RM_CS
mkdir build && cd build
cmake ..
make -j6
```
The above commands should complete without raising any error.

**8. FAQ**

*1. When the package was trying to setup NVIDIA driver, it throws an exception that says it's compatible with nouveau driver which is currently running*
Disable nouveau driver because it's incompatible with Nvidia driver, which we have to install. (According to [CUDA installation Guide](http://docs.nvidia.com/cuda/cuda-installation-guide-linux/index.html#pre-installation-actions))
```
sudo touch /etc/modprobe.d/blacklist-nouveau.conf
```
Then, edit it with whatever editor of your choice. It should look like,
```
blacklist nouveau
options nouveau modeset=0
```
Then, regenerate init ram filesystem,
```
sudo update-initramfs -u
```
and finally, reboot. The resolution of your GUI might just become low. Don't worry. It will be good in a while after we install the Nvidia driver.
