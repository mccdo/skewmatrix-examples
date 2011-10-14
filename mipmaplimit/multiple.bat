FOR /F %%i IN ('dir /b *.ive') DO (
mipmaplimit.exe --out %%i.ive %%i
REM PAUSE
)