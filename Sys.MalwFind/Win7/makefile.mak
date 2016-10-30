

rebuild:
	SET DRVLANG=ko
	$(WDK_ROOT)\bin\setenv.bat $(WDK_ROOT) fre WIN7 && pushd $(DRIVER_ROOT)\Win7 && build -ceZ && popd
	SET DRVLANG=en
	$(WDK_ROOT)\bin\setenv.bat $(WDK_ROOT) fre WIN7 && pushd $(DRIVER_ROOT)\Win7 && build -ceZ && popd
	
	SET DRVLANG=ko
	$(WDK_ROOT)\bin\setenv.bat $(WDK_ROOT) fre x64 WIN7 && pushd $(DRIVER_ROOT)\Win7 && build -ceZ && popd
	SET DRVLANG=en
	$(WDK_ROOT)\bin\setenv.bat $(WDK_ROOT) fre x64 WIN7 && pushd $(DRIVER_ROOT)\Win7 && build -ceZ && popd


debug:
	SET DRVLANG=ko
	$(WDK_ROOT)\bin\setenv.bat $(WDK_ROOT) chk WIN7 && pushd $(DRIVER_ROOT)\Win7 && build -ceZ && popd
	SET DRVLANG=en
	$(WDK_ROOT)\bin\setenv.bat $(WDK_ROOT) chk WIN7 && pushd $(DRIVER_ROOT)\Win7 && build -ceZ && popd
	
	SET DRVLANG=ko
	$(WDK_ROOT)\bin\setenv.bat $(WDK_ROOT) chk x64 WIN7 && pushd $(DRIVER_ROOT)\Win7 && build -ceZ && popd
	SET DRVLANG=en
	$(WDK_ROOT)\bin\setenv.bat $(WDK_ROOT) chk x64 WIN7 && pushd $(DRIVER_ROOT)\Win7 && build -ceZ && popd



export: rebuild 