# PGREP Test Makefile
# JSI - 2007.07.27

EXECUTABLE=../src/pgrep
.SILENT: test

test: 
	cd ../src; make
	$(EXECUTABLE) -k 2 -f 2 -F 2 -M 999 -b testdb.txt -p primerTest.txt -o > test.txt
	if cmp test.txt primerTest_k2f2F2oM999.txt >& foo.txt; then echo "PGREP passed test k2 f2 F2 M999"; else echo "PGREP failed test k2 f2 F2 M999"; fi

	$(EXECUTABLE) -k 2 -f 2 -F 4 -M 999 -b testdb.txt -p primerTest.txt -o -w weight3prime.txt > test.txt
	if cmp test.txt primerTest_k2f2F4oM999weight3prime.txt >& foo.txt; then echo "PGREP passed test k2 f2 F4 M999 weight3prime"; else echo "PGREP failed test k2 f2 F4 M999 weight3prime"; fi

	$(EXECUTABLE) -k 2 -f 2 -F 4 -M 999 -b testdb.txt -p primerTest.txt -o -w weightAC.txt > test.txt
	if cmp test.txt primerTest_k2f2F4oM999weightAC.txt >& foo.txt; then echo "PGREP passed test k2 f2 F4 M999 weightAC"; else echo "PGREP failed test k2 f2 F4 M999 weightAC"; fi

	$(EXECUTABLE) -k 2 -f 2 -F 4 -M 999 -b testdb.txt -p primerTest.txt -o -w weightCA.txt > test.txt
	if cmp test.txt primerTest_k2f2F4oM999weightCA.txt >& foo.txt; then echo "PGREP passed test k2 f2 F4 M999 weightCA"; else echo "PGREP failed test k2 f2 F4 M999 weightCA"; fi

	$(EXECUTABLE) -k 2 -f 2 -F 4 -M 999 -b testdb.txt -p primerTest.txt -o -w weightMK.txt > test.txt
	if cmp test.txt primerTest_k2f2F4oM999weightMK.txt >& foo.txt; then echo "PGREP passed test k2 f2 F4 M999 weightMK"; else echo "PGREP failed test k2 f2 F4 M999 weightMK"; fi

	$(EXECUTABLE) -k 2 -f 2 -F 4 -M 999 -d testdb.pdb -p primerTest.txt -o -w weightMK.txt > test.txt
	if cmp test.txt primerTest_k2f2F4oM999weightMK.txt >& foo.txt; then echo "PGREP passed test k2 f2 F4 M999 weightMK - old style db"; else echo "PGREP failed test k2 f2 F4 M999 weightMK - old style db"; fi

	$(EXECUTABLE) -k 0 -f 2 -F 2 -b testdb.txt < primer3.out > test.txt
	if cmp test.txt primer3_k0f2F2.txt >& foo.txt; then echo "PGREP passed test k0 f2 F2 STDIN input"; else echo "PGREP failed test k2 f2 F2 STDIN input"; fi

	rm foo.txt
	rm test.txt