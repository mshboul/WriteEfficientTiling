# WriteEfficientTiling
[![DOI](https://zenodo.org/badge/372949366.svg)](https://zenodo.org/badge/latestdoi/372949366)


This source code illustrates the implementation of our DAC'20 paper, titled: (WET: Write Efficient Loop Tiling for Non-Volatile Main Memory). The full citation of paper is listed below. Please use that citation to cite our paper if the code provided in this repository helped you.
The code represents the 3 main schemes evaluated in the paper, each of which is located in a separate directory, as follows:
1. Reg_MM: Regular Matrix Multiplication, with no loop tiling
2. Base_tmm: Normal single-level tiling technique used in the litriture and focuses on performance only
3. WET: The proposed WriteEfficientTiling technique that has an extra level of tiling (OuterTile) that is designed to reduce number of writes to the NVMM


In each of these directories, the main code is located in file (test.cc). The code can be built using the Makefile found in each of the directories, as follows:
1. make: builds a version that performs the computation only (without stats collection), which helps for debugging/implementation purpose since it doesn't require any additional special libraries.
2. make gem5: builds a version that will activate the gem5 simulator tools, for simulation-based evaluations. The path of the gem5 m5thread library in your system should be indicated in the Makefile through the variable (m5threads_location), in addition to providing the path to the g++ version that is compatible with your gem5 simulator, using the variable (gPP_location).
3. make pcm: builds a version that will activate the [Intel PCM tools](https://software.intel.com/content/www/us/en/develop/articles/intel-performance-counter-monitor.html), for evaluations based on real-hardware. The path of Intel PCM tools in your system should be indicated in the Makefile through the variable (pcm_location). 


**System Requirements:**

The code should work on any Linux system with proper C++ compiler. However, our testing was mainly conducted on Red Hat Enterprise Linux Server (version 7.6). Using gcc version 4.8.5. In addition, when using the gem5 or PCM evaluation modes, the corresponding libraries must be installed in the system and indicated in the Makefile following the instructions mentioned above.



**Full Paper Citation:**

Please use this to cite our paper if this code helped you:

M. Alshboul, J. Tuck and Y. Solihin, "WET: Write Efficient Loop Tiling for Non-Volatile Main Memory", 2020 57th ACM/IEEE Design Automation Conference (DAC), 2020, pp. 1-6, doi: 10.1109/DAC18072.2020.9218612.
