# Hungry-Tribe
Implementing extended Producer-Consumer problem ( Hungry Tribe problem) using semaphores

## For Linux 

* Clone the repository, using

 ```
 $ git clone https://github.com/sudo-chawhan/Hungry-Tribe.git
 ```
 
* In terminal, change the current directory to cloned repo using

```
$ cd Hungry-Tribe
```

* Edit inputs.txt to the desired inputs
```
inputs are in the form
----
K
M
n
alpha
beta
lambda
mu
----
 ```
* in the terminal, compile the .cpp files using

```
$ g++ -std=c++11 -pthread  cs16btech11037-code.cpp -lrt 
$ ./a.out
```
* ...The expected files will be created...
