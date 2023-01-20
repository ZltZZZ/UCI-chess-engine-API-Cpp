# UCI chess engine API C++

## Description
This is a C++ library, that allows you load and close chess engine, send and recieve commands and messages to and from engine, synchronize engine and your programm.  

Requirements: Windows (later Linux), C++.
 
 ### How to use
Include files UCI_API.cpp and UCI_API.h in your project, and #include .h file.  

All other useful information you can find in specification.
 
 ### Simple example
 ```C++
 int main(){  
    UCI_Engine engn(L"Stockfish.exe");
    std::string answer;
    
    engn.load();
    
    engn.send_command(_uci);
    if (engn.wait_answ(1000) != ENGINE_OK) {
       std::cout << "UCI interface not supported or engine isn't working!\n";
       engn.close();
       exit(-1);
    }
    answer = engn.get_answ_string();
    std::cout << answer << std::endl;
    
    engn.send_command(_go, _movetime, 1000, _depth, 4, _searchmoves, "e2e4");
    Sleep(1000);
    while (engn.check_answ()) 
    {  
       char c = engn.get_answ_char();
       std::cout << c;
    }
    
    engn.close();
    return 0;
 }
 ```
### Output
```
Stockfish 15.1 by the Stockfish developers (see AUTHORS file)  

id name Stockfish 15.1  
id author the Stockfish developers (see AUTHORS file)  

option name Debug Log File type string default  
option name Threads type spin default 1 min 1 max 1024  
option name Hash type spin default 16 min 1 max 33554432  
option name Clear Hash type button  
option name Ponder type check default false  
option name MultiPV type spin default 1 min 1 max 500  
option name Skill Level type spin default 20 min 0 max 20  
option name Move Overhead type spin default 10 min 0 max 5000  
option name Slow Mover type spin default 100 min 10 max 1000  
option name nodestime type spin default 0 min 0 max 10000  
option name UCI_Chess960 type check default false  
option name UCI_AnalyseMode type check default false  
option name UCI_LimitStrength type check default false  
option name UCI_Elo type spin default 1350 min 1350 max 2850  
option name UCI_ShowWDL type check default false  
option name SyzygyPath type string default <empty>  
option name SyzygyProbeDepth type spin default 1 min 1 max 100  
option name Syzygy50MoveRule type check default true  
option name SyzygyProbeLimit type spin default 7 min 0 max 7  
option name Use NNUE type check default true  
option name EvalFile type string default nn-ad9b42354671.nnue  
uciok  
info string NNUE evaluation using nn-ad9b42354671.nnue enabled  
info depth 1 seldepth 1 multipv 1 score cp 18 nodes 1 nps 1000 hashfull 0 tbhits 0 time 1 pv e2e4  
info depth 2 seldepth 2 multipv 1 score cp 18 nodes 2 nps 2000 hashfull 0 tbhits 0 time 1 pv e2e4  
info depth 3 seldepth 2 multipv 1 score cp 18 nodes 3 nps 3000 hashfull 0 tbhits 0 time 1 pv e2e4  
info depth 4 seldepth 2 multipv 1 score cp 18 nodes 4 nps 4000 hashfull 0 tbhits 0 time 1 pv e2e4  
bestmove e2e4
 ```
