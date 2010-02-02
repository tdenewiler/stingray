all:
	@echo
	@echo //////////////////
	@echo // Building all //
	@echo //////////////////
	@echo
	@echo /////////
	@echo // joy //
	@echo /////////
	@echo
	cd libs/joy; make -f Makefile; cd ../..;
	@echo
	@echo ////////////
	@echo // serial //
	@echo ////////////
	@echo
	cd libs/serial; make -f Makefile; cd ../..;
	@echo
	@echo //////////////////
	@echo // microstrain  //
	@echo //////////////////
	@echo
	cd libs/microstrain; make -f Makefile; cd ../..;
	@echo
	@echo /////////////
	@echo // timing  //
	@echo /////////////
	@echo
	cd libs/timing; make -f Makefile; cd ../..;
	@echo
	@echo ////////////
	@echo // sysid  //
	@echo ////////////
	@echo
	cd libs/sysid; make -f Makefile; cd test; make -f Makefile; cd ../../..;
	@echo
	@echo ////////////
	@echo // pololu //
	@echo ////////////
	@echo
	cd libs/pololu; make -f Makefile; cd ../..;
	@echo
	@echo ////////////
	@echo // parser //
	@echo ////////////
	@echo
	cd libs/parser; make -f Makefile; cd ../..;
	@echo
	@echo /////////////
	@echo // labjack //
	@echo /////////////
	@echo
	cd libs/labjack; make -f Makefile; cd ../..;
	@echo
	@echo ////////////
	@echo // vision //
	@echo ////////////
	@echo
	cd libs/vision; make -f Makefile; cd ../..;
	@echo
	@echo ////////////
	@echo // kalman //
	@echo ////////////
	@echo
	cd libs/kalman; make -f Makefile; cd ../..;
	@echo
	@echo /////////
	@echo // nav //
	@echo /////////
	@echo
	cd src/nav; make -f Makefile; cd ../..;
	@echo
	@echo /////////
	@echo // gui //
	@echo /////////
	@echo
	cd src/gui; make -f Makefile; cd ../..;
	@echo
	@echo /////////////
	@echo // planner //
	@echo /////////////
	@echo
	cd src/planner; make -f Makefile; cd ../..;
	@echo
	@echo /////////////
	@echo // visiond //
	@echo /////////////
	@echo
	cd src/visiond; make -f Makefile; cd ../..;
	@echo
	@echo //////////////
	@echo // labjackd //
	@echo //////////////
	@echo
	cd src/labjackd; make -f Makefile; cd ../..;
	@echo
	@echo //////////////
	@echo // joydrive //
	@echo //////////////
	@echo
	cd src/joydrive; make -f Makefile; cd ../..;
	@echo
	@echo //////////////
	@echo // estimate //
	@echo //////////////
	@echo
	cd src/estimate; make -f Makefile; cd ../..;
	@echo
	@echo /////////////
	@echo // DONE!!! //
	@echo /////////////
	@echo

clean:
	@echo
	@echo //////////////////
	@echo // Cleaning all //
	@echo //////////////////
	@echo
	@echo /////////
	@echo // joy //
	@echo /////////
	@echo
	cd libs/joy; make -f Makefile clean; cd test; make -f Makefile clean; \
	cd ../../..;
	@echo
	@echo ////////////
	@echo // serial //
	@echo ////////////
	@echo
	cd libs/serial; make -f Makefile clean; cd ../..;
	@echo
	@echo //////////////////
	@echo // microstrain  //
	@echo //////////////////
	@echo
	cd libs/microstrain; make -f Makefile clean; cd ../..;
	@echo
	@echo /////////////
	@echo // timing  //
	@echo /////////////
	@echo
	cd libs/timing; make -f Makefile clean; cd ../..;
	@echo
	@echo ////////////
	@echo // sysid  //
	@echo ////////////
	@echo
	cd libs/sysid; make -f Makefile clean; cd test; make - Makefile clean; cd ../../..;
	@echo
	@echo ////////////
	@echo // pololu //
	@echo ////////////
	@echo
	cd libs/pololu; make -f Makefile clean; cd ../..;
	@echo
	@echo ////////////
	@echo // parser //
	@echo ////////////
	@echo
	cd libs/parser; make -f Makefile clean; cd ../..;
	@echo
	@echo /////////////
	@echo // labjack //
	@echo /////////////
	@echo
	cd libs/labjack; make -f Makefile clean; cd ../..;
	@echo
	@echo ////////////
	@echo // vision //
	@echo ////////////
	@echo
	cd libs/vision; make -f Makefile clean; cd ../..;
	@echo
	@echo /////////
	@echo // nav //
	@echo /////////
	@echo
	cd src/nav; make -f Makefile clean; cd ../..;
	@echo
	@echo /////////
	@echo // gui //
	@echo /////////
	@echo
	cd src/gui; make -f Makefile clean; cd ../..;
	@echo
	@echo /////////////
	@echo // planner //
	@echo /////////////
	@echo
	cd src/planner; make -f Makefile clean; cd ../..;
	@echo
	@echo /////////////
	@echo // visiond //
	@echo /////////////
	@echo
	cd src/visiond; make -f Makefile clean; cd ../..;
	@echo
	@echo //////////////
	@echo // labjackd //
	@echo //////////////
	@echo
	cd src/labjackd; make -f Makefile clean; cd ../..;
	@echo
	@echo //////////////
	@echo // joydrive //
	@echo //////////////
	@echo
	cd src/joydrive; make -f Makefile clean; cd ../..;
	@echo
	@echo //////////////
	@echo // estimate //
	@echo //////////////
	@echo
	cd src/estimate; make -f Makefile clean; cd ../..;
	@echo
	@echo /////////////
	@echo // DONE!!! //
	@echo /////////////
	@echo

