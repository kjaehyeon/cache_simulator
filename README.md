# cache_simulator
리눅스에서 동작
- 입력형식
./cachesim -a=2 -s=64 -b=8 -f=sample.trc
- 각 argument의미  
  -s=cache size
  
  -b=block size
  
  -a=set size
  
  -f=trace file name

- 출력형식
0: 00000001 00000001 v:1 d:1<br>
   00000002 FFFC1100 v:1 d:0<br>
1: 00000003 00000001 v:1 d:1<br>
   0005FD04 00000001 v:1 d:0<br>
2: 00000000 00000001 v:0 d:1<br>
   00000001 01100001 v:1 d:1<br>
3: ABAB0022 00011111 v:1 d:0<br>
   11100001 00000001 v:1 d:1<br>
 
total number of hits: 234<br>
total number of misses: 31<br>
miss rate: 11.7%<br>
total number of dirty blocks: 4<br>
average memory access cycle: 29.4<br>
