gcc src/master.c -o bin/master
gcc src/processA.c -o bin/processA -lncurses -lrt -pthread -lbmp -lm
gcc src/processAclient.c -o bin/processAclient -lncurses -lrt -pthread -lbmp -lm
gcc src/processAserver.c -o bin/processAserver -lncurses -lrt -pthread -lbmp -lm
gcc src/processB.c -o bin/processB -lncurses -lrt -pthread -lbmp -lm
