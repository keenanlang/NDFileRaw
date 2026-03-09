TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

-include $(ADCORE)/ADApp/commonDriverMakefile

LIBRARY_IOC = NDFileRaw

DBD += NDFileRaw.dbd

LIB_SRCS += NDFileRaw.cpp

NDFileEDF_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
