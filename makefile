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
	cd libs/joy; make -f makefile; cd ../..;
	@echo
	@echo ////////////
	@echo // serial //
	@echo ////////////
	@echo
	cd libs/serial; make -f makefile; cd ../..;
	@echo
	@echo //////////////////
	@echo // microstrain  //
	@echo //////////////////
	@echo
	cd libs/microstrain; make -f makefile; cd ../..;
	@echo
	@echo ////////////
	@echo // pololu //
	@echo ////////////
	@echo
	cd libs/pololu; make -f makefile; cd ../..;
	@echo
	@echo ////////////
	@echo // parser //
	@echo ////////////
	@echo
	cd libs/parser; make -f makefile; cd ../..;
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
	cd libs/vision; make -f makefile; cd ../..;
	@echo
	@echo ////////////
	@echo // kalman //
	@echo ////////////
	@echo
	cd libs/kalman; make -f makefile; cd ../..;
	@echo
	@echo /////////
	@echo // nav //
	@echo /////////
	@echo
	cd src/nav; make -f makefile; cd ../..;
	@echo
	@echo /////////
	@echo // gui //
	@echo /////////
	@echo
	cd src/gui; make -f makefile; cd ../..;
	@echo
	@echo /////////////
	@echo // planner //
	@echo /////////////
	@echo
	cd src/planner; make -f makefile; cd ../..;
	@echo
	@echo /////////////
	@echo // visiond //
	@echo /////////////
	@echo
	cd src/visiond; make -f makefile; cd ../..;
	@echo
	@echo //////////////
	@echo // labjackd //
	@echo //////////////
	@echo
	cd src/labjackd; make -f makefile; cd ../..;
	@echo
	@echo //////////////
	@echo // joydrive //
	@echo //////////////
	@echo
	cd src/joydrive; make -f makefile; cd ../..;
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
	cd libs/joy; make -f makefile clean; cd test; make -f makefile clean; \
	cd ../../..;
	@echo
	@echo ////////////
	@echo // serial //
	@echo ////////////
	@echo
	cd libs/serial; make -f makefile clean; cd ../..;
	@echo
	@echo //////////////////
	@echo // microstrain  //
	@echo //////////////////
	@echo
	cd libs/microstrain; make -f makefile clean; cd ../..;
	@echo
	@echo //////////////
	@echo // logframe //
	@echo //////////////
	@echo
	cd libs/logframe; make -f Makefile clean; cd ../..;
	@echo
	@echo ////////////
	@echo // pololu //
	@echo ////////////
	@echo
	cd libs/pololu; make -f makefile clean; cd ../..;
	@echo
	@echo ////////////
	@echo // parser //
	@echo ////////////
	@echo
	cd libs/parser; make -f makefile clean; cd ../..;
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
	cd libs/vision; make -f makefile clean; cd ../..;
	@echo
	@echo /////////
	@echo // ssa //
	@echo /////////
	@echo
	cd libs/ssa; make -f makefile clean; cd ../..;
	@echo
	@echo /////////
	@echo // nav //
	@echo /////////
	@echo
	cd src/nav; make -f makefile clean; cd ../..;
	@echo
	@echo /////////
	@echo // gui //
	@echo /////////
	@echo
	cd src/gui; make -f makefile clean; cd ../..;
	@echo
	@echo /////////////
	@echo // planner //
	@echo /////////////
	@echo
	cd src/planner; make -f makefile clean; cd ../..;
	@echo
	@echo /////////////
	@echo // visiond //
	@echo /////////////
	@echo
	cd src/visiond; make -f makefile clean; cd ../..;
	@echo
	@echo //////////////
	@echo // labjackd //
	@echo //////////////
	@echo
	cd src/labjackd; make -f makefile clean; cd ../..;
	@echo
	@echo //////////////
	@echo // joydrive //
	@echo //////////////
	@echo
	cd src/joydrive; make -f makefile clean; cd ../..;
	@echo
	@echo /////////////
	@echo // DONE!!! //
	@echo /////////////
	@echo

