README

======


Author: Dhruv Seth

Dated: 15th Oct 2011


INSTALLATION

===========

run the command "make" on the directory you wish to install the program. The output will be a myhttpd binary file.



Root directory

============

The httpd web server starts with the default directory "/myhttpd"
If one wants to change the default directory use the -r <dir> switch. For example,

./myhttpd -r /myhttpd/dir1

the webserver will set it's root path to this directory, if it exists.



MAX EXECUTION THREADS

=====================

The maximum number of threads are set to 20. One can change the #define MAXTHREADS
value in the source.c



404 (page not found) page

=======================

The page has been created such as it would display the path requested by the client and the webserver's root directory.

Snapshots
========
snapshots are included for reference.
