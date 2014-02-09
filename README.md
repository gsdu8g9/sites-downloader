sites-downloader
================

Program to download static websites

####Usage:
sd [options]... site...

####Options:
+ --enable-links-origin   Enables showing links origin (file)
+ --disable-links-origin  Disables showing links origin (file)
+ -i PAGE_URL             Set ignore urls with prefix PAGE_URL
+ -w WRONGS_FILE          Repair/continue download page, WRONGS_FILE is file to which the program prints the error logs, you can use only one this option
+ -j [N], --jobs[=N]      Allow N jobs at once
