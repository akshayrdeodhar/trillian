# I want a makefile that detects changes in header files, export project to this dir rather than src. 
# Wrote a makefile in this directory, but does not detect changes in .h.
# So a jugadu script.

cd src
make
mv project ../
