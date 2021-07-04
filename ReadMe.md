# Direct Rainfall Model Approximation
## Final Year Engineering Thesis

This program has been created to accept inputs from a TUFLOW Rainfall on Grid model and delineate catchments and calculate hydrographs using the Time-Area Method hydrological method.

These outputs are intended to be used to check the calibration and verify the outputs of the DRM approach used by TUFLOW.

### Command Line Arguments (CLAs)

>            [Executable] "Relative Path to Grid Data" [Options]

#### Options

| Short CLA | Long CLA          | Description                                                                       |
| --------- | ----------------- | --------------------------------------------------------------------------------- |
| **-t**    | **--treereuse**   | Reuse previously generated tree structures for computation                        |
| **-m**    | **--memorylimit** | Set the programs memory limit, the default value is 1000MB                        |
| **-a**    | **--ascfiles**    | Force use of ASC files instead of CSV data                                        |
| **-s**    | **--spacing**     | Set the spacing between data points in the x and y direction. default value is 1m |

This project is currently a work in progress and should not be used without reading the Thesis documents attached to it.

To obtain a copy of the Thesis documents please email ben.olsen@cqumail.com

---

Licensed under the Creative Commons Attribution-NonCommercial 3.0 Australia (CC BY-NC 3.0 AU)
