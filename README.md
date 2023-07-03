# IT3Dumper
This tool reads IT3 files from falcom games using the Yamaneko engine (or, at least Ys VIII and IX are supported) and extracts models along their textures.
It is the first tool to my knowledge that allows extracting animations to be reused in other games, but as it is experimental some problems could occur, and expect manual fix to be necessary.
# Current state
- Mesh export (OK)
- Textures export (OK)
- UV maps (Not perfect)
- Materials (Not really)
- Animations (~OK)
# Instructions
First run the following command:  
  
`IT3Dumper.exe <regular CXXXX.it3 file> <CXXXXm.it3 file> <CXXXX.mtb file>`  
  
Note that CXXXX.it3 file contains the mesh, textures, etc. While CXXXXm (or CXXX_m) contains the key frames (likely m is for "motion"). Finally the .mtb file contains the list of animations.  

Running the command for the first time will generate a file called "exported_animations.txt". It will contain the complete list of animations' names for the character. It is also an input file that will help you select the animations you want to export.  

When running the same command as above:   
  
`IT3Dumper.exe <regular CXXXX.it3 file> <CXXXXm.it3 file> <CXXXX.mtb file>`  

  
and having "exported_animations.txt" in your working directory, the animations contained in "exported_animations.txt" (and only them) will be extracted and put inside the .fbx.  
![image](https://github.com/TwnKey/IT3Dumper/assets/69110695/4a9bc0b6-37bc-4a13-bdb2-b35f8f021ed1)

# Known problems
While most animations should be working, some need a manual fix. Some have a gimbal lock problem due to assimp converting the quaternions back to euler. I don't know if that's a requirement for the FBX file format but whatever the reason it is retarded.

