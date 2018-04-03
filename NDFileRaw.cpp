/* NDFileRaw.cpp
 * Writes NDArrays to raw files.
 *
 * Keenan Lang
 * October 5th, 2016
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>

#include <epicsStdio.h>
#include <epicsString.h>
#include <epicsTime.h>
#include <iocsh.h>
#define epicsAssertAuthor "the EPICS areaDetector collaboration (https://github.com/areaDetector/ADCore/issues)"
#include <epicsAssert.h>

#include <asynDriver.h>

#include <epicsExport.h>
#include "NDFileRaw.h"


static const char *driverName = "NDFileRaw";

asynStatus NDFileRaw::openFile(const char *fileName, NDFileOpenMode_t openMode, NDArray *pArray)
{
	static const char *functionName = "openFile";

	asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, "%s::%s Filename: %s\n", driverName, functionName, fileName);

	// We don't support reading yet
	if (openMode & NDFileModeRead) 
	{
		setIntegerParam(NDFileCapture, 0);
		setIntegerParam(NDWriteFile, 0);
		return asynError;
	}

	// We don't support opening an existing file for appending yet
	if (openMode & NDFileModeAppend) 
	{
		setIntegerParam(NDFileCapture, 0);
		setIntegerParam(NDWriteFile, 0);
		return asynError;
	}

	// Check if an invalid (<0) number of frames has been configured for capture
	int numCapture;
	getIntegerParam(NDFileNumCapture, &numCapture);
	if (numCapture < 0) 
	{
		asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
				  "%s::%s Invalid number of frames to capture: %d. Please specify a number >= 0\n",
				  driverName, functionName, numCapture);
		return asynError;
	}
	
	// Check to see if a file is already open and close it
	if (this->file.is_open())    { this->closeFile(); }

	// Create the new file
	this->file.open(fileName, std::ofstream::binary);
	
	if (! this->file.is_open())
	{
		asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, 
				  "%s::%s ERROR Failed to create a new output file\n",
				  driverName, functionName);
		return asynError;
	}
	
	// Write 8192 byte header, currently just zeroes.
	char header[8192] = {0};
	
	this->file.write(header, 8192);
	
	return asynSuccess;
}

/** Writes NDArray data to a raw file.
  * \param[in] pArray Pointer to an NDArray to write to the file. This function can be called multiple
  *            times between the call to openFile and closeFile if NDFileModeMultiple was set in 
  *            openMode in the call to NDFileRaw::openFile.
  */
asynStatus NDFileRaw::writeFile(NDArray *pArray)
{
	asynStatus status = asynSuccess;
	static const char *functionName = "writeFile";

	if (! this->file.is_open())
	{
		asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, 
				  "%s::%s file is not open!\n", 
				  driverName, functionName);
		return asynError;
	}

	this->file.write((const char*) pArray->pData, pArray->dims[0].size * pArray->dims[1].size * 2);

	return asynSuccess;
}

/** Read NDArray data from a HDF5 file; NOTE: not implemented yet.
  * \param[in] pArray Pointer to the address of an NDArray to read the data into.  */ 
asynStatus NDFileRaw::readFile(NDArray **pArray)
{
  //static const char *functionName = "readFile";
  return asynError;
}

/** Closes the file opened with NDFileRaw::openFile 
 */ 
asynStatus NDFileRaw::closeFile()
{
	epicsInt32 numCaptured;
	static const char *functionName = "closeFile";

	if (!this->file.is_open())
	{
		asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, 
				  "%s::%s file was not open! Ignoring close command.\n", 
				  driverName, functionName);
		return asynSuccess;
	}

	this->file.close();

	asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, "%s::%s file closed!\n", driverName, functionName);


	return asynSuccess;
}


/** Constructor for NDFileRaw; parameters are identical to those for NDPluginFile::NDPluginFile,
    and are passed directly to that base class constructor.
  * After calling the base class constructor this method sets NDPluginFile::supportsMultipleArrays=1.
  */
NDFileRaw::NDFileRaw(const char *portName, int queueSize, int blockingCallbacks, 
                     const char *NDArrayPort, int NDArrayAddr,
                     int priority, int stackSize)
  /* Invoke the base class constructor.
   * We allocate 2 NDArrays of unlimited size in the NDArray pool.
   * This driver can block (because writing a file can be slow), and it is not multi-device.  
   * Set autoconnect to 1.  priority and stacksize can be 0, which will use defaults. */
  : NDPluginFile(portName, queueSize, blockingCallbacks,
                 NDArrayPort, NDArrayAddr, 1, NUM_NDFILE_RAW_PARAMS,
                 2, 0, asynGenericPointerMask, asynGenericPointerMask, 
                 ASYN_CANBLOCK, 1, priority, stackSize)
{
  //static const char *functionName = "NDFileRaw";
   setStringParam(NDPluginDriverPluginType, "NDFileRaw");
   
   this->supportsMultipleArrays = true;
}



/** Configuration routine. */
extern "C" int NDFileRawConfigure(const char *portName, int queueSize, int blockingCallbacks, 
                                  const char *NDArrayPort, int NDArrayAddr,
                                  int priority, int stackSize)
{
  NDFileRaw* temp = new NDFileRaw(portName, queueSize, blockingCallbacks, NDArrayPort, NDArrayAddr, priority, stackSize);
  
  return temp->start();
}


/** EPICS iocsh shell commands */
static const iocshArg initArg0 = { "portName",iocshArgString};
static const iocshArg initArg1 = { "frame queue size",iocshArgInt};
static const iocshArg initArg2 = { "blocking callbacks",iocshArgInt};
static const iocshArg initArg3 = { "NDArray Port",iocshArgString};
static const iocshArg initArg4 = { "NDArray Addr",iocshArgInt};
static const iocshArg initArg5 = { "priority",iocshArgInt};
static const iocshArg initArg6 = { "stack size",iocshArgInt};
static const iocshArg * const initArgs[] = {&initArg0,
                                            &initArg1,
                                            &initArg2,
                                            &initArg3,
                                            &initArg4,
                                            &initArg5,
                                            &initArg6};
static const iocshFuncDef initFuncDef = {"NDFileRawConfigure",7,initArgs};
static void initCallFunc(const iocshArgBuf *args)
{
  NDFileRawConfigure(args[0].sval, args[1].ival, args[2].ival, args[3].sval, 
                      args[4].ival, args[5].ival, args[6].ival);
}

extern "C" void NDFileRawRegister(void)
{
  iocshRegister(&initFuncDef,initCallFunc);
}

extern "C" {
epicsExportRegistrar(NDFileRawRegister);
}

