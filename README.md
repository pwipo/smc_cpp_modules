# smc_cpp_modules

# Windows installation

install:

    jdk 11
    Visual Studio Community 2019 or high
    cmake

Prepare and Compile:

    Start Visual Studio Command prompt
    cd modules_pub
    check exist lib ../SMCModuleDefinitionProvider/cmake-build-release-visual-studio-64/ModuleLoaderProviderLINUX64.lib
    set _WIN64=1
    cmake -A x64 -S . -B WIN64
    cmake --build WIN64 --target Example
    copy WIN64\example\Debug\Example.dll WIN64\example\Example.dll
    cmake --build WIN64 --target Example_SMCM
    copy WIN64\example\ExampleCpp.smcm WIN64\ExampleCpp.smcm


# Linux installation

CentOS:

    yum groupinstall 'Development Tools'
    yum install cmake
    yum install java-11-openjdk-devel
    for 32: yum install libgcc.i686 glibc-devel.i686 libstdc++-devel.i686

Debian(Ubuntu):

    dkpg --add-architecture i386
    apt update
    apt intstall build-essential openjdk-11-jdk openjdk-11-jdk:i386 gcc-multilib g++-multilib

Prepare and Compile:
    
    cd modules_pub
    check exist lib ../SMCModuleDefinitionProvider/Linux64/ModuleLoaderProviderLINUX64.so
    mkdir build
    cd build
    #rm -r *
    _LINUX64=1 cmake ..
    make Example
    make Example_SMCM
    mkdir ../Linux64
    cp example/ExampleCpp.smcm ../Linux64/for_centos/ExampleCpp.smcm
