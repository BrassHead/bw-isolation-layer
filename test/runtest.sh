#! /bin/sh
#
echo ""
echo ""
echo "======  Running BW tests  ======="
echo ""
echo "The following tests should run to completion or assert on failure."
echo ""
./string1
./string2
echo "...string test completed"
./filename1
echo "...filename test completed"
./cptr1
echo "...cptr/countable test completed"
./ini1
echo "...INI file test completed"
./xml1 xmldata1.xml /tmp/data1.out1
./xml1 /tmp/data1.out1 /tmp/data1.out2
diff -s /tmp/data1.out1 /tmp/data1.out2
rm -f /tmp/data1.out1 /tmp/data1.out2
echo "...XML processor test completed"
echo ""
echo ""
echo "======  Button test ======"
echo ""
echo "Tests various size buttons.  Find the the 1x1 pixel button!"
echo "Press upper left button to exit"
echo ""
echo "Press enter to continue..."
read foo
./button1
echo ""
echo ""
echo "======  Hello World ======"
echo ""
echo "Tests basic operation."
echo "Press button to exit"
echo ""
echo "Press enter to continue..."
read foo
./bwhi
echo ""
echo ""
echo "======  X Isolation Layer ======"
echo ""
echo "Tests basic operation."
echo "Press close window to exit"
echo ""
echo "Press enter to continue..."
read foo
./bwisohi
./bwiso1
echo ""
echo ""
echo "====== End of BW tests ======"

