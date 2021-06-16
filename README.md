# WriteEfficientTiling
[![DOI](https://zenodo.org/badge/372949366.svg)](https://zenodo.org/badge/latestdoi/372949366)


This source code illustrates the implementation of our DAC'20 paper, titled: (WET: Write Efficient Loop Tiling for Non-Volatile Main Memory).
The code represents the 3 main schemes evaluated in the paper, each of which is located in a separate directory, as follows:
1. Reg_MM: Regular Matrix Multiplication, with no loop tiling
2. Base_tmm: Normal single-level tiling technique used in the litriture and focuses on performance only
3. WET: The proposed WriteEfficientTiling technique that has an extra level of tiling (OuterTile) that is designed to reduce number of writes to the NVMM


In each of these directories, the main code is located in file (test.cc). The code can be built using the Makefile found in each of the directories, as follows:
1. make gem5: builds a version that will activate the gem5 simulator tools, for simulation-based evaluations
2. make pcm: builds a version that will activate the [Intel PCM tools](https://software.intel.com/content/www/us/en/develop/articles/intel-performance-counter-monitor.html), for evaluations based on real-hardware. The path of Intel PCM tools in your system should be indicated in the Makefile through the variable (pcm_location). 
