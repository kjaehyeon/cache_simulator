# cache_simulator
리눅스에서 동작
<입력형식>
./cachesim -a=2 -s=64 -b=8 -f=sample.trc
<각 argument의미>  
  -s=<cache size>
  -b=<block size> 
  -a=<set size>
  -f=<trace file name>
<출력형식>
0: 00000001 00000001 v:1 d:1
   00000002 FFFC1100 v:1 d:0
1: 00000003 00000001 v:1 d:1
   0005FD04 00000001 v:1 d:0
2: 00000000 00000001 v:0 d:1
   00000001 01100001 v:1 d:1
3: ABAB0022 00011111 v:1 d:0
   11100001 00000001 v:1 d:1
 
total number of hits: 234
total number of misses: 31
miss rate: 11.7%
total number of dirty blocks: 4
average memory access cycle: 29.4
