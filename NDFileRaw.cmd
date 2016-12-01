NDFileRawConfigure("Raw1", 20, 0, "$(PORT)", 0, 0, 0)
dbLoadRecords("$(ADCORE)/ADApp/Db/NDFileRaw.template", "P=$(PREFIX), R=raw1:, PORT=Raw1, ADDR=0, TIMEOUT=1, NDARRAY_PORT=$(PORT), NDARRAY_ADDR=0") 
