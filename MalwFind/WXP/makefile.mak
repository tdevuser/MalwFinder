

rebuild:
	SET DRVLANG=ko
	$(WDK_ROOT)\bin\setenv.bat $(WDK_ROOT) fre WXP  && pushd $(DRIVER_ROOT)\WXP  && build -ceZ && popd
	SET DRVLANG=en
	$(WDK_ROOT)\bin\setenv.bat $(WDK_ROOT) fre WXP  && pushd $(DRIVER_ROOT)\WXP  && build -ceZ && popd


debug:
	SET DRVLANG=ko
	$(WDK_ROOT)\bin\setenv.bat $(WDK_ROOT) chk WXP  && pushd $(DRIVER_ROOT)\WXP  && build -ceZ && popd
	SET DRVLANG=en
	$(WDK_ROOT)\bin\setenv.bat $(WDK_ROOT) chk WXP  && pushd $(DRIVER_ROOT)\WXP  && build -ceZ && popd


export: rebuild 


