#动态

nmake clean
perl Configure VC-WIN64A --debug --prefix=E:\openssl\Lib\x64\Debug --openssldir=E:\openssl\SSL
nmake
nmake test
nmake install
nmake clean 

nmake clean
perl Configure VC-WIN64A --prefix=E:\openssl\Lib\x64\Release --openssldir=E:\openssl\SSL
nmake
nmake test
nmake install
nmake clean

#静态

nmake clean
perl Configure VC-WIN64A --debug --prefix=E:\openssl\Lib\x64\Debug --openssldir=E:\openssl\SSL no-shared
nmake
nmake test
nmake install
nmake clean

nmake clean
perl Configure VC-WIN64A --prefix=E:\openssl\Lib\x64\Release --openssldir=E:\openssl\SSL no-shared
nmake
nmake test
nmake install
nmake clean
