# DLL injection

## Details to note

1. PvZ is a 32bit application
    - So both injector & dll should be compiled as 32bit
    - Ensure your system has the 32bit compiler: [Guide](https://superuser.com/questions/1473717/compile-in-msys2-mingw64-with-m32-option)


2. PvZ must be running
    - Injector searches for the application by name


3. `injector.cpp` is the injector file
    - It searches for the PvZ process
    - It injects the `test-inject.dll` into the PvZ process
    - Cmd line process ends after a while, but the injected code will still be running (test out to try)

4. `test-inject.cpp` is the dll file
    - It contains the code that will be injected into PvZ
    - Current Functionality: Press `F6` to display a message box

## Steps
1. Compile injector.cpp 
```
C:/msys64/mingw32/bin/g++ -m32 injector.cpp -o injector.exe
```

2. Compile dll.cpp 
```
C:/msys64/mingw32/bin/g++ -m32 -shared -o test-inject.dll test-inject.cpp
```

3. Copy & paste the dll into the same directory as the `PlantsVsZombies.exe` file

4. In current directory (dll-stuff), run `./injector.exe`

A message box should pop up with "DLL injected !" and you can press the `F6` key to test out.

## References
- [DLL Injector](https://www.youtube.com/watch?v=RasdnQmM3IY)
- [DLL Injection Code](https://www.youtube.com/watch?v=KCtLiBnlpk4&t=454s)