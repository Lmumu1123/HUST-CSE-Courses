@echo off
chcp 65001
title 算命大师
color 0b
echo ++++++++++++++++++++++++++++
echo     欢迎使用算命大师程序
echo     请输入您的出生日期

echo     格式YYYYMMDD

echo     按q退出程序
echo ++++++++++++++++++++++++++++

:START
:scanf
set /p temp=请输入您的生日（格式：YYYYMMDD）:

:: 判断是否退出
if /i "%temp%"=="q" goto END

set "year=%temp:~0,4%"
set "var2=%temp:~4,4%"
set /a var1=%year%%%12

if %var2% GEQ 0132 (if %var2% LEQ 0200 (echo 输入有问题，请重新输入
goto scanf) )
if %var2% GEQ 0229 (if %var2% LEQ 0300 (echo 输入有问题，请重新输入
goto scanf) )
if %var2% GEQ 0332 (if %var2% LEQ 0400 (echo 输入有问题，请重新输入
goto scanf) )
if %var2% GEQ 0431 (if %var2% LEQ 0500 (echo 输入有问题，请重新输入
goto scanf) )
if %var2% GEQ 0532 (if %var2% LEQ 0600 (echo 输入有问题，请重新输入
goto scanf) )
if %var2% GEQ 0631 (if %var2% LEQ 0700 (echo 输入有问题，请重新输入
goto scanf) )
if %var2% GEQ 0732 (if %var2% LEQ 0800 (echo 输入有问题，请重新输入
goto scanf) )
if %var2% GEQ 0832 (if %var2% LEQ 0900 (echo 输入有问题，请重新输入
goto scanf) )
if %var2% GEQ 0931 (if %var2% LEQ 1000 (echo 输入有问题，请重新输入
goto scanf) )
if %var2% GEQ 1032 (if %var2% LEQ 1100 (echo 输入有问题，请重新输入
goto scanf) )
if %var2% GEQ 1131 (if %var2% LEQ 1200 (echo 输入有问题，请重新输入
goto scanf) )
if %var2% GEQ 1232 (echo 输入有问题，请重新输入
goto scanf)

if %var1% EQU 0 (echo 属相: 猴)
if %var1% EQU 1 (echo 属相: 鸡)
if %var1% EQU 2 (echo 属相: 狗)
if %var1% EQU 3 (echo 属相: 猪)
if %var1% EQU 4 (echo 属相: 鼠)
if %var1% EQU 5 (echo 属相: 牛)
if %var1% EQU 6 (echo 属相: 虎)
if %var1% EQU 7 (echo 属相: 兔)
if %var1% EQU 8 (echo 属相: 龙)
if %var1% EQU 9 (echo 属相: 蛇)
if %var1% EQU 10 (echo 属相: 马)
if %var1% EQU 11 (echo 属相: 羊)


if %var2% GEQ 0121 (if %var2% LEQ 0219 (echo 星座: 水瓶座) )
if %var2% GEQ 0220 (if %var2% LEQ 0320 (echo 星座: 双鱼座) )
if %var2% GEQ 0321 (if %var2% LEQ 0420 (echo 星座: 白羊座) )
if %var2% GEQ 0421 (if %var2% LEQ 0521 (echo 星座: 金牛座) )
if %var2% GEQ 0522 (if %var2% LEQ 0621 (echo 星座: 双子座) )
if %var2% GEQ 0622 (if %var2% LEQ 0723 (echo 星座: 巨蟹座) )
if %var2% GEQ 0724 (if %var2% LEQ 0823 (echo 星座: 狮子座) )
if %var2% GEQ 0824 (if %var2% LEQ 0923 (echo 星座: 处女座) )
if %var2% GEQ 0924 (if %var2% LEQ 1023 (echo 星座: 天秤座) )
if %var2% GEQ 1024 (if %var2% LEQ 1122 (echo 星座: 天蝎座) )
if %var2% GEQ 1123 (if %var2% LEQ 1222 (echo 星座: 射手座) )
if %var2% GEQ 1223 (echo 星座: 摩羯座)
if %var2% GEQ 0101 (if %var2% LEQ 0122 (echo 星座: 摩羯座) )

echo.
goto START

:END
echo 谢谢使用，程序结束！
pause
exit
