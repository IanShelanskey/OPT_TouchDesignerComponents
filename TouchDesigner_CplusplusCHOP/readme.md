# open_ptrack TouchDesigner Cplusplus CHOP
The DLL and source files for streaming open_ptrack data into TouchDesigner via UDP. This implementation is much faster than parseing and sorting with python. 

## Build
To build the DLL, open the solution file located in vs/OPT_CHOP/. Then build for your architecture. 

## Use
An example file for x64 is located in examples/OPT_TestBench.toe. This file can be used with the ../optsimulate.py file in order to test. 

There is a Custom parameters page called OPT General which has a parameter called "Max Tracked". This parameter is the amoint of people you want to be able to track in TouchDesigner at any given time. The open_ptrack data stream may carry more tracks. Adjust Max Tracked to see more or less.  

This CHOP will automatically fill any dropped track ID with a -1 value in order to hold the place of the other tracks in the array. If there is overflow data past the Max Tracked value, the CHOP will fill missing ids with new ones.

## Authors
- **Ian Shelanskey** [ianshelanskey.com](http://ianshelanskey.com)

   