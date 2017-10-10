================================================================================
	IPC8 Library (Readme_en.txt)

						December 17, 2014
						DENSO WAVE INCORPORATED
================================================================================

Thank you for downloading 'IPC8 Library'.

----------
1. Description of Libraries
1-1. IPC8 Driver
Driver for using IPC8 functions, such as I/O or FRAM.
For more details, refer to driver_dkms/IPC8Driver_UsersGuide_en.doc
(Sorry we do not have IPC8Driver_UsersGuide_en.doc yet.)

1-2. TPComm Library
Library for communicating with a teach pendant.
To comile this library, enter following commands.

$ cd library/src/TPComm
$ make -f Makefile.Linux
$ make -f Makefile.Linux install

1-3. b-CAP Client Library
Library for communicating with a RC8 controller by b-CAP.
To comile this library, enter following commands.

$ cd library/src/bCAPClient
$ make -f Makefile.Linux
$ make -f Makefile.Linux install

----------
2. Description of Samples
2-1. IPC8 Driver
A sample program for using the IPC8 driver.
To run the sample program, enter following commands.

$ cd sample/Driver
$ make
$ make run

2-2. TPComm Library
A sample program for using the TPComm library.
To run the sample program, enter following commands.

$ cd sample/TPComm
$ make
$ make run

2-3. b-CAP Client Library
A sample program for using the b-CAP client library.
To run the sample program, enter following commands.

$ cd sample/bCAPClient
$ make
$ make run

[NOTE]
Please modify the Makefile if you need.

----------
3. License
3-1. GNU General Public License
All programs in driver_dkms are published under GNU General Public License as shown in driver_dkms/License.txt.

3-2. MIT License
All programs in library are published under MIT License as shown in library/License.txt.

----------
4. Release note

There are summaries of fixed points from the previous version in this section.
Please read carefully before using this version.

[v1.0.0] First edition
