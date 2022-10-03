@call parameters.bat main.cpp stack.cpp -o main.exe

@if not errorlevel 1 (
    main.exe %*
)