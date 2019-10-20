# TDS Model
A computational model for reactions occurring during Thermal Desorption Spectroscopy, supporting automated search of parametres that fit experimental data

## Compilation

Use Qt's tools to load the `tds_model.pro` file and use it to compile the project. Dependencies are OpenCL and Qt, both should be easily available for all platforms.

## Use

The text fields on the left are the currently set parametres. The *Simulate* button causes the program to compute a desorption curve from the parametres. The *Load Comparison* button adds a reference curve using existing data (saved in a file with one column for temperatures and another for partial pressures), using the *Column* field to select which column contains the data (1 means first column after the one with temperatures) and the *Multiplier* column that multiplies the values to be more suitable for computation.

The *Fit* button automatically modifies the parametres to better fit the supplied data. The number of times it is done is set in the text field next to it. The second column of numeric parametres determines the size of steps how these parametres are modified. The are automatically increased and decreased to help the process, but can be set manually or reset using the *Reset* button under them. This process requires an OpenCL capable GPU. Doing this is not recommended on Intel GPUs, because they don't support OpenCL well and may crash the operating system.
