# OS
An operating system written in C. Functions are Unix-like but implemented mostly as user-level libraries rather than built in to the kernel. 

### Dependencies
Qemu:
```
sudo apt-get install libfdt-dev libsdl1.2-dev libtool-bin libglib2.0-dev libz-dev libpixman-1-dev python git

cd ~/qemu

mkdir build

./configure --disable-kvm --disable-werror --prefix=~/qemu/build --target-list="i386-softmmu x86_64-softmmu"
```
Open your ~/.bashrc file and add the following line at the end of it:
```
PATH=~/qemu/build/bin:$PATH
```

Compiler Toolchain:
```
sudo apt-get install -y build-essential gdb

sudo apt-get install gcc-multilib
```


### Start Shell
```
make run-icode
```
or
```
make run-icode-nox
```

### Shell Command Examples
```
$ echo hello world | cat

$ cat lorem |num |num |num

$ lsfd
```

### Running n CPUs
```
make qemu CPUS=n
```
or
```
make qemu-nox CPUS=n
```
